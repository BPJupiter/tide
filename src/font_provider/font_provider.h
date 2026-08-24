// Copyright (c) Epic Games Tools
// Licensed under the MIT license (https://opensource.org/license/mit/)

#ifndef FONT_PROVIDER_H
#define FONT_PROVIDER_H

#define fp_hook C_LINKAGE

////////////////////////////////
//~ rjf: Types

typedef u32 FP_RasterFlags;
enum
{
  FP_RasterFlag_Smooth  = (1<<0),
  FP_RasterFlag_Hinted  = (1<<1),
};

typedef struct FP_Handle FP_Handle;
struct FP_Handle
{
  u64 u64[2];
};

typedef struct FP_Metrics FP_Metrics;
struct FP_Metrics
{
  f32 design_units_per_em;
  f32 ascent;
  f32 descent;
  f32 line_gap;
  f32 capital_height;
};

typedef struct FP_Raster_Result FP_Raster_Result;
struct FP_Raster_Result
{
  Vec2s16 atlas_dim;
  void *atlas;
  f32 advance;
};

////////////////////////////////
//~ rjf: Basic Type Functions

internal FP_Handle fp_handle_zero(void);
internal bool32 fp_handle_match(FP_Handle a, FP_Handle b);

////////////////////////////////
//~ rjf: Backend Hooks

fp_hook void fp_init(void);
fp_hook FP_Handle fp_font_open(String8 path);
fp_hook FP_Handle fp_font_open_from_static_data_string(String8 *data_ptr);
fp_hook void fp_font_close(FP_Handle handle);
fp_hook FP_Metrics fp_metrics_from_font(FP_Handle font);
fp_hook ASAN_NO_ADDR FP_Raster_Result fp_raster(Arena *arena, FP_Handle font, f32 size, FP_RasterFlags flags, String8 string);

#endif // FONT_PROVIDER_H
