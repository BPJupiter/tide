
//////////////////////////////
// UI Widgets: Fancy Title Strings

internal DR_FStr_List bp_title_fstrs_from_cfg(Arena *arena, CFG_Node *cfg, bool32 include_extras)
{
    DR_FStr_List result = {0};
    {
        Temp scratch = scratch_begin(&arena, 1);
        
        //- rjf: unpack config
        //
        bool32 is_disabled = bp_disabled_from_cfg(cfg);
        String8 name_string = bp_name_from_cfg(cfg);
        String8 label_string = bp_label_from_cfg(cfg);
        String8 file_path = bp_path_from_cfg(cfg);
        Vec4f32 rgba = bp_color_from_cfg(cfg);
        if(rgba.w == 0)
        {
            rgba = ui_color_from_name(str8_lit("text"));
        }
        Vec4f32 rgba_secondary = rgba;
        UI_TagF("weak")
        {
            rgba_secondary = ui_color_from_name(str8_lit("text"));
        }
        BP_IconKind icon_kind = bp_icon_kind_from_code_name(cfg->string);
        bool32 is_from_command_line = 0;
        {
            CFG_Node *cmd_line_root = cfg_node_child_from_string(cfg_node_root(), str8_lit("command_line"));
            for(CFG_Node *p = cfg->parent; p != &cfg_nil_node; p = p->parent)
            {
                if(p == cmd_line_root)
                {
                    is_from_command_line = 1;
                    break;
                }
      }
        }
        bool32 is_within_window = 0;
        {
            for(CFG_Node *p = cfg->parent; p != &cfg_nil_node; p = p->parent)
            {
                if(str8_match(p->string, str8_lit("window"), 0))
                {
                    is_within_window = 1;
                    break;
                }
            }
        }
        if(file_path.size != 0)
        {
            icon_kind = BP_IconKind_FileOutline;
        }
        
        //- rjf: set up color/size for all parts of the title
        //
        DR_FStr_Params params = {bp_font_from_slot(BP_FontSlot_Main), bp_raster_flags_from_slot(BP_FontSlot_Main), rgba, ui_top_font_size()};
        bool32 running_is_secondary = 0;
#define start_secondary() if(!running_is_secondary){running_is_secondary = 1; params.color = rgba_secondary; params.size = ui_top_font_size()*0.95f;}
        
        //- rjf: disabled? -> soften color
        if(is_disabled)
        {
            params.color = rgba_secondary;
        }
        
        //- rjf: push icon
        if(icon_kind != BP_IconKind_Null)
        {
            dr_fstrs_push_new(arena, &result, &params, bp_icon_kind_text_table[icon_kind], .font = bp_font_from_slot(BP_FontSlot_Icons), .raster_flags = bp_raster_flags_from_slot(BP_FontSlot_Icons), .color = rgba_secondary);
            dr_fstrs_push_new(arena, &result, &params, str8_lit("  "));
        }
        
        //- rjf: push warning icon for command-line entities
        if(is_from_command_line)
        {
            dr_fstrs_push_new(arena, &result, &params, bp_icon_kind_text_table[BP_IconKind_Info], .font = bp_font_from_slot(BP_FontSlot_Icons), .raster_flags = bp_raster_flags_from_slot(BP_FontSlot_Icons), .color = rgba_secondary);
            dr_fstrs_push_new(arena, &result, &params, str8_lit("  "));
        }
        
        //- rjf: push view title, if from window, and no file path, and no label
        if(is_within_window && file_path.size == 0 && label_string.size == 0)
        {
            String8 view_display_name = bp_display_from_code_name(cfg->string);
            if(view_display_name.size != 0)
            {
                dr_fstrs_push_new(arena, &result, &params, view_display_name);
                dr_fstrs_push_new(arena, &result, &params, str8_lit("  "));
                start_secondary();
            }
        }
        
        //- rjf: push bucket name
        if(cfg->parent == cfg_node_root())
        {
            if(str8_match(cfg->string, str8_lit("user"), 0))
            {
                dr_fstrs_push_new(arena, &result, &params, str8_lit("User"), .font = bp_font_from_slot(BP_FontSlot_Main), .raster_flags = bp_raster_flags_from_slot(BP_FontSlot_Main));
                dr_fstrs_push_new(arena, &result, &params, str8_lit("  "));
                start_secondary();
            }
            else if(str8_match(cfg->string, str8_lit("project"), 0))
            {
                dr_fstrs_push_new(arena, &result, &params, str8_lit("Project"), .font = bp_font_from_slot(BP_FontSlot_Main), .raster_flags = bp_raster_flags_from_slot(BP_FontSlot_Main));
                dr_fstrs_push_new(arena, &result, &params, str8_lit("  "));
                start_secondary();
            }
        }
        
        //- rjf: push name
        if(name_string.size != 0)
        {
            dr_fstrs_push_new(arena, &result, &params, name_string);
            dr_fstrs_push_new(arena, &result, &params, str8_lit("  "));
            start_secondary();
        }
        
        //- rjf: push label
        if(label_string.size != 0)
        {
            dr_fstrs_push_new(arena, &result, &params, label_string, .font = bp_font_from_slot(BP_FontSlot_Code), .raster_flags = bp_raster_flags_from_slot(BP_FontSlot_Code));
            dr_fstrs_push_new(arena, &result, &params, str8_lit("  "));
            start_secondary();
        }
        
        //- rjf: query is file path - do specific file name strings
        //
        if(file_path.size != 0)
        {
            String8_List qualifiers = {0};
            String8 file_name = str8_skip_last_slash(file_path);
            if(bp_state->ambiguous_path_slots_count != 0)
            {
                u64 hash = u64_hash_from_str8__case_insensitive(file_name);
                u64 slot_idx = hash%bp_state->ambiguous_path_slots_count;
                BP_Ambiguous_Path_Node *node = 0;
                {
                    for(BP_Ambiguous_Path_Node *n = bp_state->ambiguous_path_slots[slot_idx];
                        n != 0;
                        n = n->next)
                    {
                        if(str8_match(n->name, file_name, StringMatchFlag_CaseInsensitive))
                        {
                            node = n;
                            break;
                        }
                    }
                }
                if(node != 0 && node->paths.node_count > 1)
                {
                    String8_Array collisions = str8_array_from_list(scratch.arena, &node->paths);
                    String8_List *collision_parts_reversed = push_array(scratch.arena, String8_List, collisions.count);
                    for EachIndex(idx, collisions.count)
                    {
                        String8_List parts = str8_split_path(scratch.arena, collisions.v[idx]);
                        for(String8_Node *n = parts.first; n != 0; n = n->next)
                        {
                            str8_list_push_front(scratch.arena, &collision_parts_reversed[idx], n->string);
                        }
                    }
                    String8_List parts = str8_split_path(scratch.arena, file_path);
                    String8_List parts_reversed = {0};
                    for(String8_Node *n = parts.first; n != 0; n = n->next)
                    {
                        str8_list_push_front(scratch.arena, &parts_reversed, n->string);
                    }
                    {
                        u64 num_collisions_left = collisions.count;
                        String8_Node **collision_nodes = push_array(scratch.arena, String8_Node *, collisions.count);
                        for EachIndex(idx, collisions.count)
                        {
                            collision_nodes[idx] = collision_parts_reversed[idx].first;
                        }
                        for(String8_Node *n = parts_reversed.first; num_collisions_left > 1 && n != 0; n = n->next)
                        {
                            bool32 part_is_qualifier = 0;
                            for EachIndex(idx, collisions.count)
                            {
                                if(collision_nodes[idx] != 0 && !str8_match(collision_nodes[idx]->string, n->string, StringMatchFlag_CaseInsensitive))
                                {
                                    collision_nodes[idx] = 0;
                                    num_collisions_left -= 1;
                                    part_is_qualifier = 1;
                                }
                                else if(collision_nodes[idx] != 0)
                                {
                                    collision_nodes[idx] = collision_nodes[idx]->next;
                                }
                            }
                            if(part_is_qualifier)
                            {
                                str8_list_push_front(scratch.arena, &qualifiers, n->string);
                            }
                        }
                    }
                }
            }
            
            if(qualifiers.node_count != 0) UI_TagF("weak")
            {
                for(String8_Node *n = qualifiers.first; n != 0; n = n->next)
                {
                    String8 string = push_str8f(arena, "<%S> ", n->string);
                    dr_fstrs_push_new(arena, &result, &params, string, .color = ui_color_from_name(str8_lit("text")));
                }
            }
            
            dr_fstrs_push_new(arena, &result, &params, push_str8_copy(arena, str8_skip_last_slash(file_path)));
            dr_fstrs_push_new(arena, &result, &params, str8_lit("  "));
            start_secondary();
        }
        
        //- rjf: push disabled marker
        if(is_disabled)
        {
            dr_fstrs_push_new(arena, &result, &params, str8_lit("(Disabled)"));
            dr_fstrs_push_new(arena, &result, &params, str8_lit("  "));
        }
        
        //- rjf: special case: colors
        //
        if(str8_match(cfg->string, str8_lit("theme_color"), 0))
        {
            String8 tags = cfg_node_child_from_string(cfg, str8_lit("tags"))->first->string;
            String8 color_string = cfg_node_child_from_string(cfg, str8_lit("value"))->first->string;
            u64 color_u64 = 0;
            try_u64_from_str8_c_rules(color_string, &color_u64);
            u32 color_u32 = (u32)color_u64;
            Vec4f32 color = linear_from_srgba(rgba_from_u32(color_u32));
            if(tags.size != 0)
            {
                dr_fstrs_push_new(arena, &result, &params, tags);
            }
            else
            {
                dr_fstrs_push_new(arena, &result, &params, str8_lit("Color"), .color = rgba_secondary);
            }
            dr_fstrs_push_new(arena, &result, &params, str8_lit("  "));
            dr_fstrs_push_new(arena, &result, &params, bp_icon_kind_text_table[BP_IconKind_CircleFilled], .font = bp_font_from_slot(BP_FontSlot_Icons), .raster_flags = bp_raster_flags_from_slot(BP_FontSlot_Icons), .color = color);
        }
        
#undef start_secondary
        scratch_end(scratch);
    }
  return result;
}

internal DR_FStr_List bp_title_fstrs_from_code_name(Arena *arena, String8 code_name)
{
    DR_FStr_List result = {0};
    {
        BP_Vocab_Info *info = bp_vocab_info_from_code_name(code_name);

        // set up color/size for all parts of the title
        //
        // the "running" part implies that it changes as things are added -
        // so if a primary title is pushed, we can make the rest of the title
        // more faded/smaller, but only after a primary title is pushed,
        // which could be caused by many different potential parts of a cfg.
        //
        DR_FStr_Params params = {bp_font_from_slot(BP_FontSlot_Main),
                                 bp_raster_flags_from_slot(BP_FontSlot_Main),
                                 ui_color_from_name(str8_lit("text")),
                                 ui_top_font_size()};

        // push icon
        if (info->icon_kind != BP_IconKind_Null) UI_Tag(str8_lit("weak"))
        {
            dr_fstrs_push_new(arena, &result, &params, bp_icon_kind_text_table[info->icon_kind],
                              .font = bp_font_from_slot(BP_FontSlot_Icons),
                              .raster_flags = bp_raster_flags_from_slot(BP_FontSlot_Icons),
                              .color = ui_color_from_name(str8_lit("text")));
            dr_fstrs_push_new(arena, &result, &params, str8_lit("  "));
        }

        // push display name
        if (info->display_name.size != 0)
        {
            dr_fstrs_push_new(arena, &result, &params, info->display_name);
        }

        // push code name as a fallback
        else
        {
            dr_fstrs_push_new(arena, &result, &params, code_name,
                              .font = bp_font_from_slot(BP_FontSlot_Code),
                              .raster_flags = bp_raster_flags_from_slot(BP_FontSlot_Code));
        }
    }
    return result;
}

internal DR_FStr_List bp_title_fstrs_from_file_path(Arena *arena, String8 file_path, bool32 include_folder)
{
    DR_FStr_List fstrs = {0};
    String8 file_name = str8_skip_last_slash(file_path);
    File_Properties props = properties_from_file_path(file_path);
    BP_IconKind icon_kind = BP_IconKind_FileOutline;
    if (props.flags & FilePropertyFlag_IsFolder)
    {
        icon_kind = BP_IconKind_FolderClosedFilled;
    }
    if (file_path.size == 0 || str8_match(file_path, str8_lit("/"), StringMatchFlag_SlashInsensitive))
    {
        icon_kind = BP_IconKind_Machine;
        file_name = str8_lit("File System");
    }
    DR_FStr_Params params = {bp_font_from_slot(BP_FontSlot_Main),
                             bp_raster_flags_from_slot(BP_FontSlot_Main),
                             ui_color_from_name(str8_lit("text")),
                             ui_top_font_size()};
    UI_TagF("weak")
    {
        dr_fstrs_push_new(arena, &fstrs, &params,
                          bp_icon_kind_text_table[icon_kind],
                          .font = bp_font_from_slot(BP_FontSlot_Icons),
                          .raster_flags = bp_raster_flags_from_slot(BP_FontSlot_Icons),
                          .color = ui_color_from_name(str8_lit("text")));
    }
    dr_fstrs_push_new(arena, &fstrs, &params, str8_lit("  "));
    dr_fstrs_push_new(arena, &fstrs, &params, file_name);
    if (include_folder)
    {
        dr_fstrs_push_new(arena, &fstrs, &params, str8_lit("  "));
        UI_TagF("weak")
        {
            dr_fstrs_push_new(arena, &fstrs, &params,
                              str8_chop_last_slash(file_path),
                              .size = params.size * 0.9f,
                              .color = ui_color_from_name(str8_lit("text")));
        }
    }
    return fstrs;
}

//////////////////////////
// UI Widgets: Loading Overlay

internal void bp_loading_overlay(Rng2f32 rect, f32 loading_t, u64 progress_v, u64 progress_v_target)
{
    if (loading_t >= 0.001f) UI_Focus(UI_FocusKind_Off)
    {
        // set up dimensions
        f32 edge_padding = 30.f;
        f32 width         = ui_top_font_size() * 10;
        f32 height        = ui_top_font_size() * 1.f;
        f32 min_thickness = ui_top_font_size() / 2;
        f32 trail         = ui_top_font_size() * 4;
        f32 t = pow_f32(sin_f32((f32)bp_state->time_in_seconds / 1.8f), 2.f);
        f64 v = 1.f - abs_f32(0.5f - t);

        // build indicator
        UI_CornerRadius(height / 3.f) UI_Transparency(1 - loading_t)
        {
            // rects
            Rng2f32 indicator_region_rect =
                r2f32p((rect.x0 + rect.x1)/2 - width/2  - rect.x0,
                       (rect.y0 + rect.y1)/2 - height/2 - rect.y0,
                       (rect.x0 + rect.x1)/2 + width/2  - rect.x0,
                       (rect.y0 + rect.y1)/2 + height/2 - rect.y0);
            Rng2f32 indicator_rect =
                r2f32p(indicator_region_rect.x0 + width * t - min_thickness/2 - trail * v,
                       indicator_region_rect.y0,
                       indicator_region_rect.x0 + width * t - min_thickness/2 + trail * v,
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
                                          indicator_region_rect.x0 + (indicator_region_rect.x1 - indicator_region_rect.x0) * pct_done,
                                          indicator_region_rect.y1);
                UI_Rect(pct_rect)
                    ui_build_box_from_key(UI_BoxFlag_DrawBackground | UI_BoxFlag_Floating, ui_key_zero());
            }

            // fill
            UI_TagF("pop") UI_Rect(indicator_rect)
                ui_build_box_from_key(UI_BoxFlag_DrawBackground | UI_BoxFlag_Floating, ui_key_zero());

            // animated bar
            UI_Rect(indicator_region_rect)
            {
                UI_Box *box = ui_build_box_from_stringf(UI_BoxFlag_DrawBackground|
                                                       UI_BoxFlag_DrawBorder    |
                                                       UI_BoxFlag_Floating      |
                                                       UI_BoxFlag_Clickable, "bg_system_status");
                UI_Signal sig = ui_signal_from_box(box);
            }
        }

        // build background
        UI_WidthFill UI_HeightFill UI_Transparency(1 - loading_t) UI_BlurSize(10.f * loading_t)
        {
            ui_set_next_blur_size(10.f * loading_t);
            ui_build_box_from_key(UI_BoxFlag_DrawBackground|
                                  UI_BoxFlag_DrawBackgroundBlur|
                                  UI_BoxFlag_Floating,
                                  ui_key_zero());
        }
    }
}

//////////////////////
// UI Widgets: Fancy Buttons

internal void bp_cmd_binding_buttons(String8 name, String8 filter, u64 limit, BP_CmdBindingButtonFlags flags)
{
    Temp scratch = scratch_begin(0, 0);
    CFG_Key_Map_Node_Ptr_List key_map_nodes = cfg_key_map_node_ptr_list_from_name(scratch.arena,
                                                                                  bp_state->key_map,
                                                                                  name);
    // build buttons for each binding
    u64 key_map_idx = 0;
    UI_CornerRadius(ui_top_font_size() * 0.5f)
        for (CFG_Key_Map_Node_Ptr *n = key_map_nodes.first; n != 0; n = n->next, key_map_idx += 1)
    {
        if (key_map_idx >= limit) { break; }
        ui_spacer(ui_em(1.f, 1.f));
        CFG_Binding binding = n->v->binding;
        bool32 rebinding_active_for_this_binding = (bp_state->bind_change_active &&
                                                    str8_match(bp_state->bind_change_cmd_name, name, 0) &&
                                                    n->v->cfg_id == bp_state->bind_change_binding_id);

        // grab all conflicts
        bool32 has_conflicts = false;
        CFG_Key_Map_Node_Ptr_List nodes_with_this_binding = cfg_key_map_node_ptr_list_from_binding(scratch.arena,
                                                                                                   bp_state->key_map,
                                                                                                   binding);
        {
            for (CFG_Key_Map_Node_Ptr *n2 = nodes_with_this_binding.first; n2 != 0; n2 = n2->next)
            {
                if (!str8_match(n->v->name, n2->v->name, 0))
                {
                    has_conflicts = true;
                    break;
                }
            }
        }

        // form binding string
        String8 keybinding_str = {0};
        {
            if (binding.key != WM_Key_Null)
            {
                keybinding_str = wm_string_from_modifiers_key(scratch.arena, binding.modifiers, binding.key);
            }
            else
            {
                keybinding_str = str8_lit("- no binding -");
            }
        }

        // compute fuzzy matches
        Fuzzy_Match_Range_List matches = {0};
        if (filter.size != 0)
        {
            matches = fuzzy_match_find(scratch.arena, filter, keybinding_str);
        }

        // build box
        ui_set_next_tag(has_conflicts ? str8_lit("bad_pop")
                        : rebinding_active_for_this_binding ? str8_lit("pop")
                        : str8_zero());
        ui_set_next_text_alignment(UI_TextAlign_Center);
        ui_set_next_group_key(ui_key_zero());
        ui_set_next_pref_width(ui_text_dim(ui_top_font_size() * 1.f, 1));
        UI_Box *box = ui_build_box_from_stringf(UI_BoxFlag_DrawText|
                                                (!(flags & BP_CmdBindingButtonFlag_NoEdit) * UI_BoxFlag_Clickable)|
                                                UI_BoxFlag_DrawActiveEffects|
                                                UI_BoxFlag_DrawHotEffects|
                                                UI_BoxFlag_DrawBorder|
                                                UI_BoxFlag_DrawBackground,
                                                "%S###bind_btn_%S_%x_%x", keybinding_str, name, binding.key, binding.modifiers);
        ui_box_equip_fuzzy_match_ranges(box, &matches);

        // interaction
        UI_Signal sig = ui_signal_from_box(box);
        {
            // click => toggle activity
            if (!bp_state->bind_change_active && ui_clicked(sig))
            {
                if ((binding.key == WM_Key_Esc || binding.key == WM_Key_Delete) && binding.modifiers == 0)
                {
                    log_user_error(str8_lit("Cannot rebind; this command uses a reserved keybinding."));
                }
                else
                {
                    arena_clear(bp_state->bind_change_arena);
                    bp_state->bind_change_active = true;
                    bp_state->bind_change_cmd_name = push_str8_copy(bp_state->bind_change_arena, name);
                    bp_state->bind_change_binding_id = n->v->cfg_id;
                }
            }
            else if (bp_state->bind_change_active && ui_clicked(sig))
            {
                bp_state->bind_change_active = false;
            }

            // hover w/ conflicts => show conflicts
            if (ui_hovering(sig) && has_conflicts) UI_Tooltip
            {
                UI_PrefWidth(ui_children_sum(1)) bp_error_label(str8_lit("This binding conflicts with those for:"));
                for (CFG_Key_Map_Node_Ptr *n2 = nodes_with_this_binding.first; n2 != 0; n2 = n2->next)
                {
                    if (!str8_match(n2->v->name, n->v->name, 0))
                    {
                        String8 display_name = bp_display_from_code_name(n2->v->name);
                        ui_labelf("%S", display_name);
                    }
                }
            }
        }

        // delete button
        if (rebinding_active_for_this_binding)
            UI_PrefWidth(ui_em(2.5f, 1.f))
            UI_TagF("bad_pop")
        {
            ui_set_next_group_key(ui_key_zero());
            UI_Signal sig = bp_icon_button(BP_IconKind_X, 0, str8_lit("###delete_binding"));
            if (ui_clicked(sig))
            {
                cfg_node_release(bp_state->cfg, cfg_node_from_id(bp_state->bind_change_binding_id));
                bp_state->bind_change_active = 0;
            }
        }
    }

    // build "add new binding" button
    if (flags & BP_CmdBindingButtonFlag_AddNew)
    {
        bool32 adding_new_binding = (bp_state->bind_change_active &&
                                     str8_match(bp_state->bind_change_cmd_name, name, 0) &&
                                     bp_state->bind_change_binding_id == 0);
        ui_spacer(ui_em(1.f, 1.f));
        {
            UI_Box *box = &ui_nil_box;
            BP_Font(BP_FontSlot_Icons) UI_TagF(adding_new_binding ? "pop" : "") UI_CornerRadius(ui_top_font_size() * 0.5f)
            {
                ui_set_next_text_alignment(UI_TextAlign_Center);
                ui_set_next_group_key(ui_key_zero());
                ui_set_next_pref_width(ui_text_dim(ui_top_font_size() * 1.5f, 1));
                box = ui_build_box_from_stringf(UI_BoxFlag_DrawText|
                                                UI_BoxFlag_Clickable|
                                                UI_BoxFlag_DrawActiveEffects|
                                                UI_BoxFlag_DrawHotEffects|
                                                UI_BoxFlag_DrawBorder|
                                                UI_BoxFlag_DrawBackground,
                                                "%S###add_binding", bp_icon_kind_text_table[BP_IconKind_Add]);
            }
            UI_Signal sig = ui_signal_from_box(box);
            if (ui_hovering(sig)) UI_Tooltip
            {
                ui_state->tooltip_anchor_key = box->key;
                ui_labelf("Add New Binding");
            }
            if (ui_clicked(sig))
            {
                if (!adding_new_binding && ui_clicked(sig))
                {
                    arena_clear(bp_state->bind_change_arena);
                    bp_state->bind_change_active = true;
                    bp_state->bind_change_cmd_name = push_str8_copy(bp_state->bind_change_arena, name);
                    bp_state->bind_change_binding_id = 0;
                }
                else if (adding_new_binding && ui_clicked(sig))
                {
                    bp_state->bind_change_active = 0;
                }
            }
        }
    }

    scratch_end(scratch);
}

internal UI_Signal bp_menu_bar_button(String8 string)
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

internal UI_Signal bp_cmd_spec_button(String8 name)
{
    BP_Cmd_Kind_Info *info = bp_cmd_kind_info_from_string(name);
    ui_set_next_child_layout_axis(Axis2_X);
    UI_Box *box = ui_build_box_from_stringf(UI_BoxFlag_DrawBorder|
                                            UI_BoxFlag_DrawBackground|
                                            UI_BoxFlag_DrawHotEffects|
                                            UI_BoxFlag_DrawActiveEffects|
                                            UI_BoxFlag_Clickable,
                                            "###cmd_%p", info);
    UI_Parent(box) UI_HeightFill UI_Padding(ui_em(1.f, 1.f))
    {
        BP_IconKind canonical_icon = bp_icon_kind_from_code_name(name);
        if (canonical_icon != BP_IconKind_Null)
        {
            BP_Font(BP_FontSlot_Icons)
                UI_PrefWidth(ui_em(2.f, 1.f))
                UI_TextAlignment(UI_TextAlign_Center)
                UI_TagF("weak")
            {
                ui_label(bp_icon_kind_text_table[canonical_icon]);
            }
        }
        UI_PrefWidth(ui_text_dim(10, 1.f))
        {
            UI_Flags(UI_BoxFlag_DrawTextFastpathCodepoint)
                UI_FastpathCodepoint(box->fastpath_codepoint)
                ui_label(bp_display_from_code_name(name));
            ui_spacer(ui_pct(1, 0));
            ui_set_next_flags(UI_BoxFlag_Clickable);
            ui_set_next_group_key(ui_key_zero());
            UI_PrefWidth(ui_children_sum(1))
                UI_FontSize(ui_top_font_size() * 0.95f) UI_HeightFill
                UI_NamedRow(str8_lit("###bindings"))
                UI_TagF("weak")
                UI_FastpathCodepoint(0)
            {
                bp_cmd_binding_buttons(name, str8_zero(), max_u64, BP_CmdBindingButtonFlag_AddNew);
            }
        }
    }
    UI_Signal sig = ui_signal_from_box(box);
    return sig;
}

internal void bp_cmd_list_menu_buttons(u64 count, String8 *cmd_names, u32 *fastpath_codepoints)
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
            UI_Signal sig = bp_cmd_spec_button(cmd_names[idx]);
            if (ui_clicked(sig))
            {
                bp_cmd(BP_CmdKind_RunCommand, .cmd_name = cmd_names[idx]);
                ui_ctx_menu_close();
                CFG_Node *window = cfg_node_from_id(bp_regs()->window);
                BP_Window_State *ws = bp_window_state_from_cfg(window);
                ws->menu_bar_focused = false;
            }
        }
    }
    scratch_end(scratch);
}

internal UI_Signal bp_icon_button(BP_IconKind kind, Fuzzy_Match_Range_List *matches, String8 string)
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
            BP_Font(BP_FontSlot_Icons)
            UI_PrefWidth(ui_em(2.f, 1.f))
            UI_PrefHeight(ui_pct(1, 0))
            UI_FlagsAdd(UI_BoxFlag_DisableTextTrunc)
            UI_TagF("weak")
            ui_label(bp_icon_kind_text_table[kind]);
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

internal UI_Signal bp_icon_buttonf(BP_IconKind kind, Fuzzy_Match_Range_List *matches, char *fmt, ...)
{
    Temp scratch = scratch_begin(0, 0);
    va_list args;
    va_start(args, fmt);
    String8 string = push_str8fv(scratch.arena, fmt, args);
    va_end(args);
    UI_Signal sig = bp_icon_button(kind, matches, string);
    scratch_end(scratch);
    return sig;
}

////////////////////////
// UI Widgets: Text View

internal BP_Code_Slice_Signal bp_code_slice(BP_Code_Slice_Params *params, u64 *cursor, u64 *mark, s64 *preferred_column, String8 string)
{
    BP_Code_Slice_Signal result = {0};
    ProfBeginFunction();
    Temp scratch = scratch_begin(0, 0);
  
    bool32 is_focused = ui_is_focus_active();
    bool32 ctrlified = (wm_get_modifiers() & WM_Modifier_Ctrl);
    f32 line_num_padding_px = ui_top_font_size()*1.f;
    bool32 do_scope_lines = bp_setting_bool32_from_name(s("cursor_scope_lines"));
    bool32 do_scope_end_annotations = bp_setting_bool32_from_name(s("cursor_scope_end_annotations"));
    bool32 do_cursor_trail = bp_setting_bool32_from_name(s("animations")) && bp_setting_bool32_from_name(s("cursor_trail"));
    Vec4f32 pop_color = {0};
    UI_TagF("pop")
    {
        pop_color = ui_color_from_name(s("background"));
    }
    Vec4f32 highlight_color = {0};
    UI_TagF("focus")
    {
        highlight_color = ui_color_from_name(s("border"));
    }
    
    //////////////////////////////
    //- rjf: build top-level container
    //
    UI_Box *top_container_box = &ui_nil_box;
    Rng2f32 clipped_top_container_rect = {0};
    {
        ui_set_next_child_layout_axis(Axis2_X);
        ui_set_next_pref_width(ui_px(params->line_text_max_width_px, 1));
        ui_set_next_pref_height(ui_children_sum(1));
        top_container_box = ui_build_box_from_string(UI_BoxFlag_DisableFocusEffects|UI_BoxFlag_DrawBorder, string);
        clipped_top_container_rect = top_container_box->rect;
        for(UI_Box *b = top_container_box; !ui_box_is_nil(b); b = b->parent)
        {
            if(b->flags & UI_BoxFlag_Clip)
            {
                clipped_top_container_rect = intersect_2f32(b->rect, clipped_top_container_rect);
            }
        }
    }
    
    //////////////////////////////
    //- rjf: build line numbers
    //
    if(params->flags & BP_CodeSliceFlag_LineNums) UI_Parent(top_container_box) ProfScope("build line numbers") UI_Focus(UI_FocusKind_Off)
    UI_TagF("floating")
    {
        Rng1u64 select_rng = r1u64(*cursor, *mark);
        ui_set_next_fixed_x(floor_f32(params->margin_float_off_px));
        ui_set_next_pref_width(ui_px(params->line_num_width_px, 1.f));
        ui_set_next_pref_height(ui_px(params->line_height_px*(dim_1s64(params->line_num_range)+1), 1.f));
        ui_set_next_flags(UI_BoxFlag_DrawSideRight);
        UI_Column
            UI_PrefHeight(ui_px(params->line_height_px, 1.f))
            BP_Font(BP_FontSlot_Code)
            UI_FontSize(params->font_size)
            UI_CornerRadius(0)
        {
            u64 line_idx = 0;
            for(s64 line_num = params->line_num_range.min;
                line_num <= params->line_num_range.max;
                line_num += 1, line_idx += 1)
            {
                Rng1u64 line_range = params->line_ranges[line_idx];
                bool32 line_is_selected = (dim_1u64(intersect_1u64(select_rng, line_range)) != 0) || (line_range.min <= *cursor && *cursor <= line_range.max);
                UI_TagF(line_is_selected ? "" : "weak")
                    ui_build_box_from_stringf(UI_BoxFlag_DrawText, "%I64u##line_num", line_num);
            }
        }
    }
    
    //////////////////////////////
    //- rjf: build background for line numbers
    //
    {
        UI_Parent(top_container_box) UI_TagF("floating")
        {
            ui_set_next_pref_width(ui_px(params->line_num_width_px, 1));
            ui_set_next_pref_height(ui_px(params->line_height_px*(dim_1s64(params->line_num_range)+1), 1.f));
            ui_set_next_fixed_x(floor_f32(params->margin_float_off_px));
            ui_build_box_from_key(UI_BoxFlag_DrawBackgroundBlur|UI_BoxFlag_DrawBackground|UI_BoxFlag_DrawDropShadow, ui_key_zero());
        }
    }
    
    //////////////////////////////
    //- rjf: build main text container box, for mouse interaction on both lines & line numbers
    //
    UI_Box *text_container_box = &ui_nil_box;
    UI_Parent(top_container_box) UI_Focus(UI_FocusKind_Off)
    {
        ui_set_next_hover_cursor(ctrlified ? WM_Cursor_HandPoint : WM_Cursor_IBar);
        ui_set_next_pref_height(ui_px(params->line_height_px*(dim_1s64(params->line_num_range)+1), 1.f));
        text_container_box = ui_build_box_from_string(UI_BoxFlag_Clickable*!!(params->flags & BP_CodeSliceFlag_Clickable), s("text_container"));
    }
    
    //////////////////////////////
    //- rjf: mouse -> text coordinates
    //
    u64 mouse_off = 0;
    u64 mouse_y_line_idx = 0;
    ProfScope("mouse -> text coordinates")
    {
        Vec2f32 mouse = ui_mouse();
        s64 mouse_y_line_idx_signed = ((mouse.y - text_container_box->rect.y0) / params->line_height_px);
        mouse_y_line_idx_signed = Clamp(0, mouse_y_line_idx_signed, dim_1s64(params->line_num_range)-1);
        mouse_y_line_idx = (u64)mouse_y_line_idx_signed;
        s64 line_num = (params->line_num_range.min + mouse_y_line_idx);
        Rng1u64 line_range = (params->line_num_range.min <= line_num && line_num <= params->line_num_range.max) ? (params->line_ranges[mouse_y_line_idx]) : r1u64(0, 0);
        String8 line_string = (params->line_num_range.min <= line_num && line_num <= params->line_num_range.max) ? (params->line_text[mouse_y_line_idx]) : str8_zero();
        u64 mouse_line_off = fnt_char_pos_from_tag_size_string_p(params->font, params->font_size, 0, params->tab_size, line_string, mouse.x-text_container_box->rect.x0-params->line_num_width_px-line_num_padding_px);
        mouse_off = line_range.min + mouse_line_off;
        {
            Rng1u64 legal_range = r1u64(params->line_ranges[0].min, params->line_ranges[dim_1s64(params->line_num_range)].max);
            mouse_off = clamp_1u64(legal_range, mouse_off);
        }
        result.mouse_off = mouse_off;
    }
    
    //////////////////////////////
    //- rjf: mouse point -> mouse token range, mouse line range
    //
    Rng1u64 mouse_token_rng = r1u64(mouse_off, mouse_off);
    Rng1u64 mouse_line_rng = r1u64(mouse_off, mouse_off);
    if(mouse_y_line_idx < (u64)dim_1s64(params->line_num_range))
    {
        TXT_Token_Array *line_tokens = &params->line_tokens[mouse_y_line_idx];
        Rng1u64 line_range = params->line_ranges[mouse_y_line_idx];
        for(u64 line_token_idx = 0; line_token_idx < line_tokens->count; line_token_idx += 1)
        {
            TXT_Token *line_token = &line_tokens->v[line_token_idx];
            if(contains_1u64(line_token->range, mouse_off))
            {
                Rng1u64 line_token_range_clamped = intersect_1u64(line_token->range, line_range);
                mouse_token_rng = r1u64(line_token_range_clamped.min, line_token_range_clamped.max);
                break;
            }
        }
        mouse_line_rng = line_range;
    }
    
    //////////////////////////////
    //- rjf: interact with text box
    //
    UI_Signal text_container_sig = ui_signal_from_box(text_container_box);
    bool32 search_query_invalidated = 0;
    {
        Rng1u64 mouse_drag_rng = r1u64(mouse_off, mouse_off);
        if(text_container_sig.f & UI_SignalFlag_LeftTripleDragging)
        {
            mouse_drag_rng = mouse_line_rng;
        }
        else if(text_container_sig.f & UI_SignalFlag_LeftDoubleDragging)
        {
            mouse_drag_rng = mouse_token_rng;
        }
        
        if(!ctrlified && ui_dragging(text_container_sig))
        {
            if(ui_pressed(text_container_sig))
            {
                *cursor = mouse_drag_rng.max;
                *mark = mouse_drag_rng.min;
            }
            if(mouse_off < *mark)
            {
                *cursor = mouse_drag_rng.min;
            }
            else
            {
                *cursor = mouse_drag_rng.max;
            }
            *preferred_column = *cursor - mouse_line_rng.min;
        }
        
        if(ui_dragging(text_container_sig) || ui_released(text_container_sig))
        {
            search_query_invalidated = 1;
        }
        
        if(ui_right_clicked(text_container_sig))
        {
            if(*cursor == *mark)
            {
                *cursor = *mark = mouse_off;
            }
            String8 commands_expr = (*cursor == *mark) ? s("query:text_pt_commands") : s("query:text_range_commands");
            bp_cmd(BP_CmdKind_FocusPanel);
            bp_cmd(BP_CmdKind_PushQuery,
                   .expr = str8f(scratch.arena, "%S, query:config.$%I64x", commands_expr, bp_regs()->view),
                   .do_implicit_root = 1,
                   .do_lister = 1,
                   .small_size = 1,
                   .activate_with_single_click = 1,
                   .ui_key = ui_get_selected_state()->root->key,
                   .off_px = ui_mouse(),
                   .cursor = *cursor,
                   .mark = *mark);
        }
        
        result.base = text_container_sig;
    }
    
    //////////////////////////////
    //- rjf: cursor -> scope info
    //
    TXT_Scope_Node *cursor_scope_node = &txt_scope_node_nil;
    if(params->text_info != 0)
    {
        cursor_scope_node = txt_scope_node_from_info_off(params->text_info, *cursor);
    }
    
    //////////////////////////////
    //- rjf: equip cursor scope rendering info
    //
    if(do_scope_lines && cursor_scope_node != &txt_scope_node_nil)
    {
        f32 scope_line_thickness = params->font_size*0.1f;
        scope_line_thickness = Max(scope_line_thickness, 1.f);
        DR_Bucket *bucket = dr_bucket_make();
        DR_BucketScope(bucket)
        {
            Vec2f32 text_base_pos = v2f32(text_container_box->rect.x0 + params->line_num_width_px + line_num_padding_px,
                                          text_container_box->rect.y0);
            f32 ancestor_chain_depth = 0;
            for(TXT_Scope_Node *scope_n = cursor_scope_node;
                scope_n != &txt_scope_node_nil;
                scope_n = txt_scope_node_from_info_num(params->text_info, scope_n->parent_num), ancestor_chain_depth += 1)
            {
                Vec4f32 scope_line_color = highlight_color;
                f32 scope_line_color_target = highlight_color.w;
                scope_line_color_target *= 1 - ancestor_chain_depth / 6.f;
                scope_line_color_target = Max(0.2f, scope_line_color_target);
                f32 scope_line_color_t = ui_anim(ui_key_from_stringf(text_container_box->key, "###scope_depth_%I64x_%I64x", scope_n->token_idx_range.min, scope_n->token_idx_range.max), scope_line_color_target, .rate = bp_state->menu_animation_rate__slow);
                scope_line_color.w = scope_line_color_t*0.5f;
                Rng1u64 token_idx_range = scope_n->token_idx_range;
                Rng1u64 off_range = r1u64(params->text_info->tokens.v[token_idx_range.min].range.min, params->text_info->tokens.v[token_idx_range.max].range.min);
                Txt_Rng txt_range = txt_rng(txt_pt_from_off__linear_scan(params->text_info, params->patches, off_range.min), txt_pt_from_off__linear_scan(params->text_info, params->patches, off_range.max));
                
                if(txt_range.min.line == txt_range.max.line && contains_1s64(params->line_num_range, txt_range.min.line))
                {
                    s64 line_num = txt_range.min.line;
                    u64 line_idx = (u64)(line_num - params->line_num_range.min);
                    String8 line_string = params->line_text[line_idx];
                    Rng1u64 line_off_range = r1u64(off_range.min - params->line_ranges[line_idx].min, off_range.max+1 - params->line_ranges[line_idx].min);
                    Rng1f32 x_px_range = r1f32(fnt_dim_from_tag_size_string(params->font, params->font_size, 0, params->tab_size, str8_prefix(line_string, line_off_range.min)).x,
                                               fnt_dim_from_tag_size_string(params->font, params->font_size, 0, params->tab_size, str8_prefix(line_string, line_off_range.max)).x);
                    f32 line_y = line_idx*params->line_height_px;
                    Rng2f32 underline_rect = r2f32p(text_base_pos.x + x_px_range.min,
                                                    text_base_pos.y + line_y + params->line_height_px*0.5f,
                                                    text_base_pos.x + x_px_range.max+1,
                                                    text_base_pos.y + line_y + params->line_height_px + params->font_size*0.1f);
                    f32 midpoint = center_1f32(r1f32(underline_rect.x0, underline_rect.x1));
                    f32 t = ui_anim(ui_key_from_stringf(text_container_box->key, "###scope_%I64x_%I64x", scope_n->token_idx_range.min, scope_n->token_idx_range.max), 1.f, .rate = bp_state->catchall_animation_rate);
                    Rng2f32 underline_clip = {0};
                    underline_clip.x0 = mix_1f32(midpoint, underline_rect.x0 - params->font_size, t);
                    underline_clip.x1 = mix_1f32(midpoint, underline_rect.x1 + params->font_size, t);
                    underline_clip.y0 = underline_rect.y0 + (underline_rect.y1 - underline_rect.y0) * 0.65f;
                    underline_clip.y1 = 10000;
                    DR_ClipScope(underline_clip)
                    {
                        dr_rect(underline_rect, scope_line_color, params->font_size*0.1f, scope_line_thickness, 1.f);
                    }
                }
                
                if(txt_range.min.line != txt_range.max.line && params->line_num_range.max > txt_range.min.line && params->line_num_range.min < txt_range.max.line)
                {
                    String8 opener_line = txt_string_from_info_data_line_num(params->text_info, params->text_data, txt_range.min.line);
                    String8 closer_line = txt_string_from_info_data_line_num(params->text_info, params->text_data, txt_range.max.line);
                    String8 opener_line_pre_opener = str8_prefix(opener_line, txt_range.min.column-1);
                    String8 closer_line_pre_closer = str8_prefix(closer_line, txt_range.max.column-1);
                    f32 opener_line_pre_opener_px = fnt_dim_from_tag_size_string(params->font, params->font_size, 0, params->tab_size, opener_line_pre_opener).x;
                    f32 closer_line_pre_closer_px = fnt_dim_from_tag_size_string(params->font, params->font_size, 0, params->tab_size, closer_line_pre_closer).x;
                    f32 indent_depth_px = Min(opener_line_pre_opener_px, closer_line_pre_closer_px);
                    Rng1f32 scope_range_y_px = r1f32(0, dim_2f32(text_container_box->rect).y);
                    if(contains_1s64(params->line_num_range, txt_range.min.line))
                    {
                        scope_range_y_px.min = (txt_range.min.line - params->line_num_range.min) * params->line_height_px;
                    }
                    if(contains_1s64(params->line_num_range, txt_range.max.line))
                    {
                        scope_range_y_px.max = ((txt_range.max.line - params->line_num_range.min) + 1) * params->line_height_px;
                    }
                    f32 midpoint = center_1f32(scope_range_y_px);
                    f32 t = ui_anim(ui_key_from_stringf(text_container_box->key, "###scope_%I64x_%I64x", scope_n->token_idx_range.min, scope_n->token_idx_range.max), 1.f, .rate = bp_state->catchall_animation_rate);
                    Rng2f32 scope_rect = r2f32p(text_base_pos.x + indent_depth_px - params->font_size*0.2f,
                                                text_base_pos.y + scope_range_y_px.min,
                                                text_base_pos.x + indent_depth_px - params->font_size*0.2f + params->font_size*1.f,
                                                text_base_pos.y + scope_range_y_px.max);
                    Rng2f32 scope_clip_rect = {0};
                    {
                        scope_clip_rect.x0 = scope_rect.x0 - params->font_size*10.f;
                        scope_clip_rect.x1 = scope_rect.x0 + (scope_rect.x1 - scope_rect.x0)*0.4f;
                        scope_clip_rect.y0 = mix_1f32(midpoint, scope_rect.y0 - params->font_size*0.1f, t);
                        scope_clip_rect.y1 = mix_1f32(midpoint, scope_rect.y1 + params->font_size*0.1f, t);
                    }
                    DR_ClipScope(scope_clip_rect)
                    {
                        dr_rect(scope_rect, scope_line_color, params->font_size*0.1f, scope_line_thickness, 1.f);
                    }
                }
                
                if(do_scope_end_annotations && txt_range.min.line != txt_range.max.line && contains_1s64(params->line_num_range, txt_range.max.line)) UI_TagF("weak")
                {
                    String8 opener_line = str8_skip_chop_whitespace(txt_string_from_info_data_line_num(params->text_info, params->text_data, txt_range.min.line));
                    String8 scope_title_string = opener_line;
                    if(str8_match(opener_line, s("{"), 0) ||
                       str8_match(opener_line, s("["), 0) ||
                       str8_match(opener_line, s("("), 0))
                    {
                        scope_title_string = str8_skip_chop_whitespace(txt_string_from_info_data_line_num(params->text_info, params->text_data, txt_range.min.line-1));
                    }
                    if(!str8_match(scope_title_string, s("{"), 0) &&
                       !str8_match(scope_title_string, s("["), 0) &&
                       !str8_match(scope_title_string, s("("), 0))
                    {
                        f32 t = ui_anim(ui_key_from_stringf(text_container_box->key, "###scope_end_annotation_%I64x_%I64x", scope_n->token_idx_range.min, scope_n->token_idx_range.max), 1.f, .rate = bp_state->catchall_animation_rate);
                        String8 closer_line = txt_string_from_info_data_line_num(params->text_info, params->text_data, txt_range.max.line);
                        f32 closer_line_px = fnt_dim_from_tag_size_string(params->font, params->font_size, 0, params->tab_size, closer_line).x;
                        Vec4f32 color = ui_color_from_name(s("text"));
                        color.w *= 0.5f*t;
                        dr_text(params->font, params->font_size * 0.85f, 0, 0, ui_top_text_raster_flags(),
                                v2f32(text_base_pos.x + closer_line_px + ui_top_font_size()*0.5f*t,
                                      text_base_pos.y + (txt_range.max.line - params->line_num_range.min) * params->line_height_px + params->line_height_px*0.7f),
                                color, scope_title_string);
                    }
                }
            }
        }
        ui_box_equip_draw_bucket(text_container_box, bucket);
    }
    
    //////////////////////////////
    //- rjf: produce fancy strings for each line
    //
    // NOTE: unchanged - token-based syntax highlighting, fully generic.
    //
    DR_FStr_List *lines_fstrs = push_array(scratch.arena, DR_FStr_List, dim_1s64(params->line_num_range)+1);
    {
        DR_FStr_Params fstr_params = {
            params->font,
            bp_raster_flags_from_slot(BP_FontSlot_Code),
            bp_rgba_from_code_color_slot(BP_CodeColorSlot_CodeDefault),
            params->font_size,
        };
        u64 line_idx = 0;
        for(s64 line_num = params->line_num_range.min;
            line_num <= params->line_num_range.max;
            line_num += 1, line_idx += 1)
        {
            String8 line_string = params->line_text[line_idx];
            Rng1u64 line_range = params->line_ranges[line_idx];
            TXT_Token_Array *line_tokens = &params->line_tokens[line_idx];
            DR_FStr_List fstrs = {0};
            if(line_tokens->count == 0)
            {
                dr_fstrs_push_new(scratch.arena, &fstrs, &fstr_params, line_string);
            }
            else
            {
                TXT_Token *line_tokens_first = line_tokens->v;
                TXT_Token *line_tokens_opl = line_tokens->v + line_tokens->count;
                bool32 preceded_by_dot = 0;
                for(TXT_Token *token = line_tokens_first; token < line_tokens_opl; token += 1)
                {
                    String8 token_string = {0};
                    {
                        Rng1u64 token_range = r1u64(0, line_string.size);
                        if(token->range.min > line_range.min)
                        {
                            token_range.min += token->range.min-line_range.min;
                        }
                        if(token->range.max < line_range.max)
                        {
                            token_range.max = token->range.max-line_range.min;
                        }
                        token_string = str8_substr(line_string, token_range);
                    }
                    
                    BP_CodeColorSlot token_color_slot = bp_code_color_slot_from_txt_token_kind(token->kind);
                    BP_CodeColorSlot lookup_color_slot = preceded_by_dot ? token_color_slot : bp_code_color_slot_from_txt_token_kind_lookup_string(token->kind, token_string, 0, 0);
                    Vec4f32 token_color = bp_rgba_from_code_color_slot(token_color_slot);
                    if(lookup_color_slot != BP_CodeColorSlot_CodeDefault)
                    {
                        Vec4f32 lookup_color = bp_rgba_from_code_color_slot(lookup_color_slot);
                        f32 lookup_color_mix_t = ui_anim(ui_key_from_stringf(ui_key_zero(), "%S_lookup", token_string), 1.f);
                        token_color = mix_4f32(token_color, lookup_color, lookup_color_mix_t);
                    }
                    
                    for(TXT_Scope_Node *scope_n = cursor_scope_node;
                        scope_n != &txt_scope_node_nil;
                        scope_n = txt_scope_node_from_info_num(params->text_info, scope_n->parent_num))
                    {
                        if(params->text_info->tokens.v[scope_n->token_idx_range.min].range.min == token->range.min ||
                           params->text_info->tokens.v[scope_n->token_idx_range.max].range.min == token->range.min)
                        {
                            token_color = highlight_color;
                            break;
                        }
                    }
                    
                    dr_fstrs_push_new(scratch.arena, &fstrs, &fstr_params, token_string, .color = token_color);
                    preceded_by_dot = (token->kind == TXT_TokenKind_Symbol && str8_match(token_string, s("."), 0));
                }
            }
            lines_fstrs[line_idx] = fstrs;
        }
    }
    
    //////////////////////////////
    //- rjf: mouse -> expression range info
    //
    Vec2f32 mouse_expr_baseline_pos = {0};
    String8 mouse_expr = {0};
    if(ui_hovering(text_container_sig) && mouse_y_line_idx < (u64)dim_1s64(params->line_num_range))
    {
        Rng1u64 selected_rng = r1u64(*cursor, *mark);
        if(*cursor != *mark && contains_1u64(selected_rng, mouse_off))
        {
            String8 line_text = params->line_text[mouse_y_line_idx];
            Rng1u64 line_range = params->line_ranges[mouse_y_line_idx];
            Rng1u64 selected_in_line_range = intersect_1u64(line_range, selected_rng);
            f32 expr_hoff_px = params->line_num_width_px + fnt_dim_from_tag_size_string(params->font, params->font_size, 0, params->tab_size, str8_prefix(line_text, selected_in_line_range.min)).x;
            result.mouse_expr_rng = selected_in_line_range;
            mouse_expr_baseline_pos = v2f32(text_container_box->rect.x0+expr_hoff_px,
                                            text_container_box->rect.y0+mouse_y_line_idx*params->line_height_px + params->line_height_px*0.85f);
            mouse_expr = str8_substr(line_text, selected_in_line_range);
        }
        else
        {
            String8 line_text = params->line_text[mouse_y_line_idx];
            TXT_Token_Array line_tokens = params->line_tokens[mouse_y_line_idx];
            Rng1u64 line_range = params->line_ranges[mouse_y_line_idx];
            Rng1u64 expr_off_rng = txt_expr_off_range_from_line_off_range_string_tokens(mouse_off, line_range, line_text, &line_tokens);
            if(expr_off_rng.max != expr_off_rng.min)
            {
                f32 expr_hoff_px = params->line_num_width_px + fnt_dim_from_tag_size_string(params->font, params->font_size, 0, params->tab_size, str8_prefix(line_text, expr_off_rng.min-line_range.min)).x;
                result.mouse_expr_rng = expr_off_rng;
                mouse_expr_baseline_pos = v2f32(text_container_box->rect.x0+expr_hoff_px,
                                                text_container_box->rect.y0+mouse_y_line_idx*params->line_height_px + params->line_height_px*0.85f);
                mouse_expr = str8_substr(line_text, r1u64(expr_off_rng.min-line_range.min, expr_off_rng.max-line_range.min));
            }
        }
    }
    
    //////////////////////////////
    //- rjf: (cursor*mark*list(flash_range)) -> list(text_range*color)
    //
    typedef struct Txt_Rng_Color_Pair_Node Txt_Rng_Color_Pair_Node;
    struct Txt_Rng_Color_Pair_Node
    {
        Txt_Rng_Color_Pair_Node *next;
        Rng1u64 range;
        Vec4f32 color;
    };
    Txt_Rng_Color_Pair_Node *first_txt_rng_color_pair = 0;
    Txt_Rng_Color_Pair_Node *last_txt_rng_color_pair = 0;
    {
        {
            Txt_Rng_Color_Pair_Node *n = push_array(scratch.arena, Txt_Rng_Color_Pair_Node, 1);
            n->range = r1u64(*cursor, *mark);
            n->color = ui_color_from_name(s("selection"));
            SLLQueuePush(first_txt_rng_color_pair, last_txt_rng_color_pair, n);
        }
        if(ctrlified && result.mouse_expr_rng.max != result.mouse_expr_rng.min) UI_Tag(s("pop"))
        {
            Txt_Rng_Color_Pair_Node *n = push_array(scratch.arena, Txt_Rng_Color_Pair_Node, 1);
            n->range = result.mouse_expr_rng;
            n->color = ui_color_from_name(s("background"));
            n->color.w *= 0.2f;
            SLLQueuePush(first_txt_rng_color_pair, last_txt_rng_color_pair, n);
        }
    }
    
    //////////////////////////////
    //- rjf: build line numbers region (line number interaction should be basically identical to lines)
    //
    if(params->flags & BP_CodeSliceFlag_LineNums) UI_Parent(text_container_box) ProfScope("build line number interaction box") UI_Focus(UI_FocusKind_Off)
    {
        ui_set_next_pref_width(ui_px(params->line_num_width_px, 1.f));
        ui_set_next_pref_height(ui_px(params->line_height_px*(dim_1s64(params->line_num_range)+1), 1.f));
        ui_build_box_from_key(0, ui_key_zero());
    }
    
    //////////////////////////////
    //- rjf: build line text
    //
    UI_Parent(text_container_box) ProfScope("build line text") UI_Focus(UI_FocusKind_Off)
    {
        ui_set_next_pref_height(ui_px(params->line_height_px*(dim_1s64(params->line_num_range)+1), 1.f));
        UI_WidthFill
            UI_Column
            UI_PrefHeight(ui_px(params->line_height_px, 1.f))
            BP_Font(BP_FontSlot_Code)
            UI_FontSize(params->font_size)
            UI_CornerRadius(0)
        {
            u64 line_idx = 0;
            for(s64 line_num = params->line_num_range.min;
                line_num <= params->line_num_range.max; line_num += 1, line_idx += 1)
            {
                String8 line_string = params->line_text[line_idx];
                Rng1u64 line_range = params->line_ranges[line_idx];
                DR_FStr_List line_fstrs = lines_fstrs[line_idx];
                ui_set_next_text_padding(line_num_padding_px);
                UI_Key line_key = ui_key_from_stringf(top_container_box->key, "ln_%I64x", line_num);
                ui_set_next_tab_size(params->tab_size);
                UI_Box *line_box = ui_build_box_from_key(UI_BoxFlag_DisableTextTrunc|UI_BoxFlag_DrawText|UI_BoxFlag_DisableIDString, line_key);
                DR_Bucket *line_bucket = dr_bucket_make();
                dr_push_bucket(line_bucket);
                ui_box_equip_display_fstrs(line_box, &line_fstrs);
                
                if(!search_query_invalidated && params->search_query.size != 0)
                {
                    for(u64 needle_pos = 0; needle_pos < line_string.size;)
                    {
                        needle_pos = str8_find_needle(line_string, needle_pos, params->search_query, StringMatchFlag_CaseInsensitive);
                        if(needle_pos < line_string.size)
                        {
                            Rng1u64 match_range = r1u64(needle_pos, needle_pos+params->search_query.size);
                            Rng1f32 match_column_pixel_off_range =
                                {
                                    fnt_dim_from_tag_size_string(line_box->font, line_box->font_size, 0, params->tab_size, str8_prefix(line_string, match_range.min)).x,
                                    fnt_dim_from_tag_size_string(line_box->font, line_box->font_size, 0, params->tab_size, str8_prefix(line_string, match_range.max)).x,
                                };
                            Rng2f32 match_rect = {
                                line_box->rect.x0+line_num_padding_px+match_column_pixel_off_range.min,
                                line_box->rect.y0,
                                line_box->rect.x0+line_num_padding_px+match_column_pixel_off_range.max+2.f,
                                line_box->rect.y1,
                            };
                            Vec4f32 color = pop_color;
                            if(!is_focused)
                            {
                                color.w *= 0.5f;
                            }
                            color.w *= 0.2f;
                            dr_rect(match_rect, color, 4.f, 0, 1.f);
                            needle_pos += 1;
                        }
                    }
                }
                
                {
                    Rng1u64 prev_line_range = (line_idx > 0) ? params->line_ranges[line_idx-1] : r1u64(0, 0);
                    Rng1u64 next_line_range = (line_idx+1 < dim_1s64(params->line_num_range)) ? params->line_ranges[line_idx+1] : r1u64(0, 0);
                    for(Txt_Rng_Color_Pair_Node *n = first_txt_rng_color_pair; n != 0; n = n->next)
                    {
                        Rng1u64 select_range = n->range;
                        Rng1u64 select_range_in_line = intersect_1u64(select_range, line_range);
                        if(select_range_in_line.min < select_range_in_line.max)
                        {
                            Rng1u64 select_range_in_prev_line = intersect_1u64(prev_line_range, select_range);
                            Rng1u64 select_range_in_next_line = intersect_1u64(next_line_range, select_range);
                            bool32 prev_line_good = (select_range_in_prev_line.min < select_range_in_prev_line.max);
                            bool32 next_line_good = (select_range_in_next_line.min < select_range_in_next_line.max);
                            Rng1f32 select_column_pixel_off_range = {
                                fnt_dim_from_tag_size_string(line_box->font, line_box->font_size, 0, params->tab_size, str8_prefix(line_string, select_range_in_line.min - line_range.min)).x,
                                fnt_dim_from_tag_size_string(line_box->font, line_box->font_size, 0, params->tab_size, str8_prefix(line_string, select_range_in_line.max - line_range.min)).x,
                            };
                            Rng2f32 select_rect = {
                                line_box->rect.x0+line_num_padding_px+select_column_pixel_off_range.min-2.f,
                                floor_f32(line_box->rect.y0) - 1.f,
                                line_box->rect.x0+line_num_padding_px+select_column_pixel_off_range.max+2.f,
                                ceil_f32(line_box->rect.y1) + 1.f,
                            };
                            Vec4f32 color = n->color;
                            if(!is_focused)
                            {
                                color.w *= 0.5f;
                            }
                            f32 rounded_radius = params->font_size*0.4f;
                            R_Rect2D_Inst *inst = dr_rect(select_rect, color, rounded_radius, 0, 1);
                            Rng1u64 prev_line_selection_off_range = r1u64(select_range_in_prev_line.min - prev_line_range.min, select_range_in_prev_line.max - prev_line_range.min);
                            Rng1u64 next_line_selection_off_range = r1u64(select_range_in_next_line.min - next_line_range.min, select_range_in_next_line.max - next_line_range.min);
                            Rng1u64 crnt_line_selection_off_range = r1u64(select_range_in_line.min - line_range.min, select_range_in_line.max - line_range.min);
                            inst->corner_radii[Corner_00] = !prev_line_good || prev_line_selection_off_range.min > crnt_line_selection_off_range.min ? rounded_radius : 0.f;
                            inst->corner_radii[Corner_10] = (!prev_line_good || crnt_line_selection_off_range.max > prev_line_selection_off_range.max || crnt_line_selection_off_range.max < prev_line_selection_off_range.min) ? rounded_radius : 0.f;
                            inst->corner_radii[Corner_01] = (!next_line_good || next_line_selection_off_range.min > crnt_line_selection_off_range.min || next_line_selection_off_range.max < crnt_line_selection_off_range.min) ? rounded_radius : 0.f;
                            inst->corner_radii[Corner_11] = !next_line_good || crnt_line_selection_off_range.max > next_line_selection_off_range.max ? rounded_radius : 0.f;
                        }
                    }
                }
                
                if(line_range.min <= *cursor && *cursor <= line_range.max)
                {
                    Vec2f32 advance = fnt_dim_from_tag_size_string(line_box->font, line_box->font_size, 0, params->tab_size, str8_prefix(line_string, *cursor - line_range.min));
                    f32 cursor_y = text_container_box->rect.y0 + line_idx*params->line_height_px - params->font_size*0.125f;
                    f32 cursor_y__animated = ui_anim(ui_key_from_stringf(text_container_box->key, "cursor_y_px"), cursor_y, .initial = cursor_y);
                    f32 cursor_off_pixels = advance.x;
                    f32 cursor_off_pixels__animated = ui_anim(ui_key_from_stringf(text_container_box->key, "cursor_off_px"), cursor_off_pixels, .initial = cursor_off_pixels);
                    f32 cursor_thickness = ClampBot(1.f, floor_f32(line_box->font_size/10.f));
                    Rng2f32 cursor_rect =
                        {
                            ui_box_text_position(line_box).x+cursor_off_pixels,
                            line_box->rect.y0-params->font_size*0.125f,
                            ui_box_text_position(line_box).x+cursor_off_pixels+cursor_thickness,
                            line_box->rect.y1+params->font_size*0.125f,
                        };
                    Rng1f32 trail_off_span = r1f32(cursor_off_pixels__animated, cursor_off_pixels);
                    Rng2f32 trail_rect =
                        {
                            ui_box_text_position(line_box).x+trail_off_span.min,
                            line_box->rect.y0-params->font_size*0.125f,
                            ui_box_text_position(line_box).x+trail_off_span.max,
                            line_box->rect.y1+params->font_size*0.125f,
                        };
                    Vec4f32 cursor_color = ui_color_from_name(s("cursor"));
                    Vec4f32 trail_color = cursor_color;
                    if(!is_focused)
                    {
                        cursor_color.w *= 0.5f;
                    }
                    trail_color.w *= 0.25f;
                    dr_rect(cursor_rect, cursor_color, 1.f, 0, 0.f);
                    if(do_cursor_trail && !ui_key_match(ui_active_key(UI_MouseButtonKind_Left), text_container_box->key))
                    {
                        R_Rect2D_Inst *trail_inst = dr_rect(trail_rect, trail_color, ui_top_font_size()*0.2f, 0, 1.f);
                        trail_inst->shear = cursor_y - cursor_y__animated;
                        if(cursor_off_pixels > cursor_off_pixels__animated)
                        {
                            trail_inst->dst = shift_2f32(trail_inst->dst, v2f32(0, -trail_inst->shear));
                            trail_inst->colors[Corner_00].w *= 0.1f;
                            trail_inst->colors[Corner_01].w *= 0.1f;
                        }
                        else
                        {
                            trail_inst->shear *= -1;
                            trail_inst->colors[Corner_10].w *= 0.1f;
                            trail_inst->colors[Corner_11].w *= 0.1f;
                        }
                    }
                }
                
                if(line_bucket->passes.count != 0)
                {
                    ui_box_equip_draw_bucket(line_box, line_bucket);
                }
                
                dr_pop_bucket();
            }
        }
    }
    
    scratch_end(scratch);
    ProfEnd();
    return result;
}

internal BP_Code_Slice_Signal bp_code_slicef(BP_Code_Slice_Params *params, u64 *cursor, u64 *mark, s64 *preferred_column, char *fmt, ...)
{
    Temp scratch = scratch_begin(0, 0);
    va_list args;
    va_start(args, fmt);
    String8 string = push_str8fv(scratch.arena, fmt, args);
    BP_Code_Slice_Signal sig = bp_code_slice(params, cursor, mark, preferred_column, string);
    va_end(args);
    scratch_end(scratch);
    return sig;
}

internal bool32 bp_do_txt_controls(TXT_Text_Info *info, String8 data, TXT_Patch_List *pathces, u64 line_count_per_range, Txt_Pt *cursor, Txt_Pt *mark, s64 *preferred_column)
{
    Temp scratch = scratch_begin(0, 0);
    bool32 change = false;
    

    scratch_end(scratch);
    return change;
}

////////////////////////
// UI Widgets: Fancy Labels

internal DR_FStr_List bp_fstrs_from_rich_string(Arena *arena, String8 string)
{
    Temp scratch = scratch_begin(&arena, 1);
    typedef u32 StringPartFlags;
    enum {
        StringPartFlag_Code      = (1 << 0),
        StringPartFlag_Underline = (1 << 1),
        StringPartFlag_Bright    = (1 << 2),
    };
    typedef struct String_Part String_Part;
    struct String_Part {
        String_Part *next;
        StringPartFlags flags;
        String8 string;
    };
    String_Part *first_part = 0;
    String_Part *last_part = 0;
    u64 active_part_start_idx = 0;
    StringPartFlags active_part_flags = 0;
    for (u64 idx = 0; idx <= string.size; idx += 1)
    {
        if (idx == string.size)
        {
            String_Part *p = push_array(scratch.arena, String_Part, 1);
            p->flags = active_part_flags;
            p->string = str8_substr(string, r1u64(active_part_start_idx, idx));
            SLLQueuePush(first_part, last_part, p);
        }
        else if (string.str[idx] == '`')
        {
            String_Part *p = push_array(scratch.arena, String_Part, 1);
            p->flags = active_part_flags;
            p->string = str8_substr(string, r1u64(active_part_start_idx, idx));
            SLLQueuePush(first_part, last_part, p);
            active_part_start_idx = idx + 1;
            active_part_flags ^= StringPartFlag_Code;
        }
    }
    DR_FStr_List fstrs = {0};
    for (String_Part *p = first_part; p != 0; p = p->next)
    {
        DR_FStr fstr = {0};
        {
            fstr.string = p->string;
            fstr.params.font   = ui_top_font();
            fstr.params.color  = ui_color_from_name(str8_lit("text"));
            fstr.params.size   = ui_top_font_size();
            fstr.params.raster_flags = bp_raster_flags_from_slot(BP_FontSlot_Main);
            if (p->flags & StringPartFlag_Code)
            {
                fstr.params.font = bp_font_from_slot(BP_FontSlot_Code);
                fstr.params.raster_flags = bp_raster_flags_from_slot(BP_FontSlot_Code);
                fstr.params.color = bp_rgba_from_code_color_slot(BP_CodeColorSlot_CodeDefault);
            }
        }
        dr_fstrs_push(arena, &fstrs, &fstr);
    }
    scratch_end(scratch);
    return fstrs;
}

internal UI_Signal bp_label(String8 string)
{
    Temp scratch = scratch_begin(0, 0);
    DR_FStr_List fstrs = bp_fstrs_from_rich_string(scratch.arena, string);
    UI_Box *box = ui_build_box_from_key(UI_BoxFlag_DrawText, ui_key_zero());
    ui_box_equip_display_fstrs(box, &fstrs);
    UI_Signal sig = ui_signal_from_box(box);
    scratch_end(scratch);
    return sig;
}

internal UI_Signal bp_error_label(String8 string)
{
    UI_Box *box = ui_build_box_from_key(0, ui_key_zero());
    UI_Signal sig = ui_signal_from_box(box);
    UI_Parent(box)
    {
        ui_set_next_font(bp_font_from_slot(BP_FontSlot_Icons));
        ui_set_next_text_raster_flags(FNT_RasterFlag_Smooth);
        ui_set_next_text_alignment(UI_TextAlign_Center);
        UI_TagF("weak") UI_PrefWidth(ui_em(2.25f, 1.f)) ui_label(bp_icon_kind_text_table[BP_IconKind_WarningBig]);
        UI_PrefWidth(ui_text_dim(10, 0)) bp_label(string);
    }
    return sig;
}

internal bool32 bp_help_label(String8 string)
{
    bool32 result = false;
    UI_Box *box = ui_build_box_from_stringf(UI_BoxFlag_Clickable, "###%S_help_label", string);
    UI_Signal sig = ui_signal_from_box(box);
    UI_Parent(box)
    {
        UI_PrefWidth(ui_pct(1, 0)) ui_label(string);
        if (ui_hovering(sig)) UI_PrefWidth(ui_em(2.25f, 1))
        {
            result = true;
            ui_set_next_font(bp_font_from_slot(BP_FontSlot_Icons));
            ui_set_next_text_raster_flags(FNT_RasterFlag_Smooth);
            ui_set_next_text_alignment(UI_TextAlign_Center);
            UI_Box *help_hoverer = ui_build_box_from_stringf(UI_BoxFlag_DrawText|
                                                             UI_BoxFlag_DrawBorder|
                                                             UI_BoxFlag_DrawHotEffects,
                                                             "###help_hoverer_%S", string);
            ui_box_equip_display_string(help_hoverer, bp_icon_kind_text_table[BP_IconKind_QuestionMark]);
            if (!contains_2f32(help_hoverer->rect, ui_mouse()))
            {
                result = false;
            }
        }
    }
    return result;
}

internal DR_FStr_List bp_fstrs_from_code_string(Arena *arena, f32 alpha, bool32 indirection_size_change, Vec4f32 base_color, String8 string)
{
    ProfBeginFunction();
    Temp scratch = scratch_begin(&arena, 1);
    DR_FStr_List fstrs = {0};
    TXT_Token_Array tokens = txt_token_array_from_string__c_cpp(scratch.arena, 0, string);
    TXT_Token *tokens_opl = tokens.v + tokens.count;
    s32 indirection_counter = 0;
    indirection_size_change = 0;
    bool32 preceded_by_dot = 0;
    for (TXT_Token *token = tokens.v; token < tokens_opl; token += 1)
    {
        BP_CodeColorSlot token_color_slot = bp_code_color_slot_from_txt_token_kind(token->kind);
        Vec4f32 token_color_rgba = bp_rgba_from_code_color_slot(token_color_slot);
        String8 token_string = str8_substr(string, token->range);
        if (str8_match(token_string, str8_lit("{"), 0)) { indirection_counter += 1; }
        if (str8_match(token_string, str8_lit("["), 0)) { indirection_counter += 1; }
        indirection_counter = ClampBot(0, indirection_counter);
        switch (token->kind)
        {
            default: {
                {
                    token_color_rgba.w *= alpha;
                    DR_FStr fstr = {
                        token_string,
                        {
                            ui_top_font(),
                            ui_top_text_raster_flags(),
                            token_color_rgba,
                            ui_top_font_size() * (1.f - !!indirection_size_change * (indirection_counter/10.f)),
                        }
                    };
                    dr_fstrs_push(arena, &fstrs, &fstr);
                }
            } break;
            case TXT_TokenKind_Identifier:
            case TXT_TokenKind_Keyword: {
                {
                    BP_CodeColorSlot lookup_theme_color_slot = BP_CodeColorSlot_CodeDefault;
                    bool32 is_called = (token + 1 < tokens_opl &&
                                        token[1].kind == TXT_TokenKind_Symbol &&
                                        str8_match(str8_substr(string, token[1].range), str8_lit("("), 0));
                    if (!preceded_by_dot)
                    {
                        lookup_theme_color_slot = bp_code_color_slot_from_txt_token_kind_lookup_string(token->kind,
                                                                                                       token_string,
                                                                                                       1,
                                                                                                       is_called);
                    }
                    if (lookup_theme_color_slot != BP_CodeColorSlot_CodeDefault)
                    {
                        Vec4f32 lookup_color = bp_rgba_from_code_color_slot(lookup_theme_color_slot);
                        f32 lookup_color_mix_t = ui_anim(ui_key_from_stringf(ui_key_zero(), "%S_lookup", token_string), 1.f);
                        token_color_rgba = mix_4f32(token_color_rgba, lookup_color, lookup_color_mix_t);
                    }
                    token_color_rgba.w *= alpha;
                    DR_FStr fstr = {
                        token_string,
                        {
                            ui_top_font(),
                            ui_top_text_raster_flags(),
                            token_color_rgba,
                            ui_top_font_size() * (1.f - !!indirection_size_change * (indirection_counter/10.f)),
                        },
                    };
                    dr_fstrs_push(arena, &fstrs, &fstr);
                }
            } break;
            case TXT_TokenKind_Numeric: {
                {
                    token_color_rgba.w *= alpha;
                    Vec4f32 token_color_rgba_alt = bp_rgba_from_code_color_slot(BP_CodeColorSlot_CodeNumericAltDigitGroup);
                    token_color_rgba_alt.w *= alpha;
                    f32 font_size = ui_top_font_size() * (1.f - !!indirection_size_change * (indirection_counter/10.f));

                    // unpack string
                    u32 base = 10;
                    u64 prefix_skip = 0;
                    u64 digit_group_size = 3;
                    if (str8_match(str8_prefix(token_string, 2), str8_lit("0x"), StringMatchFlag_CaseInsensitive))
                    {
                        base = 16;
                        prefix_skip = 2;
                        digit_group_size = 4;
                    }
                    else if (str8_match(str8_prefix(token_string, 2), str8_lit("0b"), StringMatchFlag_CaseInsensitive))
                    {
                        base = 2;
                        prefix_skip = 2;
                        digit_group_size = 8;
                    }
                    else if (str8_match(str8_prefix(token_string, 2), str8_lit("0o"), StringMatchFlag_CaseInsensitive))
                    {
                        base = 8;
                        prefix_skip = 2;
                        digit_group_size = 2;
                    }

                    // grab string parts
                    u64 dot_pos = str8_find_needle(token_string, 0, str8_lit("."), 0);
                    String8 prefix = str8_prefix(token_string, prefix_skip);
                    String8 whole = str8_substr(token_string, r1u64(prefix_skip, dot_pos));
                    String8 decimal = str8_skip(token_string, dot_pos);

                    // determine # of digits
                    u64 num_digits = 0;
                    for (u64 idx = 0; idx < whole.size; idx += 1)
                    {
                        num_digits += char_is_digit(whole.str[idx], base);
                    }

                    // push prefix
                    {
                        DR_FStr fstr = {
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

                    // push digit groups
                    {
                        bool32 odd = false;
                        u64 start_idx = 0;
                        u64 num_digits_passed = digit_group_size - num_digits % digit_group_size;
                        for (u64 idx = 0; idx <= whole.size; idx += 1)
                        {
                            u8 byte = idx < whole.size ? whole.str[idx] : 0;
                            if (num_digits_passed >= digit_group_size || idx == whole.size)
                            {
                                num_digits_passed = 0;
                                if (start_idx < idx)
                                {
                                    DR_FStr fstr = {
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
                            if (char_is_digit(byte, base))
                            {
                                num_digits_passed += 1;
                            }
                        }
                    }

                    // push decimal
                    {
                        DR_FStr fstr = {
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
                }
            } break;
        }
        if (token->kind == TXT_TokenKind_Symbol && str8_match(token_string, str8_lit("."), 0))
        {
            preceded_by_dot = true;
        }
        else
        {
            preceded_by_dot = false;
        }
        if (str8_match(token_string, str8_lit("}"), 0)) { indirection_counter -= 1; }
        if (str8_match(token_string, str8_lit("]"), 0)) { indirection_counter -= 1; }
        indirection_counter = ClampBot(0, indirection_counter);
    }
    scratch_end(scratch);
    ProfEnd();
    return fstrs;
}

internal UI_Box *bp_code_label(f32 alpha, bool32 indirection_size_change, Vec4f32 base_color, String8 string)
{
    Temp scratch = scratch_begin(0, 0);
    DR_FStr_List fstrs = bp_fstrs_from_code_string(scratch.arena, alpha, indirection_size_change, base_color, string);
    UI_Box *box = ui_build_box_from_key(UI_BoxFlag_DrawText, ui_key_zero());
    ui_box_equip_display_fstrs(box, &fstrs);
    scratch_end(scratch);
    return box;
}


////////////////////
// UI Widgets: Line Edit

internal UI_Signal bp_cell(BP_Cell_Params *params, String8 string)
{
    ProfBeginFunction();
    Temp scratch = scratch_begin(0, 0);
    bool32 do_cursor_trail = bp_setting_bool32_from_name(s("animations")) && bp_setting_bool32_from_name(str8_lit("cursor_trail"));
    /////////////////////////
    // unpack visual metrics
    f32 expander_size_px = floor_f32(ui_top_font_size() * 2.f);

    //////////////
    // make key
    UI_Key key = ui_key_from_string(ui_active_seed_key(), string);

    ///////////////////////
    // calculate & push focus
    bool32 is_auto_focus_hot = ui_is_key_auto_focus_hot(key);
    bool32 is_auto_focus_active = ui_is_key_auto_focus_active(key);
    if (is_auto_focus_hot)    { ui_push_focus_hot(UI_FocusKind_On); }
    if (is_auto_focus_active) { ui_push_focus_active(UI_FocusKind_On); }
    bool32 is_focus_hot    = ui_is_focus_hot();
    bool32 is_focus_active = ui_is_focus_active();
    bool32 is_focus_hot_disabled = (!is_focus_hot && ui_top_focus_hot() == UI_FocusKind_On);
    bool32 is_focus_active_disabled = (!is_focus_hot && ui_top_focus_active() == UI_FocusKind_On);

    // determine which sub-cell components we'll need
    //
    // (the base line edit textual label / editor is always built, but this can be enriched
    // with extra widgets & metadata)
    //
    bool32 build_toggle_switch = !!(params->flags & BP_CellFlag_ToggleSwitch) && !is_focus_active;
    bool32 build_slider        = !!(params->flags & BP_CellFlag_Slider)       && !is_focus_active;
    bool32 build_bindings      = !!(params->flags & BP_CellFlag_Bindings)     && !is_focus_active;
    bool32 build_lhs_name_desc = (params->meta_fstrs.node_count != 0 || params->description.size != 0);
    bool32 build_line_edit     = (params->pre_edit_value.size != 0 || params->value_fstrs.node_count != 0);
    bool32 build_note          = (params->note_fstrs.node_count != 0 && !is_focus_active);
    DR_FStr_List lhs_name_fstrs   = params->meta_fstrs;
    DR_FStr_List value_name_fstrs = params->value_fstrs;
    DR_FStr_List note_fstrs       = params->note_fstrs;

    //////////////////////////////////
    // determine autocompletion string
    String8 autocomplete_hint_string = {0};
    if (is_focus_active)
    {
        autocomplete_hint_string = ui_autocomplete_string();
    }

    ////////////////////////
    // build top-level box
    if (is_focus_active || is_focus_active_disabled)
    {
        ui_set_next_hover_cursor(WM_Cursor_IBar);
    }
    UI_Box *box = ui_build_box_from_key(UI_BoxFlag_MouseClickable|
                                        (!!build_lhs_name_desc * UI_BoxFlag_DisableFocusBorder)|
                                        (!!(params->flags & BP_CellFlag_KeyboardClickable) * UI_BoxFlag_KeyboardClickable)|
                                        UI_BoxFlag_ClickToFocus|
                                        (!!(params->flags & BP_CellFlag_Button) * UI_BoxFlag_DrawHotEffects)|
                                        (!!(params->flags & BP_CellFlag_SingleClickActivate) * UI_BoxFlag_DrawActiveEffects)|
                                        (!(params->flags & BP_CellFlag_NoBackground) * UI_BoxFlag_DrawBackground)|
                                        (!!(params->flags & BP_CellFlag_Border) * UI_BoxFlag_DrawBorder)|
                                        ((is_auto_focus_hot || is_auto_focus_active) * UI_BoxFlag_KeyboardClickable)|
                                        (is_focus_active || is_focus_active_disabled) * (UI_BoxFlag_Clip),
                                        key);
    ////////////////////
    // build indent
    UI_Parent(box) for(s32 idx = 0; idx < params->depth; idx += 1)
    {
        ui_set_next_flags(UI_BoxFlag_DrawSideLeft);
        ui_spacer(ui_em(1.f, 1.f));
    }

    //////////////////////////////////////////////
    // build expander (or placeholder, or space)
    {
        // build expander
        if (params->flags & BP_CellFlag_Expander) UI_PrefWidth(ui_px(expander_size_px, 1.f)) UI_Parent(box)
            UI_Flags(UI_BoxFlag_DrawSideLeft)
            UI_Focus(UI_FocusKind_Off)
        {
            UI_Signal expander_sig = ui_expanderf(params->expanded_out[0], "expander");
            if (ui_pressed(expander_sig))
            {
                params->expanded_out[0] ^= 1;
            }
        }

        // build expander placeholder
        else if (params->flags & BP_CellFlag_ExpanderPlaceholder) UI_Parent(box) UI_PrefWidth(ui_px(expander_size_px, 1.f)) UI_Focus(UI_FocusKind_Off)
        {
            UI_TagF("weak")
                UI_Flags(UI_BoxFlag_DrawSideLeft)
                BP_Font(BP_FontSlot_Icons)
                UI_TextAlignment(UI_TextAlign_Center)
                ui_label(bp_icon_kind_text_table[BP_IconKind_Dot]);
        }

        // build expander space
        else if (params->flags & BP_CellFlag_ExpanderSpace) UI_Parent(box) UI_Focus(UI_FocusKind_Off)
        {
            UI_Flags(UI_BoxFlag_DrawSideLeft) ui_spacer(ui_px(expander_size_px, 1.f));
        }
    }

    /////////////////////////////////
    // build left-hand-side container box
    UI_Box *lhs_box = &ui_nil_box;
    if (build_lhs_name_desc)
    {
        UI_Parent(box) UI_WidthFill UI_ChildLayoutAxis(Axis2_Y)
        {
            if (ui_top_text_alignment() == UI_TextAlign_Left &&
                (params->flags & (BP_CellFlag_Expander | BP_CellFlag_ExpanderSpace | BP_CellFlag_ExpanderPlaceholder)) ==0)
            {
                ui_spacer(ui_em(1.f, 1.f));
            }
            lhs_box = ui_build_box_from_stringf(0, "lhs_box");
        }
    }

    // build left-hand-side name/desc box
    if (build_lhs_name_desc) UI_Parent(lhs_box) UI_Padding(ui_em(3.f, 0.f)) UI_WidthFill UI_HeightFill
    {
        Fuzzy_Match_Range_List fuzzy_matches = {0};
        if (params->search_needle.size != 0)
        {
            fuzzy_matches = dr_fuzzy_match_find_from_fstrs(scratch.arena, &lhs_name_fstrs, params->search_needle);
        }
        UI_Row
        {
            UI_Box *name_box = ui_build_box_from_key(UI_BoxFlag_DrawText, ui_key_zero());
            ui_box_equip_display_fstrs(name_box, &lhs_name_fstrs);
            ui_box_equip_fuzzy_match_ranges(name_box, &fuzzy_matches);
        }
        if (params->description.size != 0) BP_Font(BP_FontSlot_Main) UI_FontSize(ui_top_font_size() * 0.85f)
        {
            UI_Row
            {
                UI_Box *desc_box = ui_label(params->description).box;
                Fuzzy_Match_Range_List desc_fuzzy_matches = fuzzy_match_find(scratch.arena,
                                                                             params->search_needle,
                                                                             params->description);
                ui_box_equip_fuzzy_match_ranges(desc_box, &desc_fuzzy_matches);
            }
        }
    }

    ///////////////////////////////////
    // build line edit container box
    UI_Box *edit_box = &ui_nil_box;
    f32 editable_edit_box_dim = dim_2f32(box->rect).x;
    if ((is_focus_active || is_focus_active_disabled) || build_line_edit)
        UI_Parent(box)
    {
        bool32 is_editing = (is_focus_active || is_focus_active_disabled);
        UI_Size edit_box_size = ui_pct(1, 0);
        if (build_lhs_name_desc)
        {
            if (is_editing)
            {
                editable_edit_box_dim = floor_f32(dim_2f32(box->rect).x * 0.5f);
                edit_box_size = ui_px(editable_edit_box_dim, 1.f);
            }
            else
            {
                edit_box_size = ui_children_sum(1);
            }
        }
        UI_PrefWidth(edit_box_size)
        {
            if (ui_top_px_height() > ui_top_font_size() * 3.f)
            {
                ui_set_next_pref_width(ui_children_sum(1));
                UI_Column UI_Padding(ui_em(1, 0)) UI_Focus(UI_FocusKind_On)
                {
                    UI_PrefHeight(ui_em(3.f, 1.f)) UI_CornerRadius(ui_top_font_size() * 0.5f)
                        edit_box = ui_build_box_from_stringf((!!is_editing * UI_BoxFlag_DrawBorder)|
                                                             UI_BoxFlag_Clickable|
                                                             UI_BoxFlag_DisableFocusOverlay,
                                                             "edit_box");
                    if (params->line_edit_key_out)
                    {
                        params->line_edit_key_out[0] = edit_box->key;
                    }
                }
                if (ui_top_text_alignment() == UI_TextAlign_Left)
                {
                    ui_spacer(ui_em(1.f, 1.f));
                }
            }
            else
            {
                edit_box = ui_build_box_from_stringf(0, "edit_box");
                if (params->line_edit_key_out)
                {
                    params->line_edit_key_out[0] = edit_box->key;
                }
            }
        }
    }

    ////////////////////////////////////////////////////////////////////
    // build edit-button, if line edit is embedded, and has no string
    bool32 edit_started = false;
    if (params->flags & BP_CellFlag_EmptyEditButton &&
        !is_focus_active &&
        !is_focus_active_disabled &&
        build_lhs_name_desc &&
        build_line_edit &&
        value_name_fstrs.total_size == 0)
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
                UI_CornerRadius(ui_top_font_size() * 0.5f)
                BP_Font(BP_FontSlot_Icons)
                UI_TextAlignment(UI_TextAlign_Center)
            {
                UI_Box *edit_start_box = ui_build_box_from_stringf(UI_BoxFlag_DrawText|
                                                                   UI_BoxFlag_DrawHotEffects|
                                                                   UI_BoxFlag_DrawBorder|
                                                                   UI_BoxFlag_DrawBackground|
                                                                   UI_BoxFlag_DisableFocusOverlay|
                                                                   UI_BoxFlag_DisableFocusBorder|
                                                                   UI_BoxFlag_Clickable,
                                                                   "%S##edit", bp_icon_kind_text_table[BP_IconKind_Pencil]);
                UI_Signal sig = ui_signal_from_box(edit_start_box);
                if (ui_pressed(sig))
                {
                    edit_started = true;
                }
            }
            ui_spacer(ui_em(1.f, 1.f));
        }
    }
    
    ///////////////////////////////////
    // build scrollable container box
    UI_Box *scrollable_box = &ui_nil_box;
    if (edit_box != &ui_nil_box)
    {
        UI_Parent(edit_box) UI_PrefWidth(ui_children_sum(0))
        {
            scrollable_box = ui_build_box_from_stringf(is_focus_active * (UI_BoxFlag_AllowOverflowX|UI_BoxFlag_Clip),
                                                       "scroll_box_%p",
                                                       params->edit_buffer);
        }
    }

    /////////////////////////
    // build revert-button
    if (params->flags & BP_CellFlag_RevertButton &&
        !is_focus_active &&
        !is_focus_active_disabled)
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
                UI_CornerRadius(ui_top_font_size() * 0.5f)
                BP_Font(BP_FontSlot_Icons)
                UI_TextAlignment(UI_TextAlign_Center)
            {
                UI_Box *revert_box = ui_build_box_from_stringf(UI_BoxFlag_DrawText|
                                                               UI_BoxFlag_DrawHotEffects|
                                                               UI_BoxFlag_DrawBorder|
                                                               UI_BoxFlag_DrawBackground|
                                                               UI_BoxFlag_DisableFocusOverlay|
                                                               UI_BoxFlag_DisableFocusBorder|
                                                               UI_BoxFlag_Clickable,
                                                               "%S##revert",
                                                               bp_icon_kind_text_table[BP_IconKind_Undo]);
                UI_Signal sig = ui_signal_from_box(revert_box);
                if (ui_hovering(sig)) UI_Tooltip BP_Font(BP_FontSlot_Main)
                {
                    ui_state->tooltip_anchor_key = revert_box->key;
                    ui_label(str8_lit("Revert To Default"));
                }
                if (ui_pressed(sig) && params->revert_out)
                {
                    params->revert_out[0] = 1;
                }
            }
            // TODO: @hack
            if (build_toggle_switch || build_slider)
            {
                ui_spacer(ui_em(1.f, 1.f));
            }
        }
    }

    ////////////////////////
    // build toggle-switch
    if (build_toggle_switch) UI_Parent(box)
    {
        bool32 is_toggled = !!params->toggled_out[0];
        f32 toggle_t = ui_anim(ui_key_from_stringf(key, "toggled"),
                               (f32)is_toggled,
                               .initial = (f32)is_toggled,
                               .rate = bp_state->menu_animation_rate);
        f32 height_px = ceil_f32(ui_top_font_size() * 1.75f);
        f32 padding_px = ceil_f32((ui_top_px_height() - height_px) / 2.f);
        UI_PrefWidth(ui_children_sum(1.f))
            UI_HeightFill
            UI_Column UI_Padding(ui_px(padding_px, 1.f))
            UI_Row
        {
            if (ui_top_text_alignment() == UI_TextAlign_Center)
            {
                ui_spacer(ui_em(1.f, 0.f));
            }
            UI_PrefWidth(ui_em(3.5f, 1.f))
                UI_CornerRadius(floor_f32(height_px / 2.f - 1.f))
                UI_TagF(is_toggled ? "good_pop" : "")
                UI_GroupKey(ui_key_from_stringf(ui_key_zero(), "toggle_switch_group_key"))
            {
                UI_Box *switch_box = ui_build_box_from_stringf(UI_BoxFlag_DrawHotEffects|
                                                               UI_BoxFlag_DrawBorder|
                                                               UI_BoxFlag_DrawBackground|
                                                               UI_BoxFlag_Clickable,
                                                               "toggle_switch");
                UI_Parent(switch_box)
                {
                    BP_Font(BP_FontSlot_Icons) UI_PrefWidth(ui_pct(toggle_t, 0)) UI_Transparency(1.f - toggle_t)
                    {
                        ui_build_box_from_stringf(UI_BoxFlag_DisableTextTrunc |
                                                  (toggle_t > 0.001f ? UI_BoxFlag_DrawText : 0),
                                                  "%S", bp_icon_kind_text_table[BP_IconKind_Check]);
                    }
                    UI_BackgroundColor(ui_color_from_name(str8_lit("text")))
                        UI_PrefWidth(ui_px(height_px, 1.f))
                    {
                        f32 extratoggler_padding_px = floor_f32(ui_top_font_size() * 0.35f);
                        f32 toggler_size_px = ceil_f32(height_px - extratoggler_padding_px * 2.f) - 1.f;
                        UI_Column UI_Padding(ui_px(extratoggler_padding_px, 1.f))
                            UI_Row UI_Padding(ui_px(extratoggler_padding_px, 1.f))
                            UI_PrefWidth(ui_px(toggler_size_px, 1.f))
                            UI_PrefHeight(ui_px(toggler_size_px, 1.f))
                            UI_CornerRadius(floor_f32(toggler_size_px/2.f - 1.f))
                        {
                            ui_build_box_from_key(UI_BoxFlag_DrawBackground | UI_BoxFlag_DrawDropShadow,
                                                  ui_key_zero());
                        }
                    }
                    ui_spacer(ui_pct(1.f - toggle_t, 0));
                }
                UI_Signal switch_sig = ui_signal_from_box(switch_box);

                // press -> toggle, & gather this key
                if (ui_pressed(switch_sig))
                {
                    if (ui_dragging(switch_sig))
                    {
                        ui_store_drag_struct(&switch_box->key);
                    }
                    params->toggled_out[0] ^= 1;
                }

                // dragging -> check if key is in batch of touched keys. if so, do nothing, otherwise, toggle.
                // always store this new key if not in batch
                if (ui_dragging(switch_sig))
                {
                    String8 all_keys_data = ui_get_drag_data(sizeof(UI_Key));
                    UI_Key *keys = (UI_Key *)all_keys_data.str;
                    u64 keys_count = all_keys_data.size / sizeof(UI_Key);
                    bool32 key_is_touched = false;
                    for EachIndex(idx, keys_count)
                    {
                        if (ui_key_match(keys[idx], switch_box->key))
                        {
                            key_is_touched = true;
                            break;
                        }
                    }
                    if (!key_is_touched)
                    {
                        params->toggled_out[0] ^= 1;
                        UI_Key *new_keys = push_array(scratch.arena, UI_Key, keys_count + 1);
                        MemoryCopy(new_keys, keys, sizeof(UI_Key) * keys_count);
                        new_keys[keys_count] = switch_box->key;
                        ui_store_drag_data(str8((u8 *)new_keys, sizeof(UI_Key) * (keys_count + 1)));
                    }
                }
            }
            if (ui_top_text_alignment() == UI_TextAlign_Center)
            {
                ui_spacer(ui_em(1.f, 0.f));
            }
        }
        if (ui_top_text_alignment() == UI_TextAlign_Left)
        {
            ui_spacer(ui_em(1.f, 1.f));
        }
    }

    /////////////////
    // build slider
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
            UI_Box *slider_box = ui_build_box_from_stringf(UI_BoxFlag_DrawHotEffects|
                                                           UI_BoxFlag_DrawBorder|
                                                           UI_BoxFlag_DrawBackground|
                                                           UI_BoxFlag_Clickable,
                                                           "slider");
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
    if(build_bindings) UI_Parent(box) BP_Font(BP_FontSlot_Main) UI_PermissionFlags(UI_PermissionFlag_Clicks)
    {
        UI_PrefWidth(ui_children_sum(1)) UI_Column UI_Padding(ui_px(ui_top_px_height()*0.2f, 1.f)) UI_HeightFill
        {
            UI_PrefWidth(ui_children_sum(1)) UI_Row UI_Padding(ui_em(1.f, 1.f))
            {
                bp_cmd_binding_buttons(params->bindings_name, params->search_needle, max_u64, BP_CmdBindingButtonFlag_AddNew);
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
            if(!(params->flags & BP_CellFlag_Button))
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
    if(!(params->flags & BP_CellFlag_DisableEdit) && (is_focus_active || focus_started))
    {
        Temp scratch = scratch_begin(0, 0);
        bp_state->text_edit_mode = 1;
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
                CFG_Node *window = cfg_node_from_id(bp_regs()->window);
                BP_Window_State *ws = bp_window_state_from_cfg(window);
                BP_Autocomp_Cursor_Info *autocomp_cursor_info = &ws->autocomp_cursor_info;
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
        else if(!is_focus_active && !is_focus_active_disabled && params->flags & BP_CellFlag_CodeContents && params->pre_edit_value.size != 0)
        {
            String8 display_string = params->pre_edit_value;
            fstrs = bp_fstrs_from_code_string(scratch.arena, 1, 0, ui_color_from_name(str8_lit("text")), display_string);
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
            if(params->flags & BP_CellFlag_CodeContents)
            {
                edit_string_fstrs = bp_fstrs_from_code_string(scratch.arena, 1.f, 0, ui_color_from_name(str8_lit("text")), edit_string);
            }
            else
            {
                String8 edit_string = str8(params->edit_buffer, params->edit_string_size_out[0]);
                DR_FStr_Params params = {ui_top_font(), ui_top_text_raster_flags(), ui_color_from_name(str8_lit("text")), ui_top_font_size()};
                dr_fstrs_push_new(scratch.arena, &edit_string_fstrs, &params, edit_string);
            }
            if(autocomplete_hint_string.size != 0)
            {
                CFG_Node *window = cfg_node_from_id(bp_regs()->window);
                BP_Window_State *ws = bp_window_state_from_cfg(window);
                BP_Autocomp_Cursor_Info *autocomp_cursor_info = &ws->autocomp_cursor_info;
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
    // build scrolled contents
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
        if(ui_top_text_alignment() == UI_TextAlign_Left &&
           (params->flags & (BP_CellFlag_Expander|BP_CellFlag_ExpanderSpace|BP_CellFlag_ExpanderPlaceholder)) == 0)
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
            if(params->flags & BP_CellFlag_CodeContents)
            {
                font = bp_font_from_slot(BP_FontSlot_Code);
            }
            mouse_off = fnt_char_pos_from_tag_size_string_p(font,
                                                            font_size,
                                                            0,
                                                            ui_top_tab_size(),
                                                            edit_string,
                                                            text2mouse.x);
            cursor_off = fnt_dim_from_tag_size_string(ui_top_font(),
                                                      ui_top_font_size(),
                                                      0,
                                                      ui_top_tab_size(),
                                                      str8_prefix(edit_string, params->cursor[0])).x;
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

internal UI_Signal bp_cellf(BP_Cell_Params *params, char *fmt, ...)
{
    Temp scratch = scratch_begin(0, 0);
    va_list args;
    va_start(args, fmt);
    String8 string = push_str8fv(scratch.arena, fmt, args);
    va_end(args);
    UI_Signal sig = bp_cell(params, string);
    scratch_end(scratch);
    return sig;
}



