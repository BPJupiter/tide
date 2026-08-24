// Copyright (c) Epic Games Tools
// Licensed under the MIT license (https://opensource.org/license/mit/)

#undef LAYER_COLOR
#define LAYER_COLOR 0xf0a215ff

////////////////////
// Generated Code

#include "generated/broken_proxy.meta.c"

///////////////////////
// Registers Type Functions

internal void bp_regs_copy_contents(Arena *arena, BP_Regs *dst, BP_Regs *src)
{
    MemoryCopyStruct(dst, src);
    dst->file_path   = push_str8_copy(arena, src->file_path);
    dst->expr        = push_str8_copy(arena, src->expr);
    dst->string      = push_str8_copy(arena, src->string);
    dst->cmd_name    = push_str8_copy(arena, src->cmd_name);
}

internal BP_Regs *bp_regs_copy(Arena *arena, BP_Regs *src)
{
    BP_Regs *dst = push_array(arena, BP_Regs, 1);
    bp_regs_copy_contents(arena, dst, src);
    return dst;
}

///////////////////////////
// Commands Type Functions

internal void bp_cmd_list_push_new(Arena *arena, BP_Cmd_List *cmds, String8 name, BP_Regs *regs)
{
    BP_Cmd_Node *n = push_array(arena, BP_Cmd_Node, 1);
    n->cmd.name = push_str8_copy(arena, name);
    n->cmd.regs = bp_regs_copy(arena, regs);
    DLLPushBack(cmds->first, cmds->last, n);
    cmds->count += 1;
}

////////////////////////////
// View UI Rule Functions

internal BP_View_UI_Rule_Map *bp_view_ui_rule_map_make(Arena *arena, u64 slots_count)
{
    BP_View_UI_Rule_Map *map = push_array(arena, BP_View_UI_Rule_Map, 1);
    map->slots_count = slots_count;
    map->slots = push_array(arena, BP_View_UI_Rule_Slot, map->slots_count);
    return map;
}

internal void bp_view_ui_rule_map_insert(Arena *arena, BP_View_UI_Rule_Map *map, String8 string, BP_View_UI_Function_Type *ui)
{
    u64 hash = d_hash_from_string(string);
    u64 slot_idx = hash & map->slots_count;
    BP_View_UI_Rule_Node *n = push_array(arena, BP_View_UI_Rule_Node, 1);
    n->v.name = push_str8_copy(arena, string);
    n->v.ui = ui;
    SLLQueuePush(map->slots[slot_idx].first, map->slots[slot_idx].last, n);
}

internal BP_View_UI_Rule *bp_view_ui_rule_from_string(String8 string)
{
    BP_View_UI_Rule *rule = &bp_nil_view_ui_rule;
    {
        BP_View_UI_Rule_Map *map = bp_state->view_ui_rule_map;
        u64 hash = d_hash_from_string(string);
        u64 slot_idx = hash % map->slots_count;
        for (BP_View_UI_Rule_Node *n = map->slots[slot_idx].first; n != 0; n = n->next)
        {
            if (str8_match(n->v.name, string, 0))
            {
                rule = &n->v;
                break;
            }
        }
    }
    return rule;
}

//////////////////////////////////////////////////////
// Global Cross-Window UI Interaction State Functions

internal bool32 bp_drag_is_active(void)
{
    return ((bp_state->drag_drop_state == BP_DragDropState_Dragging) ||
            (bp_state->drag_drop_state == BP_DragDropState_Dropping));
}

internal void bp_drag_begin(BP_Reg_Slot slot)
{
    if (!bp_drag_is_active())
    {
        arena_clear(bp_state->drag_drop_arena);
        bp_state->drag_drop_regs = bp_regs_copy(bp_state->drag_drop_arena, bp_regs());
        bp_state->drag_drop_regs_slot = slot;
        bp_state->drag_drop_state = BP_DragDropState_Dragging;
    }
}

internal bool32 bp_drag_drop(void)
{
    bool32 result = false;
    if (bp_state->drag_drop_state == BP_DragDropState_Dropping)
    {
        result = true;
        bp_state->drag_drop_state = BP_DragDropState_Null;
    }
    return result;
}

internal void bp_drag_kill(void)
{
    bp_state->drag_drop_state = BP_DragDropState_Null;
}

internal void bp_set_hover_regs(BP_Reg_Slot slot)
{
    bp_state->next_hover_regs = bp_regs_copy(bp_frame_arena(), bp_regs());
    bp_state->next_hover_regs_slot = slot;
}

internal BP_Regs *bp_get_hover_regs(void)
{
    return bp_state->hover_regs;
}

/////////////////////
// Config Functions

internal bool32 bp_cfg_is_project_filtered(CFG_Node *cfg)
{
    CFG_Node *project = cfg_node_child_from_string(cfg, str8_lit("project"));
    bool32 result = (project != &cfg_nil_node &&
                     project->first->string.size != 0 &&
                     !path_match_normalised(bp_state->project_path, project->first->string));
    return result;
}

internal Vec4f32 bp_hsva_from_cfg(CFG_Node *cfg)
{
    Vec4f32 hsva = {0};
    CFG_Node *hsva_root = cfg_node_child_from_string(cfg, str8_lit("hsva"));
    CFG_Node *h = hsva_root->first;
    CFG_Node *s = h->next;
    CFG_Node *v = s->next;
    CFG_Node *a = v->next;
    hsva.x = (f32)f64_from_str8(h->string);
    hsva.y = (f32)f64_from_str8(s->string);
    hsva.z = (f32)f64_from_str8(v->string);
    hsva.w = (f32)f64_from_str8(a->string);
    return hsva;
}

internal Vec4f32 bp_color_from_cfg(CFG_Node *cfg)
{
    Vec4f32 hsva = bp_hsva_from_cfg(cfg);
    Vec4f32 rgba = linear_from_srgba(rgba_from_hsva(hsva));
    return rgba;
}

internal bool32 bp_disabled_from_cfg(CFG_Node *cfg)
{
    Temp scratch = scratch_begin(0, 0);
    MD_Node *child_schema = &md_nil_node;
    MD_Node_Ptr_List schemas = cfg_schemas_from_name(scratch.arena, bp_state->cfg_schema_table, cfg->string);
    for (MD_Node_Ptr_Node *n = schemas.first; n != 0 && child_schema == &md_nil_node; n = n->next)
    {
        child_schema = md_child_from_string(n->v, str8_lit("enabled"), 0);
    }
    MD_Node *default_tag = md_tag_from_string(child_schema, str8_lit("default"), 0);
    String8 value_string = cfg_node_child_from_string(cfg, str8_lit("enabled"))->first->string;
    if (value_string.size == 0)
    {
        value_string = default_tag->first->string;
    }
    u64 value_u64 = 0;
    try_u64_from_str8_c_rules(value_string, &value_u64);
    bool32 is_enabled = !!value_u64;
    bool32 is_disabled = !is_enabled;
    if (value_string.size == 0)
    {
        is_disabled = false;
    }
    scratch_end(scratch);
    return is_disabled;
}

internal String8 bp_name_from_cfg(CFG_Node *cfg)
{
    CFG_Node *name_root = cfg_node_child_from_string(cfg, str8_lit("name"));
    String8 result = name_root->first->string;
    return result;
}

internal String8 bp_label_from_cfg(CFG_Node *cfg)
{
    CFG_Node *label_root = cfg_node_child_from_string(cfg, str8_lit("label"));
    String8 result = label_root->first->string;
    return result;
}

internal String8 bp_path_from_cfg(CFG_Node *cfg)
{
    CFG_Node *root = cfg_node_child_from_string(cfg, str8_lit("path"));
    String8 result = root->first->string;
    return result;
}

internal String8 bp_default_setting_from_names(String8 schema_name, String8 setting_name)
{
    String8 result = {0};
    {
        Temp scratch = scratch_begin(0, 0);
        MD_Node *setting_schema = &md_nil_node;
        MD_Node_Ptr_List schemas = cfg_schemas_from_name(scratch.arena, bp_state->cfg_schema_table, schema_name);
        for (MD_Node_Ptr_Node *n = schemas.first; n != 0 && setting_schema == &md_nil_node; n = n->next)
        {
            setting_schema = md_child_from_string(n->v, setting_name, 0);
        }
        if (setting_schema != &md_nil_node)
        {
            MD_Node *default_tag = md_tag_from_string(setting_schema, str8_lit("default"), 0);
            if (default_tag != &md_nil_node)
            {
                result = default_tag->first->string;
            }
        }
        scratch_end(scratch);
    }
    return result;
}

internal String8 bp_setting_from_name(String8 name)
{
    String8 result = {0};
    if (name.size != 0)
    {
        Temp scratch = scratch_begin(0, 0);

        // find most-granular config scopes to begin looking for the setting
        typedef struct Cfg_Seed_Task Cfg_Seed_Task;
        struct Cfg_Seed_Task {
            Cfg_Seed_Task *next;
            CFG_Node *cfg;
            bool32 allow_bucket_chains;
        };
        CFG_Node *view_cfg = cfg_node_from_id(bp_regs()->view);
        if (view_cfg == &cfg_nil_node)
        {
            view_cfg = cfg_node_from_id(bp_regs()->tab);
        }
        Cfg_Seed_Task panel_task = {0, &cfg_nil_node, true};
        if (panel_task.cfg == &cfg_nil_node) { panel_task.cfg = cfg_node_from_id(bp_regs()->panel); }
        if (panel_task.cfg == &cfg_nil_node) { panel_task.cfg = cfg_node_from_id(bp_regs()->window); }
        if (panel_task.cfg == &cfg_nil_node) { panel_task.cfg = cfg_node_child_from_string(cfg_node_root(), s("user")); }
        Cfg_Seed_Task view_task = {&panel_task, view_cfg, true};
        Cfg_Seed_Task *first_task = &view_task;
        Cfg_Seed_Task *last_task = &panel_task;

        // For each task, look for the setting, follow parent chain upwards
        CFG_Node *setting = &cfg_nil_node;
        for (Cfg_Seed_Task *t = first_task; t != 0; t = t->next)
        {
            for (CFG_Node *cfg = t->cfg; cfg != &cfg_nil_node; cfg = cfg->parent)
            {
                setting = cfg_node_child_from_string(cfg, name);
                if (setting != &cfg_nil_node)
                {
                    goto break_all;
                }
                if (cfg->parent == cfg_node_root() && t->allow_bucket_chains)
                {
                    String8 next_bucket = {0};
                    bool32 allow_bucket_chains = false;
                    if (str8_match(cfg->string, str8_lit("user"), 0))
                    {
                        next_bucket = str8_lit("project");
                    }
                    else if (str8_match(cfg->string, str8_lit("project"), 0))
                    {
                        next_bucket = str8_lit("user");
                    }
                    else
                    {
                        allow_bucket_chains = true;
                        next_bucket = str8_lit("user");
                    }
                    if (next_bucket.size != 0)
                    {
                        Cfg_Seed_Task *task = push_array(scratch.arena, Cfg_Seed_Task, 1);
                        SLLQueuePush(first_task, last_task, task);
                        task->cfg = cfg_node_child_from_string(cfg_node_root(), next_bucket);
                        task->allow_bucket_chains = allow_bucket_chains;
                    }
                }
            }
        }
        break_all:;

        // return resultant child string stored under this key
        result = setting->first->string;

        // no result -> look for default in schemas
        if (result.size == 0)
        {
            for (Cfg_Seed_Task *t = first_task; t != 0; t = t->next)
            {
                for (CFG_Node *cfg = t->cfg; cfg != &cfg_nil_node; cfg = cfg->parent)
                {
                    result = bp_default_setting_from_names(cfg->string, name);
                    if (result.size != 0)
                    {
                        goto break_all2;
                    }
                }
            }
            break_all2:;
        }

        scratch_end(scratch);
    }
    return result;
}

internal bool32 bp_setting_bool32_from_name(String8 name)
{
    bool32 result = false;
    String8 value = bp_setting_from_name(name);
    if (value.size != 0)
    {
        u64 value_u64 = 0;
        try_u64_from_str8_c_rules(value, &value_u64);
        result = !!value_u64;
    }
    return result;
}

internal u64 bp_setting_u64_from_name(String8 name)
{
    u64 result = 0;
    String8 value = bp_setting_from_name(name);
    if (value.size != 0)
    {
        try_u64_from_c_rules(value, &result);
    }
    return result;
}

internal f32 bp_setting_f32_from_name(String8 name)
{
    f32 result = 0.f;
    String8 value = bp_setting_from_name(name);
    if (value.size != 0)
    {
        if (value.str[value.size - 1] == 'f' || value.str[value.size - 1] == 'F')
        {
            value = str8_chop(value, 1);
        }
        result = (f32)f64_from_str8(value);
    }
    return result;
}

internal CFG_Node *bp_immediate_cfg_from_key(String8 string)
{
    CFG_Node *transient = cfg_node_child_from_string(cfg_node_root(), str8_lit("transient"));
    CFG_Node *immediate = &cfg_nil_node;
    CFG_Node *cfg = &cfg_nil_node;
    for (CFG_Node *child = transient->first; child != &cfg_nil_node; child = child->next)
    {
        if (str8_match(child->string, str8_lit("immediate"), 0))
        {
            cfg = cfg_node_child_from_string(child, string);
            if (cfg != &cfg_nil_node)
            {
                immediate = child;
                break;
            }
        }
    }
    if (cfg == &cfg_nil_node)
    {
        immediate = cfg_node_new(bp_state->cfg, transient, str8_lit("immediate"));
        cfg = cfg_node_new(bp_state->cfg, immediate, string);
    }
    cfg_node_child_from_string_or_alloc(bp_state->cfg, immediate, str8_lit("hot"));
    return cfg;
}

internal CFG_Node *bp_immediate_cfg_from_keyf(char *fmt, ...)
{
    Temp scratch = scratch_begin(0, 0);
    va_list args;
    va_start(args, fmt);
    String8 key = push_str8fv(scratch.arena, fmt, args);
    CFG_Node *result = bp_immediate_cfg_from_key(key);
    va_end(args);
    scratch_end(scratch);
    return result;
}

////////////////////
// View Functions

internal BP_View_State *bp_view_state_from_cfg(CFG_Node *cfg)
{
    BP_View_State *view_state = &bp_nil_view_state;
    CFG_ID id = cfg->id;
    if (id != 0 &&
        id == bp_state->view_state_last_accessed_id &&
        id == bp_state->view_state_last_accessed->cfg_id)
    {
        view_state = bp_state->view_state_last_accessed;
    }
    else
    {
        u64 hash = d_hash_from_string(str8_struct(&id));
        u64 slot_idx = hash % bp_state->view_state_slots_count;
        BP_View_State_Slot *slot = &bp_state->view_state_slots[slot_idx];
        for (BP_View_State *v = slot->first; v != 0; v = v->hash_next)
        {
            if (v->cfg_id == id)
            {
                view_state = v;
                break;
            }
        }
    }
    if (view_state == &bp_nil_view_state)
    {
        view_state = bp_state->free_view_state;
        if (view_state)
        {
            SLLStackPop_N(bp_state->free_view_state, hash_next);
        }
        else
        {
            view_state = push_array(bp_state->arena, BP_View_State, 1);
        }
        MemoryCopyStruct(view_state, &bp_nil_view_state);
        u64 hash = d_hash_from_string(str8_struct(&id));
        u64 slot_idx = hash % bp_state->view_state_slots_count;
        BP_View_State_Slot *slot = &bp_state->view_state_slots[slot_idx];
        DLLPushBack_NP(slot->first, slot->last, view_state, hash_next, hash_prev);
        view_state->cfg_id = id;
        view_state->arena = arena_alloc();
        view_state->arena_reset_pos = arena_pos(view_state->arena);
    }
    if (view_state != &bp_nil_view_state)
    {
        view_state->last_frame_index_touched = bp_state->frame_index;
    }
    bp_state->view_state_last_accessed = view_state;
    bp_state->view_state_last_accessed_id = id;
    return view_state;
}

internal void bp_view_ui(Rng2f32 rect)
{
    ProfBeginFunction();
    CFG_Node *view = cfg_node_from_id(bp_regs()->view);
    BP_View_State *vs = bp_view_state_from_cfg(view);
    String8 view_name = view->string;
    bool32 view_is_floating = false;
    for (CFG_Node *p = view->parent; p != &cfg_nil_node; p = p->parent)
    {
        if (str8_match(p->string, str8_lit("immediate"), 0))
        {
            view_is_floating = true;
            break;
        }
    }

    /////////////////////
    // query extension

    CFG_Node *query_root = cfg_node_child_from_string(view, str8_lit("query"));
    CFG_Node *input_root = cfg_node_child_from_string(query_root, str8_lit("input"));
    CFG_Node *cmd_root = cfg_node_child_from_string(query_root, str8_lit("cmd"));
    String8 current_input = input_root->first->string;
    bool32 search_row_is_open = (vs->query_is_open);
    f32 search_row_open_t = ui_anim(ui_key_from_stringf(ui_key_zero(), "search_row_open_%p", view),
                                    (f32)!!search_row_is_open,
                                    .initial = (f32)!!search_row_is_open,
                                    .epsilon = 0.01f,
                                    .rate    = bp_state->menu_animation_rate);
    if (search_row_open_t > 0.001f)
    {
        String8 cmd_name = cmd_root->first->string;
        BP_IconKind icon = bp_icon_kind_from_code_name(cmd_name);
        BP_Cmd_Kind_Info *cmd_kind_info = bp_cmd_kind_info_from_string(cmd_name);

        // store cfg's string to view's
        vs->query_string_size = Min(sizeof(vs->query_buffer), current_input.size);
        MemoryCopy(vs->query_buffer, current_input.str, vs->query_string_size);

        // determine dimensions
        f32 search_row_height_target = ui_top_px_height();
        f32 search_row_height = search_row_open_t * search_row_height_target;
        search_row_height = Min(search_row_height, dim_2f32(rect).y);
        rect.y0 += search_row_height;
        rect.y0 = floor_f32(rect.y0);

        // build container
        UI_Box *search_row = &ui_nil_box;
        UI_PrefHeight(ui_px(search_row_height, 1.f))
        {
            if (!vs->contents_are_focused)
            {
                ui_set_next_border_color(ui_color_from_name(s("border")));
            }
            search_row = ui_build_box_from_stringf(UI_BoxFlag_DrawBorder|UI_BoxFlag_DrawDropShadow, "###search");
        }

        // build contents
        UI_Parent(search_row) UI_WidthFill UI_HeightFill UI_Focus(vs->query_is_open && !vs->contents_are_focused ? UI_FocusKind_On : UI_FocusKind_Off)
            BP_Font(cmd_kind_info->query.flags & BP_QueryFlag_CodeInput ? BP_FontSlot_Code : BP_FontSlot_Main)
            UI_Flags(UI_BoxFlag_DisableFocusOverlay | UI_BoxFlag_DisableFocusBorder)
        {
            UI_TagF("weak")
            {
                if (cmd_name.size != 0)
                {
                    ui_spacer(ui_em(0.5f, 1.f));
                    UI_TextAlignment(UI_TextAlign_Center)
                        UI_Transparency(1 - search_row_open_t)
                        UI_PrefWitdth(ui_em(3.f, 1.f))
                        BP_Font(BP_FontSlot_Icons)
                        ui_label(bp_icon_kind_text_table[icon == BP_IconKind_Null ? BP_IconKind_Find : icon]);
                    UI_Transparency(1 - search_row_open_t)
                        BP_Font(BP_FontSlot_Main) UI_PrefWidth(ui_text_dim(1, 1))
                        UI_FontSize(ui_top_font_size()*0.85f)
                        ui_label(bp_display_from_code_name(cmd_name));
                }
                else
                {
                    ui_spacer(ui_em(0.5f, 1.f));
                    UI_TextAlignment(UI_TextAlign_Center) BP_Font(BP_FontSlot_Icons) UI_PrefWidth(ui_em(2.f, 1.f))
                        ui_label(bp_icon_kind_text_table[BP_IconKind_Find]);
                }
            }
            UI_Key line_edit_key = {0};
            BP_Cell_Params params = {0};
            {
                params.flags |= !!(cmd_kind_info->query.flags & BP_QueryFlag_CodeInput) * BP_CellFlag_CodeContents;
                params.flags |= BP_CellFlag_NoBackground;
                params.cursor               = &vs->query_cursor;
                params.mark                 = &vs->query_mark;
                params.edit_buffer          = vs->query_buffer;
                params.edit_string_size_out = &vs->query_string_size;
                params.edit_buffer_size     = sizeof(vs->query_buffer);
                params.pre_edit_value       = current_input;
                params.line_edit_key_out    = &line_edit_key;
            }
            UI_Transparency(1 - search_row_open_t)
            {
                UI_Signal sig = bp_cellf(&params, "###search");
                if (ui_pressed(sig))
                {
                    vs->query_is_open = true;
                    vs->contents_are_focused = false;
                    bp_cmd(BP_CmdKind_FocusPanel);
                }
            }
        }

        // commit string to view
        if (input_root == &cfg_nil_node)
        {
            input_root = cfg_node_child_from_string_or_alloc(bp_state->cfg, query_root, str8_lit("input"));
        }
        cfg_node_new_replace(bp_state->cfg, input_root, str8(vs->query_buffer, vs->query_string_size));
    }

    /////////////////////////////
    // build main view container
    UI_Box *view_container = &ui_nil_box;
    UI_WidthFill UI_HeightFill
    {
        view_container = ui_build_box_from_key(0, ui_key_zero());
    }

    //////////////////////////
    // fill view container
    UI_Parent(view_container)
        UI_FontSize(bp_font_size())
        UI_PrefHeight(ui_px(floor_f32(ui_top_font_size() * bp_setting_f32_from_name(str8_lit8("row_height"))), 1.f))
    {
        if (0){}
        else if (str8_match(view_name, str8_lit("getting_started"), 0))
        {
            Temp scratch = scratch_begin(0, 0);
            ui_set_next_flags(UI_BoxFlag_DefaultFocusNav);
            UI_Focus(UI_FocusKind_On) UI_WidthFill UI_HeightFill UI_NamedColumn(str8_lit("empty_view"))
                UI_Padding(ui_pct(1, 0)) UI_Focus(UI_FocusKind_Null)
            {
                // icons & info
                UI_Padding(ui_em(2.f, 1.f)) UI_TagF("weak")
                {
                    // icon
                    {
                        f32 icon_dim = ui_top_font_size() * 10.f;
                        UI_PrefHeight(ui_px(icon_dim, 1.f))
                            UI_Row
                            UI_Padding(ui_pct(1, 0))
                            UI_PrefWdith(ui_px(icon_dim, 1.f))
                        {
                            R_Handle texture = bp_state->icon_texture;
                            Vec2s32 texture_dim = r_size_from_tex2d(texture);
                            ui_image(texture,
                                     R_Tex2DSampleKind_Linear,
                                     r2f32p(0, 0, texture_dim.x, texture_dim.y),
                                     v4f32(1, 1, 1, 1),
                                     0,
                                     str8_lit(""));
                        }
                    }

                    // info
                    UI_Padding(ui_em(2.f, 1.f))
                        UI_WidthFill UI_PrefHeight(ui_em(2.f, 1.f))
                        UI_Row
                        UI_Padding(ui_pct(1, 0))
                        UI_TextAlignment(UI_TextAlign_Center)
                        UI_PrefWidth(ui_text_dim(10, 1))
                    {
                        ui_label(str8_lit(BUILD_TITLE_STRING_LITERAL));
                    }
                }

                // helper text for command lister activation
                UI_TagF("weak")
                    UI_PrefHeight(ui_em(2.25f, 1.f)) UI_Row
                    UI_PrefWdith(ui_text_dim(10, 1))
                    UI_TextAlignment(UI_TextAlign_Center)
                    UI_Padding(ui_pct(1, 0))
                {
                    ui_labelf("use");
                    UI_TextAlignment(UI_TextAlign_Center) bp_cmd_binding_buttons(bp_cmd_kind_info_table[BP_CmdKind_OpenPalette].string, s(""), 1, 0);
                    ui_labelf("to search for commands and options");
                }
            }
            scratch_end(scratch);
        }
        ////////////////////
        // visualiser hook

        Temp scratch = scratch_begin(0, 0);
        BP_View_UI_Rule *view_ui_rule = bp_view_ui_rule_from_string(view_name);

        // peek presses, steal focus from query bar
        for (UI_Event *evt = 0; ui_next_event(&evt);)
        {
            if (evt->kind == UI_EventKind_Press && contains_2f32(rect, evt->pos))
            {
                vs->contents_are_focused = true;
                break;
            }
        }

        // 'pull out' button, if floating
        if (view_is_floating)
        {
            UI_Signal pull_out_sig = {0};
            UI_TagF(".") UI_TagF("tab") UI_Rect(r2f32p(floor_f32(ui_top_font_size() * 1.5f),
                                                       floor_f32(ui_top_font_size() * 1.5f),
                                                       floor_f32(ui_top_font_size() * 1.5f + ui_top_font_size() * 3.f),
                                                       floor_f32(ui_top_font_size() * 1.5f + ui_top_font_size() * 3.f)))
                UI_CornerRadius(floor_f32(ui_top_font_size() * 1.5f))
                UI_TextAlignment(UI_TextAlignCenter)
                BP_Font(BP_FontSlot_Icons)
                UI_FontSize(floor_f32(ui_top_font_size() * 0.9f))
            {
                UI_Box *box = ui_build_box_from_stringf(UI_BoxFlag_Clickable|
                                                        UI_BoxFlag_Floating|
                                                        UI_BoxFlag_DrawText|
                                                        UI_BoxFlag_DrawBorder|
                                                        UI_BoxFlag_DrawBackground|
                                                        UI_BoxFlag_DrawActiveEffects|
                                                        UI_BoxFlag_DrawHotEffects,
                                                        "%S###pull_out",
                                                        bp_icon_kind_text_table[BP_IconKind_Window]);
                pull_out_sig = ui_signal_from_box(box);
            }
            if (ui_dragging(pull_out_sig) && !contains_2f32(pull_out_sig.box->rect, ui_mouse()))
            {
                bp_drag_begin(BP_RegSlot_View);
            }
            if (ui_hovering(pull_out_sig)) UI_Tooltip BP_Font(BP_FontSlot_Main)
            {
                ui_state->tooltip_achor_key = pull_out_sig.box->key;
                ui_labelf("Pull Out As New Tab");
            }
        }

        // build ui via hook
        view_ui_rule->ui(view, rect);

        scratch_end(scratch);
    }

    ///////////////////////////////
    // catchall completion controls

    if (vs->query_is_open) UI_Focus(UI_FocusKind_On)
    {
        if (ui_is_focus_active() && ui_slot_press(UI_EventActionSlot_Cancel))
        {
            vs->query_is_open = false;
            vs->query_string_size = false;
        }
        if (ui_is_focus_active() && ui_slot_press(UI_EventActionSlot_Accept))
        {
            String8 cmd_name = bp_view_query_cmd();
            String8 input = bp_view_query_input();
            BP_Cmd_Kind_Info *cmd_kind_info = bp_cmd_kind_info_from_string(cmd_name);
            BP_RegsScope()
            {
                bp_regs_fill_slot_from_string(cmd_kind_info->query.slot, str8_zero(), input);
                bp_cmd(BP_CmdKind_CompleteQuery);
            }
        }
    }

    vs->last_frame_index_built = bp_state->frame_index;
    ProfEnd();
}

///////////////////////
// View Building API

// view info extraction

internal Arena *bp_view_arena(void)
{
    CFG_Node *view = cfg_node_from_id(bp_regs()->view);
    BP_View_State *view_state = bp_view_state_from_cfg(view);
    return view_state->arena;
}

internal UI_Scroll_Pt2 bp_view_scroll_pos(void)
{
    CFG_Node *view = cfg_node_from_id(bp_regs()->view);
    BP_View_State *view_state = bp_view_state_from_cfg(view);
    return view_state->scroll_pos;
}

internal String8 bp_view_query_cmd(void)
{
    CFG_Node *view = cfg_node_from_id(bp_regs()->view);
    CFG_Node *query = cfg_node_child_from_string(view, str8_lit("query"));
    CFG_Node *cmd = cfg_node_child_from_string(query, str8_lit("cmd"));
    String8 string = cmd->first->string;
    return string;
}

internal String8 bp_view_query_input(void)
{
    CFG_Node *view = cfg_node_from_id(bp_regs()->view);
    CFG_Node *query = cfg_node_child_from_string(view, str8_lit("query"));
    CFG_Node *input = cfg_node_child_from_string(query, str8_lit("input"));
    String8 string = input->first->string;
    return string;
}

internal String8 bp_view_setting_from_name(String8 string)
{
    CFG_Node *view = cfg_node_from_id(bp_regs()->view);
    String8 result = cfg_node_child_from_string(view, name)->first->string;
    if (result.size == 0)
    {
        result = bp_default_setting_from_names(view->string, name);
    }
    return result;
}

internal bool32 bp_view_setting_bool32_from_name(String8 string)
{
    String8 string = bp_view_setting_from_name(name);
    u64 value_u64 = 0;
    try_u64_from_str8_c_rules(string, &value_u64);
    bool32 result = !!value_u64;
    return result;
}

internal u64 bp_view_setting_u64_from_name(String8 string)
{
    String8 string = bp_view_setting_from_name(name);
    u64 result = 0;
    try_u64_from_str8_c_rules(string, &result);
    return result;
}

internal f32 bp_view_setting_f32_from_name(String8 string)
{
    String8 string = bp_view_setting_from_name(name);
    if (string.size != 0 && (string.str[string.size] - 1] == 'f' || string.str[string.size - 1] == 'F')
    {
        string = str8_chop(string, 1);
    }
    f32 result = (f32)f64_from_str8(string);
    return result;
}

// language kind for tab, from its file path (user for syntax highlighting)
internal TXT_LangKind bp_lang_kind_from_file_path(String8 file_path)
{
    TXT_LandKind kind = TXT_LangKind_Null;
    if (file_path.size != 0)
    {
        kind = txt_lang_kind_from_extension(str8_skip_last_dot(file_path));
    }
    return kind;
}

// pushing/attaching view resources
internal void *bp_view_state_by_size(u64 size)
{
    CFG_Node *view = cfg_node_from_id(bp_regs()->view);
    BP_View_State *view_state = bp_view_state_from_cfg(view);
    if (view_state->user_data == 0)
    {
        view_state->user_data = push_array(view_state->arena, u8, size);
    }
    return view_state->user_data;
}

internal Arena *bp_push_view_arena(void)
{
    CFG_Node *view = cfg_node_from_id(bp_regs()->view);
    BP_View_State *view_state = bp_view_state_from_cfg(view);
    BP_Arena_Ext *ext = push_array(view_state->arena, BP_Arena_Ext, 1);
    ext->arena = arena_alloc();
    SLLQueuePush(view_state->first_arena_ext, view_state->last_arena_ext, ext);
    return ext->arena;
}

// storing view-attached state
internal void bp_store_view_loading_info(bool32 is_loading, u64 progress_u64, u64 progress_u64_target)
{
    CFG_Node *view = cfg_node_from_id(bp_regs()->view);
    BP_View_State *view_state = bp_view_state_from_cfg(view);
    bool32 loading_state_is_new = (is_loading && view_state->loading_t_target != (f32)!!is_loading);
    view_state->loading_t_target = (f32)!!is_loading;
    view_state->loading_progress_v = progress_u64;
    view_state->loading_progress_v_target = progress_u64_target;
    if(loading_state_is_new || view_state->last_frame_index_built+1 < bp_state->frame_index)
    {
        view_state->loading_t = view_state->loading_t_target;
    }
}

internal void bp_store_view_scroll_pos(UI_Scroll_Pt2 pos)
{
    CFG_Node *view = cfg_node_from_id(bp_regs()->view);
    BP_View_State *view_state = bp_view_state_from_cfg(view);
    view_state->scroll_pos = pos;
}

internal void bp_store_view_param(String8 key, String8 value)
{
    CFG_Node *view = cfg_node_from_id(bp_regs()->view);
    CFG_Node *child = cfg_node_child_from_string_or_alloc(bp_state->cfg, view, key);
    cfg_node_new_replace(bp_state->cfg, child, value);
}
 
internal void bp_store_view_paramf(String8 key, char *fmt, ...)
{
    Temp scratch = scratch_begin(0, 0);
    va_list args;
    va_start(args, fmt);
    String8 string = push_str8fv(scratch.arena, fmt, args);
    bp_store_view_param(key, string);
    va_end(args);
    scratch_end(scratch);
}

/////////////////////
// Window Functions

internal String8 bp_push_window_title(Arena *arena)
{
    CFG_Node *root = cfg_node_root();
    CFG_Node *project = cfg_node_child_from_string(root, str8_lit("project"));
    CFG_Node *name = cfg_node_child_from_string(project, str8_lit("name"));
    String8 project_name = name->first->string;
    if(project_name.size == 0)
    {
        String8 prof_path = bp_state->project_path;
        prof_path = str8_chop_last_dot(prof_path);
        project_name = str8_skip_last_slash(prof_path);
    }
    String8 result = push_str8f(arena, "%S%s%s", project_name, project_name.size != 0 ? " - " : "", BUILD_TITLE " (" BUILD_VERSION_STRING_LITERAL " " BUILD_RELEASE_PHASE_STRING_LITERAL ")");
    return result;
}

internal CFG_Node *bp_window_from_cfg(CFG_Node *cfg)
{
    CFG_Node *result = &cfg_nil_node;
    for(CFG_Node *c = cfg; c != &cfg_nil_node; c = c->parent)
    {
        if(c->parent->parent == cfg_node_root() && str8_match(c->string, str8_lit("window"), 0))
        {
            result = c;
            break;
        }
    }
    return result;
}

internal BP_Window_State *bp_window_state_from_cfg(CFG_Node *cfg)
{
    //- rjf: unpack
    CFG_Node *window_cfg = bp_window_from_cfg(cfg);
    CFG_ID id = window_cfg->id;
  
    //- rjf: scan for existing window
    BP_Window_State *ws = &bp_nil_window_state;
    if(id != 0 &&
       id == bp_state->window_state_last_accessed_id &&
       id == bp_state->window_state_last_accessed->cfg_id)
    {
        ws = bp_state->window_state_last_accessed;
    }
    else
    {
        u64 hash = d_hash_from_string(str8_struct(&id));
        u64 slot_idx = hash%bp_state->window_state_slots_count;
        BP_Window_StateSlot *slot = &bp_state->window_state_slots[slot_idx];
        for(BP_Window_State *w = slot->first; w != 0; w = w->hash_next)
        {
            if(w->cfg_id == id)
            {
                ws = w;
                break;
            }
        }
    }
  
    //- rjf: allocate/open new window if one was not found
    if(window_cfg != &cfg_nil_node && ws == &bp_nil_window_state)
    {
        Temp scratch = scratch_begin(0, 0);
    
        // rjf: unpack configuration options
        bool32 has_pos = 0;
        Vec2f32 pos = {0};
        Vec2f32 size = {0};
        WM_Monitor preferred_monitor = {0};
        {
            CFG_Node *pos_cfg = cfg_node_child_from_string(window_cfg, str8_lit("pos"));
            has_pos = (pos_cfg != &cfg_nil_node);
            CFG_Node *size_cfg = cfg_node_child_from_string(window_cfg, str8_lit("size"));
            CFG_Node *monitor_cfg = cfg_node_child_from_string(window_cfg, str8_lit("monitor"));
            pos.x = (f32)f64_from_str8(pos_cfg->first->string);
            pos.y = (f32)f64_from_str8(pos_cfg->first->next->string);
            size.x = (f32)f64_from_str8(size_cfg->first->string);
            size.y = (f32)f64_from_str8(size_cfg->first->next->string);
            WM_MonitorArray monitors = wm_push_monitors_array(scratch.arena);
            for EachIndex(idx, monitors.count)
                         {
                             String8 monitor_name = wm_name_from_monitor(scratch.arena, monitors.v[idx]);
                             if(str8_match(monitor_name, monitor_cfg->first->string, StringMatchFlag_CaseInsensitive))
                             {
                                 preferred_monitor = monitors.v[idx];
                                 break;
                             }
                         }
        }
    
        // rjf: allocate window
        ws = bp_state->free_window_state;
        if(ws != 0)
        {
            SLLStackPop_N(bp_state->free_window_state, order_next);
        }
        else
        {
            ws = push_array_no_zero(bp_state->arena, BP_Window_State, 1);
        }
        MemoryZeroStruct(ws);
    
        // rjf: fill out window
        ws->cfg_id = id;
        ws->arena = arena_alloc();
        {
            String8 title = bp_push_window_title(scratch.arena);
            ws->os = wm_window_open(r2f32p(pos.x, pos.y, pos.x+size.x, pos.y+size.y), (!has_pos*WM_WindowFlag_UseDefaultPosition)|WM_WindowFlag_CustomBorder, title);
        }
        ws->r = r_window_equip(ws->os);
        ws->ui = ui_state_alloc();
        ws->drop_completion_arena = arena_alloc();
        ws->query_arena = arena_alloc();
        ws->hover_eval_arena = arena_alloc();
        ws->autocomp_arena = arena_alloc();
        ws->last_dpi = wm_dpi_from_window(ws->os);
        WM_Monitor zero_monitor = {0};
        if(!wm_monitor_match(zero_monitor, preferred_monitor))
        {
            wm_window_set_monitor(ws->os, preferred_monitor);
        }
        if(cfg_node_child_from_string(window_cfg, str8_lit("fullscreen")) != &cfg_nil_node)
        {
            wm_window_set_fullscreen(ws->os, 1);
        }
        if(cfg_node_child_from_string(window_cfg, str8_lit("maximized")) != &cfg_nil_node)
        {
            wm_window_set_maximized(ws->os, 1);
        }
    
        // rjf: hook up window links
        u64 hash = d_hash_from_string(str8_struct(&id));
        u64 slot_idx = hash%bp_state->window_state_slots_count;
        BP_Window_StateSlot *slot = &bp_state->window_state_slots[slot_idx];
        DLLPushBack_NPZ(&bp_nil_window_state, bp_state->first_window_state, bp_state->last_window_state, ws, order_next, order_prev);
        DLLPushBack_NP(slot->first, slot->last, ws, hash_next, hash_prev);
    
        scratch_end(scratch);
    }
  
    //- rjf: touch window for this frame
    if(ws != &bp_nil_window_state)
    {
        ws->last_frame_index_touched = bp_state->frame_index;
    }
  
    bp_state->window_state_last_accessed_id = ws->cfg_id;
    bp_state->window_state_last_accessed = ws;
    return ws;
}

internal BP_Window_State *bp_window_state_from_os_handle(WM_Window os)
{
    BP_Window_State *ws = &bp_nil_window_state;
    {
        for(BP_Window_State *w = bp_state->first_window_state;
            w != &bp_nil_window_state;
            w = w->order_next)
        {
            if(wm_window_match(w->os, os))
            {
                ws = w;
                break;
            }
        }
    }
    return ws;
}

#if COMPILER_MSVC && !BUILD_DEBUG
NO_OPTIMIZE_BEGIN
#endif

internal void bp_window_frame(void)
{
    Temp scratch = scratch_begin(0, 0);
    ProfBeginFunction();
    
    //////////////////////////////
    //- rjf: @window_frame_part unpack context
    //
    CFG_Node *window          = cfg_node_from_id(bp_regs()->window);
    BP_Window_State *ws      = bp_window_state_from_cfg(cfg_node_from_id(bp_regs()->window));
    CFG_PanelTree panel_tree = cfg_panel_tree_from_cfg(scratch.arena, window);
    bool32 window_is_focused   = wm_window_is_focused(ws->os);
    bool32 popup_is_open       = (bp_state->popup_active);
    bool32 query_is_open       = (ws->query_is_active);
    if(!window_is_focused || popup_is_open)
    {
        ws->menu_bar_key_held = 0;
        ws->menu_bar_focus_press_started = 0;
    }
    ui_select_state(ws->ui);
    
    //////////////////////////////
    //- rjf: @window_frame_part fill panel/view interaction registers
    //
    bp_regs()->panel = panel_tree.focused->cfg->id;
    bp_regs()->tab   = panel_tree.focused->selected_tab->id;
    bp_regs()->view = panel_tree.focused->selected_tab->id;
    
    //////////////////////////////
    //- rjf: @window_frame_part compute window's theme
    //
    {
        Access *access = access_open();
        
        //- rjf: try to find theme settings from the project, then the user.
        CFG_Node_Ptr_List colors_cfgs = {0};
        CFG_Node *theme_parents[] = {
            cfg_node_child_from_string(cfg_node_root(), str8_lit("project")),
            cfg_node_child_from_string(cfg_node_root(), str8_lit("user"))
        };
        CFG_Node *theme_cfgs[] = {
            &cfg_nil_node,
            &cfg_nil_node,
        };
        for EachIndex(idx, ArrayCount(theme_parents))
        {
            CFG_Node *parent_cfg = theme_parents[idx];
            if(theme_cfgs[idx] == &cfg_nil_node)
            {
                CFG_Node *possible_theme_cfg = cfg_node_child_from_string(parent_cfg, str8_lit("theme"));
                if(possible_theme_cfg != &cfg_nil_node)
                {
                    theme_cfgs[idx] = possible_theme_cfg;
                }
            }
            for(CFG_Node *child = parent_cfg->first; child != &cfg_nil_node; child = child->next)
            {
                if(str8_match(child->string, str8_lit("theme_color"), 0))
                {
                    cfg_node_ptr_list_push_front(scratch.arena, &colors_cfgs, child);
                }
            }
        }
        
        //- rjf: choose which theme cfg to use
        CFG_Node *theme_cfg = theme_cfgs[1];
        if(bp_setting_b32_from_name(str8_lit("use_project_theme")))
        {
            theme_cfg = theme_cfgs[0];
            if(theme_cfg == &cfg_nil_node)
            {
                theme_cfg = theme_cfgs[1];
            }
        }
        
        //- rjf: map the theme config to the associated tree (either from a preset, or from a file)
        MD_Node *theme_tree = bp_theme_tree_from_name(scratch.arena, access, theme_cfg->first->string);
        if(colors_cfgs.count == 0 && theme_tree == &md_nil_node)
        {
            theme_tree = bp_state->theme_preset_trees[BP_ThemePreset_DefaultDark];
        }
        
        //- rjf: build tasks for color applications - each task comprises of a metadesk
        // tree, describing the color patterns
        typedef struct Theme_Task Theme_Task;
        struct Theme_Task
        {
            Theme_Task *next;
            MD_Node *tree;
        };
        Theme_Task start_task = {0, theme_tree};
        Theme_Task *first_task = &start_task;
        Theme_Task *last_task = first_task;
        {
            for(CFG_Node_Ptr_Node *n = colors_cfgs.first; n != 0; n = n->next)
            {
                Theme_Task *t = push_array(scratch.arena, Theme_Task, 1);
                SLLQueuePushFront(first_task, last_task, t);
                t->tree = md_tree_from_string(scratch.arena, cfg_string_from_tree(scratch.arena, bp_state->cfg_schema_table, str8_zero(), n->v));
            }
        }
        
        //- rjf: apply theme tasks, build each color pattern for this window's
        // structured theme
        typedef struct Theme_Pattern_Node Theme_Pattern_Node;
        struct Theme_Pattern_Node
        {
            Theme_Pattern_Node *next;
            UI_ThemePattern pattern;
        };
        Theme_Pattern_Node *first_pattern = 0;
        Theme_Pattern_Node *last_pattern = 0;
        u64 pattern_count = 0;
        for(Theme_Task *t = first_task; t != 0; t = t->next)
        {
            MD_Node *tree_root = t->tree;
            for(MD_Node *n = tree_root; !md_node_is_nil(n); n = md_node_rec_depth_first_pre(n, tree_root).next)
            {
                if(str8_match(n->string, str8_lit("theme_color"), 0))
                {
                    MD_Node *tags_child = md_child_from_string(n, str8_lit("tags"), 0);
                    MD_Node *value_child = md_child_from_string(n, str8_lit("value"), 0);
                    u8 split_char = ' ';
                    String8_List tags = str8_split(scratch.arena, tags_child->first->string, &split_char, 1, 0);
                    u64 color_u64 = 0;
                    try_u64_from_str8_c_rules(value_child->first->string, &color_u64);
                    U32 color_u32 = (U32)color_u64;
                    Vec4f32 color_linear = linear_from_srgba(rgba_from_u32(color_u32));
                    Theme_Pattern_Node *node = push_array(scratch.arena, Theme_Pattern_Node, 1);
                    node->pattern.tags = str8_array_from_list(bp_frame_arena(), &tags);
                    node->pattern.linear = color_linear;
                    SLLQueuePush(first_pattern, last_pattern, node);
                    pattern_count += 1;
                }
            }
        }
        
        //- rjf: convert to final pattern array
        ws->theme = push_array(bp_frame_arena(), UI_Theme, 1);
        ws->theme->patterns_count = pattern_count;
        ws->theme->patterns = push_array(bp_frame_arena(), UI_ThemePattern, ws->theme->patterns_count);
        {
            u64 idx = 0;
            for(Theme_Pattern_Node *n = first_pattern; n != 0; n = n->next, idx += 1)
            {
                ws->theme->patterns[idx] = n->pattern;
            }
        }
        
        access_close(access);
    }
    
    //////////////////////////////
    //- rjf: @window_frame_part compute window's font raster flags
    //
    {
        ws->font_slot_raster_flags[BP_FontSlot_Icons] = FNT_RasterFlag_Smooth;
        ws->font_slot_raster_flags[BP_FontSlot_Main] = (bp_setting_b32_from_name(str8_lit("smooth_ui_text"))*FNT_RasterFlag_Smooth)|(bp_setting_b32_from_name(str8_lit("hint_ui_text"))*FNT_RasterFlag_Hinted);
        ws->font_slot_raster_flags[BP_FontSlot_Code] = (bp_setting_b32_from_name(str8_lit("smooth_code_text"))*FNT_RasterFlag_Smooth)|(bp_setting_b32_from_name(str8_lit("hint_code_text"))*FNT_RasterFlag_Hinted);
    }
    
    //////////////////////////////
    //- rjf: @window_frame_part pre-emptively rasterize common glyphs on the first frame
    //
    if(bp_state->first_window_state == ws && bp_state->last_window_state == ws && ws->frames_alive == 0)
    {
        f32 font_size = bp_font_size();
        BP_FontSlot english_font_slots[] = {BP_FontSlot_Main, BP_FontSlot_Code};
        BP_FontSlot icon_font_slot = BP_FontSlot_Icons;
        for(u64 idx = 0; idx < ArrayCount(english_font_slots); idx += 1)
        {
            Temp scratch = scratch_begin(0, 0);
            BP_FontSlot slot = english_font_slots[idx];
            String8 sample_text = str8_lit("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890~!@#$%^&*()-_+=[{]}\\|;:'\",<.>/?");
            fnt_run_from_string(bp_font_from_slot(slot),
                                font_size,
                                0, 0, 0,
                                sample_text);
            fnt_run_from_string(bp_font_from_slot(slot),
                                font_size,
                                0, 0, 0,
                                sample_text);
            scratch_end(scratch);
        }
        for(BP_IconKind icon_kind = BP_IconKind_Null; icon_kind < BP_IconKind_COUNT; icon_kind = (BP_IconKind)(icon_kind+1))
        {
            Temp scratch = scratch_begin(0, 0);
            fnt_run_from_string(bp_font_from_slot(icon_font_slot),
                                font_size,
                                0, 0, FNT_RasterFlag_Smooth,
                                bp_icon_kind_text_table[icon_kind]);
            fnt_run_from_string(bp_font_from_slot(icon_font_slot),
                                font_size,
                                0, 0, FNT_RasterFlag_Smooth,
                                bp_icon_kind_text_table[icon_kind]);
            fnt_run_from_string(bp_font_from_slot(icon_font_slot),
                                font_size,
                                0, 0, FNT_RasterFlag_Smooth,
                                bp_icon_kind_text_table[icon_kind]);
            scratch_end(scratch);
        }
    }
  
    //////////////////////////////
    //- rjf: @window_frame_part commit window's position/status to underlying cfg tree
    {
        Temp scratch = scratch_begin(0, 0);
        bool32 is_fullscreen = wm_window_is_fullscreen(ws->os);
        bool32 is_maximized = wm_window_is_maximized(ws->os);
        bool32 is_minimized = wm_window_is_minimized(ws->os);
        if(is_fullscreen)
        {
            cfg_node_child_from_string_or_alloc(bp_state->cfg, window, str8_lit("fullscreen"));
        }
        else
        {
            cfg_node_release(bp_state->cfg, cfg_node_child_from_string(window, str8_lit("fullscreen")));
        }
        if(is_maximized)
        {
            cfg_node_child_from_string_or_alloc(bp_state->cfg, window, str8_lit("maximized"));
        }
        else
        {
            cfg_node_release(bp_state->cfg, cfg_node_child_from_string(window, str8_lit("maximized")));
        }
    
        //- rjf: DPI changes -> xform font size / window size
        f32 dpi = wm_dpi_from_window(ws->os);
        if(dpi != ws->last_dpi)
        {
            fnt_reset();
            f32 current_font_size = bp_font_size();
            f32 new_font_size = current_font_size * (dpi / ws->last_dpi);
            new_font_size = Clamp(6.f, new_font_size, 72.f);
            CFG_Node *font_size_cfg = cfg_node_child_from_string_or_alloc(bp_state->cfg, window, str8_lit("font_size"));
            cfg_node_new_replacef(bp_state->cfg, font_size_cfg, "%I64u", (u64)new_font_size);
            ws->last_dpi = dpi;
        }
    
        //- rjf: commit position
        Rng2f32 window_rect = wm_rect_from_window(ws->os);
        if(!is_fullscreen && !is_maximized && !is_minimized)
        {
            Vec2f32 pos = window_rect.p0;
            CFG_Node *pos_root = cfg_node_child_from_string_or_alloc(bp_state->cfg, window, str8_lit("pos"));
            if((S32)pos.x != (S32)f64_from_str8(pos_root->first->string) ||
               (S32)pos.y != (S32)f64_from_str8(pos_root->last->string))
            {
                CFG_Node *x = pos_root->first;
                if(x == &cfg_nil_node)
                {
                    x= cfg_node_alloc(bp_state->cfg);
                    cfg_node_insert_child(bp_state->cfg, pos_root, &cfg_nil_node, x);
                }
                CFG_Node *y = x->next;
                if(y == &cfg_nil_node)
                {
                    y = cfg_node_alloc(bp_state->cfg);
                    cfg_node_insert_child(bp_state->cfg, pos_root, x, y);
                }
                cfg_node_equip_stringf(bp_state->cfg, x, "%i", (S32)pos.x);
                cfg_node_equip_stringf(bp_state->cfg, y, "%i", (S32)pos.y);
            }
        }
    
        //- rjf: commit size
        if(!is_fullscreen && !is_maximized && !is_minimized)
        {
            Vec2f32 size = dim_2f32(window_rect);
            CFG_Node *size_root = cfg_node_child_from_string_or_alloc(bp_state->cfg, window, str8_lit("size"));
            if((S32)size.x != (S32)f64_from_str8(size_root->first->string) ||
               (S32)size.y != (S32)f64_from_str8(size_root->last->string))
            {
                CFG_Node *width = size_root->first;
                if(width == &cfg_nil_node)
                {
                    width = cfg_node_alloc(bp_state->cfg);
                    cfg_node_insert_child(bp_state->cfg, size_root, &cfg_nil_node, width);
                }
                CFG_Node *height = width->next;
                if(height == &cfg_nil_node)
                {
                    height = cfg_node_alloc(bp_state->cfg);
                    cfg_node_insert_child(bp_state->cfg, size_root, width, height);
                }
                cfg_node_equip_stringf(bp_state->cfg, width, "%i", (S32)size.x);
                cfg_node_equip_stringf(bp_state->cfg, height, "%i", (S32)size.y);
            }
        }
    
        //- rjf: commit monitor
        if(!is_minimized)
        {
            WM_Monitor monitor = wm_monitor_from_window(ws->os);
            String8 monitor_name = wm_name_from_monitor(scratch.arena, monitor);
            CFG_Node *monitor_root = cfg_node_child_from_string_or_alloc(bp_state->cfg, window, str8_lit("monitor"));
            if(!str8_match(monitor_root->first->string, monitor_name, 0))
            {
                cfg_node_new_replace(bp_state->cfg, monitor_root, monitor_name);
            }
        }
        scratch_end(scratch);
    }
  
    //////////////////////////////
    //- rjf: @window_frame_part build UI
    //
    UI_Box *lister_box = &ui_nil_box;
    ProfScope("build UI")
    {
        ////////////////////////////
        //- rjf: @window_ui_part set up
        {
            f32 top_level_font_size = 0;
            BP_RegsScope(.view = 0, .tab = 0) top_level_font_size = bp_font_size();
      
            UI_IconInfo icon_info = {0};
            {
                icon_info.icon_font = bp_font_from_slot(BP_FontSlot_Icons);
                icon_info.icon_kind_text_map[UI_IconKind_RightArrow]     = bp_icon_kind_text_table[BP_IconKind_RightScroll];
                icon_info.icon_kind_text_map[UI_IconKind_DownArrow]      = bp_icon_kind_text_table[BP_IconKind_DownScroll];
                icon_info.icon_kind_text_map[UI_IconKind_LeftArrow]      = bp_icon_kind_text_table[BP_IconKind_LeftScroll];
                icon_info.icon_kind_text_map[UI_IconKind_UpArrow]        = bp_icon_kind_text_table[BP_IconKind_UpScroll];
                icon_info.icon_kind_text_map[UI_IconKind_RightCaret]     = bp_icon_kind_text_table[BP_IconKind_RightCaret];
                icon_info.icon_kind_text_map[UI_IconKind_DownCaret]      = bp_icon_kind_text_table[BP_IconKind_DownCaret];
                icon_info.icon_kind_text_map[UI_IconKind_LeftCaret]      = bp_icon_kind_text_table[BP_IconKind_LeftCaret];
                icon_info.icon_kind_text_map[UI_IconKind_UpCaret]        = bp_icon_kind_text_table[BP_IconKind_UpCaret];
                icon_info.icon_kind_text_map[UI_IconKind_CheckHollow]    = bp_icon_kind_text_table[BP_IconKind_CheckHollow];
                icon_info.icon_kind_text_map[UI_IconKind_CheckFilled]    = bp_icon_kind_text_table[BP_IconKind_CheckFilled];
            }
      
            UI_AnimationInfo animation_info = {0};
            {
                animation_info.hot_animation_rate      = bp_state->catchall_animation_rate;
                animation_info.active_animation_rate   = bp_state->catchall_animation_rate;
                animation_info.focus_animation_rate    = 1.f;
                animation_info.tooltip_animation_rate  = bp_state->tooltip_animation_rate;
                animation_info.menu_animation_rate     = bp_state->menu_animation_rate;
                animation_info.scroll_animation_rate   = bp_state->scrolling_animation_rate;
            }
      
            ui_begin_build(ws->os, &ws->ui_events, &icon_info, ws->theme, &animation_info, bp_state->frame_dt, bp_state->frame_dt);
            ui_push_font(bp_font_from_slot(BP_FontSlot_Main));
            ui_push_font_size(top_level_font_size);
            ui_push_text_padding(floor_f32(ui_top_font_size()*0.3f));
            ui_push_pref_width(ui_px(floor_f32(ui_top_font_size()*20.f), 1.f));
            ui_push_pref_height(ui_px(floor_f32(ui_top_font_size()*3.f), 1.f));
            ui_push_blur_size(10.f);
            FNT_RasterFlags text_raster_flags = 0;
            if(bp_setting_b32_from_name(str8_lit("smooth_ui_text"))) {text_raster_flags |= FNT_RasterFlag_Smooth;}
            if(bp_setting_b32_from_name(str8_lit("hint_ui_text"))) {text_raster_flags |= FNT_RasterFlag_Hinted;}
            ui_push_text_raster_flags(text_raster_flags);
        }
    
        ////////////////////////////
        //- rjf: @window_ui_part calculate code color slot RGBAs
        //
        for EachEnumVal(BP_CodeColorSlot, s)
        {
            ws->theme_code_colors[s] = ui_color_from_name(bp_code_color_slot_name_table[s]);
        }
    
        ////////////////////////////
        //- rjf: @window_ui_part calculate top-level rectangles/sizes
        //
        f32 window_edge_px = ui_top_font_size()*0.2f;
        Rng2f32 window_rect = wm_client_rect_from_window(ws->os);
        Vec2f32 window_rect_dim = dim_2f32(window_rect);
        f32 top_bar_dim_px = floor_f32(ui_top_font_size()*3.f);
        Rng2f32 top_bar_rect = r2f32p(window_rect.x0, window_rect.y0, window_rect.x0+window_rect_dim.x, window_rect.y0+top_bar_dim_px);
        Rng2f32 bottom_bar_rect = r2f32p(window_rect.x0, window_rect_dim.y - top_bar_dim_px, window_rect.x0+window_rect_dim.x, window_rect.y0+window_rect_dim.y);
        Rng2f32 content_rect = r2f32p(window_rect.x0, top_bar_rect.y1 + window_edge_px*1.5f, window_rect.x0+window_rect_dim.x, bottom_bar_rect.y0);
        content_rect = pad_2f32(content_rect, -window_edge_px);
    
        ////////////////////////////
        //- rjf: @window_ui_part truncated string hover
        //
        if(ui_string_hover_active()) UI_Tooltip
        {
            Temp scratch = scratch_begin(0, 0);
            DR_FStrList fstrs = ui_string_hover_fstrs(scratch.arena);
            UI_Box *box = ui_build_box_from_key(UI_BoxFlag_DrawText, ui_key_zero());
            ui_box_equip_display_fstrs(box, &fstrs);
            scratch_end(scratch);
        }
    
        ////////////////////////////
        //- rjf: @window_ui_part rich hover / drag/drop tooltips
        //
        if((bp_state->hover_regs_slot != BP_RegSlot_Null) || (bp_state->drag_drop_regs_slot != BP_RegSlot_Null && bp_drag_is_active()))
        {
            Temp scratch = scratch_begin(0, 0);
            BP_RegSlot slot = ((bp_state->drag_drop_regs_slot != BP_RegSlot_Null && bp_drag_is_active()) ? bp_state->drag_drop_regs_slot : bp_state->hover_regs_slot);
            BP_Regs *regs = (((bp_state->drag_drop_regs_slot != BP_RegSlot_Null && bp_drag_is_active()) ? bp_state->drag_drop_regs : bp_state->hover_regs));
            ui_state->tooltip_anchor_key = regs->ui_key;
            ui_state->tooltip_can_overflow_window = bp_drag_is_active();
            switch(slot)
            {
                default:{}break;
        
                    ////////////////////////
                    //- rjf: command tooltips
                    //
                case BP_RegSlot_CmdName:
                    UI_Tooltip
                    {
                        String8 cmd_name = regs->cmd_name;
                        DR_FStrList fstrs = bp_title_fstrs_from_code_name(scratch.arena, cmd_name);
                        UI_PrefWidth(ui_children_sum(1)) UI_Row UI_PrefWidth(ui_text_dim(5, 1))
                        {
                            UI_Box *box = ui_build_box_from_key(UI_BoxFlag_DrawText, ui_key_zero());
                            ui_box_equip_display_fstrs(box, &fstrs);
                            bp_cmd_binding_buttons(cmd_name, str8_zero(), 4, BP_CmdBindingButtonFlag_NoEdit);
                        }
                    }break;
        
                    ////////////////////////
                    //- rjf: file path tooltips
                    //
                case BP_RegSlot_FilePath:
                    UI_Tooltip
                    {
                        FileProperties props = properties_from_file_path(regs->file_path);
                        ui_set_next_pref_width(ui_children_sum(1));
                        UI_Row
                        {
                            BP_Font(BP_FontSlot_Icons) ui_label(bp_icon_kind_text_table[props.flags & FilePropertyFlag_IsFolder ? BP_IconKind_FolderClosedFilled : BP_IconKind_FileOutline]);
                            ui_label(regs->file_path);
                        }
                    }break;
        
                    ////////////////////////
                    //- rjf: cfg tooltips
                    //
                case BP_RegSlot_Cfg:
                    UI_Tooltip
                    {
                        CFG_Node *cfg = cfg_node_from_id(regs->cfg);
                        DR_FStrList fstrs = bp_title_fstrs_from_cfg(scratch.arena, cfg, 0);
                        UI_PrefWidth(ui_children_sum(1)) UI_Row UI_PrefWidth(ui_text_dim(5, 1))
                        {
                            UI_Box *box = ui_build_box_from_key(UI_BoxFlag_DrawText, ui_key_zero());
                            ui_box_equip_display_fstrs(box, &fstrs);
                        }
                    }break;
            }
            scratch_end(scratch);
        }
    
        ////////////////////////////
        //- rjf: @window_ui_part drag/drop visualization tooltips
        //
        if(bp_drag_is_active() && window_is_focused)
            BP_RegsScope(.window = bp_state->drag_drop_regs->window,
                         .panel = bp_state->drag_drop_regs->panel,
                         .tab = 0,
                         .view = bp_state->drag_drop_regs->view)
            {
                Temp scratch = scratch_begin(0, 0);
                CFG_Node *view = cfg_node_from_id(bp_state->drag_drop_regs->view);
                {
                    if(bp_state->drag_drop_regs_slot == BP_RegSlot_View && view != &cfg_nil_node)
                    {
                        CFG_Node *immediate_parent = &cfg_nil_node;
                        for(CFG_Node *p = view->parent; p != &cfg_nil_node; p = p->parent)
                        {
                            if(str8_match(p->parent->string, str8_lit("immediate"), 0))
                            {
                                immediate_parent = p->parent;
                                break;
                            }
                        }
                        if(immediate_parent != &cfg_nil_node)
                        {
                            cfg_node_child_from_string_or_alloc(bp_state->cfg, immediate_parent, str8_lit("hot"));
                        }
                        UI_Size main_width = ui_top_pref_width();
                        UI_Size main_height = ui_top_pref_height();
                        UI_TextAlign main_text_align = ui_top_text_alignment();
                        UI_Tooltip
                            UI_PrefWidth(main_width)
                            UI_PrefHeight(main_height)
                            UI_TextAlignment(main_text_align)
                        {
                            ui_state->tooltip_can_overflow_window = 1;
                            ui_set_next_pref_width(ui_em(60.f, 1.f));
                            ui_set_next_pref_height(ui_em(40.f, 1.f));
                            ui_set_next_child_layout_axis(Axis2_Y);
                            UI_Box *container = ui_build_box_from_key(0, ui_key_zero());
                            UI_Parent(container)
                            {
                                UI_Row UI_PrefWidth(ui_text_dim(10, 1))
                                {
                                    DR_FStrList fstrs = bp_title_fstrs_from_cfg(scratch.arena, view, 0);
                                    UI_Box *name_box = ui_build_box_from_key(UI_BoxFlag_DrawText, ui_key_zero());
                                    ui_box_equip_display_fstrs(name_box, &fstrs);
                                }
                                ui_set_next_pref_width(ui_pct(1, 0));
                                ui_set_next_pref_height(ui_pct(1, 0));
                                ui_set_next_child_layout_axis(Axis2_Y);
                                UI_Box *view_preview_container = ui_build_box_from_stringf(UI_BoxFlag_DrawBorder|UI_BoxFlag_DrawBackground|UI_BoxFlag_Clip, "###view_preview_container");
                                UI_Parent(view_preview_container) UI_Focus(UI_FocusKind_Off) UI_WidthFill
                                {
                                    bp_view_ui(view_preview_container->rect);
                                }
                            }
                        }
                    }
                }
                scratch_end(scratch);
            }
    
        ////////////////////////////
        //- rjf: @window_ui_part developer menu
        //
        if(ws->dev_menu_is_open) BP_Font(BP_FontSlot_Code)
        {
            ui_set_next_flags(UI_BoxFlag_ViewScrollY|UI_BoxFlag_AllowOverflowY|UI_BoxFlag_ViewClamp);
            UI_PaneF(r2f32p(30, 30, 30+ui_top_font_size()*100, ui_top_font_size()*60), "###dev_ctx_menu")
            {
                //- rjf: capture
                if(!ProfIsCapturing() && ui_clicked(ui_buttonf("Begin Profiler Capture###prof_cap")))
                {
                    ProfBeginCapture("app");
                }
                else if(ProfIsCapturing() && ui_clicked(ui_buttonf("End Profiler Capture###prof_cap")))
                {
                    ProfEndCapture();
                }
                
                //- rjf: toggles
                for(u64 idx = 0; idx < ArrayCount(DEV_toggle_table); idx += 1)
                {
                    if(ui_clicked(bp_icon_button(*DEV_toggle_table[idx].value_ptr ? BP_IconKind_CheckFilled : BP_IconKind_CheckHollow, 0, DEV_toggle_table[idx].name)))
                    {
                        *DEV_toggle_table[idx].value_ptr ^= 1;
                    }
                }
                
                ui_divider(ui_em(1.f, 1.f));
                
                //- rjf: draw registers
                ui_labelf("hover_reg_slot: %i", bp_state->hover_regs_slot);
                struct
                {
                    String8 name;
                    BP_Regs *regs;
                }
                    regs_info[] =
                    {
                        {str8_lit("regs"),       bp_regs()},
                        {str8_lit("hover_regs"), bp_state->hover_regs},
                    };
                for EachElement(idx, regs_info)
                {
                    ui_divider(ui_em(1.f, 1.f));
                    ui_label(regs_info[idx].name);
                    BP_Regs *regs = regs_info[idx].regs;
#define ID(name) ui_labelf("%s: $0x%I64x", #name, (regs->name))
                    ID(window);
                    ID(panel);
                    ID(view);
#undef ID
                    ui_labelf("file_path: \"%S\"", regs->file_path);
                    ui_labelf("expr: \"%S\"", regs->expr);
                    ui_labelf("cursor: %I64u", regs->cursor);
                    ui_labelf("mark: %I64u", regs->mark);
                    ui_labelf("line_num: %I64u", regs->line_num);
                    ui_labelf("column_num: %I64u", regs->column_num);
                    ui_labelf("text_key: [0x%I64x / 0x%I64x:0x%I64x]", regs->text_key.root.u64[0], regs->text_key.id.u128[0].u64[0], regs->text_key.id.u128[0].u64[1]);
                    ui_labelf("lang_kind: '%S'", txt_extension_from_lang_kind(regs->lang_kind));
                }
                
                ui_divider(ui_em(1.f, 1.f));
                
                //- rjf: draw per-window stats
                for(BP_Window_State *w = bp_state->first_window_state; w != &bp_nil_window_state; w = w->order_next)
                {
                    f64 avg_ui_hash_chain_length = 0;
                    {
                        f64 chain_count = 0;
                        f64 chain_length_sum = 0;
                        for(u64 idx = 0; idx < w->ui->box_table_size; idx += 1)
                        {
                            f64 chain_length = 0;
                            for(UI_Box *b = w->ui->box_table[idx].hash_first; !ui_box_is_nil(b); b = b->hash_next)
                            {
                                chain_length += 1;
                            }
                            if(chain_length > 0)
                            {
                                chain_length_sum += chain_length;
                                chain_count += 1;
                            }
                        }
                        avg_ui_hash_chain_length = chain_length_sum / chain_count;
                    }
                    ui_labelf("Target Hz: %.2f", 1.f/bp_state->frame_dt);
                    ui_labelf("Window %p", w);
                    ui_set_next_pref_width(ui_children_sum(1));
                    ui_set_next_pref_height(ui_children_sum(1));
                    UI_Row
                    {
                        ui_spacer(ui_em(2.f, 1.f));
                        ui_labelf("Box Count: %I64u", w->ui->last_build_box_count);
                    }
                    ui_set_next_pref_width(ui_children_sum(1));
                    ui_set_next_pref_height(ui_children_sum(1));
                    UI_Row
                    {
                        ui_spacer(ui_em(2.f, 1.f));
                        ui_labelf("Average UI Hash Chain Length: %f", avg_ui_hash_chain_length);
                    }
                }
                
                ui_divider(ui_em(1.f, 1.f));
            }
        }
        
        ////////////////////////////
        //- rjf: @window_ui_part drop-completion context menu
        //
        if(ws->top_drop_completion_task != 0)
        {
            BP_Drop_Completion_Task *task = ws->top_drop_completion_task;
            bool32 done = 0;
            UI_CtxMenu(bp_state->drop_completion_key) UI_PrefWidth(ui_em(40.f, 1.f)) UI_TagF("implicit")
            {
                // rjf: file names
                UI_TagF("weak") UI_Row UI_Padding(ui_em(1.25f, 1.f))
                {
                    String8_List strings = {0};
                    u64 idx = 0;
                    for(String8_Node *n = task->paths.first; n != 0 && idx < 20; n = n->next, idx += 1)
                    {
                        str8_list_push(scratch.arena, &strings, str8_skip_last_slash(n->string));
                        if(idx+1 == 20)
                        {
                            str8_list_push(scratch.arena, &strings, str8_lit("..."));
                        }
                    }
                    StringJoin join = {.sep = str8_lit(", ")};
                    String8 string = str8_list_join(scratch.arena, &strings, &join);
                    UI_PrefWidth(ui_pct(1, 0)) ui_label(string);
                }
        
                // rjf: option to open as project
                if(task->cfg)
                {
                    if(ui_clicked(bp_icon_buttonf(BP_IconKind_Briefcase, 0, "Open as project%s", (task->paths.node_count > 1) ? "s" : "")))
                    {
                        for(String8_Node *n = task->paths.first; n != 0; n = n->next)
                        {
                            bp_cmd(BP_CmdKind_OpenProject, .file_path = n->string);
                        }
                        done = 1;
                    }
                }
        
                // rjf: option to just open & view the file contents
                if(ui_clicked(bp_icon_buttonf(BP_IconKind_FileOutline, 0, "View file%s contents", (task->paths.node_count > 1) ? "s'" : "")))
                {
                    for(String8_Node *n = task->paths.first; n != 0; n = n->next)
                    {
                        bp_cmd(BP_CmdKind_Open, .file_path = n->string);
                    }
                    done = 1;
                }
            }
      
            if(done)
            {
                SLLStackPop(ws->top_drop_completion_task);
                if(ws->top_drop_completion_task == 0)
                {
                    ui_ctx_menu_close();
                }
            }
        }
    
        ////////////////////////////
        //- rjf: @window_ui_part popup
        //
        {
            if(bp_state->popup_t > 0.005f) UI_TextAlignment(UI_TextAlign_Center) UI_Focus(bp_state->popup_active ? UI_FocusKind_Root : UI_FocusKind_Off)
            {
                Vec2f32 window_dim = dim_2f32(window_rect);
                UI_Box *bg_box = &ui_nil_box;
                Vec4f32 shadow_color = ui_color_from_name(str8_lit("drop_shadow"));
                shadow_color.w += (1.f - shadow_color.w) * 0.5f;
                UI_Rect(window_rect)
                    UI_ChildLayoutAxis(Axis2_X)
                    UI_Focus(UI_FocusKind_On)
                    UI_Transparency(1-bp_state->popup_t)
                    UI_TagF("floating")
                    UI_BackgroundColor(shadow_color)
                {
                    bg_box = ui_build_box_from_stringf(UI_BoxFlag_FixedSize|
                                                       UI_BoxFlag_Floating|
                                                       UI_BoxFlag_Clickable|
                                                       UI_BoxFlag_Scroll|
                                                       UI_BoxFlag_DefaultFocusNav|
                                                       UI_BoxFlag_DisableFocusOverlay|
                                                       UI_BoxFlag_DisableFocusBorder|
                                                       UI_BoxFlag_DrawBackground, "###popup_%p", ws);
                }
                if(bp_state->popup_active) UI_Parent(bg_box) UI_Transparency(1-bp_state->popup_t)
                {
                    ui_ctx_menu_close();
                    UI_WidthFill UI_PrefHeight(ui_children_sum(1.f)) UI_Column UI_Padding(ui_pct(1, 0)) UI_TagF("floating")
                    {
                        ui_set_next_blur_size(10*bp_state->popup_t);
                        ui_set_next_pref_width(ui_children_sum(1));
                        ui_set_next_pref_height(ui_children_sum(1));
                        ui_set_next_child_layout_axis(Axis2_Y);
                        UI_Box *panel = ui_build_box_from_stringf(UI_BoxFlag_DrawBackground|UI_BoxFlag_DrawBackgroundBlur|UI_BoxFlag_DrawBorder|UI_BoxFlag_DrawDropShadow, "");
                        UI_Parent(panel)
                        {
                            ui_spacer(ui_em(1.5f, 1.f));
                            UI_TextRasterFlags(bp_raster_flags_from_slot(BP_FontSlot_Main)) UI_FontSize(ui_top_font_size()*2.f) UI_PrefHeight(ui_em(3.f, 1.f)) ui_label(bp_state->popup_title);
                            UI_PrefHeight(ui_em(3.f, 1.f)) UI_TagF("weak") ui_label(bp_state->popup_desc);
                            ui_spacer(ui_em(1.5f, 1.f));
                            UI_Row UI_Padding(ui_pct(1.f, 0.f)) UI_PrefWidth(ui_em(16.f, 1.f)) UI_PrefHeight(ui_em(3.5f, 1.f)) UI_CornerRadius(ui_top_font_size()*0.5f)
                            {
                                UI_TagF("pop")
                                    if(ui_clicked(ui_buttonf("OK")) || (ui_key_match(bg_box->default_nav_focus_hot_key, ui_key_zero()) && ui_slot_press(UI_EventActionSlot_Accept)))
                                    {
                                        bp_cmd(BP_CmdKind_PopupAccept);
                                    }
                                ui_spacer(ui_em(1.f, 1.f));
                                if(ui_clicked(ui_buttonf("Cancel")) || ui_slot_press(UI_EventActionSlot_Cancel))
                                {
                                    bp_cmd(BP_CmdKind_PopupCancel);
                                }
                            }
                            ui_spacer(ui_em(3.f, 1.f));
                        }
                    }
                }
                ui_signal_from_box(bg_box);
            }
        }
    
        ////////////////////////////
        //- rjf: @window_ui_part gather all tasks to build floating views
        //
        typedef struct FloatingViewTask FloatingViewTask;
        struct FloatingViewTask
        {
            FloatingViewTask *next;
            CFG_Node *view;
            BP_Regs *regs;
            Rng2f32 rect;
            bool32 is_focused;
            bool32 is_anchored;
            bool32 force_inside_window_x;
            bool32 force_inside_window_y;
            bool32 reset_open;
            UI_Signal signal; // NOTE(rjf): output, from build
            bool32 pressed;
            bool32 pressed_outside;
        };
        FloatingViewTask *query_floating_view_task = 0;
        FloatingViewTask *first_floating_view_task = 0;
        FloatingViewTask *last_floating_view_task = 0;
        BP_Font(BP_FontSlot_Code)
        {
            //- rjf: force-close query, if it's anchored, but box is gone
            if(query_is_open)
            {
                UI_Box *box = ui_box_from_key(ws->query_regs->ui_key);
                if(!ui_key_match(ui_key_zero(), ws->query_regs->ui_key) && ui_box_is_nil(box))
                {
                    query_is_open = 0;
                    bp_cmd(BP_CmdKind_CancelQuery);
                }
            }
      
            //- rjf: try to add opened query
            if(query_is_open)
            {
                // rjf: unpack view for query
                //
                CFG_Node *root = bp_immediate_cfg_from_keyf("window_query_%p", window);
                CFG_Node *view = cfg_node_child_from_string_or_alloc(bp_state->cfg, root, str8_lit("commands"));
                CFG_Node *query = cfg_node_child_from_string_or_alloc(bp_state->cfg, view, str8_lit("query"));
                bool32 is_lister = (cfg_node_child_from_string(view, str8_lit("lister")) != &cfg_nil_node);
                bool32 is_small = (cfg_node_child_from_string(view, str8_lit("small")) != &cfg_nil_node);
                BP_View_State *vs = bp_view_state_from_cfg(view);
        
                // rjf: did this view ID change? -> reset open animation
                bool32 reset_open = 0;
                if(view->id != ws->query_last_view_id)
                {
                    ws->query_last_view_id = view->id;
                    reset_open = 1;
                }
        
                // rjf: unpack query info
                String8 cmd_name = ws->query_regs->cmd_name;
                BP_CmdKindInfo *cmd_kind_info = bp_cmd_kind_info_from_string(cmd_name);
                String8 query_expr = ws->query_regs->expr;
                if(query_expr.size == 0 && cmd_name.size != 0)
                {
                    query_expr = cmd_kind_info->query.expr;
                }
                bool32 query_is_anchored = (!ui_box_is_nil(ui_box_from_key(ws->query_regs->ui_key)));
        
                // rjf: compute query expression
                if(query_expr.size == 0)
                {
                    query_expr = str8(vs->query_buffer, vs->query_string_size);
                }
                else
                {
                    u64 input_insertion_pos = str8_find_needle(query_expr, 0, str8_lit("$input"), 0);
                    if(input_insertion_pos < query_expr.size)
                    {
                        String8 pre_insertion  = str8_prefix(query_expr, input_insertion_pos);
                        String8 post_insertion = str8_skip(query_expr, input_insertion_pos + 6);
                        String8 input_text = str8(vs->query_buffer, vs->query_string_size);
                        String8 input_text__escaped = escaped_from_raw_str8(scratch.arena, input_text);
                        query_expr = push_str8f(scratch.arena, "%S%S%S", pre_insertion, input_text__escaped, post_insertion);
                    }
                }
        
                // rjf: store expression
                CFG_Node *expr = cfg_node_child_from_string_or_alloc(bp_state->cfg, view, str8_lit("expression"));
                cfg_node_new_replace(bp_state->cfg, expr, query_expr);
        
                // rjf: determine & store row-height setting
                if(ws->query_regs->do_big_rows)
                {
                    f32 row_height = 5.f;
                    CFG_Node *row_height_root = cfg_node_child_from_string_or_alloc(bp_state->cfg, view, str8_lit("row_height"));
                    cfg_node_new_replacef(bp_state->cfg, row_height_root, "%f", row_height);
                }
        
                // rjf: compute query view's top-level rectangle
                Rng2f32 rect = {0};
                {
                    Vec2f32 content_rect_center = center_2f32(content_rect);
                    Vec2f32 content_rect_dim = dim_2f32(content_rect);
                    f32 query_width_px = floor_f32(content_rect_dim.x * 0.35f);
                    f32 max_query_height_px = content_rect_dim.y*0.8f;
                    if(is_small)
                    {
                        query_width_px = floor_f32(content_rect_dim.x * 0.15f);
                        max_query_height_px = content_rect_dim.y*0.3f;
                    }
                    if(!ui_key_match(ui_key_zero(), ws->query_regs->ui_key))
                    {
                        query_width_px = is_small ? (ui_top_font_size()*40.f) : (ui_top_font_size()*60.f);
                        max_query_height_px = is_small ? (ui_top_font_size()*40.f) : (ui_top_font_size()*80.f);
                    }
                    f32 query_height_px = max_query_height_px;
                    rect = r2f32p(content_rect_center.x - query_width_px/2,
                                  content_rect_center.y - max_query_height_px/2.f,
                                  content_rect_center.x + query_width_px/2,
                                  content_rect_center.y - max_query_height_px/2.f + query_height_px);
                    if(!ui_key_match(ui_key_zero(), ws->query_regs->ui_key))
                    {
                        UI_Box *anchor_box = ui_box_from_key(ws->query_regs->ui_key);
                        if(anchor_box != &ui_nil_box)
                        {
                            rect.x0 = anchor_box->rect.x0 + ws->query_regs->off_px.x;
                            rect.y0 = anchor_box->rect.y1 + ws->query_regs->off_px.y;
                            rect.x1 = rect.x0 + query_width_px;
                            rect.y1 = rect.y0 + query_height_px;
                        }
                    }
                }
        
                // rjf: push query task
                {
                    FloatingViewTask *t = push_array(scratch.arena, FloatingViewTask, 1);
                    SLLQueuePush(first_floating_view_task, last_floating_view_task, t);
                    query_floating_view_task = t;
                    t->view          = view;
                    t->regs          = ws->query_regs;
                    t->rect          = rect;
                    t->is_focused    = 1;
                    t->is_anchored   = query_is_anchored;
                    t->reset_open    = reset_open;
                    t->force_inside_window_x = 1;
                    t->force_inside_window_y = 1;
                }
            }
        }
    
        ////////////////////////////
        //- rjf: @window_ui_part build all floating views
        //
        ProfScope("build all floating views")
            BP_Font(BP_FontSlot_Code)
            UI_TagF("floating")
            UI_Focus(ui_any_ctx_menu_is_open() || ws->menu_bar_focused ? UI_FocusKind_Off : UI_FocusKind_Null)
        {
            f32 fast_open_rate = bp_state->menu_animation_rate;
            f32 slow_open_rate = bp_state->menu_animation_rate__slow;
            for(FloatingViewTask *t = first_floating_view_task; t != 0; t = t->next)
            {
                CFG_Node *view    = t->view;    
                Rng2f32 rect      = t->rect;
                bool32 is_focused    = t->is_focused;
                bool32 is_anchored   = t->is_anchored;
                bool32 is_lister = (cfg_node_child_from_string(view, str8_lit("lister")) != &cfg_nil_node);
                f32 open_t        = ui_anim(ui_key_from_stringf(ui_key_zero(), "floating_view_open_%p", view), 1.f,
                                            .rate = is_anchored ? fast_open_rate : slow_open_rate,
                                            .reset = t->reset_open,
                                            .initial = 0.f);
        
                if(t->force_inside_window_x || t->force_inside_window_y)
                {
                    bool32 axis_mask[] = {t->force_inside_window_x, t->force_inside_window_y};
                    Rng2f32 window_rect = wm_client_rect_from_window(ws->os);
                    for EachEnumVal(Axis2, axis)
                                   {
                                       if(!axis_mask[axis]) { continue; }
                                       f32 max_delta = rect.p1.v[axis] - window_rect.p1.v[axis];
                                       f32 min_delta = window_rect.p0.v[axis] - rect.p0.v[axis];
                                       f32 total_delta = Max(min_delta, 0) - Max(max_delta, 0);
                                       rect.p0.v[axis] += total_delta;
                                       rect.p1.v[axis] += total_delta;
                                   }
                }
        
                bp_push_regs();
                {
                    if(t->regs != 0)
                    {
                        bp_regs()->cfg = t->regs->cfg;
                    }
                    bp_regs()->view = view->id;
                    String8 view_file_path = bp_path_from_cfg(view);
                    if(view_file_path.size != 0)
                    {
                        bp_regs()->file_path = view_file_path;
                    }
                }
        
                UI_Focus(is_focused ? UI_FocusKind_On : UI_FocusKind_Off)
                    UI_PermissionFlags(UI_PermissionFlag_All)
                {
                    bool32 has_footer = (is_lister && !is_anchored);
                    f32 container_corner_radius_px = has_footer ? ui_top_font_size()*1.f : ui_top_font_size()*0.15f;
          
                    UI_Box *container = &ui_nil_box;
                    UI_Rect(rect) UI_ChildLayoutAxis(Axis2_Y)
                        UI_Squish(0.1f-0.1f*open_t)
                        UI_Transparency(1.f-open_t)
                        UI_CornerRadius(container_corner_radius_px)
                    {
                        container = ui_build_box_from_stringf(UI_BoxFlag_Clickable|
                                                              UI_BoxFlag_DrawBorder|
                                                              UI_BoxFlag_DrawBackground|
                                                              UI_BoxFlag_DrawBackgroundBlur|
                                                              UI_BoxFlag_RoundChildrenByParent|
                                                              UI_BoxFlag_DisableFocusOverlay|
                                                              UI_BoxFlag_DrawDropShadow|
                                                              (UI_BoxFlag_SquishAnchored*!!is_anchored),
                                                              "floating_view_container_%p", view);
                    }
          
                    {
                        for(UI_Event *evt = 0; ui_next_event(&evt);)
                        {
                            if(evt->kind == UI_EventKind_Press &&
                               evt->key == WM_Key_LeftMouseButton)
                            {
                                if(contains_2f32(container->rect, evt->pos))
                                {
                                    t->pressed = 1;
                                }
                                else
                                {
                                    t->pressed_outside = 1;
                                }
                            }
                        }
                    }
          
                    UI_Box *loading_overlay_container = &ui_nil_box;
                    UI_Parent(container) UI_WidthFill UI_HeightFill
                    {
                        loading_overlay_container = ui_build_box_from_key(UI_BoxFlag_Floating, ui_key_zero());
                    }
          
                    f32 total_height_px = dim_2f32(rect).y;
                    f32 footer_height_px = 0;
                    if(has_footer)
                    {
                        footer_height_px = ui_top_font_size()*4.f;
                    }
                    UI_Parent(container) UI_Focus(is_focused ? UI_FocusKind_Null : UI_FocusKind_Off)
                    {
                        ui_set_next_pref_width(ui_pct(1, 0));
                        ui_set_next_pref_height(ui_px(total_height_px - footer_height_px, 1));
                        ui_set_next_child_layout_axis(Axis2_Y);
                        UI_Box *view_contents_container = ui_build_box_from_stringf(UI_BoxFlag_Clip, "###view_contents_container");
                        UI_Parent(view_contents_container) UI_WidthFill
                        {
                            bp_view_ui(rect);
                        }
                    }
          
                    if(is_lister && !is_anchored) UI_Parent(container) UI_WidthFill UI_HeightFill UI_Focus(UI_FocusKind_Off)
                    {
                        ui_set_next_flags(UI_BoxFlag_DrawBackground|UI_BoxFlag_DrawBorder);
                        UI_Column UI_Padding(ui_em(1, 1))
                        {
                            UI_Row UI_Padding(ui_pct(1, 0)) BP_Font(BP_FontSlot_Main) UI_FontSize(ui_top_font_size()*0.8f) UI_TagF("alt")
                                UI_PrefWidth(ui_text_dim(1, 1)) UI_TextAlignment(UI_TextAlign_Center)
                            {
                                bp_cmd_binding_buttons(bp_cmd_kind_info_table[BP_CmdKind_MoveUp].string, s(""), 1, BP_CmdBindingButtonFlag_NoEdit);
                                ui_spacer(ui_em(1.f, 1.f));
                                ui_label(s("/"));
                                bp_cmd_binding_buttons(bp_cmd_kind_info_table[BP_CmdKind_MoveDown].string, s(""), 1, BP_CmdBindingButtonFlag_NoEdit);
                                ui_label(s("to navigate"));
                                
                                ui_spacer(ui_em(1, 1));
                                
                                bp_cmd_binding_buttons(bp_cmd_kind_info_table[BP_CmdKind_Accept].string, s(""), 1, BP_CmdBindingButtonFlag_NoEdit);
                                ui_label(s("to use"));
                                
                                ui_spacer(ui_em(1, 1));
                                
                                bp_cmd_binding_buttons(bp_cmd_kind_info_table[BP_CmdKind_Cancel].string, s(""), 1, BP_CmdBindingButtonFlag_NoEdit);
                                ui_label(s("to cancel"));
                            }
                        }
                    }
                    
                    {
                        BP_View_State *vs = bp_view_state_from_cfg(view);
                        f32 loading_t = vs->loading_t;
                        if(loading_t > 0.01f) UI_Parent(loading_overlay_container)
                        {
                            bp_loading_overlay(rect, loading_t, vs->loading_progress_v, vs->loading_progress_v_target);
                        }
                    }
                    
                    UI_Signal sig = ui_signal_from_box(container);
                    t->signal = sig;
                }
        
                BP_Regs *view_regs = bp_pop_regs();
                if(is_focused)
                {
                    MemoryCopyStruct(bp_regs(), view_regs);
                }
        
                if(!is_anchored)
                {
                    UI_TagF("inactive") UI_Transparency(1-open_t) UI_Rect(content_rect) ui_build_box_from_key(UI_BoxFlag_DrawBackground|UI_BoxFlag_Floating, ui_key_zero());
                }
        
                //- rjf: query interactions
                if(t == query_floating_view_task)
                {
                    CFG_Node *view = query_floating_view_task->view;
                    BP_View_State *vs = bp_view_state_from_cfg(query_floating_view_task->view);
                    String8 cmd_name = ws->query_regs->cmd_name;
                    BP_CmdKindInfo *cmd_kind_info = bp_cmd_kind_info_from_string(cmd_name);
          
                    if(query_floating_view_task->pressed_outside ||
                       (cfg_node_child_from_string(view, str8_lit("lister")) != &cfg_nil_node && !vs->query_is_open) ||
                       (cmd_name.size != 0 && !vs->query_is_open) ||
                       ui_slot_press(UI_EventActionSlot_Cancel))
                    {
                        bp_cmd(BP_CmdKind_CancelQuery);
                    }
          
                    if(cmd_kind_info->query.slot == BP_RegSlot_FilePath)
                    {
                        CFG_Node *query = cfg_node_child_from_string(view, str8_lit("query"));
                        CFG_Node *input = cfg_node_child_from_string(query, str8_lit("input"));
                        if(input != &cfg_nil_node)
                        {
                            String8 path_chopped = str8_chop_last_slash(input->first->string);
                            CFG_Node *user = cfg_node_child_from_string(cfg_node_root(), str8_lit("user"));
                            CFG_Node *current_path = cfg_node_child_from_string_or_alloc(bp_state->cfg, user, str8_lit("current_path"));
                            if(!str8_match(current_path->first->string, path_chopped, 0))
                            {
                                bp_cmd(BP_CmdKind_SetCurrentPath, .file_path = path_chopped);
                            }
                        }
                    }
                }
            }
        }
    
        ////////////////////////////
        //- rjf: @window_ui_part top bar
        //
        ProfScope("build top bar")
        {
            wm_window_clear_custom_border_data(ws->os);
            wm_window_push_custom_edges(ws->os, window_edge_px);
            wm_window_push_custom_title_bar(ws->os, dim_2f32(top_bar_rect).y);
            ui_set_next_flags(UI_BoxFlag_DefaultFocusNav|UI_BoxFlag_DisableFocusOverlay|UI_BoxFlag_DrawDropShadow);
            UI_Focus((ws->menu_bar_focused && window_is_focused && !ui_any_ctx_menu_is_open()) ? UI_FocusKind_On : UI_FocusKind_Null)
                UI_TagF("menu_bar")
                UI_Pane(top_bar_rect, str8_lit("###top_bar"))
                UI_WidthFill UI_Row
                UI_Focus(UI_FocusKind_Null)
            {
                UI_Key menu_bar_group_key = ui_key_from_string(ui_key_zero(), str8_lit("###top_bar_group"));
                MemoryZeroArray(ui_top_parent()->parent->corner_radii);
        
                //- rjf: left column
                {
                    ui_set_next_flags(UI_BoxFlag_Clip|UI_BoxFlag_ViewScrollX|UI_BoxFlag_ViewClamp);
                    UI_WidthFill UI_NamedRow(str8_lit("###menu_bar"))
                    {
                        //- rjf: icon
                        UI_Padding(ui_em(0.5f, 1.f))
                        {
                            UI_PrefWidth(ui_px(dim_2f32(top_bar_rect).y - ui_top_font_size()*0.8f, 1.f))
                                UI_Column
                                UI_Padding(ui_em(0.4f, 1.f))
                                UI_HeightFill
                                {
                                    R_Handle texture = bp_state->icon_texture;
                                    Vec2S32 texture_dim = r_size_from_tex2d(texture);
                                    ui_image(texture, R_Tex2DSampleKind_Linear, r2f32p(0, 0, texture_dim.x, texture_dim.y), v4f32(1, 1, 1, 1), 0, str8_lit(""));
                                }
                        }
            
                        //- rjf: menu items
                        //
                        if(dim_2f32(top_bar_rect).x > ui_top_font_size()*60)
                        {
                            ui_set_next_flags(UI_BoxFlag_DrawBackground);
                            UI_PrefWidth(ui_children_sum(1)) UI_Row UI_PrefWidth(ui_text_dim(20, 1)) UI_GroupKey(menu_bar_group_key)
                            {
                                // rjf: file menu
                                UI_Key file_menu_key = ui_key_from_string(ui_key_zero(), str8_lit("_file_menu_key_"));
                                UI_CtxMenu(file_menu_key) UI_PrefWidth(ui_em(50.f, 1.f)) UI_TagF("implicit")
                                {
                                    String8 cmds[] = {
                                        bp_cmd_kind_info_table[BP_CmdKind_Open].string,
                                        {0},//-
                                        bp_cmd_kind_info_table[BP_CmdKind_NewProject].string,
                                        bp_cmd_kind_info_table[BP_CmdKind_OpenProject].string,
                                        bp_cmd_kind_info_table[BP_CmdKind_OpenRecentProject].string,
                                        bp_cmd_kind_info_table[BP_CmdKind_SaveProject].string,
                                        bp_cmd_kind_info_table[BP_CmdKind_ProjectSettings].string,
                                        {0},//-
                                        bp_cmd_kind_info_table[BP_CmdKind_NewUser].string,
                                        bp_cmd_kind_info_table[BP_CmdKind_OpenUser].string,
                                        bp_cmd_kind_info_table[BP_CmdKind_SaveUser].string,
                                        bp_cmd_kind_info_table[BP_CmdKind_UserSettings].string,
                                        {0},//-
                                        bp_cmd_kind_info_table[BP_CmdKind_Exit].string,
                                    };
                                    U32 codepoints[] = {
                                        'o',
                                        0,//-
                                        'j', 'p', 'r', 'a', 't',
                                        0,//-
                                        'w', 'u', 's', 'e',
                                        0,//-
                                        'x',
                                    };
                                    Assert(ArrayCount(codepoints) == ArrayCount(cmds));
                                    bp_cmd_list_menu_buttons(ArrayCount(cmds), cmds, codepoints);
                                }
                
                                // rjf: window menu
                                UI_Key window_menu_key = ui_key_from_string(ui_key_zero(), str8_lit("_window_menu_key_"));
                                UI_CtxMenu(window_menu_key) UI_PrefWidth(ui_em(50.f, 1.f)) UI_TagF("implicit")
                                {
                                    String8 cmds[] = {
                                        bp_cmd_kind_info_table[BP_CmdKind_OpenWindow].string,
                                        bp_cmd_kind_info_table[BP_CmdKind_CloseWindow].string,
                                        bp_cmd_kind_info_table[BP_CmdKind_ToggleFullscreen].string,
                                        {0},//-
                                        bp_cmd_kind_info_table[BP_CmdKind_WindowSettings].string,
                                    };
                                    U32 codepoints[] = {'w', 'c', 'f', 0, 's'};
                                    Assert(ArrayCount(codepoints) == ArrayCount(cmds));
                                    bp_cmd_list_menu_buttons(ArrayCount(cmds), cmds, codepoints);
                                }
                
                                // rjf: panel menu
                                UI_Key panel_menu_key = ui_key_from_string(ui_key_zero(), str8_lit("_panel_menu_key_"));
                                UI_CtxMenu(panel_menu_key) UI_PrefWidth(ui_em(50.f, 1.f)) UI_TagF("implicit")
                                {
                                    String8 cmds[] = {
                                        bp_cmd_kind_info_table[BP_CmdKind_NewPanelUp].string,
                                        bp_cmd_kind_info_table[BP_CmdKind_NewPanelDown].string,
                                        bp_cmd_kind_info_table[BP_CmdKind_NewPanelRight].string,
                                        bp_cmd_kind_info_table[BP_CmdKind_NewPanelLeft].string,
                                        bp_cmd_kind_info_table[BP_CmdKind_ClosePanel].string,
                                        {0},//-
                                        bp_cmd_kind_info_table[BP_CmdKind_NextPanel].string,
                                        bp_cmd_kind_info_table[BP_CmdKind_PrevPanel].string,
                                        {0},//-
                                        bp_cmd_kind_info_table[BP_CmdKind_TabBarTop].string,
                                        bp_cmd_kind_info_table[BP_CmdKind_TabBarBottom].string,
                                        {0},//-
                                        bp_cmd_kind_info_table[BP_CmdKind_ResetToDefaultPanels].string,
                                        bp_cmd_kind_info_table[BP_CmdKind_ResetToCompactPanels].string,
                                        bp_cmd_kind_info_table[BP_CmdKind_ResetToSimplePanels].string,
                                    };
                                    U32 codepoints[] = {'u','d','r','l','o', 0, 'n','p', 0, 0,0, 0, 0,0,0};
                                    Assert(ArrayCount(codepoints) == ArrayCount(cmds));
                                    bp_cmd_list_menu_buttons(ArrayCount(cmds), cmds, codepoints);
                                }
                
                                // rjf: tab menu
                                UI_Key tab_menu_key = ui_key_from_string(ui_key_zero(), str8_lit("_tab_menu_key_"));
                                UI_CtxMenu(tab_menu_key) UI_PrefWidth(ui_em(50.f, 1.f)) UI_TagF("implicit")
                                {
                                    String8 cmds[] = {
                                        bp_cmd_kind_info_table[BP_CmdKind_CloseTab].string,
                                        bp_cmd_kind_info_table[BP_CmdKind_DuplicateTab].string,
                                        {0},//-
                                        bp_cmd_kind_info_table[BP_CmdKind_MoveTabLeft].string,
                                        bp_cmd_kind_info_table[BP_CmdKind_MoveTabRight].string,
                                        {0},//-
                                        bp_cmd_kind_info_table[BP_CmdKind_NextTab].string,
                                        bp_cmd_kind_info_table[BP_CmdKind_PrevTab].string,
                                        {0},//-
                                        bp_cmd_kind_info_table[BP_CmdKind_TabSettings].string,
                                    };
                                    U32 codepoints[] = {'c','d', 0, 'l','r', 0, 'n','p', 0, 's'};
                                    Assert(ArrayCount(codepoints) == ArrayCount(cmds));
                                    bp_cmd_list_menu_buttons(ArrayCount(cmds), cmds, codepoints);
                                }
                
                                // rjf: help menu
                                UI_Key help_menu_key = ui_key_from_string(ui_key_zero(), str8_lit("_help_menu_key_"));
                                UI_CtxMenu(help_menu_key) UI_PrefWidth(ui_em(50.f, 1.f)) UI_TagF("implicit")
                                {
                                    UI_Row UI_TextAlignment(UI_TextAlign_Center) UI_TagF("weak")
                                        ui_label(str8_lit(BUILD_TITLE));
                                    ui_spacer(ui_em(1.f, 1.f));
                                    UI_PrefHeight(ui_children_sum(1)) UI_Row UI_Padding(ui_pct(1, 0))
                                    {
                                        R_Handle texture = bp_state->icon_texture;
                                        Vec2S32 texture_dim = r_size_from_tex2d(texture);
                                        UI_PrefWidth(ui_px(ui_top_font_size()*10.f, 1.f))
                                            UI_PrefHeight(ui_px(ui_top_font_size()*10.f, 1.f))
                                            ui_image(texture, R_Tex2DSampleKind_Linear, r2f32p(0, 0, texture_dim.x, texture_dim.y), v4f32(1, 1, 1, 1), 0, str8_lit(""));
                                    }
                                    ui_spacer(ui_em(1.f, 1.f));
                                    CFG_KeyMapNode_Ptr_List key_map_nodes = cfg_key_map_node_ptr_list_from_name(scratch.arena, bp_state->key_map, bp_cmd_kind_info_table[BP_CmdKind_OpenPalette].string);
                                    CFG_Binding binding = {0};
                                    String8 binding_str = {0};
                                    if(key_map_nodes.first != 0)
                                    {
                                        binding = key_map_nodes.first->v->binding;
                                        binding_str = wm_string_from_modifiers_key(scratch.arena, binding.modifiers, binding.key);
                                    }
                                    UI_TagF(".")
                                        UI_Row
                                        UI_TextAlignment(UI_TextAlign_Center)
                                        UI_Padding(ui_pct(1, 0))
                                    {
                                        UI_Signal sig = {0};
                                        UI_PrefWidth(ui_children_sum(1))
                                            UI_Flags(UI_BoxFlag_DrawBorder)
                                            UI_CornerRadius(ui_top_font_size()*0.5f)
                                            sig = ui_buttonf("###open_palette");
                                        UI_Parent(sig.box) UI_PrefWidth(ui_text_dim(ui_top_font_size()*2.f, 1))
                                        {
                                            ui_labelf("Search for commands & settings");
                                            if(binding_str.size != 0)
                                            {
                                                UI_TagF("weak") ui_labelf("(%S)", binding_str);
                                            }
                                        }
                                        if(ui_clicked(sig))
                                        {
                                            bp_cmd(BP_CmdKind_RunCommand, .cmd_name = bp_cmd_kind_info_table[BP_CmdKind_OpenPalette].string);
                                        }
                                    }
                                    ui_spacer(ui_em(1.f, 1.f));
                                }
                
                                // rjf: buttons
                                UI_TextAlignment(UI_TextAlign_Center) UI_HeightFill
                                {
                                    struct
                                    {
                                        String8 name;
                                        U32 codepoint;
                                        WM_Key key;
                                        UI_Key menu_key;
                                    } items[] = {
                                        {str8_lit("File"),     'f', WM_Key_F, file_menu_key},
                                        {str8_lit("Window"),   'w', WM_Key_W, window_menu_key},
                                        {str8_lit("Panel"),    'p', WM_Key_P, panel_menu_key},
                                        {str8_lit("Tab"),      'b', WM_Key_V, tab_menu_key},
                                        {str8_lit("Help"),     'h', WM_Key_H, help_menu_key},
                                    };
                  
                                    bool32 menu_open = 0;
                                    u64 open_menu_idx = 0;
                                    for(u64 idx = 0; idx < ArrayCount(items); idx += 1)
                                    {
                                        if(ui_ctx_menu_is_open(items[idx].menu_key))
                                        {
                                            menu_open = 1;
                                            open_menu_idx = idx;
                                            break;
                                        }
                                    }
                  
                                    u64 open_menu_idx_prime = open_menu_idx;
                                    if(menu_open && ws->menu_bar_focused && window_is_focused)
                                    {
                                        for(UI_Event *evt = 0; ui_next_event(&evt);)
                                        {
                                            bool32 taken = 0;
                                            if(evt->delta_2s32.x > 0)
                                            {
                                                taken = 1;
                                                open_menu_idx_prime += 1;
                                                open_menu_idx_prime = open_menu_idx_prime%ArrayCount(items);
                                            }
                                            if(evt->delta_2s32.x < 0)
                                            {
                                                taken = 1;
                                                open_menu_idx_prime = open_menu_idx_prime > 0 ? open_menu_idx_prime-1 : (ArrayCount(items)-1);
                                            }
                                            if(taken)
                                            {
                                                ui_eat_event(evt);
                                            }
                                        }
                                    }
                  
                                    UI_TagF("implicit")
                                        UI_VisualMargin(ui_top_font_size()*0.45f)
                                        UI_CornerRadius(ui_top_font_size()*0.5f)
                                        for(u64 idx = 0; idx < ArrayCount(items); idx += 1)
                                        {
                                            ui_set_next_fastpath_codepoint(items[idx].codepoint);
                                            bool32 alt_fastpath_key = 0;
                                            if(bp_setting_b32_from_name(str8_lit("focus_menu_bar_with_alt")) && ui_key_press(WM_Modifier_Alt, items[idx].key))
                                            {
                                                alt_fastpath_key = 1;
                                            }
                                            if((ws->menu_bar_key_held || ws->menu_bar_focused) && !ui_any_ctx_menu_is_open())
                                            {
                                                ui_set_next_flags(UI_BoxFlag_DrawTextFastpathCodepoint);
                                            }
                                            UI_TagF(!ui_ctx_menu_is_open(items[idx].menu_key) ? "weak" : "")
                                            {
                                                UI_Signal sig = bp_menu_bar_button(items[idx].name);
                                                wm_window_push_custom_title_bar_client_area(ws->os, sig.box->rect);
                                                if(menu_open)
                                                {
                                                    if((ui_hovering(sig) && !ui_ctx_menu_is_open(items[idx].menu_key)) || (open_menu_idx_prime == idx && open_menu_idx_prime != open_menu_idx))
                                                    {
                                                        ui_ctx_menu_open(items[idx].menu_key, sig.box->key, v2f32(0, sig.box->rect.y1-sig.box->rect.y0));
                                                    }
                                                }
                                                else if(ui_pressed(sig) || alt_fastpath_key)
                                                {
                                                    if(ui_ctx_menu_is_open(items[idx].menu_key))
                                                    {
                                                        ui_ctx_menu_close();
                                                    }
                                                    else
                                                    {
                                                        ui_ctx_menu_open(items[idx].menu_key, sig.box->key, v2f32(0, sig.box->rect.y1-sig.box->rect.y0));
                                                    }
                                                }
                                            }
                                        }
                                }
                            }
                        }
                    }
                }
        
                //- rjf: right column
                //
                UI_WidthFill UI_Row
                {
                    bool32 do_user_prof = (dim_2f32(top_bar_rect).x > ui_top_font_size()*80);
          
                    ui_spacer(ui_pct(1, 0));
          
                    if(do_user_prof)
                        UI_VisualMargin(ui_top_font_size()*0.5f)
                            UI_CornerRadius(ui_top_font_size()*0.5f)
                        {
                            ui_set_next_pref_width(ui_children_sum(1));
                            ui_set_next_child_layout_axis(Axis2_X);
                            UI_Box *prof_box = ui_build_box_from_stringf(UI_BoxFlag_Clickable|
                                                                         UI_BoxFlag_DrawBackground|
                                                                         UI_BoxFlag_DrawHotEffects|
                                                                         UI_BoxFlag_DrawActiveEffects,
                                                                         "###loaded_project_button");
                            wm_window_push_custom_title_bar_client_area(ws->os, prof_box->rect);
                            UI_Parent(prof_box) UI_PrefWidth(ui_text_dim(10, 0)) UI_TextAlignment(UI_TextAlign_Center) UI_Padding(ui_em(0.5f, 1.f))
                            {
                                CFG_Node *root = cfg_node_root();
                                CFG_Node *project = cfg_node_child_from_string(root, str8_lit("project"));
                                CFG_Node *name = cfg_node_child_from_string(project, str8_lit("name"));
                                String8 project_name = name->first->string;
                                if(project_name.size == 0)
                                {
                                    String8 prof_path = bp_state->project_path;
                                    prof_path = str8_chop_last_dot(prof_path);
                                    project_name = str8_skip_last_slash(prof_path);
                                }
                                if(project_name.size == 0)
                                {
                                    project_name = str8_lit("Untitled Project");
                                }
                                BP_Font(BP_FontSlot_Icons)
                                    ui_label(bp_icon_kind_text_table[BP_IconKind_Briefcase]);
                                ui_label(project_name);
                            }
                            UI_Signal prof_sig = ui_signal_from_box(prof_box);
                            if(ui_clicked(prof_sig))
                            {
                                bp_cmd(BP_CmdKind_RunCommand, .cmd_name = bp_cmd_kind_info_table[BP_CmdKind_OpenProject].string);
                            }
                        }
          
                    // rjf: close dropdown
                    UI_Key close_ctx_menu_key = ui_key_from_stringf(ui_key_zero(), "###close_ctx_menu");
                    UI_CtxMenu(close_ctx_menu_key) UI_TagF("implicit")
                    {
                        if(ui_clicked(bp_icon_buttonf(BP_IconKind_Window, 0, "Close Window")))
                        {
                            bp_cmd(BP_CmdKind_CloseWindow);
                        }
                        if(ui_clicked(bp_icon_buttonf(BP_IconKind_X, 0, "Exit")))
                        {
                            bp_cmd(BP_CmdKind_Exit);
                        }
                    }
          
                    // rjf: min/max/close buttons
                    UI_TagF("implicit")
                        UI_TagF("weak")
                        UI_VisualMargin(ui_top_font_size()*0.5f)
                        UI_CornerRadius(ui_top_font_size()*0.9f)
                    {
                        UI_Signal min_sig = {0};
                        UI_Signal max_sig = {0};
                        UI_Signal cls_sig = {0};
                        Vec2f32 bar_dim = dim_2f32(top_bar_rect);
                        f32 button_dim = floor_f32(bar_dim.y);
                        UI_PrefWidth(ui_px(button_dim, 1.f))
                            UI_FontSize(ui_top_font_size()*0.75f)
                        {
                            min_sig = bp_icon_buttonf(BP_IconKind_WindowMinimize,  0, "##minimize");
                            max_sig = bp_icon_buttonf(wm_window_is_maximized(ws->os) ? BP_IconKind_WindowRestore : BP_IconKind_Window, 0, "##maximize");
                        }
                        UI_PrefWidth(ui_px(button_dim, 1.f))
                            UI_FontSize(ui_top_font_size()*0.85f)
                        {
                            cls_sig = bp_icon_buttonf(BP_IconKind_X, 0, "##close");
                        }
                        if(ui_clicked(min_sig))
                        {
                            wm_window_set_minimized(ws->os, 1);
                        }
                        if(ui_clicked(max_sig))
                        {
                            wm_window_set_maximized(ws->os, !wm_window_is_maximized(ws->os));
                        }
                        if(ui_clicked(cls_sig))
                        {
                            if(ws->order_next != &bp_nil_window_state ||
                               ws->order_prev != &bp_nil_window_state)
                            {
                                ui_ctx_menu_open(close_ctx_menu_key, cls_sig.box->key, v2f32(0, dim_2f32(cls_sig.box->rect).y));
                            }
                            else
                            {
                                bp_cmd(BP_CmdKind_Exit);
                            }
                        }
                        wm_window_push_custom_title_bar_client_area(ws->os, min_sig.box->rect);
                        wm_window_push_custom_title_bar_client_area(ws->os, max_sig.box->rect);
                        wm_window_push_custom_title_bar_client_area(ws->os, pad_2f32(cls_sig.box->rect, 2.f));
                    }
                }
            }
        }
    
        ////////////////////////////
        //- rjf: @window_ui_part bottom bar
        //
        ProfScope("build bottom bar")
        {
            String8 tag = str8_lit("pop");
            if(bp_state->bind_change_active)
            {
                tag = str8_lit("pop");
            }
            else if(ws->error_t >= 0.01f && ws->error_string_size != 0)
            {
                tag = str8_lit("bad_pop");
            }
      
            DR_FStrList status_fstrs = {0};
            {
                if(bp_state->bind_change_active)
                {
                    BP_CmdKindInfo *info = bp_cmd_kind_info_from_string(bp_state->bind_change_cmd_name);
                    String8 display_name = bp_display_from_code_name(info->string);
                    String8 string = push_str8f(scratch.arena, "Currently rebinding \"%S\"", display_name);
                    DR_FStrParams params = {ui_top_font(), ui_top_text_raster_flags(), ui_color_from_name(str8_lit("text")), ui_top_font_size()};
                    dr_fstrs_push_new(scratch.arena, &status_fstrs, &params, string);
                }
                else if(ws->error_t >= 0.01f && ws->error_string_size != 0)
                {
                    String8 error_string = str8(ws->error_buffer, ws->error_string_size);
                    ws->error_t -= bp_state->frame_dt/8.f;
                    bp_request_frame();
                    ui_set_next_pref_width(ui_children_sum(1));
                    UI_CornerRadius(4)
                        UI_Row
                        UI_PrefWidth(ui_text_dim(10, 1))
                        UI_TextAlignment(UI_TextAlign_Center)
                    {
                        DR_FStrList error_fstrs = bp_fstrs_from_rich_string(scratch.arena, error_string);
                        DR_FStrParams params = {ui_top_font(), ui_top_text_raster_flags(), ui_color_from_name(str8_lit("text")), ui_top_font_size()};
                        dr_fstrs_push_new(scratch.arena, &status_fstrs, &params, bp_icon_kind_text_table[BP_IconKind_WarningBig],
                                          .font = bp_font_from_slot(BP_FontSlot_Icons),
                                          .raster_flags = bp_raster_flags_from_slot(BP_FontSlot_Icons));
                        dr_fstrs_push_new(scratch.arena, &status_fstrs, &params, str8_lit("  "));
                        dr_fstrs_concat_in_place(&status_fstrs, &error_fstrs);
                    }
                }
            }
      
            UI_Flags(UI_BoxFlag_DrawBackground) UI_CornerRadius(0)
                UI_Tag(tag)
                UI_Pane(bottom_bar_rect, str8_lit("###bottom_bar")) UI_WidthFill UI_Row
                UI_Flags(0)
            {
                Temp scratch = scratch_begin(0, 0);
        
                if(DEV_updating_indicator)
                {
                    f32 animation_t = pow_f32(sin_f32(bp_state->time_in_seconds/2.f), 2.f);
                    ui_spacer(ui_em(0.3f, 1.f));
                    ui_spacer(ui_em(1.5f*animation_t, 1.f));
                    UI_PrefWidth(ui_text_dim(10, 1)) ui_labelf("*");
                    ui_spacer(ui_em(1.5f*(1-animation_t), 1.f));
                }
        
                UI_PrefWidth(ui_text_dim(10, 1))
                {
                    ui_spacer(ui_em(1.f, 1.f));
                    UI_Box *box = ui_build_box_from_key(UI_BoxFlag_DrawText, ui_key_zero());
                    ui_box_equip_display_fstrs(box, &status_fstrs);
                }
        
                ui_spacer(ui_pct(1, 0));
        
                UI_FontSize(ui_top_font_size()*0.85f)
                    UI_PrefWidth(ui_text_dim(10, 1))
                    UI_TextAlignment(UI_TextAlign_Center)
                {
                    ui_label(str8_lit(BUILD_TITLE));
                }
        
                scratch_end(scratch);
            }
        }
    
        ////////////////////////////
        //- rjf: @window_ui_part panel non-leaf UI (drag boundaries, drag/drop sites)
        //
        bool32 is_changing_panel_boundaries = 0;
        ProfScope("non-leaf panel UI")
            for(CFG_PanelNode *panel = panel_tree.root;
                panel != &cfg_nil_panel_node;
                panel = cfg_panel_node_rec__depth_first_pre(panel_tree.root, panel).next)
            {
                if(panel->first == &cfg_nil_panel_node)
                {
                    continue;
                }
      
                Axis2 split_axis = panel->split_axis;
                Rng2f32 panel_rect = cfg_target_rect_from_panel_node(content_rect, panel_tree.root, panel);
      
                {
                    CFG_Node *drag_view = cfg_node_from_id(bp_state->drag_drop_regs->view);
                    if(bp_drag_is_active() && bp_state->drag_drop_regs_slot == BP_RegSlot_View && drag_view != &cfg_nil_node)
                    {
                        f32 drop_site_major_dim_px = ceil_f32(ui_top_font_size()*7.f);
                        f32 drop_site_minor_dim_px = ceil_f32(ui_top_font_size()*5.f);
                        f32 corner_radius = ui_top_font_size()*0.5f;
                        f32 padding = ceil_f32(ui_top_font_size()*0.5f);
          
                        if(panel == panel_tree.root) UI_CornerRadius(corner_radius)
                        {
                            Vec2f32 panel_rect_center = center_2f32(panel_rect);
                            Axis2 axis = axis2_flip(panel_tree.root->split_axis);
                            for EachEnumVal(Side, side)
                            {
                                UI_Key key = ui_key_from_stringf(ui_key_zero(), "root_extra_split_%i", side);
                                Rng2f32 site_rect = panel_rect;
                                site_rect.p0.v[axis2_flip(axis)] = panel_rect_center.v[axis2_flip(axis)] - drop_site_major_dim_px/2;
                                site_rect.p1.v[axis2_flip(axis)] = panel_rect_center.v[axis2_flip(axis)] + drop_site_major_dim_px/2;
                                site_rect.p0.v[axis] = panel_rect.v[side].v[axis] - drop_site_minor_dim_px/2;
                                site_rect.p1.v[axis] = panel_rect.v[side].v[axis] + drop_site_minor_dim_px/2;
                                
                                UI_Box *site_box = &ui_nil_box;
                                {
                                    f32 site_open_t = ui_anim(ui_key_from_stringf(key, "open_t"), 1.f, .rate = bp_state->menu_animation_rate);
                                    UI_Rect(site_rect) UI_Squish(0.1f-0.1f*site_open_t) UI_Transparency(1-site_open_t)
                                    {
                                        site_box = ui_build_box_from_key(UI_BoxFlag_DropSite|UI_BoxFlag_DrawHotEffects, key);
                                        ui_signal_from_box(site_box);
                                    }
                                    UI_Box *site_box_viz = &ui_nil_box;
                                    UI_Parent(site_box) UI_WidthFill UI_HeightFill
                                        UI_Padding(ui_px(padding, 1.f))
                                        UI_Column
                                        UI_Padding(ui_px(padding, 1.f))
                                        UI_GroupKey(key)
                                    {
                                        ui_set_next_child_layout_axis(axis2_flip(axis));
                                        site_box_viz = ui_build_box_from_key(UI_BoxFlag_DrawBackground|
                                                                             UI_BoxFlag_DrawBorder|
                                                                             UI_BoxFlag_DrawDropShadow|
                                                                             UI_BoxFlag_DrawBackgroundBlur|
                                                                             UI_BoxFlag_DrawHotEffects, ui_key_zero());
                                    }
                                    UI_Parent(site_box_viz) UI_WidthFill UI_HeightFill UI_Padding(ui_px(padding, 1.f))
                                    {
                                        ui_set_next_child_layout_axis(axis);
                                        UI_Box *row_or_column = ui_build_box_from_key(0, ui_key_zero()); UI_Parent(row_or_column) UI_Padding(ui_px(padding, 1.f))
                                        {
                                            ui_build_box_from_key(UI_BoxFlag_DrawBorder, ui_key_zero());
                                            ui_spacer(ui_px(padding, 1.f));
                                            ui_build_box_from_key(UI_BoxFlag_DrawBorder, ui_key_zero());
                                        }
                                    }
                                }
                                
                                if(ui_key_match(site_box->key, ui_drop_hot_key()))
                                {
                                    Rng2f32 future_split_rect_target = site_rect;
                                    future_split_rect_target.p0.v[axis] -= drop_site_major_dim_px;
                                    future_split_rect_target.p1.v[axis] += drop_site_major_dim_px;
                                    future_split_rect_target.p0.v[axis2_flip(axis)] = panel_rect.p0.v[axis2_flip(axis)];
                                    future_split_rect_target.p1.v[axis2_flip(axis)] = panel_rect.p1.v[axis2_flip(axis)];
                                    future_split_rect_target = pad_2f32(future_split_rect_target, -ui_top_font_size()*2.f);
                                    Vec2f32 future_split_rect_target_center = center_2f32(future_split_rect_target);
                                    Rng2f32 future_split_rect =
                                        {
                                            ui_anim(ui_key_from_stringf(ui_key_zero(), "drop_site_v0"), future_split_rect_target.x0, .initial = future_split_rect_target_center.x, .rate = bp_state->menu_animation_rate),
                                            ui_anim(ui_key_from_stringf(ui_key_zero(), "drop_site_v1"), future_split_rect_target.y0, .initial = future_split_rect_target_center.y, .rate = bp_state->menu_animation_rate),
                                            ui_anim(ui_key_from_stringf(ui_key_zero(), "drop_site_v2"), future_split_rect_target.x1, .initial = future_split_rect_target_center.x, .rate = bp_state->menu_animation_rate),
                                            ui_anim(ui_key_from_stringf(ui_key_zero(), "drop_site_v3"), future_split_rect_target.y1, .initial = future_split_rect_target_center.y, .rate = bp_state->menu_animation_rate),
                                        };
                                    UI_Rect(future_split_rect) UI_TagF("drop_site") UI_CornerRadius(ui_top_font_size()*2.f)
                                    {
                                        ui_build_box_from_key(UI_BoxFlag_DrawBackground|UI_BoxFlag_DrawBorder, ui_key_zero());
                                    }
                                }
                                
                                if(ui_key_match(site_box->key, ui_drop_hot_key()) && bp_drag_drop())
                                {
                                    Dir2 dir = (axis == Axis2_Y ? (side == Side_Min ? Dir2_Up : Dir2_Down) :
                                                axis == Axis2_X ? (side == Side_Min ? Dir2_Left : Dir2_Right) :
                                                Dir2_Invalid);
                                    if(dir != Dir2_Invalid)
                                    {
                                        CFG_PanelNode *split_panel = panel;
                                        bp_cmd(BP_CmdKind_SplitPanel,
                                               .dst_panel  = split_panel->cfg->id,
                                               .panel      = bp_state->drag_drop_regs->panel,
                                               .view      = bp_state->drag_drop_regs->view,
                                               .dir2       = dir);
                                    }
                                }
                            }
                        }
                        
                        Axis2 split_axis = panel->split_axis;
                        UI_CornerRadius(corner_radius) for(CFG_PanelNode *child = panel->first;; child = child->next)
                        {
                            Rng2f32 child_rect = cfg_target_rect_from_panel_node_child(panel_rect, panel, child);
                            Vec2f32 child_rect_center = center_2f32(child_rect);
                            UI_Key key = ui_key_from_stringf(ui_key_zero(), "drop_boundary_%p_%p", panel->cfg, child->cfg);
                            Rng2f32 site_rect = r2f32(child_rect_center, child_rect_center);
                            site_rect.p0.v[split_axis] = child_rect.p0.v[split_axis] - drop_site_minor_dim_px/2;
                            site_rect.p1.v[split_axis] = child_rect.p0.v[split_axis] + drop_site_minor_dim_px/2;
                            site_rect.p0.v[axis2_flip(split_axis)] -= drop_site_major_dim_px/2;
                            site_rect.p1.v[axis2_flip(split_axis)] += drop_site_major_dim_px/2;
            
                            UI_Box *site_box = &ui_nil_box;
                            {
                                f32 site_open_t = ui_anim(ui_key_from_stringf(key, "open_t"), 1.f, .rate = bp_state->menu_animation_rate);
                                UI_Rect(site_rect) UI_Squish(0.1f-0.1f*site_open_t) UI_Transparency(1-site_open_t)
                                {
                                    site_box = ui_build_box_from_key(UI_BoxFlag_DropSite|UI_BoxFlag_DrawHotEffects, key);
                                    ui_signal_from_box(site_box);
                                }
                                UI_Box *site_box_viz = &ui_nil_box;
                                UI_Parent(site_box) UI_WidthFill UI_HeightFill
                                    UI_Padding(ui_px(padding, 1.f))
                                    UI_Column
                                    UI_Padding(ui_px(padding, 1.f))
                                    UI_GroupKey(key)
                                {
                                    ui_set_next_child_layout_axis(axis2_flip(split_axis));
                                    site_box_viz = ui_build_box_from_key(UI_BoxFlag_DrawBackground|
                                                                         UI_BoxFlag_DrawBorder|
                                                                         UI_BoxFlag_DrawDropShadow|
                                                                         UI_BoxFlag_DrawBackgroundBlur|
                                                                         UI_BoxFlag_DrawHotEffects, ui_key_zero());
                                }
                                UI_Parent(site_box_viz) UI_WidthFill UI_HeightFill UI_Padding(ui_px(padding, 1.f))
                                {
                                    ui_set_next_child_layout_axis(split_axis);
                                    UI_Box *row_or_column = ui_build_box_from_key(0, ui_key_zero()); UI_Parent(row_or_column) UI_Padding(ui_px(padding, 1.f))
                                    {
                                        ui_build_box_from_key(UI_BoxFlag_DrawBorder, ui_key_zero());
                                        ui_spacer(ui_px(padding, 1.f));
                                        ui_build_box_from_key(UI_BoxFlag_DrawBorder, ui_key_zero());
                                    }
                                }
                            }
            
                            if(ui_key_match(site_box->key, ui_drop_hot_key()))
                            {
                                Rng2f32 future_split_rect_target = site_rect;
                                future_split_rect_target.p0.v[split_axis] -= drop_site_major_dim_px;
                                future_split_rect_target.p1.v[split_axis] += drop_site_major_dim_px;
                                future_split_rect_target.p0.v[axis2_flip(split_axis)] = child_rect.p0.v[axis2_flip(split_axis)];
                                future_split_rect_target.p1.v[axis2_flip(split_axis)] = child_rect.p1.v[axis2_flip(split_axis)];
                                future_split_rect_target = pad_2f32(future_split_rect_target, -ui_top_font_size()*2.f);
                                Vec2f32 future_split_rect_target_center = center_2f32(future_split_rect_target);
                                Rng2f32 future_split_rect = {
                                    ui_anim(ui_key_from_stringf(ui_key_zero(), "drop_site_v0"), future_split_rect_target.x0, .initial = future_split_rect_target_center.x, .rate = bp_state->menu_animation_rate),
                                    ui_anim(ui_key_from_stringf(ui_key_zero(), "drop_site_v1"), future_split_rect_target.y0, .initial = future_split_rect_target_center.y, .rate = bp_state->menu_animation_rate),
                                    ui_anim(ui_key_from_stringf(ui_key_zero(), "drop_site_v2"), future_split_rect_target.x1, .initial = future_split_rect_target_center.x, .rate = bp_state->menu_animation_rate),
                                    ui_anim(ui_key_from_stringf(ui_key_zero(), "drop_site_v3"), future_split_rect_target.y1, .initial = future_split_rect_target_center.y, .rate = bp_state->menu_animation_rate),
                                };
                                UI_Rect(future_split_rect) UI_TagF("drop_site") UI_CornerRadius(ui_top_font_size()*2.f)
                                {
                                    ui_build_box_from_key(UI_BoxFlag_DrawBackground|UI_BoxFlag_DrawBorder, ui_key_zero());
                                }
                            }
            
                            if(ui_key_match(site_box->key, ui_drop_hot_key()) && bp_drag_drop())
                            {
                                Dir2 dir = (panel->split_axis == Axis2_X ? Dir2_Left : Dir2_Up);
                                CFG_PanelNode *split_panel = child;
                                if(split_panel == &cfg_nil_panel_node)
                                {
                                    split_panel = panel->last;
                                    dir = (panel->split_axis == Axis2_X ? Dir2_Right : Dir2_Down);
                                }
                                bp_cmd(BP_CmdKind_SplitPanel,
                                       .dst_panel  = split_panel->cfg->id,
                                       .panel      = bp_state->drag_drop_regs->panel,
                                       .view      = bp_state->drag_drop_regs->view,
                                       .dir2       = dir);
                            }
            
                            if(child == &cfg_nil_panel_node)
                            {
                                break;
                            }
                        }
                    }
                }
      
                for(CFG_PanelNode *child = panel->first;
                    child != &cfg_nil_panel_node && child->next != &cfg_nil_panel_node;
                    child = child->next)
                {
                    CFG_PanelNode *min_child = child;
                    CFG_PanelNode *max_child = min_child->next;
                    Rng2f32 min_child_rect = cfg_target_rect_from_panel_node_child(panel_rect, panel, min_child);
                    Rng2f32 max_child_rect = cfg_target_rect_from_panel_node_child(panel_rect, panel, max_child);
                    Rng2f32 boundary_rect = {0};
                    {
                        boundary_rect.p0.v[split_axis] = min_child_rect.p1.v[split_axis] - ui_top_font_size()/3;
                        boundary_rect.p1.v[split_axis] = max_child_rect.p0.v[split_axis] + ui_top_font_size()/3;
                        boundary_rect.p0.v[axis2_flip(split_axis)] = panel_rect.p0.v[axis2_flip(split_axis)];
                        boundary_rect.p1.v[axis2_flip(split_axis)] = panel_rect.p1.v[axis2_flip(split_axis)];
                    }
        
                    UI_Rect(boundary_rect)
                    {
                        ui_set_next_hover_cursor(split_axis == Axis2_X ? WM_Cursor_LeftRight : WM_Cursor_UpDown);
                        UI_Box *box = ui_build_box_from_stringf(UI_BoxFlag_Clickable, "###%p_%p", min_child->cfg, max_child->cfg);
                        UI_Signal sig = ui_signal_from_box(box);
                        if(ui_double_clicked(sig))
                        {
                            ui_kill_action();
                            f32 sum_pct = min_child->pct_of_parent + max_child->pct_of_parent;
                            min_child->pct_of_parent = 0.5f * sum_pct;
                            max_child->pct_of_parent = 0.5f * sum_pct;
                            cfg_node_equip_stringf(bp_state->cfg, min_child->cfg, "%f", min_child->pct_of_parent);
                            cfg_node_equip_stringf(bp_state->cfg, max_child->cfg, "%f", max_child->pct_of_parent);
                        }
                        else if(ui_pressed(sig))
                        {
                            Vec2f32 v = {min_child->pct_of_parent, max_child->pct_of_parent};
                            ui_store_drag_struct(&v);
                        }
                        else if(ui_dragging(sig))
                        {
                            Vec2f32 v = *ui_get_drag_struct(Vec2f32);
                            Vec2f32 mouse_delta      = ui_drag_delta();
                            f32 total_size           = dim_2f32(panel_rect).v[split_axis];
                            f32 min_pct__before      = v.v[0];
                            f32 min_pixels__before   = min_pct__before * total_size;
                            f32 min_pixels__after    = min_pixels__before + mouse_delta.v[split_axis];
                            if(min_pixels__after < 50.f)
                            {
                                min_pixels__after = 50.f;
                            }
                            f32 min_pct__after       = min_pixels__after / total_size;
                            f32 pct_delta            = min_pct__after - min_pct__before;
                            f32 max_pct__before      = v.v[1];
                            f32 max_pct__after       = max_pct__before - pct_delta;
                            f32 max_pixels__after    = max_pct__after * total_size;
                            if(max_pixels__after < 50.f)
                            {
                                max_pixels__after = 50.f;
                                max_pct__after = max_pixels__after / total_size;
                                pct_delta = -(max_pct__after - max_pct__before);
                                min_pct__after = min_pct__before + pct_delta;
                            }
                            min_child->pct_of_parent = min_pct__after;
                            max_child->pct_of_parent = max_pct__after;
                            cfg_node_equip_stringf(bp_state->cfg, min_child->cfg, "%f", min_pct__after);
                            cfg_node_equip_stringf(bp_state->cfg, max_child->cfg, "%f", max_pct__after);
                            is_changing_panel_boundaries = 1;
                        }
                    }
                }
            }
    
        ////////////////////////////
        //- rjf: @window_ui_part animate panels
        {
            bool32 window_is_resizing = (ws->last_window_rect.x1 != window_rect.x1 ||
                                      ws->last_window_rect.y1 != window_rect.y1);
            Vec2f32 content_rect_dim = dim_2f32(content_rect);
            if(content_rect_dim.x > 0 && content_rect_dim.y > 0)
            {
                for(CFG_PanelNode *panel = panel_tree.root;
                    panel != &cfg_nil_panel_node;
                    panel = cfg_panel_node_rec__depth_first_pre(panel_tree.root, panel).next)
                {
                    Rng2f32 target_rect_px = cfg_target_rect_from_panel_node(content_rect, panel_tree.root, panel);
                    Rng2f32 target_rect_pct = r2f32p(target_rect_px.x0/content_rect_dim.x,
                                                     target_rect_px.y0/content_rect_dim.y,
                                                     target_rect_px.x1/content_rect_dim.x,
                                                     target_rect_px.y1/content_rect_dim.y);
                    bool32 reset = (window_is_resizing || ws->window_layout_reset || ws->frames_alive < 5 || is_changing_panel_boundaries);
                    ui_anim(ui_key_from_stringf(ui_key_zero(), "panel_%p_x0", panel->cfg), target_rect_pct.x0, .initial = target_rect_pct.x0, .reset = reset, .rate = bp_state->menu_animation_rate);
                    ui_anim(ui_key_from_stringf(ui_key_zero(), "panel_%p_y0", panel->cfg), target_rect_pct.y0, .initial = target_rect_pct.y0, .reset = reset, .rate = bp_state->menu_animation_rate);
                    ui_anim(ui_key_from_stringf(ui_key_zero(), "panel_%p_x1", panel->cfg), target_rect_pct.x1, .initial = target_rect_pct.x1, .reset = reset, .rate = bp_state->menu_animation_rate);
                    ui_anim(ui_key_from_stringf(ui_key_zero(), "panel_%p_y1", panel->cfg), target_rect_pct.y1, .initial = target_rect_pct.y1, .reset = reset, .rate = bp_state->menu_animation_rate);
                }
            }
            ws->window_layout_reset = 0;
        }
    
        ////////////////////////////
        //- rjf: @window_ui_part panel leaf UI
        //
        if(content_rect.x1 > content_rect.x0 && content_rect.y1 > content_rect.y0)
        {
            ProfScope("leaf panel UI")
                for(CFG_PanelNode *panel = panel_tree.root;
                    panel != &cfg_nil_panel_node;
                    panel = cfg_panel_node_rec__depth_first_pre(panel_tree.root, panel).next)
            {
                if(panel->first != &cfg_nil_panel_node) {continue;}
                bool32 panel_is_focused = (window_is_focused &&
                                        !bp_state->popup_active &&
                                        !ws->menu_bar_focused &&
                                        !query_is_open &&
                                        !ui_any_ctx_menu_is_open() &&
                                        panel_tree.focused == panel);
                CFG_Node *selected_tab = panel->selected_tab;
                BP_View_State *selected_tab_view_state = bp_view_state_from_cfg(selected_tab);
                ProfScope("leaf panel UI work - %.*s: %.*s", str8_varg(selected_tab->string), str8_varg(bp_path_from_cfg(selected_tab)))
                    UI_Focus(panel_is_focused ? UI_FocusKind_Null : UI_FocusKind_Off)
                {
                    //////////////////////////
                    //- rjf: calculate UI rectangles
                    //
                    Vec2f32 content_rect_dim = dim_2f32(content_rect);
                    Rng2f32 target_rect_px = cfg_target_rect_from_panel_node(content_rect, panel_tree.root, panel);
                    Rng2f32 target_rect_pct = r2f32p(target_rect_px.x0 / content_rect_dim.x,
                                                     target_rect_px.y0 / content_rect_dim.y,
                                                     target_rect_px.x1 / content_rect_dim.x,
                                                     target_rect_px.y1 / content_rect_dim.y);
                    Rng2f32 panel_rect_pct = r2f32p(ui_anim(ui_key_from_stringf(ui_key_zero(), "panel_%p_x0", panel->cfg), target_rect_pct.x0, .initial = target_rect_pct.x0, .rate = bp_state->menu_animation_rate),
                                                    ui_anim(ui_key_from_stringf(ui_key_zero(), "panel_%p_y0", panel->cfg), target_rect_pct.y0, .initial = target_rect_pct.y0, .rate = bp_state->menu_animation_rate),
                                                    ui_anim(ui_key_from_stringf(ui_key_zero(), "panel_%p_x1", panel->cfg), target_rect_pct.x1, .initial = target_rect_pct.x1, .rate = bp_state->menu_animation_rate),
                                                    ui_anim(ui_key_from_stringf(ui_key_zero(), "panel_%p_y1", panel->cfg), target_rect_pct.y1, .initial = target_rect_pct.y1, .rate = bp_state->menu_animation_rate));
                    Rng2f32 panel_rect = r2f32p(panel_rect_pct.x0*content_rect_dim.x,
                                                panel_rect_pct.y0*content_rect_dim.y,
                                                panel_rect_pct.x1*content_rect_dim.x,
                                                panel_rect_pct.y1*content_rect_dim.y);
                    panel_rect = pad_2f32(panel_rect, floor_f32(-ui_top_font_size()*0.25f));
                    panel_rect = r2f32p(round_f32(panel_rect.x0), round_f32(panel_rect.y0), round_f32(panel_rect.x1), round_f32(panel_rect.y1));
                    f32 tab_bar_rheight = floor_f32(ui_top_font_size()*3.5f);
                    f32 tab_bar_vheight = floor_f32(ui_top_font_size()*bp_setting_f32_from_name(str8_lit("tab_height")));
                    f32 tab_bar_rv_diff = tab_bar_rheight - tab_bar_vheight;
                    f32 tab_spacing = floor_f32(ui_top_font_size()*0.4f);
                    Rng2f32 tab_bar_rect = r2f32p(panel_rect.x0, panel_rect.y0, panel_rect.x1, panel_rect.y0 + tab_bar_vheight);
                    Rng2f32 content_rect = r2f32p(panel_rect.x0, panel_rect.y0+tab_bar_vheight, panel_rect.x1, panel_rect.y1);
                    if(panel->tab_side == Side_Max)
                    {
                        tab_bar_rect.y0 = panel_rect.y1 - tab_bar_vheight;
                        tab_bar_rect.y1 = panel_rect.y1;
                        content_rect.y0 = panel_rect.y0;
                        content_rect.y1 = panel_rect.y1 - tab_bar_vheight;
                    }
                    tab_bar_rect = intersect_2f32(tab_bar_rect, panel_rect);
                    content_rect = intersect_2f32(content_rect, panel_rect);
                    
                    bool32 build_panel = (content_rect.x1 > content_rect.x0 && content_rect.y1 > content_rect.y0);
                    
                    //////////////////////////
                    //- rjf: build combined split+movetab drag/drop sites
                    //
                    if(build_panel)
                    {
                        CFG_Node *view = cfg_node_from_id(bp_state->drag_drop_regs->view);
                        if(bp_drag_is_active() && bp_state->drag_drop_regs_slot == BP_RegSlot_View && view != &cfg_nil_node && contains_2f32(panel_rect, ui_mouse()) && ui_key_match(ui_drop_hot_key(), ui_key_zero()))
                        {
                            f32 drop_site_dim_px = ceil_f32(ui_top_font_size()*7.f);
                            drop_site_dim_px = Min(drop_site_dim_px, dim_2f32(panel_rect).v[panel->split_axis]/4.f);
                            drop_site_dim_px = Max(drop_site_dim_px, ceil_f32(ui_top_font_size()*3.f));
                            Vec2f32 drop_site_half_dim = v2f32(drop_site_dim_px/2, drop_site_dim_px/2);
                            Vec2f32 panel_center = center_2f32(panel_rect);
                            f32 corner_radius = ui_top_font_size()*0.5f;
                            f32 padding = ceil_f32(ui_top_font_size()*0.5f);
                            struct
                            {
                                UI_Key key;
                                Dir2 split_dir;
                                Rng2f32 rect;
                            } sites[] = {
                                {
                                    ui_key_from_stringf(ui_key_zero(), "drop_split_center_%p", panel->cfg),
                                    Dir2_Invalid,
                                    r2f32(sub_2f32(panel_center, drop_site_half_dim),
                                          add_2f32(panel_center, drop_site_half_dim))
                                },
                                {
                                    ui_key_from_stringf(ui_key_zero(), "drop_split_up_%p", panel->cfg),
                                    Dir2_Up,
                                    r2f32p(panel_center.x-drop_site_half_dim.x,
                                           panel_center.y-drop_site_half_dim.y - drop_site_half_dim.y*2,
                                           panel_center.x+drop_site_half_dim.x,
                                           panel_center.y+drop_site_half_dim.y - drop_site_half_dim.y*2),
                                },
                                {
                                    ui_key_from_stringf(ui_key_zero(), "drop_split_down_%p", panel->cfg),
                                    Dir2_Down,
                                    r2f32p(panel_center.x-drop_site_half_dim.x,
                                           panel_center.y-drop_site_half_dim.y + drop_site_half_dim.y*2,
                                           panel_center.x+drop_site_half_dim.x,
                                           panel_center.y+drop_site_half_dim.y + drop_site_half_dim.y*2),
                                },
                                {
                                    ui_key_from_stringf(ui_key_zero(), "drop_split_left_%p", panel->cfg),
                                    Dir2_Left,
                                    r2f32p(panel_center.x-drop_site_half_dim.x - drop_site_half_dim.x*2,
                                           panel_center.y-drop_site_half_dim.y,
                                           panel_center.x+drop_site_half_dim.x - drop_site_half_dim.x*2,
                                           panel_center.y+drop_site_half_dim.y),
                                },
                                {
                                    ui_key_from_stringf(ui_key_zero(), "drop_split_right_%p", panel->cfg),
                                    Dir2_Right,
                                    r2f32p(panel_center.x-drop_site_half_dim.x + drop_site_half_dim.x*2,
                                           panel_center.y-drop_site_half_dim.y,
                                           panel_center.x+drop_site_half_dim.x + drop_site_half_dim.x*2,
                                           panel_center.y+drop_site_half_dim.y),
                                },
                            };
                            UI_CornerRadius(corner_radius)
                                for(u64 idx = 0; idx < ArrayCount(sites); idx += 1)
                                {
                                    UI_Key key = sites[idx].key;
                                    Dir2 dir = sites[idx].split_dir;
                                    Rng2f32 rect = sites[idx].rect;
                                    Axis2 split_axis = axis2_from_dir2(dir);
                                    Side split_side = side_from_dir2(dir);
                                    if(dir != Dir2_Invalid && split_axis == panel->parent->split_axis)
                                    {
                                        continue;
                                    }
                                    UI_Box *site_box = &ui_nil_box;
                                    {
                                        f32 site_open_t = ui_anim(ui_key_from_stringf(key, "open_t"), 1.f, .rate = bp_state->menu_animation_rate);
                                        UI_Rect(rect) UI_Squish(0.1f-0.1f*site_open_t) UI_Transparency(1-site_open_t)
                                        {
                                            site_box = ui_build_box_from_key(UI_BoxFlag_DropSite|UI_BoxFlag_DrawHotEffects, key);
                                            ui_signal_from_box(site_box);
                                        }
                                        UI_Box *site_box_viz = &ui_nil_box;
                                        UI_GroupKey(key)
                                            UI_Parent(site_box) UI_WidthFill UI_HeightFill
                                            UI_Padding(ui_px(padding, 1.f))
                                            UI_Column
                                            UI_Padding(ui_px(padding, 1.f))
                                        {
                                            ui_set_next_child_layout_axis(axis2_flip(split_axis));
                                            site_box_viz = ui_build_box_from_key(UI_BoxFlag_DrawBackground|
                                                                                 UI_BoxFlag_DrawBorder|
                                                                                 UI_BoxFlag_DrawDropShadow|
                                                                                 UI_BoxFlag_DrawBackgroundBlur|
                                                                                 UI_BoxFlag_DrawHotEffects, ui_key_zero());
                                        }
                                        if(dir != Dir2_Invalid)
                                        {
                                            UI_Parent(site_box_viz) UI_WidthFill UI_HeightFill UI_Padding(ui_px(padding, 1.f))
                                            {
                                                ui_set_next_child_layout_axis(split_axis);
                                                UI_Box *row_or_column = ui_build_box_from_key(0, ui_key_zero());
                                                UI_Parent(row_or_column) UI_Padding(ui_px(padding, 1.f)) UI_TagF("drop_site")
                                                {
                                                    if(split_side == Side_Min) { ui_set_next_flags(UI_BoxFlag_DrawBackground); }
                                                    ui_build_box_from_key(UI_BoxFlag_DrawBorder, ui_key_zero());
                                                    ui_spacer(ui_px(padding, 1.f));
                                                    if(split_side == Side_Max) { ui_set_next_flags(UI_BoxFlag_DrawBackground); }
                                                    ui_build_box_from_key(UI_BoxFlag_DrawBorder, ui_key_zero());
                                                }
                                            }
                                        }
                                        else
                                        {
                                            UI_Parent(site_box_viz) UI_WidthFill UI_HeightFill UI_Padding(ui_px(padding, 1.f))
                                            {
                                                ui_set_next_child_layout_axis(split_axis);
                                                UI_Box *row_or_column = ui_build_box_from_key(0, ui_key_zero());
                                                UI_Parent(row_or_column) UI_Padding(ui_px(padding, 1.f)) UI_TagF("drop_site")
                                                {
                                                    ui_build_box_from_key(UI_BoxFlag_DrawBorder|UI_BoxFlag_DrawBackground, ui_key_zero());
                                                }
                                            }
                                        }
                                    }
                                    if(ui_key_match(site_box->key, ui_drop_hot_key()) && bp_drag_drop())
                                    {
                                        if(dir != Dir2_Invalid)
                                        {
                                            bp_cmd(BP_CmdKind_SplitPanel,
                                                   .dst_panel = panel->cfg->id,
                                                   .panel = bp_state->drag_drop_regs->panel,
                                                   .view = bp_state->drag_drop_regs->view,
                                                   .dir2 = dir);
                                        }
                                        else
                                        {
                                            bp_cmd(BP_CmdKind_MoveView,
                                                   .dst_panel = panel->cfg->id,
                                                   .panel = bp_state->drag_drop_regs->panel,
                                                   .view = bp_state->drag_drop_regs->view,
                                                   .prev_tab = cfg_node_ptr_list_last(&panel->tabs)->id);
                                        }
                                    }
                                }
                            for(u64 idx = 0; idx < ArrayCount(sites); idx += 1)
                            {
                                bool32 is_drop_hot = ui_key_match(ui_drop_hot_key(), sites[idx].key);
                                if(is_drop_hot)
                                {
                                    Axis2 split_axis = axis2_from_dir2(sites[idx].split_dir);
                                    Side split_side = side_from_dir2(sites[idx].split_dir);
                                    Rng2f32 future_split_rect_target = panel_rect;
                                    if(sites[idx].split_dir != Dir2_Invalid)
                                    {
                                        Vec2f32 panel_center = center_2f32(panel_rect);
                                        future_split_rect_target.v[side_flip(split_side)].v[split_axis] = panel_center.v[split_axis];
                                    }
                                    future_split_rect_target = pad_2f32(future_split_rect_target, -ui_top_font_size()*2.f);
                                    Vec2f32 future_split_rect_target_center = center_2f32(future_split_rect_target);
                                    Rng2f32 future_split_rect = {
                                        ui_anim(ui_key_from_stringf(ui_key_zero(), "drop_site_v0"), future_split_rect_target.x0, .initial = future_split_rect_target_center.x, .rate = bp_state->menu_animation_rate),
                                        ui_anim(ui_key_from_stringf(ui_key_zero(), "drop_site_v1"), future_split_rect_target.y0, .initial = future_split_rect_target_center.y, .rate = bp_state->menu_animation_rate),
                                        ui_anim(ui_key_from_stringf(ui_key_zero(), "drop_site_v2"), future_split_rect_target.x1, .initial = future_split_rect_target_center.x, .rate = bp_state->menu_animation_rate),
                                        ui_anim(ui_key_from_stringf(ui_key_zero(), "drop_site_v3"), future_split_rect_target.y1, .initial = future_split_rect_target_center.y, .rate = bp_state->menu_animation_rate),
                                    };
                                    UI_Rect(future_split_rect) UI_TagF("drop_site") UI_CornerRadius(ui_top_font_size()*2.f)
                                    {
                                        ui_build_box_from_key(UI_BoxFlag_DrawBackground|UI_BoxFlag_DrawBorder, ui_key_zero());
                                    }
                                }
                            }
                        }
                    }
                    
                    //////////////////////////
                    //- rjf: build catch-all panel drop-site
                    //
                    UI_Key catchall_drop_site_key = ui_key_from_stringf(ui_key_zero(), "catchall_drop_site_%p", panel->cfg);
                    if(build_panel && bp_drag_is_active() && bp_state->drag_drop_regs_slot == BP_RegSlot_View) UI_Rect(panel_rect)
                    {
                        UI_Box *catchall_drop_site = ui_build_box_from_key(UI_BoxFlag_DropSite, catchall_drop_site_key);
                        ui_signal_from_box(catchall_drop_site);
                    }
                    
                    //////////////////////////
                    //- rjf: panel not selected? -> darken
                    //
                    if(build_panel) if(panel != panel_tree.focused)
                    {
                        UI_Rect(content_rect) UI_TagF("inactive")
                            ui_build_box_from_key(UI_BoxFlag_DrawBackground, ui_key_zero());
                    }
                    
                    //////////////////////////
                    //- rjf: build panel container box
                    //
                    UI_Box *panel_box = &ui_nil_box;
                    if(build_panel) UI_Rect(content_rect) UI_ChildLayoutAxis(Axis2_Y) UI_CornerRadius(0) UI_Focus(UI_FocusKind_On)
                    {
                        UI_Key panel_key = ui_key_from_stringf(ui_key_zero(), "panel_box_%p", panel->cfg);
                        panel_box = ui_build_box_from_key(UI_BoxFlag_MouseClickable|
                                                          UI_BoxFlag_Clip|
                                                          UI_BoxFlag_DrawBorder|
                                                          UI_BoxFlag_DisableFocusOverlay|
                                                          ((panel_tree.focused != panel)*UI_BoxFlag_DisableFocusBorder),
                                                          panel_key);
                    }
                    
                    UI_Box *loading_overlay_container = &ui_nil_box;
                    if(build_panel) UI_Parent(panel_box) UI_WidthFill UI_HeightFill
                    {
                        loading_overlay_container = ui_build_box_from_key(UI_BoxFlag_Floating, ui_key_zero());
                    }
                    
                    //////////////////////////
                    //- rjf: build selected tab view
                    //
                    if(build_panel)
                        UI_Parent(panel_box)
                            UI_Focus(panel_is_focused ? UI_FocusKind_Null : UI_FocusKind_Off)
                            UI_WidthFill
                    {
                        bp_push_regs(.panel = panel->cfg->id,
                                     .tab = selected_tab->id,
                                     .view = selected_tab->id);
                        {
                            String8 view_file_path = bp_path_from_cfg(selected_tab);
                            if(view_file_path.size != 0)
                            {
                                bp_regs()->file_path = view_file_path;
                            }
                        }
                        
                        UI_Box *view_container_box = &ui_nil_box;
                        UI_FixedWidth(dim_2f32(content_rect).x)
                            UI_FixedHeight(dim_2f32(content_rect).y)
                            UI_ChildLayoutAxis(Axis2_Y)
                        {
                            view_container_box = ui_build_box_from_key(0, ui_key_zero());
                        }
                        
                        UI_Parent(view_container_box) if(selected_tab == &cfg_nil_node && panel->parent != &cfg_nil_panel_node)
                        {
                            ui_set_next_flags(UI_BoxFlag_DefaultFocusNav);
                            UI_Focus(UI_FocusKind_On) UI_WidthFill UI_HeightFill UI_NamedColumn(str8_lit("empty_view")) UI_TagF("weak")
                                UI_Padding(ui_pct(1, 0)) UI_Focus(UI_FocusKind_Null)
                            {
                                UI_PrefHeight(ui_em(3.f, 1.f))
                                    UI_Row
                                    UI_Padding(ui_pct(1, 0))
                                    UI_TextAlignment(UI_TextAlign_Center)
                                    UI_PrefWidth(ui_em(15.f, 1.f))
                                    UI_CornerRadius(ui_top_font_size()/2.f)
                                    UI_TagF("bad_pop")
                                {
                                    if(ui_clicked(bp_icon_buttonf(BP_IconKind_X, 0, "Close Panel")))
                                    {
                                        bp_cmd(BP_CmdKind_ClosePanel);
                                    }
                                }
                            }
                        }
                        
                        UI_Parent(view_container_box) if(selected_tab != &cfg_nil_node) ProfScope("build tab view")
                        {
                            bp_view_ui(content_rect);
                        }
                        
                        BP_Regs *view_regs = bp_pop_regs();
                        if(panel_tree.focused == panel)
                        {
                            MemoryCopyStruct(bp_regs(), view_regs);
                        }
                    }
                    
                    if(build_panel)
                    {
                        f32 selected_tab_loading_t = selected_tab_view_state->loading_t;
                        if(selected_tab_loading_t > 0.01f) UI_Parent(loading_overlay_container)
                        {
                            bp_loading_overlay(panel_rect, selected_tab_loading_t, selected_tab_view_state->loading_progress_v, selected_tab_view_state->loading_progress_v_target);
                        }
                    }
                    
                    if(build_panel)
                    {
                        UI_Signal panel_sig = ui_signal_from_box(panel_box);
                        if(ui_pressed(panel_sig))
                        {
                            bp_cmd(BP_CmdKind_FocusPanel, .panel = panel->cfg->id);
                        }
                    }
                    
                    //////////////////////////
                    //- rjf: compute tab build tasks
                    //
                    typedef struct TabTask TabTask;
                    struct TabTask
                    {
                        TabTask *next;
                        CFG_Node *tab;
                        DR_FStrList fstrs;
                        f32 tab_width;
                    };
                    TabTask *first_tab_task = 0;
                    TabTask *last_tab_task = 0;
                    u64 tab_task_count = 0;
                    f32 tab_close_width_px = ui_top_font_size()*2.5f;
                    f32 max_tab_width_px = ui_top_font_size()*20.f;
                    if(build_panel) UI_TagF("tab")
                    {
                        bool32 reset = (ws->window_layout_reset || ws->frames_alive < 5 || is_changing_panel_boundaries);
                        for(CFG_Node_Ptr_Node *n = panel->tabs.first; n != 0; n = n->next)
                        {
                            CFG_Node *tab = n->v;
                            if(bp_cfg_is_project_filtered(tab))
                            {
                                continue;
                            }
                            UI_TagF(tab != panel->selected_tab ? "inactive" : "")
                            {
                                TabTask *t = push_array(scratch.arena, TabTask, 1);
                                t->tab = tab;
                                t->fstrs = bp_title_fstrs_from_cfg(scratch.arena, tab, 0);
                                f32 tab_width_target = dr_dim_from_fstrs(ui_top_tab_size(), &t->fstrs).x + tab_close_width_px + ui_top_font_size()*1.f;
                                bool32 tab_is_selected = (tab == panel->selected_tab);
                                if(tab_is_selected && panel_tree.focused == panel)
                                {
                                    tab_width_target += tab_close_width_px;
                                }
                                tab_width_target = Min(max_tab_width_px, tab_width_target);
                                t->tab_width = floor_f32(ui_anim(ui_key_from_stringf(ui_key_zero(), "tab_width_%p", tab), tab_width_target, .initial = reset ? tab_width_target : 0, .rate = bp_state->menu_animation_rate));
                                SLLQueuePush(first_tab_task, last_tab_task, t);
                                tab_task_count += 1;
                            }
                        }
                    }
                    
                    //////////////////////////
                    //- rjf: build tab bar container
                    //
                    UI_Box *tab_bar_box = &ui_nil_box;
                    if(build_panel) UI_CornerRadius(0) UI_Rect(tab_bar_rect)
                    {
                        tab_bar_box = ui_build_box_from_stringf(UI_BoxFlag_Clip|
                                                                UI_BoxFlag_AllowOverflowY|
                                                                UI_BoxFlag_ViewClampX|
                                                                UI_BoxFlag_ViewScrollX|
                                                                UI_BoxFlag_Clickable,
                                                                "tab_bar_%p", panel->cfg);
                        if(panel->tab_side == Side_Max)
                        {
                            tab_bar_box->view_off.y = tab_bar_box->view_off_target.y = (tab_bar_rheight - tab_bar_vheight);
                        }
                        else
                        {
                            tab_bar_box->view_off.y = tab_bar_box->view_off_target.y = 0;
                        }
                    }
                    
                    //////////////////////////
                    //- rjf: determine tab drop site
                    //
                    bool32 tab_drop_is_active = bp_drag_is_active() && ui_key_match(ui_drop_hot_key(), catchall_drop_site_key);
                    CFG_Node *tab_drop_prev = &cfg_nil_node;
                    if(build_panel)
                    {
                        f32 best_prev_distance_px = 1000000.f;
                        TabTask start_boundary_tab_task = {first_tab_task, &cfg_nil_node};
                        f32 off = 0;
                        for(TabTask *task = &start_boundary_tab_task; task != 0; task = task->next)
                        {
                            off += task->tab_width;
                            Vec2f32 anchor_pt = v2f32(tab_bar_box->rect.x0 + off, tab_bar_box->rect.y1);
                            f32 distance = length_2f32(sub_2f32(ui_mouse(), anchor_pt));
                            if(distance < best_prev_distance_px)
                            {
                                best_prev_distance_px = distance;
                                tab_drop_prev = task->tab;
                            }
                        }
                    }
                    
                    if(tab_drop_is_active && bp_state->drag_drop_regs->panel == panel->cfg->id)
                    {
                        TabTask start_boundary_tab_task = {first_tab_task, &cfg_nil_node};
                        if(tab_drop_prev->id == bp_state->drag_drop_regs->view)
                        {
                            tab_drop_is_active = 0;
                        }
                        if(tab_drop_is_active) for(TabTask *t = &start_boundary_tab_task; t != 0; t = t->next)
                        {
                            if(t->tab == tab_drop_prev && t->next != 0 && t->next->tab->id == bp_state->drag_drop_regs->view)
                            {
                                tab_drop_is_active = 0;
                                break;
                            }
                        }
                    }
                    
                    //////////////////////////
                    //- rjf: build tab bar contents
                    //
                    if(build_panel) UI_Focus(UI_FocusKind_Off) UI_Parent(tab_bar_box) UI_Padding(ui_em(0.5f, 1.f)) UI_PrefHeight(ui_pct(1, 0)) UI_TagF("tab")
                    {
                        f32 corner_radius = ui_top_font_size()*0.6f;
                        TabTask start_boundary_tab_task = {first_tab_task, &cfg_nil_node};
                        UI_CornerRadius00(panel->tab_side == Side_Min ? corner_radius : 0)
                            UI_CornerRadius01(panel->tab_side == Side_Min ? 0 : corner_radius)
                            UI_CornerRadius10(panel->tab_side == Side_Min ? corner_radius : 0)
                            UI_CornerRadius11(panel->tab_side == Side_Min ? 0 : corner_radius)
                            for(TabTask *tab_task = &start_boundary_tab_task; tab_task != 0; tab_task = tab_task->next)
                            {
                                CFG_Node *tab = tab_task->tab;
                                
                                DR_FStrList tab_fstrs = tab_task->fstrs;
                                f32 tab_width_px = tab_task->tab_width;
                                if(tab != &cfg_nil_node) BP_RegsScope(.panel = panel->cfg->id, .view = tab->id, .tab = tab->id)
                                {
                                    bool32 tab_is_selected = (tab == panel->selected_tab);
                                    bool32 tab_is_auto = bp_view_setting_b32_from_name(str8_lit("auto"));
                                    
                                    ui_set_next_child_layout_axis(Axis2_Y);
                                    ui_set_next_pref_width(ui_px(tab_width_px, 1));
                                    UI_Box *tab_column_box = ui_build_box_from_stringf(!is_changing_panel_boundaries*UI_BoxFlag_AnimatePosX, "tab_column_%p", tab);
                                    
                                    bool32 omit_name = 0;
                                    if(bp_drag_is_active() && bp_state->drag_drop_regs->view == tab->id && bp_state->drag_drop_regs_slot == BP_RegSlot_View)
                                    {
                                        omit_name = 1;
                                    }
                                    
                                    UI_Parent(tab_column_box)
                                        UI_PrefHeight(ui_px(tab_bar_vheight, 1))
                                        UI_TagF(omit_name ? "hollow" : "")
                                        UI_TagF(!omit_name && !tab_is_selected ? "inactive" : "")
                                        UI_TagF(!omit_name && tab_is_auto ? "auto" : "")
                                    {
                                        if(panel->tab_side == Side_Max)
                                        {
                                            ui_spacer(ui_px(tab_bar_rv_diff-1.f, 1.f));
                                        }
                                        else
                                        {
                                            ui_spacer(ui_px(1.f, 1.f));
                                        }
                                        UI_Box *tab_box = ui_build_box_from_stringf(UI_BoxFlag_DrawHotEffects|
                                                                                    UI_BoxFlag_DrawBackground|
                                                                                    UI_BoxFlag_DrawBorder|
                                                                                    (UI_BoxFlag_DrawDropShadow*tab_is_selected)|
                                                                                    UI_BoxFlag_Clickable,
                                                                                    "tab_%p", tab);
                                        
                                        if(!omit_name) UI_Parent(tab_box)
                                        {
                                            UI_WidthFill UI_Row
                                            {
                                                ui_spacer(ui_em(0.5f, 1.f));
                                                UI_PrefWidth(ui_text_dim(10, 0))
                                                {
                                                    UI_Box *name_box = ui_build_box_from_key(UI_BoxFlag_DrawText, ui_key_zero());
                                                    ui_box_equip_display_fstrs(name_box, &tab_fstrs);
                                                }
                                            }
                                            if(tab_is_selected && panel_tree.focused == panel)
                                            {
                                                UI_PrefWidth(ui_px(tab_close_width_px, 1.f))
                                                    UI_TextAlignment(UI_TextAlign_Center)
                                                    BP_Font(BP_FontSlot_Icons)
                                                    UI_FontSize(ui_top_font_size()*0.75f)
                                                    UI_TagF(".") UI_TagF("tab") UI_TagF("weak") UI_TagF("implicit")
                                                    UI_VisualMarginX(ceil_f32(ui_top_font_size()*0.5f))
                                                    UI_VisualMarginY(floor_f32(tab_bar_vheight - tab_close_width_px + ui_top_font_size()*0.25f))
                                                    UI_CornerRadius(ui_top_font_size()*1.f)
                                                {
                                                    UI_Box *edit_box = ui_build_box_from_stringf(UI_BoxFlag_Clickable|
                                                                                                 UI_BoxFlag_DrawBorder|
                                                                                                 UI_BoxFlag_DrawBackground|
                                                                                                 UI_BoxFlag_DrawText|
                                                                                                 UI_BoxFlag_DrawHotEffects|
                                                                                                 UI_BoxFlag_DrawActiveEffects,
                                                                                                 "%S###edit_view_%p", bp_icon_kind_text_table[BP_IconKind_Gear], tab);
                                                    UI_Signal sig = ui_signal_from_box(edit_box);
                                                    if(ui_pressed(sig))
                                                    {
                                                        if(ws->query_is_active &&
                                                           ui_key_match(sig.box->key, ws->query_regs->ui_key))
                                                        {
                                                            bp_cmd(BP_CmdKind_CancelQuery);
                                                        }
                                                        else
                                                        {
                                                            bp_cmd(BP_CmdKind_PushQuery,
                                                                   .ui_key       = sig.box->key,
                                                                   .expr         = push_str8f(scratch.arena, "query:config.$%I64x", tab->id));
                                                        }
                                                    }
                                                }
                                            }
                                            UI_PrefWidth(ui_px(tab_close_width_px, 1.f))
                                                UI_TextAlignment(UI_TextAlign_Center)
                                                BP_Font(BP_FontSlot_Icons)
                                                UI_FontSize(ui_top_font_size()*0.75f)
                                                UI_TagF(".") UI_TagF("tab") UI_TagF("weak") UI_TagF("implicit")
                                                UI_VisualMarginX(ceil_f32(ui_top_font_size()*0.5f))
                                                UI_VisualMarginY(floor_f32(tab_bar_vheight - tab_close_width_px + ui_top_font_size()*0.25f))
                                                UI_CornerRadius(ui_top_font_size()*1.f)
                                            {
                                                UI_Box *close_box = ui_build_box_from_stringf(UI_BoxFlag_Clickable|
                                                                                              UI_BoxFlag_DrawBorder|
                                                                                              UI_BoxFlag_DrawBackground|
                                                                                              UI_BoxFlag_DrawText|
                                                                                              UI_BoxFlag_DrawHotEffects|
                                                                                              UI_BoxFlag_DrawActiveEffects,
                                                                                              "%S###close_view_%p", bp_icon_kind_text_table[BP_IconKind_X], tab);
                                                UI_Signal sig = ui_signal_from_box(close_box);
                                                if(ui_clicked(sig) || ui_middle_clicked(sig))
                                                {
                                                    bp_cmd(BP_CmdKind_CloseTab);
                                                }
                                            }
                                        }
                                        
                                        {
                                            UI_Signal sig = ui_signal_from_box(tab_box);
                                            if(ui_pressed(sig))
                                            {
                                                bp_cmd(BP_CmdKind_FocusTab);
                                                bp_cmd(BP_CmdKind_FocusPanel);
                                            }
                                            else if(ui_dragging(sig) && !bp_drag_is_active() && length_2f32(ui_drag_delta()) > 10.f)
                                            {
                                                bp_drag_begin(BP_RegSlot_View);
                                            }
                                            else if(ui_right_clicked(sig))
                                            {
                                                bp_cmd(BP_CmdKind_PushQuery,
                                                       .ui_key       = sig.box->key,
                                                       .expr         = push_str8f(scratch.arena, "query:config.$%I64x", tab->id));
                                            }
                                            else if(ui_middle_clicked(sig))
                                            {
                                                bp_cmd(BP_CmdKind_CloseTab);
                                            }
                                        }
                                    }
                                    
                                    {
                                        ui_spacer(ui_px(floor_f32(ui_top_font_size()*0.4f), 1.f));
                                    }
                                }
                                
                                if(tab_drop_is_active &&
                                   bp_drag_is_active() &&
                                   bp_state->drag_drop_regs_slot == BP_RegSlot_View &&
                                   tab == tab_drop_prev)
                                {
                                    ui_set_next_child_layout_axis(Axis2_Y);
                                    ui_set_next_pref_width(ui_px(ui_top_font_size()*4.f, 1));
                                    UI_Box *tab_column_box = ui_build_box_from_stringf(!is_changing_panel_boundaries*UI_BoxFlag_AnimatePosX, "tab_column_%p", tab);
                                    
                                    UI_Parent(tab_column_box)
                                        UI_PrefHeight(ui_px(tab_bar_vheight, 1))
                                        UI_TagF("hollow")
                                    {
                                        if(panel->tab_side == Side_Max)
                                        {
                                            ui_spacer(ui_px(tab_bar_rv_diff-1.f, 1.f));
                                        }
                                        else
                                        {
                                            ui_spacer(ui_px(1.f, 1.f));
                                        }
                                        ui_set_next_group_key(catchall_drop_site_key);
                                        UI_Box *tab_box = ui_build_box_from_key(UI_BoxFlag_DrawHotEffects|
                                                                                UI_BoxFlag_DrawBackground|
                                                                                UI_BoxFlag_DrawBorder|
                                                                                UI_BoxFlag_Clickable,
                                                                                ui_key_zero());
                                    }
                                    
                                    {
                                        ui_spacer(ui_px(floor_f32(ui_top_font_size()*0.4f), 1.f));
                                    }
                                }
                            }
                        
                        UI_TextAlignment(UI_TextAlign_Center)
                            UI_PrefWidth(ui_px(tab_bar_vheight, 1.f))
                            UI_PrefHeight(ui_px(tab_bar_vheight, 1.f))
                            UI_TagF(".")
                        {
                            ui_set_next_child_layout_axis(Axis2_Y);
                            UI_Box *container = ui_build_box_from_stringf(!is_changing_panel_boundaries*UI_BoxFlag_AnimatePosX, "###add_new_tab");
                            UI_Parent(container)
                            {
                                if(panel->tab_side == Side_Max)
                                {
                                    ui_spacer(ui_px(tab_bar_rv_diff-1.f, 1.f));
                                }
                                else
                                {
                                    ui_spacer(ui_px(1.f, 1.f));
                                }
                                {
                                    UI_Box *add_new_box = &ui_nil_box;
                                    BP_Font(BP_FontSlot_Icons)
                                        UI_CornerRadius((tab_bar_vheight - tab_bar_vheight/4.f) / 3.f)
                                        UI_VisualMargin(tab_bar_vheight/4.f)
                                        UI_TagF("implicit")
                                        UI_TagF("weak")
                                    {
                                        add_new_box = ui_build_box_from_stringf(UI_BoxFlag_DrawText|
                                                                                UI_BoxFlag_DrawBorder|
                                                                                UI_BoxFlag_DrawBackground|
                                                                                UI_BoxFlag_DrawHotEffects|
                                                                                UI_BoxFlag_DrawActiveEffects|
                                                                                UI_BoxFlag_Clickable|
                                                                                UI_BoxFlag_DisableTextTrunc,
                                                                                "%S##add_new_tab_button_%p",
                                                                                bp_icon_kind_text_table[BP_IconKind_Add],
                                                                                panel->cfg);
                                    }
                                    UI_Signal sig = ui_signal_from_box(add_new_box);
                                    if(ui_pressed(sig))
                                    {
                                        bp_cmd(BP_CmdKind_FocusPanel, .panel = panel->cfg->id);
                                        if(ws->query_is_active &&
                                           ui_key_match(add_new_box->key, ws->query_regs->ui_key))
                                        {
                                            bp_cmd(BP_CmdKind_CancelQuery);
                                        }
                                        else
                                        {
                                            bp_cmd(BP_CmdKind_RunCommand, .cmd_name = bp_cmd_kind_info_table[BP_CmdKind_Open].string);
                                        }
                                    }
                                    if(ui_hovering(sig)) UI_Tooltip
                                    {
                                        ui_state->tooltip_anchor_key = add_new_box->key;
                                        ui_set_next_pref_width(ui_children_sum(1));
                                        UI_Row
                                        {
                                            ui_labelf("Open File");
                                            bp_cmd_binding_buttons(bp_cmd_kind_info_table[BP_CmdKind_Open].string, s(""), 1, BP_CmdBindingButtonFlag_NoEdit);
                                        }
                                    }
                                }
                            }
                        }
                        
                        ui_signal_from_box(tab_bar_box);
                    }
                    
                    //////////////////////////
                    //- rjf: accept tab drops
                    //
                    if(tab_drop_is_active && bp_drag_drop() && bp_state->drag_drop_regs_slot == BP_RegSlot_View)
                    {
                        bp_cmd(BP_CmdKind_MoveView,
                               .dst_panel = panel->cfg->id,
                               .panel     = bp_state->drag_drop_regs->panel,
                               .view     = bp_state->drag_drop_regs->view,
                               .prev_tab  = tab_drop_prev->id);
                    }
                    
                    //////////////////////////
                    //- rjf: accept file drops
                    //
                    {
                        for(UI_Event *evt = 0; ui_next_event(&evt);)
                        {
                            if(evt->kind == UI_EventKind_FileDrop && contains_2f32(content_rect, evt->pos))
                            {
                                arena_clear(ws->drop_completion_arena);
                                ws->top_drop_completion_task = 0;
                                ws->drop_completion_panel = panel->cfg->id;
                                String8_List cfg_paths = {0};
                                for(String8_Node *n = evt->paths.first; n != 0; n = n->next)
                                {
                                    Temp scratch = scratch_begin(0, 0);
                                    String8 path = n->string;
                                    bool32 looks_like_our_cfg = 0;
                                    {
                                        File file = file_open(AccessFlag_Read|AccessFlag_ShareRead, path);
                                        u8 app_cfg_magic[] = "// raddbg ";
                                        u8 file_magic_maybe[ArrayCount(app_cfg_magic)] = {0};
                                        file_read(file, r1u64(0, 10), file_magic_maybe);
                                        looks_like_our_cfg = MemoryMatchArray(app_cfg_magic, file_magic_maybe);
                                        file_close(file);
                                    }
                                    if(looks_like_our_cfg)
                                    {
                                        str8_list_push(ws->drop_completion_arena, &cfg_paths, str8_copy(ws->drop_completion_arena, path));
                                    }
                                    else
                                    {
                                        bp_cmd(BP_CmdKind_Open, .file_path = path, .panel = panel->cfg->id);
                                    }
                                    scratch_end(scratch);
                                }
                                if(cfg_paths.node_count != 0)
                                {
                                    BP_Drop_Completion_Task *t = push_array(ws->drop_completion_arena, BP_Drop_Completion_Task, 1);
                                    SLLStackPush(ws->top_drop_completion_task, t);
                                    t->cfg = 1;
                                    t->paths = cfg_paths;
                                }
                                if(ws->top_drop_completion_task != 0)
                                {
                                    ui_ctx_menu_open(bp_state->drop_completion_key, ui_key_zero(), evt->pos);
                                }
                                ui_eat_event(evt);
                            }
                        }
                    }
                }
            }
        }
    
        ////////////////////////////
        //- rjf: @window_ui_part drag/drop cancelling
        //
        if(bp_drag_is_active() && ui_slot_press(UI_EventActionSlot_Cancel))
        {
            bp_drag_kill();
            ui_kill_action();
        }
    
        ////////////////////////////
        //- rjf: @window_ui_part top-level font size changing
        //
        for(UI_Event *evt = 0; ui_next_event(&evt);)
        {
            if(evt->kind == UI_EventKind_Scroll && evt->modifiers == WM_Modifier_Ctrl)
            {
                ui_eat_event(evt);
                if(evt->delta_2f32.y < 0)
                {
                    bp_cmd(BP_CmdKind_IncWindowFontSize);
                }
                else if(evt->delta_2f32.y > 0)
                {
                    bp_cmd(BP_CmdKind_DecWindowFontSize);
                }
            }
        }
    
        ui_end_build();
    }
  
    //////////////////////////////
    //- rjf: @window_frame_part animate
    //
    if(ui_animating_from_state(ws->ui))
    {
        bp_request_frame();
    }
  
    //////////////////////////////
    //- rjf: @window_frame_part draw UI
    //
    ws->draw_bucket = dr_bucket_make();
    DR_BucketScope(ws->draw_bucket)
        ProfScope("draw UI")
    {
        Temp scratch = scratch_begin(0, 0);
        f32 box_squish_epsilon = 0.001f;
        Rng2f32 window_rect = wm_client_rect_from_window(ws->os);
    
        f32 rounded_corner_amount = bp_setting_f32_from_name(str8_lit("rounded_corner_amount"));
        f32 border_softness = 1.f;
        bool32 do_background_blur = bp_setting_b32_from_name(str8_lit("background_blur"));
        bool32 force_opaque_floating_backgrounds = bp_setting_b32_from_name(str8_lit("opaque_backgrounds"));
        bool32 do_drop_shadows = bp_setting_b32_from_name(str8_lit("drop_shadows"));
        Vec4f32 base_background_color = ui_color_from_name(str8_lit("background"));
        Vec4f32 base_border_color = ui_color_from_name(str8_lit("border"));
        Vec4f32 drop_shadow_color = ui_color_from_name(str8_lit("drop_shadow"));
    
        f32 heatmap_bucket_size = 32.f;
        u64 *heatmap_buckets = 0;
        u64 heatmap_bucket_pitch = 0;
        u64 heatmap_bucket_count = 0;
        if(DEV_draw_ui_box_heatmap)
        {
            Rng2f32 rect = wm_client_rect_from_window(ws->os);
            Vec2f32 size = dim_2f32(rect);
            Vec2S32 buckets_dim = {(S32)(size.x/heatmap_bucket_size), (S32)(size.y/heatmap_bucket_size)};
            heatmap_bucket_pitch = buckets_dim.x;
            heatmap_bucket_count = buckets_dim.x*buckets_dim.y;
            heatmap_buckets = push_array(scratch.arena, u64, heatmap_bucket_count);
        }
    
        {
            dr_rect(wm_client_rect_from_window(ws->os), base_background_color, 0, 0, 0);
        }
    
        {
            dr_rect(wm_client_rect_from_window(ws->os), base_border_color, 0, 1.f, border_softness*0.5f);
        }
    
        u64 total_heatmap_sum_count = 0;
        UI_Box *hover_debug_box = &ui_nil_box;
        for(UI_Box *box = ui_root_from_state(ws->ui); !ui_box_is_nil(box);)
        {
            f32 box_corner_radii[Corner_COUNT] = {
                box->corner_radii[Corner_00]*rounded_corner_amount,
                box->corner_radii[Corner_01]*rounded_corner_amount,
                box->corner_radii[Corner_10]*rounded_corner_amount,
                box->corner_radii[Corner_11]*rounded_corner_amount,
            };
            
            UI_BoxRec rec = ui_box_rec_df_post(box, &ui_nil_box);
      
            if(DEV_draw_ui_box_heatmap)
            {
                Vec2f32 center = center_2f32(box->rect);
                Vec2S32 p = v2s32(center.x / heatmap_bucket_size, center.y / heatmap_bucket_size);
                u64 bucket_idx = p.y * heatmap_bucket_pitch + p.x;
                if(bucket_idx < heatmap_bucket_count)
                {
                    heatmap_buckets[bucket_idx] += 1;
                    total_heatmap_sum_count += 1;
                }
            }
      
            if(box->flags & UI_BoxFlag_Debug && contains_2f32(box->rect, ui_mouse()))
            {
                hover_debug_box = box;
            }
      
            if(box->transparency != 0)
            {
                dr_push_transparency(box->transparency);
            }
      
            if(box->squish > box_squish_epsilon)
            {
                Vec2f32 box_dim = dim_2f32(box->rect);
                Vec2f32 anchor_off = {0};
                if(box->flags & UI_BoxFlag_SquishAnchored)
                {
                    anchor_off.x = box_dim.x/2.f;
                }
                else
                {
                    anchor_off.y = -box_dim.y/8.f;
                }
                Mat3x3f32 box2origin_xform = make_translate_3x3f32(v2f32(-box->rect.x0 - box_dim.x/2 + anchor_off.x, -box->rect.y0 + anchor_off.y));
                Mat3x3f32 scale_xform = make_scale_3x3f32(v2f32(1-box->squish, 1-box->squish));
                Mat3x3f32 origin2box_xform = make_translate_3x3f32(v2f32(box->rect.x0 + box_dim.x/2 - anchor_off.x, box->rect.y0 - anchor_off.y));
                Mat3x3f32 xform = mul_3x3f32(origin2box_xform, mul_3x3f32(scale_xform, box2origin_xform));
                dr_push_xform2d(xform);
                dr_push_tex2d_sample_kind(R_Tex2DSampleKind_Linear);
            }
      
            if(do_drop_shadows && box->flags & UI_BoxFlag_DrawDropShadow)
            {
                Rng2f32 drop_shadow_rect = shift_2f32(pad_2f32(box->rect, 8), v2f32(4, 4));
                R_Rect2D_Inst *inst = dr_rect(drop_shadow_rect, drop_shadow_color, 0.8f, 0, 8.f);
                MemoryCopyArray(inst->corner_radii, box_corner_radii);
            }
      
            if(do_background_blur && box->flags & UI_BoxFlag_DrawBackgroundBlur)
            {
                R_PassParams_Blur *params = dr_blur(pad_2f32(box->rect, 1.f), box->blur_size*(1-box->transparency), 0);
                MemoryCopyArray(params->corner_radii, box_corner_radii);
            }
      
            f32 effective_active_t = box->active_t;
            if(!(box->flags & UI_BoxFlag_DrawActiveEffects))
            {
                effective_active_t = 0;
            }
            f32 t = box->hot_t*(1-effective_active_t);
      
            Vec4f32 box_background_color = box->background_color;
            if(force_opaque_floating_backgrounds && box->flags & UI_BoxFlag_Floating && box->flags & UI_BoxFlag_DrawDropShadow)
            {
                box_background_color.w = 1.f;
            }
      
            if(box->flags & UI_BoxFlag_DrawBackground)
            {
                Rng2f32 box_bg_rect = r2f32p(box->rect.x0 + box->visual_margin.x,
                                             box->rect.y0 + box->visual_margin.y,
                                             box->rect.x1 - box->visual_margin.x,
                                             box->rect.y1 - box->visual_margin.y);
        
                if(box->flags & UI_BoxFlag_DrawHotEffects)
                {
                    Rng2f32 drop_shadow_rect = shift_2f32(pad_2f32(box_bg_rect, 8), v2f32(4, 4));
                    Vec4f32 color = drop_shadow_color;
                    color.w *= t*box_background_color.w;
                    dr_rect(drop_shadow_rect, color, 0.8f, 0, 8.f);
                }
        
                R_Rect2D_Inst *inst = dr_rect(pad_2f32(box_bg_rect, 1.f), box_background_color, 0, 0, border_softness*1.f);
                MemoryCopyArray(inst->corner_radii, box_corner_radii);
        
                if(box->flags & UI_BoxFlag_DrawHotEffects)
                {
                    bool32 is_hot = !ui_key_match(box->key, ui_key_zero()) && ui_key_match(box->key, ui_hot_key());
                    Vec4f32 hover_color = ui_color_from_tags_key_name(box->tags_key, str8_lit("hover"));
          
                    if(is_hot)
                    {
                        Vec4f32 color = hover_color;
                        color.w *= 0.015f;
                        R_Rect2D_Inst *inst = dr_rect(pad_2f32(box_bg_rect, 1.f), v4f32(0, 0, 0, 0), 0, 0, border_softness*1.f);
                        inst->colors[Corner_00] = color;
                        inst->colors[Corner_10] = color;
                        inst->colors[Corner_01] = color;
                        inst->colors[Corner_11] = color;
                        MemoryCopyArray(inst->corner_radii, box_corner_radii);
                    }
          
                    if(box->hot_t > 0.01f && dim_2f32(box->rect).x > box->font_size*8.f) DR_ClipScope(intersect_2f32(box_bg_rect, dr_top_clip()))
                    {
                        Vec4f32 color = hover_color;
                        color.w *= 0.025f;
                        if(!is_hot)
                        {
                            color.w *= t;
                        }
                        Vec2f32 center = ui_mouse();
                        Vec2f32 box_dim = dim_2f32(box->rect);
                        f32 max_dim = Max(box_dim.x, box_dim.y);
                        f32 radius = box->font_size*24.f;
                        radius = Min(max_dim, radius);
                        dr_rect(pad_2f32(r2f32(center, center), radius), color, radius, 0, radius/3.f);
                    }
                }
        
                if(box->flags & UI_BoxFlag_DrawActiveEffects)
                {
                    Vec4f32 shadow_color = drop_shadow_color;
                    shadow_color.w *= 0.5f*box->active_t;
                    Vec2f32 shadow_size = {
                        (box_bg_rect.x1 - box_bg_rect.x0)*0.60f*box->active_t,
                        (box_bg_rect.y1 - box_bg_rect.y0)*0.60f*box->active_t,
                    };
                    shadow_size.x = Clamp(0, shadow_size.x, box->font_size*2.f);
                    shadow_size.y = Clamp(0, shadow_size.y, box->font_size*2.f);
          
                    {
                        R_Rect2D_Inst *inst = dr_rect(r2f32p(box_bg_rect.x0, box_bg_rect.y0, box_bg_rect.x1, box_bg_rect.y0 + shadow_size.y), v4f32(0, 0, 0, 0), 0, 0, 1.f);
                        inst->colors[Corner_00] = inst->colors[Corner_10] = shadow_color;
                        inst->colors[Corner_01] = inst->colors[Corner_11] = v4f32(0.f, 0.f, 0.f, 0.0f);
                        MemoryCopyArray(inst->corner_radii, box_corner_radii);
                    }
          
                    {
                        R_Rect2D_Inst *inst = dr_rect(r2f32p(box_bg_rect.x0, box_bg_rect.y1 - shadow_size.y, box_bg_rect.x1, box_bg_rect.y1), v4f32(0, 0, 0, 0), 0, 0, 1.f);
                        inst->colors[Corner_00] = inst->colors[Corner_10] = v4f32(0, 0, 0, 0);
                        inst->colors[Corner_01] = inst->colors[Corner_11] = v4f32(1.0f, 1.0f, 1.0f, 0.08f*box->active_t);
                        MemoryCopyArray(inst->corner_radii, box_corner_radii);
                    }
          
                    {
                        R_Rect2D_Inst *inst = dr_rect(r2f32p(box_bg_rect.x0, box_bg_rect.y0, box_bg_rect.x0 + shadow_size.x, box_bg_rect.y1), v4f32(0, 0, 0, 0), 0, 0, 1.f);
                        inst->colors[Corner_10] = inst->colors[Corner_11] = v4f32(0.f, 0.f, 0.f, 0.f);
                        inst->colors[Corner_00] = shadow_color;
                        inst->colors[Corner_01] = shadow_color;
                        MemoryCopyArray(inst->corner_radii, box_corner_radii);
                    }
          
                    {
                        R_Rect2D_Inst *inst = dr_rect(r2f32p(box_bg_rect.x1 - shadow_size.x, box_bg_rect.y0, box_bg_rect.x1, box_bg_rect.y1), v4f32(0, 0, 0, 0), 0, 0, 1.f);
                        inst->colors[Corner_00] = inst->colors[Corner_01] = v4f32(0.f, 0.f, 0.f, 0.f);
                        inst->colors[Corner_10] = shadow_color;
                        inst->colors[Corner_11] = shadow_color;
                        MemoryCopyArray(inst->corner_radii, box_corner_radii);
                    }
                }
            }
      
            if(box->flags & UI_BoxFlag_DrawText)
            {
                Vec2f32 text_position = ui_box_text_position(box);
                if(DEV_draw_ui_text_pos)
                {
                    dr_rect(r2f32p(text_position.x-4, text_position.y-4, text_position.x+4, text_position.y+4),
                            v4f32(1, 0, 1, 1), 1, 0, 1);
                }
                f32 max_x = 100000.f;
                FNT_Run ellipses_run = {0};
                if(!(box->flags & UI_BoxFlag_DisableTextTrunc))
                {
                    FNT_Tag ellipses_font = box->font;
                    f32 ellipses_size = box->font_size;
                    FNT_RasterFlags ellipses_raster_flags = box->text_raster_flags;
                    if(box->display_fstrs.last)
                    {
                        ellipses_font = box->display_fstrs.last->v.params.font;
                        ellipses_size = box->display_fstrs.last->v.params.size;
                        ellipses_raster_flags = box->display_fstrs.last->v.params.raster_flags;
                    }
                    max_x = (box->rect.x1-text_position.x);
                    ellipses_run = fnt_run_from_string(ellipses_font, ellipses_size, 0, box->tab_size, ellipses_raster_flags, str8_lit("..."));
                }
                if(box->flags & UI_BoxFlag_HasFuzzyMatchRanges) UI_TagF("match")
                {
                    Vec4f32 match_color = ui_color_from_tags_key_name(ui_top_tags_key(), str8_lit("background"));
                    dr_truncated_fancy_run_fuzzy_matches(text_position, &box->display_fruns, max_x, &box->fuzzy_match_ranges, match_color);
                }
                dr_truncated_fancy_run_list(text_position, &box->display_fruns, max_x, ellipses_run);
            }
      
            if(DEV_draw_ui_focus_debug)
            {
                bool32 focused = (box->flags & (UI_BoxFlag_FocusHot|UI_BoxFlag_FocusActive) &&
                               box->flags & UI_BoxFlag_Clickable);
                bool32 disabled = 0;
                for(UI_Box *p = box; !ui_box_is_nil(p); p = p->parent)
                {
                    if(p->flags & (UI_BoxFlag_FocusHotDisabled|UI_BoxFlag_FocusActiveDisabled))
                    {
                        disabled = 1;
                        break;
                    }
                }
                if(focused)
                {
                    Vec4f32 color = v4f32(0.3f, 0.8f, 0.3f, 1.f);
                    if(disabled)
                    {
                        color = v4f32(0.8f, 0.3f, 0.3f, 1.f);
                    }
                    dr_rect(r2f32p(box->rect.x0-6, box->rect.y0-6, box->rect.x0+6, box->rect.y0+6), color, 2, 0, 1);
                    dr_rect(box->rect, color, 2, 2, 1);
                }
                if(box->flags & (UI_BoxFlag_FocusHot|UI_BoxFlag_FocusActive))
                {
                    if(box->flags & (UI_BoxFlag_FocusHotDisabled|UI_BoxFlag_FocusActiveDisabled))
                    {
                        dr_rect(r2f32p(box->rect.x0-6, box->rect.y0-6, box->rect.x0+6, box->rect.y0+6), v4f32(1, 0, 0, 0.2f), 2, 0, 1);
                    }
                    else
                    {
                        dr_rect(r2f32p(box->rect.x0-6, box->rect.y0-6, box->rect.x0+6, box->rect.y0+6), v4f32(0, 1, 0, 0.2f), 2, 0, 1);
                    }
                }
            }
      
            if(box->flags & UI_BoxFlag_Clip)
            {
                Rng2f32 top_clip = dr_top_clip();
                Rng2f32 new_clip = pad_2f32(box->rect, -1);
                if(top_clip.x1 != 0 || top_clip.y1 != 0)
                {
                    new_clip = intersect_2f32(new_clip, top_clip);
                }
                dr_push_clip(new_clip);
            }
      
            if(box->flags & UI_BoxFlag_DrawBucket)
            {
                Mat3x3f32 xform = make_translate_3x3f32(box->position_delta);
                DR_XForm2DScope(xform)
                {
                    dr_sub_bucket(box->draw_bucket);
                }
            }
      
            if(box->custom_draw != 0)
            {
                box->custom_draw(box, box->custom_draw_user_data);
            }
      
            {
                S32 pop_idx = 0;
                for(UI_Box *b = box; !ui_box_is_nil(b) && pop_idx <= rec.pop_count; b = b->parent)
                {
                    pop_idx += 1;
                    if(b == box && rec.push_count != 0)
                    {
                        continue;
                    }
          
                    if(b->flags & UI_BoxFlag_Clip)
                    {
                        dr_pop_clip();
                    }
          
                    f32 b_corner_radii[Corner_COUNT] =
                        {
                            b->corner_radii[Corner_00]*rounded_corner_amount,
                            b->corner_radii[Corner_01]*rounded_corner_amount,
                            b->corner_radii[Corner_10]*rounded_corner_amount,
                            b->corner_radii[Corner_11]*rounded_corner_amount,
                        };
          
                    if(b->flags & UI_BoxFlag_DrawBorder)
                    {
                        Vec4f32 border_color = b->border_color;
                        Rng2f32 b_border_rect = r2f32p(b->rect.x0 - 1.f + b->visual_margin.x,
                                                       b->rect.y0 - 1.f + b->visual_margin.y,
                                                       b->rect.x1 + 1.f - b->visual_margin.x,
                                                       b->rect.y1 + 1.f - b->visual_margin.y);
                        R_Rect2D_Inst *inst = dr_rect(b_border_rect, border_color, 0, 1.f, border_softness*1.f);
                        MemoryCopyArray(inst->corner_radii, b_corner_radii);
            
                        if(b->flags & UI_BoxFlag_DrawHotEffects)
                        {
                            Vec4f32 color = ui_color_from_tags_key_name(box->tags_key, str8_lit("hover"));
                            if(ui_key_match(b->key, ui_key_zero()) || !ui_key_match(b->key, ui_hot_key()))
                            {
                                color.w *= b->hot_t;
                            }
                            color.w *= 0.01f;
                            R_Rect2D_Inst *inst = dr_rect(b_border_rect, color, 0, 1.f, 1.f);
                            MemoryCopyArray(inst->corner_radii, b_corner_radii);
                        }
                    }
          
                    if(b->flags & (UI_BoxFlag_DrawFadeTop|UI_BoxFlag_DrawFadeBottom|UI_BoxFlag_DrawFadeLeft|UI_BoxFlag_DrawFadeRight))
                    {
                        Vec2f32 fade_dim = scale_2f32(dim_2f32(b->rect), 0.05f);
                        Vec4f32 fade_color = drop_shadow_color;
                        if(b->flags & UI_BoxFlag_DrawFadeTop)
                        {
                            f32 t = ui_anim(ui_key_from_string(b->key, s("fade_top")), 1.f, .rate = bp_state->catchall_animation_rate);
                            Rng2f32 rect = r2f32p(b->rect.x0, b->rect.y0, b->rect.x1, b->rect.y0 + fade_dim.y*t);
                            R_Rect2D_Inst *r = dr_rect(rect, fade_color, 0, 0, 0);
                            MemoryCopyArray(r->corner_radii, b_corner_radii);
                            r->colors[Corner_01] = r->colors[Corner_11] = v4f32(0, 0, 0, 0);
                        }
                        if(b->flags & UI_BoxFlag_DrawFadeBottom)
                        {
                            f32 t = ui_anim(ui_key_from_string(b->key, s("fade_bottom")), 1.f, .rate = bp_state->catchall_animation_rate);
                            Rng2f32 rect = r2f32p(b->rect.x0, b->rect.y1 - fade_dim.y*t, b->rect.x1, b->rect.y1);
                            R_Rect2D_Inst *r = dr_rect(rect, fade_color, 0, 0, 0);
                            MemoryCopyArray(r->corner_radii, b_corner_radii);
                            r->colors[Corner_00] = r->colors[Corner_10] = v4f32(0, 0, 0, 0);
                        }
                        if(b->flags & UI_BoxFlag_DrawFadeLeft)
                        {
                            f32 t = ui_anim(ui_key_from_string(b->key, s("fade_left")), 1.f, .rate = bp_state->catchall_animation_rate);
                            Rng2f32 rect = r2f32p(b->rect.x0, b->rect.y0, b->rect.x1+fade_dim.x*t, b->rect.y1);
                            R_Rect2D_Inst *r = dr_rect(rect, fade_color, 0, 0, 0);
                            MemoryCopyArray(r->corner_radii, b_corner_radii);
                            r->colors[Corner_11] = r->colors[Corner_10] = v4f32(0, 0, 0, 0);
                        }
                        if(b->flags & UI_BoxFlag_DrawFadeRight)
                        {
                            f32 t = ui_anim(ui_key_from_string(b->key, s("fade_right")), 1.f, .rate = bp_state->catchall_animation_rate);
                            Rng2f32 rect = r2f32p(b->rect.x1 - fade_dim.x*t, b->rect.y0, b->rect.x1, b->rect.y1);
                            R_Rect2D_Inst *r = dr_rect(rect, fade_color, 0, 0, 0);
                            MemoryCopyArray(r->corner_radii, b_corner_radii);
                            r->colors[Corner_00] = r->colors[Corner_01] = v4f32(0, 0, 0, 0);
                        }
                    }
          
                    if(b->flags & UI_BoxFlag_Debug)
                    {
                        R_Rect2D_Inst *inst = dr_rect(b->rect, v4f32(1*box->pref_size[Axis2_X].strictness, 0, 1, 0.25f), 0, 1.f, 0);
                        MemoryCopyArray(inst->corner_radii, b_corner_radii);
                    }
          
                    if(b->flags & (UI_BoxFlag_DrawSideTop|UI_BoxFlag_DrawSideBottom|UI_BoxFlag_DrawSideLeft|UI_BoxFlag_DrawSideRight))
                    {
                        Vec4f32 border_color = b->border_color;
                        Rng2f32 r = b->rect;
                        f32 half_thickness = 1.f;
                        f32 softness = 0.f;
                        if(b->flags & UI_BoxFlag_DrawSideTop)
                        {
                            dr_rect(r2f32p(r.x0, r.y0, r.x1, r.y0+2*half_thickness), border_color, 0, 0, softness);
                        }
                        if(b->flags & UI_BoxFlag_DrawSideBottom)
                        {
                            dr_rect(r2f32p(r.x0, r.y1-2*half_thickness, r.x1, r.y1), border_color, 0, 0, softness);
                        }
                        if(b->flags & UI_BoxFlag_DrawSideLeft)
                        {
                            dr_rect(r2f32p(r.x0, r.y0, r.x0+2*half_thickness, r.y1), border_color, 0, 0, softness);
                        }
                        if(b->flags & UI_BoxFlag_DrawSideRight)
                        {
                            dr_rect(r2f32p(r.x1-2*half_thickness, r.y0, r.x1, r.y1), border_color, 0, 0, softness);
                        }
                    }
          
                    if(b->flags & UI_BoxFlag_Clickable && !(b->flags & UI_BoxFlag_DisableFocusOverlay) && b->focus_hot_t > 0.01f)
                    {
                        String8 extras[] = {str8_lit("focus"), str8_lit("overlay")};
                        String8_Array extras_array = {extras, ArrayCount(extras)};
                        Vec4f32 color = ui_color_from_tags_key_extras(b->tags_key, extras_array);
                        color.w *= b->focus_hot_t;
                        R_Rect2D_Inst *inst = dr_rect(b->rect, color, 0, 0, 0.f);
                        MemoryCopyArray(inst->corner_radii, b_corner_radii);
                    }
          
                    if(b->flags & UI_BoxFlag_Clickable && !(b->flags & UI_BoxFlag_DisableFocusBorder) && b->focus_active_t > 0.01f)
                    {
                        Rng2f32 rect = b->rect;
                        if(b->flags & UI_BoxFlag_Floating)
                        {
                            rect = pad_2f32(rect, 1.f);
                            rect = intersect_2f32(window_rect, rect);
                        }
                        String8 extras[] = {str8_lit("focus"), str8_lit("border")};
                        String8_Array extras_array = {extras, ArrayCount(extras)};
                        Vec4f32 color = ui_color_from_tags_key_extras(b->tags_key, extras_array);
                        color.w *= b->focus_active_t;
                        R_Rect2D_Inst *inst = dr_rect(rect, color, 0, 1.f, border_softness*1.f);
                        MemoryCopyArray(inst->corner_radii, b_corner_radii);
                    }
          
                    if(b->disabled_t >= 0.005f)
                    {
                        Vec4f32 disabled_overlay_color = v4f32(base_background_color.x, base_background_color.y, base_background_color.z, b->disabled_t*0.3f);
                        R_Rect2D_Inst *inst = dr_rect(b->rect, disabled_overlay_color, 0, 0, 1);
                        MemoryCopyArray(inst->corner_radii, b_corner_radii);
                    }
          
                    if(b->squish > box_squish_epsilon)
                    {
                        dr_pop_xform2d();
                        dr_pop_tex2d_sample_kind();
                    }
          
                    if(b->transparency != 0)
                    {
                        dr_pop_transparency();
                    }
                }
            }
      
            box = rec.next;
        }
    
        if(DEV_draw_ui_box_heatmap)
        {
            u64 uniform_dist_count = total_heatmap_sum_count / heatmap_bucket_count;
            uniform_dist_count = ClampBot(uniform_dist_count, 10);
            for(u64 bucket_idx = 0; bucket_idx < heatmap_bucket_count; bucket_idx += 1)
            {
                u64 x = bucket_idx % heatmap_bucket_pitch;
                u64 y = bucket_idx / heatmap_bucket_pitch;
                u64 bucket = heatmap_buckets[bucket_idx];
                f32 pct = (f32)bucket / uniform_dist_count;
                pct = Clamp(0, pct, 1);
                Vec3f32 hsv = v3f32((1-pct) * 0.9411f, 1, 0.5f);
                Vec3f32 rgb = rgb_from_hsv(hsv);
                Rng2f32 rect = r2f32p(x*heatmap_bucket_size, y*heatmap_bucket_size, (x+1)*heatmap_bucket_size, (y+1)*heatmap_bucket_size);
                dr_rect(rect, v4f32(rgb.x, rgb.y, rgb.z, 0.3f), 0, 0, 0);
            }
        }
    
        if(hover_debug_box != &ui_nil_box)
        {
            FNT_Tag font = bp_font_from_slot(BP_FontSlot_Code);
            Vec2f32 p = ui_mouse();
            dr_rect(hover_debug_box->rect, v4f32(1, 1, 1, 0.2f), 0, 0, 0);
            R_Rect2D_Inst *border = dr_rect(hover_debug_box->rect, v4f32(1, 0, 0, 1.f), 0, 0, 0);
            MemoryCopyArray(border->corner_radii, hover_debug_box->corner_radii);
            dr_text(font, 12.f, 0, 0, FNT_RasterFlag_Hinted, p, v4f32(1, 1, 1, 1), push_str8f(scratch.arena, "key: 0x%I64x", hover_debug_box->key.u64[0]));
            p.y += 20.f;
            dr_text(font, 12.f, 0, 0, FNT_RasterFlag_Hinted, p, v4f32(1, 1, 1, 1), push_str8f(scratch.arena, "string: '%S'", hover_debug_box->string));
            p.y += 20.f;
        }
    
        if(ws->error_t > 0.01f) UI_TagF("bad")
        {
            Vec4f32 color = ui_color_from_name(str8_lit("text"));
            color.w *= ws->error_t;
            Rng2f32 rect = wm_client_rect_from_window(ws->os);
            dr_rect(pad_2f32(rect, 24.f), color, 0, 16.f, 12.f);
            dr_rect(rect, v4f32(color.x, color.y, color.z, color.w*0.025f), 0, 0, 0);
        }
        
        if(bp_state->bind_change_active) UI_TagF("pop")
        {
            Vec4f32 color = ui_color_from_name(str8_lit("background"));
            Rng2f32 rect = wm_client_rect_from_window(ws->os);
            dr_rect(pad_2f32(rect, 24.f), color, 0, 16.f, 12.f);
            dr_rect(rect, v4f32(color.x, color.y, color.z, color.w*0.025f), 0, 0, 0);
        }
    
        scratch_end(scratch);
    }
  
    //////////////////////////////
    //- rjf: @window_frame_part update per-window frame counters/info
    //
    ws->frames_alive += 1;
    ws->last_window_rect = wm_client_rect_from_window(ws->os);
  
    ProfEnd();
    scratch_end(scratch);
}

#if COMPILER_MSVC && !BUILD_DEBUG
NO_OPTIMIZE_END
#endif

///////////////////////////
// Autocompletion Lister

internal void bp_set_autocomp_regs_(BP_Regs *regs)
{
}

///////////////////////////
// Colors, Fonts, Config

// colors
internal MD_Node *bp_theme_tree_from_name(Arena *arena, Access *access, String8 theme_name)
{
}

internal Vec4f32 bp_rgba_from_code_color_slot(BP_CodeColorSlot slot)
{
}

internal BP_CodeColorSlot bp_code_color_slot_from_txt_token_kind(TXT_TokenKind kind)
{
}

internal BP_CodeColorSlot bp_code_color_slot_from_txt_token_kind_lookup_string(TXT_TokenKind kind, String8 string, bool32 allow_macros, bool32 is_called)
{
}

// fonts
internal f32 bp_font_size(void)
{
}

internal FNT_Tag bp_font_from_slot(BP_FontSlot slot)
{
}

internal FNT_RasterFlags bp_raster_flags_from_slot(BP_FontSlot slot)
{
}

////////////////////////
// Vocab Info Lookups

internal BP_Vocab_Info *bp_vocab_info_from_code_name(String8 code_name)
{
}

internal BP_Vocab_Info *bp_vocab_info_from_code_name_plural(String8 code_name_plural)
{
}

///////////////////////////////
// Continuous Frame Requests

internal void bp_request_frame(void)
{
    bp_state->num_frames_requested = 4;
}

//////////////////////////
// Main State Accessors

// per-frame arena

internal Arena *bp_frame_arena(void)
{
    return bp_state->frame_arenas[bp_state->frame_index % ArrayCount(bp_state->frame_arenas)];
}


///////////////
// Registers

internal BP_Regs *bp_push_regs_(BP_Regs *regs)
{
}

internal BP_Regs *bp_pop_regs(void)
{
}

//////////////
// Commands

// name -> info
internal BP_CmdKind bp_cmd_kind_from_string(String8 string)
{
}

internal BP_Cmd_Kind_Info *bp_cmd_kind_info_from_string(String8 string)
{
}

// pushing
internal void bp_push_cmd(String8 name, BP_Regs *regs)
{
}

// iterating
internal bool32 bp_next_cmd(BP_Cmd **cmd)
{
}

internal bool32 bp_next_view_cmd(BP_Cmd **cmd)
{
}


/////////////////////////////////
// Main Layer Top-Level Calls

#if !defined(STBI_INCLUDE_STB_IMAGE_H)
# define STB_IMAGE_IMPLEMENTATION
# define STBI_ONLY_PNG
# define STBI_ONLY_BMP
# include "thibp_party/stb/stb_image.h"
#endif

internal void bp_init(Cmd_Line *cmdline)
{
    Temp scratch = scratch_begin(0, 0);
    ProfBeginFunction();
    Arena *arena = arena_alloc();
    bp_state = push_array(arena, BP_State, 1);
    bp_state->arena = arena;
    bp_state->user_path_arena    = arena_alloc();
    bp_state->project_path_arena = arena_alloc();
    bp_state->theme_path_arena   = arena_alloc();
    for (u64 idx = 0; idx < ArrayCount(bp_state->frame_arenas); idx += 1)
    {
        bp_state->frame_areans[idx] = arena_alloc();
    }
    bp_state->log = log_alloc();
    log_select(bp_state->log);
    {
        Temp scratch = scratch_begin(0, 0);
        bp_state->log_path = push_str8f(bp_state->arena, "%S/ui_thread.broken_proxy_log", g_logs_folder);
        write_data_to_file_path(bp_state->log_path, str8_zero());
        scratch_end();
    }
    bp_state->num_frames_requested = 2;
    bp_state->seconds_until_autosave = 0.5f;
    for (u64 idx = 0; idx < ArrayCount(bp_state->cmds_arenas); idx += 1)
    {
        bp_state->cmds_arenas[idx] = arena_alloc();
    }
    bp_state->cmd_output_arena = arena_alloc();
    bp_state->popup_arena      = arena_alloc();
    bp_state->ctx_menu_key        = ui_key_from_string(ui_key_zero(), str8_lit("top_level_ctx_menu"));
    bp_state->drop_completion_key = ui_key_from_string(ui_key_zero(), str8_lit("drop_completion_ctx_menu"));
    bp_state->bind_change_arena = arena_alloc();
    bp_state->drag_drop_arena   = arena_alloc();
    bp_state->drag_drop_regs    = push_array(bp_state->drag_drop_arena, BP_Regs, 1);
    bp_state->top_regs          = &bp_state->base_regs;

    // set up schemas
    {
        bp_state->cfg_schema_table = push_array(bp_state->arena, CFG_Schema_Table, 1);
        bp_state->cfg_schema_table->slots_count = 4096;
        bp_state->cfg_schema_table->slots = push_array(bp_state->arena, CFG_Schema_Node *, bp_state->cfg_schema_table->slots_count);
        for EachElement(idx, bp_name_schema_info_table)
        {
            MD_Node *schema = md_tree_from_string(bp_state->arena, bp_name_schema_info_table[idx].schema)->first;
            cfg_schema_table_insert(bp_state->arena, bp_state->cfg_schema_table, bp_name_schema_info_table[idx].name, schema);
        }
    }

    // set up theme presets
    {
        for EachEnumVal(BP_ThemePreset, p)
        {
            bp_state->theme_preset_trees[p] = md_tree_from_string(bp_state->arena, bp_theme_preset_cfg_string_table[p])->first;
        }
    }

    // set up vocab info map
    {
        bp_state->vocab_info_map.single_slots_count = 1024;
        bp_state->vocab_info_map.single_slots = push_array(bp_state->arena, BP_Vocab_Info_Map_Slot, bp_state->vocab_info_map.single_slots_count);
        bp_state->vocab_info_map.plural_slots_count = 1024;
        bp_state->vocab_info_map.plural_slots = push_array(bp_state->arena, BP_Vocab_Info_Map_Slot, bp_state->vocab_info_map.plural_slots_count);
        for EachElement(idx, bp_vocab_info_table)
        {
            BP_Vocab_Info_Map_Node *n = push_array(bp_state->arena, BP_Vocab_Info_Map_Node, 1);
            MemoryCopyStruct(&n->v, &bp_vocab_info_table[idx]);
            u64 single_hash = d_hash_from_string(n->v.code_name);
            u64 plural_hash = d_hash_from_string(n->v.code_name_plural);
            u64 single_slot_idx = single_hash % bp_state->vocab_info_map.single_slots_count;
            u64 plural_slot_idx = plural_hash % bp_state->vocab_info_map.plural_slots_count;
            if (n->v.code_name.size != 0)
            {
                SLLQueuePush_N(bp_state->vocab_info_map.single_slots[single_slot_idx].first, bp_state->vocab_info_map.single_slots[single_slot_idx].last, n, single_next);
            }
            if (n->v.code_name_plural_size != 0)
            {
                SLLQueuePush_N(bp_state->vocab_info_map.plural_slots[plural_slot_idx].first, bp_state->vocab_info_map.single_slots[single_slot_idx].last, n, plural_next);
            }
        }
    }

    // set up top-level config entity trees & tables
    {
        bp_state->cfg = cfg_state_alloc();
        cfg_ctx_select(cfg_state_ctx(bp_state->cfg));
        cfg_node_new(bp_state->cfg, cfg_node_root(), str8_lit("user"));
        cfg_node_new(bp_state->cfg, cfg_node_root(), str8_lit("project"));
        cfg_node_new(bp_state->cfg, cfg_node_root(), str8_lit("command_line"));
        cfg_node_new(bp_state->cfg, cfg_node_root(), str8_lit("transient"));
    }

    // set up window cache
    {
        bp_state->window_state_slots_count = 64;
        bp_state->window_state_slots = push_array(arena, BP_Window_State_Slot, bp_state->window_state_slots_count);
        bp_state->first_window_state = bp_state->last_window_state = &bp_nil_window_state;
    }

    // set up view cache
    {
        bp_state->view_state_slots_count = 4096;
        bp_state->view_state_slots = push_array(arena, BP_View_State_Slot, bp_state->view_state_slots_count);
    }

    // parse command line args
    String8 implicit_user_arg = {0};
    String8 implicit_project_arg = {0};
    {
        Temp scratch2 = scratch_begin(&scratch.arena, 1);
        for (u64 idx = = 1; idx < cmdline->argc, idx += 1)
        {
            String8 arg = str8_cstring(cmdline->argv[idx]);
            bool32 is_flag = (str8_match(str8_prefix(arg, 1), str8_lit("-"), 0) ||
                              str8_match(str8_prefix(arg, 1), str8_lit("--"), 0) ||
                              str8_match(str8_prefix(arg, 1), str8_lit("/"), 0));
            if (!is_flag)
            {
                File file = file_open(AccessFlag_Read|AccessFlag_ShareRead, arg);
                u8 bp_cfg_magic[] = "// broken_proxy";
                u8 file_magic_maybe[ArrayCount(bp_cfg_magic)] = {0};
                file_read(file, r1u64(0, 10), file_magic_maybe);
                if (MemoryMatchArray(bp_cfg_magic, file_magic_maybe))
                {
                    u8 header_suffix_buffer[256] = {0};
                    String8 header_suffix = {0};
                    header_suffix.str = header_suffix_buffer;
                    header_suffix.size = file_read(file, r1u64(10, 10 + 256), header_suffix_buffer);
                    String8 header_type_suffix = str8_skip(header_suffix, str8_find_needle(header_suffix, 0, str8_lit(" "), 0) + 1);
                    if (str8_match(header_type_suffix, str8_lit("user"), StringMatchFlag_RightSideSlopp))
                    {
                        implicit_user_arg = path_absolute_dst_from_relative_dst_src(scratch.arena, arg, get_process_info()->initial_path);
                    }
                    else if (str8_match(header_type_suffix, str8_lit("project"), StringMatchFlag_RightSideSloppy))
                    {
                        implicit_project_arg = path_absolute_dst_from_relative_dst_src(scratch.arena, arg, get_process_info()->initial_path);
                    }
                }
                file_close(file);
                break; // only first non-flag arg is considered.
                // TODO: Probably want to be able to load .pcapng files.....
            }
        }
        scratch_end(scratch2);
    }

    // set up user / project paths
    {
        Temp scratch2 = scratch_begin(&scratch.arena, 1);

        // unpack commandline arguments
        String8 user_path = cmd_line_string(cmdline, str8_lit("user"));
        String8 project_path = cmd_line_string(cmdline, st8r_lit("project"));
        {
            if (user_path.size != 0)
            {
                user_path = path_absolute_dst_from_relative_dst_src(scratch2.arena, user_path, get_process_info()->initial_path);
            }
            if (project_path.size != 0)
            {
                project_path = path_absolute_dst_from_relative_dst_src(scratch2.arena, project_path, get_process_info()->initial_path);
            }
        }
        {
            String8 user_program_config_data_path = get_process_info()->user_program_config_data_path;
            String8 user_data_folder = str8f(scratch2.arena, "%S/brokenproxy", user_program_config_data_path);
            make_directory(user_data_folder);
            if (user_path.size == 0)
            {
                user_path = implicit_user_arg;
            }
            if (user_path.size == 0)
            {
                String8 last_user_path = str8f(scratch2.arena, "%S/last_user", user_data_folder);
                user_path = data_from_file_path(scracth2.arena, last_user_path);
            }
            if (user_path.size == 0)
            {
                user_path = str8f(scratch2.arena, "%S/default.broken_proxy_user", user_data_folder);
            }
        }
        if (project_path.size == 0)
        {
            project_path = implicit_project_arg;
        }
        if (project_path.size == 0)
        {
            arena_clear(bp_sate->project_path_arena);
            bp_state->project_path = push_str8_copy(bp_state->project_path_arena, project_path);
        }

        // do initial load of user/project
        bp_cmd(BP_CmdKind_OpenUser, .file_path = user_path, .non_graphical = 1);
        if (project_path.size != 0)
        {
            bp_cmd(BP_CmdKind_OpenProject, .file_path = project_path);
        }

        scratch_end(scratch2);
    }

    // unpack incon image data
    {
        Temp scratch = scratch_begin(0, 0);
        String8 data = bp_icon_file_bytes;
        u8 *ptr = data.str;
        u8 *opl = ptr + data.size;

        // read header
#pragma pack(push, 1)
        typedef struct ICO_Header ICO_Header;
        struct ICO_Header
        {
            u16 reserved_padding; // must be 0
            u16 image_type; // 1 -> ICO, 2 -> CUR
            u16 num_images;
        };
        typedef struct ICO_Entry ICO_Entry;
        struct ICO_Entry
        {
            u8 image_width_px;
            u8 image_hegiht_px;
            u8 num_colors;
            u8 reserved_padding; // should be 0
            union
            {
                u16 ico_color_planes; // in ICO
                u16 cur_hotspot_x_px; // in CUR
            };
            union
            {
                u16 ico_bits_per_pixel; // in ICO
                u16 cur_hotspot_y_px; // in CUR
            };
            u32 image_data_size;
            u32 image_data_off;
        };
#pragma pack(pop)
        ICO_Header hdr = {0};
        if (ptr + sizeof(hdr) < opl)
        {
            MemoryCopy(&hdr, ptr, sizeof(hdr));
            ptr += sizeof(hdr);
        }

        // read image entries
        u64 entries_count = hdr.num_images;
        ICO_Entry *entires = push_array(scratch.arena, ICO_Entry, hdr.num_images);
        {
            u64 bytes_to_read = sizeof(ICO_Entry) * entries_count;
            bytes_to_read = Min(bytes_to_read, opl - ptr);
            MemoryCopy(entries, ptr, bytes_to_read);
            ptr += bytes_to_read;
        }

        // find largest image
        ICO_Entry *best_entry = 0;
        u64 best_entry_area = 0;
        for (u64 idx = 0; idx < entries_count; idx += 1)
        {
            ICO_Entry *entry = &entries[idx];
            u64 width = entry->image_width_px;
            if (width == 0) { width = 256; }
            u64 height = entry->image_height_px;
            if (height == 0) { height = 256; }
            u64 entry_area = width * height;
            if (entry_area > best_entry_area)
            {
                best_entry = entry;
                best_entry_area = entry_area;
            }
        }

        // deserialise raw image data from best entry's offset
        u8 *image_data = 0;
        Vec2s32 image_dim = {0};
        if (best_entry != 0)
        {
            u8 *file_data_ptr = data.str + best_entry->image_data_off;
            u64 file_data_size = best_entry->image_data_size;
            int width = 0;
            int height = 0;
            int components = 0;
            image_data = stbi_load_from_memory(file_data_ptr, file_data_size, &width, &height, &componenets, 4);
            image_dim.x = width;
            image_dim.y = height;
        }

        // upload to gpu texture
        bp_state->icon_texture = r_tex2d_alloc(R_ResourceKind_Static, image_dim, R_Tex2DFormat_RDBA8, image_data);

        // release
        stbi_image_free(image_data);
        scratch_end(scratch);
    }

    // check initial installation status
    bp_state->installed = sh_install_or_uninstall_self(0, 0);

    ProfEnd();
    scratch_end(scratch);
}

internal void bp_frame(void)
{
    ProfBeginFunction();
    Temp scratch = scratch_begin(0, 0);
    log_scope_begin();
    bp_state->frame_depth += 1;

    /////////////////////////
    // do per frame resets
    {
        Temp scratch = scratch_begin(0, 0);
        bp_state->top_regs = &bp_state->base_regs;
        bp_regs_copy_contents(scratch.arena, &bp_state->top_regs->v, &bp_state->top_regs->v);
        arena_clear(bp_frame_arena());
        bp_regs_copy_contents(bp_frame_arena(), &bp_state->top_regs->v, &bp_state->top_regs->v);
        scratch_end(scratch);
    }
    if (bp_state->next_hover_regs != 0)
    {
        bp_state->hover_regs = bp_regs_copy(bp_frame_arena(), bp_state->next_hover_regs);
        bp_state->hover_regs_slot = bp_state->next_hover_regs_slot;
        bp_state->next_hover_regs = 0;
    }
    else
    {
        bp_state->hover_regs = push_array(bp_frame_arena(), BP_Regs, 1);
        bp_state->hover_regs_slot = BP_RegSlot_Null;
    }
    bool32 allow_text_hotkeys = !bp_state->text_edit_mode;
    bool32 allow_text_multiline_hotkeys = !bp_state->text_edit_mode_multiline;
    bp_state->text_edit_mode = false;
    bp_state->text_edit_mode_multiline = false;
    if (bp_state->frame_depth == 1)
    {
        arena_clear(bp_state->cmd_output_arena);
        MemoryZeroStruct(&bp_state->cmd_outputs);
    }
    
    ///////////////////////////////////////////////
    // Iterate all tabs, touch their view-states
    if (bp_state->frame_depth == 1)
    {
        Temp scratch = scratch_begin(0, 0);
        CFG_Node_Ptr_List windows = cfg_node_top_level_list_from_string(scratch.arena, str8_lit("window"));
        for (CFG_Node_Ptr_Node *n = windows.first; n != 0; n = n->next)
        {
            CFG_Node *window = n->v;
            CFG_Panel_Tree panel_tree = cfg_panel_tree_from_cfg(scratch.arena, window);
            for (CFG_Panel_Node *p = panel_tree.root; p != &cfg_nil_panel_node; p = cfg_panel_node_rec__depth_first_pre(panel_tree.root, p).next)
            {
                CFG_Node *first_unfiltered_tab = &cfg_nil_node;
                for (CFG_Node_Ptr_Node *n = p->tabs.first; n != 0; n = n->next)
                {
                    CFG_Node *tab = n->v;
                    if (bp_cfg_is_project_filtered(tab))
                    {
                        continue;
                    }
                    if (first_unfiltered_tab == &cfg_nil_node)
                    {
                        first_unfiltered_tab = tab;
                    }
                    bp_view_state_from_cfg(tab);
                }
                if (p->selected_tab == &cfg_nil_node && first_unfiltered_tab != &cfg_nil_node)
                {
                    bp_cmd(BP_CmdKind_FocusTab, .panel = p->cfg->id, .tab = first_unfiltered_tab->id);
                }
            }
        }
        scratch_end(scratch);
    }

    ///////////////////////////////////////////////////
    // garbage collect untouched immediate cfg trees
    if (bp_state->frame_depth == 1)
    {
        CFG_Node *transient = cfg_node_child_from_string(cfg_node_root(), str8_lit("transient"));
        for (CFG_Node *tln = transient->first, *next = &cfg_nil_node; tln != &cfg_nil_node; tln = next)
        {
            next = tln->next;
            if (str8_match(tln->string, str8_lit("immediate"), 0))
            {
                if (cfg_node_child_from_string(tln, str8_lit("hot")) == &cfg_nil_node)
                {
                    cfg_node_release(bp_state->cfg, tln);
                }
            }
        }
        for (CFG_Node *tln = transient->first; tln != &cfg_nil_node; tln = tln->next)
        {
            if (str8_match(tln->string, str8_lit("immediate"), 0))
            {
                for (CFG_Node *child = tln->first, *next = &cfg_nil_node; child != &cfg_nil_node; child = next)
                {
                    next = child->next;
                    if (str8_match(child->string, str8_lit("hot"), 0))
                    {
                        cfg_node_release(bp_state->cfg, child);
                    }
                }
            }
        }
    }

    ///////////////////////////////////////////
    // Garbage collect untouched view states
    if (bp_state->frame_depth == 1)
    {
        for EachIndex(slot_idx, bp_state->view_state_slots_count)
        {
            for (BP_View_State *vs = bp_state->view_state_slots[slot_idx].first, *next; vs != 0; vs = next)
            {
                next = vs->hash_next;
                if (vs->last_frame_index_touched + 2 < bp_state->frame_index)
                {
                    for (BP_Arena_Ext *ext = vs->first_arena_ext; ext != 0; ext = ext->next)
                    {
                        arena_release(ext->arena);
                    }
                    arena_release(vs->arena);
                    DLLRemove_NP(bp_state->view_state_slots[slot_idx].first, bp_state->view_state_slots[slot_idx].last, vs, hash_next, hash_prev);
                    SLLStackPush_N(bp_state->free_view_state, vs, hash_next);
                }
            }
        }
    }

    ///////////////////////
    // animate all views
    if (bp_state->frame_depth == 1)
    {
        bool32 any_window_is_focused = false;
        for (BP_Window_State *w = bp_state->first_window_state; w != &bp_nil_window_state; w = w->order_next)
        {
            if (wm_window_is_focused(w->os))
            {
                any_window_is_focused = true;
                break;
            }
        }

        f32 slow_rate = 1 - pow_f32(2, (-10.f * bp_state->frame_dt));
        for EachIndex(slot_idx, bp_state->view_state_slots_count)
        {
            for (BP_View_State *vs = bp_state->view_state_slots[slot_idx].first; vs != 0; vs = vs->hash_next)
            {
                f32 scroll_x_diff = (-vs->scroll_pos.x.off);
                f32 scroll_y_diff = (-vs->scroll_pos.y.off);
                f32 loading_t_diff = (vs->loading_t_target - vs->loading_t);
                vs->scroll_pos.x.off += scroll_x_diff * bp_state->scrolling_animation_rate;
                vs->scroll_pos.y.off += scroll_y_diff * bp_state->scrolling_animation_rate;
                vs->loading_t += loading_t_diff * slow_rate;
                if ((any_window_is_focused && abs_f32(loading_t_diff) > 0.01f) ||
                    abs_fs(scroll_x_diff) > 0.01f ||
                    abs_fs(scroll_y_diff) > 0.01f)
                {
                    bp_request_frame();
                }
                if (abs_f32(scroll_x_diff) <= 0.01f)
                {
                    vs->scroll_pos.x.off = 0;
                }
                if (abs_f32(scroll_y_diff) <= 0.01f)
                {
                    vs->scroll_pos.y.off = 0;
                }
                CFG_Node *vcfg = cfg_node_from_id(vs->cfg_id);
                if (cfg_node_child_from_string(vcfg, str8_lit("selected")) != &cfg_nil_node)
                {
                    if (vs->loading_t_target > 0.5f && any_window_is_focused)
                    {
                        bp_request_frame();
                    }
                    vs->loading_t_target = 0;
                }
            }
        }
    }

    ////////////////////////
    // get events from OS
    WM_Event_List events = {0};
    if (bp_state->frame_depth == 1)
    {
        events = wm_get_events(scratch.arena, bp_state->num_frames_requested == 0);
    }

    ///////////////////////
    // push frame scopes
    Access *frame_access_restore = bp_state->frame_access;
    bp_state->frame_access = access_open();

    ////////////////////////////////////////////////////
    // Calculate avg length in us of last many frames
    u64 frame_time_history_avg_us = 0;
    {
        u64 num_frames_in_history = Min(ArrayCount(bp_state->frame_time_us_history), bp_state->frame_index);
        u64 frame_time_history_sum_us = 0;
        if (num_frames_in_history > 0)
        {
            for (u64 idx = 0; idx < num_frames_in_history; idx += 1)
            {
                frame_time_history_sum_us += bp_state->frame_time_us_history[idx];
            }
            frame_time_history_avg_us = frame_time_history_sum_us / num_frames_in_history;
        }
    }

    ////////////////////
    // pick target hz
    //
    // pick among a number of sensible targets to snap to
    //
    f32 target_hz = wm_get_system_info()->default_refresh_rate;
    if (bp_state->frame_index > 32)
    {
        f32 possible_alternate_hz_targets[] = {target_hz, 60.f, 75.f, 120.f, 144.f, 165.f, 240.f, 360.f};
        f32 best_target_hz = target_hz;
        s64 best_target_hz_frame_time_us_diff = max_s64;
        for (u64 idx = 0; idx < ArrayCount(possible_alternate_hz_targets); idx += 1)
        {
            f32 candidate = possible_alternate_hz_targets[idx];
            if (candidate <= target_hz)
            {
                u64 candidate_frame_time_us = 1000000 / (u64)candidate;
                s64 frame_time_us_diff = (s64)frame_time_history_avg_us - (s64)candidate_frame_time_us;
                if (abs_s64(frame_time_us_diff) < best_target_hz_frame_time_us_diff &&
                    frame_time_history_avg_us < candidate_frame_time_us + candidate_frame_time_us / 4)
                {
                    best_target_hz = candidate;
                    best_target_hz_fame_time_us_diff = frame_time_us_diff;
                }
            }
        }
        target_hz = best_target_hz;
    }

    /////////////////////////////
    // Target HZ -> delta time
    bp_state->frame_dt = 1.f / target_hz;

    ///////////////////////////////////////////
    // begin measuring actual per-frame work
    u64 begin_time_us = now_time_us();

    /////////////////
    // bind change
    if (!bp_state->popup_active && bp_state->bind_change_active)
    {
        if (wm_key_press(&events, wm_window_zero(), 0, WM_Key_Esc))
        {
            bp_request_frame();
            bp_state->bind_change_active = 0;
        }
        if (wm_key_press(&events, wm_window_zero(), 0, WM_Key_Delete))
        {
            bp_request_frame();
            cfg_node_release(bp_state->cfg, cfg_node_from_id(bp_state->bind_change_binding_id));
            bp_state->bind_change_active = false;
        }
        for (WM_Event *event = events.first, *next = 0; event != 0; event = next)
        {
            if (event->kind == WM_EventKind_Press &&
                event->key != WM_Key_Esc &&
                event->key != WM_Key_Return &&
                event->key != WM_Key_Backspace &&
                event->key != WM_Key_Delete &&
                event->key != WM_Key_LeftMouseButton &&
                event->key != WM_Key_RightMouseButton &&
                event->key != WM_Key_MiddleMouseButton &&
                event->key != WM_Key_Ctrl &&
                event->key != WM_Key_Alt &&
                event->key != WM_Key_Shift)
            {
                bp_state->bind_change_active = false;
                CFG_Node *binding = cfg_node_from_id(bp_state->bind_change_binding_id);
                if (binding == &cfg_nil_node)
                {
                    CFG_Node *user = cfg_node_child_from_string(cfg_node_root(), str8_lit("user"));
                    CFG_Node *keybindings = cfg_node_child_from_string_or_alloc(bp_state->cfg, user, str8_lit("keybindings"));
                    binding = cfg_node_new(bp_state->cfg, keybindings, str8_lit(""));
                }
                cfg_node_release_all_children(bp_state->cfg, binding);
                cfg_node_new(bp_state->cfg, binding, bp_state->bind_change_cmd_name);
                cfg_node_new(bp_state->cfg, binding, wm_key_cfg_name_table[event->key]);
                if (event->modifiers & WM_Modifier_Ctrl)  { cfg_node_new(bp_state->cfg, binding, str8_lit("ctrl")); }
                if (event->modifiers & WM_Modifier_Shift) { cfg_node_new(bp_state->cfg, binding, str8_lit("shift")); }
                if (event->modifiers & WM_Modifier_Alt)   { cfg_node_new(bp_state->cfg, binding, str8_lit("alt")); }
                u32 codepoint = wm_codepoint_from_modifiers_and_key(event->modifiers, event->key);
                wm_text(&events, event->window, codepoint);
                wm_eat_event(&events, event);
                bp_request_frame();
                break;
            }
        }
    }

    ///////////////////////////////
    // build key map from config
    ProfScope("build key map from config")
    {
        bp_state->key_map = cfg_key_ap_from_cfg(bp_frame_arena());
    }

    ///////////////////////////
    // get fonts from config
    ProfScope("get fonts from config")
    {
        String8 main_font_name = bp_setting_from_name(str8_lit("main_font"));
        String8 code_font_name = bp_setting_from_name(str8_lit("code_font"));
        bp_state->font_slot_table[BP_FontSlot_Main] = fnt_tag_from_path(main_font_name);
        bp_state->font_slot_table[BP_FontSlot_Code] = fnt_tag_from_path(code_font_name);
        if (fnt_tag_match(bp_state->font_slot_table[BP_FontSlot_Main], fnt_tag_zero()))
        {
            bp_state->font_slot_table[BP_FontSlot_Main] = fnt_tag_from_static_data_string(&bp_default_main_font_bytes);
        }
        if (fnt_tag_match(bp_state->font_slot_table[BP_FontSlot_Code], fnt_ag_zero()))
        {
            bp_state->font_slot_table[BP_FontSlot_Code] = fnt_tag_from_static_data_string(&bp_default_code_font_bytes);
        }
        bp_state->font_slot_table[BP_FontSlot_Icons] = fnt_tag_from_static_state_string(&bp_icon_font_types);
    }

    ////////////////////
    // consume events
    ProfScope("consume events")
    {
        for (WM_Event *event = events.first, *next = 0; event != 0; event = next)
        {
            next = event->next;
            BP_Window_State *ws = bp_window_state_from_os_handle(event->window);
            if (ws != 0 && ws != bp_window_state_from_cfg(cfg_node_from_id(bp_regs()->window)))
            {
                Temp scratch = scratch_begin(0, 0);
                CFG_Panel_Tree panel_tree = cfg_panel_tree_from_cfg(scratch.arena, cfg_node_from_id(ws->cfg_id));
                bp_regs()->window = ws->cfg_id;
                bp_regs()->panel  = panel_tree.focused->cfg->id;
                bp_regs()->tab    = panel_tree.focused->selected_tab->id;
                bp_regs()->view   = panel_tree.focused->selected_tab->id;
                scratch_end(scratch);
            }
            bool32 take = false;

            // try drag/drop drop-kickoff
            if (bp_drag_is_active() && event->kind == WM_EventKind_Release && event->key == WM_Key_LeftMouseButton)
            {
                bp_state->drag_drop_state = BP_DragDropState_Dropping;
            }

            // try window close
            if (!take && event->kind == WM_EventKind_WindowClose && ws != 0)
            {
                take = true;
                bp_cmd(BP_CmdKind_Exit);
            }

            // try menu bar options
            if (bp_state->alt_menu_bar_enabled && wm_window_is_focused(ws->os))
            {
                if (!take && event->kind == WM_EventKind_Press && event->key == WM_Key_Alt && event->modifiers == 0 && event->is_repeat == 0)
                {
                    take = true;
                    bp_request_frame();
                    ws->menu_bar_focused_on_press = ws->menu_bar_focused;
                    ws->menu_bar_key_held = true;
                    ws->menu_bar_focus_press_started = true;
                }
                if (!take && event->kind == WM_EventKind_Release && event->key == WM_Key_Alt && event->modifiers == 0 && event->is_repeat == 0)
                {
                    take = true;
                    bp_request_frame();
                    ws->menu_bar_key_held = false;
                }
                if (ws->menu_bar_focused && event->kind == WM_EventKind_Press && event->key == WM_Key_Alt && event->modifiers == 0 && event->is_repeaat == 0)
                {
                    take = true;
                    bp_request_frame();
                    ws->manu_bar_focused = false;
                }
                else if (ws->manu_bar_focus_press_started && !ws->menu_bar_focused && event->kind == WM_EventKind_Release && event->modifiers == 0 && event->key == WM_Key_Alt && event->is_repeat == 0)
                {
                    take = true;
                    bp_request_frame();
                    ws->menu_bar_focused = !ws->menu_bar_focused_on_press;
                    ws->menu_bar_focus_press_started = false;
                }
                else if (event->kind == WM_EventKind_Press && event->key == WM_Key_Esc && ws->menu_bar_focused && !ui_any_ctx_menu_is_open())
                {
                    take = true;
                    bp_request_frame();
                    ws->menu_bar_focused = false;
                }
            }

            // try hotkey presses
            if (!take && event->kind == WM_EventKind_Press)
            {
                CFG_Binding binding = {event->key, event->modifiers};
                CFG_Key_Map_Node_Ptr_List key_map_nodes = cfg_key_map_node_ptr_list_from_binding(scratch.arena, bp_state->key_map, binding);
                if (key_map_nodes.first != 0)
                {
                    u32 hit_char = wm_codepoint_from_modifiers_and_key(event->modifiers, event->key);
                    if ((allow_text_hotkeys || hit_char == 0 || (hit_char == '\n' && alllow_text_multiline_hotkeys)))
                    {
                        String8 cmd_name = key_map_nodes.first->v->name;
                        for (u64 idx = 0; idx < ArrayCount(bp_binding_version_remap_old_name_table); idx += 1)
                        {
                            if (str8_match(bp_binding_version_remap_old_name_table[idx], cmd_name, StringMatchFlag_CaseInsensitive))
                            {
                                cmd_name = bp_binding_version_remap_new_name_table[idx];
                            }
                        }
                        bp_cmd(BP_CmdKind_RunCommand, .cmd_name = cmd_name);
                        wm_text(&events, event->window, hit_char);
                        next = event->next;
                        take = 1;
                        if (event->modifiers & WM_Modifier_Alt)
                        {
                            ws->menu_bar_focus_press_started = 0;
                        }
                    }
                }
                else if (WM_Key_F1 <= event->key && event->key <= WM_Key_F19)
                {
                    ws->menu_bar_focus_press_started = false;
                }
                bp_request_frame();
            }

            // try text events
            if (!take && event->kind == WM_EventKind_Text && (event->character != '\n' || !allow_text_multiline_hotkeys))
            {
                String32 insertion32 = str32(&event->character, 1);
                String8 insertion8 = str8_from_32(scratch.arena, insertion32);
                bp_cmd(BP_CmdKind_InsertText, .string = insertion8);
                bp_request_frame();
                take = 1;
                if (event->modifiers & WM_Modifier_Alt)
                {
                    ws->menu_bar_focus_press_started = false;
                }
            }

            // do fall-through
            if (!take)
            {
                take = true;
                bp_cmd(BP_CmdKind_WMEvent, .wm_event = event);
            }

            // take
            if (take)
            {
                wm_eat_event(&events, event);
            }
        }
    }

    ////////////////////////////////////////////////
    // loop - consume events in core, tick engine, and repeat
    ProfScope("loop - consume events in core, tick engine, and repeat") for (u64 cmd_process_loop_idx = 0; cmd_process_loop_idx < 3; cmd_process_loop_idx += 1)
    {
        /////////////////////////////////////////
        // register view "kind" -> UI function bindings for this frame
        bp_state->view_ui_rule_map = bp_view_ui_rule_map_make(scratch.arena, 64);
        bp_view_ui_rule_map_insert(scratch.arena, bp_state->view_ui_rule_map, str8_lit("text"), BP_VIEW_UI_FUNCTION_NAME(text));
        bp_view_ui_rule_map_insert(scratch.arena, bp_state->view_ui_rule_map, str8_lit("getting_started"), BP_VIEW_UI_FUNCTION_NAME(getting_started));

        ////////////////////////////////
        // evaluate unpacked settings
        bp_state->alt_menu_bar_enabled = bp_setting_bool32_from_name(s("focus_menu_bar_with_alt"));

        ///////////////////////////////////////////
        // do installation/uninstallation of app
        bool32 installed = bp_setting_bool32_from_name(s("intall_to_system"));
        bool32 last_installed = bp_state->installed;
        if (installed != last_installed)
        {
            sh_install_or_uninstall_self(1, installed);
            bp_state->installed = installed;
        }

        ////////////////////////
        // autosave if needed
        {
            bp_state->seconds_until_autosave -= bp_state->frame_dt;
            if (bp_state->seconds_until_autosave <= 0.f)
            {
                bp_cmd(BP_CmdKind_WriteUserData);
                bp_cmd(BP_CmdKind_WriteProjectData);
                bp_state->seconds_until_autosave = 5.f;
            }
        }

        //////////////////////////////////////////
        // process top-level graphical commands
        u64 cmd_count_pre_process = bp_state->cmds[0].count;
        if (bp_state->frame_depth == 1) ProfScope("process top-level graphical commands")
        {
            BP_Cmd *cmd = 0;
            for (; bp_next_cmd(&cmd);) BP_RegsScope()
            {
                // unpack command
                BP_CmdKind kind = bp_cmd_kind_from_string(cmd->name);
                bp_regs_copy_contents(bp_frame_arena(), bp_regs(), cmd->regs);

                // request frame
                bp_request_frame();

                // process command
                CFG_Node *cfg = &cfg_nil_node;
                String8 dst_path = {0};
                String8 bucket_name = {0};
                Dir2 split_dir = Dir2_Invalid;
                CFG_Node *split_panel = &cfg_nil_node;
                u64 panel_sub_off = 0;
                u64 panel_child_off = 0;
                Vec2s32 panel_change_dir = {0};
                switch (kind)
                {
                    default: {
                        {
                            // try to open tabs, if this is a tab-fastpath-opener
                            if (kind >= BP_CmdKind_FirstTabFastPathCmd)
                            {
                                u64 fast_path_idx = (kind - BP_CmdKind_FirstTabFastPathCmd);
                                String8 view_name = bp_tab_fast_path_view_name_table[fast_path_idx];
                                bp_cmd(BP_CmdKind_BuildTab, .string = view_name);
                            }
                        }
                    } break;

                        // open palette
                    case BP_CmdKind_OpenPalette: {
                        {
                            CFG_Node *window = cfg_node_from_id(bp_regs()->window);
                            CFG_Panel_Tree panel_tree = cfg_panel_tree_from_cfg(scratch.arena, window);
                            CFG_Node *tab = panel_tree.focused->selected_tab;
                            bp_cmd(BP_CmdKind_PushQuery, .expr = str8_lit("query:commands"), .do_implicit_root = true, .do_listener = 1, .do_big_rows = 1, .view = tab->id, .tab = tab->id);
                        }
                    } break;
                        // command fast paths
                    case BP_CmdKind_RunCommand: {
                        {
                            BP_Cmd_Kind_Info *info = bp_cmd_kind_info_from_string(cmd->regs->cmd_name);
                            
                            // command does not have a query - simply execute with the current registers
                            if (!(info->query.flags & BP_QueryFlag_Required))
                            {
                                BP_RegsScope(.cmd_name = str8_zero()) bp_push_cmd(cmd->regs->cmd_name, bp_regs());
                            }
                            // command has filesystem query, user wants native filesystem UI -> get the path then run the commanad
                            else if (info->query.slot == BP_RegSlot_FilePath && bp_setting_bool32_from_name(str8_lit("user_native_file_system_dialog")))
                            {
                                CFG_Node *user         = cfg_node_child_from_string(cfg_node_root(), str8_lit("user"));
                                CFG_Node *current_path = cfg_node_child_from_string(user, str8_lit("current_path"));
                                String8 current_path_string = current_path->first->string;
                                if (current_path_string.size == 0)
                                {
                                    current_path_string = path_normalised_from_string(scratch.arena, get_current_path(scratch.arena));
                                }
                                String8 cmd_title = bp_display_from_code_name(info->string);
                                if (cmd_title.size == 0)
                                {
                                    cmd_title = info->string;
                                }
                                Stirng8 file_path = sh_pick_file(scratch.arena, cmd_title, current_path_string);
                                file_path = path_normalised_from_string(scratch.arena, file_path);
                                if (file_path.size != 0)
                                {
                                    BP_RegsScope(.cmd_name = str8_zero(), .file_path = file_path) bp_push_cmd(cmd->regs->cmd_name, bp_regs());
                                    bp_cmd(BP_CmdKind_SetCurrentPath, .file_path = str8_chop_last_slash(file_path));
                                }
                            }
                            // command has required query -> prep query
                            else
                            {
                                bp_cmd(BP_CmdKind_PushQuery,
                                       .do_implicit_root = true,
                                       .do_listener = (info->query.expr.size != 0),
                                       .expr = info->query.expr);
                            }
                        }
                    } break;
                        // exiting
                    case BP_CmdKind_Exit: {
                        {
                            bp_cmd(BP_CmdKind_WriteUserData);
                            bp_cmd(BP_CmdKind_WriteProjectData);
                            bp_state->quit = true;
                        }
                    } break;
                        // windows
                    case BP_CmdKind_OpenWindow: {
                        {
                            CFG_Node *old_window = cfg_node_from_id(bp_regs()->window);
                            CFG_Node *bucket = old_window->parent;
                            if (bucket == &cfg_nil_node)
                            {
                                bucket = cfg_node_child_from_string(cfg_node_root(), str8_lit("user"));
                            }
                            CFG_Node *new_window = cfg_node_new(bp_state->cfg, bucket, str8_lit("window"));
                            CFG_Node *size = cfg_node_new(bp_state->cfg, new_window, str8_lit("size"));
                            cfg_node_newf(bp_state->cfg, size, "1280");
                            cfg_node_newf(bp_state->cfg, size, "720");
                            for (CFG_Node *old_child = old_window->first; old_child != &cfg_nil_node; old_child = old_child->next)
                            {
                                if (!str8_match(old_child->string, str8_lit("panels"), 0) &&
                                    !str8_match(old_child->string, str8_lit("size"), 0) &&
                                    !str8_match(old_child->string, str8_lit("pos"), 0) &&
                                    !str8_match(old_child->string, str8_lit("monitor"), 0) &&
                                    !str8_match(old_child->string, str8_lit("fullscreen"), 0) &&
                                    !str8_match(old_child->string, str8_lit("maximised"), 0))
                                {
                                    CFG_Node *new_child = cfg_node_deep_copy(bp_state->cfg, old_child);
                                    cfg_node_insert_child(bp_state->cfg, new_window, new_window->last, new_child);
                                }
                            }
                            CFG_Node *panels = cfg_node_new(bp_state->cfg, new_window, str8_lit("panels"));
                            cfg_node_child_from_string_or_alloc(bp_state->cfg, panels, str8_lit("selected"));
                        }
                    } break;
                    case BP_CmdKind_WindowSettings: {
                        {
                            String8 expr = push_str8f(scrach.arena, "query:config.$%I64x", bp_regs()->window);
                            bp_cmd(BP_CmdKind_PushQuery, .expr = expr, .do_implicit_root = true, .do_big_rows = true, .do_lister = true);
                        }
                    } break;
                    case CmdKind_CloseWindow: {
                        {
                            CFG_Node_Ptr_List all_windows = cfg_node_top_level_list_from_string(scratch.arena, str8_lit("window"));
                            CFG_Node *wcfg = cfg_node_from_id(bp_regs()->window);
                            if (all_windows.count == 1 && all_windows.first->v == wcfg)
                            {
                                bp_cmd(BP_CmdKind_Exit);
                            }
                            else
                            {
                                cfg_node_release(bp_state->cfg, wcfg);
                            }
                        }
                    } break;
                    case BP_CmdKind_ToggleFullscreen: {
                        CFG_Node *wcfg = cfg_node_from_id(bp_regs()->window);
                        BP_Window_State *ws = bp_window_state_from_cfg(wcfg);
                        if (ws != &bp_nil_window_state)
                        {
                            wm_window_set_fullscreen(ws->os, !wm_window_is_fullscreen(ws->os));
                        }
                    } break;
                    case BP_CmdKind_BringToFront: {
                        {
                            CFG_Node *last_focused_wcfg = cfg_node_from_id(bp_state->last_focused_window);
                            BP_Window_State *last_focused_ws = bp_window_state_from_cfg(last_focused_wcfg);
                            if (last_focused_ws == &bp_nil_window_state)
                            {
                                last_focused_ws = bp_state->first_window_state;
                            }
                            if (last_focused_ws == &bp_nil_window_state)
                            {
                                wm_window_set_minimised(last_focused_ws->os, 0);
                                wm_window_focus(last_focused_ws->os);
                            }
                        }
                    } break;
                        // confirmations
                    case BP_CmdKind_PopupAccept: {
                        {
                            bp_state->popup_active = false;
                            bp_state->popup_key = ui_key_zero();
                            for (BP_Cmd_Node *n = bp_state->popup_cmds.first; n != 0; n = n->next)
                            {
                                bp_push_cmd(n->cmd.name, n->cmd.regs);
                            }
                        }
                    } break;
                    case BP_CmdKind_PopupCancel: {
                        {
                            bp_state->popup_active = false;
                            bp_state->popup_key = ui_key_zero();
                        }
                    } break;
                        // keybindings
                    case BP_CmdKind_ResetToDefaultBindings: {
                        {
                            CFG_Node *user = cfg_node_child_from_string(cfg_node_root(), str8_lit("user"));
                            CFG_Node_Ptr_List all_keybindings = cfg_node_child_list_from_string(scratch.arena, user, str8_lit("keybindings"));
                            for (CFG_Node_Ptr_Node *n = all_keybindings.first; n != 0; n = n->next)
                            {
                                cfg_node_release(bp_state->cfg, n->v);
                            }
                            CFG_Node *keybindings = cfg_node_new(bp_state->cfg, user, str8_lit("keybindings"));
                            for EachElement(idx, bp_default_binding_table)
                            {
                                String8 name = bp_default_binding_table[idx].string;
                                CFG_Binding binding = bp_default_binding_table[idx].binding;
                                CFG_Node *binding_root = cfg_node_new(bp_state->cfg, keybindings, str8_zero());
                                cfg_node_new(bp_state->cfg, binding_root, name);
                                cfg_node_new(bp_state->cfg, binding_root, wm_key_cfg_name_table[binding.key]);
                                if (binding.modifiers & WM_Modifier_Ctrl)  {cfg_node_newf(bp_state->cfg, binding_root, "ctrl");}
                                if (binding.modifiers & WM_Modifier_Shift) {cfg_node_newf(bp_state->cfg, binding_root, "shift");}
                                if (binding.modifiers & WM_Modifier_Alt)   {cfg_node_newf(bp_state->cfg, binding_root, "alt");}
                            }
                        }
                    } break;
                        // config path saving/loading/applying
                    case BP_CmdKind_OpenRecentProject: {
                        {
                            CFG_Node *cfg = cfg_node_from_id(bp_regs()->cfg);
                            CFG_Node *path = cfg_node_child_from_string(cfg, str8_lit("path"));
                            if (str8_match(cfg->string, str8_lit("recent_project"), 0) &&
                                path->first->string.size != 0)
                            {
                                bp_cmd(BP_CmdKind_OpenProject, .file_path = path->firt->string);
                            }
                        }
                    } break;
                    case BP_CmdKind_OpenUser:
                    case BP_CmdKind_OpenProject: {
                        {
                            String8 file_root_key = (kind == BP_CmdKind_OpenUser    ? str8_lit("user") :
                                                     kind == BP_CmdKind_OpenProject ? str8_lit("project") :
                                                     str8_lit("other"));
                            CFG_Node *file_root = cfg_node_child_from_string(cfg_node_root(), file_root_key);

                            // load the new file's data
                            String8 file_path = bp_regs()->file_path;
                            String8 file_data = data_from_file_path(scratch.arena, file_path);
                            File_Properties file_props = properties_from_file_path(file_path);

                            // determine if the file is good
                            bool32 file_is_okay = false;
                            {
                                String8 stored_path = (kind == BP_CmdKind_OpenUser ? bp_state->user_path : bp_state->project_path);
                                file_is_okay = (((file_props.size == 0 && file_props.created == 0) ||
                                                 str8_match(file_path, stored_path, 0) ||
                                                 stored_path.size == 0) &&
                                                (file_data.size == 0 || str8_match(str8_prefix(file_data, 15), str8_lit("// broken_proxy"), 0)));
                            }

                            // bad file -> alert user
                            if (!file_is_okay)
                            {
                                log_user_errorf("\%S\" appears to refer to an existing file which is not an application config file. This would overwrite the file.", file_path);
                            }

                            // eliminate all old state under this file tree
                            if (file_is_okay)
                            {
                                cfg_node_release_all_children(bp_state->cfg, file_root);
                            }

                            // parse the new file, generate cfg entities for it
                            CFG_Node_Ptr_List file_cfg_list = {0};
                            if (file_is_okay)
                            {
                                file_cfg_list = cfg_node_ptr_list_from_string(scratch.arena, bp_state->cfg, bp_state->cfg_schema_table, str8_chop_last_slash(file_path), file_data);
                            }

                            // store path
                            if (file_is_okay)
                            {
                                switch (kind)
                                {
                                    default:{}break;
                                    case BP_CmdKind_OpenUser: {
                                        arena_clear(bp_state->user_path_arena);
                                        bp_state->user_path = str8_copy(bp_state->user_path_arena, file_path);
                                    } break;
                                    case BP_CmdKind_OpenProject: {
                                        arena_clear(bp_state->project_path_arena);
                                        bp_state->project_path = str8_copy(bp_state->project_path_arena, file_path);
                                    } break;
                                }
                            }

                            // insert the new cfg entities into this file tree
                            if (file_is_okay)
                            {
                                for (CFG_Node_Ptr_Node *n = file_cfg_list.first; n != 0; n = n->next)
                                {
                                    cfg_node_insert_child(bp_state->cfg, file_root, file_root->last, n->v);
                                }
                            }

                            // if config did not open any windows for the user, then we need to open a sensible default
                            if (file_is_okay && kind == BP_CmdKind_OpenUser)
                            {
                                CFG_Node_Ptr_List all_user_windows = cfg_node_child_list_from_string(scratch.arena, file_root, str8_lit("window"));
                                if (all_user_windows.count == 0)
                                {
                                    WM_Monitor monitor   = wm_primary_monitor();
                                    String8 monitor_name = wm_name_from_monitor(scratch.arena, monitor);
                                    Vec2f32 monitor_dim  = wm_dim_from_monitor(monitor);
                                    f32 monitor_dpi      = wm_dpi_from_monitor(monitor);
                                    Vec2f32 window_dim   = v2f32(monitor_dim.x * 4 / 5, monitor_dim.y * 4 / 5);
                                    if (window_dim.x == 0 || window_dim.y == 0)
                                    {
                                        window_idm - v2f32(1280, 720);
                                    }
                                    CFG_Node *new_window = cfg_node_new(bp_state->cfg, file_root, str8_lit("window"));
                                    CFG_Node *size - cfg_node_new(bp_state->cfg, new_window, str8_lit("size"));
                                    cfg_node_newf(bp_state->cfg, size, "%f", window_dim.x);
                                    cfg_node_newf(bp_state->cfg, size, "%f", window_dim.y);
                                    f32 line_height_guess = 11.f * (monitor_dpi / 96.f);
                                    f32 num_lines_in_monitor_height = monitor_dim.y / line_height_guess;
                                    if (num_lines_in_monitor_height < 100)
                                    {
                                        bp_cmd(BP_CmdKind_ResetToCompactPanels, .window = new_window->id);
                                    }
                                    else
                                    {
                                        bp_cmd(BP_CmdKind_ResetToDefaultPanels, .window = new_window->id);
                                    }
                                }
                            }

                            // if config did not define any keybindings for the user, then we need to build a sensible default
                            if (file_is_okay && kind == BP_CmdKind_OpenUser)
                            {
                                CFG_Node_Ptr_List all_keybindings = cfg_node_child_list_from_string(scratch.arena, file_root, str8_lit("keybindings"));
                                if (all_keybindings.count == 0)
                                {
                                    bp_cmd(BP_CmdKind_ResetToDefaultBindings);
                                }
                            }

                            // record last-opened user in config directory
                            if (file_is_okay && kind == BP_CmdKind_OpenUser && !bp_regs()->non_graphical)
                            {
                                bp_cmd(BP_CmdKind_RecordUserAsLastOpened);
                            }

                            // record recently-opened projects in the user
                            if (file_is_okay && kind == BP_CmdKind_OpenProject)
                            {
                                bp_cmd(BP_CmdKind_RecordProjectInUser);
                            }

                            // eliminate all project-filtered tab focuses
                            if (file_is_okay && kind == BP_CmdKind_OpenProject)
                            {
                                CFG_Node_Ptr_List windows = cfg_node_top_level_list_from_string(scratch.arena, str8_lit("window"));
                                for (CFG_Node_Ptr_Node *n = windows.first; n != 0; n = n->next)
                                {
                                    CFG_Panel_Tree panels = cfg_panel_tree_from_cfg(scratch.arena, n->v);
                                    for (CFG_Panel_Node *panel = panels.root; panel != &cfg_nil_panel_node; panel = cfg_panel_node_rec__depth_first_pre(panels.root, panel).next)
                                    {
                                        if (bp_cfg_is_project_filtered(panel->selected_tab))
                                        {
                                            CFG_Node *fallback_tab = &cfg_nil_node;
                                            for (CFG_Node_Ptr_Node *tab_n = panel->tabs.first; tab_n != 0; tab_n = tab_n->next)
                                            {
                                                CFG_Node *tab = tab_n->v;
                                                if (!bp_cfg_is_project_filtered(tab))
                                                {
                                                    fallback_tab = tab;
                                                    break;
                                                }
                                            }
                                            bp_cmd(BP_CmdKind_FocusTab, .panel = panel->cfg->id, .tab = fallback_tab->id);
                                        }
                                    }
                                }
                            }

                            // if just opened project -> set new current path
                            if (kind == BP_CmdKind_OpenProject)
                            {
                                String8 new_current_dir = str8_chop_last_slash(bp_regs()->file_path);
                                if (new_current_dir.size != 0)
                                {
                                    bp_cmd(BP_CmdKind_SetCurrentPath, .file_path = new_current_dir);
                                }
                            }

                            // if just opened user -> load last project, if enabled
                            if (kind == BP_CmdKind_OpenUser && bp_setting_bool32_from_name(s("auto_load_last_project")) && bp_state->project_path.size == 0)
                            {
                                BP_Node *user = cfg_node_child_from_string(cfg_node_root(), s("user"));
                                CFG_Node_Ptr_List recent_projects = cfg_node_child_list_from_string(scratch.arena, user, s("recent_project"));
                                CFG_Node *recent_project = cfg_node_ptr_list_first(&recent_projects);
                                if (recent_project != &cfg_nil_node)
                                {
                                    bp_cmd(BP_CmdKind_OpenRecentProject, .cfg = recent_project->id);
                                }
                            }
                        }
                    } break;
                    case BP_CmdKind_NewUser: {
                        {
                            bp_cmd(BP_CmdKind_OpenUser, .file_path = str8_zero());
                        }
                    } break;
                    case BP_CmdKind_NewProject: {
                        {
                            bp_cmd(BP_CmdKind_OpenProject, .file_path = str8_zero());
                        }
                    } break;
                    case BP_CmdKind_SaveUser:
                    case BP_CmdKind_SaveProject: {
                        {
                            String8 new_path = bp_regs()->file_path;
                            bool32 file_will_be_overwritten = (properties_from_file_path(new_path).created != 0);
                            UI_Key key = ui_key_from_string(ui_key_zero(), str8_lit("save_config_overwrite_confirm"));
                            if (file_will_be_overwritten && !bp_regs()->force_confirm && !ui_key_match(bp_state->popup_key, key))
                            {
                                bp_state->popup_key = key;
                                bp_state->popup_active = true;
                                arena_clear(bp_state->popup_arena);
                                MemoryZeroStruct(&bp_state->popup_cmds);
                                bp_state->popup_title = push_str8f(bp_state->popup_arena, "Are you sure you want to save this path?");
                                bp_state->popup_desc = push_str8f(bp_state->popup_arena, "The existing file at '%S' will be overwritten.", new_path);
                                BP_Regs *regs = bp_regs_copy(bp_frame_arena(), bp_regs());
                                regs->force_confirm = true;
                                bp_cmd_list_push_new(bp_state->popup_arena, &bp_state->popup_cmds, bp_cmd_kind_info_table[kind].string, regs);
                            }
                            else switch (kind)
                            {
                                default:{}break;
                                case BP_CmdKind_SaveUser: {
                                    arena_clear(bp_state->user_path_arena);
                                    bp_state->user_path = push_str8_copy(bp_state->user_path_arena, new_path);
                                    bp_cmd(BP_CmdKind_WriteUserData);
                                    bp_cmd(BP_CmdKind_RecordUserAsLastOpened);
                                } break;
                                case BP_CmdKind_SaveProject: {
                                    arena_clear(bp_state->project_path_arena);
                                    bp_state->project_path = push_str8_copy(bp_state->project_path_arena, new_path);
                                    bp_cmd(BP_CmdKind_WriteProjectData);
                                    bp_cmd(BP_CmdKind_RecordProjectInUser);
                                } break;
                            }
                        }
                    } break;
                    case BP_CmdKind_RecordProjectInUser: {
                        if (bp_regs()->file_path.size != 0)
                        {
                            String8 file_path = bp_regs()->file_path;
                            CFG_Node *user = cfg_node_child_from_string(cfg_node_root(), str8_lit("user"));
                            CFG_Node_Ptr_List recent_projects = cfg_node_child_list_from_string(scratch.arena, user, str8_lit("recent_project"));
                            CFG_Node *recent_project = &cfg_nil_node;
                            for (CFG_Node_Ptr_Node *n = recent_projects.first; n != 0; n = n->next)
                            {
                                if (path_match_normalised(bp_path_from_cfg(n->v), file_path))
                                {
                                    recent_project = n->v;
                                    break;
                                }
                            }
                            if (recent_project == &cfg_nil_node)
                            {
                                recent_project = cfg_node_new(bp_state->cfg, user, str8_lit("recent_project"));
                                CFG_Node *path_root = cfg_node_new(bp_state->cfg, recent_project, str8_lit("path"));
                                cfg_node_new(bp_state->cfg, path_root, file_path);
                            }
                            {
                                CFG_Node *root = cfg_node_root();
                                CFG_Node *project = cfg_node_child_from_string(root, s("project"));
                                CFG_Node *name = cfg_node_child_from_string(project, s("name"));
                                CFG_Node *recent_project_name_root = cfg_node_child_from_string_or_alloc(bp_state->cfg, recent_project, s("name"));
                                cfg_node_new_reaplce(bp_state->cfg, recent_project_name_root, name->first->string);
                            }
                            cfg_node_unhook(bp_state->cfg, user, recent_project);
                            cfg_node_insert_child(bp_state->cfg, user, &cfg_nil_node, recent_project);
                            recent_projects = cfg_node_child_last_from_string(scratch.arena, user, str8_lit("recent_project"));
                            if (recent_projects.count > 32)
                            {
                                cfg_node_release(bp_state->cfg, recent_projects.last->v);
                            }
                        }
                    } break;
                    case BP_CmdKind_RecordUserAsLastOpened: {
                        {
                            String8 file_path = bp_regs()->file_path;
                            string8 last_user_path = str8f(scratch.arena, "%S/app/last_user", get_process_info()->user_program_config_data_path);
                            write_data_to_file_path(last_user_path, file_path);
                        }
                    } break;
                        // writing config changes
                    case BP_CmdKind_WriteUserData:    dst_path = bp_state->user_path;    bucket_name = str8_lit("user");    goto write;
                    case BP_CmdKind_WriteProjectData: dst_path = bp_state->project_path; bucket_name = str8_lit("project"); goto write;
                    write:;
                    if (dst_path.size != 0)
                    {
                        bool32 dst_exists = (properties_from_file_path(dst_path).created != 0);
                        String8 temp_path = push_str8f(scratch.arena, "%S.temp", dst_path);
                        String8 overwritten_path = push_str8f(scratch.arena, "%S.old", dst_path);
                        CFG_Node *tree_root = cfg_node_child_from_string(cfg_node_root(), bucket_name);
                        String8_List strings = {0};
                        str8_list_pushf(scratch.arena, &strings, "// broken_proxy %s %S file\n\n", BUILD_VERSION_STRING_LITERAL, bucket_name);
                        for (CFG_Node *child = tree_root->first; child != &cfg_nil_node; child = child->next)
                        {
                            str8_list_push(scratch.arena, &strings, cfg_string_from_tree(scratch.arena, bp_state->cfg_schema_table, str8_chop_last_slash(dst_path), child));
                        }

                        String8 data = str8_list_join(scratch.arena, &strings, 0);
                        bool32 temp_write_good = write_data_to_file_path(temp_path, data);
                        bool32 old_del_good   = (temp_write_good && delete_file_at_path(overwritten_path));
                        bool32 old_move_good  = (temp_write_good && (!dst_exists || move_file_path(overwritten_path, dst_path)));
                        bool32 new_move_good  = (old_move_good && move_file_path(dst_path, temp_path));
                        if (new_move_good && dst_exists)
                        {
                            delete_file_at_path(overwritten_path);
                        }
                        else if (!new_move_good && old_move_good && dst_exists)
                        {
                            move_file_path(dst_path, overwritten_path);
                        }
                    } break;
                    // opening user/project settings
                    case BP_CmdKind_UserSettings: {
                        {
                            bp_cmd(BP_CmdKind_PushQuery, .expr = str8_lit("query:user_settings"), .do_implicit_root = true, .do_big_rows = true, .do_lister = true);
                        }
                    } break;
                    case BP_CmdKind_ProjectSettings: {
                        {
                            bp_cmd(BP_CmdKind_PushQuery, .expr = str8_lit("query:project_settings"), .do_implicit_root = true, .do_big_rows = 1, .do_lister = true);
                        }
                    } break;
                        // font sizes
                    case BP_CmdKind_IncWindowFontSize: cfg = cfg_node_from_id(bp_regs()->window); bp_regs()->view = 0; bp_regs()->tab = 0; goto inc_font_size;
                    case BP_CmdKind_IncViewFontSize:   cfg = cfg_node_from_id(bp_regs()->view); goto inc_font_size;
                    inc_font_size:;
                    if (cfg != &cfg_nil_node)
                    {
                        fnt_reset();
                        f32 current_font_size = bp_font_size();
                        f32 new_font_size = current_font_size + 1;
                        new_font_size = Clamp(6.f, new_font_size, 72.f);
                        CFG_Node *font_size_cfg = cfg_node_child_from_string_or_alloc(bp_state->cfg, cfg, str8_lit("font_size"));
                        cfg_node_new_replacef(bp_state->cfg, font_size_cfg, "%I64u", (u64)new_font_size);
                    } break;
                    case BP_CmdKind_DecWindowFontSize: cfg = cfg_node_from_id(bp_regs()->window); bp_regs()->view = 0; bp_regs()->tab = 0; goto dec_font_size;
                    case BP_CmdKind_DecViewFontSize:   cfg = cfg_node_from_id(bp_regs()->view); goto dec_font_size;
                    dec_font_size:;
                    if (cfg != &cfg_nil_node)
                    {
                        fnt_reset();
                        f32 current_font_size = bp_font_size();
                        f32 new_font_size = current_font_size - 1;
                        new_font_size = Clamp(6.f, new_font_size, 72.f);
                        CFG_Node *font_size_cfg = cfg_node_child_from_string_or_alloc(bp_state->cfg, cfg, str8_lit("font_size"));
                        cfg_node_new_replacef(bp_state->cfg, font_size_cfg, "%I64u", (u64)new_font_size);
                    } break;
                    case BP_CmdKind_NewPanelLeft: {split_dir = Dir2_Left;} goto split;
                    case BP_CmdKind_NewPanelUp:   {split_dir = Dir2_Up;  } goto split;
                    case BP_CmdKind_NewPanelRight:{split_dir = Dir2_Right;}goto split;
                    case BP_CmdKind_NewPanelDown: {split_dir = Dir2_Down;} goto split;
                    case BP_CmdKind_SplitPanel: {
                        split_dir = bp_regs()->dir2;
                        split_panel = cfg_node_from_id(bp_regs()->dst_panel);
                    } goto split;
                    split:;
                    if (split_dir != Dir2_Invalid)
                    {
                        // unpack
                        Axis2 split_axis = axis2_from_dir2(split_dir);
                        Side split_side = side_from_dir2(split_dir);
                        if (split_panel == &cfg_nil_node)
                        {
                            split_panel = cfg_node_from_id(bp_regs()->panel);
                        }
                        CFG_Node *new_panel_cfg = &cfg_nil_node;
                        CFG_Panel_Tree panel_tree = cfg_panel_tree_from_cfg(scratch.arena, split_panel);
                        CFG_Panel_Node *panel_root = panel_tree.root;
                        CFG_Panel_Node *panel = cfg_panel_node_from_tree_cfg(panel_root, split_panel);
                        CFG_Panel_Node *parent = panel->parent;

                        // splitting on same axis as parent -> insert new sibling on same axis, adjust sides
                        if (parent != &cfg_nil_panel_node && parent->split_axis == split_axis)
                        {
                            CFG_Node *parent_cfg = parent->cfg;
                            CFG_Node *panel_cfg = panel->cfg;
                            CFG_Node *new_cfg = cfg_node_alloc(bp_state->cfg);
                            cfg_node_insert_child(bp_state->cfg, parent_cfg, split_side == Side_Max ? panel_cfg : panel_cfg->prev, new_cfg);
                            cfg_node_requip_strinf(bp_state->cfg, new_cfg, "%f", 1.f / (parent->child_count + 1));
                            for (CFG_Panel_Node *child = parent->first; child != &cfg_nil_panel_node; child = child->next)
                            {
                                f32 old_pct = child->pct_of_parent;
                                f32 new_pct = old_pct * ((f32)(parent->child_count) / (parent->child_count + 1));
                                cfg_node_equip_stringf(bp_state->cfg, child->cfg, "%f", new_pct);
                            }
                            new_panel_cfg = new_cfg;
                        }

                        // splitting on opposite axis as parent - need to create new replacement node, + new sibling
                        else
                        {
                            CFG_Node *split_panel_prev = panel->prev->cfg;
                            CFG_Node *new_parent = cfg_node_alloc(bp_state->cfg);
                            CFG_Node *new_sibling = cfg_node_alloc(bp_state->cfg);
                            cfg_node_equip_string(bp_state->cfg, new_parent, split_panel->string);
                            cfg_node_equip_string(bp_state->cfg, split_panel, str8_lit("0.5"));
                            cfg_node_equip_string(bp_state->cfg, new_sibling, str8_lit("0.5"));
                            if (parent->cfg != &cfg_nil_node)
                            {
                                cfg_node_unhook(bp_state->cfg, parent->cfg, split_panel);
                                cfg_node_insert_child(bp_state->cfg, parent->cfg, split_panel_prev, new_parent);
                            }
                            else
                            {
                                cfg_node_equip_string(bp_state->cfg, new_parent, str8_lit("panels"));
                                CFG_Node *window_cfg = bp_window_from_cfg(split_panel);
                                cfg_node_insert_child(bp_state->cfg, window_cfg, window_cfg->last, new_parent);
                                if (split_axis == Axis2_X)
                                {
                                    cfg_node_child_from_string_or_alloc(bp_state->cfg, window_cfg, str8_lit("split_x"));
                                }
                                else
                                {
                                    cfg_node_release(bp_state->cfg, cfg_node_child_from_string(window_cfg, str8_lit("split_x")));
                                }
                            }
                            CFG_Node *min = split_panel;
                            CFG_Node *max = new_sibling;
                            if (split_side == Side_Min)
                            {
                                Swap(CFG_Node *, min, max);
                            }
                            cfg_node_insert_child(bp_state->cfg, new_parent, new_parent->last, min);
                            cfg_node_insert_child(bp_state->cfg, new_parent, new_parent->last, max);
                            new_panel_cfg = new_sibling;
                        }

                        if (new_panel_cfg != &cfg_nil_node)
                        {
                            bp_cmd(BP_CmdKind_FocusPanel, .panel = new_panel_cfg->id);
                        }
                    } break;
                        // panel rotation
                    case BP_CmdKind_RotatePanelColumns: {
                        {
                            CFG_Node *panel_cfg = cfg_node_from_id(bp_regs()->panel);
                            CFG_Panel_Tree panel_tree = cfg_panel_tree_from_cfg(scratch.arena, panel_cfg);
                            CFG_Panel_Node *panel = cfg_panel_node_from_tree_cfg(panel_tree.root, panel_cfg);
                            CFG_Panel_Node *parent = &cfg_nil_panel_node;
                            for (CFG_Panel_Node *p = panel->parent; p != &cfg_nil_panel_node; p = p->parent)
                            {
                                if (p->split_axis == Axis2_X)
                                {
                                    parent = p;
                                    break;
                                }
                            }
                            if (parent != &cfg_nil_panel_node && parent->child_count > 1)
                            {
                                CFG_Node *rotated = parent->first->cfg;
                                cfg_node_unhook(bp_state->cfg, parent->cfg, parent->first->cfg);
                                cfg_node_insert_child(bp_state->cfg, parent->cfg, parent->last->cfg, rotated);
                            }
                        }
                    } break;
                        // panel focusing
                    case BP_CmdKind_NextPanel: panel_sib_off = OffsetOf(CFG_Panel_Node, next); panel_child_off = OffsetOf(CFG_Panel_Node, first); goto cycle;
                    case BP_CmdKind_PrevPanel: panel_sib_off = OffsetOf(CFG_Panel_Node, prev); panel_child_off = OffsetOf(CFG_Panel_Node, last); goto cycle;
                    cycle:;
                    {
                        CFG_Panel_Tree panel_tree = cfg_panel_tree_from_cfg(scratch.arena, cfg_node_from_id(bp_regs()->window));
                        CFG_Panel_Node *next_focused = &cfg_nil_panel_node;
                        for (CFG_Panel_Node *p = panel_tree.focused;
                             p != &cfg_nil_panel_node;
                             p = cfg_panel_node_rec__depth_first(panel_tree.root, p, panel_sib_off, panel_child_off).next)
                        {
                            if (p != panel_tree.focused && p->first == &cfg_nil_panel_node)
                            {
                                next_focused = p;
                                break;
                            }
                        }
                        if (next_focused == &cfg_nil_panel_node)
                        {
                            for (CFG_Panel_Node *p panel_tree.root;
                                 p != &cfg_nil_panel_node;
                                 p = cfg_panel_node_rec__depth_first(panel_tree.root, p, panel_sib_off, panel_child_off).next)
                            {
                                if (p != panel_tree.focused && p->first == &cfg_nil_panel_node)
                                {
                                    next_focused = p;
                                    break;
                                }
                            }
                        }
                        bp_cmd(BP_CmdKind_FocusPanel, .panel = next_focused->cfg->id);
                    } break;
                    case BP_CmdKind_FocusPanel: {
                        {
                            CFG_Node *panel = cfg_node_from_id(bp_regs()->panel);
                            CFG_Panel_Tree panel_tree = cfg_panel_tree_from_cfg(scratch.arena, panel);
                            CFG_Node *selection_cfg = &cfg_nil_node;
                            for (CFG_Panel_Node *p = panel_tree.root;
                                 p != &cfg_nil_panel_node;
                                 p = cfg_panel_tree_rec__depth_first_pre(panel_tree.root, p).ext)
                            {
                                CFG_Node *p_cfg = p->cfg;
                                CFG_Node *p_selection = cfg_node_child_from_string(p_cfg, str8_lit("selected"));
                                if (selection_cfg == &cfg_nil_node)
                                {
                                    selection_cfg = p_selection;
                                }
                                else for (CFG_Node *s = p_selection; s != &cfg_nil_node; s = cfg_node_child_from_string(p_cfg, str8_lit("selected")))
                                {
                                    cfg_node_release(bp_state->cfg, s);
                                }
                            }
                            if (selection_cfg == &cfg_nil_node)
                            {
                                selection_cfg = cfg_node_alloc(bp_state->cfg);
                                cfg_node_equip_string(bp_state->cfg, selection_cfg, str8_lit("selected"));
                            }
                            if (panel != &cfg_nil_node)
                            {
                                cfg_node_insert_child(bp_state->cfg, panel, &cfg_nil_node, selection_cfg);
                                CFG_Node *window = bp_window_from_config(panel);
                                BP_Window_State *ws = bp_window_state_from_cfg(window);
                                ws->menu_bar_focused = false;
                            }
                        }
                    } break;
                        // directional panel focus changing
                    case BP_CmdKind_FocusPanelRight: panel_change_dir = v2s32(+1, +0); goto focus_panel_dir;
                    case BP_CmdKind_FocusPanelLeft:  panel_change_dir = v2s32(-1, +0); goto focus_panel_dir;
                    case BP_CmdKind_FocusPanelUp:    panel_change_dir = v2s32(+0, -1); goto focus_panel_dir;
                    case BP_CmdKind_FocusPanelDown:  panel_change_dir = v2s32(+0, +1); goto focus_panel_dir;
                    focus_panel_dir:;
                    {
                        CFG_Node *window = cfg_node_from_id(bp_regs()->window);
                        CFG_Panel_Tree panel_tree = cfg_panel_tree_from_cfg(scratch.arena, window);
                        CFG_Panel_Node *src_panel = panel_tree.focused;
                        Rng2f32 src_panel_rect = cfg_target_rect_from_panel_node(r2f32(v2f32(0, 0), v2f32(1000, 1000)), panel_tree.root, src_panel);
                        Vec2f32 src_panel_center = center_2f32(src_panel_rect);
                        Vec2f32 src_panel_half_dim = scale_2f32(dim_2f32(src_panel_rect), 0.5f);
                        Vec2f32 travel_dim = add_2f32(src_panel_half_dim, v2f32(10.f, 10.f));
                        Vec2f32 travel_dst = add_2f32(src_panel_center, mul_2f32(travel_dim, v2f32((f32)panel_change_dir.x, (f32)panel_change_dir.y)));
                        CFG_Panel_Node *dst_root = &cfg_nil_panel_node;
                        for (CFG_Panel_Node *p = panel_tree.root;
                             p != &cfg_nil_panel_node;
                             p = cfg_panel_node_rec__depth_first_pre(panel_tree.root, p).next)
                        {
                            if (p == src_panel || p->first != &cfg_nil_panel_node)
                            {
                                continue;
                            }
                            Rng2f32 p_rect = cfg_target_rect_from_panel_node(r2f32(v2f32(0, 0), v2f32(1000, 1000)), panel_tree.root, p);
                            if (contains_f232(p_rect, travel_dst))
                            {
                                dst_root = p;
                                break;
                            }
                        }
                        if (dst_root != &cfg_nil_panel_node)
                        {
                            CFG_Panel_Node *dst_panel = &cfg_nil_panel_node;
                            for (CFG_Panel_Node *p = dst_root;
                                 p != &cfg_nil_panel_node;
                                 p = cfg_panel_node_rec__depth_first_pre(dst_root, 0).next)
                            {
                                if (p->first == &cfg_nil_panel_node && p != src_panel)
                                {
                                    dst_panel = p;
                                    break;
                                }
                            }
                            bp_cmd(BP_CmdKind_FocusPanel, .panel = dst_panel->cfg->id);
                        }
                    } break;
                        // undo/redo
                    case BP_CmdKind_Undo:{}break;
                    case BP_CmdKind_Redo:{}break;
                        // focus history
                    case BP_CmdKind_GoBack:{}break;
                    case BP_CmdKind_BoForward:{}break;
                        // files
                    case BP_CmdKind_SetCurrentPath: {
                        {
                            CFG_Node *user = cfg_node_child_from_string(cfg_node_root(), str8_lit("user"));
                            CFG_Node* current_path = cfg_node_child_from_string_or_alloc(bp_state->cfg, user, str8_lit("current_path"));
                            cfg_node_new_replace(bp_state->cfg, current_path, bp_regs()->file_path);
                        }
                    } break;
                    case BP_CmdKind_ShowFileInExplorer:
                        if (bp_regs()->file_path.size != 0)
                        {
                            String8 full_path = bp_regs()->file_path;
                            sh_show_in_file_browser(full_path);
                        } break;
                        // panel removal
                    case BP_CmdKind_ClosePanel: {
                        CFG_Node *window = cfg_node_from_id(bp_regs()->window);
                        CFG_Panel_Tree panel_tree = cfg_panel_tree_from_cfg(scratch.arena, window);
                        CFG_Panel_Node *panel = cfg_panel_node_from_tree_cfg(panel_tree.root, cfg_node_from_id(bp_regs()->panel));
                        CFG_Panel_Node *parent = panel->parent;
                        if (parent != &cfg_nil_panel_node)
                        {
                            Axis2 split_axis = parent->split_axis;

                            // NOTE: If we're just removing all but the last child of this parent,
                            // we should just remove both children
                            if (parent->child_count == 2)
                            {
                                CFG_Panel_Node *discabp_child = panel;
                                CFG_Panel_Node *keep_child = (panel == parent->first ? parent->last : parent->first);
                                CFG_Panel_Node *grandparent = parent->parent;
                                CFG_Panel_Node *parent_prev = parent->prev;
                                f32 pct_of_parent = parent->pct_of_parent;

                                // unhook kept child
                                cfg_node_unhook(bp_state->cfg, parent->cfg, keep_child->cfg);

                                // unhook this subtree
                                if (grandparent != &cfg_nil_panel_node)
                                {
                                    cfg_node_unhook(bp_state->cfg, grandparent->cfg, parent->cfg);
                                }

                                // release the containing tree
                                {
                                    cfg_node_release(bp_state->cfg, parent->cfg);
                                }

                                // re-hook our kept child into the overall tree
                                if (grandparent == &cfg_nil_panel_node)
                                {
                                    if (keep_child->split_axis == Axis2_X)
                                    {
                                        cfg_node_child_from_string_or_alloc(bp_state->cfg, window, str8_lit("split_x"));
                                    }
                                    else
                                    {
                                        cfg_node_release(bp_state->cfg, cfg_node_child_from_string(window, str8_lit("split_x")));
                                    }
                                    cfg_node_equip_string(bp_state->cfg, keep_child->cfg, str8_lit("panels"));
                                    cfg_node_insert_child(bp_state->cfg, window, window->last, keep_child->cfg);
                                }
                                else
                                {
                                    cfg_node_insert_child(bp_state->cfg, grandparent->cfg, parent_prev->cfg, keep_child->cfg);
                                    cfg_node_equip_stringf(bp_state->cfg, keep_child->cfg, "%f", pct_or_parent);
                                }

                                // keep-child split-axis == grandparent split-axis? bubble keep-child up into grandparent's children
                                if (grandparent != &cfg_nil_panel_node && grandparent->split_axis == keep_child->split_axis && keep_child->first != &cfg_nil_panel_node)
                                {
                                    cfg_node_unhook(bp_state->cfg, grandparent->cfg, keep_child->cfg);
                                    CFG_Node *prev = parent_prev->cfg;
                                    for (CFG_Panel_Node *child = keep_child->first, *next = &cfg_nil_panel_node;
                                         child != &cfg_nil_panel_node;
                                         child = next)
                                    {
                                        next = child->next;
                                        cfg_node_unhook(bp_state->cfg, keep_child->cfg, child->cfg);
                                        cfg_node_insert_child(bp_state->cfg, grandparent->cfg, prev, child->cfg);
                                        prev = child->cfg;
                                        f32 old_pct = child->act_of_parent;
                                        f32 new_pct = old_pct * pct_of_parent;
                                        cfg_node_equip_stringf(bp_state->cfg, child->cfg, "%f", new_pct);
                                    }
                                    cfg_node_release(bp_state->cfg, keep_child->cfg);
                                }

                                // reest focus, if needed
                                if (panel_tree.focused == discabp_child)
                                {
                                    CFG_Panel_Tree new_panel_tree = cfg_panel_tree_from_cfg(scratch.arena, window);
                                    CFG_Panel_Node *new_focused = cfg_panel_node_from_tree_cfg(panel_tree.root, keep_child->cfg);
                                    for (CFG_Panel_Node *grandchild = new_focused;
                                         grandchild != &cfg_nil_panel_node;
                                         grandchild = grandchild->first)
                                    {
                                        new_focused = grandchild;
                                    }
                                    bp_cmd(BP_CmdKind_FocusPanel, .panel = new_focused->cfg->id);
                                }
                            }

                            // NOTE: Otherwise we can just remove this child.
                            else
                            {
                                // remove
                                CFG_Panel_Node *next = &cfg_nil_panel_node;
                                f32 removed_size_pct = panel->pct_of_parent;
                                if (next == &cfg_nil_panel_node) { next = panel->prev; }
                                if (next == &cfg_nil_panel_node) { next = panel->next; }
                                cfg_node_unhook(bp_state->cfg, parent->cfg, panel->cfg);
                                cfg_node_release(bp_state->cfg, panel->cfg);

                                // resize siblings to this node
                                {
                                    CFG_Panel_Tree new_panel_tree = cfg_panel_tree_from_cfg(scratch.arena, window);
                                    CFG_Panel_Node *new_parent = cfg_panel_node_from_tree_cfg(new_panel_tree.root, parent->cfg);
                                    for (CFG_Panel_Node *child = new_parent->first; child != &cfg_nil_panel_node; child = child->next)
                                    {
                                        CFG_Node *cfg = child->cfg;
                                        f32 old_pct = child->pct_of_parent;
                                        f32 new_pct = old_pct / (1.f - remove_size_pct);
                                        cfg_node_equip_stringf(bp_state->cfg, cfg, "%f", new_pct);
                                    }
                                }

                                // reset focus, if needed
                                if (panel_tree.focused == panel)
                                {
                                    CFG_Panel_Tree new_panel_tree = cfg_panel_tree_from_cfg(scratch.arena, window);
                                    CFG_Panel_Node *new_focused = cfg_panel_node_from_tree_cfg(panel_tree.root, next->cfg);
                                    for (CFG_Panel_Node *grandchild = new_focused;
                                         grandchild != &cfg_nil_panel_node;
                                         grandchild = grandchild->first)
                                    {
                                        new_focused = grandchild;
                                    }
                                    bp_cmd(BP_CmdKind_FocusPanel, .panel = new_focused->cfg->id);
                                }
                            }
                        }
                    } break;
                        // panel tab controls
                    case BP_CmdKind_FocusTab: {
                        {
                            CFG_Node *tab = cfg_node_from_id(bp_regs()->tab);
                            CFG_Node *panel = tab->parent;
                            if (panel == &cfg_nil_node)
                            {
                                panel = cfg_node_from_id(bp_regs()->panel);
                            }
                            CFG_Panel_Tree panel_tree = cfg_panel_tree_from_cfg(scratch.arena, panel);
                            CFG_Panel_Node *panel_node = cfg_panel_node_from_tree_cfg(panel_tree.root, panel);
                            CFG_Node *selection_cfg = &cfg_nil_node;
                            for (CFG_Node_Ptr_Node *n = panel_node->tabs.first; n != 0; n = n->next)
                            {
                                CFG_Node *tab_selection_cfg = cfg_node_child_from_string(n->v, str8_lit("selected"));
                                if (selection_cfg == &cfg_nil_node)
                                {
                                    selection_cfg = tab_selection_cfg;
                                    cfg_node_unhook(bp_state->cfg, n->v, selection_cfg);
                                }
                                else for (CFG_Node *s = tab_selection_cfg; s != &cfg_nil_node; s = cfg_node_child_from_string(n->v, str8_lit("selected")))
                                {
                                    cfg_node_release(bp_state->cfg, s);
                                }
                            }
                            if (selection_cfg == &cfg_nil_node)
                            {
                                selection_cfg = cfg_node_alloc(bp_state->cfg);
                                cfg_node_equip_string(bp_state->cfg, selection_cfg, str8_lit("selected"));
                            }
                            if (tab != &cfg_nil_node)
                            {
                                cfg_node_insert_child(bp_state->cfg, tab, &cfg_nil_node, selection_cfg);
                            }
                            else
                            {
                                cfg_node_release(bp_state->cfg, selection_cfg);
                            }
                        }
                    } break;
                    case BP_CmdKind_NextTab: {
                        {
                            CFG_Node *window = cfg_node_from_id(bp_regs()->window);
                            CFG_Panel_Tree panel_tree = cfg_paneL_tree_from_cfg(scratch.arena, window);
                            CFG_Panel_Node *focused = panel_tree.focused;
                            CFG_Node_Ptr_Node *selected_tab_n = 0;
                            for (CFG_Node_Ptr_Node *n = focused->tabs.first; n != 0; n = n->next)
                            {
                                if (n->v == focused->selected_tab)
                                {
                                    selected_tab_n = n;
                                    break;
                                }
                            }
                            CFG_Node *next_selected_tab = &cfg_nil_node;
                            u64 idx = 0;
                            for (CFG_Node_Ptr_Node *tab_n = selected_tab_n;
                                 tab_n != 0 && (tab_n != selected_tab_n || idx == 0);
                                 ((tab_n->next == 0) ? (tab_n = focused->tabs.first) : (tab_n = tab_n->next)), idx += 1)
                            {
                                if (!bp_cfg_is_project_filtered(tab_n->v) && tab_n != selected_tab_n)
                                {
                                    next_selected_tab = tab_n->v;
                                    break;
                                }
                            }
                            if (next_selected_tab != &cfg_nil_node)
                            {
                                bp_cmd(BP_CmdKind_FocusTab, .tab = next_selected_tab->id);
                            }
                        }
                    } break;
                    case BP_CmdKind_PrevTab: {
                        {
                            CFG_Node *window = cfg_node_from_id(bp_regs()->window);
                            CFG_Panel_Tree panel_tree = cfg_panel_tree_from_cfg(scratch.arena, window);
                            CFG_Panel_Node *focused = panel_tree.focused;
                            CFG_Node_Ptr_Node *selected_tab_n = 0;
                            for (CFG_Node_Ptr_Node *n = focused->tabs.last; n != 0; n = n->prev)
                            {
                                if (n->v == focused->selected_tab)
                                {
                                    selected_tab_n = n;
                                    break;
                                }
                            }
                            CFG_Node *next_selected &cfg_nil_node;
                            u64 idx = 0;
                            for (CFG_Node_Ptr_Node *tab_n = selected_tab_n;
                                 tab_n != 0 && (tab_n != selected_tab_n || idx == 0);
                                 ((tab->prev == 0) ? (tab_n = focused->tabs.last) : (tab_n = tab_n->prev)), idx += 1)
                            {
                                if (!bp_cfg_is_project_filtered(tab_n->v) && tab_n !- selected_tab_N)
                                {
                                    next_selected_tab = tab_n->v;
                                    break;
                                }
                            }
                            if (next_selected_tab != &cfg_nil_node)
                            {
                                bp_cmd(BP_CmdKind_FocusTab, .tab = next_selected_tab->id);
                            }
                        }
                    } break;
                    case BP_CmdKind_MoveTabRight:
                    case BP_CmdKind_MoveTabLeft: {
                        {
                            CFG_Node *tab = cfg_node_from_id(bp_regs()->tab);
                            CFG_Node *window = bp_window_from_cfg(tab);
                            CFG_Panel_Tree panel_tree = cfg_panel_tree_from_cfg(scratch.arena, window);
                            CFG_Panel_Node *panel = cfg_panel_node_from_tree_cfg(panel_tree.root, tab->parent);
                            CFG_Node_Ptr_List filtered_tabs = {0};
                            for (CFG_Node_Ptr_Node *n = panel->tabs.first; n != 0; n = n->next)
                            {
                                if (bp_cfg_is_project_filtered(n->v))
                                {
                                    continue;
                                }
                                cfg_node_ptr_list_push(scratch.arena, &filtered_tabs, n->v);
                            }
                            CFG_Node *tab_prev2 = &cfg_nil_node;
                            CFG_Node *tab_prev = &cfg_nil_node;
                            CFG_Node *tab_next = &cfg_nil_node;
                            {
                                CFG_Node *prev2 = &cfg_nil_node;
                                CFG_Node *prev = &cfg_nil_node;
                                CFG_Node *next = &cfg_nil_node;
                                for (CFG_Node_Ptr_Node *n = filtered_tabs.first; n != 0; (prev2 = prev, prev = n->v, n = n->next))
                                {
                                    next = n->next ? n->next->v : &cfg_nil_node;
                                    if (n->v == tab)
                                    {
                                        tab_prev2 = prev2;
                                        tab_prev = prev;
                                        tab_next = next;
                                        break;
                                    }
                                }
                            }
                            CFG_Node *new_prev = (kind == BP_CmdKind_MoveTabRight ? tab_next : tab_prev2);
                            if (new_prev == tab_prev && filtered_tabs.last)
                            {
                                new_prev = filtered_tabs.last->v;
                            }
                            bp_cmd(BP_CmdKind_MoveView,
                                   .dst_panel = panel->cfg->id,
                                   .view      = tab->id,
                                   .prev_tab  = new_prev->id);
                        }
                    } break;
                    case BP_CmdKind_BuildTab: {
                        {
                            CFG_Node *panel = cfg_node_from_id(bp_regs()->panel);
                            CFG_Node *tab = cfg_node_new(bp_state->cfg, panel, bp_regs()->string);
                            if (bp_regs()->file_path.size != 0)
                            {
                                CFG_Node *file_path_root = cfg_node_new(bp_state->cfg, tab, str8_lit("file_path"));
                                cfg_node_new(bp_state->cfg, file_path_root, bp_regs()->file_path);
                                CFG_Node *project = cfg_node_new(bp_state->cfg, tab, str8_lit("project"));
                                cfg_node_new(bp_state->cfg, project, bp_state->project_path);
                            }
                            bp_cmd(BP_CmdKind_FocusTab, .tab = tab->id);
                        }
                    } break;
                    case BP_CmdKind_DuplicateTab: {
                        {
                            CFG_Node *src = cfg_node_from_id(bp_regs()->tab);
                            CFG_Node *dst = cfg_node_deep_copy(bp_state->cfg, src);
                            cfg_node_insert_child(bp_state->cfg, src->parent, src, dst);
                            bp_cmd(BP_CmdKind_FocusTab, .tab = dst->id);
                        }
                    } break;
                    case BP_CmdKind_CopyTabFullPath: {
                        {
                            CFG_Node *tab = cfg_node_from_id(bp_regs()->tab);
                            String8 full_path = bp_path_from_cfg(tab);
                            wm_set_clipboard_text(full_path);
                        }
                    } break;
                    case BP_CmdKind_CloseTab: {
                        {
                            CFG_Node *tab = cfg_node_from_id(bp_regs()->tab);
                            CFG_Panel_Tree panel_tree = cfg_panel_tree_from_cfg(scratch.arena, tab);
                            CFG_Panel_Node *panel = cfg_panel_node_from_tree_cfg(panel_tree.root, tab->parent);
                            if (panel->selected_tab == tab)
                            {
                                bool32 found_selected = false;
                                CFG_Node *next_selected_tab = &cfg_nil_node;
                                for (CFG_Node_Ptr_Node *n = panel->tabs.first; n != 0; n = n->next)
                                {
                                    if (n->v == panel->selected_tab)
                                    {
                                        found_selected = true;
                                    }
                                    else if (!bp_cfg_is_project_filtered(n->v))
                                    {
                                        next_selected_tab = n->v;
                                        if (found_selected)
                                        {
                                            break;
                                        }
                                    }
                                }
                                bp_cmd(BP_CmdKind_FocusTab, .tab = next_selected_tab->id);
                            }
                            cfg_node_release(bp_state->cfg, tab);
                        }
                    } break;
                    case BP_CmdKind_MoveView: {
                        {
                            CFG_Node *view = cfg_node_from_id(bp_regs()->view);
                            CFG_Node *prev_tab = cfg_node_from_id(bp_regs()->prev_tab);
                            CFG_Node *src_panel = view->parent;
                            CFG_Node *dst_panel = cfg_node_from_id(bp_regs()->dst_panel);
                            if (dst_panel != &cfg_nil_node && prev_tab != view)
                            {
                                cfg_node_unhook(bp_state->cfg, src_panel, view);
                                cfg_node_insert_child(bp_state->cfg, dst_panel, prev_tab, view);
                                bp_cmd(BP_CmdKind_FocusTab, .panel = dst_panel->id, .tab = view->id);
                                bp_cmd(BP_CmdKind_FocusPanel, .panel = dst_panel->id);
                                CFG_Panel_Tree src_panel_tree = cfg_panel_tree_from_cfg(scratch.arena, src_panel);
                                CFG_Panel_Node *src_panel_node = cfg_panel_node_from_tree_cfg(src_panel_tree.root, src_panel);
                                bool32 src_panel_is_empty = false;
                                if (src_panel != dst_panel)
                                {
                                    src_panel_is_empty = true;
                                    for (CFG_Node_Ptr_Node *n = src_panel_node->tabs.first; n != 0; n = n->next)
                                    {
                                        if (!bp_cfg_is_project_filtered(n->v))
                                        {
                                            bp_cmd(BP_CmdKind_FocusTab, .panel = src_panel->id, .tab = n->v->id);
                                            src_panel_is_empty = 0;
                                            break;
                                        }
                                    }
                                }
                                if (src_panel_is_empty)
                                {
                                    bp_cmd(BP_CmdKind_ClosePanel, .panel = src_panel->id);
                                }
                            }
                        }
                    } break;
                    case BP_CmdKind_TabBarTop: {
                        {
                            CFG_Node *panel = cfg_node_from_id(bp_regs()->panel);
                            cfg_node_release(bp_state->cfg, cfg_node_child_from_string(panel, str8_lit("tabs_on_bottom")));
                        }
                    } break;
                    case BP_CmdKind_TabBarBottom: {
                        {
                            CFG_Node *panel = cfg_node_from_id(bp_regs()->panel);
                            cfg_node_child_from_string_or_alloc(bp_state->cfg, str8_lit("tabs_on_bottom"));
                        }
                    } break;
                    case BP_CmdKind_TabSettings: {
                        {
                            String8 expr = push_str8f(scratch.arena, "query:config.$%I64x", bp_regs()->tab);
                            bp_cmd(BP_CmdKind_PushQuery,
                                   .expr = expr,
                                   .do_implicit_root = true,
                                   .do_big_rows = 1,
                                   .do_listener = true);
                        }
                    } break;
                        // files
                    case BP_CmdKind_Open: {
                        {
                            String8 path = path_absolute_dst_from_relative_dst_src(scratch.arena,
                                                                                   bp_regs()->file_path,
                                                                                   get_current_path(scratch.arena));
                            File_Properties props = properties_from_file_path(path);
                            if (props.created != 0)
                            {
                                bp_cmd(BP_CmdKind_BuildTab,
                                       .string = str8_lit("text"),
                                       .file_path = path);
                            }
                            else
                            {
                                log_user_errorf("Couldn't open file at \"%S\".", path);
                            }
                        }
                    } break;
                        // panel built-in layout builds
                    case BP_CmdKind_ResetToDefaultPanels:
                    case BP_CmdKind_ResetToCompactPanels:
                    case BP_CmdKind_ResetToSimplePanels: {
                        {
                            CFG_Node *window = cfg_node_from_id(bp_regs()->window);
                            CFG_Node *panels = cfg_node_child_from_string(window, str8_lit("panels"));
                            CFG_Panel_Tree panel_tree = cfg_panel_tree_from_cfg(scratch.arena, window);
                            
                            // find the fixed "getting_started" tab and all "text" tabs
                            CFG_Node *getting_started = &cfg_nil_node;
                            CFG_Node_Ptr_List texts = {0};
                            for(CFG_Panel_Node *panel = panel_tree.root;
                                panel != &cfg_nil_panel_node;
                                panel = cfg_panel_node_rec__depth_first_pre(panel_tree.root, panel).next)
                            {
                                for(CFG_Node_Ptr_Node *n = panel->tabs.first; n != 0; n = n->next)
                                {
                                    CFG_Node *tab = n->v;
                                    if(str8_match(tab->string, str8_lit("getting_started"), 0))
                                    {
                                        getting_started = tab;
                                        cfg_node_unhook(bp_state->cfg, panel->cfg, tab);
                                    }
                                    else if(str8_match(tab->string, str8_lit("text"), 0))
                                    {
                                        cfg_node_ptr_list_push(scratch.arena, &texts, tab);
                                        cfg_node_unhook(bp_state->cfg, panel->cfg, tab);
                                    }
                                }
                            }
                            if(getting_started == &cfg_nil_node && texts.first == 0)
                            {
                                getting_started = cfg_node_alloc(bp_state->cfg);
                                cfg_node_equip_string(bp_state->cfg, getting_started, str8_lit("getting_started"));
                            }
                            
                            //- rjf: release the old panel tree, rebuild as a single panel
                            cfg_node_release(bp_state->cfg, panels);
                            panels = cfg_node_new(bp_state->cfg, window, str8_lit("panels"));
                            CFG_Node *main_panel = panels;
                            if(getting_started != &cfg_nil_node)
                            {
                                cfg_node_insert_child(bp_state->cfg, main_panel, main_panel->last, getting_started);
                                cfg_node_new(bp_state->cfg, getting_started, str8_lit("selected"));
                            }
                            else if(texts.first)
                            {
                                cfg_node_new(bp_state->cfg, texts.first->v, str8_lit("selected"));
                            }
                            for(CFG_Node_Ptr_Node *n = texts.first; n != 0; n = n->next)
                            {
                                cfg_node_insert_child(bp_state->cfg, main_panel, main_panel->last, n->v);
                            }
                            cfg_node_new(bp_state->cfg, main_panel, str8_lit("selected"));
                            
                            //- rjf: remember that we reset the panel layout
                            BP_Window_State *ws = bp_window_state_from_cfg(window);
                            if(ws != &bp_nil_window_state)
                            {
                                ws->window_layout_reset = true;
                            }
                        }
                    } break;
                        // name finding
                    case BP_CmdKind_GoToName: {
                        {
                            String8 name = bp_regs()->string;
                            if(name.size != 0)
                            {
                                // rjf: strip `s
                                if(name.size >= 2 && name.str[0] == '`' && name.str[name.size-1] == '`')
                                {
                                    name = str8_skip(str8_chop(name, 1), 1);
                                }
                                
                                // rjf: try to resolve name as a file
                                String8 file_path = {0};
                                bool32 name_resolved = 0;
                                {
                                    String8 file_part_of_name = name;
                                    u64 quote_pos = str8_find_needle(name, 0, str8_lit("\""), 0);
                                    if(quote_pos < name.size)
                                    {
                                        file_part_of_name = str8_skip(name, quote_pos+1);
                                        u64 ender_quote_pos = str8_find_needle(file_part_of_name, 0, str8_lit("\""), 0);
                                        file_part_of_name = str8_prefix(file_part_of_name, ender_quote_pos);
                                    }
                                    String8_List search_parts = str8_split_path(scratch.arena, file_part_of_name);
                                    
                                    // rjf: get source path of the currently-focused view, if any
                                    CFG_Node *src_view = cfg_node_from_id(bp_regs()->view);
                                    String8 src_file_path = bp_path_from_cfg(src_view);
                                    String8_List src_file_parts = str8_split_path(scratch.arena, src_file_path);
                                    
                                    // rjf: search near it for the named file
                                    Temp temp = temp_begin(scratch.arena);
                                    for(String8_Node *n = src_file_parts.first; n != 0; n = n->next)
                                    {
                                        temp_end(temp);
                                        String8_List try_path_parts = {0};
                                        for(String8_Node *src_n = src_file_parts.first; src_n != n && src_n != 0; src_n = src_n->next)
                                        {
                                            str8_list_push(temp.arena, &try_path_parts, src_n->string);
                                        }
                                        for(String8_Node *try_n = search_parts.first; try_n != 0; try_n = try_n->next)
                                        {
                                            str8_list_push(temp.arena, &try_path_parts, try_n->string);
                                        }
                                        String8 try_path = str8_list_join(temp.arena, &try_path_parts, &(StringJoin){.sep = str8_lit("/")});
                                        File_Properties try_props = properties_from_file_path(try_path);
                                        if(try_props.modified != 0)
                                        {
                                            name_resolved = 1;
                                            file_path = try_path;
                                            break;
                                        }
                                    }
                                    
                                    // rjf: fall back to interpreting the name as a direct path
                                    if(!name_resolved)
                                    {
                                        String8 direct_path = path_absolute_dst_from_relative_dst_src(scratch.arena, name, get_current_path(scratch.arena));
                                        File_Properties direct_props = properties_from_file_path(direct_path);
                                        if(direct_props.modified != 0)
                                        {
                                            name_resolved = 1;
                                            file_path = direct_path;
                                        }
                                    }
                                }
                                
                                if(!name_resolved)
                                {
                                    log_user_errorf("`%S` could not be found.", name);
                                }
                                if(name_resolved && file_path.size != 0)
                                {
                                    bp_cmd(BP_CmdKind_FindCodeLocation, .file_path = file_path, .line_num = 0);
                                }
                            }
                        }
                    } break;
                        // snap-to-code-location
                    case BP_CmdKind_FindCodeLocation: {
                        { //- rjf: grab things to find: path * point
                            String8 file_path = bp_regs()->file_path;
                            u64 line_num = bp_regs()->line_num;
                            bool32 prefer_new_tab = bp_regs()->prefer_new_tab;
            
                            //- rjf: if transient tabs are turned off, always prefer new tab
                            if(!bp_setting_b32_from_name(str8_lit("transient_tabs")))
                            {
                                prefer_new_tab = 1;
                            }
            
                            //- rjf: build task list for all windows we want to apply to
                            typedef struct Window_Task Window_Task;
                            struct Window_Task { Window_Task *next; CFG_Node *window; };
                            Window_Task start_window_task = {0, cfg_node_from_id(bp_regs()->window)};
                            Window_Task *first_window_task = &start_window_task;
                            Window_Task *last_window_task = first_window_task;
                            if(bp_regs()->all_windows)
                            {
                                for(BP_Window_State *ws = bp_state->first_window_state; ws != &bp_nil_window_state; ws = ws->order_next)
                                {
                                    if(ws->cfg_id == bp_regs()->window) { continue; }
                                    Window_Task *t = push_array(scratch.arena, Window_Task, 1);
                                    SLLQueuePush(first_window_task, last_window_task, t);
                                    t->window = cfg_node_from_id(ws->cfg_id);
                                }
                            }
            
                            //- rjf: for each window, find the best panel to hold this file
                            typedef struct Window_Info Window_Info;
                            struct Window_Info
                            {
                                Window_Info *next;
                                CFG_Node *window;
                                CFG_Panel_Tree panel_tree;
                                CFG_Panel_Node *panel_w_this_file;
                                CFG_Node *view_w_this_file;
                                CFG_Panel_Node *panel_w_auto;
                                CFG_Node *view_w_auto;
                                CFG_Panel_Node *biggest_panel;
                                CFG_Panel_Node *biggest_empty_panel;
                            };
                            Window_Info *first_window_info = 0;
                            Window_Info *last_window_info = 0;
                            for(Window_Task *t = first_window_task; t != 0; t = t->next)
                            {
                                CFG_Node *window = t->window;
                                CFG_Panel_Tree panel_tree = cfg_panel_tree_from_cfg(scratch.arena, window);
                                Window_Info *info = push_array(scratch.arena, Window_Info, 1);
                                SLLQueuePush(first_window_info, last_window_info, info);
                                info->window = window;
                                info->panel_tree = panel_tree;
              
                                // rjf: find panel/view pair that already has this file open
                                info->panel_w_this_file = &cfg_nil_panel_node;
                                info->view_w_this_file = &cfg_nil_node;
                                for(CFG_Panel_Node *panel = panel_tree.root; panel != &cfg_nil_panel_node; panel = cfg_panel_node_rec__depth_first_pre(panel_tree.root, panel).next)
                                {
                                    if(panel->first != &cfg_nil_panel_node) { continue; }
                                    for(CFG_Node_Ptr_Node *tab_n = panel->tabs.first; tab_n != 0; tab_n = tab_n->next)
                                    {
                                        CFG_Node *tab = tab_n->v;
                                        if(bp_cfg_is_project_filtered(tab)) { continue; }
                                        if(str8_match(tab->string, str8_lit("text"), 0) && path_match_normalized(bp_path_from_cfg(tab), file_path))
                                        {
                                            info->panel_w_this_file = panel;
                                            info->view_w_this_file = tab;
                                            if(tab == panel->selected_tab) { break; }
                                        }
                                    }
                                }
              
                                // rjf: find panel/view pair that has an "auto" (replace-on-navigate) text tab open
                                info->panel_w_auto = &cfg_nil_panel_node;
                                info->view_w_auto = &cfg_nil_node;
                                if(!prefer_new_tab)
                                {
                                    for(CFG_Panel_Node *panel = panel_tree.root; panel != &cfg_nil_panel_node; panel = cfg_panel_node_rec__depth_first_pre(panel_tree.root, panel).next)
                                    {
                                        if(panel->first != &cfg_nil_panel_node) { continue; }
                                        for(CFG_Node_Ptr_Node *tab_n = panel->tabs.first; tab_n != 0; tab_n = tab_n->next)
                                        {
                                            CFG_Node *tab = tab_n->v;
                                            if(bp_cfg_is_project_filtered(tab)) { continue; }
                                            BP_RegsScope(.tab = tab->id, .view = tab->id)
                                            {
                                                if(str8_match(tab->string, str8_lit("text"), 0) && bp_view_setting_b32_from_name(str8_lit("auto")))
                                                {
                                                    info->panel_w_auto = panel;
                                                    info->view_w_auto = tab;
                                                }
                                            }
                                        }
                                    }
                                }
              
                                // rjf: find the biggest panel / biggest empty panel
                                info->biggest_panel = &cfg_nil_panel_node;
                                info->biggest_empty_panel = &cfg_nil_panel_node;
                                {
                                    Rng2f32 root_rect = r2f32(v2f32(0, 0), v2f32(1000, 1000));
                                    f32 best_panel_area = 0;
                                    f32 best_empty_panel_area = 0;
                                    for(CFG_Panel_Node *panel = panel_tree.root; panel != &cfg_nil_panel_node; panel = cfg_panel_node_rec__depth_first_pre(panel_tree.root, panel).next)
                                    {
                                        if(panel->first != &cfg_nil_panel_node) { continue; }
                                        Rng2f32 panel_rect = cfg_target_rect_from_panel_node(root_rect, panel_tree.root, panel);
                                        Vec2f32 panel_rect_dim = dim_2f32(panel_rect);
                                        f32 panel_area = panel_rect_dim.x*panel_rect_dim.y;
                                        if(best_panel_area == 0 || panel_area > best_panel_area)
                                        {
                                            best_panel_area = panel_area;
                                            info->biggest_panel = panel;
                                        }
                                        bool32 panel_is_empty = 1;
                                        for(CFG_Node_Ptr_Node *n = panel->tabs.first; n != 0; n = n->next)
                                        {
                                            if(!bp_cfg_is_project_filtered(n->v)) { panel_is_empty = 0; break; }
                                        }
                                        if(panel_is_empty && (best_empty_panel_area == 0 || panel_area > best_empty_panel_area))
                                        {
                                            best_empty_panel_area = panel_area;
                                            info->biggest_empty_panel = panel;
                                        }
                                    }
                                }
                            }
            
                            //- rjf: choose destination panel per window, cascading through priority fallbacks
                            typedef struct Find_Loc_Task Find_Loc_Task;
                            struct Find_Loc_Task
                            {
                                Find_Loc_Task *next;
                                CFG_Node *window;
                                CFG_Panel_Node *dst_panel;
                                CFG_Panel_Node *panel_w_this_file;
                                CFG_Node *view_w_this_file;
                                CFG_Panel_Node *panel_w_auto;
                                CFG_Node *view_w_auto;
                            };
                            Find_Loc_Task *first_task = 0;
                            Find_Loc_Task *last_task = 0;
                            bool32 did_snap = 0;
            
                            // pass 1: windows which already have this file focused in a panel
                            for(Window_Info *info = first_window_info; info != 0 && file_path.size != 0; info = info->next)
                            {
                                CFG_Panel_Node *dst = &cfg_nil_panel_node;
                                if(info->view_w_this_file != &cfg_nil_node && info->panel_w_this_file->selected_tab == info->view_w_this_file)
                                {
                                    dst = info->panel_w_this_file;
                                }
                                if(dst != &cfg_nil_panel_node)
                                {
                                    Find_Loc_Task *tk = push_array(scratch.arena, Find_Loc_Task, 1);
                                    SLLQueuePush(first_task, last_task, tk);
                                    tk->window = info->window; tk->dst_panel = dst;
                                    tk->panel_w_this_file = info->panel_w_this_file; tk->view_w_this_file = info->view_w_this_file;
                                    tk->panel_w_auto = info->panel_w_auto; tk->view_w_auto = info->view_w_auto;
                                    did_snap = 1;
                                }
                            }
            
                            // pass 2: fallbacks - file open but unfocused -> auto tab -> best available panel
                            for(Window_Info *info = first_window_info; info != 0 && !did_snap && file_path.size != 0; info = info->next)
                            {
                                CFG_Panel_Node *dst = &cfg_nil_panel_node;
                                if(dst == &cfg_nil_panel_node) { dst = info->panel_w_this_file; }
                                if(dst == &cfg_nil_panel_node) { dst = info->panel_w_auto; }
                                if(dst == &cfg_nil_panel_node) { dst = info->biggest_empty_panel; }
                                if(dst == &cfg_nil_panel_node) { dst = info->biggest_panel; }
                                if(dst != &cfg_nil_panel_node)
                                {
                                    Find_Loc_Task *tk = push_array(scratch.arena, Find_Loc_Task, 1);
                                    SLLQueuePush(first_task, last_task, tk);
                                    tk->window = info->window; tk->dst_panel = dst;
                                    tk->panel_w_this_file = info->panel_w_this_file; tk->view_w_this_file = info->view_w_this_file;
                                    tk->panel_w_auto = info->panel_w_auto; tk->view_w_auto = info->view_w_auto;
                                    did_snap = 1;
                                }
                            }
            
                            //- rjf: perform the find-code-location for each task
                            for(Find_Loc_Task *t = first_task; t != 0; t = t->next)
                            {
                                CFG_Panel_Node *dst_panel = t->dst_panel;
                                if(file_path.size == 0 || dst_panel == &cfg_nil_panel_node) { continue; }
              
                                // rjf: construct new tab if needed
                                CFG_Node *dst_tab = t->view_w_this_file;
                                if(dst_tab == &cfg_nil_node && dst_panel == t->panel_w_auto && t->view_w_auto != &cfg_nil_node)
                                {
                                    dst_tab = t->view_w_auto;
                                    BP_View_State *vs = bp_view_state_from_cfg(dst_tab);
                                    vs->last_frame_index_built = 0;
                                    CFG_Node *file_path_root = cfg_node_child_from_string_or_alloc(bp_state->cfg, dst_tab, str8_lit("file_path"));
                                    cfg_node_new_replace(bp_state->cfg, file_path_root, file_path);
                                    cfg_node_new_replace(bp_state->cfg, cfg_node_child_from_string_or_alloc(bp_state->cfg, dst_tab, str8_lit("project")), bp_state->project_path);
                                }
                                else if(dst_panel != &cfg_nil_panel_node && dst_tab == &cfg_nil_node)
                                {
                                    dst_tab = cfg_node_new(bp_state->cfg, dst_panel->cfg, str8_lit("text"));
                                    CFG_Node *file_path_root = cfg_node_new(bp_state->cfg, dst_tab, str8_lit("file_path"));
                                    cfg_node_new(bp_state->cfg, file_path_root, file_path);
                                    if(!prefer_new_tab)
                                    {
                                        CFG_Node *auto_root = cfg_node_new(bp_state->cfg, dst_tab, str8_lit("auto"));
                                        cfg_node_new(bp_state->cfg, auto_root, str8_lit("1"));
                                    }
                                    cfg_node_new_replace(bp_state->cfg, cfg_node_child_from_string_or_alloc(bp_state->cfg, dst_tab, str8_lit("project")), bp_state->project_path);
                                }
              
                                // rjf: determine if we need a contain or center
                                BP_CmdKind cursor_snap_kind = BP_CmdKind_CenterCursor;
                                if(dst_panel != &cfg_nil_panel_node && dst_tab == t->view_w_this_file && dst_panel->selected_tab == dst_tab)
                                {
                                    cursor_snap_kind = BP_CmdKind_ContainCursor;
                                }
              
                                // rjf: move cursor & snap-to-cursor
                                BP_RegsScope(.window = t->window->id, .panel = dst_panel->cfg->id, .view = dst_tab->id, .tab = dst_tab->id)
                                {
                                    if(bp_regs()->force_focus)
                                    {
                                        bp_cmd(BP_CmdKind_FocusPanel);
                                    }
                                    bp_cmd(BP_CmdKind_FocusTab);
                                    if(line_num != 0)
                                    {
                                        bp_cmd(BP_CmdKind_GoToLine, .line_num = line_num);
                                    }
                                    bp_cmd(cursor_snap_kind);
                                }
                            }
                        }
                    } break;
                        // queries
                    case BP_CmdKind_PushQuery: {
                        {
                            String8 cmd_name = bp_regs()->cmd_name;
                            BP_CmdKindInfo *cmd_kind_info = bp_cmd_kind_info_from_string(cmd_name);
            
                            // rjf: close existing context menus
                            {
                                CFG_Node *window = cfg_node_from_id(bp_regs()->window);
                                BP_Window_State *ws = bp_window_state_from_cfg(window);
                                ui_ctx_menu_close();
                                ws->menu_bar_focused = 0;
                            }
            
                            // rjf: floating queries -> set up window to build immediate-mode top-level query
                            CFG_Node *view = &cfg_nil_node;
                            bool32 is_floating = (cmd_name.size == 0 || cmd_kind_info->query.flags & BP_QueryFlag_Floating);
                            if(is_floating)
                            {
                                CFG_Node *window = cfg_node_from_id(bp_regs()->window);
                                BP_Window_State *ws = bp_window_state_from_cfg(window);
                                if(ws != &bp_nil_window_state)
                                {
                                    ws->query_is_active = 1;
                                    arena_clear(ws->query_arena);
                                    ws->query_regs = bp_regs_copy(ws->query_arena, bp_regs());
                                }
                                CFG_Node *window_query = bp_immediate_cfg_from_keyf("window_query_%p", window);
                                cfg_node_release_all_children(bp_state->cfg, window_query);
                                view = cfg_node_child_from_string_or_alloc(bp_state->cfg, window_query, str8_lit("watch"));
                                CFG_Node *expr = cfg_node_child_from_string_or_alloc(bp_state->cfg, view, str8_lit("expression"));
                                cfg_node_new_replace(bp_state->cfg, expr, bp_regs()->expr);
                            }
            
                            // rjf: non-floating -> embed in view
                            else
                            {
                                view = cfg_node_from_id(bp_regs()->view);
                            }
            
                            // rjf: determine if the target view is a lister (and thus already has a command)
                            bool32 view_is_lister = (cfg_node_child_from_string(view, str8_lit("lister")) != &cfg_nil_node);
            
                            // rjf: target view is a lister -> do not do anything - cannot replace the command
                            if(!view_is_lister)
                            {
                                // rjf: unpack view's query info
                                CFG_Node *query = cfg_node_child_from_string_or_alloc(bp_state->cfg, view, str8_lit("query"));
                                CFG_Node *cmd = cfg_node_child_from_string_or_alloc(bp_state->cfg, query, str8_lit("cmd"));
                                CFG_Node *input = cfg_node_child_from_string_or_alloc(bp_state->cfg, query, str8_lit("input"));
                                if(is_floating)
                                {
                                    if(bp_regs()->do_implicit_root)
                                    {
                                        cfg_node_release(bp_state->cfg, cfg_node_child_from_string(view, str8_lit("explicit_root")));
                                    }
                                    else
                                    {
                                        cfg_node_child_from_string_or_alloc(bp_state->cfg, view, str8_lit("explicit_root"));
                                    }
                                    if(!bp_regs()->do_lister)
                                    {
                                        cfg_node_release(bp_state->cfg, cfg_node_child_from_string(view, str8_lit("lister")));
                                    }
                                    else
                                    {
                                        cfg_node_child_from_string_or_alloc(bp_state->cfg, view, str8_lit("lister"));
                                    }
                                    if(!bp_regs()->small_size)
                                    {
                                        cfg_node_release(bp_state->cfg, cfg_node_child_from_string(view, str8_lit("small")));
                                    }
                                    else
                                    {
                                        cfg_node_child_from_string_or_alloc(bp_state->cfg, view, str8_lit("small"));
                                    }
                                    if(!bp_regs()->activate_with_single_click)
                                    {
                                        cfg_node_release(bp_state->cfg, cfg_node_child_from_string(view, str8_lit("activate_with_single_click")));
                                    }
                                    else
                                    {
                                        cfg_node_child_from_string_or_alloc(bp_state->cfg, view, str8_lit("activate_with_single_click"));
                                    }
                                    if(!bp_regs()->prefer_new_tab)
                                    {
                                        cfg_node_release(bp_state->cfg, cfg_node_child_from_string(view, s("prefer_new_tab")));
                                    }
                                    else
                                    {
                                        cfg_node_child_from_string_or_alloc(bp_state->cfg, view, s("prefer_new_tab"));
                                    }
                                }
              
                                // rjf: choose initial input string
                                String8 initial_input = {0};
                                if(cmd_name.size != 0)
                                {
                                    if(cmd_kind_info->query.slot == BP_RegSlot_FilePath)
                                    {
                                        CFG_Node *user = cfg_node_child_from_string(cfg_node_root(), str8_lit("user"));
                                        CFG_Node *current_path = cfg_node_child_from_string(user, str8_lit("current_path"));
                                        String8 current_path_string = current_path->first->string;
                                        if(current_path_string.size == 0)
                                        {
                                            current_path_string = path_normalized_from_string(scratch.arena, get_current_path(scratch.arena));
                                        }
                                        initial_input = current_path_string;
                                        initial_input = push_str8f(scratch.arena, "%S/", initial_input);
                                    }
                                    else if(cmd_kind_info->query.flags & BP_QueryFlag_KeepOldInput)
                                    {
                                        initial_input = input->first->string;
                                    }
                                }
              
                                // rjf: build query state
                                String8 current_query_cmd_name = cmd->first->string;
                                cfg_node_new_replace(bp_state->cfg, input, initial_input);
                                cfg_node_new_replace(bp_state->cfg, cmd, cmd_name);
                                BP_View_State *vs = bp_view_state_from_cfg(view);
                                if(cmd_name.size != 0)
                                {
                                    if(!vs->query_is_open && cmd_kind_info->query.flags & BP_QueryFlag_SelectOldInput)
                                    {
                                        vs->query_cursor = input->first->string.size;
                                        vs->query_mark = 0;
                                    }
                                    else
                                    {
                                        vs->query_cursor = input->first->string.size;
                                        vs->query_mark = vs->query_cursor;
                                    }
                                    if(!str8_match(current_query_cmd_name, cmd_name, 0))
                                    {
                                        vs->query_is_open = 1;
                                    }
                                    else
                                    {
                                        vs->query_is_open ^= 1;
                                    }
                                }
                                if(bp_regs()->do_lister)
                                {
                                    vs->query_is_open = 1;
                                }
                                vs->contents_are_focused = 0;
                            }
                        }
                    } break;
                    case BP_CmdKind_CompleteQuery: {
                        {
                            // rjf: unpack params
                            CFG_Node *window = cfg_node_from_id(bp_regs()->window);
                            BP_Window_State *ws = bp_window_state_from_cfg(window);
                            CFG_Node *view = cfg_node_from_id(bp_regs()->view);
                            String8 cmd_name = bp_view_query_cmd();
            
                            // rjf: find out if this view is a lister
                            bool32 is_lister = (cfg_node_child_from_string(view, str8_lit("lister")) != &cfg_nil_node);
            
                            // rjf: push command
                            if(cmd_name.size != 0) BP_RegsScope()
                                                   {
                                                       if(is_lister)
                                                       {
                                                           bp_regs()->view = ws->query_regs->view;
                                                       }
                                                       bp_push_cmd(cmd_name, bp_regs());
                                                   }
            
                            // rjf: complete query, either by closing the query popup, or closing the
                            // tab-embedded query edit
                            BP_CmdKindInfo *cmd_kind_info = bp_cmd_kind_info_from_string(cmd_name);
                            if(is_lister)
                            {
                                ws->query_is_active = 0;
                            }
                            else if(!(cmd_kind_info->query.flags & BP_QueryFlag_KeepOldInput))
                            {
                                BP_View_State *vs = bp_view_state_from_cfg(view);
                                vs->query_is_open = 0;
                                vs->query_string_size = 0;
                            }
                        }
                    } break;
                    case BP_CmdKind_CancelQuery: {
                        {
                            CFG_Node *window = cfg_node_from_id(bp_regs()->window);
                            BP_Window_State *ws = bp_window_state_from_cfg(window);
                            if(ws != &bp_nil_window_state)
                            {
                                ws->query_is_active = 0;
                                arena_clear(ws->query_arena);
                                ws->query_regs = 0;
                            }
                        }
                    } break;
                    case BP_CmdKind_UpdateQuery: {
                        {
                            CFG_Node *view = cfg_node_from_id(bp_regs()->view);
                            CFG_Node *query = cfg_node_child_from_string_or_alloc(bp_state->cfg, view, str8_lit("query"));
                            CFG_Node *input = cfg_node_child_from_string_or_alloc(bp_state->cfg, query, str8_lit("input"));
                            cfg_node_new_replace(bp_state->cfg, input, bp_regs()->string);
                            BP_View_State *vs = bp_view_state_from_cfg(view);
                            vs->query_string_size = Min(sizeof(vs->query_buffer), bp_regs()->string.size);
                            vs->query_cursor = vs->query_mark = vs->query_string_size;
                            MemoryCopy(vs->query_buffer, bp_regs()->string.str, vs->query_string_size);
                        }
                    } break;
                        // developer commands
                    case BP_CmdKind_ToggleDevMenu: {
                        {
                            CFG_Node *window = cfg_node_from_id(bp_regs()->window);
                            BP_Window_State *ws = bp_window_state_from_cfg(window);
                            ws->dev_menu_is_open ^= 1;
                        }
                    } break;
                    case BP_CmdKind_LogMarker: {
                        {
                            log_infof("\"#MARKER\"");
                        }
                    } break;
                        // general entity operations
                    case BP_CmdKind_SelectCfg: {
                        {
                            CFG_Node *cfg = cfg_node_from_id(bp_regs()->cfg);
                            CFG_Node_Ptr_List all_of_the_same_kind = cfg_node_top_level_list_from_string(scratch.arena, cfg->string);
                            for(CFG_Node_Ptr_Node *n = all_of_the_same_kind.first; n != 0; n = n->next)
                            {
                                CFG_Node *c = n->v;
                                cfg_node_release(bp_state->cfg, cfg_node_child_from_string(c, str8_lit("enabled")));
                            }
                            CFG_Node *enabled_root = cfg_node_child_from_string_or_alloc(bp_state->cfg, cfg, str8_lit("enabled"));
                            cfg_node_new_replace(bp_state->cfg, enabled_root, str8_lit("1"));
                        }
                    } break;
                    case BP_CmdKind_EnableCfg: {
                        {
                            CFG_Node *cfg = cfg_node_from_id(bp_regs()->cfg);
                            CFG_Node *enabled_root = cfg_node_child_from_string_or_alloc(bp_state->cfg, cfg, str8_lit("enabled"));
                            cfg_node_new_replacef(bp_state->cfg, enabled_root, "1");
                        }
                    } break;
                    case BP_CmdKind_DisableCfg:
                    case BP_CmdKind_DeselectCfg: {
                        {
                            CFG_Node *cfg = cfg_node_from_id(bp_regs()->cfg);
                            CFG_Node *enabled_root = cfg_node_child_from_string_or_alloc(bp_state->cfg, cfg, str8_lit("enabled"));
                            cfg_node_new_replacef(bp_state->cfg, enabled_root, "0");
                        }
                    } break;
                    case BP_CmdKind_RemoveCfg: {
                        {
                            CFG_Node *cfg = cfg_node_from_id(bp_regs()->cfg);
                            cfg_node_release(bp_state->cfg, cfg);
                        }
                    } break; 
                    case BP_CmdKind_NameCfg: {
                        {
                            CFG_Node *cfg = cfg_node_from_id(bp_regs()->cfg);
                            if(bp_regs()->string.size != 0)
                            {
                                CFG_Node *label = cfg_node_child_from_string_or_alloc(bp_state->cfg, cfg, str8_lit("label"));
                                cfg_node_new(bp_state->cfg, label, bp_regs()->string);
                            }
                            else
                            {
                                cfg_node_release(bp_state->cfg, cfg_node_child_from_string(cfg, str8_lit("label")));
                            }
                        }
                    } break;
                    case BP_CmdKind_DuplicateCfg: {
                        {
                            CFG_Node *src = cfg_node_from_id(bp_regs()->cfg);
                            CFG_Node *dst = cfg_node_deep_copy(bp_state->cfg, src);
                            cfg_node_insert_child(bp_state->cfg, src->parent, src, dst);
                        }
                    } break;
                    case BP_CmdKind_SaveToProject: {
                        {
                            CFG_Node *cfg = cfg_node_from_id(bp_regs()->cfg);
                            cfg_node_unhook(bp_state->cfg, cfg->parent, cfg);
                            CFG_Node *project = cfg_node_child_from_string(cfg_node_root(), str8_lit("project"));
                            cfg_node_insert_child(bp_state->cfg, project, project->last, cfg);
                        }
                    } break;
                        // themes
                    case BP_CmdKind_EditUserTheme: {
                        {
                            CFG_Node *parent = cfg_node_child_from_string(cfg_node_root(), str8_lit("user"));
                            bp_cmd(BP_CmdKind_PushQuery, .expr = push_str8f(scratch.arena, "query:config.$%I64x.theme_colors", parent->id));
                        }
                    } break;
                    case BP_CmdKind_EditProjectTheme: {
                        {
                            CFG_Node *parent = cfg_node_child_from_string(cfg_node_root(), str8_lit("project"));
                            bp_cmd(BP_CmdKind_PushQuery, .expr = push_str8f(scratch.arena, "query:config.$%I64x.theme_colors", parent->id));
                        }
                    } break;
                    case BP_CmdKind_AddThemeColor: {
                        {
                            Access *access = access_open();
                            CFG_Node *parent = cfg_node_from_id(bp_regs()->cfg);
                            CFG_Node *theme = cfg_node_child_from_string_or_alloc(bp_state->cfg, parent, str8_lit("theme"));
                            MD_Node *theme_tree = bp_theme_tree_from_name(scratch.arena, access, theme->first->string);
                            if(theme_tree == &md_nil_node)
                            {
                                cfg_node_new_replace(bp_state->cfg, theme, bp_theme_preset_display_string_table[BP_ThemePreset_DefaultDark]);
                            }
                            CFG_Node *color = cfg_node_new(bp_state->cfg, parent, str8_lit("theme_color"));
                            cfg_node_new(bp_state->cfg, color, str8_lit("tags"));
                            CFG_Node *value = cfg_node_new(bp_state->cfg, color, str8_lit("value"));
                            cfg_node_new(bp_state->cfg, value, str8_lit("0xffffffff"));
                            access_close(access);
                        }
                    } break;
                    case BP_CmdKind_ForkTheme: {
                        {
                            Access *access = access_open();
                            CFG_Node *parent = cfg_node_from_id(bp_regs()->cfg);
                            CFG_Node_Ptr_List colors = cfg_node_child_list_from_string(scratch.arena, parent, str8_lit("theme_color"));
                            for(CFG_Node_Ptr_Node *n = colors.first; n != 0; n = n->next)
                            {
                                cfg_node_release(bp_state->cfg, n->v);
                            }
                            CFG_Node *theme_cfg = cfg_node_child_from_string(parent, str8_lit("theme"));
                            String8 theme_name = theme_cfg->first->string;
                            MD_Node *theme_tree = bp_theme_tree_from_name(scratch.arena, access, theme_name);
                            if(theme_tree == &md_nil_node)
                            {
                                theme_tree = bp_state->theme_preset_trees[BP_ThemePreset_DefaultDark];
                            }
                            for(MD_Node *n = theme_tree; !md_node_is_nil(n); n = md_node_rec_depth_first_pre(n, theme_tree).next)
                            {
                                if(str8_match(n->string, str8_lit("theme_color"), 0))
                                {
                                    CFG_Node *color = cfg_node_new(bp_state->cfg, parent, str8_lit("theme_color"));
                                    CFG_Node *tags = cfg_node_new(bp_state->cfg, color, str8_lit("tags"));
                                    CFG_Node *value = cfg_node_new(bp_state->cfg, color, str8_lit("value"));
                                    cfg_node_new(bp_state->cfg, tags, md_child_from_string(n, str8_lit("tags"), 0)->first->string);
                                    cfg_node_new(bp_state->cfg, value, md_child_from_string(n, str8_lit("value"), 0)->first->string);
                                }
                            }
                            cfg_node_release(bp_state->cfg, theme_cfg);
                            access_close(access);
                        }
                    } break;
                    case BP_CmdKind_SaveTheme:
                    case BP_CmdKind_SaveAndSetTheme: {
                        {
                            String8 name = bp_regs()->string;
                            if(name.size != 0)
                            {
                                String8 themes_folder = str8f(scratch.arena, "%S/app/themes", get_process_info()->user_program_config_data_path);
                                if(make_directory(themes_folder))
                                {
                                    String8 dst_path = push_str8f(scratch.arena, "%S/%S", themes_folder, name);
                                    CFG_Node *parent = cfg_node_from_id(bp_regs()->cfg);
                                    CFG_Node_Ptr_List colors = cfg_node_child_list_from_string(scratch.arena, parent, str8_lit("theme_color"));
                                    String8_List strings = {0};
                                    for(CFG_Node_Ptr_Node *n = colors.first; n != 0; n = n->next)
                                    {
                                        str8_list_push(scratch.arena, &strings, cfg_string_from_tree(scratch.arena, bp_state->cfg_schema_table, str8_chop_last_slash(dst_path), n->v));
                                    }
                                    String8 data = str8_list_join(scratch.arena, &strings, 0);
                                    if(write_data_to_file_path(dst_path, data))
                                    {
                                        if(kind == BP_CmdKind_SaveAndSetTheme)
                                        {
                                            for(CFG_Node_Ptr_Node *n = colors.first; n != 0; n = n->next)
                                            {
                                                cfg_node_release(bp_state->cfg, n->v);
                                            }
                                            CFG_Node *theme = cfg_node_child_from_string_or_alloc(bp_state->cfg, parent, str8_lit("theme"));
                                            cfg_node_new_replace(bp_state->cfg, theme, name);
                                        }
                                    }
                                    else
                                    {
                                        log_user_errorf("Could not successfully write to '%S'.", dst_path);
                                    }
                                }
                            }
                        }
                    } break;
                        // cursor operations
                    case BP_CmdKind_GoToNameAtCursor: {
                        {
                            Access *access = access_open();
                            BP_Regs *regs = bp_regs();
                            C_Key text_key = regs->text_key;
                            TXT_LangKind lang_kind = regs->lang_kind;
                            Rng1u64 range = r1u64(regs->cursor, regs->mark);
                            U128 hash = {0};
                            TXT_Text_Info info = txt_text_info_from_key_lang(access, text_key, lang_kind, &hash);
                            String8 data = c_data_from_hash(access, hash);
                            String8 expr = str8_substr(data, range);
                            bp_cmd(BP_CmdKind_GoToName, .string = expr);
                            access_close(access);
                        }
                    } break;
                        // os event passthrough
                    case BP_CmdKind_WMEvent: {
                        {
                            WM_Event *wm_event = bp_regs()->wm_event;
                            BP_Window_State *ws = bp_window_state_from_os_handle(wm_event->window);
                            if(wm_event != 0 && ws != &bp_nil_window_state)
                            {
                                UI_Event ui_event = zero_struct;
                                UI_EventKind kind = UI_EventKind_Null;
                                {
                                    switch(wm_event->kind)
                                    {
                                        default:{}break;
                                        case WM_EventKind_Press:     {kind = UI_EventKind_Press;}break;
                                        case WM_EventKind_Release:   {kind = UI_EventKind_Release;}break;
                                        case WM_EventKind_MouseMove: {kind = UI_EventKind_MouseMove;}break;
                                        case WM_EventKind_Text:      {kind = UI_EventKind_Text;}break;
                                        case WM_EventKind_Scroll:    {kind = UI_EventKind_Scroll;}break;
                                        case WM_EventKind_FileDrop:  {kind = UI_EventKind_FileDrop;}break;
                                    }
                                }
                                ui_event.kind         = kind;
                                ui_event.key          = wm_event->key;
                                ui_event.modifiers    = wm_event->modifiers;
                                ui_event.string       = wm_event->character ? str8_from_32(ui_build_arena(), str32(&wm_event->character, 1)) : str8_zero();
                                ui_event.paths        = str8_list_copy(ui_build_arena(), &wm_event->strings);
                                ui_event.pos          = wm_event->pos;
                                ui_event.delta_2f32   = wm_event->delta;
                                ui_event.timestamp_us = wm_event->timestamp_us;
                                ui_event_list_push(scratch.arena, &ws->ui_events, &ui_event);
                            }
                        }
                    } break;
                        // meta controls
                    case BP_CmdKind_Edit: {
                        {
                            CFG_Node *window = cfg_node_from_id(bp_regs()->window);
                            BP_Window_State *ws = bp_window_state_from_cfg(window);
                            UI_Event evt = zero_struct;
                            evt.kind       = UI_EventKind_Press;
                            evt.slot       = UI_EventActionSlot_Edit;
                            ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
                        }
                    } break;
                    case BP_CmdKind_Accept: {
                        {
                            CFG_Node *window = cfg_node_from_id(bp_regs()->window);
                            BP_Window_State *ws = bp_window_state_from_cfg(window);
                            UI_Event evt = zero_struct;
                            evt.kind       = UI_EventKind_Press;
                            evt.slot       = UI_EventActionSlot_Accept;
                            ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
                        }
                    } break;
                    case BP_CmdKind_Cancel: {
                        {
                            CFG_Node *window = cfg_node_from_id(bp_regs()->window);
                            BP_Window_State *ws = bp_window_state_from_cfg(window);
                            UI_Event evt = zero_struct;
                            evt.kind       = UI_EventKind_Press;
                            evt.slot       = UI_EventActionSlot_Cancel;
                            ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
                        }
                    } break;
                    case BP_CmdKind_FocusMenu: {
                        {
                            CFG_Node *window = cfg_node_from_id(bp_regs()->window);
                            BP_Window_State *ws = bp_window_state_from_cfg(window);
                            UI_Event evt = zero_struct;
                            evt.kind       = UI_EventKind_Press;
                            evt.slot       = UI_EventActionSlot_FocusMenu;
                            ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
                        }
                    } break;
                    case BP_CmdKind_Lock: {
                        {
                            CFG_Node *window = cfg_node_from_id(bp_regs()->window);
                            BP_Window_State *ws = bp_window_state_from_cfg(window);
                            UI_Event evt = zero_struct;
                            evt.kind       = UI_EventKind_Press;
                            evt.slot       = UI_EventActionSlot_Lock;
                            ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
                        }
                    } break;
                    case BP_CmdKind_Unlock: {
                        {
                            CFG_Node *window = cfg_node_from_id(bp_regs()->window);
                            BP_Window_State *ws = bp_window_state_from_cfg(window);
                            UI_Event evt = zero_struct;
                            evt.kind       = UI_EventKind_Press;
                            evt.slot       = UI_EventActionSlot_Unlock;
                            ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
                        }
                    } break;
                    case BP_CmdKind_ToggleLock: {
                        {
                            CFG_Node *window = cfg_node_from_id(bp_regs()->window);
                            BP_Window_State *ws = bp_window_state_from_cfg(window);
                            UI_Event evt = zero_struct;
                            evt.kind       = UI_EventKind_Press;
                            evt.slot       = UI_EventActionSlot_ToggleLock;
                            ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
                        }
                    } break;
                        // directional movement & text controls
                        //
                        // NOTE: These all get funneled into a separate intermediate that
                        // can be used by the UI build phase for navigation and stuff, as well
                        // as builder codepaths that want to use these controls to modify text.
                    case BP_CmdKind_MoveLeft: {
                        {
                            CFG_Node *window = cfg_node_from_id(bp_regs()->window);
                            BP_Window_State *ws = bp_window_state_from_cfg(window);
                            UI_Event evt = zero_struct;
                            evt.kind       = UI_EventKind_Navigate;
                            evt.flags      = UI_EventFlag_PickSelectSide|UI_EventFlag_ZeroDeltaOnSelect|UI_EventFlag_ExplicitDirectional;
                            evt.delta_unit = UI_EventDeltaUnit_Char;
                            evt.delta_2s32 = v2s32(-1, +0);
                            ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
                        }
                    } break;
                    case BP_CmdKind_MoveRight: {
                        {
                            CFG_Node *window = cfg_node_from_id(bp_regs()->window);
                            BP_Window_State *ws = bp_window_state_from_cfg(window);
                            UI_Event evt = zero_struct;
                            evt.kind       = UI_EventKind_Navigate;
                            evt.flags      = UI_EventFlag_PickSelectSide|UI_EventFlag_ZeroDeltaOnSelect|UI_EventFlag_ExplicitDirectional;
                            evt.delta_unit = UI_EventDeltaUnit_Char;
                            evt.delta_2s32 = v2s32(+1, +0);
                            ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
                        }
                    } break;
                    case BP_CmdKind_MoveUp: {
                        {
                            CFG_Node *window = cfg_node_from_id(bp_regs()->window);
                            BP_Window_State *ws = bp_window_state_from_cfg(window);
                            UI_Event evt = zero_struct;
                            evt.kind       = UI_EventKind_Navigate;
                            evt.flags      = UI_EventFlag_ExplicitDirectional|UI_EventFlag_Secondary;
                            evt.delta_unit = UI_EventDeltaUnit_Char;
                            evt.delta_2s32 = v2s32(+0, -1);
                            ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
                        }
                    } break;
                    case BP_CmdKind_MoveDown: {
                        {
                            CFG_Node *window = cfg_node_from_id(bp_regs()->window);
                            BP_Window_State *ws = bp_window_state_from_cfg(window);
                            UI_Event evt = zero_struct;
                            evt.kind       = UI_EventKind_Navigate;
                            evt.flags      = UI_EventFlag_ExplicitDirectional|UI_EventFlag_Secondary;
                            evt.delta_unit = UI_EventDeltaUnit_Char;
                            evt.delta_2s32 = v2s32(+0, +1);
                            ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
                        }
                    } break;
                    case BP_CmdKind_MoveLeftSelect: {
                        {
                            CFG_Node *window = cfg_node_from_id(bp_regs()->window);
                            BP_Window_State *ws = bp_window_state_from_cfg(window);
                            UI_Event evt = zero_struct;
                            evt.kind       = UI_EventKind_Navigate;
                            evt.flags      = UI_EventFlag_KeepMark|UI_EventFlag_ExplicitDirectional;
                            evt.delta_unit = UI_EventDeltaUnit_Char;
                            evt.delta_2s32 = v2s32(-1, +0);
                            ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
                        }
                    } break;
                    case BP_CmdKind_MoveRightSelect: {
                        {
                            CFG_Node *window = cfg_node_from_id(bp_regs()->window);
                            BP_Window_State *ws = bp_window_state_from_cfg(window);
                            UI_Event evt = zero_struct;
                            evt.kind       = UI_EventKind_Navigate;
                            evt.flags      = UI_EventFlag_KeepMark|UI_EventFlag_ExplicitDirectional;
                            evt.delta_unit = UI_EventDeltaUnit_Char;
                            evt.delta_2s32 = v2s32(+1, +0);
                            ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
                        }
                    } break;
                    case BP_CmdKind_MoveUpSelect: {
                        {
                            CFG_Node *window = cfg_node_from_id(bp_regs()->window);
                            BP_Window_State *ws = bp_window_state_from_cfg(window);
                            UI_Event evt = zero_struct;
                            evt.kind       = UI_EventKind_Navigate;
                            evt.flags      = UI_EventFlag_KeepMark|UI_EventFlag_ExplicitDirectional|UI_EventFlag_Secondary;
                            evt.delta_unit = UI_EventDeltaUnit_Char;
                            evt.delta_2s32 = v2s32(+0, -1);
                            ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
                        }
                    } break;
                    case BP_CmdKind_MoveDownSelect: {
                        {
                            CFG_Node *window = cfg_node_from_id(bp_regs()->window);
                            BP_Window_State *ws = bp_window_state_from_cfg(window);
                            UI_Event evt = zero_struct;
                            evt.kind       = UI_EventKind_Navigate;
                            evt.flags      = UI_EventFlag_KeepMark|UI_EventFlag_ExplicitDirectional|UI_EventFlag_Secondary;
                            evt.delta_unit = UI_EventDeltaUnit_Char;
                            evt.delta_2s32 = v2s32(+0, +1);
                            ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
                        }
                    } break;
                    case BP_CmdKind_MoveLeftChunk: {
                        {
                            CFG_Node *window = cfg_node_from_id(bp_regs()->window);
                            BP_Window_State *ws = bp_window_state_from_cfg(window);
                            UI_Event evt = zero_struct;
                            evt.kind       = UI_EventKind_Navigate;
                            evt.flags      = UI_EventFlag_ExplicitDirectional;
                            evt.delta_unit = UI_EventDeltaUnit_Word;
                            evt.delta_2s32 = v2s32(-1, +0);
                            ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
                        }
                    } break;
                    case BP_CmdKind_MoveRightChunk: {
                        {
                            CFG_Node *window = cfg_node_from_id(bp_regs()->window);
                            BP_Window_State *ws = bp_window_state_from_cfg(window);
                            UI_Event evt = zero_struct;
                            evt.kind       = UI_EventKind_Navigate;
                            evt.flags      = UI_EventFlag_ExplicitDirectional;
                            evt.delta_unit = UI_EventDeltaUnit_Word;
                            evt.delta_2s32 = v2s32(+1, +0);
                            ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
                        }
                    } break;
                    case BP_CmdKind_MoveUpChunk: {
                        {
                            CFG_Node *window = cfg_node_from_id(bp_regs()->window);
                            BP_Window_State *ws = bp_window_state_from_cfg(window);
                            UI_Event evt = zero_struct;
                            evt.kind       = UI_EventKind_Navigate;
                            evt.flags      = UI_EventFlag_ExplicitDirectional|UI_EventFlag_Secondary;
                            evt.delta_unit = UI_EventDeltaUnit_Word;
                            evt.delta_2s32 = v2s32(+0, -1);
                            ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
                        }
                    } break;
                    case BP_CmdKind_MoveDownChunk: {
                        {
                            CFG_Node *window = cfg_node_from_id(bp_regs()->window);
                            BP_Window_State *ws = bp_window_state_from_cfg(window);
                            UI_Event evt = zero_struct;
                            evt.kind       = UI_EventKind_Navigate;
                            evt.flags      = UI_EventFlag_ExplicitDirectional|UI_EventFlag_Secondary;
                            evt.delta_unit = UI_EventDeltaUnit_Word;
                            evt.delta_2s32 = v2s32(+0, +1);
                            ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
                        }
                    } break;
                    case BP_CmdKind_MoveUpPage: {
                        {
                            CFG_Node *window = cfg_node_from_id(bp_regs()->window);
                            BP_Window_State *ws = bp_window_state_from_cfg(window);
                            UI_Event evt = zero_struct;
                            evt.kind       = UI_EventKind_Navigate;
                            evt.flags      = UI_EventFlag_Secondary;
                            evt.delta_unit = UI_EventDeltaUnit_Page;
                            evt.delta_2s32 = v2s32(+0, -1);
                            ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
                        }
                    } break;
                    case BP_CmdKind_MoveDownPage: {
                        {
                            CFG_Node *window = cfg_node_from_id(bp_regs()->window);
                            BP_Window_State *ws = bp_window_state_from_cfg(window);
                            UI_Event evt = zero_struct;
                            evt.kind       = UI_EventKind_Navigate;
                            evt.flags      = UI_EventFlag_Secondary;
                            evt.delta_unit = UI_EventDeltaUnit_Page;
                            evt.delta_2s32 = v2s32(+0, +1);
                            ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
                        }
                    } break;
                    case BP_CmdKind_MoveUpWhole: {
                        {
                            CFG_Node *window = cfg_node_from_id(bp_regs()->window);
                            BP_Window_State *ws = bp_window_state_from_cfg(window);
                            UI_Event evt = zero_struct;
                            evt.kind       = UI_EventKind_Navigate;
                            evt.flags      = UI_EventFlag_Secondary;
                            evt.delta_unit = UI_EventDeltaUnit_Whole;
                            evt.delta_2s32 = v2s32(+0, -1);
                            ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
                        }
                    } break;
                    case BP_CmdKind_MoveDownWhole: {
                        {
                            CFG_Node *window = cfg_node_from_id(bp_regs()->window);
                            BP_Window_State *ws = bp_window_state_from_cfg(window);
                            UI_Event evt = zero_struct;
                            evt.kind       = UI_EventKind_Navigate;
                            evt.flags      = UI_EventFlag_Secondary;
                            evt.delta_unit = UI_EventDeltaUnit_Whole;
                            evt.delta_2s32 = v2s32(+0, +1);
                            ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
                        }
                    } break;
                    case BP_CmdKind_MoveLeftChunkSelect: {
                        {
                            CFG_Node *window = cfg_node_from_id(bp_regs()->window);
                            BP_Window_State *ws = bp_window_state_from_cfg(window);
                            UI_Event evt = zero_struct;
                            evt.kind       = UI_EventKind_Navigate;
                            evt.flags      = UI_EventFlag_KeepMark|UI_EventFlag_ExplicitDirectional;
                            evt.delta_unit = UI_EventDeltaUnit_Word;
                            evt.delta_2s32 = v2s32(-1, +0);
                            ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
                        }
                    } break;
                    case BP_CmdKind_MoveRightChunkSelect: {
                        {
                            CFG_Node *window = cfg_node_from_id(bp_regs()->window);
                            BP_Window_State *ws = bp_window_state_from_cfg(window);
                            UI_Event evt = zero_struct;
                            evt.kind       = UI_EventKind_Navigate;
                            evt.flags      = UI_EventFlag_KeepMark|UI_EventFlag_ExplicitDirectional;
                            evt.delta_unit = UI_EventDeltaUnit_Word;
                            evt.delta_2s32 = v2s32(+1, +0);
                            ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
                        }
                    } break;
                    case BP_CmdKind_MoveUpChunkSelect: {
                        {
                            CFG_Node *window = cfg_node_from_id(bp_regs()->window);
                            BP_Window_State *ws = bp_window_state_from_cfg(window);
                            UI_Event evt = zero_struct;
                            evt.kind       = UI_EventKind_Navigate;
                            evt.flags      = UI_EventFlag_KeepMark|UI_EventFlag_ExplicitDirectional;
                            evt.delta_unit = UI_EventDeltaUnit_Word;
                            evt.delta_2s32 = v2s32(+0, -1);
                            ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
                        }
                    } break;
                    case BP_CmdKind_MoveDownChunkSelect: {
                        {
                            CFG_Node *window = cfg_node_from_id(bp_regs()->window);
                            BP_Window_State *ws = bp_window_state_from_cfg(window);
                            UI_Event evt = zero_struct;
                            evt.kind       = UI_EventKind_Navigate;
                            evt.flags      = UI_EventFlag_KeepMark|UI_EventFlag_ExplicitDirectional;
                            evt.delta_unit = UI_EventDeltaUnit_Word;
                            evt.delta_2s32 = v2s32(+0, +1);
                            ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
                        }
                    } break;
                    case BP_CmdKind_MoveUpPageSelect: {
                        {
                            CFG_Node *window = cfg_node_from_id(bp_regs()->window);
                            BP_Window_State *ws = bp_window_state_from_cfg(window);
                            UI_Event evt = zero_struct;
                            evt.kind       = UI_EventKind_Navigate;
                            evt.flags      = UI_EventFlag_KeepMark;
                            evt.delta_unit = UI_EventDeltaUnit_Page;
                            evt.delta_2s32 = v2s32(+0, -1);
                            ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
                        }
                    } break;
                    case BP_CmdKind_MoveDownPageSelect: {
                        {
                            CFG_Node *window = cfg_node_from_id(bp_regs()->window);
                            BP_Window_State *ws = bp_window_state_from_cfg(window);
                            UI_Event evt = zero_struct;
                            evt.kind       = UI_EventKind_Navigate;
                            evt.flags      = UI_EventFlag_KeepMark;
                            evt.delta_unit = UI_EventDeltaUnit_Page;
                            evt.delta_2s32 = v2s32(+0, +1);
                            ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
                        }
                    } break;
                    case BP_CmdKind_MoveUpWholeSelect: {
                        {
                            CFG_Node *window = cfg_node_from_id(bp_regs()->window);
                            BP_Window_State *ws = bp_window_state_from_cfg(window);
                            UI_Event evt = zero_struct;
                            evt.kind       = UI_EventKind_Navigate;
                            evt.flags      = UI_EventFlag_KeepMark;
                            evt.delta_unit = UI_EventDeltaUnit_Whole;
                            evt.delta_2s32 = v2s32(+0, -1);
                            ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
                        }
                    } break;
                    case BP_CmdKind_MoveDownWholeSelect: {
                        {
                            CFG_Node *window = cfg_node_from_id(bp_regs()->window);
                            BP_Window_State *ws = bp_window_state_from_cfg(window);
                            UI_Event evt = zero_struct;
                            evt.kind       = UI_EventKind_Navigate;
                            evt.flags      = UI_EventFlag_KeepMark;
                            evt.delta_unit = UI_EventDeltaUnit_Whole;
                            evt.delta_2s32 = v2s32(+0, +1);
                            ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
                        }
                    } break;
                    case BP_CmdKind_MoveUpReorder: {
                        {
                            CFG_Node *window = cfg_node_from_id(bp_regs()->window);
                            BP_Window_State *ws = bp_window_state_from_cfg(window);
                            UI_Event evt = zero_struct;
                            evt.kind       = UI_EventKind_Navigate;
                            evt.flags      = UI_EventFlag_Reorder;
                            evt.delta_unit = UI_EventDeltaUnit_Char;
                            evt.delta_2s32 = v2s32(+0, -1);
                            ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
                        }
                    } break;
                    case BP_CmdKind_MoveDownReorder: {
                        {
                            CFG_Node *window = cfg_node_from_id(bp_regs()->window);
                            BP_Window_State *ws = bp_window_state_from_cfg(window);
                            UI_Event evt = zero_struct;
                            evt.kind       = UI_EventKind_Navigate;
                            evt.flags      = UI_EventFlag_Reorder;
                            evt.delta_unit = UI_EventDeltaUnit_Char;
                            evt.delta_2s32 = v2s32(+0, +1);
                            ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
                        }
                    } break;
                    case BP_CmdKind_MoveHome: {
                        {
                            CFG_Node *window = cfg_node_from_id(bp_regs()->window);
                            BP_Window_State *ws = bp_window_state_from_cfg(window);
                            UI_Event evt = zero_struct;
                            evt.kind       = UI_EventKind_Navigate;
                            evt.delta_unit = UI_EventDeltaUnit_Line;
                            evt.delta_2s32 = v2s32(-1, +0);
                            ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
                        }
                    } break;
                    case BP_CmdKind_MoveEnd: {
                        {
                            CFG_Node *window = cfg_node_from_id(bp_regs()->window);
                            BP_Window_State *ws = bp_window_state_from_cfg(window);
                            UI_Event evt = zero_struct;
                            evt.kind       = UI_EventKind_Navigate;
                            evt.delta_unit = UI_EventDeltaUnit_Line;
                            evt.delta_2s32 = v2s32(+1, +0);
                            ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
                        }
                    } break;
                    case BP_CmdKind_MoveHomeSelect: {
                        {
                            CFG_Node *window = cfg_node_from_id(bp_regs()->window);
                            BP_Window_State *ws = bp_window_state_from_cfg(window);
                            UI_Event evt = zero_struct;
                            evt.kind       = UI_EventKind_Navigate;
                            evt.flags      = UI_EventFlag_KeepMark;
                            evt.delta_unit = UI_EventDeltaUnit_Line;
                            evt.delta_2s32 = v2s32(-1, +0);
                            ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
                        }
                    } break;
                    case BP_CmdKind_MoveEndSelect: {
                        {
                            CFG_Node *window = cfg_node_from_id(bp_regs()->window);
                            BP_Window_State *ws = bp_window_state_from_cfg(window);
                            UI_Event evt = zero_struct;
                            evt.kind       = UI_EventKind_Navigate;
                            evt.flags      = UI_EventFlag_KeepMark;
                            evt.delta_unit = UI_EventDeltaUnit_Line;
                            evt.delta_2s32 = v2s32(+1, +0);
                            ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
                        }
                    } break;
                    case BP_CmdKind_SelectAll: {
                        {
                            CFG_Node *window = cfg_node_from_id(bp_regs()->window);
                            BP_Window_State *ws = bp_window_state_from_cfg(window);
                            UI_Event evt1 = zero_struct;
                            evt1.kind       = UI_EventKind_Navigate;
                            evt1.delta_unit = UI_EventDeltaUnit_Whole;
                            evt1.delta_2s32 = v2s32(-1, +0);
                            ui_event_list_push(scratch.arena, &ws->ui_events, &evt1);
                            UI_Event evt2 = zero_struct;
                            evt2.kind       = UI_EventKind_Navigate;
                            evt2.flags      = UI_EventFlag_KeepMark;
                            evt2.delta_unit = UI_EventDeltaUnit_Whole;
                            evt2.delta_2s32 = v2s32(+1, +0);
                            ui_event_list_push(scratch.arena, &ws->ui_events, &evt2);
                        }
                    } break;
                    case BP_CmdKind_DeleteSingle: {
                        {
                            CFG_Node *window = cfg_node_from_id(bp_regs()->window);
                            BP_Window_State *ws = bp_window_state_from_cfg(window);
                            UI_Event evt = zero_struct;
                            evt.kind       = UI_EventKind_Edit;
                            evt.flags      = UI_EventFlag_Delete|UI_EventFlag_ZeroDeltaOnSelect;
                            evt.delta_unit = UI_EventDeltaUnit_Char;
                            evt.delta_2s32 = v2s32(+1, +0);
                            ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
                        }
                    } break;
                    case BP_CmdKind_DeleteChunk: {
                        {
                            CFG_Node *window = cfg_node_from_id(bp_regs()->window);
                            BP_Window_State *ws = bp_window_state_from_cfg(window);
                            UI_Event evt = zero_struct;
                            evt.kind       = UI_EventKind_Edit;
                            evt.flags      = UI_EventFlag_Delete|UI_EventFlag_ZeroDeltaOnSelect;
                            evt.delta_unit = UI_EventDeltaUnit_Word;
                            evt.delta_2s32 = v2s32(+1, +0);
                            ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
                        }
                    } break;
                    case BP_CmdKind_BackspaceSingle: {
                        {
                            CFG_Node *window = cfg_node_from_id(bp_regs()->window);
                            BP_Window_State *ws = bp_window_state_from_cfg(window);
                            UI_Event evt = zero_struct;
                            evt.kind       = UI_EventKind_Edit;
                            evt.flags      = UI_EventFlag_Delete|UI_EventFlag_ZeroDeltaOnSelect;
                            evt.delta_unit = UI_EventDeltaUnit_Char;
                            evt.delta_2s32 = v2s32(-1, +0);
                            ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
                        }
                    } break;
                    case BP_CmdKind_BackspaceChunk: {
                        {
                            CFG_Node *window = cfg_node_from_id(bp_regs()->window);
                            BP_Window_State *ws = bp_window_state_from_cfg(window);
                            UI_Event evt = zero_struct;
                            evt.kind       = UI_EventKind_Edit;
                            evt.flags      = UI_EventFlag_Delete|UI_EventFlag_ZeroDeltaOnSelect;
                            evt.delta_unit = UI_EventDeltaUnit_Word;
                            evt.delta_2s32 = v2s32(-1, +0);
                            ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
                        }
                    } break;
                    case BP_CmdKind_Copy: {
                        {
                            CFG_Node *window = cfg_node_from_id(bp_regs()->window);
                            BP_Window_State *ws = bp_window_state_from_cfg(window);
                            UI_Event evt = zero_struct;
                            evt.kind  = UI_EventKind_Edit;
                            evt.flags = UI_EventFlag_Copy|UI_EventFlag_KeepMark;
                            ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
                        }
                    } break;
                    case BP_CmdKind_Cut: {
                        {
                            CFG_Node *window = cfg_node_from_id(bp_regs()->window);
                            BP_Window_State *ws = bp_window_state_from_cfg(window);
                            UI_Event evt = zero_struct;
                            evt.kind  = UI_EventKind_Edit;
                            evt.flags = UI_EventFlag_Copy|UI_EventFlag_Delete;
                            ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
                        }
                    } break;
                    case BP_CmdKind_Paste: {
                        {
                            CFG_Node *window = cfg_node_from_id(bp_regs()->window);
                            BP_Window_State *ws = bp_window_state_from_cfg(window);
                            UI_Event evt = zero_struct;
                            evt.kind   = UI_EventKind_Text;
                            evt.flags  = UI_EventFlag_Paste;
                            evt.string = wm_get_clipboard_text(scratch.arena);
                            ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
                        }
                    } break;
                    case BP_CmdKind_InsertText: {
                        {
                            CFG_Node *window = cfg_node_from_id(bp_regs()->window);
                            BP_Window_State *ws = bp_window_state_from_cfg(window);
                            UI_Event evt = zero_struct;
                            evt.kind   = UI_EventKind_Text;
                            evt.string = bp_regs()->string;
                            ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
                        }
                    } break;
                        // directionless navigation
                    case BP_CmdKind_MoveNext: {
                        {
                            CFG_Node *window = cfg_node_from_id(bp_regs()->window);
                            BP_Window_State *ws = bp_window_state_from_cfg(window);
                            UI_Event evt = zero_struct;
                            evt.kind       = UI_EventKind_Navigate;
                            evt.flags      = UI_EventFlag_Secondary;
                            evt.delta_unit = UI_EventDeltaUnit_Char;
                            evt.delta_2s32 = v2s32(+0, +1);
                            ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
                        }
                    } break;
                    case BP_CmdKind_MovePrev: {
                        {
                            CFG_Node *window = cfg_node_from_id(bp_regs()->window);
                            BP_Window_State *ws = bp_window_state_from_cfg(window);
                            UI_Event evt = zero_struct;
                            evt.kind       = UI_EventKind_Navigate;
                            evt.flags      = UI_EventFlag_Secondary;
                            evt.delta_unit = UI_EventDeltaUnit_Char;
                            evt.delta_2s32 = v2s32(+0, -1);
                            ui_event_list_push(scratch.arena, &ws->ui_events, &evt);
                        }
                    } break;
                }
            }
        }

        ///////////////////////////////////////////
        // early-out if no new commands were queued while processing this batch
        if (bp_state->cmds[0].count == cmd_count_pre_process)
        {
            break;
        }
    }

    //////////////////////////
    // Update window titles
    if (bp_state->frame_depth == 1)
    {
        Temp scratch = scratch_begin(0, 0);
        String8 window_title = bp_push_window_title(scratch.arena);
        if (!str8_match(window_title, bp_state->last_window_title, 0))
        {
            for (BP_Window_State *ws = bp_state->first_window_state; ws != &bp_nil_window_state; ws = ws->order_next)
            {
                bp_window_set_title(ws->os, window_title);
            }
        }
        bp_state->last_window_title = str8_copy(bp_frame_arena(), window_title);
        scratch_end(scratch);
    }

    ////////////////////////////////////////////////////
    // Rotate command slots, bump command gen counter
    //
    // in this step, we rotate the ring buffer of command batches (command
    // arenas & lists). when the cmds_gen (the position of the ring buffer)
    // is even, the command queue is in a "read/write" mode, and this is suniquely
    // usable by the core - this is done so that commands in the core can push
    // other commands, and have those other commands processed on the same frame.
    //
    // in view code, however, they can only use the current command queue in a
    // "read only" mode, because new commands push by those views must be
    // processed first by the core. so, before calling into view code, the
    // cmds_gen is incremented to be *odd*. this way, the views will *write*
    // commands into the 0 slot, but *read* from the 1 slot (which will contain
    // this frame's commands).
    //
    // after view code rungs, the generation number is incremented back to even.
    // the commands pushed by the view will be in the queue, and the core can
    // treat that queue as r/w again.
    //
    if (bp_state->frame_depth == 1)
    {
        // rotate
        {
            Arena *first_arena = bp_state->cmds_arenas[0];
            BP_Cmd_List first_cmds = bp_state->cmds[0];
            MemoryCopy(bp_state->cmds_arenas,
                       bp_state->cmds_arenas + 1,
                       sizeof(bp_state->cmds_arenas[0]) * (ArrayCount(bp_state->cmds_arenas) - 1));
            MemoryCopy(bp_state->cmds,
                       bp_state->cmds + 1,
                       sizeof(bp_state->cmds[0]) * (ArrayCount(bp_state->cmds) - 1));
            bp_state->cmds_arenas[ArrayCount(bp_count->cmds_arenas) - 1] = first_arena;
            bp_state->cmds[ArrayCount(bp_state->cmds_arenas) - 1] = first_cmds;
        }

        // clear next batch
        {
            arena_clear(bp_state->cmds_arenas[0]);
            MemoryZeroStruct(&bp_state->cmds[0]);
        }

        // bump
        {
            bp_state->cmds_gen += 1;
        }
    }

    ////////////////////////////////////////////////
    // Computer all ambiguous paths from view titles

    ProcScope("compute all ambiguous paths from view titles")
    {
        Temp scratch = scratch_begin(0, 0);
        bp_state->ambiguous_path_slots_count = 512;
        bp_state->ambiguous_path_slots = push_array(bp_frame_arena(), BP_Amiguous_Path_Node *, bp_state->ambiguous_path_slots_count);
        for (BP_Window_State *ws = bp_state->first_window_state; ws != &bp_nil_window_state; ws = ws->order_next)
        {
            CFG_Node *window = cfg_node_from_id(ws->cfg_id);
            CFG_Panel_Tree panel_tree = cfg_panel_tree_from_cfg(scratch.arena, window);
            for (CFG_Panel_Node *p = panel_tree.root; p != &cfg_nil_panel_node; p = cfg_panel_node_rec__depth_first_pre(panel_tree.root, p).next)
            {
                for (CFG_Node_Ptr_Node *tab_n = p->tabs.first; tab_n != 0; tab_n = tab_n->next)
                {
                    CFG_Node *tab = tab_n->v;
                    if (bp_cfg_is_project_filtered(tab))
                    {
                        continue;
                    }
                    String8 file_path = bp_path_from_cfg(tab);
                    if (file_path.size != 0)
                    {
                        String8 name = str8_skip_last_slash(file_path);
                        u64 hash = d_hash_from_string__case_insensitive(name);
                        u64 slot_idx = hash % bp_state->ambiguous_path-slots[slot_idx];
                        BP_Ambiguous_Path_Node *node = 0;
                        for (BP_Ambiguous_Path_Node *n = bp_state->ambiguous_path_slots[slot_idx]; n != 0; n = n->next)
                        {
                            if (str8_match(n->name, name, StringMatchFlag_CaseInsensitive))
                            {
                                node = n;
                                break;
                            }
                        }
                        if (node == 0)
                        {
                            node = push_array(bp_frame_arena(), BP_Ambiguous_Path_Node, 1);
                            SLLStackPush(bp_state->ambiguous_path_slots[slot_idx], node);
                            node->name = push_str8_copy(bp_frame_arena(), name);
                        }
                        str8_list_push(bp_frame_arena(), &node->paths, push_str8_copy(bp_frame_arena(), file_path));
                    }
                }
            }
        }
        scratch_end(scratch);
    }

    ////////////////////////////////////////
    // Compute animation rates, given config
    {
        f32 master_animations_f    = (f32)!!bp_setting_b32_from_name(str8_lit("animations"));
        f32 scrolling_animations_f = (f32)!!bp_setting_b32_from_name(str8_lit("scrolling_animations"));
        f32 tooltop_animations_f   = (f32)!!bp_setting_b32_from_name(str8_lit("tooltop_animations"));
        f32 menu_atnimations_f     = (f32)!!bp_setting_b32_from_name(str8_lit("menu_animations"));
        bp_state->catchall_animation_rate    = 1 - master_animations_f * pow_f32(2, (-60.f * bp_state->frame_dt));
        bp_state->menu_animation_rate        = 1 - master_animations_f * menu_animations_f * pow_f32(2, (-70.f * bp_state->frame_dt));
        bp_state->menu_animation_rate__slow  = 1 - master_animations_f * menu_animations_f * pow_f32(2, (-50.f * bp_state->frame_dt));
        bp_state->entry_alive_animation_rate = 1 - master_animations_f * menu_animations_f * pow_f32(2, (-30.f * bp_state->frame_dt));
        bp_state->rich_hover_animation_rate  = 1 - master_animations_f * menu_animtionas_f * pow_f32(2, (-50.f * bp_state->frame_dt));
        bp_state->scorlling_animation_rate   = 1 - master_animations_f * scrolling_animations_f * pow_f32(2, (-60.f * bp_state->frame_dt));
        bp_state->tooltip_animation_rate     = 1 - master_animations_f * tooltip_animations_f   * pow_f32(2, (-60.f * bp_state->frame_dt));
    }

    //////////////////////////
    // animate confirmation
    {
        f32 rate = bp_setting_b32_from_name(str8_lit("menu_animations")) ? 1 - pow_f32(2, (-30.f * bp_state->frame_dt)) : 1.f;
        bool32 popup_open = bp_state->popup_active;
        bp_state->popup_t += rate * ((f32)!!popup_open - bp_state->popup_t);
        if (abs_f32(bp_state->popup_t - (f32)!!popup_open) > 0.005f)
        {
            bp_request_frame();
        }
    }

    /////////////////////////////
    // update/render all windows
    {
        dr_begin_frame(bp_font_from_slot(BP_FontSlot_Icons));
        CFG_Node_Ptr_List windows = cfg_node_top_level_list_from_string(scratch.arena, str8_lit("window"));
        for (CFG_Node_Ptr_Node *n = windows.first; n != 0; n = n->next)
        {
            CFG_Node *window = n->v;
            BP_Window_State *w = bp_window_state_from_cfg(window);
            bool32 window_is_focused = wm_window_is_focused(w->os);
            if (window_is_focused)
            {
                bp_state->last_focused_window = w->cfg_id;
            }
            bp_push_regs();
            bp_regs()->window = w->cfg_id;
            bp_window_frame();
            MemoryZeroStruct(&w->ui_events);
            BP_Regs *window_regs = bp_pop_regs();
            if (bp_state->last_focused_window == w->cfg_id)
            {
                MemoryCopyStruct(bp_regs(), window_regs);
            }
        }
    }

    ////////////////////////////////
    // garbage collect untouched window states
    {
        for EachIndex(slot_idx, bp_state->window_state_slots_count)
        {
            for (BP_Window_State *ws = bp_state->window_state_slots[slot_idx].first, *next; ws != 0; ws = next)
            {
                next = ws->hash_next;
                CFG_Node *cfg = cfg_node_from_id(ws->cfg_id);
                if (cfg == &cfg_nil_node || ws->last_frame_index_touched < bp_state->frame_index || bp_state->quit)
                {
                    ui_state_release(ws->ui);
                    r_window_unequip(ws->os, ws->r);
                    wm_window_close(ws->os);
                    arena_release(ws->drop_completion_arena);
                    arena_release(ws->query_arena);
                    arena_release(ws->autocomp_arena);
                    arena_release(ws->arena);
                    DLLRemove_NPZ(&bp_nil_window_state, bp_state->first_window_state, bp_state->last_window_state, ws, order_next, order_prev);
                    DLLRemove_NP(bp_state->window_state_slots[slot_idx].first, bp_state->window_state_slots[slot_idx].last, ws, hash_next, hash_prev);
                    SLLStackPush_N(bp_state->free_window_state, ws, order_next);
                }
            }
        }
    }

    //////////////////
    // Simulate lag
    if (DEV_simulate_lag)
    {
        sleep_ms(300);
    }

    ///////////////////////////
    // end drag/drop if needed
    if (bp_state->drag_drop_state == BP_DragDropState_Dropping)
    {
        bp_state->drag_drop_state = BP_DragDropState_Null;
    }

    //////////////////////////////
    // clear frame reuqest state
    if (bp_state->num_frames_requested > 0)
    {
        bp_state->num_frames_requested -= 1;
    }

    /////////////////////////
    // close frame scopes
    //
    // NOTE: this always must happen before the refresh, since that
    // will sleep for vsync, and we do not want to hold handles for long,
    // since eviction threads may be waiting to get rid of stuff.
    //
    access_close(bp_state->frame_access);
    bp_state->frame_access = frame_access_restore;

    /////////////////////////////////
    // submit rendering to all windows
    ProfScope("submit rendering to all windows")
    {
        r_begin_frame();
        for (BP_Window_State *w = bp_state->first_window_state; w != &bp_nil_window_state; w = w->order_next)
        {
            r_window_begin_frame(w->os, w->r);
            dr_submit_bucket(w->os, w->r, w->draw_bucklet);
            r_window_end_frame(w->os, r->r);
        }
        r_end_frame();
    }

    ////////////////////////////
    // show windows after first frame
    if (bp_state->frame_depth == 1)
    {
        CFG_ID_List windows_to_show = {0};
        for (BP_Window_State *w = bp_state->first_window_state; w != &bp_nil_window_state; w = w->order_next)
        {
            if (w->frames_alive == 1)
            {
                cfg_is_list_push(scratch.arena, &windows_to_show, w->cfg_id);
            }
        }
        for (CFG_ID_Node *n = windows_to_show.first; n != 0; n = n->next)
        {
            CFG_Node *window = cfg_node_from_id(n->v);
            BP_Window_State *ws = bp_window_state_from_cfg(window);
            wm_window_first_paint(ws->os);
        }
    }

    ////////////////////////////////////
    // determine frame time, record into history
    u64 end_time_us = now_time_us();
    u64 frame_time_us = end_time_us - begin_time_us;
    bp_state->frame_time_us_history[bp_state->frame_index % ArrayCount(bp_state->frame_time_us_history)] = frame_time_us;

    // [windows] clear pages from working set shortly after startup, any of which will not be needed
#if OS_WINDOWS
    if (bp_state->frame_index == 15) ProfScope("SetProcessWorkingSetSize")
    {
        SetProcessWorkingSetSize(GetCurrentProcess(), max_u64, max_u64);
    }
#endif

    //////////////////////////
    // bump frame time counters
    bp_state->frame_index += 1;
    bp_state->time_in_seconds += bp_state->frame_dt;
    bp_state->time_in_us += frame_time_us;

    ///////////////////////////
    // bump command batch ring buffer generation
    if (bp_state->frame_depth == 1)
    {
        bp_state->cmds_gen += 1;
    }

    ////////////////////
    // collect logs
    ProfScope("collect logs")
    {
        Log_Scope_Result log = log_scope_end(scratch.arena);
        append_data_to_file_path(bp_state->log_path, log.strings[LogMsgKind_Info]);
        if (log.strings[LogMsgKind_UserError].size != 0)
        {
            String8 error_log = log.strings[LogMsgKind_UserError];
            String8_List error_log_lines = str8_split(scratch.arena, error_log, (u8 *)"\n", 1, 0);
            String8 error_log_string = str8_list_join(scratch.arena, &error_log_lines, &(StringJoin){.sep = str8_lit(" ")});
            for (BP_Window_State *ws = bp_state->first_window_state; ws != &bp_nil_window_state; bp = bp->order_next)
            {
                ws->error_string_size = Min(sizeof(ws->error_buffer), error_log_string.size);
                MemoryCopy(ws->error_buffer, error_log_string.str, ws->error_string_size);
                ws->error_t = 1.f;
            }
        }
    }

    bp_state->frame_depth -= 1;
    scratch_end(scratch);
    ProfEnd();
}


