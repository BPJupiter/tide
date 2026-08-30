// Copyright (c) Epic Games Tools
// Licensed under the MIT license (https://opensource.org/license/mit/)

#ifndef RENDER_CORE_H
#define RENDER_CORE_H

#define r_hook C_LINKAGE

////////////////////
// Generated Code

#include "generated/render.meta.h"

///////////
// Enums

typedef u8 R_ChannelCode; // 3 bits
typedef enum R_ChannelCodeEnum {
    R_ChannelCode_Null,
    R_ChannelCode_R,
    R_ChannelCode_G,
    R_ChannelCode_B,
    R_ChannelCode_A
} R_ChannelCodeEnum;

typedef u8 R_ChannelSizeKind; // 3 bits
typedef enum R_ChannelSizeKindEnum {
    R_ChannelSizeKind_Null,
    R_ChannelSizeKind_2,
    R_ChannelSizeKind_8,
    R_ChannelSizeKind_10,
    R_ChannelSizeKind_11,
    R_ChannelSizeKind_16,
    R_ChannelSizeKind_24,
    R_ChannelSizeKind_32,
} R_ChannelSizeKindEnum;

typedef u8 R_ChannelTypeKind; // 3 bits
typedef enum R_ChannelTypeKindEnum {
    R_ChannelTypeKind_Null,
    R_ChannelTypeKind_uint,
    R_ChannelTypeKind_sint,
    R_ChannelTypeKind_unorm,
    R_ChannelTypeKind_snorm,
    R_ChannelTypeKind_float,
} R_ChannelTypeKindEnum;

typedef u64 R_Tex2DFmt;
//
// set of channels, each channel including {code, size, type kind}, 3 bits each:
// [0, 3) -> channel code
// [3, 6) -> channel size
// [6, 9) -> channel type kind
//
// 9 bits per channel, * number of channels, e.g. 4 channels -> 36 bits

#define R_Channel(channel_idx, code_name, size_kind_name, type_kind_name) ((((u64)(R_ChannelCode_##code_name & 0x7)) | ((u64)(R_ChannelSizeKind_##size_kind_name & 0x7) << 3) | ((u64)(R_ChannelTypeKind_##type_kind_name & 0x7) << 6)) << ((channel_idx) * 9))
#define r_code_from_tex2dfmt_channel(fmt, channel_idx) ((R_ChannelCode)(((fmt) & (0x7 << ((channel_idx) * 9))) >> ((channel_idx) * 9)))
#define r_size_kind_from_tex2dfmt_channel(fmt, channel_idx) ((R_ChannelSizeKind)(((fmt) & (0x38 << ((channel_idx) * 9))) >> ((channel_idx) * 9 + 3)))
#define r_type_kind_from_tex2dfmt_channel(fmt, channel_idx) ((R_ChannelTypeKind)(((fmt) & (0x1c0 << ((channel_idx) * 9))) >> ((channel_idx) * 9 + 6)))

#define R_Tex2DFmt_R8    (R_Channel(0, R, 8, uint))
#define R_Tex2DFmt_RG8   (R_Channel(0, R, 8, uint) | R_Channel(1, G, 8, uint))
#define R_Tex2DFmt_RGB8  (R_Channel(0, R, 8, uint) | R_Channel(1, G, 8, uint) | R_Channel(2, B, 8, uint))
#define R_Tex2DFmt_RGBA8 (R_Channel(0, R, 8, uint) | R_Channel(1, G, 8, uint) | R_Channel(2, B, 8, uint) | R_Channel(3, A, 8, uint))

typedef u32 R_GeoVertexFlags;
enum {
    R_GeoVertexFlag_TexCoord = (1 << 0),
    R_GeoVertexFlag_Normals  = (1 << 1),
    R_GeoVertexFlag_RGB      = (1 << 2),
    R_GeoVertexFlag_RGBA     = (1 << 3),
};

/////////////////
// Handle Type

typedef union R_Handle R_Handle;
union R_Handle {
    u64 u64[1];
    u32 u32[2];
    u16 u16[4];
};

////////////////////
// Instance Types

typedef struct R_Rect2D_Inst R_Rect2D_Inst;
struct R_Rect2D_Inst {
    Rng2f32 dst;
    Rng2f32 src;
    Vec4f32 colors[Corner_COUNT];
    f32 corner_radii[Corner_COUNT];
    f32 border_thickness;
    f32 edge_softness;
    f32 white_texture_override;
    f32 shear;
};

typedef struct R_Mesh3D_Inst R_Mesh3D_Inst;
struct R_Mesh3D_Inst {
    Mat4x4f32 xform;
};

/////////////////
// Batch Types

typedef struct R_Batch R_Batch;
struct R_Batch {
    u8 *v;
    u64 byte_count;
    u64 byte_cap;
};

typedef struct R_Batch_Node R_Batch_Node;
struct R_Batch_Node {
    R_Batch_Node *next;
    R_Batch v;
};

typedef struct R_Batch_List R_Batch_List;
struct R_Batch_List {
    R_Batch_Node *first;
    R_Batch_Node *last;
    u64 batch_count;
    u64 byte_count;
    u64 bytes_per_inst;
};

typedef struct R_Batch_Group_2D_Params R_Batch_Group_2D_Params;
struct R_Batch_Group_2D_Params {
    R_Handle tex;
    R_Tex2DSampleKind tex_sample_kind;
    Mat3x3f32 xform;
    Rng2f32 clip;
    f32 transparency;
};

typedef struct R_Batch_Group_2D_Node R_Batch_Group_2D_Node;
struct R_Batch_Group_2D_Node {
    R_Batch_Group_2D_Node *next;
    R_Batch_List batches;
    R_Batch_Group_2D_Params params;
};

typedef struct R_Batch_Group_2D_List R_Batch_Group_2D_List;
struct R_Batch_Group_2D_List {
    R_Batch_Group_2D_Node *first;
    R_Batch_Group_2D_Node *last;
    u64 count;
};

typedef struct R_Batch_Group_3D_Params R_Batch_Group_3D_Params;
struct R_Batch_Group_3D_Params {
    R_Handle mesh_vertices;
    R_Handle mesh_indices;
    R_GeoTopologyKind mesh_geo_topology;
    R_GeoVertexFlags mesh_geo_vertex_flags;
    R_Handle albedo_tex;
    R_Tex2DSampleKind albedo_tex_sample_kind;
    Mat4x4f32 xform;
};

typedef struct R_Batch_Group_3D_Map_Node R_Batch_Group_3D_Map_Node;
struct R_Batch_Group_3D_Map_Node {
    R_Batch_Group_3D_Map_Node *next;
    u64 hash;
    R_Batch_List batches;
    R_Batch_Group_3D_Params params;
};

typedef struct R_Batch_Group_3D_Map R_Batch_Group_3D_Map;
struct R_Batch_Group_3D_Map {
    R_Batch_Group_3D_Map_Node **slots;
    u64 slots_count;
};

////////////////
// Pass Types

typedef struct R_Pass_Params_UI R_Pass_Params_UI;
struct R_Pass_Params_UI {
    R_Batch_Group_2D_List rects;
};

typedef struct R_Pass_Params_Blur R_Pass_Params_Blur;
struct R_Pass_Params_Blur {
    Rng2f32 rect;
    Rng2f32 clip;
    f32 blur_size;
    f32 corner_radii[Corner_COUNT];
};

typedef struct R_Pass_Params_Geo3D R_Pass_Params_Geo3D;
struct R_Pass_Params_Geo3D {
    Rng2f32 viewport;
    Rng2f32 clip;
    Mat4x4f32 view;
    Mat4x4f32 projection;
    R_Batch_Group_3D_Map mesh_batches;
};

typedef struct R_Pass R_Pass;
struct R_Pass {
    R_PassKind kind;
    union
    {
        void *params;
        R_Pass_Params_UI *params_ui;
        R_Pass_Params_Blur *params_blur;
        R_Pass_Params_Geo3D *params_geo3d;
    };
};

typedef struct R_Pass_Node R_Pass_Node;
struct R_Pass_Node {
    R_Pass_Node *next;
    R_Pass v;
};

typedef struct R_Pass_List R_Pass_List;
struct R_Pass_List {
    R_Pass_Node *first;
    R_Pass_Node *last;
    u64 count;
};

/////////////
// Helpers

internal u64 r_bytes_per_pixel_from_tex2dfmt(R_Tex2DFmt fmt);
internal Mat4x4f32 r_sample_channel_map_from_tex2dfmt(R_Tex2DFmt fmt);
internal Mat4x4f32 r_sample_channel_map_from_tex2dformat(R_Tex2DFormat fmt);

///////////////////////////
// Handle Type Functions

internal R_Handle r_handle_zero(void);
internal bool32 r_handle_match(R_Handle a, R_Handle b);

//////////////////////////
// Batch Type Functions

internal R_Batch_List r_batch_list_make(u64 instance_size);
internal void *r_batch_list_push_inst(Arena *arena, R_Batch_List *list, u64 batch_inst_cap);

/////////////////////////
// Pass Type Functions

internal R_Pass *r_pass_from_kind(Arena *arena, R_Pass_List *list, R_PassKind kind);

///////////////////
// Backend Hooks

// top-level layer initialisation
r_hook void           r_init(Cmd_Line *cmdline);

// window setup/teardown
r_hook R_Handle       r_window_equip(WM_Window window);
r_hook void           r_window_unequip(WM_Window window, R_Handle window_equip);

// textures
r_hook R_Handle       r_tex2d_alloc(R_ResourceKind kind, Vec2s32 size, R_Tex2DFormat format, void *data);
r_hook void           r_tex2d_release(R_Handle texture);
r_hook R_ResourceKind r_kind_from_tex2d(R_Handle texture);
r_hook Vec2s32        r_size_from_tex2d(R_Handle texture);
r_hook R_Tex2DFormat  r_format_from_tex2d(R_Handle texture);
r_hook void           r_fill_tex2d_region(R_Handle texture, Rng2s32 subrect, void *data);

// buffers
r_hook R_Handle       r_buffer_alloc(R_ResourceKind kind, u64 size, void *data);
r_hook void           r_buffer_release(R_Handle buffer);

// frame markers
r_hook void           r_begin_frame(void);
r_hook void           r_end_frame(void);
r_hook void           r_window_begin_frame(WM_Window window, R_Handle window_equip);
r_hook void           r_window_end_frame(WM_Window window, R_Handle window_equip);

// render pass submission
r_hook void           r_window_submit(WM_Window window, R_Handle window_equip, R_Pass_List *passes);

#endif // RENDER_CORE_H
