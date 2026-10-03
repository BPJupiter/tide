// Copyright (c) Epic Games Tools
// Licensed under the MIT license (https://opensource.org/license/mit/)

//- GENERATED CODE

#ifndef DRAW_META_H
#define DRAW_META_H

typedef struct DR_Tex2DSampleKind_Node DR_Tex2DSampleKind_Node; struct DR_Tex2DSampleKind_Node {DR_Tex2DSampleKind_Node *next; R_Tex2DSampleKind v;};
typedef struct DR_XForm2D_Node DR_XForm2D_Node; struct DR_XForm2D_Node {DR_XForm2D_Node *next; Mat3x3f32 v;};
typedef struct DR_Clip_Node DR_Clip_Node; struct DR_Clip_Node {DR_Clip_Node *next; Rng2f32 v;};
typedef struct DR_Transparency_Node DR_Transparency_Node; struct DR_Transparency_Node {DR_Transparency_Node *next; f32 v;};
#define DR_BucketStackDecls struct{\
DR_Tex2DSampleKind_Node *top_tex2d_sample_kind;\
DR_XForm2D_Node *top_xform2d;\
DR_Clip_Node *top_clip;\
DR_Transparency_Node *top_transparency;\
}
read_only global DR_Tex2DSampleKind_Node dr_nil_tex2d_sample_kind = {0, R_Tex2DSampleKind_Nearest};
read_only global DR_XForm2D_Node dr_nil_xform2d = {0, {1, 0, 0, 0, 1, 0, 0, 0, 1}};
read_only global DR_Clip_Node dr_nil_clip = {0, {0}};
read_only global DR_Transparency_Node dr_nil_transparency = {0, 0};
#define DR_BucketStackInits(b) do{\
(b)->top_tex2d_sample_kind = &dr_nil_tex2d_sample_kind;\
(b)->top_xform2d = &dr_nil_xform2d;\
(b)->top_clip = &dr_nil_clip;\
(b)->top_transparency = &dr_nil_transparency;\
}while(0)
#if 0
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
#endif
#if 0
#define DR_Tex2DSampleKindScope(v)  DeferLoop(dr_push_tex2d_sample_kind(v), dr_pop_tex2d_sample_kind())
#define DR_XForm2DScope(v)          DeferLoop(dr_push_xform2d(v), dr_pop_xform2d())
#define DR_ClipScope(v)             DeferLoop(dr_push_clip(v), dr_pop_clip())
#define DR_TransparencyScope(v)     DeferLoop(dr_push_transparency(v), dr_pop_transparency())
#endif
#endif // DRAW_META_H
