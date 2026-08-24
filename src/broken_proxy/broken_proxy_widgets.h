
#ifndef BROKEN_PROXY_WIDGETS_H
#define BROKEN_PROXY_WIDGETS_H

///////////////
// Cell Types

typedef u32 BP_CellFlags;
enum {
    // expander
    BP_CellFlag_Expander            = (1 << 0),
    BP_CellFlag_ExpanderSpace       = (1 << 1),
    BP_CellFlag_ExpanderPlaceholder = (1 << 2),

    // toggle switch extension
    BP_CellFlag_ToggleSwitch        = (1 << 3),

    // slider extension
    BP_CellFlag_Slider              = (1 << 4),

    // bindings extension
    BP_CellFlag_Bindings            = (1 << 5),

    // extra buttons extensions
    BP_CellFlag_EmptyEditButton     = (1 << 6),
    BP_CellFlag_RevertButton        = (1 << 7),

    // reserved (1 << 8)

    // behaviour
    BP_CellFlag_DisableEdit         = (1 << 9),
    BP_CellFlag_KeyboardClickable   = (1 << 10),
    BP_CellFlag_SingleClickActivate = (1 << 11),

    // contents description
    BP_CellFlag_CodeContents        = (1 << 12),

    // appearance
    BP_CellFlag_Border              = (1 << 13),
    BP_CellFlag_NoBackground        = (1 << 14),
    BP_CellFlag_Button              = (1 << 15),
};

typedef struct BP_Cell_Params BP_Cell_Params;
struct BP_Cell_Params {
    // catchall parameters
    BP_CellFlags flags;
    s32 depth;
    String8 pre_edit_value;
    DR_Fstr_List meta_fstrs;
    DR_Fstr_List value_fstrs;
    DR_Fstr_List node_fstrs;
    UI_Size note_width;
    String8 search_needle;
    String8 description;

    // expander r/w info
    bool32 *expanded_out;

    // toggle-switch r/w info
    bool32 *toggled_out;

    // slider info r/w info
    bool32 *slider_value_out;

    // bindings name w info
    String8 bindings_name;

    // revert out
    bool32 *revert_out;

    // text editing r/w info
    u64 *cursor;
    u64 *mark;
    u8 *edit_buffer;
    u64 edit_buffer_size;
    u64 *edit_string_size_out;
    UI_Key *line_edit_key_out;
};

///////////////////////
// Code Slice Types

typedef u32 BP_CodeSliceFlags;
enum {
    BP_CodeSliceFlag_Clickable      = (1 << 0),
    BP_CodeSliceFlag_LineNums       = (1 << 1),
};

typedef struct BP_Code_Slice_Params BP_Code_Slice_Params;
struct BP_Code_Slice_Params {
    // content
    BP_CodeSliceFlags flags;;
    Rng1s64 line_num_range;
    String8 *line_text;
    Rng1u64 *line_ranges;
    TXT_Token_Array *line_tokens;
    TXT_Text_Info *text_info;
    TXT_Patch_List *patches;
    String8 text_data;

    // visual parameters
    FNT_Tag font;
    f32 font_size;
    f32 tab_size;
    String8 search_query;
    f32 line_height_px;
    f32 line_num_width_px;
    f32 line_text_max_width_px;
    f32 margin_float_off_px;
};

typedef struct BP_Code_Slice_Signal BP_Code_Slice_Signal;
struct BP_Code_Slice_Signal {
    UI_Signal base;
    u64 mouse_off;
    Rng1u64 mouse_expr_rng;
};

////////////////////////
// UI Building Helpers

#define BP_Font(slot) UI_Font(bp_font_from_slot(slot)) UI_TextRasterFlags(bp_raster_flags_from_slot((slot)))

//////////////////////////////
// UI Widgets: Fancy Title Strings

internal DR_FStr_List bp_title_fstrs_from_cfg(Arena *arena, CFG_Node *cfg, bool32 include_extras);
internal DR_FStr_List bp_title_fstrs_from_code_name(Arena *arena, String8 code_name);
internal DR_FStr_List bp_title_fstrs_from_file_path(Arena *arena, String8 file_path, bool32 include_folder);

//////////////////////////
// UI Widgets: Loading Overlay

internal void bp_loading_overlay(Rng2f32 rect, f32 loading_t, u64 progress_v, u64 progress_v_target);

//////////////////////
// UI Widgets: Fancy Buttons

typedef u32 BP_CmdBindingButtonFlags;
enum {
    BP_CmdBindingButtonFlag_AddNew = (1 << 0),
    BP_CmdBindingButtonFlag_NoEdit = (1 << 1),
};

internal void      bp_cmd_binding_buttons(String8 name, String8 filter, u64 limit, BP_CmdBindingButtonFlags flags);
internal UI_Signal bp_menu_bar_button(String8 string);
internal UI_Signal bp_cmd_spec_button(String8 name);
internal void      bp_cmd_list_menu_buttons(u64 count, String8 cmd_names, u32 *fastpath_codepoints);
internal UI_Signal bp_icon_button(BP_IconKind kind, Fuzzy_Match_Range_List *matches, String8 string);
internal UI_Signal bp_icon_buttonf(BP_IconKind kind, Fuzzy_Match_Range_List *matches, char *fmt, ...);

////////////////////////
// UI Widgets: Text View

internal UI_BOX_CUSTOM_DRAW(bp_code_slice_text_draw_extensions);
internal BP_Code_Slice_Signal bp_code_slice(BP_Code_Slice_Params *params, u64 *cursor, u64 *mark, s64 *preferred_column, String8 string);
internal BP_Code_Slice_Signal bp_code_slicef(BP_Code_Slice_Params *params, u64 *cursor, u64 *mark, s64 *preferred_column, char *fmt, ...);

internal bool32 bp_do_txt_controls(TXT_Text_Info *info, String8 data, TXT_Patch_List *pathces, u64 line_count_per_range, Txt_Pt *cursor, Txt_Pt *mark, s64 *preferred_column);

////////////////////////
// UI Widgets: Fancy Labels

internal DR_FStr_List bp_fstrs_from_rich_string(Arena *arena, String8 string);
internal UI_Signal    bp_label(String8 string);
internal UI_Signal    bp_error_label(String8 string);
internal bool32       bp_help_label(String8 string);
internal DR_Fstr_List bp_fstrs_from_code_string(Arena *arena, f32 alpha, bool32 indirection_size_change, Vec4f32 base_color, String8 string);
internal UI_Box      *bp_code_label(f32 alpha, bool32 indirection_size_change, Vec4f32 base_color, String8 string);

////////////////////
// UI Widgets: Line Edit

internal UI_Signal bp_cell(BP_Cell_Params *params, String8 string);
internal UI_Signal bp_cellf(BP_Cell_Params *params, char *fmt, ...);


#endif // BROKEN_PROXY_WIDGETS_H
