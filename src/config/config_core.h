// Copyright (c) Epic Games Tools
// Licensed under the MIT license (https://opensource.org/license/mit/)

#ifndef CONFIG_CORE_H
#define CONFIG_CORE_H

////////////////////////////////
//~ rjf: IDs

typedef u64 CFG_ID;

typedef struct CFG_ID_Node CFG_ID_Node;
struct CFG_ID_Node
{
  CFG_ID_Node *next;
  CFG_ID v;
};

typedef struct CFG_ID_List CFG_ID_List;
struct CFG_ID_List
{
  CFG_ID_Node *first;
  CFG_ID_Node *last;
  u64 count;
};

////////////////////////////////
//~ rjf: Tree Types

typedef struct CFG_Node CFG_Node;
struct CFG_Node
{
  CFG_Node *first;
  CFG_Node *last;
  CFG_Node *next;
  CFG_Node *prev;
  CFG_Node *parent;
  CFG_ID id;
  String8 string;
};

typedef struct CFG_Node_Ptr_Node CFG_Node_Ptr_Node;
struct CFG_Node_Ptr_Node
{
  CFG_Node_Ptr_Node *next;
  CFG_Node_Ptr_Node *prev;
  CFG_Node *v;
};

typedef struct CFG_Node_Ptr_Slot CFG_Node_Ptr_Slot;
struct CFG_Node_Ptr_Slot
{
  CFG_Node_Ptr_Node *first;
  CFG_Node_Ptr_Node *last;
};

typedef struct CFG_Node_Ptr_List CFG_Node_Ptr_List;
struct CFG_Node_Ptr_List
{
  CFG_Node_Ptr_Node *first;
  CFG_Node_Ptr_Node *last;
  u64 count;
};

typedef struct CFG_Node_Ptr_Array CFG_Node_Ptr_Array;
struct CFG_Node_Ptr_Array
{
  CFG_Node **v;
  u64 count;
};

typedef struct CFG_Node_Rec CFG_Node_Rec;
struct CFG_Node_Rec
{
  CFG_Node *next;
  s32 push_count;
  s32 pop_count;
};

////////////////////////////////
//~ rjf: String Allocator

read_only global u64 cfg_string_bucket_chunk_sizes[] =
{
    16,
    64,
    256,
    1024,
    4096,
    16384,
    65536,
    0xffffffffffffffffull,
};

typedef struct CFG_String_Chunk_Node CFG_String_Chunk_Node;
struct CFG_String_Chunk_Node
{
  CFG_String_Chunk_Node *next;
  u64 size;
};

////////////////////////////////
//~ rjf: Config State Bundles

typedef struct CFG_Ctx CFG_Ctx;
struct CFG_Ctx
{
  CFG_Node *root;
  u64 id_slots_count;
  CFG_Node_Ptr_Slot *id_slots;
  u64 change_gen;
  CFG_ID last_accessed_id;
  CFG_Node *last_accessed;
};

typedef struct CFG_State CFG_State;
struct CFG_State
{
  Arena *arena;
  CFG_Node *free;
  CFG_Node_Ptr_Node *free_id_node;
  CFG_String_Chunk_Node *free_string_chunks[ArrayCount(cfg_string_bucket_chunk_sizes)];
  u64 id_gen;
  CFG_Ctx ctx;
};

////////////////////////////////
//~ rjf: Schema Table

typedef struct CFG_Schema_Node CFG_Schema_Node;
struct CFG_Schema_Node
{
  CFG_Schema_Node *next;
  String8 name;
  MD_Node *schema;
};

typedef struct CFG_Schema_Table CFG_Schema_Table;
struct CFG_Schema_Table
{
  CFG_Schema_Node **slots;
  u64 slots_count;
};

////////////////////////////////
//~ rjf: Globals

read_only global CFG_Node cfg_nil_node =
{
    &cfg_nil_node,
    &cfg_nil_node,
    &cfg_nil_node,
    &cfg_nil_node,
    &cfg_nil_node,
};

thread_static CFG_Ctx *cfg_ctx = 0;

////////////////////////////////
//~ rjf: ID Functions

internal void cfg_id_list_push(Arena *arena, CFG_ID_List *list, CFG_ID id);
internal CFG_ID_List cfg_id_list_copy(Arena *arena, CFG_ID_List *src);

////////////////////////////////
//~ rjf: Node Pointer Data Structure Functions

internal void cfg_node_ptr_list_push(Arena *arena, CFG_Node_Ptr_List *list, CFG_Node *node);
internal void cfg_node_ptr_list_push_front(Arena *arena, CFG_Node_Ptr_List *list, CFG_Node *node);
#define cfg_node_ptr_list_first(list) ((list)->count ? (list)->first->v : &cfg_nil_node)
#define cfg_node_ptr_list_last(list)  ((list)->count ? (list)->last->v  : &cfg_nil_node)
internal CFG_Node_Ptr_Array cfg_node_ptr_array_from_list(Arena *arena, CFG_Node_Ptr_List *list);

////////////////////////////////
//~ rjf: Schema Data Structure Functions

internal void cfg_schema_table_insert(Arena *arena, CFG_Schema_Table *table, String8 name, MD_Node *schema);
internal MD_Node *cfg_schema_from_name(CFG_Schema_Table *table, String8 name);
internal MD_Node_Ptr_List cfg_schemas_from_name(Arena *arena, CFG_Schema_Table *table, String8 name);

////////////////////////////////
//~ rjf: Config Reading Functions

//- rjf: context selection
internal void cfg_ctx_select(CFG_Ctx *ctx);

//- rjf: tree navigations
internal u64 cfg_change_gen(void);
internal CFG_Node *cfg_node_root(void);
internal CFG_Node *cfg_node_from_id(CFG_ID id);
internal CFG_Node *cfg_node_child_from_string(CFG_Node *parent, String8 string);
internal CFG_Node *cfg_node_child_from_string_or_parent(CFG_Node *parent, String8 string);
internal CFG_Node_Ptr_List cfg_node_child_list_from_string(Arena *arena, CFG_Node *parent, String8 string);
internal CFG_Node_Ptr_List cfg_node_top_level_list_from_string(Arena *arena, String8 string);
internal CFG_Node_Rec cfg_node_rec__depth_first(CFG_Node *root, CFG_Node *node);

//- rjf: serialization
internal String8 cfg_string_from_tree(Arena *arena, CFG_Schema_Table *schema_table, String8 root_path, CFG_Node *root);

////////////////////////////////
//~ rjf: Config Writing Functions

//- rjf: state creation / destroying
internal CFG_State *cfg_state_alloc(void);
internal void cfg_state_release(CFG_State *state);

//- rjf: state -> ctx
internal CFG_Ctx *cfg_state_ctx(CFG_State *state);

//- rjf: string allocations
internal u64 cfg_string_bucket_num_from_size(u64 size);
internal String8 cfg_string_alloc(CFG_State *state, String8 string);
internal void cfg_string_release(CFG_State *state, String8 string);

//- rjf: tree building
internal CFG_Node *cfg_node_alloc                (CFG_State *state);
internal void      cfg_node_release              (CFG_State *state, CFG_Node *node);
internal void      cfg_node_release_all_children (CFG_State *state, CFG_Node *node);
internal CFG_Node *cfg_node_new                  (CFG_State *state, CFG_Node *parent, String8 string);
internal CFG_Node *cfg_node_newf                 (CFG_State *state, CFG_Node *parent, char *fmt, ...);
internal CFG_Node *cfg_node_new_replace          (CFG_State *state, CFG_Node *parent, String8 string);
internal CFG_Node *cfg_node_new_replacef         (CFG_State *state, CFG_Node *parent, char *fmt, ...);
internal CFG_Node *cfg_node_deep_copy            (CFG_State *state, CFG_Node *src_root);
internal void      cfg_node_equip_string         (CFG_State *state, CFG_Node *node, String8 string);
internal void      cfg_node_equip_stringf        (CFG_State *state, CFG_Node *node, char *fmt, ...);
internal void      cfg_node_insert_child         (CFG_State *state, CFG_Node *parent, CFG_Node *prev_child, CFG_Node *new_child);
internal void      cfg_node_unhook               (CFG_State *state, CFG_Node *parent, CFG_Node *child);
internal CFG_Node *cfg_node_child_from_string_or_alloc(CFG_State *state, CFG_Node *parent, String8 string);

//- rjf: deserialization
internal CFG_Node_Ptr_List cfg_node_ptr_list_from_string(Arena *arena, CFG_State *state, CFG_Schema_Table *schema_table, String8 root_path, String8 string);

#endif // CONFIG_CORE_H
