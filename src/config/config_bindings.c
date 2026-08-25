// Copyright (c) Epic Games Tools
// Licensed under the MIT license (https://opensource.org/license/mit/)

internal CFG_Key_Map *
cfg_key_map_from_cfg(Arena *arena)
{
  Temp scratch = scratch_begin(&arena, 1);
  CFG_Key_Map *key_map = push_array(arena, CFG_Key_Map, 1);
  {
    key_map->name_slots_count = 4096;
    key_map->name_slots = push_array(arena, CFG_Key_Map_Slot, key_map->name_slots_count);
    key_map->binding_slots_count = 4096;
    key_map->binding_slots = push_array(arena, CFG_Key_Map_Slot, key_map->binding_slots_count);
    
    //- rjf: gather & parse all explicitly stored keybinding sets
    CFG_Node_Ptr_List keybindings_cfg_list = cfg_node_top_level_list_from_string(scratch.arena, str8_lit("keybindings"));
    for(CFG_Node_Ptr_Node *n = keybindings_cfg_list.first; n != 0; n = n->next)
    {
      CFG_Node *keybindings_root = n->v;
      for(CFG_Node *keybinding = keybindings_root->first; keybinding != &cfg_nil_node; keybinding = keybinding->next)
      {
        String8 name = {0};
        CFG_Binding binding = {0};
        for(CFG_Node *child = keybinding->first; child != &cfg_nil_node; child = child->next)
        {
          if(0){}
          else if(str8_match(child->string, str8_lit("ctrl"), 0))   { binding.modifiers |= WM_Modifier_Ctrl; }
          else if(str8_match(child->string, str8_lit("alt"), 0))    { binding.modifiers |= WM_Modifier_Alt; }
          else if(str8_match(child->string, str8_lit("shift"), 0))  { binding.modifiers |= WM_Modifier_Shift; }
          else
          {
            WM_Key key = WM_Key_Null;
            for EachEnumVal(WM_Key, k)
            {
              if(str8_match(child->string, wm_key_cfg_name_table[k], StringMatchFlag_CaseInsensitive))
              {
                key = k;
                break;
              }
            }
            if(key != WM_Key_Null)
            {
              binding.key = key;
            }
            else
            {
              name = child->string;
            }
          }
        }
        if(name.size != 0)
        {
          u64 name_hash = u64_hash_from_str8(name);
          u64 binding_hash = u64_hash_from_str8(str8_struct(&binding));
          u64 name_slot_idx = name_hash%key_map->name_slots_count;
          u64 binding_slot_idx = binding_hash%key_map->binding_slots_count;
          CFG_Key_Map_Node *n = push_array(arena, CFG_Key_Map_Node, 1);
          n->cfg_id = keybinding->id;
          n->name = push_str8_copy(arena, name);
          n->binding = binding;
          SLLQueuePush_N(key_map->name_slots[name_slot_idx].first, key_map->name_slots[name_slot_idx].last, n, name_hash_next);
          SLLQueuePush_N(key_map->binding_slots[binding_slot_idx].first, key_map->binding_slots[binding_slot_idx].last, n, binding_hash_next);
        }
      }
    }
  }
  scratch_end(scratch);
  return key_map;
}

internal CFG_Key_Map_Node_Ptr_List
cfg_key_map_node_ptr_list_from_name(Arena *arena, CFG_Key_Map *key_map, String8 string)
{
  CFG_Key_Map_Node_Ptr_List list = {0};
  {
    u64 hash = u64_hash_from_str8(string);
    u64 slot_idx = hash%key_map->name_slots_count;
    for(CFG_Key_Map_Node *n = key_map->name_slots[slot_idx].first; n != 0; n = n->name_hash_next)
    {
      if(str8_match(n->name, string, 0))
      {
        CFG_Key_Map_Node_Ptr *ptr = push_array(arena, CFG_Key_Map_Node_Ptr, 1);
        ptr->v = n;
        SLLQueuePush(list.first, list.last, ptr);
        list.count += 1;
      }
    }
  }
  return list;
}

internal CFG_Key_Map_Node_Ptr_List
cfg_key_map_node_ptr_list_from_binding(Arena *arena, CFG_Key_Map *key_map, CFG_Binding binding)
{
  CFG_Key_Map_Node_Ptr_List list = {0};
  {
    u64 hash = u64_hash_from_str8(str8_struct(&binding));
    u64 slot_idx = hash%key_map->binding_slots_count;
    for(CFG_Key_Map_Node *n = key_map->binding_slots[slot_idx].first; n != 0; n = n->binding_hash_next)
    {
      if(MemoryMatchStruct(&binding, &n->binding))
      {
        CFG_Key_Map_Node_Ptr *ptr = push_array(arena, CFG_Key_Map_Node_Ptr, 1);
        ptr->v = n;
        SLLQueuePush(list.first, list.last, ptr);
        list.count += 1;
      }
    }
  }
  return list;
}
