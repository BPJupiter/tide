// Copyright (c) Frances Telfar
// Licensed under the PolyForm Noncommercial License (https://polyformproject.org/licenses/noncommercial/1.0.0)

//////////////////////
// UI Widgets: Fancy Title Strings

internal DR_FStr_List ti_title_fstrs_from_cfg(Arena *arena, CFG_Node *cfg, bool32 include_extras)
{
    DR_FStr_List result = {0};
    {
        Temp scratch = scratch_begin(&arena, 1);

        
        scratch_end(scratch);
    }
    return result;
}

////////////////////
// UI Widgets: Loading Overlay

internal void ti_loading_overlay(Rng2f32 rect, f32 loading_t, u64 progress_v, u64 progress_v_target)
{
    if (loading_t >= 0.001f) UI_Focus(UI_FocusKind_Off)
    {
        // set up dimensions
        f32 edge_padding = 30.f;
        f32 width = ui_top_font_size() * 10;
        f32 height = ui_top_font_size() * 1.f;
        f32 min_thickness = ui_top_font_size()/2;
        f32 trail = ui_top_font_size() * 4;
        f32 t = pow_f32(sin_f32((f32)ti_state->time_in_seconds / 1.8f), 2.f);
        f64 v = 1.f - abs_f32(0.5f - t);

        // build indicator
        UI_CornerRadius(height/3.f) UI_Transparency(1 - loading_t)
        {
            // rects
            Rng2f32 indicator_region_rect =
                r2f32p((rect.x0 + rect.x1)/2 - width/2  - rect.x0,
                       (rect.y0 + rect.y1)/2 - height/2 - rect.y0,
                       (rect.x0 + rect.x1)/2 - width/2  - rect.x0,
                       (rect.y0 + rect.y1)/2 - height/2 - rect.y0);
            Rng2f32 indicator_rect =
                r2f32p(indicator_region_rect.x0 + width*t - min_thickness/2 - trail*v,
                       indicator_region_rect.y0,
                       indicator_region_rect.x0 + width*t + min_thickness/2 - trail*v,
                       indicator_region_rect.y1);
            indicator_rect.x0 = Clamp(indicator_region_rect.x0, indicator_rect.x0, indicator_region_rect.x1);
            indicator_rect.x1 = Clamp(indicator_region_rect.x0, indicator_rect.x1, indicator_region_rect.x1);
            indicator_rect = pad_2f32(indicator_rect, -1.f);

            // does the view have loading *progress* info? -> draw extra progress layer
            if (progress_v != progress_v_target) UI_TagF("drop_site")
            {
                f64 pct_done_f64 = ((f64)progress_v/(f64)progress_v_target);
                f32 pct_done = (f32)pct_done_f64;
                Rng2f32 pct_rect = r2f32p(indicator_region_rect.x0,
                                          indicator_region_rect.y0,
                                          indicator_region_rect.x0 + (indicator_region_rect.x1 - indicator_region_rect.x0)*pct_done,
                                          indicator_region_rect.y1);
                UI_Rect(pct_rect)
                    ui_build_box_from_key(UI_BoxFlag_DrawBackground|UI_BoxFlag_Floating, ui_key_zero());
            }

            // fill
            UI_TagF("pop") UI_Rect(indicator_rect)
                ui_build_box_from_key(UI_BoxFlag_DrawBackground|UI_BoxFlag_Floating, ui_key_zero());

            // animated bar
            UI_Rect(indicator_region_rect)
            {
                UI_Box *box = ui_build_box_from_stringf(UI_BoxFlag_DrawBackground|UI_BoxFlag_DrawBorder|UI_BoxFlag_Floating|UI_BoxFlag_Clickable, "bg_system_status");
                UI_Signal sig = ui_signal_from_box(box);
            }
        }

        // build background
        UI_WidthFill UI_HeightFill UI_Transparency(1-loading_t) UI_BlurSize(10.f*loading_t)
        {
            ui_set_next_blur_size(10.f*loading_t);
            ui_build_box_from_key(UI_BoxFlag_DrawBackground|UI_BoxFlag_DrawBackgroundBlur|UI_BoxFlag_Floating, ui_key_zero());
        }
    }
}

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
                        String8 display_name = ti_display_from_code_name(n2->v->name);
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
        TI_IconKind canonical_icon = ti_icon_kind_from_code_name(name);
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
                ui_label(ti_display_from_code_name(name));
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

internal DR_FStr_List ti_fstrs_from_code_string(Arena *arena, f32 alpha, bool32 indirection_size_change, Vec4f32 base_color, String8 string)
{
    ProfBeginFunction();
    Temp scratch = scratch_begin(&arena, 1);
    DR_FStr_List fstrs = {0};
    TXT_Token_Array tokens = txt_token_array_from_string__c_cpp(scratch.arena, 0, string);
    TXT_Token *tokens_opl = tokens.v+tokens.count;
    s32 indirection_counter = 0;
    indirection_size_change = 0;
    bool32 preceded_by_dot = 0;
    for(TXT_Token *token = tokens.v; token < tokens_opl; token += 1)
    {
        TI_CodeColorSlot token_color_slot = ti_code_color_slot_from_txt_token_kind(token->kind);
        Vec4f32 token_color_rgba = ti_rgba_from_code_color_slot(token_color_slot);
        String8 token_string = str8_substr(string, token->range);
        if(str8_match(token_string, str8_lit("{"), 0)) { indirection_counter += 1; }
        if(str8_match(token_string, str8_lit("["), 0)) { indirection_counter += 1; }
        indirection_counter = ClampBot(0, indirection_counter);
        switch(token->kind)
        {
            default:
                {
                    token_color_rgba.w *= alpha;
                    DR_FStr fstr =
                        {
                            token_string,
                            {
                                ui_top_font(),
                                ui_top_text_raster_flags(),
                                token_color_rgba,
                                ui_top_font_size() * (1.f - !!indirection_size_change*(indirection_counter/10.f)),
                            }
                        };
                    dr_fstrs_push(arena, &fstrs, &fstr);
                }break;
            case TXT_TokenKind_Identifier:
            case TXT_TokenKind_Keyword:
                {
                    TI_CodeColorSlot lookup_theme_color_slot = TI_CodeColorSlot_CodeDefault;
                    bool32 is_called = (token+1 < tokens_opl && token[1].kind == TXT_TokenKind_Symbol && str8_match(str8_substr(string, token[1].range), str8_lit("("), 0));
                    if(!preceded_by_dot)
                    {
                        lookup_theme_color_slot = ti_code_color_slot_from_txt_token_kind_lookup_string(token->kind, token_string, 1, is_called);
                    }
                    if(lookup_theme_color_slot != TI_CodeColorSlot_CodeDefault)
                    {
                        Vec4f32 lookup_color = ti_rgba_from_code_color_slot(lookup_theme_color_slot);
                        f32 lookup_color_mix_t = ui_anim(ui_key_from_stringf(ui_key_zero(), "%S_lookup", token_string), 1.f);
                        token_color_rgba = mix_4f32(token_color_rgba, lookup_color, lookup_color_mix_t);
                    }
                    token_color_rgba.w *= alpha;
                    DR_FStr fstr =
                        {
                            token_string,
                            {
                                ui_top_font(),
                                ui_top_text_raster_flags(),
                                token_color_rgba,
                                ui_top_font_size() * (1.f - !!indirection_size_change*(indirection_counter/10.f)),
                            },
                        };
                    dr_fstrs_push(arena, &fstrs, &fstr);
                }break;
            case TXT_TokenKind_Numeric:
                {
                    token_color_rgba.w *= alpha;
                    Vec4f32 token_color_rgba_alt = ti_rgba_from_code_color_slot(TI_CodeColorSlot_CodeNumericAltDigitGroup);
                    token_color_rgba_alt.w *= alpha;
                    f32 font_size = ui_top_font_size() * (1.f - !!indirection_size_change*(indirection_counter/10.f));
                    
                    // rjf: unpack string
                    u32 base = 10;
                    u64 prefix_skip = 0;
                    u64 digit_group_size = 3;
                    if(str8_match(str8_prefix(token_string, 2), str8_lit("0x"), StringMatchFlag_CaseInsensitive))
                    {
                        base = 16;
                        prefix_skip = 2;
                        digit_group_size = 4;
                    }
                    else if(str8_match(str8_prefix(token_string, 2), str8_lit("0b"), StringMatchFlag_CaseInsensitive))
                    {
                        base = 2;
                        prefix_skip = 2;
                        digit_group_size = 8;
                    }
                    else if(str8_match(str8_prefix(token_string, 2), str8_lit("0o"), StringMatchFlag_CaseInsensitive))
                    {
                        base = 8;
                        prefix_skip = 2;
                        digit_group_size = 2;
                    }
                    
                    // rjf: grab string parts
                    u64 dot_pos = str8_find_needle(token_string, 0, str8_lit("."), 0);
                    String8 prefix = str8_prefix(token_string, prefix_skip);
                    String8 whole = str8_substr(token_string, r1u64(prefix_skip, dot_pos));
                    String8 decimal = str8_skip(token_string, dot_pos);
                    
                    // rjf: determine # of digits
                    u64 num_digits = 0;
                    for(u64 idx = 0; idx < whole.size; idx += 1)
                    {
                        num_digits += char_is_digit(whole.str[idx], base);
                    }
                    
                    // rjf: push prefix
                    {
                        DR_FStr fstr =
                            {
                                prefix,
                                {
                                    ui_top_font(),
                                    ui_top_text_raster_flags(),
                                    token_color_rgba,
                                    font_size,
                                },
                            };
                        dr_fstrs_push(arena, &fstrs, &fstr);
                    }
                    
                    // rjf: push digit groups
                    {
                        bool32 odd = 0;
                        u64 start_idx = 0;
                        u64 num_digits_passed = digit_group_size - num_digits%digit_group_size;
                        for(u64 idx = 0; idx <= whole.size; idx += 1)
                        {
                            u8 byte = idx < whole.size ? whole.str[idx] : 0;
                            if(num_digits_passed >= digit_group_size || idx == whole.size)
                            {
                                num_digits_passed = 0;
                                if(start_idx < idx)
                                {
                                    DR_FStr fstr =
                                        {
                                            str8_substr(whole, r1u64(start_idx, idx)),
                                            {
                                                ui_top_font(),
                                                ui_top_text_raster_flags(),
                                                odd ? token_color_rgba_alt : token_color_rgba,
                                                font_size,
                                            },
                                        };
                                    dr_fstrs_push(arena, &fstrs, &fstr);
                                    start_idx = idx;
                                    odd ^= 1;
                                }
                            }
                            if(char_is_digit(byte, base))
                            {
                                num_digits_passed += 1;
                            }
                        }
                    }
                    
                    // rjf: push decimal
                    {
                        DR_FStr fstr =
                            {
                                decimal,
                                {
                                    ui_top_font(),
                                    ui_top_text_raster_flags(),
                                    token_color_rgba,
                                    font_size,
                                },
                            };
                        dr_fstrs_push(arena, &fstrs, &fstr);
                    }
                    
                }break;
        }
        if(token->kind == TXT_TokenKind_Symbol && str8_match(token_string, str8_lit("."), 0))
        {
            preceded_by_dot = 1;
        }
        else
        {
            preceded_by_dot = 0;
        }
        if(str8_match(token_string, str8_lit("}"), 0)) { indirection_counter -= 1; }
        if(str8_match(token_string, str8_lit("]"), 0)) { indirection_counter -= 1; }
        indirection_counter = ClampBot(0, indirection_counter);
    }
    scratch_end(scratch);
    ProfEnd();
    return fstrs;
}

//////////////////////
// UI Widgets: Line Edit

internal UI_Signal ti_cell(TI_Cell_Params *params, String8 string)
{
    ProfBeginFunction();
    Temp scratch = scratch_begin(0, 0);
    bool32 do_cursor_trail = ti_setting_bool32_from_name(s("animations")) && ti_setting_bool32_from_name(str8_lit("cursor_trail"));

    /////////////////////////
    // unpack visual metrics
    //
    f32 expander_size_px = floor_f32(ui_top_font_size()*2.f);

    ///////////////////////
    // make key
    //
    UI_Key key = ui_key_from_string(ui_active_seed_key(), string);

    ////////////////////////
    // calculate & push focus
    //
    bool32 is_auto_focus_hot    = ui_is_key_auto_focus_hot(key);
    bool32 is_auto_focus_active = ui_is_key_auto_focus_active(key);
    if(is_auto_focus_hot)    { ui_push_focus_hot(UI_FocusKind_On);    }
    if(is_auto_focus_active) { ui_push_focus_active(UI_FocusKind_On); }
    bool32 is_focus_hot    = ui_is_focus_hot();
    bool32 is_focus_active = ui_is_focus_active();
    bool32 is_focus_hot_disabled    = (!is_focus_hot    && ui_top_focus_hot()    == UI_FocusKind_On);
    bool32 is_focus_active_disabled = (!is_focus_active && ui_top_focus_active() == UI_FocusKind_On);

    ////////////////////////////////
    // determine which sub-cell components we'll build
    //
    // (the base line edit textual label / editor is always build, but this can be enriched
    // with extra widgets & metadata)
    //
    bool32 build_toggle_switch = !!(params->flags & TI_CellFlag_ToggleSwitch) && !is_focus_active;
    bool32 build_slider        = !!(params->flags & TI_CellFlag_Slider)       && !is_focus_active;
    bool32 build_bindings      = !!(params->flags & TI_CellFlag_Bindings)     && !is_focus_active;
    bool32 build_lhs_name_desc = (params->meta_fstrs.node_count != 0 || params->description.size       != 0);
    bool32 build_line_edit     = (params->pre_edit_value.size   != 0 || params->value_fstrs.node_count != 0);
    bool32 build_note          = (params->note_fstrs.node_count != 0 || !is_focus_active);
    DR_FStr_List lhs_name_fstrs   = params->meta_fstrs;
    DR_FStr_List value_name_fstrs = params->value_fstrs;
    DR_FStr_List note_fstrs       = params->note_fstrs;

    /////////////////////////
    // determine autocompletion string
    //
    String8 autocomplete_hint_string = {0};
    if(is_focus_active)
    {
        autocomplete_hint_string = ui_autocomplete_string();
    }

    //////////////////////
    // build top-level box
    //
    if(is_focus_active || is_focus_active_disabled)
    {
        ui_set_next_hover_cursor(WM_Cursor_IBar);
    }
    UI_Box *box = ui_build_box_from_key(UI_BoxFlag_MouseClickable|
                                        (!!build_lhs_name_desc                               * UI_BoxFlag_DisableFocusBorder)|
                                        (!!(params->flags & TI_CellFlag_KeyboardClickable)   * UI_BoxFlag_KeyboardClickable)|
                                        UI_BoxFlag_ClickToFocus|
                                        (!!(params->flags & TI_CellFlag_Button)              * UI_BoxFlag_DrawHotEffects)|
                                        (!!(params->flags & TI_CellFlag_SingleClickActivate) * UI_BoxFlag_DrawActiveEffects)|
                                        (!(params->flags & TI_CellFlag_NoBackground)        * UI_BoxFlag_DrawBackground)|
                                        (!!(params->flags & TI_CellFlag_Border)              * UI_BoxFlag_DrawBorder)|
                                        ((is_auto_focus_hot || is_auto_focus_active)         * UI_BoxFlag_KeyboardClickable)|
                                        (is_focus_active || is_focus_active_disabled)        * (UI_BoxFlag_Clip),
                                        key);

    ///////////////////////
    // build indent
    //
    UI_Parent(box) for(s32 idx = 0; idx < params->depth; idx += 1)
    {
        ui_set_next_flags(UI_BoxFlag_DrawSideLeft);
        ui_spacer(ui_em(1.f, 1.f));
    }

    ////////////////////////////
    // build expander (or placeholder, or space)
    //
    {
        // build expander
        if(params->flags & TI_CellFlag_Expander) UI_PrefWidth(ui_px(expander_size_px, 1.f)) UI_Parent(box)
            UI_Flags(UI_BoxFlag_DrawSideLeft)
            UI_Focus(UI_FocusKind_Off)
        {
            UI_Signal expander_sig = ui_expanderf(params->expanded_out[0], "expander");
            if(ui_pressed(expander_sig))
            {
                params->expanded_out[0] ^= 1;
            }
        }

        // build expander placeholder
        else if(params->flags & TI_CellFlag_ExpanderPlaceholder) UI_Parent(box) UI_PrefWidth(ui_px(expander_size_px, 1.f)) UI_Focus(UI_FocusKind_Off)
        {
            UI_TagF("weak")
                UI_Flags(UI_BoxFlag_DrawSideLeft)
                TI_Font(TI_FontSlot_Icons)
                UI_TextAlignment(UI_TextAlign_Center)
                ui_label(ti_icon_kind_text_table[TI_IconKind_Dot]);
        }

        // build expander space
        else if(params->flags & TI_CellFlag_ExpanderSpace) UI_Parent(box) UI_Focus(UI_FocusKind_Off)
        {
            UI_Flags(UI_BoxFlag_DrawSideLeft) ui_spacer(ui_px(expander_size_px, 1.f));
        }
    }

    /////////////////////
    // build lock
    //
    if(params->flags & TI_CellFlag_Lock && !is_focus_active && !is_focus_active_disabled) UI_Parent(box) UI_Focus(UI_FocusKind_Off)
    {
        // TODO: @hack
        {
            ui_spacer(ui_em(0.5f, 1.f));
        }
        bool32 is_locked = (params->lock_out && params->lock_out[0]);
        UI_PrefWidth(ui_em(2.f, 1.f))
            UI_TagF(".")
            UI_TagF(is_locked ? "pop" : "weak")
            UI_TagF("implicit")
            UI_Column
            UI_Padding(ui_pct(1, 0))
            UI_PrefHeight(ui_em(2.f, 1.f))
            UI_CornerRadius(ui_top_font_size()*0.5f)
            TI_Font(TI_FontSlot_Icons)
            UI_TextAlignment(UI_TextAlign_Center)
        {
            UI_Box *lock_box = ui_build_box_from_stringf(UI_BoxFlag_DrawText|
                                                         UI_BoxFlag_DrawHotEffects|
                                                         UI_BoxFlag_DrawBorder|
                                                         UI_BoxFlag_DrawBackground|
                                                         UI_BoxFlag_DisableFocusOverlay|
                                                         UI_BoxFlag_DisableFocusBorder|
                                                         UI_BoxFlag_Clickable,
                                                         "%S###lock", ti_icon_kind_text_table[is_locked ? TI_IconKind_Locked : TI_IconKind_Unlocked]);
            UI_Signal sig = ui_signal_from_box(lock_box);
            if(ui_hovering(sig)) UI_Tooltip TI_Font(TI_FontSlot_Main)
            {
                ui_state->tooltip_anchor_key = lock_box->key;
                UI_PrefWidth(ui_children_sum(1)) UI_Row
                {
                    UI_PrefWidth(ui_text_dim(10, 1)) ui_label(is_locked ? s("Unlock Location") : s("Lock Location"));
                    ti_cmd_binding_buttons(ti_cmd_kind_info_table[TI_CmdKind_ToggleLock].string, s(""), 1, TI_CmdBindingButtonFlag_NoEdit);
                }
            }
            if(ui_pressed(sig) && params->lock_out)
            {
                params->lock_out[0] ^= 1;
            }
        }
    }

    /////////////////////////////
    // build left-hande-side container box
    //
    UI_Box *lhs_box = &ui_nil_box;
    if(build_lhs_name_desc)
    {
        UI_Parent(box) UI_WidthFill UI_ChildLayoutAxis(Axis2_Y)
        {
            if(ui_top_text_alignment() == UI_TextAlign_Left && (params->flags & (TI_CellFlag_Expander|TI_CellFlag_ExpanderSpace|TI_CellFlag_ExpanderPlaceholder)) == 0)
            {
                ui_spacer(ui_em(1.f, 1.f));
            }
            lhs_box = ui_build_box_from_stringf(0, "lhs_box");
        }
    }

    ////////////////////////////
    // build left-hand-side name/desc box
    //
    if(build_lhs_name_desc) UI_Parent(lhs_box) UI_Padding(ui_em(3.f, 0.f)) UI_WidthFill UI_HeightFill
    {
        Fuzzy_Match_Range_List fuzzy_matches = {0};
        if(params->search_needle.size != 0)
        {
            fuzzy_matches = dr_fuzzy_match_find_from_fstrs(scratch.arena, &lhs_name_fstrs, params->search_needle);
        }
        UI_Row
        {
            UI_Box *name_box = ui_build_box_from_key(UI_BoxFlag_DrawText, ui_key_zero());
            ui_box_equip_display_fstrs(name_box, &lhs_name_fstrs);
            ui_box_equip_fuzzy_match_ranges(name_box, &fuzzy_matches);
        }
        if(params->description.size != 0) TI_Font(TI_FontSlot_Main) UI_FontSize(ui_top_font_size()*0.85f)
        {
            UI_Row
            {
                UI_Box *desc_box = ui_label(params->description).box;
                Fuzzy_Match_Range_List desc_fuzzy_matches = fuzzy_match_find(scratch.arena, params->search_needle, params->description);
                ui_box_equip_fuzzy_match_ranges(desc_box, &desc_fuzzy_matches);
            }
        }
    }

    ////////////////////////
    // build line edit container box
    //
    UI_Box *edit_box = &ui_nil_box;
    f32 editable_edit_box_dim = dim_2f32(box->rect).x;
    if((is_focus_active || is_focus_active_disabled) || build_line_edit)
        UI_Parent(box)
    {
        bool32 is_editing = (is_focus_active || is_focus_active_disabled);
        UI_Size edit_box_size = ui_pct(1, 0);
        if(build_lhs_name_desc)
        {
            if(is_editing)
            {
                editable_edit_box_dim = floor_f32(dim_2f32(box->rect).x*0.5f);
                edit_box_size = ui_px(editable_edit_box_dim, 1.f);
            }
            else
            {
                edit_box_size = ui_children_sum(1);
            }
        }
        UI_PrefWidth(edit_box_size)
        {
            if(ui_top_px_height() > ui_top_font_size()*3.f)
            {
                ui_set_next_pref_width(ui_children_sum(1));
                UI_Column UI_Padding(ui_em(1, 0)) UI_Focus(UI_FocusKind_On)
                {
                    UI_PrefHeight(ui_em(3.f, 1.f)) UI_CornerRadius(ui_top_font_size()*0.5f)
                        edit_box = ui_build_box_from_stringf((!!is_editing*UI_BoxFlag_DrawBorder)|
                                                             UI_BoxFlag_Clickable|
                                                             UI_BoxFlag_DisableFocusOverlay,
                                                             "edit_box");
                    if(params->line_edit_key_out)
                    {
                        params->line_edit_key_out[0] = edit_box->key;
                    }
                }
                if(ui_top_text_alignment() == UI_TextAlign_Left)
                {
                    ui_spacer(ui_em(1.f, 1.f));
                }
            }
            else
            {
                edit_box = ui_build_box_from_stringf(0, "edit_box");
                if(params->line_edit_key_out)
                {
                    params->line_edit_key_out[0] = edit_box->key;
                }
            }
        }
    }

    //////////////////////////
    // build edit-button, if line edit is embedded, and has no string
    //
    bool32 edit_started = 0;
    if(params->flags & TI_CellFlag_EmptyEditButton && !is_focus_active && !is_focus_active_disabled && build_lhs_name_desc && build_line_edit && value_name_fstrs.total_size == 0)
    {
        UI_TagF(".")
            UI_TagF("weak")
            UI_TagF("implicit")
            UI_Parent(box)
            UI_PrefWidth(ui_em(2.f, 1.f))
        {
            UI_Column
                UI_Padding(ui_pct(1, 0))
                UI_PrefHeight(ui_em(2.f, 1.f))
                UI_CornerRadius(ui_top_font_size()*0.5f)
                TI_Font(TI_FontSlot_Icons)
                UI_TextAlignment(UI_TextAlign_Center)
            {
                UI_Box *edit_start_box = ui_build_box_from_stringf(UI_BoxFlag_DrawText|
                                                                   UI_BoxFlag_DrawHotEffects|
                                                                   UI_BoxFlag_DrawBorder|
                                                                   UI_BoxFlag_DrawBackground|
                                                                   UI_BoxFlag_DisableFocusOverlay|
                                                                   UI_BoxFlag_DisableFocusBorder|
                                                                   UI_BoxFlag_Clickable,
                                                                   "%S##edit", ti_icon_kind_text_table[TI_IconKind_Pencil]);
                UI_Signal sig = ui_signal_from_box(edit_start_box);
                if(ui_pressed(sig))
                {
                    edit_started = 1;
                }
            }
            ui_spacer(ui_em(1.f, 1.f));
        }
    }

    ///////////////////////
    // build scrollable container box
    //
    UI_Box *scrollable_box = &ui_nil_box;
    if(edit_box != &ui_nil_box)
    {
        UI_Parent(edit_box) UI_PrefWidth(ui_children_sum(0))
        {
            scrollable_box = ui_build_box_from_stringf(is_focus_active*(UI_BoxFlag_AllowOverflowX|UI_BoxFlag_Clip), "scroll_box_%p", params->edit_buffer);
        }
    }

    ////////////////////
    // build revert-button
    //
    if(params->flags & TI_CellFlag_RevertButton && !is_focus_active && !is_focus_active_disabled)
    {
        UI_Parent(edit_box)
            UI_PrefWidth(ui_em(2.f, 1.f))
        {
            UI_TagF(".")
                UI_TagF("weak")
                UI_TagF("implicit")
                UI_Column
                UI_Padding(ui_pct(1, 0))
                UI_PrefHeight(ui_em(2.f, 1.f))
                UI_CornerRadius(ui_top_font_size()*0.5f)
                TI_Font(TI_FontSlot_Icons)
                UI_TextAlignment(UI_TextAlign_Center)
            {
                UI_Box *revert_box = ui_build_box_from_stringf(UI_BoxFlag_DrawText|
                                                               UI_BoxFlag_DrawHotEffects|
                                                               UI_BoxFlag_DrawBorder|
                                                               UI_BoxFlag_DrawBackground|
                                                               UI_BoxFlag_DisableFocusOverlay|
                                                               UI_BoxFlag_DisableFocusBorder|
                                                               UI_BoxFlag_Clickable,
                                                               "%S##revert", ti_icon_kind_text_table[TI_IconKind_Undo]);
                UI_Signal sig = ui_signal_from_box(revert_box);
                if(ui_hovering(sig)) UI_Tooltip TI_Font(TI_FontSlot_Main)
                {
                    ui_state->tooltip_anchor_key = revert_box->key;
                    ui_label(str8_lit("Revert To Default"));
                }
                if(ui_pressed(sig) && params->revert_out)
                {
                    params->revert_out[0] ^= 1;
                }
            }

            // TODO: @hack
            if (build_toggle_switch || build_slider)
            {
                ui_spacer(ui_em(1.f, 1.f));
            }
        }
    }

    ////////////////////
    // build browse button
    //
    if(params->flags & TI_CellFlag_BrowseButton && !is_focus_active && !is_focus_active_disabled)
    {
        UI_Parent(box)
            UI_PrefWidth(ui_children_sum(1))
            TI_Font(TI_FontSlot_Main)
        {
            UI_TagF(".")
                UI_TagF("weak")
                UI_Column
                UI_Padding(ui_pct(1, 0))
                UI_PrefHeight(ui_em(2.f, 1.f))
                UI_CornerRadius(ui_top_font_size()*0.5f)
                UI_TextAlignment(UI_TextAlign_Center)
                UI_PrefWidth(ui_text_dim(10, 1))
            {
                UI_Box *revert_box = ui_build_box_from_stringf(UI_BoxFlag_DrawText|
                                                               UI_BoxFlag_DrawHotEffects|
                                                               UI_BoxFlag_DrawActiveEffects|
                                                               UI_BoxFlag_DrawBorder|
                                                               UI_BoxFlag_DrawBackground|
                                                               UI_BoxFlag_DisableFocusOverlay|
                                                               UI_BoxFlag_DisableFocusBorder|
                                                               UI_BoxFlag_Clickable,
                                                               "Browse...");
                UI_Signal sig = ui_signal_from_box(revert_box);
                if(ui_clicked(sig) && params->browse_out)
                {
                    params->browse_out[0] = 1;
                }
            }
            ui_spacer(ui_em(1, 1));
        }
    }

    /////////////////////////
    // build toggle-switch
    //
    if(build_toggle_switch) UI_Parent(box)
    {
        bool32 is_toggled = !!params->toggled_out[0];
        f32 toggle_t = ui_anim(ui_key_from_stringf(key, "toggled"), (f32)is_toggled, .initial = (f32)is_toggled, .rate = ti_state->menu_animation_rate);
        f32 height_px = ceil_f32(ui_top_font_size() * 1.75f);
        f32 padding_px = ceil_f32((ui_top_px_height() - height_px) / 2.f);
        UI_PrefWidth(ui_children_sum(1.f))
            UI_HeightFill
            UI_Column UI_Padding(ui_px(padding_px, 1.f))
            UI_Row
        {
            if(ui_top_text_alignment() == UI_TextAlign_Center)
            {
                ui_spacer(ui_em(1.f, 0.f));
            }
            UI_PrefWidth(ui_em(3.5f, 1.f))
                UI_PrefHeight(ui_px(height_px, 1.f))
                UI_CornerRadius(floor_f32(height_px/2.f - 1.f))
                UI_TagF(is_toggled ? "good_pop" : "")
                UI_GroupKey(ui_key_from_stringf(ui_key_zero(), "toggle_switch_group_key"))
            {
                UI_Box *switch_box = ui_build_box_from_stringf(UI_BoxFlag_DrawHotEffects|UI_BoxFlag_DrawBorder|UI_BoxFlag_DrawBackground|UI_BoxFlag_Clickable, "toggle_switch");
                UI_Parent(switch_box)
                {
                    TI_Font(TI_FontSlot_Icons) UI_PrefWidth(ui_pct(toggle_t, 0)) UI_Transparency(1.f - toggle_t)
                    {
                        ui_build_box_from_stringf(UI_BoxFlag_DisableTextTrunc | (toggle_t > 0.001f ? UI_BoxFlag_DrawText : 0),
                                                  "%S", ti_icon_kind_text_table[TI_IconKind_Check]); 
                    }
                    UI_BackgroundColor(ui_color_from_name(str8_lit("text")))
                        UI_PrefWidth(ui_px(height_px, 1.f))
                    {
                        f32 extratoggler_padding_px = floor_f32(ui_top_font_size()*0.35f);
                        f32 toggler_size_px = ceil_f32(height_px - extratoggler_padding_px*2.f) - 1.f;
                        UI_Column UI_Padding(ui_px(extratoggler_padding_px, 1.f))
                            UI_Row UI_Padding(ui_px(extratoggler_padding_px, 1.f))
                            UI_PrefWidth(ui_px(toggler_size_px, 1.f))
                            UI_PrefHeight(ui_px(toggler_size_px, 1.f))
                            UI_CornerRadius(floor_f32(toggler_size_px/2.f - 1.f))
                        {
                            ui_build_box_from_key(UI_BoxFlag_DrawBackground|UI_BoxFlag_DrawDropShadow, ui_key_zero());
                        }
                    }
                    ui_spacer(ui_pct(1.f-toggle_t, 0));
                }
                UI_Signal switch_sig = ui_signal_from_box(switch_box);
                
                // rjf: press -> toggle, & gather this key
                if(ui_pressed(switch_sig))
                {
                    if(ui_dragging(switch_sig))
                    {
                        ui_store_drag_struct(&switch_box->key);
                    }
                    params->toggled_out[0] ^= 1;
                }
                
                // rjf: dragging -> check if key is in batch of touched keys. if so, do nothing, otherwise, toggle.
                // always store this new key if not in batch
                if(ui_dragging(switch_sig))
                {
                    String8 all_keys_data = ui_get_drag_data(sizeof(UI_Key));
                    UI_Key *keys = (UI_Key *)all_keys_data.str;
                    u64 keys_count = all_keys_data.size / sizeof(UI_Key);
                    bool32 key_is_touched = 0;
                    for EachIndex(idx, keys_count)
                    {
                        if(ui_key_match(keys[idx], switch_box->key))
                        {
                            key_is_touched = 1;
                            break;
                        }
                    }
                    if(!key_is_touched)
                    {
                        params->toggled_out[0] ^= 1;
                        UI_Key *new_keys = push_array(scratch.arena, UI_Key, keys_count+1);
                        MemoryCopy(new_keys, keys, sizeof(UI_Key)*keys_count);
                        new_keys[keys_count] = switch_box->key;
                        ui_store_drag_data(str8((u8 *)new_keys, sizeof(UI_Key) * (keys_count+1)));
                    }
                }
            }
            if(ui_top_text_alignment() == UI_TextAlign_Center)
            {
                ui_spacer(ui_em(1.f, 0.f));
            }
        }
        if(ui_top_text_alignment() == UI_TextAlign_Left)
        {
            ui_spacer(ui_em(1.f, 1.f));
        }
    }

    //////////////////////
    // build slider
    //
    if(build_slider) UI_Parent(box)
    {
        f32 height_px = ceil_f32(ui_top_font_size() * 1.75f);
        f32 padding_px = ceil_f32((ui_top_px_height() - height_px) / 2.f);
        UI_PrefWidth(ui_children_sum(1.f))
            UI_HeightFill
            UI_Column UI_Padding(ui_px(padding_px, 1.f))
            UI_Row
            UI_PrefWidth(ui_pct(0.5f - 0.2f*(!!build_lhs_name_desc), 0.f))
            UI_PrefHeight(ui_px(height_px, 1.f))
            UI_CornerRadius(floor_f32(height_px/2.f - 1.f))
        {
            f32 extratoggler_padding_px = floor_f32(ui_top_font_size()*0.35f);
            f32 toggler_size_px = ceil_f32(height_px - extratoggler_padding_px*2.f) - 1.f;
            ui_set_next_hover_cursor(WM_Cursor_LeftRight);
            UI_Box *slider_box = ui_build_box_from_stringf(UI_BoxFlag_DrawHotEffects|UI_BoxFlag_DrawBorder|UI_BoxFlag_DrawBackground|UI_BoxFlag_Clickable, "slider");
            UI_Parent(slider_box) UI_TagF("pop")
            {
                UI_Signal sig = ui_signal_from_box(slider_box);
                if(ui_dragging(sig))
                {
                    if(ui_pressed(sig))
                    {
                        ui_store_drag_struct(params->slider_value_out);
                    }
                    f32 draggable_region_size_px = dim_2f32(slider_box->rect).x - (extratoggler_padding_px*2 + toggler_size_px);
                    f32 initial_pct = *ui_get_drag_struct(f32);
                    f32 current_pct = initial_pct + (ui_drag_delta().x / draggable_region_size_px);
                    params->slider_value_out[0] = current_pct;
                }
                
                UI_Box *fill_box = &ui_nil_box;
                UI_PrefWidth(ui_children_sum(0))
                    UI_MinWidth(toggler_size_px + extratoggler_padding_px*2)
                    fill_box = ui_build_box_from_key(UI_BoxFlag_DrawBackground|UI_BoxFlag_DrawBorder, ui_key_zero());
                UI_Parent(fill_box)
                {
                    ui_spacer(ui_pct(Clamp(0, params->slider_value_out[0], 1), 0.f));
                    UI_BackgroundColor(ui_color_from_name(str8_lit("text")))
                        UI_PrefWidth(ui_px(height_px, 1.f))
                    {
                        UI_Column UI_Padding(ui_px(extratoggler_padding_px, 1.f))
                            UI_Row UI_Padding(ui_px(extratoggler_padding_px, 1.f))
                            UI_PrefWidth(ui_px(toggler_size_px, 1.f))
                            UI_PrefHeight(ui_px(toggler_size_px, 1.f))
                            UI_CornerRadius(floor_f32(toggler_size_px/2.f - 1.f))
                        {
                            ui_build_box_from_key(UI_BoxFlag_DrawBackground|UI_BoxFlag_DrawDropShadow, ui_key_zero());
                        }
                    }
                }
                ui_spacer(ui_pct(1-Clamp(0, params->slider_value_out[0], 1), 0.f));
            }
        }
        ui_spacer(ui_em(1.f, 1.f));
    }
  
    //////////////////////////////
    //- rjf: build bindings
    //
    if(build_bindings) UI_Parent(box) TI_Font(TI_FontSlot_Main) UI_PermissionFlags(UI_PermissionFlag_Clicks)
    {
        UI_PrefWidth(ui_children_sum(1)) UI_Column UI_Padding(ui_px(ui_top_px_height()*0.2f, 1.f)) UI_HeightFill
        {
            UI_PrefWidth(ui_children_sum(1)) UI_Row UI_Padding(ui_em(1.f, 1.f))
            {
                ti_cmd_binding_buttons(params->bindings_name, params->search_needle, max_u64, TI_CmdBindingButtonFlag_AddNew);
            }
        }
    }
    
    //////////////////////////////
    //- rjf: build notes
    //
    if(build_note) UI_Parent(box) UI_PrefWidth(params->note_width)
    {
        UI_Box *note_box = ui_build_box_from_key(UI_BoxFlag_DrawText, ui_key_zero());
        ui_box_equip_display_fstrs(note_box, &note_fstrs);
    }
    
    //////////////////////////////
    //- rjf: do non-textual edits (delete, copy, cut)
    //
    bool32 commit = 0;
    if(!is_focus_active && is_focus_hot)
    {
        for(UI_Event *evt = 0; ui_next_event(&evt);)
        {
            if(evt->flags & UI_EventFlag_Copy)
            {
                wm_set_clipboard_text(params->pre_edit_value);
            }
            if(evt->flags & UI_EventFlag_Delete)
            {
                commit = 1;
                params->edit_string_size_out[0] = 0;
            }
        }
    }
    
    //////////////////////////////
    //- rjf: get signal
    //
    UI_Signal sig = ui_signal_from_box(box);
    if(commit)
    {
        sig.f |= UI_SignalFlag_Commit;
    }
    
    //////////////////////////////
    //- rjf: do start/end editing interaction
    //
    bool32 focus_started = 0;
    if(!is_focus_active)
    {
        bool32 start_editing_via_sig = (ui_double_clicked(sig) || sig.f&UI_SignalFlag_KeyboardPressed);
        bool32 start_editing_via_typing = 0;
        if(is_focus_hot)
        {
            for(UI_Event *evt = 0; ui_next_event(&evt);)
            {
                if(evt->string.size != 0 || evt->flags & UI_EventFlag_Paste)
                {
                    start_editing_via_typing = 1;
                    break;
                }
            }
        }
        if(is_focus_hot && ui_slot_press(UI_EventActionSlot_Edit))
        {
            start_editing_via_typing = 1;
        }
        if(start_editing_via_sig || start_editing_via_typing)
        {
            String8 edit_string = params->pre_edit_value;
            edit_string.size = Min(params->edit_buffer_size, params->pre_edit_value.size);
            MemoryCopy(params->edit_buffer, edit_string.str, edit_string.size);
            params->edit_string_size_out[0] = edit_string.size;
            ui_set_auto_focus_active_key(key);
            if(!(params->flags & TI_CellFlag_Button))
            {
                ui_kill_action();
            }
            params->cursor[0] = edit_string.size;
            params->mark[0] = 0;
            focus_started = 1;
        }
    }
    else if(is_focus_active && sig.f&UI_SignalFlag_KeyboardPressed)
    {
        ui_set_auto_focus_active_key(ui_key_zero());
        sig.f |= UI_SignalFlag_Commit;
    }
    
    //////////////////////////////
    //- rjf: take navigation actions for editing
    //
    bool32 changes_made = 0;
    if(!(params->flags & TI_CellFlag_DisableEdit) && (is_focus_active || focus_started))
    {
        Temp scratch = scratch_begin(0, 0);
        ti_state->text_edit_mode = 1;
        for(UI_Event *evt = 0; ui_next_event(&evt);)
        {
            String8 edit_string = str8(params->edit_buffer, params->edit_string_size_out[0]);
            
            // rjf: do not consume anything that doesn't fit a single-line's operations
            bool32 is_autocompletion_completion = (autocomplete_hint_string.size != 0 &&
                                                evt->kind == UI_EventKind_Press &&
                                                evt->slot == UI_EventActionSlot_Accept);
            if(!is_autocompletion_completion &&
               ((evt->kind != UI_EventKind_Edit &&
                 evt->kind != UI_EventKind_Navigate &&
                 evt->kind != UI_EventKind_Text) ||
                evt->delta_2s32.y != 0))
            {
                continue;
            }
            
            // rjf: map this action to an op
            UI_Txt_Op op = ui_single_line_txt_op_from_event(scratch.arena, evt, edit_string, r1u64(0, edit_string.size), params->cursor[0], params->mark[0]);
            
            // rjf: any valid *additive* op & autocomplete hint? -> perform autocomplete first, then re-compute op
            if(!(evt->flags & UI_EventFlag_Delete) && autocomplete_hint_string.size != 0)
            {
                CFG_Node *window = cfg_node_from_id(ti_regs()->window);
                TI_Window_State *ws = ti_window_state_from_cfg(window);
                TI_Autocomp_Cursor_Info *autocomp_cursor_info = &ws->autocomp_cursor_info;
                String8 new_string = ui_push_string_replace_range(scratch.arena, edit_string, autocomp_cursor_info->replaced_range, autocomplete_hint_string);
                new_string.size = Min(params->edit_buffer_size, new_string.size);
                MemoryCopy(params->edit_buffer, new_string.str, new_string.size);
                params->edit_string_size_out[0] = new_string.size;
                params->cursor[0] = params->mark[0] = autocomp_cursor_info->replaced_range.min+autocomplete_hint_string.size;
                edit_string = str8(params->edit_buffer, params->edit_string_size_out[0]);
                op = ui_single_line_txt_op_from_event(scratch.arena, evt, edit_string, r1u64(0, edit_string.size), params->cursor[0], params->mark[0]);
                MemoryZeroStruct(&autocomplete_hint_string);
            }
            
            // rjf: perform replace range
            if(op.range.min != op.range.max || op.replace.size != 0)
            {
                String8 new_string = ui_push_string_replace_range(scratch.arena, edit_string, op.range, op.replace);
                new_string.size = Min(params->edit_buffer_size, new_string.size);
                MemoryCopy(params->edit_buffer, new_string.str, new_string.size);
                params->edit_string_size_out[0] = new_string.size;
            }
            
            // rjf: perform copy
            if(evt->flags & UI_EventFlag_Copy)
            {
                wm_set_clipboard_text(op.copy);
            }
            
            // rjf: commit op's changed cursor & mark to caller-provided state
            params->cursor[0] = op.cursor;
            params->mark[0] = op.mark;
            
            // rjf: consume event
            {
                if(!is_autocompletion_completion)
                {
                    ui_eat_event(evt);
                }
                changes_made = 1;
            }
        }
        scratch_end(scratch);
    }
    
    //////////////////////////////
    //- rjf: click-driven "start editing"
    //
    if(edit_started)
    {
        sig.f |= UI_SignalFlag_DoubleClicked;
    }
    
    //////////////////////////////
    //- rjf: compute editable fancy strings
    //
    DR_FStr_List fstrs = {0};
    {
        //- rjf: (not editing)
        if(!is_focus_active && !is_focus_active_disabled && value_name_fstrs.total_size != 0)
        {
            fstrs = value_name_fstrs;
        }
        else if(!is_focus_active && !is_focus_active_disabled && params->flags & TI_CellFlag_CodeContents && params->pre_edit_value.size != 0)
        {
            String8 display_string = params->pre_edit_value;
            fstrs = ti_fstrs_from_code_string(scratch.arena, 1, 0, ui_color_from_name(str8_lit("text")), display_string);
        }
        else if(!is_focus_active && !is_focus_active_disabled)
        {
            String8 display_string = params->pre_edit_value;
            if(params->pre_edit_value.size == 0)
            {
                display_string = ui_display_part_from_key_string(string);
            }
            UI_TagF("weak")
            {
                DR_FStr_Params params = {ui_top_font(), ui_top_text_raster_flags(), ui_color_from_name(str8_lit("text")), ui_top_font_size()};
                dr_fstrs_push_new(scratch.arena, &fstrs, &params, display_string);
            }
        }
        
        //- rjf: (editing)
        else if(is_focus_active || is_focus_active_disabled)
        {
            String8 edit_string = str8(params->edit_buffer, params->edit_string_size_out[0]);
            DR_FStr_List edit_string_fstrs = {0};
            if(params->flags & TI_CellFlag_CodeContents)
            {
                edit_string_fstrs = ti_fstrs_from_code_string(scratch.arena, 1.f, 0, ui_color_from_name(str8_lit("text")), edit_string);
            }
            else
            {
                String8 edit_string = str8(params->edit_buffer, params->edit_string_size_out[0]);
                DR_FStr_Params params = {ui_top_font(), ui_top_text_raster_flags(), ui_color_from_name(str8_lit("text")), ui_top_font_size()};
                dr_fstrs_push_new(scratch.arena, &edit_string_fstrs, &params, edit_string);
            }
            if(autocomplete_hint_string.size != 0)
            {
                CFG_Node *window = cfg_node_from_id(ti_regs()->window);
                TI_Window_State *ws = ti_window_state_from_cfg(window);
                TI_Autocomp_Cursor_Info *autocomp_cursor_info = &ws->autocomp_cursor_info;
                String8 autocomplete_append_string = str8_skip(autocomplete_hint_string, params->cursor[0] - autocomp_cursor_info->replaced_range.min);
                u64 off = 0;
                u64 cursor_off = params->cursor[0];
                DR_FStr_Node *prev_n = 0;
                for(DR_FStr_Node *n = edit_string_fstrs.first; n != 0; n = n->next)
                {
                    if(off <= cursor_off && cursor_off <= off+n->v.string.size)
                    {
                        prev_n = n;
                        break;
                    }
                    off += n->v.string.size;
                }
                {
                    DR_FStr_Node *autocomp_fstr_n = push_array(scratch.arena, DR_FStr_Node, 1);
                    DR_FStr *fstr = &autocomp_fstr_n->v;
                    fstr->string = autocomplete_append_string;
                    fstr->params.font = ui_top_font();
                    fstr->params.raster_flags = ui_top_text_raster_flags();
                    fstr->params.color = ui_color_from_name(str8_lit("text"));
                    fstr->params.color.w *= 0.5f;
                    fstr->params.size = ui_top_font_size();
                    autocomp_fstr_n->next = prev_n ? prev_n->next : 0;
                    if(prev_n != 0)
                    {
                        prev_n->next = autocomp_fstr_n;
                    }
                    if(prev_n == 0)
                    {
                        edit_string_fstrs.first = edit_string_fstrs.last = autocomp_fstr_n;
                    }
                    if(prev_n != 0 && prev_n->next == 0)
                    {
                        edit_string_fstrs.last = autocomp_fstr_n;
                    }
                    edit_string_fstrs.node_count += 1;
                    edit_string_fstrs.total_size += autocomplete_hint_string.size;
                    if(prev_n != 0 && cursor_off - off < prev_n->v.string.size)
                    {
                        String8 full_string = prev_n->v.string;
                        u64 chop_amt = full_string.size - (cursor_off - off);
                        prev_n->v.string = str8_chop(full_string, chop_amt);
                        edit_string_fstrs.total_size -= chop_amt;
                        if(chop_amt != 0)
                        {
                            String8 post_cursor = str8_skip(full_string, cursor_off - off);
                            DR_FStr_Node *post_fstr_n = push_array(scratch.arena, DR_FStr_Node, 1);
                            DR_FStr *post_fstr = &post_fstr_n->v;
                            MemoryCopyStruct(post_fstr, &prev_n->v);
                            post_fstr->string   = post_cursor;
                            if(autocomp_fstr_n->next == 0)
                            {
                                edit_string_fstrs.last = post_fstr_n;
                            }
                            post_fstr_n->next = autocomp_fstr_n->next;
                            autocomp_fstr_n->next = post_fstr_n;
                            edit_string_fstrs.node_count += 1;
                            edit_string_fstrs.total_size += post_cursor.size;
                        }
                    }
                }
            }
            fstrs = edit_string_fstrs;
        }
    }
    
    //////////////////////////////
    //- rjf: build scrolled contents
    //
    u64 mouse_off = {0};
    f32 cursor_off = 0;
    if(scrollable_box != &ui_nil_box) UI_Parent(scrollable_box)
    {
        Fuzzy_Match_Range_List fuzzy_matches = {0};
        if(params->search_needle.size != 0)
        {
            fuzzy_matches = dr_fuzzy_match_find_from_fstrs(scratch.arena, &fstrs, params->search_needle);
        }
        if(ui_top_text_alignment() == UI_TextAlign_Left && (params->flags & (TI_CellFlag_Expander|TI_CellFlag_ExpanderSpace|TI_CellFlag_ExpanderPlaceholder)) == 0)
        {
            ui_spacer(ui_em(0.5f, 1.f));
        }
        if(is_focus_active)
        {
            ui_set_next_flags(UI_BoxFlag_DisableTextTrunc);
        }
        ui_set_next_pref_width(ui_text_dim(ui_top_font_size()*0.5f, 0));
        UI_Box *text_box = ui_build_box_from_stringf(UI_BoxFlag_DrawText, "###text_box");
        ui_box_equip_display_fstrs(text_box, &fstrs);
        ui_box_equip_fuzzy_match_ranges(text_box, &fuzzy_matches);
        if(is_focus_active || is_focus_active_disabled)
        {
            String8 edit_string = str8(params->edit_buffer, params->edit_string_size_out[0]);
            UI_Line_Edit_Draw_Data *draw_data = push_array(ui_build_arena(), UI_Line_Edit_Draw_Data, 1);
            draw_data->edited_string = push_str8_copy(ui_build_arena(), edit_string);
            draw_data->cursor = params->cursor[0];
            draw_data->mark = params->mark[0];
            draw_data->trail = do_cursor_trail && !ui_dragging(sig);
            ui_box_equip_custom_draw(text_box, ui_line_edit_draw, draw_data);
            Vec2f32 text2mouse = sub_2f32(ui_mouse(), ui_box_text_position(text_box));
            FNT_Tag font = ui_top_font();
            f32 font_size = ui_top_font_size();
            if(params->flags & TI_CellFlag_CodeContents)
            {
                font = ti_font_from_slot(TI_FontSlot_Code);
            }
            mouse_off = fnt_char_pos_from_tag_size_string_p(font, font_size, 0, ui_top_tab_size(), edit_string, text2mouse.x);
            cursor_off = fnt_dim_from_tag_size_string(ui_top_font(), ui_top_font_size(), 0, ui_top_tab_size(), str8_prefix(edit_string, params->cursor[0])).x;
        }
    }
    
    //////////////////////////////
    //- rjf: click+drag
    //
    if(is_focus_active && ui_dragging(sig))
    {
        if(ui_pressed(sig))
        {
            params->mark[0] = mouse_off;
        }
        params->cursor[0] = mouse_off;
    }
    if(!is_focus_active && is_focus_active_disabled && ui_pressed(sig))
    {
        params->cursor[0] = params->mark[0] = mouse_off;
    }
    
    //////////////////////////////
    //- rjf: focus cursor
    //
    if(scrollable_box != &ui_nil_box)
    {
        f32 edit_box_dim_px = editable_edit_box_dim;
        f32 visible_dim_px = edit_box_dim_px - expander_size_px - ui_top_font_size()*params->depth;
        if(visible_dim_px > 0)
        {
            Rng1f32 cursor_range_px  = r1f32(cursor_off-ui_top_font_size()*2.f, cursor_off+ui_top_font_size()*1.f);
            Rng1f32 visible_range_px = r1f32(scrollable_box->view_off_target.x, scrollable_box->view_off_target.x + visible_dim_px);
            cursor_range_px.min = ClampBot(0, cursor_range_px.min);
            cursor_range_px.max = ClampBot(0, cursor_range_px.max);
            f32 min_delta = cursor_range_px.min-visible_range_px.min;
            f32 max_delta = cursor_range_px.max-visible_range_px.max;
            min_delta = Min(min_delta, 0);
            max_delta = Max(max_delta, 0);
            scrollable_box->view_off_target.x += min_delta;
            scrollable_box->view_off_target.x += max_delta;
        }
        if(!is_focus_active && !is_focus_active_disabled)
        {
            scrollable_box->view_off_target.x = scrollable_box->view_off.x = 0;
        }
    }
    
    //////////////////////////////
    //- rjf: pop focus
    //
    if(is_auto_focus_hot) { ui_pop_focus_hot(); }
    if(is_auto_focus_active) { ui_pop_focus_active(); }
    
    ProfEnd();
    scratch_end(scratch);
    return sig;
}

internal UI_Signal ti_cellf(TI_Cell_Params *params, char *fmt, ...)
{
    Temp scratch = scratch_begin(0, 0);
    va_list args;
    va_start(args, fmt);
    String8 string = push_str8fv(scratch.arena, fmt, args);
    va_end(args);
    UI_Signal sig = ti_cell(params, string);
    scratch_end(scratch);
    return sig;
}
