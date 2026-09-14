
///////////////////////
// UI Widgets: Fancy Buttons

internal void ti_cmd_binding_buttons(String8 name, String8 filter, u64 limit, TI_CmdBindingButtonFlags flags)
{
    Temp scratch = scratch_begin(0, 0);
    CFG_Key_Map_Node_Ptr_List key_map_nodes = cfg_key_map_node_ptr_list_from_name(scratch.arena, ti_state->key_map, name);
    
    //- rjf: build buttons for each binding
    u64 key_map_idx = 0;
    UI_CornerRadius(ui_top_font_size()*0.5f) for(CFG_Key_Map_Node_Ptr *n = key_map_nodes.first; n != 0; n = n->next, key_map_idx += 1)
    {
        if(key_map_idx >= limit) { break; }
        ui_spacer(ui_em(1.f, 1.f));
        CFG_Binding binding = n->v->binding;
        bool32 rebinding_active_for_this_binding = (ti_state->bind_change_active &&
                                                    str8_match(ti_state->bind_change_cmd_name, name, 0) &&
                                                    n->v->cfg_id == ti_state->bind_change_binding_id);
        
        //- rjf: grab all conflicts
        bool32 has_conflicts = 0;
        CFG_Key_Map_Node_Ptr_List nodes_with_this_binding = cfg_key_map_node_ptr_list_from_binding(scratch.arena, ti_state->key_map, binding);
        {
            for(CFG_Key_Map_Node_Ptr *n2 = nodes_with_this_binding.first; n2 != 0; n2 = n2->next)
            {
                if(!str8_match(n->v->name, n2->v->name, 0))
                {
                    has_conflicts = 1;
                    break;
                }
            }
        }
        
        //- rjf: form binding string
        String8 keybinding_str = {0};
        {
            if(binding.key != WM_Key_Null)
            {
                keybinding_str = wm_string_from_modifiers_key(scratch.arena, binding.modifiers, binding.key);
            }
            else
            {
                keybinding_str = str8_lit("- no binding -");
            }
        }
        
        //- rjf: compute fuzzy matches
        Fuzzy_Match_Range_List matches = {0};
        if(filter.size != 0)
        {
            matches = fuzzy_match_find(scratch.arena, filter, keybinding_str);
        }
        
        //- rjf: build box
        ui_set_next_tag(has_conflicts ? str8_lit("bad_pop") : rebinding_active_for_this_binding ? str8_lit("pop") : str8_zero());
        ui_set_next_text_alignment(UI_TextAlign_Center);
        ui_set_next_group_key(ui_key_zero());
        ui_set_next_pref_width(ui_text_dim(ui_top_font_size()*1.f, 1));
        UI_Box *box = ui_build_box_from_stringf(UI_BoxFlag_DrawText|
                                                (!(flags & TI_CmdBindingButtonFlag_NoEdit) * UI_BoxFlag_Clickable)|
                                                UI_BoxFlag_DrawActiveEffects|
                                                UI_BoxFlag_DrawHotEffects|
                                                UI_BoxFlag_DrawBorder|
                                                UI_BoxFlag_DrawBackground,
                                                "%S###bind_btn_%S_%x_%x", keybinding_str, name, binding.key, binding.modifiers);
        ui_box_equip_fuzzy_match_ranges(box, &matches);
        
        //- rjf: interaction
        UI_Signal sig = ui_signal_from_box(box);
        {
            // rjf: click => toggle activity
            if(!ti_state->bind_change_active && ui_clicked(sig))
            {
                if((binding.key == WM_Key_Esc || binding.key == WM_Key_Delete) && binding.modifiers == 0)
                {
                    log_user_error(str8_lit("Cannot rebind; this command uses a reserved keybinding."));
                }
                else
                {
                    arena_clear(ti_state->bind_change_arena);
                    ti_state->bind_change_active = 1;
                    ti_state->bind_change_cmd_name = push_str8_copy(ti_state->bind_change_arena, name);
                    ti_state->bind_change_binding_id = n->v->cfg_id;
                }
            }
            else if(ti_state->bind_change_active && ui_clicked(sig))
            {
                ti_state->bind_change_active = 0;
            }
            
            // rjf: hover w/ conflicts => show conflicts
            if(ui_hovering(sig) && has_conflicts) UI_Tooltip
            {
                UI_PrefWidth(ui_children_sum(1)) ti_error_label(str8_lit("This binding conflicts with those for:"));
                for(CFG_Key_Map_Node_Ptr *n2 = nodes_with_this_binding.first; n2 != 0; n2 = n2->next)
                {
                    if(!str8_match(n2->v->name, n->v->name, 0))
                    {
                        String8 display_name = s("test");//ti_display_from_code_name(n2->v->name);
                        ui_labelf("%S", display_name);
                    }
                }
            }
        }
    
        //- rjf: delete button
        if(rebinding_active_for_this_binding)
            UI_PrefWidth(ui_em(2.5f, 1.f))
            UI_TagF("bad_pop")
        {
            ui_set_next_group_key(ui_key_zero());
            UI_Signal sig = ti_icon_button(TI_IconKind_X, 0, str8_lit("###delete_binding"));
            if(ui_clicked(sig))
            {
                cfg_node_release(ti_state->cfg, cfg_node_from_id(ti_state->bind_change_binding_id));
                ti_state->bind_change_active = 0;
            }
        }
    }
  
    //- rjf: build "add new binding" button
    if(flags & TI_CmdBindingButtonFlag_AddNew)
    {
        bool32 adding_new_binding = (ti_state->bind_change_active &&
                                     str8_match(ti_state->bind_change_cmd_name, name, 0) &&
                                     ti_state->bind_change_binding_id == 0);
        ui_spacer(ui_em(1.f, 1.f));
        {
            UI_Box *box = &ui_nil_box;
            TI_Font(TI_FontSlot_Icons) UI_TagF(adding_new_binding ? "pop" : "") UI_CornerRadius(ui_top_font_size()*0.5f)
            {
                ui_set_next_text_alignment(UI_TextAlign_Center);
                ui_set_next_group_key(ui_key_zero());
                ui_set_next_pref_width(ui_text_dim(ui_top_font_size()*1.5f, 1));
                box = ui_build_box_from_stringf(UI_BoxFlag_DrawText|
                                                UI_BoxFlag_Clickable|
                                                UI_BoxFlag_DrawActiveEffects|
                                                UI_BoxFlag_DrawHotEffects|
                                                UI_BoxFlag_DrawBorder|
                                                UI_BoxFlag_DrawBackground,
                                                "%S###add_binding", ti_icon_kind_text_table[TI_IconKind_Add]);
            }
            UI_Signal sig = ui_signal_from_box(box);
            if(ui_hovering(sig)) UI_Tooltip
            {
                ui_state->tooltip_anchor_key = box->key;
                ui_labelf("Add New Binding");
            }
            if(ui_clicked(sig))
            {
                if(!adding_new_binding && ui_clicked(sig))
                {
                    arena_clear(ti_state->bind_change_arena);
                    ti_state->bind_change_active = 1;
                    ti_state->bind_change_cmd_name = push_str8_copy(ti_state->bind_change_arena, name);
                    ti_state->bind_change_binding_id = 0;
                }
                else if(adding_new_binding && ui_clicked(sig))
                {
                    ti_state->bind_change_active = 0;
                }
            }
        }
    }
    
    scratch_end(scratch);
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
    TI_Cmd_Kind_Info *info = ti_cmd_kind_info_from_string(name);
    ui_set_next_child_layout_axis(Axis2_X);
    UI_Box *box = ui_build_box_from_stringf(UI_BoxFlag_DrawBorder|
                                            UI_BoxFlag_DrawBackground|
                                            UI_BoxFlag_DrawHotEffects|
                                            UI_BoxFlag_DrawActiveEffects|
                                            UI_BoxFlag_Clickable,
                                            "###cmd_%p", info);
    UI_Parent(box) UI_HeightFill UI_Padding(ui_em(1.f, 1.f))
    {
        TI_IconKind canonical_icon = TI_IconKind_Null;//ti_icon_kind_from_code_name(name);
        if (canonical_icon != TI_IconKind_Null)
        {
            TI_Font(TI_FontSlot_Icons)
                UI_PrefWidth(ui_em(2.f, 1.f))
                UI_TextAlignment(UI_TextAlign_Center)
                UI_TagF("weak")
            {
                ui_label(ti_icon_kind_text_table[canonical_icon]);
            }
        }
        UI_PrefWidth(ui_text_dim(10, 1.f))
        {
            UI_Flags(UI_BoxFlag_DrawTextFastpathCodepoint)
                UI_FastpathCodepoint(box->fastpath_codepoint)
                ui_label(s("test"));
                //ui_label(ti_display_from_code_name(name));
            ui_spacer(ui_pct(1, 0));
            ui_set_next_flags(UI_BoxFlag_Clickable);
            ui_set_next_group_key(ui_key_zero());
            UI_PrefWidth(ui_children_sum(1))
                UI_FontSize(ui_top_font_size()*0.95f) UI_HeightFill
                UI_NamedRow(str8_lit("###bindings"))
                UI_TagF("weak")
                UI_FastpathCodepoint(0)
            {
                ti_cmd_binding_buttons(name, str8_zero(), max_u64, TI_CmdBindingButtonFlag_AddNew);
            }
        }
    }
    UI_Signal sig = ui_signal_from_box(box);
    return sig;
}

internal void ti_cmd_list_menu_buttons(u64 count, String8 *cmd_names, u32 *fastpath_codepoints)
{
    Temp scratch = scratch_begin(0, 0);
    for EachIndex(idx, count)
    {
        if (cmd_names[idx].size == 0)
        {
            UI_TagF("floating") ui_divider(ui_em(1.f, 1.f));
        }
        else
        {
            ui_set_next_fastpath_codepoint(fastpath_codepoints[idx]);
            UI_Signal sig = ti_cmd_spec_button(cmd_names[idx]);
            if (ui_clicked(sig))
            {
                ti_cmd(TI_CmdKind_RunCommand, .cmd_name = cmd_names[idx]);
                ui_ctx_menu_close();
                CFG_Node *window = cfg_node_from_id(ti_regs()->window);
                TI_Window_State *ws = ti_window_state_from_cfg(window);
                ws->menu_bar_focused = false;
            }
        }
    }
    scratch_end(scratch);
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

////////////////////////
// UI Widgets: Fancy Labels

internal DR_FStr_List ti_fstrs_from_rich_string(Arena *arena,  String8 string)
{
    Temp scratch = scratch_begin(&arena, 1);
    typedef u32 StringPartFlags;
    enum
        {
            StringPartFlag_Code      = (1<<0),
            StringPartFlag_Underline = (1<<1),
            StringPartFlag_Bright    = (1<<2),
        };
    typedef struct String_Part String_Part;
    struct String_Part
    {
        String_Part *next;
        StringPartFlags flags;
        String8 string;
    };
    String_Part *first_part = 0;
    String_Part *last_part = 0;
    u64 active_part_start_idx = 0;
    StringPartFlags active_part_flags = 0;
    for(u64 idx = 0; idx <= string.size; idx += 1)
    {
        if(idx == string.size)
        {
            String_Part *p = push_array(scratch.arena, String_Part, 1);
            p->flags = active_part_flags;
            p->string = str8_substr(string, r1u64(active_part_start_idx, idx));
            SLLQueuePush(first_part, last_part, p);
        }
        else if(string.str[idx] == '`')
        {
            String_Part *p = push_array(scratch.arena, String_Part, 1);
            p->flags = active_part_flags;
            p->string = str8_substr(string, r1u64(active_part_start_idx, idx));
            SLLQueuePush(first_part, last_part, p);
            active_part_start_idx = idx+1;
            active_part_flags ^= StringPartFlag_Code;
        }
    }
    DR_FStr_List fstrs = {0};
    for(String_Part *p = first_part; p != 0; p = p->next)
    {
        DR_FStr fstr = {0};
        {
            fstr.string = p->string;
            fstr.params.font   = ui_top_font();
            fstr.params.color  = ui_color_from_name(str8_lit("text"));
            fstr.params.size   = ui_top_font_size();
            fstr.params.raster_flags = ti_raster_flags_from_slot(TI_FontSlot_Main);
            if(p->flags & StringPartFlag_Code)
            {
                fstr.params.font = ti_font_from_slot(TI_FontSlot_Code);
                fstr.params.raster_flags = ti_raster_flags_from_slot(TI_FontSlot_Code);
                fstr.params.color = v4f32(1, 1, 1, 1);//ti_rgba_from_code_color_slot(TI_CodeColorSlot_CodeDefault);
            }
        }
        dr_fstrs_push(arena, &fstrs, &fstr);
    }
    scratch_end(scratch);
    return fstrs;
}

internal UI_Signal ti_label(String8 string)
{
    Temp scratch = scratch_begin(0, 0);
    DR_FStr_List fstrs = ti_fstrs_from_rich_string(scratch.arena, string);
    UI_Box *box = ui_build_box_from_key(UI_BoxFlag_DrawText, ui_key_zero());
    ui_box_equip_display_fstrs(box, &fstrs);
    UI_Signal sig = ui_signal_from_box(box);
    scratch_end(scratch);
    return sig;
}

internal UI_Signal ti_error_label(String8 string)
{
    UI_Box *box = ui_build_box_from_key(0, ui_key_zero());
    UI_Signal sig = ui_signal_from_box(box);
    UI_Parent(box)
    {
        ui_set_next_font(ti_font_from_slot(TI_FontSlot_Icons));
        ui_set_next_text_raster_flags(FNT_RasterFlag_Smooth);
        ui_set_next_text_alignment(UI_TextAlign_Center);
        UI_TagF("weak") UI_PrefWidth(ui_em(2.25f, 1.f)) ui_label(ti_icon_kind_text_table[TI_IconKind_WarningBig]);
        UI_PrefWidth(ui_text_dim(10, 0)) ti_label(string);
    }
    return sig;
}
