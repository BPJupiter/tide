
////////////////////////
// Code View Functions

internal void bp_code_view_init(BP_Code_View_State *cv)
{
    ProfBeginFunction();
    if (cv->initialised == false)
    {
        cv->initialised = true;
        cv->preferred_column = 1;
        cv->patch_arena = bp_push_view_arena();
        cv->find_text_arena = bp_push_view_arena();
        cv->center_cursor = 1;
        bp_store_view_loading_info(1, 0, 0);
    }
    ProfEnd();
}

internal void bp_code_view_build(Arena *arena, BP_Code_View_State *cv, Rng2f32 rect, String8 text_data, TXT_Text_Info *text_info)
{
    ProfBeginFunction();
    Temp scratch = scratch_begin(&arena, 1);
    
    //////////////////////////////
    //- rjf: unpack state
    //
    UI_Scroll_Pt2 scroll_pos = bp_view_scroll_pos();
  
    //////////////////////////////
    //- rjf: extract invariants
    //
    f32 main_font_size = ui_bottom_font_size();
    FNT_Tag code_font = bp_font_from_slot(BP_FontSlot_Code);
    f32 code_font_size = ui_top_font_size();
    f32 code_tab_size = fnt_column_size_from_tag_size(code_font, code_font_size)*bp_setting_u64_from_name(str8_lit("tab_width"));
    FNT_Metrics code_font_metrics = fnt_metrics_from_tag_size(code_font, code_font_size);
    f32 code_line_height = ceil_f32(fnt_line_height_from_metrics(&code_font_metrics) * 1.5f);
    f32 big_glyph_advance = fnt_dim_from_tag_size_string(code_font, code_font_size, 0, 0, str8_lit("H")).x;
    Vec2f32 panel_box_dim = dim_2f32(rect);
    f32 scroll_bar_dim = floor_f32(main_font_size*1.5f);
    Vec2f32 code_area_dim = v2f32(panel_box_dim.x - scroll_bar_dim, panel_box_dim.y - scroll_bar_dim);
    s64 num_possible_visible_lines = (s64)(code_area_dim.y/code_line_height)+1;
    bool32 do_line_numbers = bp_setting_b32_from_name(str8_lit("show_line_numbers"));
    bool32 text_is_ready = (text_info->lines_count != 0);
    
    //////////////////////////////
    //- rjf: process commands
    //
    for(BP_Cmd *cmd = 0; bp_next_view_cmd(&cmd);)
    {
        BP_CmdKind kind = bp_cmd_kind_from_string(cmd->name);
        switch(kind)
        {
            default: break;
            case BP_CmdKind_GoToLine: {
                {
                    cv->goto_line_num = cmd->regs->line_num;
                }
            }break;
            case BP_CmdKind_CenterCursor: {
                {
                    cv->center_cursor = 1;
                }
            }break;
            case BP_CmdKind_ContainCursor: {
                {
                    cv->contain_cursor = 1;
                }
            }break;
            case BP_CmdKind_Search: {
                {
                    arena_clear(cv->find_text_arena);
                    cv->find_text_fwd = str8_copy(cv->find_text_arena, cmd->regs->string);
                }
            }break;
            case BP_CmdKind_SearchBackwards: {
                {
                    arena_clear(cv->find_text_arena);
                    cv->find_text_bwd = str8_copy(cv->find_text_arena, cmd->regs->string);
                }
            }break;
            case BP_CmdKind_FindNext: {
                {
                    String8 string = bp_view_query_input();
                    arena_clear(cv->find_text_arena);
                    cv->find_text_fwd = str8_copy(cv->find_text_arena, string);
                }
            }break;
            case BP_CmdKind_FindPrev: {
                {
                    String8 string = bp_view_query_input();
                    arena_clear(cv->find_text_arena);
                    cv->find_text_bwd = str8_copy(cv->find_text_arena, string);
                }
            }break;
        }
    }
    
    //////////////////////////////
    //- rjf: set up wrap cache
    //
    if(cv->wrap_arena == 0)
    {
        cv->wrap_arena = bp_push_view_arena();
    }
    if(cv->wrap_total_vline_count == 0)
    {
        arena_clear(cv->wrap_arena);
        cv->wrap_total_vline_count = text_info->lines_count;
        cv->wrap_cache_slots_count = text_info->lines_count/64;
        cv->wrap_cache_slots = push_array(cv->wrap_arena, BP_CodeViewTLineWrapCacheSlot, cv->wrap_cache_slots_count);
    }
    
    //////////////////////////////
    //- rjf: do keyboard interaction, compute patched text state
    //
    TXT_Patched text_patched = txt_patched_from_info_data_patches(scratch.arena, text_info, text_data, &cv->patches);
    bool32 snap[Axis2_COUNT] = {0};
    UI_Focus(UI_FocusKind_On) if(ui_is_focus_active())
    {
        CFG_Node *view = cfg_node_from_id(bp_regs()->view);
        BP_ViewState *vs = bp_view_state_from_cfg(view);
        bp_state->text_edit_mode_multiline = (!vs->query_is_open || vs->contents_are_focused);
        bp_state->text_edit_mode = 1;
        u64 line_count_per_page = ClampBot(num_possible_visible_lines, 10) - 10;
        u64 *cursor = &bp_regs()->cursor;
        u64 *mark = &bp_regs()->mark;
        s64 *preferred_column = &cv->preferred_column;
        for(UI_Event *evt = 0; ui_next_event(&evt);)
        {
            if(evt->kind != UI_EventKind_Navigate && evt->kind != UI_EventKind_Edit && evt->kind != UI_EventKind_Text)
            {
                continue;
            }
            bool32 taken = 0;
            u64 start_cursor = *cursor;
            u64 start_mark = *mark;
            Vec2s32 delta = evt->delta_2s32;
            u64 line_count = text_patched.line_map.total_line_count;
            u64 line_num = txt_line_num_from_off(&text_patched.line_map, *cursor);
            Rng1u64 line_range = txt_range_from_line_num(&text_patched.line_map, line_num);
            String8 line = {0};
            line.size = dim_1u64(line_range);
            line.str = push_array(scratch.arena, u8, line.size);
            memory_map_read(&text_patched.memory_map, line_range, line.str);
            
            UI_Txt_Op single_line_op = ui_single_line_txt_op_from_event(scratch.arena, evt, line, line_range, *cursor, *mark);
            
            if(single_line_op.replace.size != 0 || evt->flags & UI_EventFlag_Delete)
            {
                continue;
            }
            
            *cursor = single_line_op.cursor;
            *mark = single_line_op.mark;
            
            bool32 need_nav = (start_cursor == start_mark || !(evt->flags & UI_EventFlag_ZeroDeltaOnSelect));
            
            if(need_nav && evt->delta_unit != UI_EventDeltaUnit_Whole && evt->delta_unit != UI_EventDeltaUnit_Line && delta.x > 0 && start_cursor == line_range.max && line_num+1 <= line_count)
            {
                Rng1u64 next_line_range = txt_range_from_line_num(&text_patched.line_map, line_num+1);
                *cursor = next_line_range.min;
                *preferred_column = 1;
            }
            
            if(need_nav && evt->delta_unit != UI_EventDeltaUnit_Whole && evt->delta_unit != UI_EventDeltaUnit_Line && delta.x < 0 && start_cursor == line_range.min && line_num-1 >= 1)
            {
                Rng1u64 prev_line_range = txt_range_from_line_num(&text_patched.line_map, line_num-1);
                *cursor = prev_line_range.max;
                *preferred_column = (s64)dim_1u64(prev_line_range)+1;
            }
            
            if(need_nav && evt->delta_unit == UI_EventDeltaUnit_Char && delta.y > 0 && line_num+1 <= line_count)
            {
                Rng1u64 next_line_range = txt_range_from_line_num(&text_patched.line_map, line_num+1);
                *cursor = next_line_range.min + *preferred_column;
                *cursor = clamp_1u64(next_line_range, *cursor);
            }
            
            if(need_nav && evt->delta_unit == UI_EventDeltaUnit_Char && delta.y < 0 && line_num > 1)
            {
                Rng1u64 prev_line_range = txt_range_from_line_num(&text_patched.line_map, line_num-1);
                *cursor = prev_line_range.min + *preferred_column;
                *cursor = clamp_1u64(prev_line_range, *cursor);
            }
            
            if(need_nav && evt->delta_unit == UI_EventDeltaUnit_Word && delta.y > 0 && line_num+1 <= line_count)
            {
                bool32 done = 0;
                for(u64 scan_line_num = line_num+1; !done && scan_line_num <= line_count; scan_line_num += 1)
                {
                    Temp scratch = scratch_begin(&arena, 1);
                    Rng1u64 line_range = txt_range_from_line_num(&text_patched.line_map, scan_line_num);
                    String8 line = memory_map_data_from_range(scratch.arena, &text_patched.memory_map, line_range);
                    String8 line_without_whitespace = str8_skip_chop_whitespace(line);
                    if(line_without_whitespace.size == 0)
                    {
                        *cursor = line_range.min + (u64)(line_without_whitespace.str - line.str);
                        done = 1;
                    }
                    else if(scan_line_num == line_count)
                    {
                        *cursor = text_patched.size;
                    }
                    scratch_end(scratch);
                }
            }
            
            if(need_nav && evt->delta_unit == UI_EventDeltaUnit_Word && delta.y < 0 && line_num > 1)
            {
                bool32 done = 0;
                for(u64 scan_line_num = line_num-1; !done && scan_line_num > 0; scan_line_num -= 1)
                {
                    Temp scratch = scratch_begin(&arena, 1);
                    Rng1u64 line_range = txt_range_from_line_num(&text_patched.line_map, scan_line_num);
                    String8 line = memory_map_data_from_range(scratch.arena, &text_patched.memory_map, line_range);
                    String8 line_without_whitespace = str8_skip_chop_whitespace(line);
                    if(line_without_whitespace.size == 0)
                    {
                        *cursor = line_range.min + (u64)(line_without_whitespace.str - line.str);
                        done = 1;
                    }
                    else if(scan_line_num == 1)
                    {
                        *cursor = 0;
                    }
                    scratch_end(scratch);
                }
            }
            
            if(need_nav && evt->delta_unit == UI_EventDeltaUnit_Page && delta.y > 0)
            {
                u64 advance = line_count_per_page;
                u64 next_line = line_num + advance;
                u64 next_line_clamped = Clamp(1, next_line, text_patched.line_map.total_line_count);
                Rng1u64 next_line_range = txt_range_from_line_num(&text_patched.line_map, next_line_clamped);
                *cursor = next_line_range.min;
            }
            
            if(need_nav && evt->delta_unit == UI_EventDeltaUnit_Page && delta.y < 0)
            {
                s64 advance = -line_count_per_page;
                if(line_num < line_count_per_page)
                {
                    advance = -(line_num - 1);
                }
                u64 next_line = (u64)((s64)line_num + advance);
                u64 next_line_clamped = Clamp(1, next_line, text_patched.line_map.total_line_count);
                Rng1u64 next_line_range = txt_range_from_line_num(&text_patched.line_map, next_line_clamped);
                *cursor = next_line_range.min;
            }
            
            if(need_nav && evt->delta_unit == UI_EventDeltaUnit_Whole && (delta.y > 0 || delta.x > 0))
            {
                *cursor = text_patched.size;
            }
            
            if(need_nav && evt->delta_unit == UI_EventDeltaUnit_Whole && (delta.y < 0 || delta.x < 0))
            {
                *cursor = 0;
            }
            
            Rng1u64 replaced_range = single_line_op.range;
            if(*cursor != single_line_op.cursor && (evt->flags & UI_EventFlag_Delete || evt->string.size != 0))
            {
                replaced_range = r1u64(*mark, *cursor);
            }
            
            if(*cursor != *mark && evt->flags & UI_EventFlag_PickSelectSide)
            {
                if(delta.x < 0 || delta.y < 0)
                {
                    *cursor = *mark = Min(*cursor, *mark);
                }
                else if(delta.x > 0 || delta.y > 0)
                {
                    *cursor = *mark = Max(*cursor, *mark);
                }
            }
            
            if(evt->flags & UI_EventFlag_Copy)
            {
                String8 text = memory_map_data_from_range(scratch.arena, &text_patched.memory_map, r1u64(*cursor, *mark));
                wm_set_clipboard_text(text);
            }
            
            if(!(evt->flags & UI_EventFlag_KeepMark))
            {
                *mark = *cursor;
            }
            
            if(replaced_range.max != replaced_range.min || single_line_op.replace.size != 0)
            {
                txt_patch_list_push_new(cv->patch_arena, &cv->patches, replaced_range, single_line_op.replace);
                text_patched = txt_patched_from_info_data_patches(scratch.arena, text_info, text_data, &cv->patches);
                *cursor = *mark = replaced_range.min + single_line_op.replace.size;
                u64 line_num = txt_line_num_from_off(&text_patched.line_map, *cursor);
                Rng1u64 line_range = txt_range_from_line_num(&text_patched.line_map, line_num);
                *preferred_column = (*cursor - line_range.min);
            }
            
            ui_eat_event(evt);
            
            if(*cursor != start_cursor)
            {
                snap[Axis2_X] = 1;
            }
            
            if(*cursor < line_range.min || line_range.max < *cursor)
            {
                snap[Axis2_Y] = 1;
            }
        }
    }
    
    //////////////////////////////
    //- rjf: determine visible line range / count
    //
    Rng1s64 visible_line_num_range = r1s64(scroll_pos.y.idx + (s64)(scroll_pos.y.off) + 1 - !!(scroll_pos.y.off < 0),
                                           scroll_pos.y.idx + (s64)(scroll_pos.y.off) + 1 + num_possible_visible_lines);
    Rng1s64 target_visible_line_num_range = r1s64(scroll_pos.y.idx + 1,
                                                  scroll_pos.y.idx + 1 + num_possible_visible_lines);
    u64 visible_line_count = 0;
    {
        visible_line_num_range.min = Clamp(1, visible_line_num_range.min, (s64)text_patched.line_map.total_line_count);
        visible_line_num_range.max = Clamp(1, visible_line_num_range.max, (s64)text_patched.line_map.total_line_count);
        visible_line_num_range.min = Max(1, visible_line_num_range.min);
        visible_line_num_range.max = Max(1, visible_line_num_range.max);
        target_visible_line_num_range.min = Clamp(1, target_visible_line_num_range.min, (s64)text_patched.line_map.total_line_count);
        target_visible_line_num_range.max = Clamp(1, target_visible_line_num_range.max, (s64)text_patched.line_map.total_line_count);
        target_visible_line_num_range.min = Max(1, target_visible_line_num_range.min);
        target_visible_line_num_range.max = Max(1, target_visible_line_num_range.max);
        visible_line_count = (u64)dim_1s64(visible_line_num_range)+1;
    }
    
    //////////////////////////////
    //- rjf: calculate scroll bounds
    //
    s64 line_size_x = 0;
    Rng1s64 scroll_idx_rng[Axis2_COUNT] = {0};
    {
        line_size_x = (text_info->lines_max_size*big_glyph_advance*3)/2;
        line_size_x = ClampBot(line_size_x, (s64)big_glyph_advance*120);
        line_size_x = ClampBot(line_size_x, (s64)code_area_dim.x);
        scroll_idx_rng[Axis2_X] = r1s64(0, line_size_x-(s64)code_area_dim.x);
        scroll_idx_rng[Axis2_Y] = r1s64(0, (s64)text_patched.line_map.total_line_count-1);
    }
  
    //////////////////////////////
    //- rjf: calculate line-range-dependent info
    //
    // NOTE: dropped priority_margin_width_px/catchall_margin_width_px -
    // both margin columns are gone, so there's nothing left to reserve
    // width for.
    //
    f32 line_num_width_px = 0;
    if(do_line_numbers)
    {
        line_num_width_px = floor_f32(big_glyph_advance * (log10(visible_line_num_range.max) + 3));
    }
    Rng1u64 visible_byte_range = r1u64(txt_range_from_line_num(&text_patched.line_map, visible_line_num_range.min).min,
                                       txt_range_from_line_num(&text_patched.line_map, visible_line_num_range.max).max);
    String8 visible_data = memory_map_data_from_range(scratch.arena, &text_patched.memory_map, visible_byte_range);
  
    //////////////////////////////
    //- rjf: find tokens for visible byte range
    //
    u64 ctx_token_pt_num = txt_token_pt_num_from_off(&text_patched.token_pt_map, visible_byte_range.min);
    TXT_Token_Pt ctx_token_pt = txt_token_pt_from_num(&text_patched.token_pt_map, ctx_token_pt_num);
    TXT_Token_Array tokens = txt_token_array_from_data(scratch.arena, bp_regs()->lang_kind, ctx_token_pt, visible_data, visible_byte_range.min, max_u64);
  
    //////////////////////////////
    //- rjf: selection on single line, no query? -> set search text
    //
    {
        u64 cursor = bp_regs()->cursor;
        u64 mark = bp_regs()->mark;
        u64 cursor_line_num = txt_line_num_from_off(&text_patched.line_map, cursor);
        Rng1u64 cursor_line_range = txt_range_from_line_num(&text_patched.line_map, cursor_line_num);
        if(cursor_line_range.min <= mark && mark <= cursor_line_range.max)
        {
            CFG_Node *view = cfg_node_from_id(bp_regs()->view);
            BP_View_State *vs = bp_view_state_from_cfg(view);
            if(!vs->query_is_open)
            {
                CFG_Node *query = cfg_node_child_from_string_or_alloc(bp_state->cfg, view, str8_lit("query"));
                CFG_Node *input = cfg_node_child_from_string_or_alloc(bp_state->cfg, query, str8_lit("input"));
                String8 text = memory_map_data_from_range(scratch.arena, &text_patched.memory_map, r1u64(cursor, mark));
                if(text.size < 256)
                {
                    cfg_node_new_replace(bp_state->cfg, input, text);
                }
                else
                {
                    cfg_node_new_replace(bp_state->cfg, input, str8_zero());
                }
            }
        }
    }
  
    //////////////////////////////
    //- rjf: get active search query
    //
    String8 search_query = bp_view_query_input();
    bool32 search_query_is_active = 0;
    
    //////////////////////////////
    //- rjf: prepare code slice info bundle, for the viewable region of text
    BP_Code_Slice_Params code_slice_params = {0};
    {
        // rjf: fill basics
        code_slice_params.flags = BP_CodeSliceFlag_Clickable;
        if(do_line_numbers)
        {
            code_slice_params.flags |= BP_CodeSliceFlag_LineNums;
        }
        code_slice_params.line_num_range            = visible_line_num_range;
        code_slice_params.line_text                 = push_array(scratch.arena, String8, visible_line_count);
        code_slice_params.line_ranges               = push_array(scratch.arena, Rng1u64, visible_line_count);
        code_slice_params.line_tokens               = push_array(scratch.arena, TXT_Token_Array, visible_line_count);
        code_slice_params.text_info                 = text_info;
        code_slice_params.text_data                 = text_data;
        code_slice_params.font                      = code_font;
        code_slice_params.font_size                 = code_font_size;
        code_slice_params.tab_size                  = code_tab_size;
        code_slice_params.line_height_px            = code_line_height;
        code_slice_params.search_query              = search_query;
        code_slice_params.line_num_width_px         = line_num_width_px;
        code_slice_params.line_text_max_width_px    = (f32)line_size_x;
        code_slice_params.margin_float_off_px       = scroll_pos.x.idx + floor_f32(scroll_pos.x.off);
    
        // rjf: fill line text / ranges
        {
            s64 line_num = visible_line_num_range.min;
            u64 line_idx = visible_line_num_range.min-1;
            for(u64 visible_line_idx = 0;
                visible_line_idx < visible_line_count && line_idx < text_patched.line_map.total_line_count;
                visible_line_idx += 1, line_idx += 1, line_num += 1)
            {
                Rng1u64 line_range = txt_range_from_line_num(&text_patched.line_map, line_num);
                String8 line_text = {0};
                line_text.size = dim_1u64(line_range);
                line_text.str = push_array(scratch.arena, u8, line_text.size);
                memory_map_read(&text_patched.memory_map, line_range, line_text.str);
                code_slice_params.line_text[visible_line_idx]   = line_text;
                code_slice_params.line_ranges[visible_line_idx] = line_range;
            }
        }
    
        // rjf: bucket tokens by line
        {
            u64 token_idx = 0;
            for EachIndex(line_idx, visible_line_count)
            {
                Temp scratch2 = scratch_begin(&scratch.arena, 1);
                Rng1u64 line_range = code_slice_params.line_ranges[line_idx];
                TXT_Token_List line_tokens = {0};
                for(;token_idx < tokens.count;)
                {
                    Rng1u64 token_range = tokens.v[token_idx].range;
                    if(dim_1u64(intersect_1u64(token_range, line_range)) != 0)
                    {
                        txt_token_list_push(scratch2.arena, &line_tokens, &tokens.v[token_idx]);
                    }
                    if(token_range.max <= line_range.max || token_range.max <= line_range.min)
                    {
                        token_idx += 1;
                    }
                    else if(line_range.max <= token_range.max)
                    {
                        break;
                    }
                }
                code_slice_params.line_tokens[line_idx] = txt_token_array_from_list(scratch.arena, &line_tokens);
                scratch_end(scratch2);
            }
        }
    }
    
    //////////////////////////////
    //- rjf: build container
    //
    UI_Box *container_box = &ui_nil_box;
    {
        ui_set_next_pref_width(ui_px(code_area_dim.x, 1));
        ui_set_next_pref_height(ui_px(code_area_dim.y, 1));
        ui_set_next_child_layout_axis(Axis2_Y);
        container_box = ui_build_box_from_stringf(UI_BoxFlag_Clip|
                                                  UI_BoxFlag_Scroll|
                                                  UI_BoxFlag_AllowOverflowX|
                                                  UI_BoxFlag_AllowOverflowY,
                                                  "###code_area");
    }
    
    //////////////////////////////
    //- rjf: cancelled search query -> center cursor
    //
    if(!search_query_is_active && cv->drifted_for_search)
    {
        cv->drifted_for_search = 0;
        cv->center_cursor = 1;
    }
    
    //////////////////////////////
    //- rjf: do searching operations
    //
    {
        u64 search_chunk_size = KB(4);
    
        if(cv->find_text_fwd.size != 0)
        {
            String8 needle = cv->find_text_fwd;
            Rng1u64 ranges[] =
                {
                    r1u64(bp_regs()->cursor+1, text_patched.size),
                    r1u64(0, bp_regs()->cursor+1),
                };
            bool32 found = 0;
            for EachElement(range_idx, ranges)
            {
                for(u64 off = ranges[range_idx].min; off < ranges[range_idx].max; off += search_chunk_size)
                {
                    Temp scratch = scratch_begin(&arena, 1);
                    String8 data = memory_map_data_from_range(scratch.arena, &text_patched.memory_map, r1u64(off, off+search_chunk_size));
                    u64 hint_needle_pos = str8_find_needle(data, 0, needle, StringMatchFlag_CaseInsensitive|StringMatchFlag_RightSideSloppy);
                    if(hint_needle_pos < data.size)
                    {
                        String8 candidate = memory_map_data_from_range(scratch.arena, &text_patched.memory_map, r1u64(off+hint_needle_pos, off+hint_needle_pos+needle.size));
                        if(str8_match(candidate, needle, StringMatchFlag_CaseInsensitive))
                        {
                            found = 1;
                            bp_regs()->mark = off+hint_needle_pos;
                            bp_regs()->cursor = bp_regs()->mark + needle.size;
                        }
                    }
                    scratch_end(scratch);
                    if(found) { goto done_fwd; }
                }
            }
        done_fwd:;
            if(!found)
            {
                log_user_errorf("Could not find `%S`", needle);
            }
            cv->center_cursor = found;
        }
        
        if(cv->find_text_bwd.size != 0 && bp_regs()->cursor > 0)
        {
            String8 needle = cv->find_text_bwd;
            Rng1u64 ranges[] =
                {
                    r1u64(0, Min(bp_regs()->cursor, bp_regs()->mark)),
                    r1u64(bp_regs()->cursor+1, text_patched.size),
                };
            bool32 found = 0;
            for EachElement(range_idx, ranges)
            {
                u64 start_off = ranges[range_idx].max + (search_chunk_size-1);
                start_off -= start_off%search_chunk_size;
                start_off -= search_chunk_size;
                for(u64 off = start_off;; off -= search_chunk_size)
                {
                    Temp scratch = scratch_begin(&arena, 1);
                    String8 data = memory_map_data_from_range(scratch.arena, &text_patched.memory_map, r1u64(off, Min(off+search_chunk_size, ranges[range_idx].max)));
                    u64 hint_needle_pos = str8_find_needle(data, 0, needle, StringMatchFlag_CaseInsensitive|StringMatchFlag_RightSideSloppy);
                    for(;hint_needle_pos < data.size;)
                    {
                        u64 next_hint_needle_pos = str8_find_needle(data, hint_needle_pos+1, needle, StringMatchFlag_CaseInsensitive|StringMatchFlag_RightSideSloppy);
                        if(next_hint_needle_pos < data.size)
                        {
                            hint_needle_pos = next_hint_needle_pos;
                        }
                        else
                        {
                            break;
                        }
                    }
                    if(hint_needle_pos < data.size)
                    {
                        String8 candidate = memory_map_data_from_range(scratch.arena, &text_patched.memory_map, r1u64(off+hint_needle_pos, off+hint_needle_pos+needle.size));
                        if(str8_match(candidate, needle, StringMatchFlag_CaseInsensitive))
                        {
                            found = 1;
                            bp_regs()->mark = off+hint_needle_pos;
                            bp_regs()->cursor = bp_regs()->mark + needle.size;
                        }
                    }
                    scratch_end(scratch);
                    if(found) { goto done_bwd; }
                    if(off == 0) { break; }
                }
            }
        done_bwd:;
            if(!found)
            {
                log_user_errorf("Could not find `%S`", needle);
            }
            cv->center_cursor = found;
        }
    
        MemoryZeroStruct(&cv->find_text_fwd);
        MemoryZeroStruct(&cv->find_text_bwd);
        arena_clear(cv->find_text_arena);
    }
  
    //////////////////////////////
    //- rjf: do goto line
    //
    if(cv->goto_line_num != 0 && text_is_ready)
    {
        s64 line_num = cv->goto_line_num;
        cv->goto_line_num = 0;
        line_num = Clamp(1, line_num, text_patched.line_map.total_line_count);
        Rng1u64 range = txt_range_from_line_num(&text_patched.line_map, line_num);
        bp_regs()->cursor = bp_regs()->mark = range.min;
        cv->center_cursor = !cv->force_contain_only && (!cv->contain_cursor || (line_num < target_visible_line_num_range.min+4 || target_visible_line_num_range.max-4 < line_num));
    }
  
    //////////////////////////////
    //- rjf: build container contents
    //
    // NOTE: dropped the "watch expr at mouse" branch (command's gone).
    // Kept ctrl+click -> GoToName - that's the generic "jump to whatever
    // token is under the cursor" affordance, and GoToName now just
    // searches the filesystem for it (see the earlier bp_frame.c
    // conversion), so it's still meaningful here.
    //
    UI_Parent(container_box)
    {
        container_box->view_off.x = container_box->view_off_target.x = scroll_pos.x.idx + scroll_pos.x.off;
        container_box->view_off.y = container_box->view_off_target.y = code_line_height*mod_f32(scroll_pos.y.off, 1.f) + code_line_height*(scroll_pos.y.off < 0) - code_line_height*(scroll_pos.y.off == -1.f && scroll_pos.y.idx == 1);
    
        BP_Code_Slice_Signal sig = {0};
        UI_Focus(UI_FocusKind_On)
        {
            sig = bp_code_slicef(&code_slice_params, &bp_regs()->cursor, &bp_regs()->mark, &cv->preferred_column, "code_slice");
        }
    
        if(ui_pressed(sig.base))
        {
            bp_cmd(BP_CmdKind_FocusPanel);
        }
    
        if(ui_dragging(sig.base) && sig.base.event_flags == 0)
        {
            if(!contains_2f32(sig.base.box->rect, ui_mouse()))
            {
                cv->contain_cursor = 1;
            }
            else
            {
                snap[Axis2_X] = 1;
            }
        }
    
        if(ui_pressed(sig.base) && sig.base.event_flags & WM_Modifier_Ctrl)
        {
            ui_kill_action();
            bp_cmd(BP_CmdKind_GoToName, .string = memory_map_data_from_range(scratch.arena, &text_patched.memory_map, sig.mouse_expr_rng));
        }
    }
  
    //////////////////////////////
    //- rjf: apply post-build view snapping rules
    //
    if(text_is_ready)
    {
        u64 cursor = bp_regs()->cursor;
        u64 cursor_line_num = txt_line_num_from_off(&text_patched.line_map, cursor);
        bool32 cursor_in_range = (1 <= cursor_line_num && cursor_line_num <= text_patched.line_map.total_line_count);
    
        if(cv->contain_cursor)
        {
            cv->contain_cursor = 0;
            snap[Axis2_X] = 1;
            snap[Axis2_Y] = 1;
        }
    
        if(cv->center_cursor)
        {
            cv->center_cursor = 0;
            if(cursor_in_range)
            {
                Rng1u64 cursor_line_range = txt_range_from_line_num(&text_patched.line_map, cursor_line_num);
                String8 cursor_line = memory_map_data_from_range(scratch.arena, &text_patched.memory_map, cursor_line_range);
                f32 cursor_advance = fnt_dim_from_tag_size_string(code_font, code_font_size, 0, code_tab_size, str8_prefix(cursor_line, cursor-cursor_line_range.min)).x;
        
                {
                    s64 new_idx = (s64)(cursor_advance - code_area_dim.x/2);
                    new_idx = Clamp(scroll_idx_rng[Axis2_X].min, new_idx, scroll_idx_rng[Axis2_X].max);
                    ui_scroll_pt_target_idx(&scroll_pos.x, new_idx);
                    snap[Axis2_X] = 0;
                }
        
                {
                    s64 new_idx = ((s64)cursor_line_num-1) - num_possible_visible_lines/2 + 2;
                    new_idx = Clamp(scroll_idx_rng[Axis2_Y].min, new_idx, scroll_idx_rng[Axis2_Y].max);
                    ui_scroll_pt_target_idx(&scroll_pos.y, new_idx);
                    snap[Axis2_Y] = 0;
                }
            }
        }
    
        if(snap[Axis2_X] && cursor_in_range)
        {
            Rng1u64 cursor_line_range = txt_range_from_line_num(&text_patched.line_map, cursor_line_num);
            String8 cursor_line = memory_map_data_from_range(scratch.arena, &text_patched.memory_map, cursor_line_range);
            s64 cursor_off = (s64)(fnt_dim_from_tag_size_string(code_font, code_font_size, 0, code_tab_size, str8_prefix(cursor_line, cursor-cursor_line_range.min)).x + line_num_width_px);
            Rng1s64 visible_pixel_range =
                {
                    scroll_pos.x.idx,
                    scroll_pos.x.idx + (s64)code_area_dim.x,
                };
            Rng1s64 cursor_pixel_range =
                {
                    cursor_off - (s64)(big_glyph_advance*4) - (s64)line_num_width_px,
                    cursor_off + (s64)(big_glyph_advance*4),
                };
            s64 min_delta = Min(0, cursor_pixel_range.min - visible_pixel_range.min);
            s64 max_delta = Max(0, cursor_pixel_range.max - visible_pixel_range.max);
            s64 new_idx = scroll_pos.x.idx+min_delta+max_delta;
            new_idx = Clamp(scroll_idx_rng[Axis2_X].min, new_idx, scroll_idx_rng[Axis2_X].max);
            ui_scroll_pt_target_idx(&scroll_pos.x, new_idx);
        }
    
        if(snap[Axis2_Y])
        {
            Rng1s64 cursor_visibility_range = r1s64((s64)cursor_line_num-4, (s64)cursor_line_num+4);
            cursor_visibility_range.min = ClampBot(0, cursor_visibility_range.min);
            cursor_visibility_range.max = ClampBot(0, cursor_visibility_range.max);
            s64 min_delta = Min(0, cursor_visibility_range.min-(target_visible_line_num_range.min));
            s64 max_delta = Max(0, cursor_visibility_range.max-(target_visible_line_num_range.min+num_possible_visible_lines));
            s64 new_idx = scroll_pos.y.idx+min_delta+max_delta;
            new_idx = Clamp(0, new_idx, (s64)text_patched.line_map.total_line_count-1);
            ui_scroll_pt_target_idx(&scroll_pos.y, new_idx);
        }
    }
  
    //////////////////////////////
    //- rjf: build horizontal scroll bar
    //
    {
        ui_set_next_fixed_x(0);
        ui_set_next_fixed_y(code_area_dim.y);
        ui_set_next_fixed_width(panel_box_dim.x - scroll_bar_dim);
        ui_set_next_fixed_height(scroll_bar_dim);
        {
            scroll_pos.x = ui_scroll_bar(Axis2_X,
                                         ui_px(scroll_bar_dim, 1.f),
                                         scroll_pos.x,
                                         scroll_idx_rng[Axis2_X],
                                         (s64)code_area_dim.x);
        }
    }
  
    //////////////////////////////
    //- rjf: build vertical scroll bar
    //
    {
        ui_set_next_fixed_x(code_area_dim.x);
        ui_set_next_fixed_y(0);
        ui_set_next_fixed_width(scroll_bar_dim);
        ui_set_next_fixed_height(panel_box_dim.y - scroll_bar_dim);
        {
            scroll_pos.y = ui_scroll_bar(Axis2_Y,
                                         ui_px(scroll_bar_dim, 1.f),
                                         scroll_pos.y,
                                         scroll_idx_rng[Axis2_Y],
                                         num_possible_visible_lines);
        }
    }
  
    //////////////////////////////
    //- rjf: top-level container interaction (scrolling)
    //
    if(text_is_ready)
    {
        UI_Signal sig = ui_signal_from_box(container_box);
        if(sig.scroll.x != 0)
        {
            s64 new_idx = scroll_pos.x.idx+sig.scroll.x*big_glyph_advance;
            new_idx = clamp_1s64(scroll_idx_rng[Axis2_X], new_idx);
            ui_scroll_pt_target_idx(&scroll_pos.x, new_idx);
        }
        if(sig.scroll.y != 0)
        {
            s64 new_idx = scroll_pos.y.idx + sig.scroll.y;
            new_idx = clamp_1s64(scroll_idx_rng[Axis2_Y], new_idx);
            ui_scroll_pt_target_idx(&scroll_pos.y, new_idx);
        }
        ui_scroll_pt_clamp_idx(&scroll_pos.x, scroll_idx_rng[Axis2_X]);
        ui_scroll_pt_clamp_idx(&scroll_pos.y, scroll_idx_rng[Axis2_Y]);
        if(ui_mouse_over(sig))
        {
            for(UI_Event *evt = 0; ui_next_event(&evt);)
            {
                if(evt->kind == UI_EventKind_Scroll && evt->modifiers & WM_Modifier_Ctrl && evt->modifiers & WM_Modifier_Shift)
                {
                    ui_eat_event(evt);
                    if(evt->delta_2f32.y < 0)
                    {
                        bp_cmd(BP_CmdKind_IncViewFontSize);
                    }
                    else if(evt->delta_2f32.y > 0)
                    {
                        bp_cmd(BP_CmdKind_DecViewFontSize);
                    }
                }
            }
        }
    }
  
    //////////////////////////////
    //- rjf: store state
    //
    bp_store_view_scroll_pos(scroll_pos);
    cv->force_contain_only = 0;
  
    scratch_end(scratch);
    ProfEnd();
}

/////////////////////////
// null @view_hook_impl

BP_VIEW_UI_FUNCTION_DEF(null) {}

////////////////////////////
// text @view_hook_impl

BP_VIEW_UI_FUNCTION_DEF(text)
{
    BP_Code_View_State *cv = bp_view_state(BP_Code_View_State);
    bp_code_view_init(cv);
    Temp scratch = scratch_begin(0, 0);
    Access *access = access_open();
  
    //////////////////////////////
    //- rjf: set up invariants
    //
    f32 main_font_size = ui_bottom_font_size();
    f32 bottom_bar_height = main_font_size*2.f;
    Rng2f32 code_area_rect = r2f32p(rect.x0, rect.y0, rect.x1, rect.y1 - bottom_bar_height);
    Rng2f32 bottom_bar_rect = r2f32p(rect.x0, rect.y1 - bottom_bar_height, rect.x1, rect.y1);
  
    //////////////////////////////
    //- rjf: process code-file commands
    //
    ProfScope("process code-file commands") for(BP_Cmd *cmd = 0; bp_next_view_cmd(&cmd);)
    {
        BP_CmdKind kind = bp_cmd_kind_from_string(cmd->name);
        switch(kind)
        {
            default:{}break;
                
                // rjf: override file picking
            case BP_CmdKind_PickFile: {
                {
                    String8 src = bp_regs()->file_path;
                    String8 dst = cmd->regs->file_path;
                    if(src.size != 0 && dst.size != 0)
                    {
                        // rjf: record src -> dst mapping
                        bp_cmd(BP_CmdKind_SetFileReplacementPath, .string = src, .file_path = dst);
                        
                        // rjf: switch this view to viewing replacement file
                        bp_store_view_expr_string(bp_eval_string_from_file_path(scratch.arena, dst));
                    }
                }
            }break;
        }
    }
    
    //////////////////////////////
    //- rjf: unpack parameterization info
    //
    ProfBegin("unpack parameterization info");
    bp_regs()->file_path     = bp_file_path_from_eval(bp_frame_arena(), eval);
    bp_regs()->vaddr         = 0;
    bp_regs()->prefer_disasm = 0;
    bp_regs()->cursor        = bp_view_setting_value_from_name(s("cursor")).u64;
    bp_regs()->mark          = bp_view_setting_value_from_name(s("mark")).u64;
    String8List overrides = bp_possible_overrides_from_file_path(scratch.arena, bp_regs()->file_path);
    Rng1u64 range = bp_space_range_from_eval(eval);
    bp_regs()->text_key = bp_key_from_eval_space_range(eval.space, range, 1);
    String8 lang = bp_view_setting_from_name(str8_lit("lang"));
    if(lang.size == 0)
    {
        bp_regs()->lang_kind = bp_lang_kind_from_eval(eval);
    }
    else
    {
        bp_regs()->lang_kind = txt_lang_kind_from_extension(lang);
    }
    u128 hash = {0};
    TXT_Text_Info info = txt_text_info_from_key_lang(access, bp_regs()->text_key, bp_regs()->lang_kind, &hash);
    String8 data = c_data_from_hash(access, hash);
    bool32 file_is_missing = (bp_regs()->file_path.size != 0 && properties_from_file_path(bp_regs()->file_path).modified == 0);
    bool32 key_has_data = !u128_match(hash, u128_zero()) && info.lines_count;
    ProfEnd();
    
    //////////////////////////////
    //- rjf: update last hash - scroll-to-bottom if needed
    //
    if(bp_setting_b32_from_name(str8_lit("scroll_to_bottom_on_change")) && !u128_match(hash, cv->last_hash) && !u128_match(cv->last_hash, u128_zero()))
    {
        cv->goto_line_num = info.lines_count;
        cv->contain_cursor = 1;
        cv->force_contain_only = 1;
    }
    cv->last_hash = hash;
    
    //////////////////////////////
    //- rjf: build missing file interface
    //
    if(file_is_missing)
    {
        UI_WidthFill UI_HeightFill UI_Column UI_Padding(ui_pct(1, 0))
        {
            Temp scratch = scratch_begin(0, 0);
            UI_PrefWidth(ui_children_sum(1)) UI_PrefHeight(ui_em(3, 1))
                UI_Row UI_Padding(ui_pct(1, 0))
                UI_PrefWidth(ui_text_dim(1, 1))
                UI_TagF("weak")
            {
                BP_Font(BP_FontSlot_Icons) ui_label(bp_icon_kind_text_table[BP_IconKind_WarningBig]);
                ui_labelf("Could not find \"%S\".", bp_regs()->file_path);
            }
            UI_PrefHeight(ui_em(3, 1))
                UI_Row UI_Padding(ui_pct(1, 0))
                UI_PrefWidth(ui_text_dim(10, 1))
                UI_CornerRadius(ui_top_font_size()/3)
                UI_PrefWidth(ui_text_dim(10, 1))
                UI_Focus(UI_FocusKind_On)
                UI_TextAlignment(UI_TextAlign_Center)
                UI_TagF("pop")
                if(ui_clicked(ui_buttonf("Find alternative...")))
                {
                    bp_cmd(BP_CmdKind_RunCommand, .cmd_name = bp_cmd_kind_info_table[BP_CmdKind_PickFile].string);
                }
            scratch_end(scratch);
        }
    }
    
    //////////////////////////////
    //- rjf: code is not missing, but not ready -> equip loading info to this view
    //
    bool32 is_loading = (!file_is_missing && info.lines_count == 0);
    if(is_loading)
    {
        bp_store_view_loading_info(1, info.bytes_processed, info.bytes_to_process);
    }
    
    //////////////////////////////
    //- rjf: build code contents
    //
    if(!file_is_missing)
    {
        bp_code_view_build(scratch.arena, cv, code_area_rect, data, &info);
    }
    // @HERE
    //////////////////////////////
    //- rjf: produced patched text info, unpack cursor info in patched text
    //
    TXT_Patched patched = txt_patched_from_info_data_patches(scratch.arena, &info, data, &cv->patches);
    u64 cursor_line_num = txt_line_num_from_off(&patched.line_map, bp_regs()->cursor);
    Rng1u64 cursor_line_range = txt_range_from_line_num(&patched.line_map, cursor_line_num);
    
    //////////////////////////////
    //- rjf: unpack cursor info
    //
    if(bp_regs()->file_path.size != 0)
    {
        D_Entity *module = d_entity_from_handle(bp_regs()->module);
        DI_Key dbgi_key = d_dbgi_key_from_module(module);
        bp_regs()->lines = d_lines_from_dbgi_key_file_path_line_num(bp_frame_arena(), dbgi_key, bp_regs()->file_path, (S64)cursor_line_num, 8);
    }
    bp_regs()->line_num = cursor_line_num;
    bp_regs()->column_num = (bp_regs()->cursor - cursor_line_range.min);
    
    //////////////////////////////
    //- rjf: determine if file is out-of-date
    //
    bool32 file_is_out_of_date = 0;
    {
        Temp scratch = scratch_begin(0, 0);
        
        // rjf: determine checksum in selected debug info
        E_DbgInfo *dbg_info = e_base_ctx->primary_dbg_info;
        DI_Key dbgi_key = dbg_info->dbgi_key;
        RDI_ChecksumKind checksum_kind = RDI_ChecksumKind_NULL;
        String8 checksum_expected = {0};
        {
            Access *access = access_open();
            
            // rjf: unpack RDI
            RDI_Parsed *rdi = di_rdi_from_key(access, dbgi_key, 0, 0);
            
            // rjf: file_path_normalized * rdi -> src_id
            for EachNode(override_n, String8Node, overrides.first)
            {
                String8 file_path = override_n->string;
                String8 file_path_normalized = rdim_normalized_from_path(scratch.arena, file_path);
                bool32 good_src_id = 0;
                u32 src_id = 0;
                if(rdi != &rdi_parsed_nil)
                {
                    RDI_NameMap *mapptr = rdi_element_from_name_idx(rdi, NameMaps, RDI_NameMapKind_NormalSourcePaths);
                    RDI_ParsedNameMap map = {0};
                    rdi_parsed_from_name_map(rdi, mapptr, &map);
                    RDI_NameMapNode *node = rdi_name_map_lookup(rdi, &map, file_path_normalized.str, file_path_normalized.size);
                    if(node != 0)
                    {
                        u32 id_count = 0;
                        u32 *ids = rdi_matches_from_map_node(rdi, node, &id_count);
                        if(id_count > 0)
                        {
                            u32 src_id = ids[0];
                            RDI_SourceFile *src = rdi_element_from_name_idx(rdi, SourceFiles, src_id);
                            checksum_kind = src->checksum_kind;
                            RDI_SectionKind checksum_section_kind = rdi_section_kind_from_checksum_kind(checksum_kind);
                            u64 checksum_size = rdi_section_element_size_table[checksum_section_kind];
                            u8 *checksum_data = (u8 *)rdi_section_raw_element_from_kind_idx(rdi, checksum_section_kind, src->checksum_idx);
                            checksum_expected = str8_copy(scratch.arena, str8(checksum_data, checksum_size));
                            break;
                        }
                    }
                }
            }
            
            access_close(access);
        }
        
        // rjf: if we got a checksum, compute it locally - check if they match.
        switch(checksum_kind)
        {
            default:
                {
                    String8 checksum_value = bp_checksum_value_from_hash_kind(scratch.arena, hash, checksum_kind);
                    file_is_out_of_date = checksum_expected.size != 0 && !memory_is_zero(checksum_value.str, checksum_value.size) && !str8_match(checksum_value, checksum_expected, 0);
                }break;
            case RDI_ChecksumKind_Timestamp:
                {
                    File_Properties props = properties_from_file_path(bp_regs()->file_path);
                    String8 timestamp_string = str8_struct(&props.modified);
                    file_is_out_of_date = !MemoryIsZeroStruct(&props.modified) && !str8_match(timestamp_string, checksum_expected, 0);
                }break;
        }
        
        scratch_end(scratch);
    }
    
    //////////////////////////////
    //- rjf: build bottom bar
    //
    if(!file_is_missing && key_has_data) UI_FontSize(main_font_size) UI_TagF(file_is_out_of_date ? "bad_pop" : ".")
    {
        ui_set_next_rect(shift_2f32(bottom_bar_rect, scale_2f32(rect.p0, -1.f)));
        ui_set_next_flags(UI_BoxFlag_DrawBackground);
        UI_Row
            UI_TextAlignment(UI_TextAlign_Center)
            UI_PrefWidth(ui_text_dim(10, 1))
            UI_TagF("weak")
        {
            if(file_is_out_of_date) 
            {
                UI_Box *box = &ui_nil_box;
                BP_Font(BP_FontSlot_Icons)
                {
                    box = ui_build_box_from_stringf(UI_BoxFlag_DrawText|UI_BoxFlag_Clickable, "%S###file_ood_warning", bp_icon_kind_text_table[BP_IconKind_WarningBig]);
                }
                UI_Signal sig = ui_signal_from_box(box);
                if(ui_hovering(sig)) UI_Tooltip
                {
                    UI_PrefWidth(ui_children_sum(1)) UI_Row UI_PrefWidth(ui_text_dim(1, 1)) UI_TextPadding(0)
                    {
                        UI_TagF("weak") ui_labelf("This file has changed since it was compiled.");
                    }
                }
            }
            BP_Font(BP_FontSlot_Code)
            {
                if(bp_regs()->file_path.size != 0)
                {
                    ui_label(bp_regs()->file_path);
                    ui_spacer(ui_em(1.5f, 1));
                }
                ui_labelf("Line: %I64d, Column: %I64d, Offset: 0x%I64x", cursor_line_num, 1 + bp_regs()->cursor - cursor_line_range.min, bp_regs()->cursor);
                ui_spacer(ui_pct(1, 0));
                ui_labelf("(read only)");
                ui_labelf("%s",
                          info.line_end_kind == TXT_LineEndKind_LF   ? "lf" :
                          info.line_end_kind == TXT_LineEndKind_CRLF ? "crlf" :
                          "bin");
            }
        }
    }
    
    //////////////////////////////
    //- rjf: store params
    //
    bp_store_view_param_u64(s("cursor"), bp_regs()->cursor);
    bp_store_view_param_u64(s("mark"), bp_regs()->mark);
  
    access_close(access);
    scratch_end(scratch);
}
//BP_VIEW_UI_FUNCTION_DEF(geo3d);
BP_VIEW_UI_FUNCTION_DEF(getting_started);
