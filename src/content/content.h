// Copyright (c) Epic Games Tools
// Licensed under the MIT license (https://opensource.org/license/mit/)

#ifndef CONTENT_H
#define CONTENT_H

////////////////////////////////
//~ NOTE(rjf): Hash Store Notes (2025/05/18)
//
// The hash store is a general-purpose data cache. It offers three layers of
// caching: (a) content (hash of data), (b) key (unique identity correllated
// with history of hashes), and (c) root (bucket for many keys, manually
// allocated / deallocated).
//
//  (a) The "content" level of cache access is a simply hash(data) -> data
//      mapping. This bypasses all identity/key/root mechanisms and provides a
//      way to just talk about unique (and deduplicated) blobs of data.
//
//  (b) The "key" level of cache access is used to encode a history of hashes
//      for some unique "identity", where the "identity" is a concept managed
//      by the user. One example of an identity would be a particular address
//      range inside of some process to which the debugger is attached. Another
//      might be a range inside of some file.
//
//  (c) The "root" level is to provide a top-level allocation/deallocation
//      mechanism for a large set of keys. It also provides an extra level of
//      key uniqueness. For instance, each process to which the debugger is
//      attached might have its own root, and each key might correspond to a
//      particular address range within that process. This way, when the
//      process ends, all of its keys can be easily destroyed using a single
//      deallocation of the root.
//
// The way this might be generally used inside of the debugger would be that
// some evaluation - let's say it's some variable `x` - is mapped (via debug
// info) to some address range. If `x` is a `char[4096]`, then it might map
// to some address range [&x, &x + 4096). This, together with the process
// within which `x` is evaluated, forms both a `root` (for the process) and
// a `key` (for the address range). Some asynchronous memory streaming system
// can then, together with the root and key, read memory for that range, then
// submit that data to the hash store, correllating with the root and key
// combo.

///////////////
// Key Types

typedef struct C_Root C_Root;
struct C_Root
{
  u64 u64[1];
};

typedef struct C_ID C_ID;
struct C_ID
{
  u128 u128[1];
};

typedef struct C_Key C_Key;
struct C_Key
{
  C_Root root;
  u64 _padding_;
  C_ID id;
};

////////////////////////////////
//~ rjf: Cache Stripe Type

typedef struct C_Stripe C_Stripe;
struct C_Stripe
{
  Arena *arena;
  RWMutex rw_mutex;
  CondVar cv;
};

////////////////////////////////
//~ rjf: Root Cache Types

typedef struct C_Root_ID_Chunk_Node C_Root_ID_Chunk_Node;
struct C_Root_ID_Chunk_Node
{
  C_Root_ID_Chunk_Node *next;
  C_ID *v;
  u64 count;
  u64 cap;
};

typedef struct C_Root_ID_Chunk_List C_Root_ID_Chunk_List;
struct C_Root_ID_Chunk_List
{
  C_Root_ID_Chunk_Node *first;
  C_Root_ID_Chunk_Node *last;
  u64 chunk_count;
  u64 total_count;
};

typedef struct C_Root_Node C_Root_Node;
struct C_Root_Node
{
  C_Root_Node *next;
  C_Root_Node *prev;
  Arena *arena;
  C_Root root;
  C_Root_ID_Chunk_List ids;
};

typedef struct C_Root_Slot C_Root_Slot;
struct C_Root_Slot
{
  C_Root_Node *first;
  C_Root_Node *last;
};

////////////////////////////////
//~ rjf: Key Cache Types

#define C_KEY_HASH_HISTORY_COUNT 64
#define C_KEY_HASH_HISTORY_STRONG_REF_COUNT 2

typedef struct C_Key_Node C_Key_Node;
struct C_Key_Node
{
  C_Key_Node *next;
  C_Key_Node *prev;
  C_Key key;
  u128 hash_history[C_KEY_HASH_HISTORY_COUNT];
  u64 hash_history_gen;
};

typedef struct C_Key_Slot C_Key_Slot;
struct C_Key_Slot
{
  C_Key_Node *first;
  C_Key_Node *last;
};

////////////////////////////////
//~ rjf: Content Blob Cache Types

typedef struct C_Blob_Node C_Blob_Node;
struct C_Blob_Node
{
  C_Blob_Node *next;
  C_Blob_Node *prev;
  u128 hash;
  Arena *arena;
  String8 data;
  Access_Pt access_pt;
  u64 key_ref_count;
  u64 downstream_ref_count;
};

typedef struct C_Blob_Slot C_Blob_Slot;
struct C_Blob_Slot
{
  C_Blob_Node *first;
  C_Blob_Node *last;
};

////////////////////////////////
//~ rjf: Shared State

typedef struct C_Shared C_Shared;
struct C_Shared
{
  Arena *arena;
  
  // rjf: main data blob cache
  u64 blob_slots_count;
  u64 blob_stripes_count;
  C_Blob_Slot *blob_slots;
  C_Stripe *blob_stripes;
  C_Blob_Node **blob_stripes_free_nodes;
  
  // rjf: key cache
  u64 key_slots_count;
  u64 key_stripes_count;
  C_Key_Slot *key_slots;
  C_Stripe *key_stripes;
  C_Key_Node **key_stripes_free_nodes;
  
  // rjf: root cache
  u64 root_slots_count;
  u64 root_stripes_count;
  C_Root_Slot *root_slots;
  C_Stripe *root_stripes;
  C_Root_Node **root_stripes_free_nodes;
  u64 root_id_gen;
};

////////////////////////////////
//~ rjf: Globals

global C_Shared *c_shared = 0;

////////////////////////////////
//~ rjf: Basic Helpers

internal C_ID c_id_make(u64 u64_0, u64 u64_1);
internal bool32 c_id_match(C_ID a, C_ID b);
internal C_Key c_key_make(C_Root root, C_ID id);
internal bool32 c_key_match(C_Key a, C_Key b);

////////////////////////////////
//~ rjf: Main Layer Initialization

internal void c_init(void);

////////////////////////////////
//~ rjf: Root Allocation/Deallocation

internal C_Root c_root_alloc(void);
internal void c_root_release(C_Root root);

////////////////////////////////
//~ rjf: Cache Submission

internal u128 c_submit_data(C_Key key, Arena **data_arena, String8 data);

////////////////////////////////
//~ rjf: Key Closing

internal void c_close_key(C_Key key);

////////////////////////////////
//~ rjf: Downstream Accesses

internal void c_hash_downstream_inc(u128 hash);
internal void c_hash_downstream_dec(u128 hash);

////////////////////////////////
//~ rjf: Cache Lookups

internal u128 c_hash_from_key(C_Key key, u64 rewind_count);
internal String8 c_data_from_hash(Access *access, u128 hash);

////////////////////////////////
//~ rjf: Asynchronous Tick

#if !defined(NEED_ASYNC)
# define NEED_ASYNC 1
#endif
internal void c_async_tick(void);


#endif // CONTENT_H
