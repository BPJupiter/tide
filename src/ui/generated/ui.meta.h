// Copyright (c) Epic Games Tools
// Licensed under the MIT license (https://opensource.org/license/mit/)

//- GENERATED CODE

#ifndef UI_META_H
#define UI_META_H

typedef struct UI_Parent_Node UI_Parent_Node; struct UI_Parent_Node{UI_Parent_Node *next; UI_Box * v;};
typedef struct UI_ChildLayoutAxis_Node UI_ChildLayoutAxis_Node; struct UI_ChildLayoutAxis_Node{UI_ChildLayoutAxis_Node *next; Axis2 v;};
typedef struct UI_FixedX_Node UI_FixedX_Node; struct UI_FixedX_Node{UI_FixedX_Node *next; f32 v;};
typedef struct UI_FixedY_Node UI_FixedY_Node; struct UI_FixedY_Node{UI_FixedY_Node *next; f32 v;};
typedef struct UI_FixedWidth_Node UI_FixedWidth_Node; struct UI_FixedWidth_Node{UI_FixedWidth_Node *next; f32 v;};
typedef struct UI_FixedHeight_Node UI_FixedHeight_Node; struct UI_FixedHeight_Node{UI_FixedHeight_Node *next; f32 v;};
typedef struct UI_PrefWidth_Node UI_PrefWidth_Node; struct UI_PrefWidth_Node{UI_PrefWidth_Node *next; UI_Size v;};
typedef struct UI_PrefHeight_Node UI_PrefHeight_Node; struct UI_PrefHeight_Node{UI_PrefHeight_Node *next; UI_Size v;};
typedef struct UI_MinWidth_Node UI_MinWidth_Node; struct UI_MinWidth_Node{UI_MinWidth_Node *next; f32 v;};
typedef struct UI_MinHeight_Node UI_MinHeight_Node; struct UI_MinHeight_Node{UI_MinHeight_Node *next; f32 v;};
typedef struct UI_PermissionFlags_Node UI_PermissionFlags_Node; struct UI_PermissionFlags_Node{UI_PermissionFlags_Node *next; UI_PermissionFlags v;};
typedef struct UI_Flags_Node UI_Flags_Node; struct UI_Flags_Node{UI_Flags_Node *next; UI_BoxFlags v;};
typedef struct UI_OmitFlags_Node UI_OmitFlags_Node; struct UI_OmitFlags_Node{UI_OmitFlags_Node *next; UI_BoxFlags v;};
typedef struct UI_FocusHot_Node UI_FocusHot_Node; struct UI_FocusHot_Node{UI_FocusHot_Node *next; UI_FocusKind v;};
typedef struct UI_FocusActive_Node UI_FocusActive_Node; struct UI_FocusActive_Node{UI_FocusActive_Node *next; UI_FocusKind v;};
typedef struct UI_FastpathCodepoint_Node UI_FastpathCodepoint_Node; struct UI_FastpathCodepoint_Node{UI_FastpathCodepoint_Node *next; u32 v;};
typedef struct UI_GroupKey_Node UI_GroupKey_Node; struct UI_GroupKey_Node{UI_GroupKey_Node *next; UI_Key v;};
typedef struct UI_Transparency_Node UI_Transparency_Node; struct UI_Transparency_Node{UI_Transparency_Node *next; f32 v;};
typedef struct UI_Tag_Node UI_Tag_Node; struct UI_Tag_Node{UI_Tag_Node *next; String8 v;};
typedef struct UI_BackgroundColor_Node UI_BackgroundColor_Node; struct UI_BackgroundColor_Node{UI_BackgroundColor_Node *next; Vec4f32 v;};
typedef struct UI_TextColor_Node UI_TextColor_Node; struct UI_TextColor_Node{UI_TextColor_Node *next; Vec4f32 v;};
typedef struct UI_BorderColor_Node UI_BorderColor_Node; struct UI_BorderColor_Node{UI_BorderColor_Node *next; Vec4f32 v;};
typedef struct UI_Squish_Node UI_Squish_Node; struct UI_Squish_Node{UI_Squish_Node *next; f32 v;};
typedef struct UI_HoverCursor_Node UI_HoverCursor_Node; struct UI_HoverCursor_Node{UI_HoverCursor_Node *next; WM_Cursor v;};
typedef struct UI_Font_Node UI_Font_Node; struct UI_Font_Node{UI_Font_Node *next; FNT_Tag v;};
typedef struct UI_FontSize_Node UI_FontSize_Node; struct UI_FontSize_Node{UI_FontSize_Node *next; f32 v;};
typedef struct UI_TextRasterFlags_Node UI_TextRasterFlags_Node; struct UI_TextRasterFlags_Node{UI_TextRasterFlags_Node *next; FNT_RasterFlags v;};
typedef struct UI_TabSize_Node UI_TabSize_Node; struct UI_TabSize_Node{UI_TabSize_Node *next; f32 v;};
typedef struct UI_CornerRadius00_Node UI_CornerRadius00_Node; struct UI_CornerRadius00_Node{UI_CornerRadius00_Node *next; f32 v;};
typedef struct UI_CornerRadius01_Node UI_CornerRadius01_Node; struct UI_CornerRadius01_Node{UI_CornerRadius01_Node *next; f32 v;};
typedef struct UI_CornerRadius10_Node UI_CornerRadius10_Node; struct UI_CornerRadius10_Node{UI_CornerRadius10_Node *next; f32 v;};
typedef struct UI_CornerRadius11_Node UI_CornerRadius11_Node; struct UI_CornerRadius11_Node{UI_CornerRadius11_Node *next; f32 v;};
typedef struct UI_VisualMarginX_Node UI_VisualMarginX_Node; struct UI_VisualMarginX_Node{UI_VisualMarginX_Node *next; f32 v;};
typedef struct UI_VisualMarginY_Node UI_VisualMarginY_Node; struct UI_VisualMarginY_Node{UI_VisualMarginY_Node *next; f32 v;};
typedef struct UI_BlurSize_Node UI_BlurSize_Node; struct UI_BlurSize_Node{UI_BlurSize_Node *next; f32 v;};
typedef struct UI_TextPadding_Node UI_TextPadding_Node; struct UI_TextPadding_Node{UI_TextPadding_Node *next; f32 v;};
typedef struct UI_TextAlignment_Node UI_TextAlignment_Node; struct UI_TextAlignment_Node{UI_TextAlignment_Node *next; UI_TextAlign v;};
#define UI_DeclStackNils \
struct\
{\
UI_Parent_Node parent_nil_stack_top;\
UI_ChildLayoutAxis_Node child_layout_axis_nil_stack_top;\
UI_FixedX_Node fixed_x_nil_stack_top;\
UI_FixedY_Node fixed_y_nil_stack_top;\
UI_FixedWidth_Node fixed_width_nil_stack_top;\
UI_FixedHeight_Node fixed_height_nil_stack_top;\
UI_PrefWidth_Node pref_width_nil_stack_top;\
UI_PrefHeight_Node pref_height_nil_stack_top;\
UI_MinWidth_Node min_width_nil_stack_top;\
UI_MinHeight_Node min_height_nil_stack_top;\
UI_PermissionFlags_Node permission_flags_nil_stack_top;\
UI_Flags_Node flags_nil_stack_top;\
UI_OmitFlags_Node omit_flags_nil_stack_top;\
UI_FocusHot_Node focus_hot_nil_stack_top;\
UI_FocusActive_Node focus_active_nil_stack_top;\
UI_FastpathCodepoint_Node fastpath_codepoint_nil_stack_top;\
UI_GroupKey_Node group_key_nil_stack_top;\
UI_Transparency_Node transparency_nil_stack_top;\
UI_Tag_Node tag_nil_stack_top;\
UI_BackgroundColor_Node background_color_nil_stack_top;\
UI_TextColor_Node text_color_nil_stack_top;\
UI_BorderColor_Node border_color_nil_stack_top;\
UI_Squish_Node squish_nil_stack_top;\
UI_HoverCursor_Node hover_cursor_nil_stack_top;\
UI_Font_Node font_nil_stack_top;\
UI_FontSize_Node font_size_nil_stack_top;\
UI_TextRasterFlags_Node text_raster_flags_nil_stack_top;\
UI_TabSize_Node tab_size_nil_stack_top;\
UI_CornerRadius00_Node corner_radius_00_nil_stack_top;\
UI_CornerRadius01_Node corner_radius_01_nil_stack_top;\
UI_CornerRadius10_Node corner_radius_10_nil_stack_top;\
UI_CornerRadius11_Node corner_radius_11_nil_stack_top;\
UI_VisualMarginX_Node visual_margin_x_nil_stack_top;\
UI_VisualMarginY_Node visual_margin_y_nil_stack_top;\
UI_BlurSize_Node blur_size_nil_stack_top;\
UI_TextPadding_Node text_padding_nil_stack_top;\
UI_TextAlignment_Node text_alignment_nil_stack_top;\
}
#define UI_InitStackNils(state) \
state->parent_nil_stack_top.v = &ui_nil_box;\
state->child_layout_axis_nil_stack_top.v = Axis2_X;\
state->fixed_x_nil_stack_top.v = 0;\
state->fixed_y_nil_stack_top.v = 0;\
state->fixed_width_nil_stack_top.v = 0;\
state->fixed_height_nil_stack_top.v = 0;\
state->pref_width_nil_stack_top.v = ui_px(250.f, 1.f);\
state->pref_height_nil_stack_top.v = ui_px(30.f, 1.f);\
state->min_width_nil_stack_top.v = 0;\
state->min_height_nil_stack_top.v = 0;\
state->permission_flags_nil_stack_top.v = UI_PermissionFlag_All;\
state->flags_nil_stack_top.v = 0;\
state->omit_flags_nil_stack_top.v = 0;\
state->focus_hot_nil_stack_top.v = UI_FocusKind_Null;\
state->focus_active_nil_stack_top.v = UI_FocusKind_Null;\
state->fastpath_codepoint_nil_stack_top.v = 0;\
state->group_key_nil_stack_top.v = ui_key_zero();\
state->transparency_nil_stack_top.v = 0;\
state->tag_nil_stack_top.v = str8_lit("");\
state->background_color_nil_stack_top.v = v4f32(0, 0, 0, 0);\
state->text_color_nil_stack_top.v = v4f32(0, 0, 0, 0);\
state->border_color_nil_stack_top.v = v4f32(0, 0, 0, 0);\
state->squish_nil_stack_top.v = 0;\
state->hover_cursor_nil_stack_top.v = WM_Cursor_Pointer;\
state->font_nil_stack_top.v = fnt_tag_zero();\
state->font_size_nil_stack_top.v = 24.f;\
state->text_raster_flags_nil_stack_top.v = FNT_RasterFlag_Hinted;\
state->tab_size_nil_stack_top.v = 24.f*4.f;\
state->corner_radius_00_nil_stack_top.v = 0;\
state->corner_radius_01_nil_stack_top.v = 0;\
state->corner_radius_10_nil_stack_top.v = 0;\
state->corner_radius_11_nil_stack_top.v = 0;\
state->visual_margin_x_nil_stack_top.v = 0;\
state->visual_margin_y_nil_stack_top.v = 0;\
state->blur_size_nil_stack_top.v = 0;\
state->text_padding_nil_stack_top.v = 0;\
state->text_alignment_nil_stack_top.v = UI_TextAlign_Left;\

#define UI_DeclStacks \
struct\
{\
struct { UI_Parent_Node *top; UI_Box * bottom_val; UI_Parent_Node *free; u64 gen; bool32 auto_pop; } parent_stack;\
struct { UI_ChildLayoutAxis_Node *top; Axis2 bottom_val; UI_ChildLayoutAxis_Node *free; u64 gen; bool32 auto_pop; } child_layout_axis_stack;\
struct { UI_FixedX_Node *top; f32 bottom_val; UI_FixedX_Node *free; u64 gen; bool32 auto_pop; } fixed_x_stack;\
struct { UI_FixedY_Node *top; f32 bottom_val; UI_FixedY_Node *free; u64 gen; bool32 auto_pop; } fixed_y_stack;\
struct { UI_FixedWidth_Node *top; f32 bottom_val; UI_FixedWidth_Node *free; u64 gen; bool32 auto_pop; } fixed_width_stack;\
struct { UI_FixedHeight_Node *top; f32 bottom_val; UI_FixedHeight_Node *free; u64 gen; bool32 auto_pop; } fixed_height_stack;\
struct { UI_PrefWidth_Node *top; UI_Size bottom_val; UI_PrefWidth_Node *free; u64 gen; bool32 auto_pop; } pref_width_stack;\
struct { UI_PrefHeight_Node *top; UI_Size bottom_val; UI_PrefHeight_Node *free; u64 gen; bool32 auto_pop; } pref_height_stack;\
struct { UI_MinWidth_Node *top; f32 bottom_val; UI_MinWidth_Node *free; u64 gen; bool32 auto_pop; } min_width_stack;\
struct { UI_MinHeight_Node *top; f32 bottom_val; UI_MinHeight_Node *free; u64 gen; bool32 auto_pop; } min_height_stack;\
struct { UI_PermissionFlags_Node *top; UI_PermissionFlags bottom_val; UI_PermissionFlags_Node *free; u64 gen; bool32 auto_pop; } permission_flags_stack;\
struct { UI_Flags_Node *top; UI_BoxFlags bottom_val; UI_Flags_Node *free; u64 gen; bool32 auto_pop; } flags_stack;\
struct { UI_OmitFlags_Node *top; UI_BoxFlags bottom_val; UI_OmitFlags_Node *free; u64 gen; bool32 auto_pop; } omit_flags_stack;\
struct { UI_FocusHot_Node *top; UI_FocusKind bottom_val; UI_FocusHot_Node *free; u64 gen; bool32 auto_pop; } focus_hot_stack;\
struct { UI_FocusActive_Node *top; UI_FocusKind bottom_val; UI_FocusActive_Node *free; u64 gen; bool32 auto_pop; } focus_active_stack;\
struct { UI_FastpathCodepoint_Node *top; u32 bottom_val; UI_FastpathCodepoint_Node *free; u64 gen; bool32 auto_pop; } fastpath_codepoint_stack;\
struct { UI_GroupKey_Node *top; UI_Key bottom_val; UI_GroupKey_Node *free; u64 gen; bool32 auto_pop; } group_key_stack;\
struct { UI_Transparency_Node *top; f32 bottom_val; UI_Transparency_Node *free; u64 gen; bool32 auto_pop; } transparency_stack;\
struct { UI_Tag_Node *top; String8 bottom_val; UI_Tag_Node *free; u64 gen; bool32 auto_pop; } tag_stack;\
struct { UI_BackgroundColor_Node *top; Vec4f32 bottom_val; UI_BackgroundColor_Node *free; u64 gen; bool32 auto_pop; } background_color_stack;\
struct { UI_TextColor_Node *top; Vec4f32 bottom_val; UI_TextColor_Node *free; u64 gen; bool32 auto_pop; } text_color_stack;\
struct { UI_BorderColor_Node *top; Vec4f32 bottom_val; UI_BorderColor_Node *free; u64 gen; bool32 auto_pop; } border_color_stack;\
struct { UI_Squish_Node *top; f32 bottom_val; UI_Squish_Node *free; u64 gen; bool32 auto_pop; } squish_stack;\
struct { UI_HoverCursor_Node *top; WM_Cursor bottom_val; UI_HoverCursor_Node *free; u64 gen; bool32 auto_pop; } hover_cursor_stack;\
struct { UI_Font_Node *top; FNT_Tag bottom_val; UI_Font_Node *free; u64 gen; bool32 auto_pop; } font_stack;\
struct { UI_FontSize_Node *top; f32 bottom_val; UI_FontSize_Node *free; u64 gen; bool32 auto_pop; } font_size_stack;\
struct { UI_TextRasterFlags_Node *top; FNT_RasterFlags bottom_val; UI_TextRasterFlags_Node *free; u64 gen; bool32 auto_pop; } text_raster_flags_stack;\
struct { UI_TabSize_Node *top; f32 bottom_val; UI_TabSize_Node *free; u64 gen; bool32 auto_pop; } tab_size_stack;\
struct { UI_CornerRadius00_Node *top; f32 bottom_val; UI_CornerRadius00_Node *free; u64 gen; bool32 auto_pop; } corner_radius_00_stack;\
struct { UI_CornerRadius01_Node *top; f32 bottom_val; UI_CornerRadius01_Node *free; u64 gen; bool32 auto_pop; } corner_radius_01_stack;\
struct { UI_CornerRadius10_Node *top; f32 bottom_val; UI_CornerRadius10_Node *free; u64 gen; bool32 auto_pop; } corner_radius_10_stack;\
struct { UI_CornerRadius11_Node *top; f32 bottom_val; UI_CornerRadius11_Node *free; u64 gen; bool32 auto_pop; } corner_radius_11_stack;\
struct { UI_VisualMarginX_Node *top; f32 bottom_val; UI_VisualMarginX_Node *free; u64 gen; bool32 auto_pop; } visual_margin_x_stack;\
struct { UI_VisualMarginY_Node *top; f32 bottom_val; UI_VisualMarginY_Node *free; u64 gen; bool32 auto_pop; } visual_margin_y_stack;\
struct { UI_BlurSize_Node *top; f32 bottom_val; UI_BlurSize_Node *free; u64 gen; bool32 auto_pop; } blur_size_stack;\
struct { UI_TextPadding_Node *top; f32 bottom_val; UI_TextPadding_Node *free; u64 gen; bool32 auto_pop; } text_padding_stack;\
struct { UI_TextAlignment_Node *top; UI_TextAlign bottom_val; UI_TextAlignment_Node *free; u64 gen; bool32 auto_pop; } text_alignment_stack;\
}
#define UI_InitStacks(state) \
state->parent_stack.top = &state->parent_nil_stack_top; state->parent_stack.bottom_val = &ui_nil_box; state->parent_stack.free = 0; state->parent_stack.auto_pop = 0;\
state->child_layout_axis_stack.top = &state->child_layout_axis_nil_stack_top; state->child_layout_axis_stack.bottom_val = Axis2_X; state->child_layout_axis_stack.free = 0; state->child_layout_axis_stack.auto_pop = 0;\
state->fixed_x_stack.top = &state->fixed_x_nil_stack_top; state->fixed_x_stack.bottom_val = 0; state->fixed_x_stack.free = 0; state->fixed_x_stack.auto_pop = 0;\
state->fixed_y_stack.top = &state->fixed_y_nil_stack_top; state->fixed_y_stack.bottom_val = 0; state->fixed_y_stack.free = 0; state->fixed_y_stack.auto_pop = 0;\
state->fixed_width_stack.top = &state->fixed_width_nil_stack_top; state->fixed_width_stack.bottom_val = 0; state->fixed_width_stack.free = 0; state->fixed_width_stack.auto_pop = 0;\
state->fixed_height_stack.top = &state->fixed_height_nil_stack_top; state->fixed_height_stack.bottom_val = 0; state->fixed_height_stack.free = 0; state->fixed_height_stack.auto_pop = 0;\
state->pref_width_stack.top = &state->pref_width_nil_stack_top; state->pref_width_stack.bottom_val = ui_px(250.f, 1.f); state->pref_width_stack.free = 0; state->pref_width_stack.auto_pop = 0;\
state->pref_height_stack.top = &state->pref_height_nil_stack_top; state->pref_height_stack.bottom_val = ui_px(30.f, 1.f); state->pref_height_stack.free = 0; state->pref_height_stack.auto_pop = 0;\
state->min_width_stack.top = &state->min_width_nil_stack_top; state->min_width_stack.bottom_val = 0; state->min_width_stack.free = 0; state->min_width_stack.auto_pop = 0;\
state->min_height_stack.top = &state->min_height_nil_stack_top; state->min_height_stack.bottom_val = 0; state->min_height_stack.free = 0; state->min_height_stack.auto_pop = 0;\
state->permission_flags_stack.top = &state->permission_flags_nil_stack_top; state->permission_flags_stack.bottom_val = UI_PermissionFlag_All; state->permission_flags_stack.free = 0; state->permission_flags_stack.auto_pop = 0;\
state->flags_stack.top = &state->flags_nil_stack_top; state->flags_stack.bottom_val = 0; state->flags_stack.free = 0; state->flags_stack.auto_pop = 0;\
state->omit_flags_stack.top = &state->omit_flags_nil_stack_top; state->omit_flags_stack.bottom_val = 0; state->omit_flags_stack.free = 0; state->omit_flags_stack.auto_pop = 0;\
state->focus_hot_stack.top = &state->focus_hot_nil_stack_top; state->focus_hot_stack.bottom_val = UI_FocusKind_Null; state->focus_hot_stack.free = 0; state->focus_hot_stack.auto_pop = 0;\
state->focus_active_stack.top = &state->focus_active_nil_stack_top; state->focus_active_stack.bottom_val = UI_FocusKind_Null; state->focus_active_stack.free = 0; state->focus_active_stack.auto_pop = 0;\
state->fastpath_codepoint_stack.top = &state->fastpath_codepoint_nil_stack_top; state->fastpath_codepoint_stack.bottom_val = 0; state->fastpath_codepoint_stack.free = 0; state->fastpath_codepoint_stack.auto_pop = 0;\
state->group_key_stack.top = &state->group_key_nil_stack_top; state->group_key_stack.bottom_val = ui_key_zero(); state->group_key_stack.free = 0; state->group_key_stack.auto_pop = 0;\
state->transparency_stack.top = &state->transparency_nil_stack_top; state->transparency_stack.bottom_val = 0; state->transparency_stack.free = 0; state->transparency_stack.auto_pop = 0;\
state->tag_stack.top = &state->tag_nil_stack_top; state->tag_stack.bottom_val = str8_lit(""); state->tag_stack.free = 0; state->tag_stack.auto_pop = 0;\
state->background_color_stack.top = &state->background_color_nil_stack_top; state->background_color_stack.bottom_val = v4f32(0, 0, 0, 0); state->background_color_stack.free = 0; state->background_color_stack.auto_pop = 0;\
state->text_color_stack.top = &state->text_color_nil_stack_top; state->text_color_stack.bottom_val = v4f32(0, 0, 0, 0); state->text_color_stack.free = 0; state->text_color_stack.auto_pop = 0;\
state->border_color_stack.top = &state->border_color_nil_stack_top; state->border_color_stack.bottom_val = v4f32(0, 0, 0, 0); state->border_color_stack.free = 0; state->border_color_stack.auto_pop = 0;\
state->squish_stack.top = &state->squish_nil_stack_top; state->squish_stack.bottom_val = 0; state->squish_stack.free = 0; state->squish_stack.auto_pop = 0;\
state->hover_cursor_stack.top = &state->hover_cursor_nil_stack_top; state->hover_cursor_stack.bottom_val = WM_Cursor_Pointer; state->hover_cursor_stack.free = 0; state->hover_cursor_stack.auto_pop = 0;\
state->font_stack.top = &state->font_nil_stack_top; state->font_stack.bottom_val = fnt_tag_zero(); state->font_stack.free = 0; state->font_stack.auto_pop = 0;\
state->font_size_stack.top = &state->font_size_nil_stack_top; state->font_size_stack.bottom_val = 24.f; state->font_size_stack.free = 0; state->font_size_stack.auto_pop = 0;\
state->text_raster_flags_stack.top = &state->text_raster_flags_nil_stack_top; state->text_raster_flags_stack.bottom_val = FNT_RasterFlag_Hinted; state->text_raster_flags_stack.free = 0; state->text_raster_flags_stack.auto_pop = 0;\
state->tab_size_stack.top = &state->tab_size_nil_stack_top; state->tab_size_stack.bottom_val = 24.f*4.f; state->tab_size_stack.free = 0; state->tab_size_stack.auto_pop = 0;\
state->corner_radius_00_stack.top = &state->corner_radius_00_nil_stack_top; state->corner_radius_00_stack.bottom_val = 0; state->corner_radius_00_stack.free = 0; state->corner_radius_00_stack.auto_pop = 0;\
state->corner_radius_01_stack.top = &state->corner_radius_01_nil_stack_top; state->corner_radius_01_stack.bottom_val = 0; state->corner_radius_01_stack.free = 0; state->corner_radius_01_stack.auto_pop = 0;\
state->corner_radius_10_stack.top = &state->corner_radius_10_nil_stack_top; state->corner_radius_10_stack.bottom_val = 0; state->corner_radius_10_stack.free = 0; state->corner_radius_10_stack.auto_pop = 0;\
state->corner_radius_11_stack.top = &state->corner_radius_11_nil_stack_top; state->corner_radius_11_stack.bottom_val = 0; state->corner_radius_11_stack.free = 0; state->corner_radius_11_stack.auto_pop = 0;\
state->visual_margin_x_stack.top = &state->visual_margin_x_nil_stack_top; state->visual_margin_x_stack.bottom_val = 0; state->visual_margin_x_stack.free = 0; state->visual_margin_x_stack.auto_pop = 0;\
state->visual_margin_y_stack.top = &state->visual_margin_y_nil_stack_top; state->visual_margin_y_stack.bottom_val = 0; state->visual_margin_y_stack.free = 0; state->visual_margin_y_stack.auto_pop = 0;\
state->blur_size_stack.top = &state->blur_size_nil_stack_top; state->blur_size_stack.bottom_val = 0; state->blur_size_stack.free = 0; state->blur_size_stack.auto_pop = 0;\
state->text_padding_stack.top = &state->text_padding_nil_stack_top; state->text_padding_stack.bottom_val = 0; state->text_padding_stack.free = 0; state->text_padding_stack.auto_pop = 0;\
state->text_alignment_stack.top = &state->text_alignment_nil_stack_top; state->text_alignment_stack.bottom_val = UI_TextAlign_Left; state->text_alignment_stack.free = 0; state->text_alignment_stack.auto_pop = 0;\

#define UI_AutoPopStacks(state) \
if(state->parent_stack.auto_pop) { ui_pop_parent(); state->parent_stack.auto_pop = 0; }\
if(state->child_layout_axis_stack.auto_pop) { ui_pop_child_layout_axis(); state->child_layout_axis_stack.auto_pop = 0; }\
if(state->fixed_x_stack.auto_pop) { ui_pop_fixed_x(); state->fixed_x_stack.auto_pop = 0; }\
if(state->fixed_y_stack.auto_pop) { ui_pop_fixed_y(); state->fixed_y_stack.auto_pop = 0; }\
if(state->fixed_width_stack.auto_pop) { ui_pop_fixed_width(); state->fixed_width_stack.auto_pop = 0; }\
if(state->fixed_height_stack.auto_pop) { ui_pop_fixed_height(); state->fixed_height_stack.auto_pop = 0; }\
if(state->pref_width_stack.auto_pop) { ui_pop_pref_width(); state->pref_width_stack.auto_pop = 0; }\
if(state->pref_height_stack.auto_pop) { ui_pop_pref_height(); state->pref_height_stack.auto_pop = 0; }\
if(state->min_width_stack.auto_pop) { ui_pop_min_width(); state->min_width_stack.auto_pop = 0; }\
if(state->min_height_stack.auto_pop) { ui_pop_min_height(); state->min_height_stack.auto_pop = 0; }\
if(state->permission_flags_stack.auto_pop) { ui_pop_permission_flags(); state->permission_flags_stack.auto_pop = 0; }\
if(state->flags_stack.auto_pop) { ui_pop_flags(); state->flags_stack.auto_pop = 0; }\
if(state->omit_flags_stack.auto_pop) { ui_pop_omit_flags(); state->omit_flags_stack.auto_pop = 0; }\
if(state->focus_hot_stack.auto_pop) { ui_pop_focus_hot(); state->focus_hot_stack.auto_pop = 0; }\
if(state->focus_active_stack.auto_pop) { ui_pop_focus_active(); state->focus_active_stack.auto_pop = 0; }\
if(state->fastpath_codepoint_stack.auto_pop) { ui_pop_fastpath_codepoint(); state->fastpath_codepoint_stack.auto_pop = 0; }\
if(state->group_key_stack.auto_pop) { ui_pop_group_key(); state->group_key_stack.auto_pop = 0; }\
if(state->transparency_stack.auto_pop) { ui_pop_transparency(); state->transparency_stack.auto_pop = 0; }\
if(state->tag_stack.auto_pop) { ui_pop_tag(); state->tag_stack.auto_pop = 0; }\
if(state->background_color_stack.auto_pop) { ui_pop_background_color(); state->background_color_stack.auto_pop = 0; }\
if(state->text_color_stack.auto_pop) { ui_pop_text_color(); state->text_color_stack.auto_pop = 0; }\
if(state->border_color_stack.auto_pop) { ui_pop_border_color(); state->border_color_stack.auto_pop = 0; }\
if(state->squish_stack.auto_pop) { ui_pop_squish(); state->squish_stack.auto_pop = 0; }\
if(state->hover_cursor_stack.auto_pop) { ui_pop_hover_cursor(); state->hover_cursor_stack.auto_pop = 0; }\
if(state->font_stack.auto_pop) { ui_pop_font(); state->font_stack.auto_pop = 0; }\
if(state->font_size_stack.auto_pop) { ui_pop_font_size(); state->font_size_stack.auto_pop = 0; }\
if(state->text_raster_flags_stack.auto_pop) { ui_pop_text_raster_flags(); state->text_raster_flags_stack.auto_pop = 0; }\
if(state->tab_size_stack.auto_pop) { ui_pop_tab_size(); state->tab_size_stack.auto_pop = 0; }\
if(state->corner_radius_00_stack.auto_pop) { ui_pop_corner_radius_00(); state->corner_radius_00_stack.auto_pop = 0; }\
if(state->corner_radius_01_stack.auto_pop) { ui_pop_corner_radius_01(); state->corner_radius_01_stack.auto_pop = 0; }\
if(state->corner_radius_10_stack.auto_pop) { ui_pop_corner_radius_10(); state->corner_radius_10_stack.auto_pop = 0; }\
if(state->corner_radius_11_stack.auto_pop) { ui_pop_corner_radius_11(); state->corner_radius_11_stack.auto_pop = 0; }\
if(state->visual_margin_x_stack.auto_pop) { ui_pop_visual_margin_x(); state->visual_margin_x_stack.auto_pop = 0; }\
if(state->visual_margin_y_stack.auto_pop) { ui_pop_visual_margin_y(); state->visual_margin_y_stack.auto_pop = 0; }\
if(state->blur_size_stack.auto_pop) { ui_pop_blur_size(); state->blur_size_stack.auto_pop = 0; }\
if(state->text_padding_stack.auto_pop) { ui_pop_text_padding(); state->text_padding_stack.auto_pop = 0; }\
if(state->text_alignment_stack.auto_pop) { ui_pop_text_alignment(); state->text_alignment_stack.auto_pop = 0; }\

internal UI_Box *                   ui_top_parent(void);
internal Axis2                      ui_top_child_layout_axis(void);
internal f32                        ui_top_fixed_x(void);
internal f32                        ui_top_fixed_y(void);
internal f32                        ui_top_fixed_width(void);
internal f32                        ui_top_fixed_height(void);
internal UI_Size                    ui_top_pref_width(void);
internal UI_Size                    ui_top_pref_height(void);
internal f32                        ui_top_min_width(void);
internal f32                        ui_top_min_height(void);
internal UI_PermissionFlags         ui_top_permission_flags(void);
internal UI_BoxFlags                ui_top_flags(void);
internal UI_BoxFlags                ui_top_omit_flags(void);
internal UI_FocusKind               ui_top_focus_hot(void);
internal UI_FocusKind               ui_top_focus_active(void);
internal u32                        ui_top_fastpath_codepoint(void);
internal UI_Key                     ui_top_group_key(void);
internal f32                        ui_top_transparency(void);
internal String8                    ui_top_tag(void);
internal Vec4f32                    ui_top_background_color(void);
internal Vec4f32                    ui_top_text_color(void);
internal Vec4f32                    ui_top_border_color(void);
internal f32                        ui_top_squish(void);
internal WM_Cursor                  ui_top_hover_cursor(void);
internal FNT_Tag                    ui_top_font(void);
internal f32                        ui_top_font_size(void);
internal FNT_RasterFlags            ui_top_text_raster_flags(void);
internal f32                        ui_top_tab_size(void);
internal f32                        ui_top_corner_radius_00(void);
internal f32                        ui_top_corner_radius_01(void);
internal f32                        ui_top_corner_radius_10(void);
internal f32                        ui_top_corner_radius_11(void);
internal f32                        ui_top_visual_margin_x(void);
internal f32                        ui_top_visual_margin_y(void);
internal f32                        ui_top_blur_size(void);
internal f32                        ui_top_text_padding(void);
internal UI_TextAlign               ui_top_text_alignment(void);
internal UI_Box *                   ui_bottom_parent(void);
internal Axis2                      ui_bottom_child_layout_axis(void);
internal f32                        ui_bottom_fixed_x(void);
internal f32                        ui_bottom_fixed_y(void);
internal f32                        ui_bottom_fixed_width(void);
internal f32                        ui_bottom_fixed_height(void);
internal UI_Size                    ui_bottom_pref_width(void);
internal UI_Size                    ui_bottom_pref_height(void);
internal f32                        ui_bottom_min_width(void);
internal f32                        ui_bottom_min_height(void);
internal UI_PermissionFlags         ui_bottom_permission_flags(void);
internal UI_BoxFlags                ui_bottom_flags(void);
internal UI_BoxFlags                ui_bottom_omit_flags(void);
internal UI_FocusKind               ui_bottom_focus_hot(void);
internal UI_FocusKind               ui_bottom_focus_active(void);
internal u32                        ui_bottom_fastpath_codepoint(void);
internal UI_Key                     ui_bottom_group_key(void);
internal f32                        ui_bottom_transparency(void);
internal String8                    ui_bottom_tag(void);
internal Vec4f32                    ui_bottom_background_color(void);
internal Vec4f32                    ui_bottom_text_color(void);
internal Vec4f32                    ui_bottom_border_color(void);
internal f32                        ui_bottom_squish(void);
internal WM_Cursor                  ui_bottom_hover_cursor(void);
internal FNT_Tag                    ui_bottom_font(void);
internal f32                        ui_bottom_font_size(void);
internal FNT_RasterFlags            ui_bottom_text_raster_flags(void);
internal f32                        ui_bottom_tab_size(void);
internal f32                        ui_bottom_corner_radius_00(void);
internal f32                        ui_bottom_corner_radius_01(void);
internal f32                        ui_bottom_corner_radius_10(void);
internal f32                        ui_bottom_corner_radius_11(void);
internal f32                        ui_bottom_visual_margin_x(void);
internal f32                        ui_bottom_visual_margin_y(void);
internal f32                        ui_bottom_blur_size(void);
internal f32                        ui_bottom_text_padding(void);
internal UI_TextAlign               ui_bottom_text_alignment(void);
internal UI_Box *                   ui_push_parent(UI_Box * v);
internal Axis2                      ui_push_child_layout_axis(Axis2 v);
internal f32                        ui_push_fixed_x(f32 v);
internal f32                        ui_push_fixed_y(f32 v);
internal f32                        ui_push_fixed_width(f32 v);
internal f32                        ui_push_fixed_height(f32 v);
internal UI_Size                    ui_push_pref_width(UI_Size v);
internal UI_Size                    ui_push_pref_height(UI_Size v);
internal f32                        ui_push_min_width(f32 v);
internal f32                        ui_push_min_height(f32 v);
internal UI_PermissionFlags         ui_push_permission_flags(UI_PermissionFlags v);
internal UI_BoxFlags                ui_push_flags(UI_BoxFlags v);
internal UI_BoxFlags                ui_push_omit_flags(UI_BoxFlags v);
internal UI_FocusKind               ui_push_focus_hot(UI_FocusKind v);
internal UI_FocusKind               ui_push_focus_active(UI_FocusKind v);
internal u32                        ui_push_fastpath_codepoint(u32 v);
internal UI_Key                     ui_push_group_key(UI_Key v);
internal f32                        ui_push_transparency(f32 v);
internal String8                    ui_push_tag(String8 v);
internal Vec4f32                    ui_push_background_color(Vec4f32 v);
internal Vec4f32                    ui_push_text_color(Vec4f32 v);
internal Vec4f32                    ui_push_border_color(Vec4f32 v);
internal f32                        ui_push_squish(f32 v);
internal WM_Cursor                  ui_push_hover_cursor(WM_Cursor v);
internal FNT_Tag                    ui_push_font(FNT_Tag v);
internal f32                        ui_push_font_size(f32 v);
internal FNT_RasterFlags            ui_push_text_raster_flags(FNT_RasterFlags v);
internal f32                        ui_push_tab_size(f32 v);
internal f32                        ui_push_corner_radius_00(f32 v);
internal f32                        ui_push_corner_radius_01(f32 v);
internal f32                        ui_push_corner_radius_10(f32 v);
internal f32                        ui_push_corner_radius_11(f32 v);
internal f32                        ui_push_visual_margin_x(f32 v);
internal f32                        ui_push_visual_margin_y(f32 v);
internal f32                        ui_push_blur_size(f32 v);
internal f32                        ui_push_text_padding(f32 v);
internal UI_TextAlign               ui_push_text_alignment(UI_TextAlign v);
internal UI_Box *                   ui_pop_parent(void);
internal Axis2                      ui_pop_child_layout_axis(void);
internal f32                        ui_pop_fixed_x(void);
internal f32                        ui_pop_fixed_y(void);
internal f32                        ui_pop_fixed_width(void);
internal f32                        ui_pop_fixed_height(void);
internal UI_Size                    ui_pop_pref_width(void);
internal UI_Size                    ui_pop_pref_height(void);
internal f32                        ui_pop_min_width(void);
internal f32                        ui_pop_min_height(void);
internal UI_PermissionFlags         ui_pop_permission_flags(void);
internal UI_BoxFlags                ui_pop_flags(void);
internal UI_BoxFlags                ui_pop_omit_flags(void);
internal UI_FocusKind               ui_pop_focus_hot(void);
internal UI_FocusKind               ui_pop_focus_active(void);
internal u32                        ui_pop_fastpath_codepoint(void);
internal UI_Key                     ui_pop_group_key(void);
internal f32                        ui_pop_transparency(void);
internal String8                    ui_pop_tag(void);
internal Vec4f32                    ui_pop_background_color(void);
internal Vec4f32                    ui_pop_text_color(void);
internal Vec4f32                    ui_pop_border_color(void);
internal f32                        ui_pop_squish(void);
internal WM_Cursor                  ui_pop_hover_cursor(void);
internal FNT_Tag                    ui_pop_font(void);
internal f32                        ui_pop_font_size(void);
internal FNT_RasterFlags            ui_pop_text_raster_flags(void);
internal f32                        ui_pop_tab_size(void);
internal f32                        ui_pop_corner_radius_00(void);
internal f32                        ui_pop_corner_radius_01(void);
internal f32                        ui_pop_corner_radius_10(void);
internal f32                        ui_pop_corner_radius_11(void);
internal f32                        ui_pop_visual_margin_x(void);
internal f32                        ui_pop_visual_margin_y(void);
internal f32                        ui_pop_blur_size(void);
internal f32                        ui_pop_text_padding(void);
internal UI_TextAlign               ui_pop_text_alignment(void);
internal UI_Box *                   ui_set_next_parent(UI_Box * v);
internal Axis2                      ui_set_next_child_layout_axis(Axis2 v);
internal f32                        ui_set_next_fixed_x(f32 v);
internal f32                        ui_set_next_fixed_y(f32 v);
internal f32                        ui_set_next_fixed_width(f32 v);
internal f32                        ui_set_next_fixed_height(f32 v);
internal UI_Size                    ui_set_next_pref_width(UI_Size v);
internal UI_Size                    ui_set_next_pref_height(UI_Size v);
internal f32                        ui_set_next_min_width(f32 v);
internal f32                        ui_set_next_min_height(f32 v);
internal UI_PermissionFlags         ui_set_next_permission_flags(UI_PermissionFlags v);
internal UI_BoxFlags                ui_set_next_flags(UI_BoxFlags v);
internal UI_BoxFlags                ui_set_next_omit_flags(UI_BoxFlags v);
internal UI_FocusKind               ui_set_next_focus_hot(UI_FocusKind v);
internal UI_FocusKind               ui_set_next_focus_active(UI_FocusKind v);
internal u32                        ui_set_next_fastpath_codepoint(u32 v);
internal UI_Key                     ui_set_next_group_key(UI_Key v);
internal f32                        ui_set_next_transparency(f32 v);
internal String8                    ui_set_next_tag(String8 v);
internal Vec4f32                    ui_set_next_background_color(Vec4f32 v);
internal Vec4f32                    ui_set_next_text_color(Vec4f32 v);
internal Vec4f32                    ui_set_next_border_color(Vec4f32 v);
internal f32                        ui_set_next_squish(f32 v);
internal WM_Cursor                  ui_set_next_hover_cursor(WM_Cursor v);
internal FNT_Tag                    ui_set_next_font(FNT_Tag v);
internal f32                        ui_set_next_font_size(f32 v);
internal FNT_RasterFlags            ui_set_next_text_raster_flags(FNT_RasterFlags v);
internal f32                        ui_set_next_tab_size(f32 v);
internal f32                        ui_set_next_corner_radius_00(f32 v);
internal f32                        ui_set_next_corner_radius_01(f32 v);
internal f32                        ui_set_next_corner_radius_10(f32 v);
internal f32                        ui_set_next_corner_radius_11(f32 v);
internal f32                        ui_set_next_visual_margin_x(f32 v);
internal f32                        ui_set_next_visual_margin_y(f32 v);
internal f32                        ui_set_next_blur_size(f32 v);
internal f32                        ui_set_next_text_padding(f32 v);
internal UI_TextAlign               ui_set_next_text_alignment(UI_TextAlign v);
#endif // UI_META_H
