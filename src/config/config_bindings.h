// Copyright (c) Epic Games Tools
// Licensed under the MIT license (https://opensource.org/license/mit/)

#ifndef CONFIG_BINDINGS_H
#define CONFIG_BINDINGS_H

typedef struct CFG_Binding CFG_Binding;
struct CFG_Binding
{
  WM_Key key;
  WM_Modifiers modifiers;
};

typedef struct CFG_Key_Map_Node CFG_Key_Map_Node;
struct CFG_Key_Map_Node
{
  CFG_Key_Map_Node *name_hash_next;
  CFG_Key_Map_Node *binding_hash_next;
  CFG_ID cfg_id;
  String8 name;
  CFG_Binding binding;
};

typedef struct CFG_Key_Map_Node_Ptr CFG_Key_Map_Node_Ptr;
struct CFG_Key_Map_Node_Ptr
{
  CFG_Key_Map_Node_Ptr *next;
  CFG_Key_Map_Node *v;
};

typedef struct CFG_Key_Map_Node_Ptr_List CFG_Key_Map_Node_Ptr_List;
struct CFG_Key_Map_Node_Ptr_List
{
  CFG_Key_Map_Node_Ptr *first;
  CFG_Key_Map_Node_Ptr *last;
  u64 count;
};

typedef struct CFG_Key_Map_Slot CFG_Key_Map_Slot;
struct CFG_Key_Map_Slot
{
  CFG_Key_Map_Node *first;
  CFG_Key_Map_Node *last;
};

typedef struct CFG_Key_Map CFG_Key_Map;
struct CFG_Key_Map
{
  u64 name_slots_count;
  CFG_Key_Map_Slot *name_slots;
  u64 binding_slots_count;
  CFG_Key_Map_Slot *binding_slots;
};

internal CFG_Key_Map *cfg_key_map_from_cfg(Arena *arena);
internal CFG_Key_Map_Node_Ptr_List cfg_key_map_node_ptr_list_from_name(Arena *arena, CFG_Key_Map *key_map, String8 string);
internal CFG_Key_Map_Node_Ptr_List cfg_key_map_node_ptr_list_from_binding(Arena *arena, CFG_Key_Map *key_map, CFG_Binding binding);

#endif // CONFIG_BINDINGS_H
