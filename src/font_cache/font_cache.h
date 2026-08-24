// Copyright (c) Epic Games Tools
// Licensed under the MIT license (https://opensource.org/license/mit/)

#ifndef FONT_CACHE_H
#define FONT_CACHE_H

////////////////////////////////
//~ rjf: Rasterization Flags

typedef u32 FNT_RasterFlags;
enum
{
  FNT_RasterFlag_Smooth  = (1<<0),
  FNT_RasterFlag_Hinted  = (1<<1),
};

////////////////////////////////
//~ rjf: Handles & Tags

typedef struct FNT_Tag FNT_Tag;
struct FNT_Tag
{
  u64 u64[2];
};

////////////////////////////////
//~ rjf: Draw Package Types (For Cache Queries)

typedef struct FNT_Piece FNT_Piece;
struct FNT_Piece
{
  R_Handle texture;
  Rng2s16 subrect;
  Vec2s16 offset;
  f32 advance;
  u16 decode_size;
};

typedef struct FNT_Piece_Chunk_Node FNT_Piece_Chunk_Node;
struct FNT_Piece_Chunk_Node
{
  FNT_Piece_Chunk_Node *next;
  FNT_Piece *v;
  u64 count;
  u64 cap;
};

typedef struct FNT_Piece_Chunk_List FNT_Piece_Chunk_List;
struct FNT_Piece_Chunk_List
{
  FNT_Piece_Chunk_Node *first;
  FNT_Piece_Chunk_Node *last;
  u64 node_count;
  u64 total_piece_count;
};

typedef struct FNT_Piece_Array FNT_Piece_Array;
struct FNT_Piece_Array
{
  FNT_Piece *v;
  u64 count;
};

typedef struct FNT_Run FNT_Run;
struct FNT_Run
{
  FNT_Piece_Array pieces;
  Vec2f32 dim;
  f32 ascent;
  f32 descent;
};

////////////////////////////////
//~ rjf: Font Path -> Handle * Metrics * Path Cache Types

typedef struct FNT_Font_Hash_Node FNT_Font_Hash_Node;
struct FNT_Font_Hash_Node
{
  FNT_Font_Hash_Node *hash_next;
  FNT_Tag tag;
  FP_Handle handle;
  FP_Metrics metrics;
  String8 path;
};

typedef struct FNT_Font_Hash_Slot FNT_Font_Hash_Slot;
struct FNT_Font_Hash_Slot
{
  FNT_Font_Hash_Node *first;
  FNT_Font_Hash_Node *last;
};

////////////////////////////////
//~ rjf: Rasterization Cache Types

//- rjf: base glyph rasterization / dimensions cache 

typedef struct FNT_Raster_Cache_Info FNT_Raster_Cache_Info;
struct FNT_Raster_Cache_Info
{
  Rng2s16 subrect;
  Vec2s16 raster_dim;
  s16 atlas_num;
  f32 advance;
};

typedef struct FNT_Hash_2_Info_Raster_Cache_Node FNT_Hash_2_Info_Raster_Cache_Node;
struct FNT_Hash_2_Info_Raster_Cache_Node
{
  FNT_Hash_2_Info_Raster_Cache_Node *hash_next;
  FNT_Hash_2_Info_Raster_Cache_Node *hash_prev;
  u64 hash;
  FNT_Raster_Cache_Info info;
};

typedef struct FNT_Hash_2_Info_Raster_Cache_Slot FNT_Hash_2_Info_Raster_Cache_Slot;
struct FNT_Hash_2_Info_Raster_Cache_Slot
{
  FNT_Hash_2_Info_Raster_Cache_Node *first;
  FNT_Hash_2_Info_Raster_Cache_Node *last;
};

//- rjf: run cache (arrangements of many glyphs to represent a full string)

typedef struct FNT_Run_Cache_Node FNT_Run_Cache_Node;
struct FNT_Run_Cache_Node
{
  FNT_Run_Cache_Node *next;
  String8 string;
  FNT_Run run;
};

typedef struct FNT_Run_Cache_Slot FNT_Run_Cache_Slot;
struct FNT_Run_Cache_Slot
{
  FNT_Run_Cache_Node *first;
  FNT_Run_Cache_Node *last;
};

//- rjf: style hash -> artifacts/metrics cache

typedef struct FNT_Hash_2_Style_Raster_Cache_Node FNT_Hash_2_Style_Raster_Cache_Node;
struct FNT_Hash_2_Style_Raster_Cache_Node
{
  FNT_Hash_2_Style_Raster_Cache_Node *hash_next;
  FNT_Hash_2_Style_Raster_Cache_Node *hash_prev;
  u64 style_hash;
  f32 ascent;
  f32 descent;
  f32 column_width;
  FNT_Raster_Cache_Info *utf8_class1_direct_map;
  u64 utf8_class1_direct_map_mask[4];
  u64 hash2info_slots_count;
  FNT_Hash_2_Info_Raster_Cache_Slot *hash2info_slots;
  u64 run_slots_count;
  FNT_Run_Cache_Slot *run_slots;
  u64 run_slots_frame_index;
};

typedef struct FNT_Hash_2_Style_Raster_Cache_Slot FNT_Hash_2_Style_Raster_Cache_Slot;
struct FNT_Hash_2_Style_Raster_Cache_Slot
{
  FNT_Hash_2_Style_Raster_Cache_Node *first;
  FNT_Hash_2_Style_Raster_Cache_Node *last;
};

////////////////////////////////
//~ rjf: Atlas Types

typedef u32 FNT_AtlasRegionNodeFlags;
enum
{
  FNT_AtlasRegionNodeFlag_Taken = (1<<0),
};

typedef struct FNT_Atlas_Region_Node FNT_Atlas_Region_Node;
struct FNT_Atlas_Region_Node
{
  FNT_Atlas_Region_Node *parent;
  FNT_Atlas_Region_Node *children[Corner_COUNT];
  Vec2s16 max_free_size[Corner_COUNT];
  FNT_AtlasRegionNodeFlags flags;
  u64 num_allocated_descendants;
};

typedef struct FNT_Atlas FNT_Atlas;
struct FNT_Atlas
{
  FNT_Atlas *next;
  FNT_Atlas *prev;
  R_Handle texture;
  Vec2s16 root_dim;
  FNT_Atlas_Region_Node *root;
};

////////////////////////////////
//~ rjf: Metrics

typedef struct FNT_Metrics FNT_Metrics;
struct FNT_Metrics
{
  f32 ascent;
  f32 descent;
  f32 line_gap;
  f32 capital_height;
};

////////////////////////////////
//~ rjf: Main State Type

typedef struct FNT_State FNT_State;
struct FNT_State
{
  Arena *permanent_arena;
  Arena *raster_arena;
  Arena *frame_arena;
  u64 frame_index;
  
  // rjf: font table
  u64 font_hash_table_size;
  FNT_Font_Hash_Slot *font_hash_table;
  
  // rjf: hash -> raster cache table
  u64 hash2style_slots_count;
  FNT_Hash_2_Style_Raster_Cache_Slot *hash2style_slots;
  
  // rjf: atlas list
  FNT_Atlas *first_atlas;
  FNT_Atlas *last_atlas;
};

////////////////////////////////
//~ rjf: Globals

global FNT_State *fnt_state = 0;

////////////////////////////////
//~ rjf: Basic Functions

internal u128 fnt_hash_from_string(String8 string);
internal u64 fnt_little_hash_from_string(u64 seed, String8 string);
internal Vec2s32 fnt_vertex_from_corner(Corner corner);

////////////////////////////////
//~ rjf: Font Tags

internal FNT_Tag fnt_tag_zero(void);
internal bool32 fnt_tag_match(FNT_Tag a, FNT_Tag b);
internal FP_Handle fnt_handle_from_tag(FNT_Tag tag);
internal FP_Metrics fnt_fp_metrics_from_tag(FNT_Tag tag);
internal FNT_Tag fnt_tag_from_path(String8 path);
internal FNT_Tag fnt_tag_from_static_data_string(String8 *data_ptr);
internal String8 fnt_path_from_tag(FNT_Tag tag);

////////////////////////////////
//~ rjf: Atlas

internal Rng2s16 fnt_atlas_region_alloc(Arena *arena, FNT_Atlas *atlas, Vec2s16 needed_size);
internal void fnt_atlas_region_release(FNT_Atlas *atlas, Rng2s16 region);

////////////////////////////////
//~ rjf: Piece Type Functions

internal FNT_Piece *fnt_piece_chunk_list_push_new(Arena *arena, FNT_Piece_Chunk_List *list, u64 cap);
internal void fnt_piece_chunk_list_push(Arena *arena, FNT_Piece_Chunk_List *list, u64 cap, FNT_Piece *piece);
internal FNT_Piece_Array fnt_piece_array_from_chunk_list(Arena *arena, FNT_Piece_Chunk_List *list);
internal FNT_Piece_Array fnt_piece_array_copy(Arena *arena, FNT_Piece_Array *src);

////////////////////////////////
//~ rjf: Cache Usage

//- rjf: base cache lookups
internal FNT_Hash_2_Style_Raster_Cache_Node *fnt_hash2style_from_tag_size_flags(FNT_Tag tag, f32 size, FNT_RasterFlags flags);
internal FNT_Run fnt_run_from_string(FNT_Tag tag, f32 size, f32 base_align_px, f32 tab_size_px, FNT_RasterFlags flags, String8 string);

//- rjf: helpers
internal String8_List fnt_wrapped_string_lines_from_font_size_string_max(Arena *arena, FNT_Tag font, f32 size, f32 base_align_px, f32 tab_size_px, String8 string, f32 max);
internal Vec2f32 fnt_dim_from_tag_size_string(FNT_Tag tag, f32 size, f32 base_align_px, f32 tab_size_px, String8 string);
internal Vec2f32 fnt_dim_from_tag_size_string_list(FNT_Tag tag, f32 size, f32 base_align_px, f32 tab_size_px, String8_List list);
internal f32 fnt_column_size_from_tag_size(FNT_Tag tag, f32 size);
internal u64 fnt_char_pos_from_tag_size_string_p(FNT_Tag tag, f32 size, f32 base_align_px, f32 tab_size_px, String8 string, f32 p);

////////////////////////////////
//~ rjf: Metrics

internal FNT_Metrics fnt_metrics_from_tag_size(FNT_Tag tag, f32 size);
internal f32 fnt_line_height_from_metrics(FNT_Metrics *metrics);

////////////////////////////////
//~ rjf: Main Calls

internal void fnt_init(void);
internal void fnt_reset(void);
internal void fnt_frame(void);

#endif // FONT_CACHE_H
