// Copyright (c) Epic Games Tools
// Licensed under the MIT license (https://opensource.org/license/mit/)

#ifndef DRAW_H
#define DRAW_H

////////////////////////////////
//~ rjf: Fancy String Types

typedef struct DR_FStr_Params DR_FStr_Params;
struct DR_FStr_Params
{
  FNT_Tag font;
  FNT_RasterFlags raster_flags;
  Vec4f32 color;
  f32 size;
  f32 underline_thickness;
  f32 strikethrough_thickness;
};

typedef struct DR_FStr DR_FStr;
struct DR_FStr
{
  String8 string;
  DR_FStr_Params params;
};

typedef struct DR_FStr_Node DR_FStr_Node;
struct DR_FStr_Node
{
  DR_FStr_Node *next;
  DR_FStr v;
};

typedef struct DR_FStr_List DR_FStr_List;
struct DR_FStr_List
{
  DR_FStr_Node *first;
  DR_FStr_Node *last;
  u64 node_count;
  u64 total_size;
};

typedef struct DR_FRun DR_FRun;
struct DR_FRun
{
  FNT_Run run;
  Vec4f32 color;
  f32 underline_thickness;
  f32 strikethrough_thickness;
  bool32 icon;
};

typedef struct DR_FRun_Node DR_FRun_Node;
struct DR_FRun_Node
{
  DR_FRun_Node *next;
  DR_FRun v;
};

typedef struct DR_FRun_List DR_FRun_List;
struct DR_FRun_List
{
  DR_FRun_Node *first;
  DR_FRun_Node *last;
  u64 node_count;
  Vec2f32 dim;
};

////////////////////////////////
//~ rjf: Generated Code

#include "generated/draw.meta.h"

////////////////////////////////
//~ rjf: Draw Bucket Types

typedef struct DR_Bucket DR_Bucket;
struct DR_Bucket
{
  R_Pass_List passes;
  u64 stack_gen;
  u64 last_cmd_stack_gen;
  DR_BucketStackDecls;
};

////////////////////////////////
//~ rjf: Thread Context

typedef struct DR_Bucket_Selection_Node DR_Bucket_Selection_Node;
struct DR_Bucket_Selection_Node
{
  DR_Bucket_Selection_Node *next;
  DR_Bucket *bucket;
};

typedef struct DR_Thread_Ctx DR_Thread_Ctx;
struct DR_Thread_Ctx
{
  Arena *arena;
  u64 arena_frame_start_pos;
  FNT_Tag icon_font;
  DR_Bucket_Selection_Node *top_bucket;
  DR_Bucket_Selection_Node *free_bucket_selection;
};

////////////////////////////////
//~ rjf: Globals

thread_static DR_Thread_Ctx *dr_thread_ctx = 0;

////////////////////////////////
//~ rjf: Basic Helpers

internal u64 dr_hash_from_string(String8 string);

////////////////////////////////
//~ rjf: Fancy String Type Functions

internal void dr_fstrs_push_front(Arena *arena, DR_FStr_List *list, DR_FStr *str);
internal void dr_fstrs_push(Arena *arena, DR_FStr_List *list, DR_FStr *str);
internal void dr_fstrs_push_new_(Arena *arena, DR_FStr_List *list, DR_FStr_Params *params, DR_FStr_Params *overrides, String8 string);
#define dr_fstrs_push_new(arena, list, params, string, ...) dr_fstrs_push_new_((arena), (list), (params), &(DR_FStr_Params){.size = 0, __VA_ARGS__}, (string))
internal void dr_fstrs_concat_in_place(DR_FStr_List *dst, DR_FStr_List *to_push);
internal DR_FStr_List dr_fstrs_copy(Arena *arena, DR_FStr_List *src);
internal String8 dr_string_from_fstrs(Arena *arena, DR_FStr_List *list);
internal Fuzzy_Match_Range_List dr_fuzzy_match_find_from_fstrs(Arena *arena, DR_FStr_List *fstrs, String8 needle);
internal DR_FRun_List dr_fruns_from_fstrs(Arena *arena, f32 tab_size_px, DR_FStr_List *strs);
internal Vec2f32 dr_dim_from_fstrs(f32 tab_size_px, DR_FStr_List *fstrs);

////////////////////////////////
//~ rjf: Top-Level API
//
// (Frame boundaries & bucket submission)

internal void dr_begin_frame(FNT_Tag icon_font);
internal void dr_submit_bucket(WM_Window os_window, R_Handle r_window, DR_Bucket *bucket);

////////////////////////////////
//~ rjf: Bucket Construction & Selection API
//
// (Bucket: Handle to sequence of many render passes, constructed by this layer)

internal DR_Bucket *dr_bucket_make(void);
internal void dr_push_bucket(DR_Bucket *bucket);
internal void dr_pop_bucket(void);
internal DR_Bucket *dr_top_bucket(void);
#define DR_BucketScope(b) DeferLoop(dr_push_bucket(b), dr_pop_bucket())

////////////////////////////////
//~ rjf: Bucket Stacks
//
// (Pushing/popping implicit draw parameters)

internal R_Tex2DSampleKind          dr_push_tex2d_sample_kind(R_Tex2DSampleKind v);
internal Mat3x3f32                  dr_push_xform2d(Mat3x3f32 v);
internal Rng2f32                    dr_push_clip(Rng2f32 v);
internal f32                        dr_push_transparency(f32 v);
internal R_Tex2DSampleKind          dr_pop_tex2d_sample_kind(void);
internal Mat3x3f32                  dr_pop_xform2d(void);
internal Rng2f32                    dr_pop_clip(void);
internal f32                        dr_pop_transparency(void);
internal R_Tex2DSampleKind          dr_top_tex2d_sample_kind(void);
internal Mat3x3f32                  dr_top_xform2d(void);
internal Rng2f32                    dr_top_clip(void);
internal f32                        dr_top_transparency(void);

#define DR_Tex2DSampleKindScope(v)   DeferLoop(dr_push_tex2d_sample_kind(v), dr_pop_tex2d_sample_kind())
#define DR_XForm2DScope(v)           DeferLoop(dr_push_xform2d(v), dr_pop_xform2d())
#define DR_ClipScope(v)              DeferLoop(dr_push_clip(v), dr_pop_clip())
#define DR_TransparencyScope(v)      DeferLoop(dr_push_transparency(v), dr_pop_transparency())

////////////////////////////////
//~ rjf: Core Draw Calls
//
// (Apply to the calling thread's currently selected bucket)

//- rjf: rectangles
internal inline R_Rect2D_Inst *dr_rect(Rng2f32 dst, Vec4f32 color, f32 corner_radius, f32 border_thickness, f32 edge_softness);

//- rjf: images
internal inline R_Rect2D_Inst *dr_img(Rng2f32 dst, Rng2f32 src, R_Handle texture, Vec4f32 color, f32 corner_radius, f32 border_thickness, f32 edge_softness);

//- rjf: blurs
internal R_Pass_Params_Blur *dr_blur(Rng2f32 rect, f32 blur_size, f32 corner_radius);

//- rjf: 3d rendering pass params
internal R_Pass_Params_Geo3D *dr_geo3d_begin(Rng2f32 viewport, Mat4x4f32 view, Mat4x4f32 projection);

//- rjf: meshes
internal R_Mesh3D_Inst *dr_mesh(R_Handle mesh_vertices, R_Handle mesh_indices, R_GeoTopologyKind mesh_geo_topology, R_GeoVertexFlags mesh_geo_vertex_flags, R_Handle albedo_tex, Mat4x4f32 inst_xform);

//- rjf: collating one pre-prepped bucket into parent bucket
internal void dr_sub_bucket(DR_Bucket *bucket);

////////////////////////////////
//~ rjf: Draw Call Helpers

//- rjf: text
internal void dr_truncated_fancy_run_list(Vec2f32 p, DR_FRun_List *list, f32 max_x, FNT_Run trailer_run);
internal void dr_truncated_fancy_run_fuzzy_matches(Vec2f32 p, DR_FRun_List *list, f32 max_x, Fuzzy_Match_Range_List *ranges, Vec4f32 color);
internal void dr_text_run(Vec2f32 p, Vec4f32 color, FNT_Run run);
internal void dr_text(FNT_Tag font, f32 size, f32 base_align_px, f32 tab_size_px, FNT_RasterFlags flags, Vec2f32 p, Vec4f32 color, String8 string);

#endif // DRAW_H
