// Copyright (c) Frances Telfar
// Licensed under the PolyForm Noncommercial License (https://polyformproject.org/licenses/noncommercial/1.0.0)

#ifndef TIDE_WIDGETS_H
#define TIDE_WIDGETS_H

///////////////
// Cell Types
typedef u32 TI_CellFlags;
enum {
    // expander
    TI_CellFlag_Expander            = (1 << 0),
    TI_CellFlag_ExpanderSpace       = (1 << 1),
    TI_CellFlag_ExpanderPlaceholder = (1 << 2),

    // toggle switch extension
    TI_CellFlag_ToggleSwitch        = (1 << 3),

    // slider extension
    TI_CellFlag_Slider              = (1 << 4),

    // bindings extension
    TI_CellFlag_Bindings            = (1 << 5),

    // extra button extensions
    TI_CellFlag_EmptyEditButton     = (1 << 6),
    TI_CellFlag_RevertButton        = (1 << 7),
    TI_CellFlag_Lock                = (1 << 8),
    TI_CellFlag_BrowseButton        = (1 << 9),

    // behaviour
    TI_CellFlag_DisableEdit         = (1 << 10),
    TI_CellFlag_KeyboardClickable   = (1 << 11),
    TI_CellFlag_SingleClickActivate = (1 << 12),

    // contents description
    TI_CellFlag_CodeContents        = (1 << 13),

    // appearance
    TI_CellFlag_Border              = (1 << 14),
    TI_CellFlag_NoBackground        = (1 << 15),
    TI_CellFlag_Button              = (1 << 16),
};

typedef struct TI_Cell_Params TI_Cell_Params;
struct TI_Cell_Params
{
    // catchall parameters
    TI_CellFlags flags;
    s32 depth;
    String8 pre_edit_value;
    DR_FStr_List meta_fstrs;
    DR_FStr_List value_fstrs;
    DR_FStr_List note_fstrs;
    UI_Size note_width;
    String8 search_needle;
    String8 description;

    // expander r/w info
    bool32 *expanded_out;

    // lock r/w info
    bool32 *lock_out;

    // toggle-switch r/w info
    bool32 *toggled_out;

    // slider info r/w info
    f32 *slider_value_out;

    // bindings name w info
    String8 bindings_name;

    // revert out
    bool32 *revert_out;

    // browse out
    bool32 *browse_out;

    // text editing r/w info
    u64 *cursor;
    u64 *mark;
    u8 *edit_buffer;
    u64 edit_buffer_size;
    u64 *edit_string_size_out;
    UI_Key *line_edit_key_out;
};

/////////////////////////
// UI Building Helpers

#define TI_Font(slot) UI_Font(ti_font_from_slot(slot)) UI_TextRasterFlags(ti_raster_flags_from_slot((slot)))

//////////////////////
// UI Widgets: Fancy Title Strings

internal DR_FStr_List ti_title_fstrs_from_cfg(Arena *arena, CFG_Node *cfg, bool32 include_extras);

////////////////////
// UI Widgets: Loading Overlay

internal void ti_loading_overlay(Rng2f32 rect, f32 loading_t, u64 progress_v, u64 progress_v_target);

///////////////////////
// UI Widgets: Fancy Buttons

typedef u32 TI_CmdBindingButtonFlags;
enum {
    TI_CmdBindingButtonFlag_AddNew = (1 << 0),
    TI_CmdBindingButtonFlag_NoEdit = (1 << 1),
};
internal void ti_cmd_binding_buttons(String8 name, String8 filter, u64 limit, TI_CmdBindingButtonFlags flags);
internal UI_Signal ti_menu_bar_button(String8 string);
internal UI_Signal ti_cmd_spec_button(String8 name);
internal void ti_cmd_list_menu_buttons(u64 count, String8 *cmd_names, u32 *fastpath_codepoints);
internal UI_Signal ti_icon_button(TI_IconKind kind, Fuzzy_Match_Range_List *matches, String8 string);
internal UI_Signal ti_icon_buttonf(TI_IconKind kind, Fuzzy_Match_Range_List *matches, char *fmt, ...);

////////////////////////
// UI Widgets: Fancy Labels

internal DR_FStr_List ti_fstrs_from_rich_string(Arena *arena,  String8 string);
internal UI_Signal ti_label(String8 string);
internal UI_Signal ti_error_label(String8 string);
internal DR_FStr_List ti_fstrs_from_code_string(Arena *arena, f32 alpha, bool32 indirection_size_change, Vec4f32 base_color, String8 string);

//////////////////////
// UI Widgets: Line Edit

internal UI_Signal ti_cell(TI_Cell_Params *params, String8 string);
internal UI_Signal ti_cellf(TI_Cell_Params *params, char *fmt, ...);

#endif // TIDE_WIDGETS_H
