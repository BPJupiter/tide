// Copyright (c) Frances Telfar
// Licensed under the PolyForm Noncommercial License (https://polyformproject.org/licenses/noncommercial/1.0.0)

#ifndef TIDE_WIDGETS_H
#define TIDE_WIDGETS_H

/////////////////////////
// UI Building Helpers

#define TI_Font(slot) UI_Font(ti_font_from_slot(slot)) UI_TextRasterFlags(ti_raster_flags_from_slot((slot)))

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


#endif // TIDE_WIDGETS_H
