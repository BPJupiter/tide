// Copyright (c) Epic Games Tools
// Licensed under the MIT license (https://opensource.org/license/mit/)

#undef LAYER_COLOR
#define LAYER_COLOR 0x684123ff

////////////////////////////////
//~ rjf: Basic Helpers

internal C_ID
c_id_make(u64 u64_0, u64 u64_1)
{
  C_ID id;
  id.u128[0].u64[0] = u64_0;
  id.u128[0].u64[1] = u64_1;
  return id;
}

internal bool32
c_id_match(C_ID a, C_ID b)
{
  bool32 result = MemoryMatchStruct(&a, &b);
  return result;
}

internal C_Key
c_key_make(C_Root root, C_ID id)
{
  C_Key key = {root, 0, id};
  return key;
}

internal bool32
c_key_match(C_Key a, C_Key b)
{
  return (MemoryMatchStruct(&a.root, &b.root) && c_id_match(a.id, b.id));
}

////////////////////////////////
//~ rjf: Main Layer Initialization

internal void
c_init(void)
{
  Arena *arena = arena_alloc();
  c_shared = push_array(arena, C_Shared, 1);
  c_shared->arena = arena;
  c_shared->blob_slots_count = 16384;
  c_shared->blob_stripes_count = Min(c_shared->blob_slots_count, get_system_info()->logical_processor_count);
  c_shared->blob_slots = push_array(arena, C_Blob_Slot, c_shared->blob_slots_count);
  c_shared->blob_stripes = push_array(arena, C_Stripe, c_shared->blob_stripes_count);
  c_shared->blob_stripes_free_nodes = push_array(arena, C_Blob_Node *, c_shared->blob_stripes_count);
  for(u64 idx = 0; idx < c_shared->blob_stripes_count; idx += 1)
  {
    C_Stripe *stripe = &c_shared->blob_stripes[idx];
    stripe->arena = arena_alloc();
    stripe->rw_mutex = rw_mutex_alloc();
    stripe->cv = cond_var_alloc();
  }
  c_shared->key_slots_count = 4096;
  c_shared->key_stripes_count = Min(c_shared->key_slots_count, get_system_info()->logical_processor_count);
  c_shared->key_slots = push_array(arena, C_Key_Slot, c_shared->key_slots_count);
  c_shared->key_stripes = push_array(arena, C_Stripe, c_shared->key_stripes_count);
  c_shared->key_stripes_free_nodes = push_array(arena, C_Key_Node *, c_shared->key_stripes_count);
  for(u64 idx = 0; idx < c_shared->key_stripes_count; idx += 1)
  {
    C_Stripe *stripe = &c_shared->key_stripes[idx];
    stripe->arena = arena_alloc();
    stripe->rw_mutex = rw_mutex_alloc();
    stripe->cv = cond_var_alloc();
  }
  c_shared->root_slots_count = 4096;
  c_shared->root_stripes_count = Min(c_shared->root_slots_count, get_system_info()->logical_processor_count);
  c_shared->root_slots = push_array(arena, C_Root_Slot, c_shared->root_slots_count);
  c_shared->root_stripes = push_array(arena, C_Stripe, c_shared->root_stripes_count);
  c_shared->root_stripes_free_nodes = push_array(arena, C_Root_Node *, c_shared->root_stripes_count);
  for(u64 idx = 0; idx < c_shared->root_stripes_count; idx += 1)
  {
    C_Stripe *stripe = &c_shared->root_stripes[idx];
    stripe->arena = arena_alloc();
    stripe->rw_mutex = rw_mutex_alloc();
    stripe->cv = cond_var_alloc();
  }
}

////////////////////////////////
//~ rjf: Root Allocation/Deallocation

internal C_Root
c_root_alloc(void)
{
  C_Root root = {0};
  root.u64[0] = ins_atomic_u64_inc_eval(&c_shared->root_id_gen);
  u64 slot_idx = root.u64[0]%c_shared->root_slots_count;
  u64 stripe_idx = slot_idx%c_shared->root_stripes_count;
  C_Root_Slot *slot = &c_shared->root_slots[slot_idx];
  C_Stripe *stripe = &c_shared->root_stripes[stripe_idx];
  MutexScopeW(stripe->rw_mutex)
  {
    C_Root_Node *node = c_shared->root_stripes_free_nodes[stripe_idx];
    if(node != 0)
    {
      SLLStackPop(c_shared->root_stripes_free_nodes[stripe_idx]);
    }
    else
    {
      node = push_array(stripe->arena, C_Root_Node, 1);
    }
    DLLPushBack(slot->first, slot->last, node);
    node->root = root;
    node->arena = arena_alloc();
  }
  return root;
}

internal void
c_root_release(C_Root root)
{
  //- rjf: unpack root
  u64 slot_idx = root.u64[0]%c_shared->root_slots_count;
  u64 stripe_idx = slot_idx%c_shared->root_stripes_count;
  C_Root_Slot *slot = &c_shared->root_slots[slot_idx];
  C_Stripe *stripe = &c_shared->root_stripes[stripe_idx];
  
  //- rjf: release root node, grab its arena / ID list
  Arena *root_arena = 0;
  C_Root_ID_Chunk_List root_ids = {0};
  MutexScopeW(stripe->rw_mutex)
  {
    for(C_Root_Node *n = slot->first; n != 0; n = n->next)
    {
      if(MemoryMatchStruct(&root, &n->root))
      {
        DLLRemove(slot->first, slot->last, n);
        root_arena = n->arena;
        root_ids = n->ids;
        SLLStackPush(c_shared->root_stripes_free_nodes[stripe_idx], n);
        break;
      }
    }
  }
  
  //- rjf: release all IDs
  for(C_Root_ID_Chunk_Node *id_chunk_n = root_ids.first; id_chunk_n != 0; id_chunk_n = id_chunk_n->next)
  {
    for EachIndex(chunk_idx, id_chunk_n->count)
    {
      C_ID id = id_chunk_n->v[chunk_idx];
      C_Key key = c_key_make(root, id);
      c_close_key(key);
    }
  }
}

////////////////////////////////
//~ rjf: Cache Submission

internal u128
c_submit_data(C_Key key, Arena **data_arena, String8 data)
{
  //- rjf: unpack key
  u64 key_hash = u64_hash_from_str8(str8_struct(&key));
  u64 key_slot_idx = key_hash%c_shared->key_slots_count;
  u64 key_stripe_idx = key_slot_idx%c_shared->key_stripes_count;
  C_Key_Slot *key_slot = &c_shared->key_slots[key_slot_idx];
  C_Stripe *key_stripe = &c_shared->key_stripes[key_stripe_idx];
  
  //- rjf: hash data, unpack hash
  u128 hash = u128_hash_from_str8(data);
  u64 slot_idx = hash.u64[1]%c_shared->blob_slots_count;
  u64 stripe_idx = slot_idx%c_shared->blob_stripes_count;
  C_Blob_Slot *slot = &c_shared->blob_slots[slot_idx];
  C_Stripe *stripe = &c_shared->blob_stripes[stripe_idx];
  
  //- rjf: commit to (hash -> data) cache
  ProfScope("commit to (hash -> data) cache") RWMutexScope(stripe->rw_mutex, 1)
  {
    // rjf: find existing node
    C_Blob_Node *node = 0;
    for(C_Blob_Node *n = slot->first; n != 0; n = n->next)
    {
      if(u128_match(n->hash, hash))
      {
        node = n;
        break;
      }
    }
    
    // rjf: release duplicate data if node already exists
    if(node != 0 && data_arena != 0 && *data_arena != 0)
    {
      arena_release(*data_arena);
    }
    
    // rjf: allocate node if needed
    if(node == 0)
    {
      node = c_shared->blob_stripes_free_nodes[stripe_idx];
      if(node)
      {
        SLLStackPop(c_shared->blob_stripes_free_nodes[stripe_idx]);
      }
      else
      {
        node = push_array_no_zero(stripe->arena, C_Blob_Node, 1);
      }
      MemoryZeroStruct(node);
      node->hash = hash;
      if(data_arena != 0)
      {
        node->arena = *data_arena;
      }
      node->data = data;
      DLLPushBack(slot->first, slot->last, node);
    }
    
    // rjf: bump key ref count
    node->key_ref_count += 1;
    
    // rjf "steal" arena from caller
    if(data_arena != 0)
    {
      *data_arena = 0;
    }
  }
  
  //- rjf: commit to (key -> list(hash)) cache
  u128 key_expired_hash = {0};
  ProfScope("commit to (key -> list(hash)) cache") RWMutexScope(key_stripe->rw_mutex, 1)
  {
    // rjf: find existing key
    C_Key_Node *key_node = 0;
    for(C_Key_Node *n = key_slot->first; n != 0; n = n->next)
    {
      if(c_key_match(n->key, key))
      {
        key_node = n;
        break;
      }
    }
    
    // rjf: create key node if it doesn't exist
    bool32 key_is_new = 0;
    if(!key_node)
    {
      key_is_new = 1;
      key_node = c_shared->key_stripes_free_nodes[key_stripe_idx];
      if(key_node)
      {
        SLLStackPop(c_shared->key_stripes_free_nodes[key_stripe_idx]);
      }
      else
      {
        key_node = push_array_no_zero(key_stripe->arena, C_Key_Node, 1);
      }
      MemoryZeroStruct(key_node);
      key_node->key = key;
      DLLPushBack(key_slot->first, key_slot->last, key_node);
    }
    
    // rjf: push hash into key's history
    if(key_node)
    {
      if(key_node->hash_history_gen >= C_KEY_HASH_HISTORY_STRONG_REF_COUNT)
      {
        key_expired_hash = key_node->hash_history[(key_node->hash_history_gen-C_KEY_HASH_HISTORY_STRONG_REF_COUNT)%ArrayCount(key_node->hash_history)];
      }
      key_node->hash_history[key_node->hash_history_gen%ArrayCount(key_node->hash_history)] = hash;
      key_node->hash_history_gen += 1;
    }
    
    // rjf: key is new -> add this key to the associated root
    if(key_is_new)
    {
      u64 root_hash = u64_hash_from_str8(str8_struct(&key.root));
      u64 root_slot_idx = root_hash%c_shared->root_slots_count;
      u64 root_stripe_idx = root_slot_idx%c_shared->root_stripes_count;
      C_Root_Slot *root_slot = &c_shared->root_slots[root_slot_idx];
      C_Stripe *root_stripe = &c_shared->root_stripes[root_stripe_idx];
      RWMutexScope(root_stripe->rw_mutex, 1)
      {
        for(C_Root_Node *n = root_slot->first; n != 0; n = n->next)
        {
          if(MemoryMatchStruct(&n->root, &key.root))
          {
            C_Root_ID_Chunk_Node *chunk = n->ids.last;
            if(chunk == 0 || chunk->count >= chunk->cap)
            {
              chunk = push_array(n->arena, C_Root_ID_Chunk_Node, 1);
              SLLQueuePush(n->ids.first, n->ids.last, chunk);
              n->ids.chunk_count += 1;
              chunk->cap = 1024;
              chunk->v = push_array_no_zero(n->arena, C_ID, chunk->cap);
            }
            chunk->v[chunk->count] = key.id;
            chunk->count += 1;
            n->ids.total_count += 1;
            break;
          }
        }
      }
    }
  }
  
  //- rjf: decrement key ref count of expired hash
  if(!u128_match(key_expired_hash, u128_zero())) ProfScope("decrement key ref count of expired hash")
  {
    u64 old_hash_slot_idx = key_expired_hash.u64[1]%c_shared->blob_slots_count;
    u64 old_hash_stripe_idx = old_hash_slot_idx%c_shared->blob_stripes_count;
    C_Blob_Slot *old_hash_slot = &c_shared->blob_slots[old_hash_slot_idx];
    C_Stripe *old_hash_stripe = &c_shared->blob_stripes[old_hash_stripe_idx];
    RWMutexScope(old_hash_stripe->rw_mutex, 0)
    {
      for(C_Blob_Node *n = old_hash_slot->first; n != 0; n = n->next)
      {
        if(u128_match(n->hash, key_expired_hash))
        {
          ins_atomic_u64_dec_eval(&n->key_ref_count);
          break;
        }
      }
    }
  }
  
  return hash;
}

////////////////////////////////
//~ rjf: Key Closing

internal void
c_close_key(C_Key key)
{
  u64 key_hash = u64_hash_from_str8(str8_struct(&key));
  u64 key_slot_idx = key_hash%c_shared->key_slots_count;
  u64 key_stripe_idx = key_slot_idx%c_shared->key_stripes_count;
  C_Key_Slot *key_slot = &c_shared->key_slots[key_slot_idx];
  C_Stripe *key_stripe = &c_shared->key_stripes[key_stripe_idx];
  RWMutexScope(key_stripe->rw_mutex, 1)
  {
    for(C_Key_Node *n = key_slot->first; n != 0; n = n->next)
    {
      if(c_key_match(n->key, key))
      {
        for(u64 history_idx = 0;
            history_idx < C_KEY_HASH_HISTORY_STRONG_REF_COUNT && history_idx < n->hash_history_gen;
            history_idx += 1)
        {
          u128 hash = n->hash_history[(n->hash_history_gen-1-history_idx) % ArrayCount(n->hash_history)];
          u64 hash_slot_idx = hash.u64[1]%c_shared->blob_slots_count;
          u64 hash_stripe_idx = hash_slot_idx%c_shared->blob_stripes_count;
          C_Blob_Slot *hash_slot = &c_shared->blob_slots[hash_slot_idx];
          C_Stripe *hash_stripe = &c_shared->blob_stripes[hash_stripe_idx];
          MutexScopeR(hash_stripe->rw_mutex)
          {
            for(C_Blob_Node *n = hash_slot->first; n != 0; n = n->next)
            {
              if(u128_match(n->hash, hash))
              {
                ins_atomic_u64_dec_eval(&n->key_ref_count);
                break;
              }
            }
          }
        }
        DLLRemove(key_slot->first, key_slot->last, n);
        SLLStackPush(c_shared->key_stripes_free_nodes[key_stripe_idx], n);
        break;
      }
    }
  }
}

////////////////////////////////
//~ rjf: Downstream Accesses

internal void
c_hash_downstream_inc(u128 hash)
{
  u64 slot_idx = hash.u64[1]%c_shared->blob_slots_count;
  u64 stripe_idx = slot_idx%c_shared->blob_stripes_count;
  C_Blob_Slot *slot = &c_shared->blob_slots[slot_idx];
  C_Stripe *stripe = &c_shared->blob_stripes[stripe_idx];
  MutexScopeR(stripe->rw_mutex)
  {
    for(C_Blob_Node *n = slot->first; n != 0; n = n->next)
    {
      if(u128_match(hash, n->hash))
      {
        ins_atomic_u64_inc_eval(&n->downstream_ref_count);
        break;
      }
    }
  }
}

internal void
c_hash_downstream_dec(u128 hash)
{
  u64 slot_idx = hash.u64[1]%c_shared->blob_slots_count;
  u64 stripe_idx = slot_idx%c_shared->blob_stripes_count;
  C_Blob_Slot *slot = &c_shared->blob_slots[slot_idx];
  C_Stripe *stripe = &c_shared->blob_stripes[stripe_idx];
  MutexScopeR(stripe->rw_mutex)
  {
    for(C_Blob_Node *n = slot->first; n != 0; n = n->next)
    {
      if(u128_match(hash, n->hash))
      {
        ins_atomic_u64_dec_eval(&n->downstream_ref_count);
        break;
      }
    }
  }
}

////////////////////////////////
//~ rjf: Cache Lookup

internal u128
c_hash_from_key(C_Key key, u64 rewind_count)
{
  u128 result = {0};
  u64 key_hash = u64_hash_from_str8(str8_struct(&key));
  u64 key_slot_idx = key_hash%c_shared->key_slots_count;
  u64 key_stripe_idx = key_slot_idx%c_shared->key_stripes_count;
  C_Key_Slot *key_slot = &c_shared->key_slots[key_slot_idx];
  C_Stripe *key_stripe = &c_shared->key_stripes[key_stripe_idx];
  RWMutexScope(key_stripe->rw_mutex, 0)
  {
    for(C_Key_Node *n = key_slot->first; n != 0; n = n->next)
    {
      if(c_key_match(n->key, key) && n->hash_history_gen > 0 && n->hash_history_gen-1 >= rewind_count)
      {
        result = n->hash_history[(n->hash_history_gen-1-rewind_count)%ArrayCount(n->hash_history)];
        break;
      }
    }
  }
  return result;
}

internal String8
c_data_from_hash(Access *access, u128 hash)
{
  ProfBeginFunction();
  String8 result = {0};
  u64 slot_idx = hash.u64[1]%c_shared->blob_slots_count;
  u64 stripe_idx = slot_idx%c_shared->blob_stripes_count;
  C_Blob_Slot *slot = &c_shared->blob_slots[slot_idx];
  C_Stripe *stripe = &c_shared->blob_stripes[stripe_idx];
  MutexScopeR(stripe->rw_mutex)
  {
    for(C_Blob_Node *n = slot->first; n != 0; n = n->next)
    {
      if(u128_match(n->hash, hash))
      {
        result = n->data;
        access_touch(access, &n->access_pt, stripe->cv);
        break;
      }
    }
  }
  ProfEnd();
  return result;
}

////////////////////////////////
//~ rjf: Asynchronous Tick

internal void
c_async_tick(void)
{
  ProfBeginFunction();
  
  //- rjf: garbage collect blobs
  {
    Rng1u64 range = lane_range(c_shared->blob_slots_count);
    for EachInRange(slot_idx, range)
    {
      u64 stripe_idx = slot_idx%c_shared->blob_stripes_count;
      C_Blob_Slot *slot = &c_shared->blob_slots[slot_idx];
      C_Stripe *stripe = &c_shared->blob_stripes[stripe_idx];
      for(bool32 write_mode = 0; write_mode <= 1; write_mode += 1)
      {
        bool32 slot_has_work = 0;
        RWMutexScope(stripe->rw_mutex, write_mode)
        {
          for(C_Blob_Node *n = slot->first, *next = 0; n != 0; n = next)
          {
            next = n->next;
            u64 key_ref_count = ins_atomic_u64_eval(&n->key_ref_count);
            u64 downstream_ref_count = ins_atomic_u64_eval(&n->downstream_ref_count);
            if(access_pt_is_expired(&n->access_pt, .time = 5000000) && key_ref_count == 0 && downstream_ref_count == 0)
            {
              slot_has_work = 1;
              if(!write_mode)
              {
                break;
              }
              else
              {
                DLLRemove(slot->first, slot->last, n);
                SLLStackPush(c_shared->blob_stripes_free_nodes[stripe_idx], n);
                if(n->arena != 0)
                {
                  arena_release(n->arena);
                }
              }
            }
          }
        }
        if(!slot_has_work)
        {
          break;
        }
      }
    }
  }
  
  ProfEnd();
}
