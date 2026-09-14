
///////////////////////
// UI Widgets: Fancy Buttons

internal void ti_cmd_binding_buttons(String8 name, String8 filter, u64 limit, TI_CmdBindingButtonFlags flags)
{
}

internal UI_Signal ti_menu_bar_button(String8 string)
{
    UI_Box *box = ui_build_box_from_string(UI_BoxFlag_DrawText|
                                           UI_BoxFlag_DrawBorder|
                                           UI_BoxFlag_DrawBackground|
                                           UI_BoxFlag_Clickable|
                                           UI_BoxFlag_DrawHotEffects,
                                           string);
    UI_Signal sig = ui_signal_from_box(box);
    return sig;
}

internal UI_Signal ti_cmd_spec_button(String8 name)
{
}

internal void ti_cmd_list_menu_buttons(u64 count, String8 *cmd_names, u32 *fastpath_codepoints)
{
}

internal UI_Signal ti_icon_button(TI_IconKind kind, Fuzzy_Match_Range_List *matches, String8 string)
{
    String8 display_string = ui_display_part_from_key_string(string);
    ui_set_next_child_layout_axis(Axis2_X);
    UI_Box *box = ui_build_box_from_string(UI_BoxFlag_Clickable|
                                           UI_BoxFlag_DrawBorder|
                                           UI_BoxFlag_DrawBackground|
                                           UI_BoxFlag_DrawHotEffects|
                                           UI_BoxFlag_DrawActiveEffects,
                                           string);
    UI_Parent(box)
    {
        if (display_string.size == 0)
        {
            ui_spacer(ui_pct(1, 0));
        }
        else
        {
            ui_spacer(ui_em(1.f, 1.f));
        }
        UI_TextAlignment(UI_TextAlign_Center)
            TI_Font(TI_FontSlot_Icons)
            UI_PrefWidth(ui_em(2.f, 1.f))
            UI_PrefHeight(ui_pct(1, 0))
            UI_FlagsAdd(UI_BoxFlag_DisableTextTrunc)
            UI_TagF("weak")
            ui_label(ti_icon_kind_text_table[kind]);
        if (display_string.size != 0)
        {
            UI_PrefWidth(ui_pct(1.f, 0.f))
            {
                UI_Box *box = ui_label(display_string).box;
                if (matches != 0)
                {
                    ui_box_equip_fuzzy_match_ranges(box, matches);
                }
            }
        }
        if (display_string.size == 0)
        {
            ui_spacer(ui_pct(1, 0));
        }
        else
        {
            ui_spacer(ui_em(1.f, 1.f));
        }
    }
    UI_Signal result = ui_signal_from_box(box);
    return result;
}

internal UI_Signal ti_icon_buttonf(TI_IconKind kind, Fuzzy_Match_Range_List *matches, char *fmt, ...)
{
    Temp scratch = scratch_begin(0, 0);
    va_list args;
    va_start(args, fmt);
    String8 string = push_str8fv(scratch.arena, fmt, args);
    va_end(args);
    UI_Signal sig = ti_icon_button(kind, matches, string);
    scratch_end(scratch);
    return sig;
}
