
internal TI_Query_Item_List ti_query_items_from_string(Arena *arena, String8 string, String8 filter)
{
    TI_Query_Item_List out = {0};
    Temp scratch = scratch_begin(&arena, 1);
    String8_List parts = str8_split(scratch.arena, string, (u8 *)",", 1, 0);
    for (String8_Node *n = parts.first; n != 0; n = n->next)
    {
        String8 p = str8_skip_chop_whitespace(n->string);
        u64 colon = str8_find_needle(p, 0, s(":"), 0);
        if (colon >= p.size) { continue; }
        String8 qual = str8_prefix(p, colon);
        String8 arg = str8_skip(p, colon+1);
        if (arg.size >= 2 && arg.str[0] == '"')
        {
            arg = raw_from_escaped_str8(scratch.arena, str8_substr(arg, r1u64(1, arg.size-1)));
        }
        // folder listing
        if (str8_match(qual, str8_lit("folder"), 0))
        {
            String8 folder_path = arg;
            String8 local_filter = filter;
            u64 folder_pos = str8_find_needle(filter, 0, folder_path, StringMatchFlag_CaseInsensitive|StringMatchFlag_SlashInsensitive);

            if (folder_pos < filter.size)
            {
                local_filter = str8_skip_chop_slashes(str8_skip(filter, folder_pos + folder_path.size));
            }
            else
            {
                MemoryZeroStruct(&local_filter);
            }

            // folders, then files
            for EachIndex(pass, 2)
            {
                File_Iter *iter = file_iter_begin(scratch.arena, folder_path, 0);
                for (File_Info info = {0}; file_iter_next(scratch.arena, iter, &info);)
                {
                    bool32 is_folder = !!(info.props.flags & FilePropertyFlag_IsFolder);
                    if (is_folder != (pass == 0))
                    {
                        continue;
                    }
                    Fuzzy_Match_Range_List m = fuzzy_match_find(scratch.arena, local_filter, info.name);
                    if (m.count != m.needle_part_count)
                    {
                        continue;
                    }
                    TI_Query_Item *item = push_array(arena, TI_Query_Item, 1);
                    item->display = str8_copy(arena, info.name);
                    item->value = push_str8f(arena, "%S%s%S", folder_path,
                                             folder_path.size != 0 ? "/" : "", info.name);
                    item->icon = is_folder ? TI_IconKind_FolderClosedFilled : TI_IconKind_FileOutline;
                    SLLQueuePush(out.first, out.last, item);
                    out.count += 1;
                }
                file_iter_end(iter);
            }
        }
        else if (str8_match(qual, str8_lit("query"), 0))
        {
            // command palette
            if (str8_match(arg, str8_lit("commands"), 0) ||
                str8_match(str8_postfix(arg, 9), str8_lit("_commands"), 0))
            {
                String8 collection_filter_tag = push_str8f(scratch.arena, "{%S}", arg);
                for EachNonZeroEnumVal(TI_CmdKind, k)
                {
                    TI_Cmd_Kind_Info *info = &ti_cmd_kind_info_table[k];
                    bool32 show = ((str8_match(arg, str8_lit("commands"), 0) && info->flags & TI_CmdKindFlag_ListInUI) ||
                                   (str8_find_needle(info->filter_tags, 0, collection_filter_tag, 0) < info->filter_tags.size));
                    if (!show)
                    {
                        continue;
                    }
                    String8 display_name = ti_display_from_code_name(info->string);
                    Fuzzy_Match_Range_List nm = fuzzy_match_find(scratch.arena, filter, display_name);
                    Fuzzy_Match_Range_List dm = fuzzy_match_find(scratch.arena, filter, info->description);
                    Fuzzy_Match_Range_List tm = fuzzy_match_find(scratch.arena, filter, info->search_tags);
                    bool32 binding_ok = false;
                    CFG_Key_Map_Node_Ptr_List bindings = cfg_key_map_node_ptr_list_from_name(scratch.arena, ti_state->key_map, info->string);
                    for (CFG_Key_Map_Node_Ptr *n = bindings.first; n != 0; n = n->next)
                    {
                        String8 text = wm_string_from_modifiers_key(scratch.arena, n->v->binding.modifiers, n->v->binding.key);
                        Fuzzy_Match_Range_List bm = fuzzy_match_find(scratch.arena, filter, text);
                        if (bm.count == bm.needle_part_count)
                        {
                            binding_ok = 1;
                            break;
                        }
                    }
                    if (nm.count == nm.needle_part_count ||
                        dm.count == dm.needle_part_count ||
                        tm.count == tm.needle_part_count ||
                        binding_ok)
                    {
                        TI_Query_Item *item = push_array(arena, TI_Query_Item, 1);
                        item->display = display_name;
                        item->value = info->string;
                        item->icon = ti_icon_kind_from_code_name(info->string);
                        SLLQueuePush(out.first, out.last, item);
                        out.count += 1;
                    }
                }
            }
            // cfg collections
            else
            {
                String8 cfg_name = ti_singular_from_code_name_plural(arg);
                if (cfg_name.size == 0)
                {
                    cfg_name = arg;
                }
                // collection commands, from cfg schema
                MD_Node_Ptr_List schemas = cfg_schemas_from_name(scratch.arena, ti_state->cfg_schema_table, cfg_name);
                for (MD_Node_Ptr_Node *n = schemas.first; n != 0; n = n->next)
                {
                    MD_Node *root = md_tag_from_string(n->v, str8_lit("collection_commands"), 0);
                    for MD_EachNode(cmd, root->first)
                    {
                        TI_Query_Item *item = push_array(arena, TI_Query_Item, 1);
                        item->display = ti_display_from_code_name(cmd->string);
                        item->value = cmd->string;
                        item->icon = ti_icon_kind_from_code_name(cmd->string);
                        SLLQueuePush(out.first, out.last, item);
                        out.count += 1;
                    }
                }

                CFG_Node_Ptr_List cfgs = cfg_node_top_level_list_from_string(scratch.arena, cfg_name);
                for EachNode(n, CFG_Node_Ptr_Node, cfgs.first)
                {
                    if (ti_cfg_is_project_filtered(n->v))
                    {
                        continue;
                    }
                    DR_FStr_List fstrs = ti_title_fstrs_from_cfg(scratch.arena, n->v, 1);
                    String8 title = dr_string_from_fstrs(scratch.arena, &fstrs);
                    if (filter.size != 0)
                    {
                        Fuzzy_Match_Range_List m = fuzzy_match_find(scratch.arena, filter, title);
                        if (m.count != m.needle_part_count)
                        {
                            continue;
                        }
                        TI_Query_Item *item = push_array(arena, TI_Query_Item, 1);
                        item->display = str8_copy(arena, title);
                        item->cfg = n->v->id;
                        item->value = push_str8f(arena, "$%I64x", n->v->id);
                        item->icon = ti_icon_kind_from_code_name(cfg_name);
                        SLLQueuePush(out.first, out.last, item);
                        out.count += 1;
                    }
                }
            }
        }
    }
    scratch_end(scratch);
    return out;
}

internal bool32 ti_query_item_is_cmd(TI_Query_Item *item)
{
    return (item->cfg == 0 &&
            item->icon != TI_IconKind_FileOutline &&
            item->icon != TI_IconKind_FolderClosedFilled &&
            ti_cmd_kind_from_string(item->value) != TI_CmdKind_Null);
}

internal bool32 ti_query_item_complete(String8 cmd_name, TI_Query_Item *item)
{
    bool32 did_cmd = 1;
    if (item->icon == TI_IconKind_FolderClosedFilled)
    {
        ti_cmd(TI_CmdKind_UpdateQuery, .string = push_str8f(ti_frame_arena(), "%S/", item->value));
    }
    else if (cmd_name.size != 0)
    {
        TI_Cmd_Kind_Info *query_info = ti_cmd_kind_info_from_string(cmd_name);
        TI_RegsScope()
        {
            ti_regs_fill_slot_from_string(query_info->query.slot, str8_zero(), item->value);
            ti_cmd(TI_CmdKind_CompleteQuery);
        }
    }
    else if (ti_query_item_is_cmd(item))
    {
        TI_Cmd_Kind_Info *info = ti_cmd_kind_info_from_string(item->value);
        if (!(info->query.flags & TI_QueryFlag_Required))
        {
            ti_push_cmd(item->value, ti_regs());
        }
        else
        {
            ti_cmd(TI_CmdKind_RunCommand, .cmd_name = item->value);
        }
        ti_cmd(TI_CmdKind_CompleteQuery);
    }
    else
    {
        did_cmd = 0;
    }
    return did_cmd;
}
