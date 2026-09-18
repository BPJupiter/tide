// Copyright (c) Frances Telfar
// Licensed under the PolyForm Noncommercial License (https://polyformproject.org/licenses/noncommercial/1.0.0)

/////////////////////
// Generated Code

#include "generated/tide.meta.c"

/////////////////////////////
// Registers Type Functions


internal void ti_regs_copy_contents(Arena *arena, TI_Regs *dst, TI_Regs *src)
{
    MemoryCopyStruct(dst, src);
    dst->cfg_list    = cfg_id_list_copy(arena, &src->cfg_list);
    dst->file_path   = push_str8_copy(arena, src->file_path);
    dst->string      = push_str8_copy(arena, src->string);
    dst->cmd_name    = push_str8_copy(arena, src->cmd_name);
    if(dst->cfg_list.count == 0 && dst->cfg != 0)
    {
        cfg_id_list_push(arena, &dst->cfg_list, dst->cfg);
    }
    /*
    dst->lines       = d_line_list_copy(arena, &src->lines);
    dst->expr        = push_str8_copy(arena, src->expr);
    */
}

internal TI_Regs *
ti_regs_copy(Arena *arena, TI_Regs *src)
{
    TI_Regs *dst = push_array(arena, TI_Regs, 1);
    ti_regs_copy_contents(arena, dst, src);
    return dst;
}

//////////////////////
// Commands Type Functions

internal void ti_cmd_list_push_new(Arena *arena, TI_Cmd_List *cmds, String8 name, TI_Regs *regs)
{
    TI_Cmd_Node *n = push_array(arena, TI_Cmd_Node, 1);
    n->cmd.name = push_str8_copy(arena, name);
    n->cmd.regs = ti_regs_copy(arena, regs);
    DLLPushBack(cmds->first, cmds->last, n);
    cmds->count += 1;
}

//////////////////////
// Config Functions

internal bool32 ti_cfg_is_project_filtered(CFG_Node *cfg)
{
    CFG_Node *project = cfg_node_child_from_string(cfg, str8_lit("project"));
    bool32 result = (project != &cfg_nil_node &&
                     project->first->string.size != 0 &&
                     !path_match_normalised(ti_state->project_path, project->first->string));
    return result;
}

internal Vec4f32 ti_hsva_from_cfg(CFG_Node *cfg)
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

internal Vec4f32
ti_color_from_cfg(CFG_Node *cfg)
{
    Vec4f32 hsva = ti_hsva_from_cfg(cfg);
    Vec4f32 rgba = linear_from_srgba(rgba_from_hsva(hsva));
    return rgba;
}

internal bool32 ti_disabled_from_cfg(CFG_Node *cfg)
{
    Temp scratch = scratch_begin(0, 0);
    MD_Node *child_schema = &md_nil_node;
    MD_Node_Ptr_List schemas = cfg_schemas_from_name(scratch.arena, ti_state->cfg_schema_table, cfg->string);
    for(MD_Node_Ptr_Node *n = schemas.first; n != 0 && child_schema == &md_nil_node; n = n->next)
    {
        child_schema = md_child_from_string(n->v, str8_lit("enabled"), 0);
    }
    MD_Node *default_tag = md_tag_from_string(child_schema, str8_lit("default"), 0);
    String8 value_string = cfg_node_child_from_string(cfg, str8_lit("enabled"))->first->string;
    if(value_string.size == 0)
    {
        value_string = default_tag->first->string;
    }
    u64 value_u64 = 0;
    try_u64_from_str8_c_rules(value_string, &value_u64);
    bool32 is_enabled = !!value_u64;
    bool32 is_disabled = !is_enabled;
    if(value_string.size == 0)
    {
        is_disabled = 0;
    }
    scratch_end(scratch);
    return is_disabled;
}

internal String8 ti_name_from_cfg(CFG_Node *cfg)
{
    CFG_Node *name_root = cfg_node_child_from_string(cfg, str8_lit("name"));
    String8 result = name_root->first->string;
    return result;
}

internal String8 ti_label_from_cfg(CFG_Node *cfg)
{
    CFG_Node *label_root = cfg_node_child_from_string(cfg, str8_lit("label"));
    String8 result = label_root->first->string;
    return result;
}

internal String8 ti_expr_from_cfg(CFG_Node *cfg)
{
    CFG_Node *expr_root = cfg_node_child_from_string(cfg, str8_lit("expression"));
    String8 result = expr_root->first->string;
    return result;
}

internal String8 ti_path_from_cfg(CFG_Node *cfg)
{
    CFG_Node *root = cfg_node_child_from_string(cfg, str8_lit("path"));
    String8 result = root->first->string;
    return result;
}

internal String8 ti_default_setting_from_names(String8 schema_name, String8 setting_name)
{
    String8 result = {0};
    {
        Temp scratch = scratch_begin(0, 0);
        MD_Node *setting_schema = &md_nil_node;
        MD_Node_Ptr_List schemas = cfg_schemas_from_name(scratch.arena, ti_state->cfg_schema_table, schema_name);
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

internal String8
ti_setting_from_name(String8 name)
{
    String8 result = {0};
    if(name.size != 0)
    {
        Temp scratch = scratch_begin(0, 0);
        
        // rjf: find most-granular config scopes to begin looking for the setting
        typedef struct Cfg_Seed_Task Cfg_Seed_Task;
        struct Cfg_Seed_Task
        {
            Cfg_Seed_Task *next;
            CFG_Node *cfg;
            bool32 allow_bucket_chains;
        };
        CFG_Node *view_cfg = cfg_node_from_id(ti_regs()->view);
        if(view_cfg == &cfg_nil_node)
        {
            view_cfg = cfg_node_from_id(ti_regs()->tab);
        }
        Cfg_Seed_Task panel_task = {0, &cfg_nil_node, 1};
        if(panel_task.cfg == &cfg_nil_node) { panel_task.cfg = cfg_node_from_id(ti_regs()->panel); }
        if(panel_task.cfg == &cfg_nil_node) { panel_task.cfg = cfg_node_from_id(ti_regs()->window); }
        if(panel_task.cfg == &cfg_nil_node) { panel_task.cfg = cfg_node_child_from_string(cfg_node_root(), s("user")); }
        Cfg_Seed_Task view_task = {&panel_task, view_cfg, 1};
        Cfg_Seed_Task *first_task = &view_task;
        Cfg_Seed_Task *last_task = &panel_task;
        
        // rjf: for each task, look for the setting, follow parent chain upwatis
        CFG_Node *setting = &cfg_nil_node;
        for(Cfg_Seed_Task *t = first_task; t != 0; t = t->next)
        {
            for(CFG_Node *cfg = t->cfg; cfg != &cfg_nil_node; cfg = cfg->parent)
            {
                setting = cfg_node_child_from_string(cfg, name);
                if(setting != &cfg_nil_node)
                {
                    goto break_all;
                }
                if(cfg->parent == cfg_node_root() && t->allow_bucket_chains)
                {
                    String8 next_bucket = {0};
                    bool32 allow_bucket_chains = 0;
                    if(str8_match(cfg->string, str8_lit("user"), 0))
                    {
                        next_bucket = str8_lit("project");
                    }
                    else if(str8_match(cfg->string, str8_lit("project"), 0))
                    {
                        next_bucket = str8_lit("user");
                    }
                    else
                    {
                        allow_bucket_chains = 1;
                        next_bucket = str8_lit("user");
                    }
                    if(next_bucket.size != 0)
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
        
        // rjf: return resultant child string stored under this key
        result = setting->first->string;
        
        // rjf: no result -> look for default in schemas
        if(result.size == 0)
        {
            for(Cfg_Seed_Task *t = first_task; t != 0; t = t->next)
            {
                for(CFG_Node *cfg = t->cfg; cfg != &cfg_nil_node; cfg = cfg->parent)
                {
                    result = ti_default_setting_from_names(cfg->string, name);
                    if(result.size != 0)
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


internal bool32 ti_setting_bool32_from_name(String8 name)
{
    bool32 result = false;
    String8 value = ti_setting_from_name(name);
    if (value.size != 0)
    {
        u64 value_u64 = 0;
        try_u64_from_str8_c_rules(value, &value_u64);
        result = !!value_u64;
    }
    return result;
}

internal u64 ti_setting_u64_from_name(String8 name)
{
    u64 result = 0;
    String8 value = ti_setting_from_name(name);
    if (value.size != 0)
    {
        try_u64_from_str8_c_rules(value, &result);
    }
    return result;
}

internal f32 ti_setting_f32_from_name(String8 name)
{
    f32 result = 0.f;
    String8 value = ti_setting_from_name(name);
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

internal CFG_Node *ti_immediate_cfg_from_key(String8 string)
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
        immediate = cfg_node_new(ti_state->cfg, transient, str8_lit("immediate"));
        cfg = cfg_node_new(ti_state->cfg, immediate, string);
    }
    cfg_node_child_from_string_or_alloc(ti_state->cfg, immediate, str8_lit("hot"));
    return cfg;
}

internal CFG_Node *ti_immediate_cfg_from_keyf(char *fmt, ...)
{
    Temp scratch = scratch_begin(0, 0);
    va_list args;
    va_start(args, fmt);
    String8 key = push_str8fv(scratch.arena, fmt, args);
    CFG_Node *result = ti_immediate_cfg_from_key(key);
    va_end(args);
    scratch_end(scratch);
    return result;
}

////////////////////////
// Evaluation Visualization
//

// eval <-> file path

internal String8 ti_file_path_from_eval_string(Arena *arena, String8 string)
{
    String8 result = {0};
    {
        String8 s = str8_skip_chop_whitespace(string);
        u64 colon = str8_find_needle(s, 0, str8_lit(":"), 0);
        if (colon < s.size)
        {
            String8 q = str8_prefix(s, colon);
            if (str8_match(q, str8_lit("file"), 0) || str8_match(q, str8_lit("folder"), 0))
            {
                String8 rest = str8_skip(s, colon+1);
                if (str8_match(str8_postfix(rest, 5), str8_lit(".data"), 0))
                {
                    rest = str8_chop(rest, 5);
                }
                if (rest.size >= 2 && rest.str[0] == '"')
                {
                    rest = str8_substr(rest, r1u64(1, rest.size-1));
                }
                result = raw_from_escaped_str8(arena, rest);
            }
        }
    }
    return result;
}

///////////////////
// View Functions

/*
internal CFG_Node *ti_view_from_string(CFG_Node *parent, String8 string)
{
}
*/

internal TI_View_State *ti_view_state_from_cfg(CFG_Node *cfg)
{
    TI_View_State *view_state = &ti_nil_view_state;
    CFG_ID id = cfg->id;
    if (id != 0 &&
        id == ti_state->view_state_last_accessed_id &&
        id == ti_state->view_state_last_accessed->cfg_id)
    {
        view_state = ti_state->view_state_last_accessed;
    }
    else
    {
        u64 hash = u64_hash_from_str8(str8_struct(&id));
        u64 slot_idx = hash%ti_state->view_state_slots_count ;
        TI_View_State_Slot *slot = &ti_state->view_state_slots[slot_idx];
        for (TI_View_State *v = slot->first; v != 0; v = v->hash_next)
        {
            if (v->cfg_id == id)
            {
                view_state = v;
                break;
            }
        }
    }
    if (view_state == &ti_nil_view_state)
    {
        view_state = ti_state->free_view_state;
        if (view_state)
        {
            SLLStackPop_N(ti_state->free_view_state, hash_next);
        }
        else
        {
            view_state = push_array(ti_state->arena, TI_View_State, 1);
        }
        MemoryCopyStruct(view_state, &ti_nil_view_state);
        u64 hash = u64_hash_from_str8(str8_struct(&id));
        u64 slot_idx = hash%ti_state->view_state_slots_count;
        TI_View_State_Slot *slot = &ti_state->view_state_slots[slot_idx];
        DLLPushBack_NP(slot->first, slot->last, view_state, hash_next, hash_prev);
        view_state->cfg_id = id;
        view_state->arena = arena_alloc();
        view_state->arena_reset_pos = arena_pos(view_state->arena);
    }
    if (view_state != &ti_nil_view_state)
    {
        view_state->last_frame_index_touched = ti_state->frame_index;
    }
    ti_state->view_state_last_accessed = view_state;
    ti_state->view_state_last_accessed_id = id;
    return view_state;
}

internal void ti_view_ui(Rng2f32 rect)
{
    // TODO
}

///////////////////////
// View Building API

// view info extraction

internal Arena *ti_view_arena(void)
{
    CFG_Node *view = cfg_node_from_id(ti_regs()->view);
    TI_View_State *view_state = ti_view_state_from_cfg(view);
    return view_state->arena;
}

internal UI_Scroll_Pt2 ti_view_scroll_pos(void)
{
    CFG_Node *view = cfg_node_from_id(ti_regs()->view);
    TI_View_State *view_state = ti_view_state_from_cfg(view);
    return view_state->scroll_pos;
}

internal String8 ti_view_query_cmd(void)
{
    CFG_Node *view = cfg_node_from_id(ti_regs()->view);
    CFG_Node *query = cfg_node_child_from_string(view, str8_lit("query"));
    CFG_Node *cmd = cfg_node_child_from_string(query, str8_lit("cmd"));
    String8 string = cmd->first->string;
    return string;
}

internal String8 ti_view_query_input(void)
{
    CFG_Node *view = cfg_node_from_id(ti_regs()->view);
    CFG_Node *query = cfg_node_child_from_string(view, str8_lit("query"));
    CFG_Node *input = cfg_node_child_from_string(query, str8_lit("input"));
    String8 string = input->first->string;
    return string;
}

internal String8 ti_view_setting_from_name(String8 name)
{
    CFG_Node *view = cfg_node_from_id(ti_regs()->view);
    String8 result = cfg_node_child_from_string(view, name)->first->string;
    if(result.size == 0)
    {
        result = ti_default_setting_from_names(view->string, name);
    }
    return result;
}

/*
internal bool32 ti_view_setting_bool32_from_name(String8 string)
{
}

internal u64 ti_view_setting_u64_from_name(String8 string)
{
}

internal f32 ti_view_setting_f32_from_name(String8 string)
{
}

internal u64 ti_view_setting_addr_from_name(String8 string)
{
}

internal String8 ti_view_setting_from_name(String8 name)
{
}
*/

// pushing/attaching view resources

// storing view-attached state

///////////////////////
// Window Functions

internal String8 ti_push_window_title(Arena *arena)
{
    CFG_Node *root = cfg_node_root();
    CFG_Node *project = cfg_node_child_from_string(root, str8_lit("project"));
    CFG_Node *name = cfg_node_child_from_string(project, str8_lit("name"));
    String8 project_name = name->first->string;
    if (project_name.size == 0)
    {
        String8 prof_path = ti_state->project_path;
        prof_path = str8_chop_last_dot(prof_path);
        project_name = str8_skip_last_slash(prof_path);
    }
    String8 result = push_str8f(arena,
                                "%S%s%s",
                                project_name,
                                project_name.size != 0 ? " - " : "",
                                BUILD_TITLE " (" BUILD_VERSION_STRING_LITERAL " " BUILD_RELEASE_PHASE_STRING_LITERAL ")");
    return result;
}

internal CFG_Node *ti_window_from_cfg(CFG_Node *cfg)
{
    CFG_Node *result = &cfg_nil_node;
    for (CFG_Node *c = cfg; c != &cfg_nil_node; c = c->parent)
    {
        if (c->parent->parent == cfg_node_root() && str8_match(c->string, str8_lit("window"), 0))
        {
            result = c;
            break;
        }
    }
    return result;
}

internal TI_Window_State *ti_window_state_from_cfg(CFG_Node *cfg)
{
    // unpack
    CFG_Node *window_cfg = ti_window_from_cfg(cfg);
    CFG_ID id = window_cfg->id;

    // scan for existing window
    TI_Window_State *ws = &ti_nil_window_state;
    if (id != 0 &&
        id == ti_state->window_state_last_accessed_id &&
        id == ti_state->window_state_last_accessed->cfg_id)
    {
        ws = ti_state->window_state_last_accessed;
    }
    else
    {
        u64 hash = u64_hash_from_str8(str8_struct(&id));
        u64 slot_idx = hash % ti_state->window_state_slots_count;
        TI_Window_State_Slot *slot = &ti_state->window_state_slots[slot_idx];
        for (TI_Window_State *w = slot->first; w != 0; w = w->hash_next)
        {
            if (w->cfg_id == id)
            {
                ws = w;
                break;
            }
        }
    }

    // allocate/open new window if one was not found
    if (window_cfg != &cfg_nil_node && ws == &ti_nil_window_state)
    {
        Temp scratch = scratch_begin(0, 0);

        // unpack configuration options
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
            WM_Monitor_Array monitors = wm_push_monitors_array(scratch.arena);
            for EachIndex(idx, monitors.count)
            {
                String8 monitor_name = wm_name_from_monitor(scratch.arena, monitors.v[idx]);
                if (str8_match(monitor_name, monitor_cfg->first->string, StringMatchFlag_CaseInsensitive))
                {
                    preferred_monitor = monitors.v[idx];
                    break;
                }
            }
        }

        // allocate window
        ws = ti_state->free_window_state;
        if (ws != 0)
        {
            SLLStackPop_N(ti_state->free_window_state, order_next);
        }
        else
        {
            ws = push_array_no_zero(ti_state->arena, TI_Window_State, 1);
        }
        MemoryZeroStruct(ws);

        // fill out window
        ws->cfg_id = id;
        ws->arena = arena_alloc();
        {
            String8 title = ti_push_window_title(scratch.arena);
            ws->os = wm_window_open(r2f32p(pos.x, pos.y, pos.x + size.x, pos.y + size.y),
                                    WM_WindowFlag_CustomBorder|
                                    (!has_pos * WM_WindowFlag_UseDefaultPosition), title);
        }
        ws->r = r_window_equip(ws->os);
        ws->ui = ui_state_alloc();
        ws->query_arena = arena_alloc();
        ws->last_dpi = wm_dpi_from_window(ws->os);
        WM_Monitor zero_monitor = {0};
        if (!wm_monitor_match(zero_monitor, preferred_monitor))
        {
            wm_window_set_monitor(ws->os, preferred_monitor);
        }
        if (cfg_node_child_from_string(window_cfg, str8_lit("fullscreen")) != &cfg_nil_node)
        {
            wm_window_set_fullscreen(ws->os, 1);
        }
        if (cfg_node_child_from_string(window_cfg, str8_lit("maximized")) != &cfg_nil_node)
        {
            wm_window_set_maximized(ws->os, 1);
        }

        // hook up window links
        u64 hash = u64_hash_from_str8(str8_struct(&id));
        u64 slot_idx = hash % ti_state->window_state_slots_count;
        TI_Window_State_Slot *slot = &ti_state->window_state_slots[slot_idx];
        DLLPushBack_NPZ(&ti_nil_window_state, ti_state->first_window_state, ti_state->last_window_state, ws, order_next, order_prev);
        DLLPushBack_NP(slot->first, slot->last, ws, hash_next, hash_prev);

        scratch_end(scratch);
    }

    // touch window for this frame
    if (ws != &ti_nil_window_state)
    {
        ws->last_frame_index_touched = ti_state->frame_index;
    }

    ti_state->window_state_last_accessed_id = ws->cfg_id;
    ti_state->window_state_last_accessed = ws;
    return ws;
}

internal TI_Window_State *ti_window_state_from_os_handle(WM_Window os)
{
    TI_Window_State *ws = &ti_nil_window_state;
    {
        for (TI_Window_State *w = ti_state->first_window_state;
             w != &ti_nil_window_state;
             w = w->order_next)
        {
            if (wm_window_match(w->os, os))
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

internal void ti_window_frame(void)
{
    Temp scratch = scratch_begin(0, 0);
    ProfBeginFunction();

    /////////////////////////////////////
    // @window_frame_part unpack context
    //
    CFG_Node *window          = cfg_node_from_id(ti_regs()->window);
    TI_Window_State *ws       = ti_window_state_from_cfg(cfg_node_from_id(ti_regs()->window));
    CFG_Panel_Tree panel_tree = cfg_panel_tree_from_cfg(scratch.arena, window);
    bool32 window_is_focused  = wm_window_is_focused(ws->os);
    //bool32 popup_is_open      = (ti_state->popup_active);
    bool32 query_is_open      = (ws->query_top != 0);

    ui_select_state(ws->ui);

    /////////////////////////////
    // @window_frame_part fill panel/view interaction registers
    //
    {
        ti_regs()->panel = panel_tree.focused->cfg->id;
        ti_regs()->tab   = panel_tree.focused->selected_tab->id;
        ti_regs()->view  = panel_tree.focused->selected_tab->id;
    }

    /////////////////////////////
    // @window_frame_part compute window's theme
    //
    {
        Access *access = access_open();

        // try to find theme settings from the projects, then the user.
        CFG_Node_Ptr_List colors_cfgs = {0};
        CFG_Node *theme_parents[] = {
            cfg_node_child_from_string(cfg_node_root(), str8_lit("project")),
            cfg_node_child_from_string(cfg_node_root(), str8_lit("user")),
        };
        CFG_Node *theme_cfgs[] = {
            &cfg_nil_node,
            &cfg_nil_node,
        };
        for EachIndex(idx, ArrayCount(theme_parents))
        {
            CFG_Node *parent_cfg = theme_parents[idx];
            if (theme_cfgs[idx] == &cfg_nil_node)
            {
                CFG_Node *possible_theme_cfg = cfg_node_child_from_string(parent_cfg, str8_lit("theme"));
                if (possible_theme_cfg != &cfg_nil_node)
                {
                    theme_cfgs[idx] = possible_theme_cfg;
                }
            }
            for (CFG_Node *child = parent_cfg->first; child != &cfg_nil_node; child = child->next)
            {
                if (str8_match(child->string, str8_lit("theme_color"), 0))
                {
                    cfg_node_ptr_list_push_front(scratch.arena, &colors_cfgs, child);
                }
            }
        }

        // chose which theme cfg to use
        CFG_Node *theme_cfg = theme_cfgs[1];

        // map the theme config to the associated tree (right now just a preset)
        MD_Node *theme_tree = ti_state->theme_preset_trees[TI_ThemePreset_DefaultDark];

        //build tasks for color applications - each task comprises of a metadesk
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
            for (CFG_Node_Ptr_Node *n = colors_cfgs.first; n != 0; n = n->next)
            {
                Theme_Task *t = push_array(scratch.arena, Theme_Task, 1);
                SLLQueuePushFront(first_task,  last_task, t);
                t->tree = md_tree_from_string(scratch.arena, cfg_string_from_tree(scratch.arena, ti_state->cfg_schema_table, str8_zero(), n->v));
            }
        }

        // apply theme tasks, build each color pattern for this window's
        // structured theme
        typedef struct Theme_Pattern_Node Theme_Pattern_Node;
        struct Theme_Pattern_Node
        {
            Theme_Pattern_Node *next;
            UI_Theme_Pattern pattern;
        };
        Theme_Pattern_Node *first_pattern = 0;
        Theme_Pattern_Node *last_pattern = 0;
        u64 pattern_count = 0;
        for (Theme_Task *t = first_task; t != 0; t = t->next)
        {
            MD_Node *tree_root = t->tree;
            for (MD_Node *n = tree_root; !md_node_is_nil(n); n = md_node_rec_depth_first_pre(n, tree_root).next)
            {
                if (str8_match(n->string, str8_lit("theme_color"), 0))
                {
                    MD_Node *tags_child = md_child_from_string(n, str8_lit("tags"), 0);
                    MD_Node *value_child = md_child_from_string(n, str8_lit("value"), 0);
                    u8 split_char = ' ';
                    String8_List tags = str8_split(scratch.arena, tags_child->first->string, &split_char, 1, 0);
                    u32 color_u32 = u32_from_str8(str8_skip(value_child->first->string, 2), 16);
                    Vec4f32 color_linear = linear_from_srgba(rgba_from_u32(color_u32));
                    Theme_Pattern_Node *node = push_array(scratch.arena, Theme_Pattern_Node, 1);
                    node->pattern.tags = str8_array_from_list(ti_frame_arena(), &tags);
                    node->pattern.linear = color_linear;
                    SLLQueuePush(first_pattern, last_pattern, node);
                    pattern_count += 1;
                }
            }
        }

        // convert to final pattern array
        ws->theme = push_array(ti_frame_arena(), UI_Theme, 1);
        ws->theme->patterns_count = pattern_count;
        ws->theme->patterns = push_array(ti_frame_arena(), UI_Theme_Pattern, ws->theme->patterns_count);
        {
            u64 idx = 0;
            for (Theme_Pattern_Node *n = first_pattern; n != 0; n = n->next, idx += 1)
            {
                ws->theme->patterns[idx] = n->pattern;
            }
        }

        access_close(access);
    }

    //////////////////////////
    // @window_frame_part pre-emptively rasterize common glyphs on the first frame
    //
    if (ti_state->first_window_state == ws && ti_state->last_window_state == ws && ws->frames_alive == 0)
    {
        f32 font_size = ti_font_size();
        TI_FontSlot english_font_slots[] = {TI_FontSlot_Main, TI_FontSlot_Code};
        TI_FontSlot icon_font_slot = TI_FontSlot_Icons;
        for (u64 idx = 0; idx < ArrayCount(english_font_slots); idx += 1)
        {
            Temp scratch = scratch_begin(0, 0);
            TI_FontSlot slot = english_font_slots[idx];
            String8 sample_text = str8_lit("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890~!@#$%^&*()-_+=[{]}\\|;:'\",<.>/?");
            fnt_run_from_string(ti_font_from_slot(slot),
                                font_size,
                                0, 0, 0,
                                sample_text);
            fnt_run_from_string(ti_font_from_slot(slot),
                                font_size,
                                0, 0, 0,
                                sample_text);
            scratch_end(scratch);
        }
        for (TI_IconKind icon_kind = TI_IconKind_Null; icon_kind < TI_IconKind_COUNT; icon_kind = (TI_IconKind)(icon_kind + 1))
        {
            Temp scratch = scratch_begin(0, 0);
            fnt_run_from_string(ti_font_from_slot(icon_font_slot),
                                font_size,
                                0, 0, FNT_RasterFlag_Smooth,
                                ti_icon_kind_text_table[icon_kind]);
            fnt_run_from_string(ti_font_from_slot(icon_font_slot),
                                font_size,
                                0, 0, FNT_RasterFlag_Smooth,
                                ti_icon_kind_text_table[icon_kind]);
            fnt_run_from_string(ti_font_from_slot(icon_font_slot),
                                font_size,
                                0, 0, FNT_RasterFlag_Smooth,
                                ti_icon_kind_text_table[icon_kind]);
            scratch_end(scratch);
        }
    }

    /////////////////////////
    // @window_frame_part commit window's position/status to underlying cfg tree
    //
    {
        Temp scratch = scratch_begin(0, 0);
        bool32 is_fullscreen = wm_window_is_fullscreen(ws->os);
        bool32 is_maximized = wm_window_is_maximized(ws->os);
        bool32 is_minimized = wm_window_is_minimized(ws->os);
        if (is_fullscreen)
        {
            cfg_node_child_from_string_or_alloc(ti_state->cfg, window, str8_lit("fullscreen"));
        }
        else
        {
            cfg_node_release(ti_state->cfg, cfg_node_child_from_string(window, str8_lit("fullscreen")));
        }
        if (is_maximized)
        {
            cfg_node_child_from_string_or_alloc(ti_state->cfg, window, str8_lit("maximized"));
        }
        else
        {
            cfg_node_release(ti_state->cfg, cfg_node_child_from_string(window, str8_lit("maximized")));
        }

        // DPI changes -> xform font size / window size
        f32 dpi = wm_dpi_from_window(ws->os);
        if (dpi != ws->last_dpi)
        {
            fnt_reset();
            f32 current_font_size = ti_font_size();
            f32 new_font_size = current_font_size * (dpi / ws->last_dpi);
            new_font_size = Clamp(6.f, new_font_size, 72.f);
            CFG_Node *font_size_cfg = cfg_node_child_from_string_or_alloc(ti_state->cfg, window, str8_lit("font_size"));
            cfg_node_new_replacef(ti_state->cfg, font_size_cfg, "%I64u", (u64)new_font_size);
            ws->last_dpi = dpi;
        }

        // commit position
        Rng2f32 window_rect = wm_rect_from_window(ws->os);
        if (!is_fullscreen && !is_maximized && !is_minimized)
        {
            Vec2f32 pos = window_rect.p0;
            CFG_Node *pos_root = cfg_node_child_from_string_or_alloc(ti_state->cfg, window, str8_lit("pos"));
            if ((s32)pos.x != (s32)f64_from_str8(pos_root->first->string) ||
                (s32)pos.y != (s32)f64_from_str8(pos_root->last->string))
            {
                CFG_Node *x = pos_root->first;
                if (x == &cfg_nil_node)
                {
                    x = cfg_node_alloc(ti_state->cfg);
                    cfg_node_insert_child(ti_state->cfg, pos_root, &cfg_nil_node, x);
                }
                CFG_Node *y = x->next;
                if (y == &cfg_nil_node)
                {
                    y = cfg_node_alloc(ti_state->cfg);
                    cfg_node_insert_child(ti_state->cfg, pos_root, x, y);
                }
                cfg_node_equip_stringf(ti_state->cfg, x, "%i", (s32)pos.x);
                cfg_node_equip_stringf(ti_state->cfg, y, "%i", (s32)pos.y);
            }
        }

        // commit size
        if (!is_fullscreen && !is_maximized && !is_minimized)
        {
            Vec2f32 size = dim_2f32(window_rect);
            CFG_Node *size_root = cfg_node_child_from_string_or_alloc(ti_state->cfg, window, str8_lit("size"));
            if((s32)size.x != (s32)f64_from_str8(size_root->first->string) ||
               (s32)size.y != (s32)f64_from_str8(size_root->last->string))
            {
                CFG_Node *width = size_root->first;
                if(width == &cfg_nil_node)
                {
                    width = cfg_node_alloc(ti_state->cfg);
                    cfg_node_insert_child(ti_state->cfg, size_root, &cfg_nil_node, width);
                }
                CFG_Node *height = width->next;
                if(height == &cfg_nil_node)
                {
                    height = cfg_node_alloc(ti_state->cfg);
                    cfg_node_insert_child(ti_state->cfg, size_root, width, height);
                }
                cfg_node_equip_stringf(ti_state->cfg, width, "%i", (s32)size.x);
                cfg_node_equip_stringf(ti_state->cfg, height, "%i", (s32)size.y);
            }
        }
        
        
        // commit monitor
        if (!is_minimized)
        {
            WM_Monitor monitor = wm_monitor_from_window(ws->os);
            String8 monitor_name = wm_name_from_monitor(scratch.arena, monitor);
            CFG_Node *monitor_root = cfg_node_child_from_string_or_alloc(ti_state->cfg, window, str8_lit("monitor"));
            if (!str8_match(monitor_root->first->string, monitor_name, 0))
            {
                cfg_node_new_replace(ti_state->cfg, monitor_root, monitor_name);
            }
        }
        scratch_end(scratch);
    }
    
    //////////////////////////////
    // @window_frame_part build UI
    ProfScope("build UI")
    {
        //////////////////////////
        // @window_ui_part set up
        {
            // get font size info
            f32 top_level_font_size = 0;
            TI_RegsScope(.view = 0, .tab = 0) top_level_font_size = ti_font_size();

            // build icon info
            UI_Icon_Info icon_info = {0};
            {
                icon_info.icon_font = ti_font_from_slot(TI_FontSlot_Icons);
                icon_info.icon_kind_text_map[UI_IconKind_RightArrow]     = ti_icon_kind_text_table[TI_IconKind_RightScroll];
                icon_info.icon_kind_text_map[UI_IconKind_DownArrow]      = ti_icon_kind_text_table[TI_IconKind_DownScroll];
                icon_info.icon_kind_text_map[UI_IconKind_LeftArrow]      = ti_icon_kind_text_table[TI_IconKind_LeftScroll];
                icon_info.icon_kind_text_map[UI_IconKind_UpArrow]        = ti_icon_kind_text_table[TI_IconKind_UpScroll];
                icon_info.icon_kind_text_map[UI_IconKind_RightCaret]     = ti_icon_kind_text_table[TI_IconKind_RightCaret];
                icon_info.icon_kind_text_map[UI_IconKind_DownCaret]      = ti_icon_kind_text_table[TI_IconKind_DownCaret];
                icon_info.icon_kind_text_map[UI_IconKind_LeftCaret]      = ti_icon_kind_text_table[TI_IconKind_LeftCaret];
                icon_info.icon_kind_text_map[UI_IconKind_UpCaret]        = ti_icon_kind_text_table[TI_IconKind_UpCaret];
                icon_info.icon_kind_text_map[UI_IconKind_CheckHollow]    = ti_icon_kind_text_table[TI_IconKind_CheckHollow];
                icon_info.icon_kind_text_map[UI_IconKind_CheckFilled]    = ti_icon_kind_text_table[TI_IconKind_CheckFilled];
            }

            // build animation info
            UI_Animation_Info animation_info = {0};
            {
                animation_info.hot_animation_rate      = ti_state->catchall_animation_rate;
                animation_info.active_animation_rate   = ti_state->catchall_animation_rate;
                animation_info.focus_animation_rate    = 1.f;
                animation_info.tooltip_animation_rate  = ti_state->tooltip_animation_rate;
                animation_info.menu_animation_rate     = ti_state->menu_animation_rate;
                animation_info.scroll_animation_rate   = ti_state->scrolling_animation_rate;
            }

            // begin & push initial stack values
            ui_begin_build(ws->os, &ws->ui_events, &icon_info, ws->theme, &animation_info, ti_state->frame_dt, ti_state->frame_dt);
            ui_push_font(ti_font_from_slot(TI_FontSlot_Main));
            ui_push_font_size(top_level_font_size);
            ui_push_text_padding(floor_f32(ui_top_font_size() * 0.3f));
            ui_push_pref_width(ui_px(floor_f32(ui_top_font_size() * 20.f), 1.f));
            ui_push_pref_height(ui_px(floor_f32(ui_top_font_size() * 3.f), 1.f));
            ui_push_blur_size(10.f);
        }

        // @window_ui_part calculate top-level rectangles/sizes
        f32 window_edge_px = ui_top_font_size() * 0.2f;
        Rng2f32 window_rect = wm_client_rect_from_window(ws->os);
        Vec2f32 window_rect_dim = dim_2f32(window_rect);
        f32 top_bar_dim_px = floor_f32(ui_top_font_size() * 3.f);
        Rng2f32 top_bar_rect = r2f32p(window_rect.x0,
                                      window_rect.y0,
                                      window_rect.x0 + window_rect_dim.x,
                                      window_rect.y0 + top_bar_dim_px);
        Rng2f32 bottom_bar_rect = r2f32p(window_rect.x0,
                                         window_rect_dim.y - top_bar_dim_px,
                                         window_rect.x0 + window_rect_dim.x,
                                         window_rect.y0 + window_rect_dim.y);
        Rng2f32 content_rect = r2f32p(window_rect.x0,
                                      top_bar_rect.y1 + window_edge_px * 1.5f,
                                      window_rect.x0 + window_rect_dim.x,
                                      bottom_bar_rect.y0);
        content_rect = pad_2f32(content_rect, -window_edge_px);

        ///////////////////////
        // @window_ui_part truncated string hover
        if (ui_string_hover_active()) UI_Tooltip
        {
            Temp scratch = scratch_begin(0, 0);
            DR_FStr_List fstrs = ui_string_hover_fstrs(scratch.arena);
            UI_Box *box = ui_build_box_from_key(UI_BoxFlag_DrawText, ui_key_zero());
            ui_box_equip_display_fstrs(box, &fstrs);
            scratch_end(scratch);
        }

        ///////////////////
        // @window_ui_part gather all tasks to build floating views
        //
        typedef struct Floating_View_Task Floating_View_Task;
        struct Floating_View_Task
        {
            Floating_View_Task *next;
            CFG_Node *view;
            TI_Regs *regs;
            Rng2f32 rect;
            bool32 is_focused;
            bool32 is_anchored;
            bool32 force_inside_window_x;
            bool32 force_inside_window_y;
            bool32 only_secondary_navigation;
            bool32 reset_open;
            bool32 disable_open_animation;
            UI_Signal signal; // NOTE: output, from build
            bool32 pressed;
            bool32 pressed_outside;
        };
        Floating_View_Task *query_floating_view_task = 0;
        Floating_View_Task *first_floating_view_task = 0;
        Floating_View_Task *last_floating_view_task = 0;
        TI_Font(TI_FontSlot_Code)
        {
            // force-close queries if:
            //
            // - query is anchored, but box is gone
            // - query has expression, but that expression does not evaluate
            //
            TI_Query_View *top_query = ws->query_top;
            {
                u64 remove_q_count = 0;
                u64 q_count = 0;
                for (TI_Query_View *q = top_query; q != 0; q = q->next, q_count += 1)
                {
                    // close if expression is busted
                    if (q->regs->cmd_name.size == 0)
                    {
                        String8 expr = q->regs->expr;
                        TI_Query_Item_List items = ti_query_items_from_string(scratch.arena, expr, str8_zero());
                        if (items.count == 0)
                        {
                            top_query = q->next;
                            remove_q_count = q_count+1;
                        }
                    }

                    // close if anchor box is gone
                    {
                        UI_Box *box = ui_box_from_key(q->regs->ui_key);
                        if (!ui_key_match(ui_key_zero(), q->regs->ui_key) && ui_box_is_nil(box))
                        {
                            top_query = q->next;
                            remove_q_count = q_count;
                        }
                    }
                }
                for EachIndex(idx, remove_q_count)
                {
                    ti_cmd(TI_CmdKind_CancelQuery);
                }
            }
            query_is_open = (top_query != 0);

            // touch immediate cfgs / view states for all active queries
            for (TI_Query_View *qv = top_query; qv != 0; qv = qv->next)
            {
                CFG_Node *root = ti_immediate_cfg_from_keyf("window_query_%p_%I64u", window, qv->q_arena_pos);
                CFG_Node *view = cfg_node_child_from_string(root, s("watch"));
                TI_View_State *vs = ti_view_state_from_cfg(view);
            }

            // try to add opened query
            if (query_is_open)
            {
                // unpack view for query
                TI_Regs *query_regs = top_query->regs;
                CFG_Node *root = ti_immediate_cfg_from_keyf("window_query_%p_%I64u", window, top_query->q_arena_pos);
                CFG_Node *view = cfg_node_child_from_string_or_alloc(ti_state->cfg, root, s("watch"));
                CFG_Node *query = cfg_node_child_from_string_or_alloc(ti_state->cfg, view, s("query"));
                bool32 is_lister = (cfg_node_child_from_string(view, s("lister")) != &cfg_nil_node);
                bool32 is_small = (cfg_node_child_from_string(view, s("small")) != &cfg_nil_node);
                bool32 root_is_explicit = (cfg_node_child_from_string(view, s("explicit_root")) != &cfg_nil_node);
                TI_View_State *vs = ti_view_state_from_cfg(view);

                // if this is not the bottom of the query stack, never animate
                bool32 disable_open_animation = false;
                if (top_query->next != 0)
                {
                    disable_open_animation = true;
                }

                // did this view ID change? -> reset open animation (for bottom-of-stack queries)
                bool32 reset_open = false;
                if (top_query->next == 0 && view->id != ws->query_last_bottom_view_id)
                {
                    ws->query_last_bottom_view_id = view->id;
                    reset_open = true;
                }
                else if (top_query->next == 0 && view->id == ws->query_last_bottom_view_id)
                {
                    disable_open_animation = true;
                }

                // unpack query info
                String8 cmd_name = query_regs->cmd_name;
                TI_Cmd_Kind_Info *cmd_kind_info = ti_cmd_kind_info_from_string(cmd_name);
                String8 query_expr = query_regs->expr;
                if (query_expr.size == 0 && cmd_name.size != 0)
                {
                    query_expr = cmd_kind_info->query.expr;
                }
                bool32 query_is_anchored = (!ui_box_is_nil(ui_box_from_key(query_regs->ui_key)));
                bool32 size_query_by_expr_eval = (query_is_anchored || query_expr.size == 0);

                // compute query expression
                if (query_expr.size == 0)
                {
                    query_expr = str8(vs->query_buffer, vs->query_string_size);
                }
                else
                {
                    u64 input_insertion_pos = str8_find_needle(query_expr, 0, str8_lit("$input"), 0);
                    if (input_insertion_pos < query_expr.size)
                    {
                        String8 pre_insertion = str8_prefix(query_expr, input_insertion_pos);
                        String8 post_insertion = str8_skip(query_expr, input_insertion_pos + 6);
                        String8 input_text = str8(vs->query_buffer, vs->query_string_size);
                        String8 input_text__escaped = escaped_from_raw_str8(scratch.arena, input_text);
                        // TODO: @hack need to escape because this is putting the user's input
                        // into a containing "folder:"..."" in all cases. but this is kinda shady
                        // and should be replaced long-term with something more solid...
                        query_expr = push_str8f(scratch.arena, "%S%S%S", pre_insertion, input_text__escaped, post_insertion);
                    }
                }

                // store expression
                CFG_Node *expr = cfg_node_child_from_string_or_alloc(ti_state->cfg, view, str8_lit("expression"));
                cfg_node_new_replace(ti_state->cfg, expr, query_expr);

                // evaluate query expression
                TI_Query_Item_List items = ti_query_items_from_string(scratch.arena, query_expr, ti_view_query_input());

                if (query_regs->do_big_rows)
                {
                    f32 row_height = 5.f;
                    f32 row_height_px = row_height*ui_top_font_size();
                    CFG_Node *row_height_root = cfg_node_child_from_string_or_alloc(ti_state->cfg, view, str8_lit("row_height"));
                    cfg_node_new_replacef(ti_state->cfg, row_height_root, "%f", row_height);
                }

                // compute query view's top-level rectangle
                Rng2f32 rect = {0};
                {
                    f32 row_height_px = ui_top_font_size()*ti_setting_f32_from_name(str8_lit("row_height"));
                    Vec2f32 content_rect_center = center_2f32(content_rect);
                    Vec2f32 content_rect_dim = dim_2f32(content_rect);
                    f32 query_width_px = floor_f32(content_rect_dim.x*0.35f);
                    f32 max_query_height_px = content_rect_dim.y*0.8f;
                    if (is_small)
                    {
                        query_width_px = floor_f32(content_rect_dim.x*0.15f);
                        max_query_height_px = content_rect_dim.y*0.3f;
                    }
                    if (!ui_key_match(ui_key_zero(), query_regs->ui_key))
                    {
                        query_width_px = is_small ? (ui_top_font_size()*40.f) : (ui_top_font_size()*60.f);
                        max_query_height_px = is_small ? (ui_top_font_size()*40.f) : (ui_top_font_size()*80.f);
                    }
                    f32 query_height_px = max_query_height_px;
                    if (size_query_by_expr_eval)
                    {
                        f32 search_row_open_t = ui_anim(ui_key_from_stringf(ui_key_zero(), "search_row_open_%p", view),
                                                        (f32)!!vs->query_is_open,
                                                        .initial = (f32)!!vs->query_is_open,
                                                        .epsilon = 0.001f,
                                                        .rate = ti_state->menu_animation_rate);
                        query_height_px = row_height_px*items.count + ui_top_px_height()*search_row_open_t;
                        query_height_px = Min(query_height_px, max_query_height_px);
                    }
                    rect = r2f32p(content_rect_center.x - query_width_px/2,
                                  content_rect_center.y - max_query_height_px/2.f,
                                  content_rect_center.x + query_width_px/2,
                                  content_rect_center.y - max_query_height_px/2.f + query_height_px);
                    if (!ui_key_match(ui_key_zero(), query_regs->ui_key))
                    {
                        UI_Box *anchor_box = ui_box_from_key(query_regs->ui_key);
                        if (anchor_box != &ui_nil_box)
                        {
                            rect.x0 = anchor_box->rect.x0 + query_regs->off_px.x;
                            rect.y0 = anchor_box->rect.y1 + query_regs->off_px.y;
                            rect.x1 = rect.x0 + query_width_px;
                            rect.y1 = rect.y0 + query_height_px;
                        }
                    }
                }

                // push query task
                {
                    Floating_View_Task *t = push_array(scratch.arena, Floating_View_Task, 1);
                    SLLQueuePush(first_floating_view_task, last_floating_view_task, t);
                    query_floating_view_task = t;
                    t->view          = view;
                    t->regs          = query_regs;
                    t->rect          = rect;
                    t->is_focused    = 1;
                    t->is_anchored   = query_is_anchored;
                    t->reset_open    = reset_open;
                    t->disable_open_animation = disable_open_animation;
                    t->force_inside_window_x = 1;
                    t->force_inside_window_y = 1;
                }
            }
        }

        //////////////////
        // @window_ui_part build all floating views
        //
        ProfScope("build all floating views")
            TI_Font(TI_FontSlot_Code)
            UI_TagF("floating")
            UI_Focus(ui_any_ctx_menu_is_open() || ws->menu_bar_focused ? UI_FocusKind_Off : UI_FocusKind_Null)
        {
            f32 fast_open_rate = ti_state->menu_animation_rate;
            f32 slow_open_rate = ti_state->menu_animation_rate__slow;
            for (Floating_View_Task *t = first_floating_view_task; t != 0; t = t->next)
            {
                // unpack
                CFG_Node *view       = t->view;
                Rng2f32 rect         = t->rect;
                bool32 is_focused    = t->is_focused;
                bool32 is_anchored   = t->is_anchored;
                bool32 is_lister = (cfg_node_child_from_string(view, str8_lit("lister")) != &cfg_nil_node);
                bool32 only_secondary_navigation = t->only_secondary_navigation;
                f32 open_t           = ui_anim(ui_key_from_stringf(ui_key_zero(), "floating_view_open_%p", view), 1.f,
                                               .rate = is_anchored ? fast_open_rate : slow_open_rate,
                                               .reset = t->reset_open,
                                               .initial = t->disable_open_animation ? 1.f : 0.f);

                // force rect inside window if needed
                if (t->force_inside_window_x || t->force_inside_window_y)
                {
                    bool32 axis_mask[] = {t->force_inside_window_x, t->force_inside_window_y};
                    Rng2f32 window_rect = wm_client_rect_from_window(ws->os);
                    for EachEnumVal(Axis2, axis)
                    {
                        if (!axis_mask[axis]) { continue; }
                        f32 max_delta = rect.p1.v[axis] - window_rect.p1.v[axis];
                        f32 min_delta = window_rect.p0.v[axis] - rect.p0.v[axis];
                        f32 total_delta = Max(min_delta, 0) - Max(max_delta, 0);
                        rect.p0.v[axis] += total_delta;
                        rect.p1.v[axis] += total_delta;
                    }
                }

                // push view regs
                ti_push_regs();
                {
                    if (t->regs != 0)
                    {
                        ti_regs()->cfg = t->regs->cfg;
                    }
                    ti_regs()->view = view->id;
                    String8 view_expr = ti_expr_from_cfg(view);
                    String8 view_file_path = ti_file_path_from_eval_string(ti_frame_arena(), view_expr);
                    // NOTE: we want to only fill out this view's file path slot if it
                    // evaluates one - this way, a view can use the slot to know the selected
                    // file path (if there is one). this is useful when pushing commands which
                    // apply to a cursor, for example.
                    if (view_file_path.size != 0)
                    {
                        ti_regs()->file_path = view_file_path;
                    }
                }

                // build
                UI_Focus(is_focused ? UI_FocusKind_On : UI_FocusKind_Off)
                    UI_PermissionFlags(only_secondary_navigation ?
                                       UI_PermissionFlag_KeyboardSecondary|UI_PermissionFlag_Clicks|UI_PermissionFlag_ScrollX|UI_PermissionFlag_ScrollY :
                                       UI_PermissionFlag_All)
                {
                    bool32 has_footer = (is_lister && !is_anchored);
                    f32 container_corner_radius_px = has_footer ? ui_top_font_size()*1.f : ui_top_font_size()*0.15f;

                    // build top-level container box
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

                    // peek press inside/outside events
                    {
                        for (UI_Event *evt = 0; ui_next_event(&evt);)
                        {
                            if (evt->kind == UI_EventKind_Press &&
                                evt->key == WM_Key_LeftMouseButton)
                            {
                                if (contains_2f32(container->rect, evt->pos))
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

                    // build overlay container for loading animation
                    UI_Box *loading_overlay_container = &ui_nil_box;
                    UI_Parent(container) UI_WidthFill UI_HeightFill
                    {
                        loading_overlay_container = ui_build_box_from_key(UI_BoxFlag_Floating, ui_key_zero());
                    }

                    // build contents
                    f32 total_height_px = dim_2f32(rect).y;
                    f32 footer_height_px = 0;
                    if (has_footer)
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
                            ti_view_ui(rect);
                        }
                    }

                    // build footer
                    if (is_lister && !is_anchored) UI_Parent(container) UI_WidthFill UI_HeightFill UI_Focus(UI_FocusKind_Off)
                    {
                        ui_set_next_flags(UI_BoxFlag_DrawBackground|UI_BoxFlag_DrawBorder);
                        UI_Column UI_Padding(ui_em(1, 1))
                        {
                            UI_Row UI_Padding(ui_pct(1, 0)) TI_Font(TI_FontSlot_Main) UI_FontSize(ui_top_font_size()*0.8f) UI_TagF("alt")
                                UI_PrefWidth(ui_text_dim(1, 1)) UI_TextAlignment(UI_TextAlign_Center)
                            {
                                /*
                                ti_cmd_binding_buttons(ti_cmd_kind_info_table[TI_CmdKind_MoveUp].string, s(""), 1, TI_CmdBindingButtonFlag_NoEdit);
                                ui_spacer(ui_em(1.f, 1.f));
                                ui_label(s("/"));
                                ti_cmd_binding_buttons(ti_cmd_kind_info_table[TI_CmdKind_MoveDown].string, s(""), 1, TI_CmdBindingButtonFlag_NoEdit);
                                ui_label(s("to navigate"));

                                ui_spacer(ui_em(1, 1));

                                ti_cmd_binding_buttons(ti_cmd_kind_info_table[TI_CmdKind_Accept].string, s(""), 1, TI_CmdBindingButtonFlag_NoEdit);
                                ui_label(s("to use"));

                                ui_spacer(ui_em(1, 1));

                                ui_cmd_binding_buttons(ti_cmd_kind_info_table[TI_CmdKind_Cancel].string, s(""), 1, TI_CmdBindingButtonFlag_NoEdit);
                                ui_label(s("to cancel"));
                                */
                            }
                        }
                    }

                    // build loading overlay
                    {
                        TI_View_State *vs = ti_view_state_from_cfg(view);
                        f32 loading_t = vs->loading_t;
                        if (loading_t > 0.01f) UI_Parent(loading_overlay_container)
                        {
                            ti_loading_overlay(rect, loading_t, vs->loading_progress_v, vs->loading_progress_v_target);
                        }
                    }

                    // interact with container
                    UI_Signal sig = ui_signal_from_box(container);
                    t->signal = sig;
                }

                // pop interaction registers; commit if this is focused
                TI_Regs *view_regs = ti_pop_regs();
                if (is_focused)
                {
                    MemoryCopyStruct(ti_regs(), view_regs);
                }

                // is not anchored? -> darken rest of screen
                if (!is_anchored)
                {
                    UI_TagF("inactive") UI_Transparency(1-open_t) UI_Rect(content_rect) ui_build_box_from_key(UI_BoxFlag_DrawBackground|UI_BoxFlag_Floating, ui_key_zero());
                }

                // autocompletion view early-closing rules

                // hover eval focus rules

                // query interactions
                if (t == query_floating_view_task)
                {
                    CFG_Node *view = query_floating_view_task->view;
                    TI_View_State *vs = ti_view_state_from_cfg(query_floating_view_task->view);
                    String8 cmd_name = query_floating_view_task->regs->cmd_name;
                    TI_Cmd_Kind_Info *cmd_kind_info = ti_cmd_kind_info_from_string(cmd_name);

                    // close queries
                    if (query_floating_view_task->pressed_outside ||
                        (cfg_node_child_from_string(view, str8_lit("lister")) != &cfg_nil_node && !vs->query_is_open) ||
                        (cmd_name.size != 0 && !vs->query_is_open) ||
                        ui_slot_press(UI_EventActionSlot_Cancel))
                    {
                        ti_cmd(TI_CmdKind_CancelQuery);
                    }

                    // any queries which take a file path mutate the program's "current path"
                    if (cmd_kind_info->query.slot == TI_RegSlot_FilePath)
                    {
                        CFG_Node *query = cfg_node_child_from_string(view, str8_lit("query"));
                        CFG_Node *input = cfg_node_child_from_string(query, str8_lit("input"));
                        if (input != &cfg_nil_node)
                        {
                            String8 path_chopped = str8_chop_last_slash(input->first->string);
                            CFG_Node *user = cfg_node_child_from_string(cfg_node_root(), str8_lit("user"));
                            CFG_Node *current_path = cfg_node_child_from_string_or_alloc(ti_state->cfg, user, str8_lit("current_path"));
                            // set current path
                        }
                    }
                }
            }
        }

        ///////////////////
        // @window_ui_part top bar
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

                // left column
                {
                    ui_set_next_flags(UI_BoxFlag_Clip|UI_BoxFlag_ViewScrollX|UI_BoxFlag_ViewClamp);
                    UI_WidthFill UI_NamedRow(str8_lit("###menu_bar"))
                    {
                        // icon
                        UI_Padding(ui_em(0.5f, 1.f))
                        {
                            UI_PrefWidth(ui_px(dim_2f32(top_bar_rect).y - ui_top_font_size()*0.8f, 1.f))
                                UI_Column
                                UI_Padding(ui_em(0.4f, 1.f))
                                UI_HeightFill
                            {
                                R_Handle texture = ti_state->icon_texture;
                                Vec2s32 texture_dim = r_size_from_tex2d(texture);
                                ui_image(texture, R_Tex2DSampleKind_Linear, r2f32p(0, 0, texture_dim.x, texture_dim.y), v4f32(1, 1, 1, 1), 0, str8_lit(""));
                            }
                        }

                        // menu items
                        
                        if (dim_2f32(top_bar_rect).x > ui_top_font_size()*60)
                        {
                            ui_set_next_flags(UI_BoxFlag_DrawBackground);
                            UI_PrefWidth(ui_children_sum(1)) UI_Row UI_PrefWidth(ui_text_dim(20, 1)) UI_GroupKey(menu_bar_group_key)
                            {
                                // file menu
                                UI_Key file_menu_key = ui_key_from_string(ui_key_zero(), str8_lit("_file_menu_key_"));
                                UI_CtxMenu(file_menu_key) UI_PrefWidth(ui_em(50.f, 1.f)) UI_TagF("implicit")
                                {
                                    String8 cmds[] = {
                                        ti_cmd_kind_info_table[TI_CmdKind_Exit].string,
                                    };
                                    u32 codepoints[] = {
                                        'x',
                                    };
                                    StaticAssert(ArrayCount(codepoints) == ArrayCount(cmds), menu_button_check);
                                    ti_cmd_list_menu_buttons(ArrayCount(cmds), cmds, codepoints);
                                }
                                
                                // window menu
                                UI_Key window_menu_key = ui_key_from_string(ui_key_zero(), str8_lit("_window_menu_key_"));
                                UI_CtxMenu(window_menu_key) UI_PrefWidth(ui_em(50.f, 1.f)) UI_TagF("implicit")
                                {
                                    String8 cmds[] = {
                                        ti_cmd_kind_info_table[TI_CmdKind_OpenWindow].string,
                                        ti_cmd_kind_info_table[TI_CmdKind_CloseWindow].string,
                                    };
                                    u32 codepoints[] = {
                                        'w',
                                        'c',
                                    };
                                    StaticAssert(ArrayCount(codepoints) == ArrayCount(cmds), menu_button_check);
                                    ti_cmd_list_menu_buttons(ArrayCount(cmds), cmds, codepoints);
                                }
                                
                                // panel menu
                                
                                // view menu
                                
                                // help menu
                                
                                // buttons
                                UI_TextAlignment(UI_TextAlign_Center) UI_HeightFill
                                {
                                    // set up table
                                    struct
                                    {
                                        String8 name;
                                        u32 codepoint;
                                        WM_Key key;
                                        UI_Key menu_key;
                                    } items[] = {
                                        {str8_lit("File"),       'f', WM_Key_F, file_menu_key},
                                        {str8_lit("Window"),     'w', WM_Key_W, window_menu_key},
                                    };
                                    
                                    // determine if one of the menus is already open
                                    bool32 menu_open = false;
                                    u64 open_menu_idx = 0;
                                    for (u64 idx = 0; idx < ArrayCount(items); idx += 1)
                                    {
                                        if (ui_ctx_menu_is_open(items[idx].menu_key))
                                        {
                                            menu_open = true;
                                            open_menu_idx = idx;
                                            break;
                                        }
                                    }
                                    
                                    // navigate between menus
                                    u64 open_menu_idx_prime = open_menu_idx;
                                    if (menu_open && ws->menu_bar_focused && window_is_focused)
                                    {
                                        for (UI_Event *evt = 0; ui_next_event(&evt);)
                                        {
                                            bool32 taken = false;
                                            if (evt->delta_2s32.x > 0)
                                            {
                                                taken = true;
                                                open_menu_idx_prime += 1;
                                                open_menu_idx_prime = open_menu_idx_prime%ArrayCount(items);
                                            }
                                            if (evt->delta_2s32.x < 0)
                                            {
                                                taken = true;
                                                open_menu_idx_prime = open_menu_idx_prime > 0 ? open_menu_idx_prime - 1 : (ArrayCount(items) - 1);
                                            }
                                            if (taken)
                                            {
                                                ui_eat_event(evt);
                                            }
                                        }
                                    }
                                    
                                    // make ui
                                    UI_TagF("implicit")
                                        UI_VisualMarginX(ui_top_font_size()*0.45f)
                                        UI_VisualMarginY(ui_top_font_size()*0.5f)
                                        UI_CornerRadius(ui_top_font_size()*0.5f)
                                        for (u64 idx = 0; idx < ArrayCount(items); idx += 1)
                                    {
                                        ui_set_next_fastpath_codepoint(items[idx].codepoint);
                                        if ((ws->menu_bar_key_held || ws->menu_bar_focused) && !ui_any_ctx_menu_is_open())
                                        {
                                            ui_set_next_flags(UI_BoxFlag_DrawTextFastpathCodepoint);
                                        }
                                        UI_TagF(!ui_ctx_menu_is_open(items[idx].menu_key) ? "weak" : "")
                                        {
                                            UI_Signal sig = ti_menu_bar_button(items[idx].name);
                                            wm_window_push_custom_title_bar_client_area(ws->os, sig.box->rect);
                                            if (menu_open)
                                            {
                                                if ((ui_hovering(sig) && !ui_ctx_menu_is_open(items[idx].menu_key))
                                                    || (open_menu_idx_prime == idx && open_menu_idx_prime != open_menu_idx))
                                                {
                                                    ui_ctx_menu_open(items[idx].menu_key, sig.box->key, v2f32(0, sig.box->rect.y1 - sig.box->rect.y0));
                                                }
                                            }
                                            else if (ui_pressed(sig))
                                            {
                                                if (ui_ctx_menu_is_open(items[idx].menu_key))
                                                {
                                                    ui_ctx_menu_close();
                                                }
                                                else
                                                {
                                                    ui_ctx_menu_open(items[idx].menu_key, sig.box->key, v2f32(0, sig.box->rect.y1 - sig.box->rect.y0));
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                
                // center column
                if (dim_2f32(top_bar_rect).x > ui_top_font_size()*60)
                    UI_PrefWidth(ui_children_sum(1.f)) UI_Row
                    UI_PrefWidth(ui_px(dim_2f32(top_bar_rect).y, 1))
                    TI_Font(TI_FontSlot_Icons)
                    UI_FontSize(ui_top_font_size()*0.85f)
                    UI_TagF("implicit")
                    UI_CornerRadius(ui_top_font_size()*1.f)
                    UI_VisualMargin(ui_top_font_size()*0.5f)
                {
                    
                }

                // right column
                UI_WidthFill UI_Row
                {
                    ui_spacer(ui_pct(1, 0));
                    
                    // close dropdown
                    UI_Key close_ctx_menu_key = ui_key_from_stringf(ui_key_zero(), "###close_ctx_menu");
                    UI_CtxMenu(close_ctx_menu_key) UI_TagF("implicit")
                    {
                        if (ui_clicked(ti_icon_buttonf(TI_IconKind_Window, 0, "Close Window")))
                        {
                            ti_cmd(TI_CmdKind_CloseWindow);
                        }
                        if (ui_clicked(ti_icon_buttonf(TI_IconKind_X, 0, "Exit")))
                        {
                            ti_cmd(TI_CmdKind_Exit);
                        }
                    }

                    // min/max/close buttons
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
                            min_sig = ti_icon_buttonf(TI_IconKind_WindowMinimize, 0, "##minimize");
                            max_sig = ti_icon_buttonf(wm_window_is_maximized(ws->os) ? TI_IconKind_WindowRestore : TI_IconKind_Window, 0, "##maximize");
                        }
                        UI_PrefWidth(ui_px(button_dim, 1.f))
                            UI_FontSize(ui_top_font_size()*0.85f)
                        {
                            cls_sig = ti_icon_buttonf(TI_IconKind_X, 0, "##close");
                        }
                        if (ui_clicked(min_sig))
                        {
                            wm_window_set_minimized(ws->os, 1);
                        }
                        if (ui_clicked(max_sig))
                        {
                            wm_window_set_maximized(ws->os, !wm_window_is_maximized(ws->os));
                        }
                        if (ui_clicked(cls_sig))
                        {
                            if (ws->order_next != &ti_nil_window_state ||
                                ws->order_prev != &ti_nil_window_state)
                            {
                                ui_ctx_menu_open(close_ctx_menu_key, cls_sig.box->key, v2f32(0, dim_2f32(cls_sig.box->rect).y));
                            }
                            else
                            {
                                ti_cmd(TI_CmdKind_Exit);
                            }
                        }
                        wm_window_push_custom_title_bar_client_area(ws->os, min_sig.box->rect);
                        wm_window_push_custom_title_bar_client_area(ws->os, max_sig.box->rect);
                        wm_window_push_custom_title_bar_client_area(ws->os, pad_2f32(cls_sig.box->rect, 2.f));
                    }
                }
            }
        }

        ///////////////////////
        // @window_ui_part bottom bar
        //
        {}

        ///////////////////
        // @window_ui_part animate panels
        //
        {
            bool32 window_is_resizing = (ws->last_window_rect.x1 != window_rect.x1 ||
                                         ws->last_window_rect.y1 != window_rect.y1);
            Vec2f32 content_rect_dim = dim_2f32(content_rect);
            if (content_rect_dim.x > 0 && content_rect_dim.y > 0)
            {
                for (CFG_Panel_Node *panel = panel_tree.root;
                     panel != &cfg_nil_panel_node;
                     panel = cfg_panel_node_rec__depth_first_pre(panel_tree.root, panel).next)
                {
                    Rng2f32 target_rect_px = cfg_target_rect_from_panel_node(content_rect, panel_tree.root, panel);
                    Rng2f32 target_rect_pct = r2f32p(target_rect_px.x0/content_rect_dim.x,
                                                     target_rect_px.y0/content_rect_dim.y,
                                                     target_rect_px.x1/content_rect_dim.x,
                                                     target_rect_px.y1/content_rect_dim.y);
                    bool32 reset = (window_is_resizing || ws->window_layout_reset || ws->frames_alive < 5);// || is_changing_panel_boundaries);
                    ui_anim(ui_key_from_stringf(ui_key_zero(), "panel_%p_x0", panel->cfg), target_rect_pct.x0, .initial = target_rect_pct.x0, .reset = reset, .rate = ti_state->menu_animation_rate);
                    ui_anim(ui_key_from_stringf(ui_key_zero(), "panel_%p_y0", panel->cfg), target_rect_pct.y0, .initial = target_rect_pct.y0, .reset = reset, .rate = ti_state->menu_animation_rate);
                    ui_anim(ui_key_from_stringf(ui_key_zero(), "panel_%p_x1", panel->cfg), target_rect_pct.x1, .initial = target_rect_pct.x1, .reset = reset, .rate = ti_state->menu_animation_rate);
                    ui_anim(ui_key_from_stringf(ui_key_zero(), "panel_%p_y1", panel->cfg), target_rect_pct.y1, .initial = target_rect_pct.y1, .reset = reset, .rate = ti_state->menu_animation_rate);
                }
            }
            ws->window_layout_reset = 0;
        }

        //////////////////////////
        // @window_ui_part panel leaf UI
        //
        if (content_rect.x1 > content_rect.x0 && content_rect.y1 > content_rect.y0)
        {
            ProfScope("leaf panel UI")
                for (CFG_Panel_Node *panel = panel_tree.root;
                     panel != &cfg_nil_panel_node;
                     panel = cfg_panel_node_rec__depth_first_pre(panel_tree.root, panel).next)
            {
                if (panel->first != &cfg_nil_panel_node) {continue;}
                bool32 panel_is_focused = (window_is_focused &&
                                           //!ti_state->popup_active &&
                                           !ws->menu_bar_focused &&
                                           //!query_is_open &&
                                           !ui_any_ctx_menu_is_open() &&
                                           //!ws->hover_eval_focused &&
                                           panel_tree.focused == panel);
                CFG_Node *selected_tab = panel->selected_tab;
                TI_View_State *selected_tab_view_state = ti_view_state_from_cfg(selected_tab);
                ProfScope("leaf panel UI work - %.*s", str8_varg(selected_tab->string))
                    UI_Focus(panel_is_focused ? UI_FocusKind_Null : UI_FocusKind_Off)
                {
                    //////////////
                    // calculate UI rectangles
                    //
                    Vec2f32 content_rect_dim = dim_2f32(content_rect);
                    Rng2f32 target_rect_px = cfg_target_rect_from_panel_node(content_rect, panel_tree.root, panel);
                    Rng2f32 target_rect_pct = r2f32p(target_rect_px.x0/content_rect_dim.x,
                                                     target_rect_px.y0/content_rect_dim.y,
                                                     target_rect_px.x1/content_rect_dim.x,
                                                     target_rect_px.y1/content_rect_dim.y);
                    Rng2f32 panel_rect_pct = r2f32p(ui_anim(ui_key_from_stringf(ui_key_zero(), "panel_%p_x0", panel->cfg), target_rect_pct.x0, .initial = target_rect_pct.x0, .rate = ti_state->menu_animation_rate),
                                                    ui_anim(ui_key_from_stringf(ui_key_zero(), "panel_%p_y0", panel->cfg), target_rect_pct.y0, .initial = target_rect_pct.y0, .rate = ti_state->menu_animation_rate),
                                                    ui_anim(ui_key_from_stringf(ui_key_zero(), "panel_%p_x1", panel->cfg), target_rect_pct.x1, .initial = target_rect_pct.x1, .rate = ti_state->menu_animation_rate),
                                                    ui_anim(ui_key_from_stringf(ui_key_zero(), "panel_%p_y1", panel->cfg), target_rect_pct.y1, .initial = target_rect_pct.y1, .rate = ti_state->menu_animation_rate));
                    Rng2f32 panel_rect = r2f32p(panel_rect_pct.x0*content_rect_dim.x,
                                                panel_rect_pct.y0*content_rect_dim.y,
                                                panel_rect_pct.x1*content_rect_dim.x,
                                                panel_rect_pct.y1*content_rect_dim.y);
                    panel_rect = pad_2f32(panel_rect, floor_f32(-ui_top_font_size()*0.25f));
                    panel_rect = r2f32p(round_f32(panel_rect.x0), round_f32(panel_rect.y0), round_f32(panel_rect.x1), round_f32(panel_rect.y1));
                    f32 tab_bar_rheight = floor_f32(ui_top_font_size()*3.5f);
                    f32 tab_bar_vheight = floor_f32(ui_top_font_size()*2.f);//*ti_setting_f32_from_name(str8_lit("tab_height")));
                    f32 tab_bar_rv_diff = tab_bar_rheight - tab_bar_vheight;
                    f32 tab_spacing = floor_f32(ui_top_font_size()*0.4f);
                    Rng2f32 tab_bar_rect = r2f32p(panel_rect.x0, panel_rect.y0, panel_rect.x1, panel_rect.y0 + tab_bar_vheight);
                    Rng2f32 content_rect = r2f32p(panel_rect.x0, panel_rect.y0+tab_bar_vheight, panel_rect.x1, panel_rect.y1);
                    if (panel->tab_side == Side_Max)
                    {
                        tab_bar_rect.y0 = panel_rect.y1 - tab_bar_vheight;
                        tab_bar_rect.y1 = panel_rect.y1;
                        content_rect.y0 = panel_rect.y0;
                        content_rect.y1 = panel_rect.y1 - tab_bar_vheight;
                    }
                    tab_bar_rect = intersect_2f32(tab_bar_rect, panel_rect);
                    content_rect = intersect_2f32(content_rect, panel_rect);
                    
                    ////////////////////
                    // decide to skip this panel (e.g. if it is too small)
                    //
                    bool32 build_panel = (content_rect.x1 > content_rect.x0 && content_rect.y1 > content_rect.y0);
                    
                    ////////////////////
                    // panel not selecteed? -> darken
                    //
                    if (build_panel) if (panel != panel_tree.focused)
                    {
                        UI_Rect(content_rect) UI_TagF("inactive")
                            ui_build_box_from_key(UI_BoxFlag_DrawBackground, ui_key_zero());
                    }
                    
                    ///////////////////
                    // build panel container box
                    //
                    UI_Box *panel_box = &ui_nil_box;
                    if (build_panel) UI_Rect(content_rect) UI_ChildLayoutAxis(Axis2_Y) UI_CornerRadius(0) UI_Focus(UI_FocusKind_On)
                    {
                        UI_Key panel_key = ui_key_from_stringf(ui_key_zero(), "panel_box_%p", panel->cfg);
                        panel_box = ui_build_box_from_key(UI_BoxFlag_MouseClickable|
                                                          UI_BoxFlag_Clip|
                                                          UI_BoxFlag_DrawBorder|
                                                          UI_BoxFlag_DisableFocusOverlay|
                                                          ((panel_tree.focused != panel)*UI_BoxFlag_DisableFocusBorder),
                                                          panel_key);
                    }
                    
                    ///////////////
                    // loading animation for stable view
                    //
                    UI_Box *loading_overlay_container = &ui_nil_box;
                    if (build_panel) UI_Parent(panel_box) UI_WidthFill UI_HeightFill
                    {
                        loading_overlay_container = ui_build_box_from_key(UI_BoxFlag_Floating, ui_key_zero());
                    }

                    ////////////////////
                    // build selected tab view
                    //
                    if (build_panel)
                        UI_Parent(panel_box)
                        UI_Focus(panel_is_focused ? UI_FocusKind_Null : UI_FocusKind_Off)
                        UI_WidthFill
                    {
                        // push interaction registers, fill with per-view states
                        ti_push_regs(.panel = panel->cfg->id,
                                     .tab = selected_tab->id,
                                     .view = selected_tab->id);
                        
                        // build view container
                        UI_Box *view_container_box = &ui_nil_box;
                        UI_FixedWidth(dim_2f32(content_rect).x)
                            UI_FixedHeight(dim_2f32(content_rect).y)
                            UI_ChildLayoutAxis(Axis2_Y)
                        {
                            view_container_box = ui_build_box_from_key(0, ui_key_zero());
                        }
                        
                        // build empty view
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
                                    if (ui_clicked(ti_icon_buttonf(TI_IconKind_X, 0, "Close Panel")))
                                    {
                                        ti_cmd(TI_CmdKind_ClosePanel);
                                    }
                                }
                            }
                        }
                        
                        // build tab view
                        UI_Parent(view_container_box) if(selected_tab != &cfg_nil_node) ProfScope("build tab view")
                        {
                            ti_view_ui(content_rect);
                        }

                        // pop interaction registers; commit if this is the selected view
                        TI_Regs *view_regs = ti_pop_regs();
                        if (panel_tree.focused == panel)
                        {
                            MemoryCopyStruct(ti_regs(), view_regs);
                        }
                    }
                    
                    /////////////
                    // loading? -> fill loading overlay container
                    //
                    if (build_panel)
                    {
                        f32 selected_tab_loading_t = selected_tab_view_state->loading_t;
                        if (selected_tab_loading_t > 0.01f) UI_Parent(loading_overlay_container)
                        {
                            ti_loading_overlay(panel_rect, selected_tab_loading_t, selected_tab_view_state->loading_progress_v, selected_tab_view_state->loading_progress_v_target);
                        }
                    }
                    
                    /////////////
                    // consume panel fallthrough interaction events
                    //
                    if (build_panel)
                    {
                        UI_Signal panel_sig = ui_signal_from_box(panel_box);
                        if (ui_pressed(panel_sig))
                        {
                            ti_cmd(TI_CmdKind_FocusPanel, .panel = panel->cfg->id);
                        }
                    }
                    
                    ////////////
                    // compute tab build tasks
                    //
                    typedef struct Tab_Task Tab_Task;
                    struct Tab_Task
                    {
                        Tab_Task *next;
                        CFG_Node *tab;
                        DR_FStr_List fstrs;
                        f32 tab_width;
                    };
                    Tab_Task *first_tab_task = 0;
                    Tab_Task *last_tab_task = 0;
                    u64 tab_task_count = 0;
                    f32 tab_close_width_px = ui_top_font_size()*2.5f;
                    f32 max_tab_width_px = ui_top_font_size()*20.f;
                    if (build_panel) UI_TagF("tab")
                    {
                        bool32 reset = (ws->window_layout_reset || ws->frames_alive < 5);// || is_changing_panel_boundaries);
                        for (CFG_Node_Ptr_Node *n = panel->tabs.first; n != 0; n = n->next)
                        {
                            CFG_Node *tab = n->v;
                            if (ti_cfg_is_project_filtered(tab))
                            {
                                continue;
                            }
                            UI_TagF(tab != panel->selected_tab ? "inactive" : "")
                            {
                                Tab_Task *t = push_array(scratch.arena, Tab_Task, 1);
                                t->tab = tab;
                                t->fstrs = ti_title_fstrs_from_cfg(scratch.arena, tab, 0);
                                f32 tab_width_target = dr_dim_from_fstrs(ui_top_tab_size(), &t->fstrs).x + tab_close_width_px + ui_top_font_size()*1.f;
                                bool32 tab_is_selected = (tab == panel->selected_tab);
                                if (tab_is_selected && panel_tree.focused == panel)
                                {
                                    tab_width_target += tab_close_width_px;
                                }
                                tab_width_target = Min(max_tab_width_px, tab_width_target);
                                t->tab_width = floor_f32(ui_anim(ui_key_from_stringf(ui_key_zero(), "tab_width_%p", tab), tab_width_target, .initial = reset  ? tab_width_target : 0, .rate = ti_state->menu_animation_rate));
                                SLLQueuePush(first_tab_task, last_tab_task, t);
                                tab_task_count += 1;
                            }
                        }
                    }
                    
                    ///////////////
                    // build tab bar container
                    //
                    UI_Box *tab_bar_box = &ui_nil_box;
                    if (build_panel) UI_CornerRadius(0) UI_Rect(tab_bar_rect)
                    {
                        tab_bar_box = ui_build_box_from_stringf(UI_BoxFlag_Clip|
                                                                UI_BoxFlag_AllowOverflowY|
                                                                UI_BoxFlag_ViewClampX|
                                                                UI_BoxFlag_ViewScrollX|
                                                                UI_BoxFlag_Clickable,
                                                                "tab_bar_%p", panel->cfg);
                        if (panel->tab_side == Side_Max)
                        {
                            tab_bar_box->view_off.y = tab_bar_box->view_off_target.y = (tab_bar_rheight - tab_bar_vheight);
                        }
                        else
                        {
                            tab_bar_box->view_off.y = tab_bar_box->view_off_target.y = 0;
                        }
                    }
                    
                    //////////////
                    // build tab bar contents
                    if (build_panel) UI_Focus(UI_FocusKind_Off) UI_Parent(tab_bar_box) UI_Padding(ui_em(0.5f, 1.f)) UI_PrefHeight(ui_pct(1, 0)) UI_TagF("tab")
                    {
                        f32 corner_radius = ui_top_font_size()*0.6f;
                        Tab_Task start_boundary_tab_task = {first_tab_task, &cfg_nil_node};
                        UI_CornerRadius00(panel->tab_side == Side_Min ? corner_radius : 0)
                            UI_CornerRadius01(panel->tab_side == Side_Min ? 0 : corner_radius)
                            UI_CornerRadius10(panel->tab_side == Side_Min ? corner_radius : 0)
                            UI_CornerRadius11(panel->tab_side == Side_Min ? 0 : corner_radius)
                            for (Tab_Task *tab_task = &start_boundary_tab_task; tab_task != 0; tab_task = tab_task->next)
                        {
                            CFG_Node *tab = tab_task->tab;
                            
                            // build tab
                            DR_FStr_List tab_fstrs = tab_task->fstrs;
                            f32 tab_width_px = tab_task->tab_width;
                            if (tab != &cfg_nil_node) TI_RegsScope(.panel = panel->cfg->id, .view = tab->id, .tab = tab->id)
                            {
                                // gather info for this tab
                                bool32 tab_is_selected = (tab == panel->selected_tab);
                                
                                // begin vertical region for this tab
                                ui_set_next_child_layout_axis(Axis2_Y);
                                ui_set_next_pref_width(ui_px(tab_width_px, 1));
                                UI_Box *tab_column_box = ui_build_box_from_stringf(UI_BoxFlag_AnimatePosX, "tab_column_%p", tab);
                                
                                // choose palette
                                bool32 omit_name = 0;
                                
                                // build tab container box
                                UI_Parent(tab_column_box)
                                    UI_PrefHeight(ui_px(tab_bar_vheight, 1))
                                    UI_TagF(omit_name ? "hollow" : "")
                                    UI_TagF(!omit_name && !tab_is_selected ? "inactive" : "")
                                    //UI_TagF(!omit_name && tab_is_auto ? "auto" : "")
                                    UI_TagF("")
                                {
                                    if (panel->tab_side == Side_Max)
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
                                    
                                    // build tab contents
                                    if (!omit_name) UI_Parent(tab_box)
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
                                        if (tab_is_selected && panel_tree.focused == panel)
                                        {
                                            UI_PrefWidth(ui_px(tab_close_width_px, 1.f))
                                                UI_TextAlignment(UI_TextAlign_Center)
                                                TI_Font(TI_FontSlot_Icons)
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
                                                                                             "%S###edit_view_%p", ti_icon_kind_text_table[TI_IconKind_Gear], tab);
                                                UI_Signal sig = ui_signal_from_box(edit_box);
                                                if (ui_pressed(sig))
                                                {
                                                    // we dont have queries yet
                                                }
                                            }
                                        }
                                        UI_PrefWidth(ui_px(tab_close_width_px, 1.f))
                                            UI_TextAlignment(UI_TextAlign_Center)
                                            TI_Font(TI_FontSlot_Icons)
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
                                                                                          "%S###close_view_%p", ti_icon_kind_text_table[TI_IconKind_X], tab);
                                            UI_Signal sig = ui_signal_from_box(close_box);
                                            if (ui_clicked(sig) || ui_middle_clicked(sig))
                                            {
                                                ti_cmd(TI_CmdKind_CloseTab);
                                            }
                                        }
                                    }

                                    // consume events for tab clicking
                                    {
                                        UI_Signal sig = ui_signal_from_box(tab_box);
                                        if (ui_pressed(sig))
                                        {
                                            ti_cmd(TI_CmdKind_FocusTab);
                                            ti_cmd(TI_CmdKind_FocusPanel);
                                        }
                                        /*
                                        else if (ui_dragging(sig) && !ti_drag_is_active() && length_2f32(ui_drag_delta()) > 10.f)
                                        {
                                            ti_drag_begin(TI_RegSlot_View);
                                        }
                                        */
                                        else if (ui_right_clicked(sig))
                                        {
                                            // no queries yet...
                                        }
                                        else if (ui_middle_clicked(sig))
                                        {
                                            ti_cmd(TI_CmdKind_CloseTab);
                                        }
                                    }
                                }

                                // space for next tab
                                {
                                    ui_spacer(ui_px(floor_f32(ui_top_font_size()*0.4f), 1.f));
                                }
                            }

                            // if this is the currently active drop site's previous tab, then build empty space
                            // to visualize where tab will be moved once dropped
                        }

                        // build add-new-tab button
                        UI_TextAlignment(UI_TextAlign_Center)
                            UI_PrefWidth(ui_px(tab_bar_vheight, 1.f))
                            UI_PrefHeight(ui_px(tab_bar_vheight, 1.f))
                            UI_TagF(".")
                        {
                            ui_set_next_child_layout_axis(Axis2_Y);
                            UI_Box *container = ui_build_box_from_stringf(UI_BoxFlag_AnimatePosX, "###add_new_tab");
                            UI_Parent(container)
                            {
                                if (panel->tab_side == Side_Max)
                                {
                                    ui_spacer(ui_px(tab_bar_rv_diff-1.f, 1.f));
                                }
                                else
                                {
                                    ui_spacer(ui_px(1.f, 1.f));
                                }
                                {
                                    UI_Box *add_new_box = &ui_nil_box;
                                    TI_Font(TI_FontSlot_Icons)
                                        UI_CornerRadius((tab_bar_vheight - tab_bar_vheight/4.f) / 3.f)
                                        UI_VisualMargin(tab_bar_vheight/5.f)
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
                                                                                ti_icon_kind_text_table[TI_IconKind_Add],
                                                                                panel->cfg);
                                    }
                                    UI_Signal sig = ui_signal_from_box(add_new_box);
                                    if (ui_pressed(sig))
                                    {
                                        ti_cmd(TI_CmdKind_FocusPanel, .panel = panel->cfg->id);
                                        ti_cmd(TI_CmdKind_RunCommand, .cmd_name = ti_cmd_kind_info_table[TI_CmdKind_OpenTab].string);
                                    }
                                    if (ui_hovering(sig)) UI_Tooltip
                                    {
                                        ui_state->tooltip_anchor_key = add_new_box->key;
                                        ui_set_next_pref_width(ui_children_sum(1));
                                        UI_Row
                                        {
                                            ui_labelf("Open New Tab");
                                            //ti_cmd_binding_buttons(ti_cmd_kind_info_table[TI_CmdKind_OpenTab].string, s(""), 1, TI_CmdBindingButtonFlag_NoEdit);
                                        }
                                    }
                                }
                            }
                        }

                        // interact with tab bar
                        ui_signal_from_box(tab_bar_box);
                    }

                    ////////////////
                    // accept tab drops
                    //
                    
                    /////////////
                    // accept file drops
                    //
                }
            }
        }

        ////////////////////////
        // @window_ui_part drag/drop cancelling
        //

        ////////////////////
        // @window_ui_part top-level font size changing
        //
        for (UI_Event *evt = 0; ui_next_event(&evt);)
        {
            if (evt->kind == UI_EventKind_Scroll && evt->modifiers == WM_Modifier_Ctrl)
            {
                ui_eat_event(evt);
                if (evt->delta_2f32.y < 0)
                {
                    ti_cmd(TI_CmdKind_IncWindowFontSize);
                }
                else if (evt->delta_2f32.y > 0)
                {
                    ti_cmd(TI_CmdKind_DecWindowFontSize);
                }
            }
        }
            
        ui_end_build();
    }

    //////////////////////
    // @window_frame_part animate
    //
    if (ui_animating_from_state(ws->ui))
    {
        ti_request_frame();
    }

    ///////////////////////////////
    // @window_frame_part draw UI
    //
    ws->draw_bucket = dr_bucket_make();
    DR_BucketScope(ws->draw_bucket)
        ProfScope("draw UI")
    {
        Temp scratch = scratch_begin(0, 0);
        f32 box_squish_epsilon = 0.001f;
        Rng2f32 window_rect = wm_client_rect_from_window(ws->os);

        // unpack settings
        f32 rounded_corner_amount = 0.5f;
        f32 border_softness       = 1.f;
        bool32 do_background_blur                = true;
        bool32 force_opaque_floating_backgrounds = true;
        bool32 do_drop_shadows                   = true;
        Vec4f32 base_background_color = ui_color_from_name(str8_lit("background"));
        Vec4f32 base_border_color     = ui_color_from_name(str8_lit("border"));
        Vec4f32 drop_shadow_color     = ui_color_from_name(str8_lit("drop_shadow"));

        // set up heatmap buckets
        f32 heatmap_bucket_size = 32.f;
        u64 *heatmap_buckets = 0;
        u64 heatmap_bucket_pitch = 0;
        u64 heatmap_bucket_count = 0;
        if (DEV_draw_ui_box_heatmap)
        {
            Rng2f32 rect = wm_client_rect_from_window(ws->os);
            Vec2f32 size = dim_2f32(rect);
            Vec2s32 buckets_dim = {(s32)(size.x/heatmap_bucket_size), (s32)(size.y/heatmap_bucket_size)};
            heatmap_bucket_pitch = buckets_dim.x;
            heatmap_bucket_count = buckets_dim.x * buckets_dim.y;
            heatmap_buckets = push_array(scratch.arena, u64, heatmap_bucket_count);
        }

        // draw background color
        {
            dr_rect(wm_client_rect_from_window(ws->os), base_background_color, 0, 0, 0);
        }

        // draw window border
        {
            dr_rect(wm_client_rect_from_window(ws->os), base_border_color, 0, 1.f, border_softness * 0.5f);
        }

        // recurse & draw
        u64 total_heatmap_sum_count = 0;
        UI_Box *hover_debug_box = &ui_nil_box;
        for (UI_Box *box = ui_root_from_state(ws->ui); !ui_box_is_nil(box);)
        {
            // get corner radii
            f32 box_corner_radii[Corner_COUNT] = {
                box->corner_radii[Corner_00] * rounded_corner_amount,
                box->corner_radii[Corner_01] * rounded_corner_amount,
                box->corner_radii[Corner_10] * rounded_corner_amount,
                box->corner_radii[Corner_11] * rounded_corner_amount,
            };

            // get recursion
            UI_Box_Rec rec = ui_box_rec_df_post(box, &ui_nil_box);

            // sum to box heatmap
            if (DEV_draw_ui_box_heatmap)
            {
                Vec2f32 center = center_2f32(box->rect);
                Vec2s32 p = v2s32(center.x / heatmap_bucket_size, center.y / heatmap_bucket_size);
                u64 bucket_idx = p.y * heatmap_bucket_pitch + p.x;
                if (bucket_idx < heatmap_bucket_count)
                {
                    heatmap_buckets[bucket_idx] += 1;
                    total_heatmap_sum_count += 1;
                }
            }

            // grab if debug
            if (box->flags & UI_BoxFlag_Debug && contains_2f32(box->rect, ui_mouse()))
            {
                hover_debug_box = box;
            }
            
            // push transparency
            if (box->transparency != 0)
            {
                dr_push_transparency(box->transparency);
            }

            // push squish
            if (box->squish > box_squish_epsilon)
            {
                Vec2f32 box_dim = dim_2f32(box->rect);
                Vec2f32 anchor_off = {0};
                if (box->flags & UI_BoxFlag_SquishAnchored)
                {
                    anchor_off.x = box_dim.x/2.f;
                }
                else
                {
                    anchor_off.y = -box_dim.y/8.f;
                }
                Mat3x3f32 box2origin_xform = make_translate_3x3f32(v2f32(-box->rect.x0 - box_dim.x/2 + anchor_off.x,
                                                                         -box->rect.y0               + anchor_off.y));
                Mat3x3f32 scale_xform = make_scale_3x3f32(v2f32(1 - box->squish, 1 - box->squish));
                Mat3x3f32 origin2box_xform = make_translate_3x3f32(v2f32(box->rect.x0 + box_dim.x/2 - anchor_off.x,
                                                                         box->rect.y0               - anchor_off.y));
                Mat3x3f32 xform = mul_3x3f32(origin2box_xform, mul_3x3f32(scale_xform, box2origin_xform));
                dr_push_xform2d(xform);
                dr_push_tex2d_sample_kind(R_Tex2DSampleKind_Linear);
            }

            // draw drop shadow
            if (do_drop_shadows && box->flags & UI_BoxFlag_DrawDropShadow)
            {
                Rng2f32 drop_shadow_rect = shift_2f32(pad_2f32(box->rect, 8), v2f32(4, 4));
                R_Rect2D_Inst *inst = dr_rect(drop_shadow_rect, drop_shadow_color, 0.8f, 0, 8.f);
                MemoryCopyArray(inst->corner_radii, box_corner_radii);
            }

            // blur background
            if (do_background_blur && box->flags & UI_BoxFlag_DrawBackgroundBlur)
            {
                R_Pass_Params_Blur *params = dr_blur(pad_2f32(box->rect, 1.f),
                                                     box->blur_size * (1 - box->transparency),
                                                     0);
                MemoryCopyArray(params->corner_radii, box_corner_radii);
            }

            // compute effective active t
            f32 effective_active_t = box->active_t;
            if (!(box->flags & UI_BoxFlag_DrawActiveEffects))
            {
                effective_active_t = 0;
            }
            f32 t = box->hot_t * (1 - effective_active_t);

            // compute background color
            Vec4f32 box_background_color = box->background_color;
            if (force_opaque_floating_backgrounds &&
                box->flags & UI_BoxFlag_Floating && box->flags & UI_BoxFlag_DrawDropShadow)
            {
                box_background_color.w = 1.f;
            }

            // draw background
            if (box->flags & UI_BoxFlag_DrawBackground)
            {
                Rng2f32 box_bg_rect = r2f32p(box->rect.x0 + box->visual_margin.x,
                                             box->rect.y0 + box->visual_margin.y,
                                             box->rect.x1 - box->visual_margin.x,
                                             box->rect.y1 - box->visual_margin.y);

                // hot effect extension (drop shadow)
                if (box->flags & UI_BoxFlag_DrawHotEffects)
                {
                    Rng2f32 drop_shadow_rect = shift_2f32(pad_2f32(box_bg_rect, 8), v2f32(4, 4));
                    Vec4f32 color = drop_shadow_color;
                    color.w *=t * box_background_color.w;
                    dr_rect(drop_shadow_rect, color, 0.8f, 0, 8.f);
                }

                // draw background
                R_Rect2D_Inst *inst = dr_rect(pad_2f32(box_bg_rect, 1.f),
                                              box_background_color, 0, 0, border_softness * 1.f);
                MemoryCopyArray(inst->corner_radii, box_corner_radii);

                // hot effect extension
                if (box->flags & UI_BoxFlag_DrawHotEffects)
                {
                    bool32 is_hot = !ui_key_match(box->key, ui_key_zero()) && ui_key_match(box->key, ui_hot_key());
                    Vec4f32 hover_color = ui_color_from_tags_key_name(box->tags_key, str8_lit("hover"));

                    // brighten
                    if (is_hot)
                    {
                        Vec4f32 color = hover_color;
                        color.w *= 0.015f;
                        R_Rect2D_Inst *inst = dr_rect(pad_2f32(box_bg_rect, 1.f),
                                                      v4f32(0, 0, 0, 0), 0, 0, border_softness * 1.f);
                        inst->colors[Corner_00] = color;
                        inst->colors[Corner_10] = color;
                        inst->colors[Corner_01] = color;
                        inst->colors[Corner_11] = color;
                        MemoryCopyArray(inst->corner_radii, box_corner_radii);
                    }

                    // soft circle around mouse
                    if (box->hot_t > 0.01f && dim_2f32(box->rect).x > box->font_size * 8.f) DR_ClipScope(intersect_2f32(box_bg_rect, dr_top_clip()))
                    {
                        Vec4f32 color = hover_color;
                        color.w *= 0.025f;
                        if (!is_hot)
                        {
                            color.w *= t;
                        }
                        Vec2f32 center = ui_mouse();
                        Vec2f32 box_dim = dim_2f32(box->rect);
                        f32 max_dim = Max(box_dim.x, box_dim.y);
                        f32 radius = box->font_size * 24.f;
                        radius = Min(max_dim, radius);
                        dr_rect(pad_2f32(r2f32(center, center), radius * 2.f), color, radius, 0, radius / 2.f);
                    }
                }

                // active effect extension
                if (box->flags & UI_BoxFlag_DrawActiveEffects)
                {
                    Vec4f32 shadow_color = drop_shadow_color;
                    shadow_color.w *= 0.5 * box->active_t;
                    Vec2f32 shadow_size = {
                        (box_bg_rect.x1 - box_bg_rect.x0) * 0.60f * box->active_t,
                        (box_bg_rect.y1 - box_bg_rect.y0) * 0.60f * box->active_t,
                    };
                    shadow_size.x = Clamp(0, shadow_size.x, box->font_size * 2.f);
                    shadow_size.y = Clamp(0, shadow_size.y, box->font_size * 2.f);

                    // top -> bottom dark effect
                    {
                        R_Rect2D_Inst *inst = dr_rect(r2f32p(box_bg_rect.x0,
                                                             box_bg_rect.y0,
                                                             box_bg_rect.x1,
                                                             box_bg_rect.y0 + shadow_size.y),
                                                      v4f32(0, 0, 0, 0),
                                                      0, 0, 1.f);
                        inst->colors[Corner_00] = inst->colors[Corner_10] = shadow_color;
                        inst->colors[Corner_01] = inst->colors[Corner_11] = v4f32(0.f, 0.f, 0.f, 0.0f);
                        MemoryCopyArray(inst->corner_radii, box_corner_radii);
                    }
                    
                    // rjf: bottom -> top light effect
                    {
                        R_Rect2D_Inst *inst = dr_rect(r2f32p(box_bg_rect.x0,
                                                             box_bg_rect.y1 - shadow_size.y,
                                                             box_bg_rect.x1,
                                                             box_bg_rect.y1),
                                                      v4f32(0, 0, 0, 0),
                                                      0, 0, 1.f);
                        inst->colors[Corner_00] = inst->colors[Corner_10] = v4f32(0, 0, 0, 0);
                        inst->colors[Corner_01] = inst->colors[Corner_11] = v4f32(1.0f, 1.0f, 1.0f, 0.08f*box->active_t);
                        MemoryCopyArray(inst->corner_radii, box_corner_radii);
                    }
          
                    // rjf: left -> right dark effect
                    {
                        R_Rect2D_Inst *inst = dr_rect(r2f32p(box_bg_rect.x0,
                                                             box_bg_rect.y0,
                                                             box_bg_rect.x0 + shadow_size.x,
                                                             box_bg_rect.y1),
                                                      v4f32(0, 0, 0, 0),
                                                      0, 0, 1.f);
                        inst->colors[Corner_10] = inst->colors[Corner_11] = v4f32(0.f, 0.f, 0.f, 0.f);
                        inst->colors[Corner_00] = shadow_color;
                        inst->colors[Corner_01] = shadow_color;
                        MemoryCopyArray(inst->corner_radii, box_corner_radii);
                    }
                    
                    // rjf: right -> left dark effect
                    {
                        R_Rect2D_Inst *inst = dr_rect(r2f32p(box_bg_rect.x1 - shadow_size.x,
                                                             box_bg_rect.y0,
                                                             box_bg_rect.x1,
                                                             box_bg_rect.y1),
                                                      v4f32(0, 0, 0, 0),
                                                      0, 0, 1.f);
                        inst->colors[Corner_00] = inst->colors[Corner_01] = v4f32(0.f, 0.f, 0.f, 0.f);
                        inst->colors[Corner_10] = shadow_color;
                        inst->colors[Corner_11] = shadow_color;
                        MemoryCopyArray(inst->corner_radii, box_corner_radii);
                    }
                }
            }

            // draw string
            if(box->flags & UI_BoxFlag_DrawText)
            {
                Vec2f32 text_position = ui_box_text_position(box);
                if(DEV_draw_ui_text_pos)
                {
                    dr_rect(r2f32p(text_position.x-4,
                                   text_position.y-4,
                                   text_position.x+4,
                                   text_position.y+4),
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
                    ellipses_run = fnt_run_from_string(ellipses_font,
                                                       ellipses_size,
                                                       0,
                                                       box->tab_size,
                                                       ellipses_raster_flags,
                                                       str8_lit("..."));
                }
                if(box->flags & UI_BoxFlag_HasFuzzyMatchRanges) UI_TagF("match")
                {
                    Vec4f32 match_color = ui_color_from_tags_key_name(ui_top_tags_key(), str8_lit("background"));
                    dr_truncated_fancy_run_fuzzy_matches(text_position,
                                                         &box->display_fruns,
                                                         max_x,
                                                         &box->fuzzy_match_ranges,
                                                         match_color);
                }
                dr_truncated_fancy_run_list(text_position, &box->display_fruns, max_x, ellipses_run);
            }

            // draw focus viz
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
                    dr_rect(r2f32p(box->rect.x0-6,
                                   box->rect.y0-6,
                                   box->rect.x0+6,
                                   box->rect.y0+6),
                            color, 2, 0, 1);
                    dr_rect(box->rect, color, 2, 2, 1);
                }
                if(box->flags & (UI_BoxFlag_FocusHot|UI_BoxFlag_FocusActive))
                {
                    if(box->flags & (UI_BoxFlag_FocusHotDisabled|UI_BoxFlag_FocusActiveDisabled))
                    {
                        dr_rect(r2f32p(box->rect.x0-6,
                                       box->rect.y0-6,
                                       box->rect.x0+6,
                                       box->rect.y0+6),
                                v4f32(1, 0, 0, 0.2f), 2, 0, 1);
                    }
                    else
                    {
                        dr_rect(r2f32p(box->rect.x0-6,
                                       box->rect.y0-6,
                                       box->rect.x0+6,
                                       box->rect.y0+6),
                                v4f32(0, 1, 0, 0.2f),
                                2, 0, 1);
                    }
                }
            }

            // rjf: push clip
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
            
            // rjf: custom draw list
            if(box->flags & UI_BoxFlag_DrawBucket)
            {
                Mat3x3f32 xform = make_translate_3x3f32(box->position_delta);
                DR_XForm2DScope(xform)
                {
                    dr_sub_bucket(box->draw_bucket);
                }
            }
            
            // rjf: call custom draw callback
            if(box->custom_draw != 0)
            {
                box->custom_draw(box, box->custom_draw_user_data);
            }
            
            // rjf: pop
            {
                s32 pop_idx = 0;
                for(UI_Box *b = box; !ui_box_is_nil(b) && pop_idx <= rec.pop_count; b = b->parent)
                {
                    pop_idx += 1;
                    if(b == box && rec.push_count != 0)
                    {
                        continue;
                    }
                    
                    // rjf: pop clips
                    if(b->flags & UI_BoxFlag_Clip)
                    {
                        dr_pop_clip();
                    }
                    
                    // rjf: get corner radii
                    f32 b_corner_radii[Corner_COUNT] =
                        {
                            b->corner_radii[Corner_00]*rounded_corner_amount,
                            b->corner_radii[Corner_01]*rounded_corner_amount,
                            b->corner_radii[Corner_10]*rounded_corner_amount,
                            b->corner_radii[Corner_11]*rounded_corner_amount,
                        };
                    
                    // rjf: draw border
                    if(b->flags & UI_BoxFlag_DrawBorder)
                    {
                        Vec4f32 border_color = b->border_color;
                        Rng2f32 b_border_rect = r2f32p(b->rect.x0 - 1.f + b->visual_margin.x,
                                                       b->rect.y0 - 1.f + b->visual_margin.y,
                                                       b->rect.x1 + 1.f - b->visual_margin.x,
                                                       b->rect.y1 + 1.f - b->visual_margin.y);
                        R_Rect2D_Inst *inst = dr_rect(b_border_rect, border_color, 0, 1.f, border_softness*1.f);
                        MemoryCopyArray(inst->corner_radii, b_corner_radii);
                        
                        // rjf: hover effect
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
                    
                    // rjf: draw scroll fade
                    if(b->flags & (UI_BoxFlag_DrawFadeTop|UI_BoxFlag_DrawFadeBottom|UI_BoxFlag_DrawFadeLeft|UI_BoxFlag_DrawFadeRight))
                    {
                        Vec2f32 fade_dim = scale_2f32(dim_2f32(b->rect), 0.05f);
                        Vec4f32 fade_color = drop_shadow_color;
                        if(b->flags & UI_BoxFlag_DrawFadeTop)
                        {
                            f32 t = ui_anim(ui_key_from_string(b->key, s("fade_top")), 1.f, .rate = ti_state->catchall_animation_rate);
                            Rng2f32 rect = r2f32p(b->rect.x0, b->rect.y0, b->rect.x1, b->rect.y0 + fade_dim.y*t);
                            R_Rect2D_Inst *r = dr_rect(rect, fade_color, 0, 0, 0);
                            MemoryCopyArray(r->corner_radii, b_corner_radii);
                            r->colors[Corner_01] = r->colors[Corner_11] = v4f32(0, 0, 0, 0);
                        }
                        if(b->flags & UI_BoxFlag_DrawFadeBottom)
                        {
                            f32 t = ui_anim(ui_key_from_string(b->key, s("fade_bottom")), 1.f, .rate = ti_state->catchall_animation_rate);
                            Rng2f32 rect = r2f32p(b->rect.x0, b->rect.y1 - fade_dim.y*t, b->rect.x1, b->rect.y1);
                            R_Rect2D_Inst *r = dr_rect(rect, fade_color, 0, 0, 0);
                            MemoryCopyArray(r->corner_radii, b_corner_radii);
                            r->colors[Corner_00] = r->colors[Corner_10] = v4f32(0, 0, 0, 0);
                        }
                        if(b->flags & UI_BoxFlag_DrawFadeLeft)
                        {
                            f32 t = ui_anim(ui_key_from_string(b->key, s("fade_left")), 1.f, .rate = ti_state->catchall_animation_rate);
                            Rng2f32 rect = r2f32p(b->rect.x0, b->rect.y0, b->rect.x1+fade_dim.x*t, b->rect.y1);
                            R_Rect2D_Inst *r = dr_rect(rect, fade_color, 0, 0, 0);
                            MemoryCopyArray(r->corner_radii, b_corner_radii);
                            r->colors[Corner_11] = r->colors[Corner_10] = v4f32(0, 0, 0, 0);
                        }
                        if(b->flags & UI_BoxFlag_DrawFadeRight)
                        {
                            f32 t = ui_anim(ui_key_from_string(b->key, s("fade_right")), 1.f, .rate = ti_state->catchall_animation_rate);
                            Rng2f32 rect = r2f32p(b->rect.x1 - fade_dim.x*t, b->rect.y0, b->rect.x1, b->rect.y1);
                            R_Rect2D_Inst *r = dr_rect(rect, fade_color, 0, 0, 0);
                            MemoryCopyArray(r->corner_radii, b_corner_radii);
                            r->colors[Corner_00] = r->colors[Corner_01] = v4f32(0, 0, 0, 0);
                        }
                    }
                    
                    // rjf: debug border rendering
                    if(b->flags & UI_BoxFlag_Debug)
                    {
                        R_Rect2D_Inst *inst = dr_rect(b->rect, v4f32(1*box->pref_size[Axis2_X].strictness, 0, 1, 0.25f), 0, 1.f, 0);
                        MemoryCopyArray(inst->corner_radii, b_corner_radii);
                    }
                    
                    // rjf: draw sides
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
                    
                    // rjf: draw focus overlay
                    if(b->flags & UI_BoxFlag_Clickable && !(b->flags & UI_BoxFlag_DisableFocusOverlay) && b->focus_hot_t > 0.01f)
                    {
                        String8 extras[] = {str8_lit("focus"), str8_lit("overlay")};
                        String8_Array extras_array = {extras, ArrayCount(extras)};
                        Vec4f32 color = ui_color_from_tags_key_extras(b->tags_key, extras_array);
                        color.w *= b->focus_hot_t;
                        R_Rect2D_Inst *inst = dr_rect(b->rect, color, 0, 0, 0.f);
                        MemoryCopyArray(inst->corner_radii, b_corner_radii);
                    }
                    
                    // rjf: draw focus border
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
                    
                    // rjf: disabled overlay
                    if(b->disabled_t >= 0.005f)
                    {
                        Vec4f32 disabled_overlay_color = v4f32(base_background_color.x, base_background_color.y, base_background_color.z, b->disabled_t*0.3f);
                        R_Rect2D_Inst *inst = dr_rect(b->rect, disabled_overlay_color, 0, 0, 1);
                        MemoryCopyArray(inst->corner_radii, b_corner_radii);
                    }
                    
                    // rjf: pop squish
                    if(b->squish > box_squish_epsilon)
                    {
                        dr_pop_xform2d();
                        dr_pop_tex2d_sample_kind();
                    }
                    
                    // rjf: pop transparency
                    if(b->transparency != 0)
                    {
                        dr_pop_transparency();
                    }
                }
            }

            // next
            box = rec.next;
        }

        //- rjf: draw heatmap
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
                Rng2f32 rect = r2f32p(x*heatmap_bucket_size,
                                      y*heatmap_bucket_size,
                                      (x+1)*heatmap_bucket_size,
                                      (y+1)*heatmap_bucket_size);
                dr_rect(rect, v4f32(rgb.x, rgb.y, rgb.z, 0.3f), 0, 0, 0);
            }
        }

        // draw hover debug box
        if (hover_debug_box != &ui_nil_box)
        {
            FNT_Tag font = fnt_tag_from_static_data_string(&ti_default_main_font_bytes);
            Vec2f32 p = ui_mouse();
            dr_rect(hover_debug_box->rect, v4f32(1, 1, 1, 0.2f), 0, 0, 0);
            R_Rect2D_Inst *border = dr_rect(hover_debug_box->rect, v4f32(1, 0, 0, 1.f), 0, 0, 0);
            MemoryCopyArray(border->corner_radii, hover_debug_box->corner_radii);
            dr_text(font, 12.f, 0, 0, FNT_RasterFlag_Hinted, p, v4f32(1, 1, 1, 1), push_str8f(scratch.arena, "key: 0x%I64x", hover_debug_box->key.u64[0]));
            p.y += 20.f;
            dr_text(font, 12.f, 0, 0, FNT_RasterFlag_Hinted, p, v4f32(1, 1, 1, 1), push_str8f(scratch.arena, "string: '%S'", hover_debug_box->string));
            p.y += 20.f;
        }

        //- rjf: draw border/overlay color to signify error
        if(ws->error_t > 0.01f) UI_TagF("bad")
        {
            Vec4f32 color = ui_color_from_name(str8_lit("text"));
            color.w *= ws->error_t;
            Rng2f32 rect = wm_client_rect_from_window(ws->os);
            dr_rect(pad_2f32(rect, 24.f), color, 0, 16.f, 12.f);
            dr_rect(rect, v4f32(color.x, color.y, color.z, color.w*0.025f), 0, 0, 0);
        }

        scratch_end(scratch);
    }

    /////////////////////
    // @window_frame_part update per-window frame counters/info
    //
    ws->frames_alive += 1;
    ws->last_window_rect = wm_client_rect_from_window(ws->os);

    ProfEnd();
    scratch_end(scratch);
}

#if COMPILER_MSVC && !BUILD_DEBUG
NO_OPTIMIZE_END
#endif

//////////////////////////
// Colors, Fonts, Config

// colors

// fonts
internal f32 ti_font_size(void)
{
    // TODO make this real
    return 10.f;
}

internal FNT_Tag ti_font_from_slot(TI_FontSlot slot)
{
    FNT_Tag tag = ti_state->font_slot_table[slot];
    return tag;
}

internal FNT_RasterFlags ti_raster_flags_from_slot(TI_FontSlot slot)
{
    CFG_Node *window = cfg_node_from_id(ti_regs()->window);
    TI_Window_State *ws = ti_window_state_from_cfg(window);
    FNT_RasterFlags flags = ws->font_slot_raster_flags[slot];
    return flags;
}

////////////////////
// Vocab Info Lookups

internal TI_Vocab_Info *ti_vocab_info_from_code_name(String8 code_name)
{
    TI_Vocab_Info *result = &ti_nil_vocab_info;
    if (code_name.size != 0)
    {
        u64 hash = u64_hash_from_str8(code_name);
        u64 slot_idx = hash%ti_state->vocab_info_map.single_slots_count;
        for (TI_Vocab_Info_Map_Node *n = ti_state->vocab_info_map.single_slots[slot_idx].first;
             n != 0;
             n = n->single_next)
        {
            if (str8_match(n->v.code_name, code_name, 0))
            {
                result = &n->v;
                break;
            }
        }
    }
    return result;
}

internal TI_Vocab_Info *ti_vocab_info_from_code_name_plural(String8 code_name_plural)
{
    TI_Vocab_Info *result = &ti_nil_vocab_info;
    if (code_name_plural.size != 0)
    {
        u64 hash = u64_hash_from_str8(code_name_plural);
        u64 slot_idx = hash%ti_state->vocab_info_map.plural_slots_count;
        for (TI_Vocab_Info_Map_Node *n = ti_state->vocab_info_map.plural_slots[slot_idx].first;
             n != 0;
             n = n->plural_next)
        {
            if (str8_match(n->v.code_name_plural, code_name_plural, 0))
            {
                result = &n->v;
                break;
            }
        }
    }
    return result;
}

//////////////////////////////
// Continuous Frame Requests

internal void ti_request_frame(void)
{
    ti_state->num_frames_requested = 4;
}

////////////////////////////
// Main State Accessors

// per-frame arena
internal Arena *ti_frame_arena(void)
{
    return ti_state->frame_arenas[ti_state->frame_index % ArrayCount(ti_state->frame_arenas)];
}

///////////////
// Registers

internal TI_Regs *ti_push_regs_(TI_Regs *regs)
{
    TI_Regs_Node *n = push_array(ti_frame_arena(), TI_Regs_Node, 1);
    ti_regs_copy_contents(ti_frame_arena(), &n->v, regs);
    SLLStackPush(ti_state->top_regs, n);
    return &n->v;
}

internal TI_Regs *ti_pop_regs(void)
{
    TI_Regs *regs = &ti_state->top_regs->v;
    SLLStackPop(ti_state->top_regs);
    if (ti_state->top_regs == 0)
    {
        ti_state->top_regs = &ti_state->base_regs;
    }
    return regs;
}

/////////////////////
// Commands

// name -> info
internal TI_CmdKind ti_cmd_kind_from_string(String8 string)
{
    TI_CmdKind result = TI_CmdKind_Null;
    for (u64 idx = 0; idx < ArrayCount(ti_cmd_kind_info_table); idx += 1)
    {
        if (str8_match(string, ti_cmd_kind_info_table[idx].string, 0))
        {
            result = (TI_CmdKind)idx;
            break;
        }
    }
    return result;
}

internal TI_Cmd_Kind_Info *ti_cmd_kind_info_from_string(String8 string)
{
    TI_Cmd_Kind_Info *info = &ti_nil_cmd_kind_info;
    {
        TI_CmdKind kind = ti_cmd_kind_from_string(string);
        if (kind != TI_CmdKind_Null)
        {
            info = &ti_cmd_kind_info_table[kind];
        }
    }
    return info;
}

// pushing

internal void ti_push_cmd(String8 name, TI_Regs *regs)
{
    ti_cmd_list_push_new(ti_state->cmds_arenas[0], &ti_state->cmds[0], name, regs);
}

// iterating

internal bool32 ti_next_cmd(TI_Cmd **cmd)
{
    u64 slot = ti_state->cmds_gen % ArrayCount(ti_state->cmds);
    TI_Cmd_Node *start_node = ti_state->cmds[slot].first;
    if (cmd[0] != 0)
    {
        start_node = CastFromMember(TI_Cmd_Node, cmd, cmd[0]);
        start_node = start_node->next;
    }
    cmd[0] = 0;
    if (start_node != 0)
    {
        cmd[0] = &start_node->cmd;
    }
    return !!cmd[0];
}

internal bool32 ti_next_view_cmd(TI_Cmd **cmd)
{
    for (;ti_next_cmd(cmd);)
    {
        if (ti_regs()->view == cmd[0]->regs->view)
        {
            break;
        }
    }
    bool32 result = !!cmd[0];
    return result;
}

//////////////////////////////
// Main Layer Top-Level Calls

#if !defined(STBI_INCLUDE_STB_IMAGE_H)
# define STB_IMAGE_IMPLEMENTATION
# define STBI_ONLY_PNG
# define STBI_ONLY_BMP
# include "third_party/stb/stb_image.h"
#endif

internal void ti_init(Cmd_Line *cmdline)
{
    Temp scratch = scratch_begin(0, 0);
    ProfBeginFunction();
    Arena *arena = arena_alloc();
    ti_state = push_array(arena, TI_State, 1);
    ti_state->arena = arena;
    ti_state->user_path_arena = arena_alloc();
    ti_state->project_path_arena = arena_alloc();
    ti_state->theme_path_arena = arena_alloc();
    for (u64 idx = 0; idx < ArrayCount(ti_state->frame_arenas); idx += 1)
    {
        ti_state->frame_arenas[idx] = arena_alloc();
    }
    ti_state->log = log_alloc();
    log_select(ti_state->log);
    {
        Temp scratch = scratch_begin(0, 0);
        ti_state->log_path = push_str8f(ti_state->arena, "%S/ui_thread.tide_log", g_logs_folder);
        write_data_to_file_path(ti_state->log_path, str8_zero());
        scratch_end(scratch);
    }
    ti_state->num_frames_requested = 2;
    for (u64 idx = 0; idx < ArrayCount(ti_state->cmds_arenas); idx += 1)
    {
        ti_state->cmds_arenas[idx] = arena_alloc();
    }
    ti_state->cmd_output_arena = arena_alloc();
    ti_state->top_regs = &ti_state->base_regs;

    // set up schemas
    {
        ti_state->cfg_schema_table = push_array(ti_state->arena, CFG_Schema_Table, 1);
        ti_state->cfg_schema_table->slots_count = 4096;
        ti_state->cfg_schema_table->slots = push_array(ti_state->arena, CFG_Schema_Node *, ti_state->cfg_schema_table->slots_count);
        for EachElement(idx, ti_name_schema_info_table)
        {
            MD_Node *schema = md_tree_from_string(ti_state->arena, ti_name_schema_info_table[idx].schema)->first;
            cfg_schema_table_insert(ti_state->arena, ti_state->cfg_schema_table, ti_name_schema_info_table[idx].name, schema);
        }
    }

    // set up theme presets
    {
        for EachEnumVal(TI_ThemePreset, p)
        {
            ti_state->theme_preset_trees[p] = md_tree_from_string(ti_state->arena, ti_theme_preset_cfg_string_table[p])->first;
        }
    }

    // set up vocab info map
    {
        ti_state->vocab_info_map.single_slots_count = 1024;
        ti_state->vocab_info_map.single_slots = push_array(ti_state->arena, TI_Vocab_Info_Map_Slot, ti_state->vocab_info_map.single_slots_count);
        ti_state->vocab_info_map.plural_slots_count = 1024;
        ti_state->vocab_info_map.plural_slots = push_array(ti_state->arena, TI_Vocab_Info_Map_Slot, ti_state->vocab_info_map.plural_slots_count);
        for EachElement(idx, ti_vocab_info_table)
        {
            TI_Vocab_Info_Map_Node *n = push_array(ti_state->arena, TI_Vocab_Info_Map_Node, 1);
            MemoryCopyStruct(&n->v, &ti_vocab_info_table[idx]);
            u64 single_hash = u64_hash_from_str8(n->v.code_name);
            u64 plural_hash = u64_hash_from_str8(n->v.code_name_plural);
            u64 single_slot_idx = single_hash%ti_state->vocab_info_map.single_slots_count;
            u64 plural_slot_idx = plural_hash%ti_state->vocab_info_map.plural_slots_count;
            if (n->v.code_name.size != 0)
            {
                SLLQueuePush_N(ti_state->vocab_info_map.single_slots[single_slot_idx].first,
                               ti_state->vocab_info_map.single_slots[single_slot_idx].last,
                               n, single_next);
            }
            if (n->v.code_name_plural.size != 0)
            {
                SLLQueuePush_N(ti_state->vocab_info_map.plural_slots[plural_slot_idx].first,
                               ti_state->vocab_info_map.plural_slots[plural_slot_idx].last,
                               n, plural_next);
            }
        }
    }

    // set up top-level config entity trees & tables
    {
        ti_state->cfg = cfg_state_alloc();
        cfg_ctx_select(cfg_state_ctx(ti_state->cfg));
        cfg_node_new(ti_state->cfg, cfg_node_root(), str8_lit("user"));
        cfg_node_new(ti_state->cfg, cfg_node_root(), str8_lit("project"));
        cfg_node_new(ti_state->cfg, cfg_node_root(), str8_lit("command_line"));
        cfg_node_new(ti_state->cfg, cfg_node_root(), str8_lit("transient"));
    }

    // set up window cache
    {
        ti_state->window_state_slots_count = 64;
        ti_state->window_state_slots = push_array(arena, TI_Window_State_Slot, ti_state->window_state_slots_count);
        ti_state->first_window_state = ti_state->last_window_state = &ti_nil_window_state;
    }

    // set up view cache
    {
        ti_state->view_state_slots_count = 4096;
        ti_state->view_state_slots = push_array(arena, TI_View_State_Slot, ti_state->view_state_slots_count);
    }

    // setup initial target from command line args
    String8 implicit_user_arg = {0};
    String8 implicit_project_arg = {0};
    String8 implicit_dmp_arg = {0};
    {
    }

    // set up user / project paths
    {
        Temp scratch2 = scratch_begin(&scratch.arena, 1);

        // unpack command line arguments
        String8 user_path = cmd_line_string(cmdline, str8_lit("user"));
        String8 project_path = cmd_line_string(cmdline, str8_lit("project"));
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
            String8 user_data_folder = str8f(scratch2.arena, "%S/raddbg", user_program_config_data_path);
            make_directory(user_data_folder);
            if (user_path.size == 0)
            {
                user_path = implicit_user_arg;
            }
            if(user_path.size == 0)
            {
                String8 last_user_path = str8f(scratch2.arena, "%S/last_user", user_data_folder);
                user_path = data_from_file_path(scratch2.arena, last_user_path);
            }
            if(user_path.size == 0)
            {
                user_path = str8f(scratch2.arena, "%S/default.tide_user", user_data_folder);
            }
        }
        if(project_path.size == 0)
        {
            project_path = implicit_project_arg;
        }
        if(project_path.size != 0)
        {
            arena_clear(ti_state->project_path_arena);
            ti_state->project_path = push_str8_copy(ti_state->project_path_arena, project_path);
        }

        // do initial load of user/project
        ti_cmd(TI_CmdKind_OpenUser, .file_path = user_path, .non_graphical = 1);
        if (project_path.size != 0)
        {
            ti_cmd(TI_CmdKind_OpenProject, .file_path = project_path);
        }
        scratch_end(scratch2);
    }

    // unpack icon image data
    {
        Temp scratch = scratch_begin(0, 0);
        String8 data = ti_icon_file_bytes;
        u8 *ptr = data.str;
        u8 *opl = ptr+data.size;

        // read header
#pragma pack(push, 1)
        typedef struct ICO_Header ICO_Header;
        struct ICO_Header
        {
            u16 reserved_padding; // must be 0
            u16 image_type; // if 1 -> ICO, if 2 -> CUR
            u16 num_images;
        };
        typedef struct ICO_Entry ICO_Entry;
        struct ICO_Entry
        {
            u8 image_width_px;
            u8 image_height_px;
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
        if (ptr+sizeof(hdr) < opl)
        {
            MemoryCopy(&hdr, ptr, sizeof(hdr));
            ptr += sizeof(hdr);
        }

        // read image entries
        u64 entries_count = hdr.num_images;
        ICO_Entry *entries = push_array(scratch.arena, ICO_Entry, hdr.num_images);
        {
            u64 bytes_to_read = sizeof(ICO_Entry)*entries_count;
            bytes_to_read = Min(bytes_to_read, opl-ptr);
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
            u64 entry_area = width*height;
            if (entry_area > best_entry_area)
            {
                best_entry = entry;
                best_entry_area = entry_area;
            }
        }

        // deserialize raw image data from best entry's offset
        u8 *image_data = 0;
        Vec2s32 image_dim = {0};
        if (best_entry != 0)
        {
            u8 *file_data_ptr = data.str + best_entry->image_data_off;
            u64 file_data_size = best_entry->image_data_size;
            int width = 0;
            int height = 0;
            int components = 0;
            image_data = stbi_load_from_memory(file_data_ptr, file_data_size, &width, &height, &components, 4);
            if (image_data == 0)
            {
                sh_message(1, s("ICO Failed to Load"), str8f(scratch.arena, "%s", stbi_failure_reason()));
            }
            image_dim.x = width;
            image_dim.y = height;
        }

        // upload to GPU texture
        ti_state->icon_texture = r_tex2d_alloc(R_ResourceKind_Static, image_dim, R_Tex2DFormat_RGBA8, image_data);

        // release
        stbi_image_free(image_data);
        scratch_end(scratch);
    }

    ProfEnd();
    scratch_end(scratch);
}
                         
internal void ti_frame(void)
{
    ProfBeginFunction();
    Temp scratch = scratch_begin(0, 0);
    log_scope_begin();
    ti_state->frame_depth += 1;

    ///////////////////////
    // do per-frame resets
    {
        Temp scratch = scratch_begin(0, 0);
        ti_state->top_regs = &ti_state->base_regs;
        ti_regs_copy_contents(scratch.arena, &ti_state->top_regs->v, &ti_state->top_regs->v);
        arena_clear(ti_frame_arena());
        ti_regs_copy_contents(ti_frame_arena(), &ti_state->top_regs->v, &ti_state->top_regs->v);
        scratch_end(scratch);
    }
    if (ti_state->frame_depth == 1)
    {
        arena_clear(ti_state->cmd_output_arena);
        MemoryZeroStruct(&ti_state->cmd_outputs);
    }

    //////////////////////////////
    // iterate all tabs, touch their view states
    //
    if (ti_state->frame_depth == 1)
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
                    if (ti_cfg_is_project_filtered(tab))
                    {
                        continue;
                    }
                    if (first_unfiltered_tab == &cfg_nil_node)
                    {
                        first_unfiltered_tab = tab;
                    }
                    ti_view_state_from_cfg(tab);
                }
                if (p->selected_tab == &cfg_nil_node && first_unfiltered_tab != &cfg_nil_node)
                {
                    ti_cmd(TI_CmdKind_FocusTab, .panel = p->cfg->id, .tab = first_unfiltered_tab->id);
                }
            }
        }
        scratch_end(scratch);
    }

    /////////////////
    // garbage collect untouched immediate cfg trees
    if (ti_state->frame_depth == 1)
    {
        CFG_Node *transient = cfg_node_child_from_string(cfg_node_root(), str8_lit("transient"));
        for (CFG_Node *tln = transient->first, *next = &cfg_nil_node; tln != &cfg_nil_node; tln = next)
        {
            next = tln->next;
            if (str8_match(tln->string, str8_lit("immediate"), 0))
            {
                if (cfg_node_child_from_string(tln, str8_lit("hot")) == &cfg_nil_node)
                {
                    cfg_node_release(ti_state->cfg, tln);
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
                        cfg_node_release(ti_state->cfg, child);
                    }
                }
            }
        }
    }

    ///////////////////
    // garbage collect untouched view states
    //
    if (ti_state->frame_depth == 1)
    {
        for EachIndex(slot_idx, ti_state->view_state_slots_count)
        {
            for (TI_View_State *vs = ti_state->view_state_slots[slot_idx].first, *next; vs != 0; vs = next)
            {
                next = vs->hash_next;
                if (vs->last_frame_index_touched+2 < ti_state->frame_index)
                {
                    for (TI_Arena_Ext *ext = vs->first_arena_ext; ext != 0; ext = ext->next)
                    {
                        arena_release(ext->arena);
                    }
                    arena_release(vs->arena);
                    DLLRemove_NP(ti_state->view_state_slots[slot_idx].first, ti_state->view_state_slots[slot_idx].last, vs, hash_next, hash_prev);
                    SLLStackPush_N(ti_state->free_view_state, vs, hash_next);
                }
            }
        }
    }

    ///////////////////
    // animate all views
    if (ti_state->frame_depth == 1)
    {
        bool32 any_window_is_focused = 0;
        for (TI_Window_State *w = ti_state->first_window_state; w != &ti_nil_window_state; w = w->order_next)
        {
            if (wm_window_is_focused(w->os))
            {
                any_window_is_focused = 1;
                break;
            }
        }
        f32 slow_rate = 1 - pow_f32(2, (-10.f * ti_state->frame_dt));
        f32 fast_rate = 1 - pow_f32(2, (-40.f * ti_state->frame_dt));
        for EachIndex(slot_idx, ti_state->view_state_slots_count)
        {
            for (TI_View_State *vs = ti_state->view_state_slots[slot_idx].first;
                 vs != 0;
                 vs = vs->hash_next)
            {
                f32 scroll_x_diff = (-vs->scroll_pos.x.off);
                f32 scroll_y_diff = (-vs->scroll_pos.y.off);
                f32 loading_t_diff = (vs->loading_t_target - vs->loading_t);
                vs->scroll_pos.x.off += scroll_x_diff*ti_state->scrolling_animation_rate;
                vs->scroll_pos.y.off += scroll_y_diff*ti_state->scrolling_animation_rate;
                vs->loading_t += loading_t_diff * slow_rate;
                if ((any_window_is_focused && abs_f32(loading_t_diff) > 0.01f) ||
                    abs_f32(scroll_x_diff) > 0.01f ||
                    abs_f32(scroll_y_diff) > 0.01f)
                {
                    ti_request_frame();
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
                        ti_request_frame();
                    }
                    vs->loading_t_target = 0;
                }
            }
        }
    }

    ////////////////////////////
    // get events from the OS
    //
    WM_Event_List events = {0};
    if (ti_state->frame_depth == 1)
    {
        events = wm_get_events(scratch.arena, ti_state->num_frames_requested == 0);// make a DEV_always_refresh here
    }

    ////////////////////
    // push frame scopes
    //
    Access *frame_access_restore = ti_state->frame_access;
    ti_state->frame_access = access_open();

    /////////////////////////////////////////////////
    // calculate avg length in us of last many frames
    //
    u64 frame_time_history_avg_us = 0;
    {
        u64 num_frames_in_history = Min(ArrayCount(ti_state->frame_time_us_history), ti_state->frame_index);
        u64 frame_time_history_sum_us = 0;
        if (num_frames_in_history > 0)
        {
            for (u64 idx = 0; idx < num_frames_in_history; idx += 1)
            {
                frame_time_history_sum_us += ti_state->frame_time_us_history[idx];
            }
            frame_time_history_avg_us = frame_time_history_sum_us / num_frames_in_history;
        }
    }
    
    ///////////////////////
    // pick target hz
    //
    // pick among number of sensible targets to snap to
    //
    f32 target_hz = wm_get_system_info()->default_refresh_rate;
    if (ti_state->frame_index > 32)
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
                    best_target_hz_frame_time_us_diff = frame_time_us_diff;
                }
            }
        }
        target_hz = best_target_hz;
    }

    //////////////////////////
    // target Hz -> delta time
    ti_state->frame_dt = 1.f / target_hz;

    /////////////////////////////////////////
    // megin measuring actual per-frame work
    u64 begin_time_us = now_time_us();

    ///////////////////////
    // build key map from config
    ProfScope("build key map from config")
    {
        ti_state->key_map = cfg_key_map_from_cfg(ti_frame_arena());
    }

    ////////////////////
    // consume events
    ProfScope("consume events")
    {
        for (WM_Event *event = events.first, *next = 0;
             event != 0;
             event = next)
            TI_RegsScope()
        {
            next = event->next;
            TI_Window_State *ws = ti_window_state_from_os_handle(event->window);
            if (ws != 0 && ws != ti_window_state_from_cfg(cfg_node_from_id(ti_regs()->window)))
            {
                Temp scratch = scratch_begin(0, 0);
                CFG_Panel_Tree panel_tree = cfg_panel_tree_from_cfg(scratch.arena, cfg_node_from_id(ws->cfg_id));
                ti_regs()->window = ws->cfg_id;
                ti_regs()->panel = panel_tree.focused->cfg->id;
                ti_regs()->tab   = panel_tree.focused->selected_tab->id;
                ti_regs()->view  = panel_tree.focused->selected_tab->id;
                scratch_end(scratch);
            }
            bool32 take = false;

            // try window close
            if (!take && event->kind == WM_EventKind_WindowClose && ws != 0)
            {
                take = true;
                ti_cmd(TI_CmdKind_Exit);
            }

            // try menu bar opertaions
            /*
            if (ti_state->alt_menu_bar_enabled && wm_window_is_focused(ws->os))
            {
                if (!take && event->kind == WM_EventKind_Press && event->key == WM_Key_Alt && event->modifiers == 0 && event->is_repeat == 0)
                {
                    take = true;
                    ti_request_frame();
                    ws->menu_bar_focused_on_press = ws->menu_bar_focused;
                    ws->menu_bar_key_held = true;
                    ws->menu_bar_focus_press_started = true;
                }
                if(!take && event->kind == WM_EventKind_Release && event->key == WM_Key_Alt && event->modifiers == 0 && event->is_repeat == 0)
                {
                    take = true;
                    ti_request_frame();
                    ws->menu_bar_key_held = 0;
                }
                if(ws->menu_bar_focused && event->kind == WM_EventKind_Press && event->key == WM_Key_Alt && event->modifiers == 0 && event->is_repeat == 0)
                {
                    take = true;
                    ti_request_frame();
                    ws->menu_bar_focused = 0;
                }
                else if(ws->menu_bar_focus_press_started && !ws->menu_bar_focused && event->kind == WM_EventKind_Release && event->modifiers == 0 && event->key == WM_Key_Alt && event->is_repeat == 0)
                {
                    take = true;
                    ti_request_frame();
                    ws->menu_bar_focused = !ws->menu_bar_focused_on_press;
                    ws->menu_bar_focus_press_started = 0;
                }
                else if(event->kind == WM_EventKind_Press && event->key == WM_Key_Esc && ws->menu_bar_focused && !ui_any_ctx_menu_is_open())
                {
                    take = true;
                    ti_request_frame();
                    ws->menu_bar_focused = 0;
                }
            }
            */

            // try hotkey presses

            // try text events

            // do fall-through
            if (!take)
            {
                take = true;
                ti_cmd(TI_CmdKind_WMEvent, .wm_event = event);
            }

            // take
            if (take)
            {
                wm_eat_event(&events, event);
            }
        }
    }

    ////////////////////////////////
    // loop - consume events in core, tick engine, and repeat
    //
    TI_Cmd *cmd = 0;
    ProfScope("loop - consume events in core, tick engine, and repeat") for (u64 cmd_process_loop_idx = 0; cmd_process_loop_idx < 3; cmd_process_loop_idx += 1)
    {
        /////////////////////////////////
        // evaluate unpacked settings (must be used earlier than this point in the frame,
        // but cannot evaluate before this point, so we need to prep for next frame
        //

        
        
        /////////////////////////////
        // process top-level graphical commands
        //
        if (ti_state->frame_depth == 1) ProfScope("process top-level graphical commands")
        {
            for (;ti_next_cmd(&cmd);) TI_RegsScope()
            {
                // unpack command
                TI_CmdKind kind = ti_cmd_kind_from_string(cmd->name);
                ti_regs_copy_contents(ti_frame_arena(), ti_regs(), cmd->regs);

                // request frame
                ti_request_frame();

                // process command
                CFG_Node *cfg = &cfg_nil_node;
                String8 dst_path = {0};
                String8 bucket_name = {0};
                Dir2 split_dir = Dir2_Invalid;
                CFG_Node *split_panel = &cfg_nil_node;
                u64 panel_sib_off = 0;
                u64 panel_child_off = 0;
                Vec2s32 panel_change_dir = {0};
                switch (kind)
                {
                    case TI_CmdKind_OpenTab: {
                        {
                            ti_cmd(TI_CmdKind_CancelAllQueries);
                            ti_cmd(TI_CmdKind_PushQuery, .expr = s("query:tab_commands"), .do_implicit_root = 1, .do_lister = 1, .prefer_new_tab = 1);
                        }
                    } break;
                        // command fast paths
                    case TI_CmdKind_RunCommand: {
                        {
                            TI_Cmd_Kind_Info *info = ti_cmd_kind_info_from_string(cmd->regs->cmd_name);

                            // command does not have a query - simply execute with the current registers
                            if (!(info->query.flags & TI_QueryFlag_Required))
                            {
                                TI_RegsScope(.cmd_name = str8_zero()) ti_push_cmd(cmd->regs->cmd_name, ti_regs());
                            }

                            // command has filesystem query, user wants native filesystem UI -> get the path the run the command
                            //else if (info->query.slot == TI_RegsSlot_FilePath && ti_setting_bool32_from_name(str8_lit("use_native_file_system_dialog")))

                            // command has required query -> prep query
                            else
                            {
                                ti_cmd(TI_CmdKind_PushQuery,
                                       .do_implicit_root = 1,
                                       .do_lister = (info->query.expr.size != 0),
                                       .expr = info->query.expr);
                            }
                        }
                    } break;
                    case TI_CmdKind_Exit: {
                        {
                            ti_state->quit = true;
                        }
                    } break;
                    case TI_CmdKind_OpenWindow: {
                        {
                            CFG_Node *old_window = cfg_node_from_id(ti_regs()->window);
                            CFG_Node *bucket = old_window->parent;
                            if (bucket == &cfg_nil_node)
                            {
                                bucket = cfg_node_child_from_string(cfg_node_root(), str8_lit("user"));
                            }
                            CFG_Node *new_window = cfg_node_new(ti_state->cfg, bucket, str8_lit("window"));
                            CFG_Node *size = cfg_node_new(ti_state->cfg, new_window, str8_lit("size"));
                            cfg_node_newf(ti_state->cfg, size, "1280");
                            cfg_node_newf(ti_state->cfg, size, "720");
                            for (CFG_Node *old_child = old_window->first;
                                 old_child != &cfg_nil_node;
                                 old_child = old_child->next)
                            {
                                if (!str8_match(old_child->string, str8_lit("panels"), 0) &&
                                    !str8_match(old_child->string, str8_lit("size"), 0) &&
                                    !str8_match(old_child->string, str8_lit("pos"), 0) &&
                                    !str8_match(old_child->string, str8_lit("monitor"), 0) &&
                                    !str8_match(old_child->string, str8_lit("fullscreen"), 0) &&
                                    !str8_match(old_child->string, str8_lit("maximized"), 0))
                                {
                                    CFG_Node *new_child = cfg_node_deep_copy(ti_state->cfg, old_child);
                                    cfg_node_insert_child(ti_state->cfg, new_window, new_window->last, new_child);
                                }
                            }
                            CFG_Node *panels = cfg_node_new(ti_state->cfg, new_window, str8_lit("panels"));
                            cfg_node_child_from_string_or_alloc(ti_state->cfg, panels, str8_lit("selected"));
                        }
                    } break;
                    case TI_CmdKind_CloseWindow: {
                        {
                            CFG_Node_Ptr_List all_windows = cfg_node_top_level_list_from_string(scratch.arena, str8_lit("window"));
                            CFG_Node *wcfg = cfg_node_from_id(ti_regs()->window);
                            if (all_windows.count == 1 && all_windows.first->v == wcfg)
                            {
                                ti_cmd(TI_CmdKind_Exit);
                            }
                            else
                            {
                                cfg_node_release(ti_state->cfg, wcfg);
                            }
                        }
                    } break;
                    case TI_CmdKind_ToggleFullscreen: {
                        {
                            CFG_Node *wcfg = cfg_node_from_id(ti_regs()->window);
                            TI_Window_State *ws = ti_window_state_from_cfg(wcfg);
                            if (ws != &ti_nil_window_state)
                            {
                                wm_window_set_fullscreen(ws->os, !wm_window_is_fullscreen(ws->os));
                            }
                        }
                    } break;
                    case TI_CmdKind_OpenUser:
                    case TI_CmdKind_OpenProject: {
                        {
                            String8 file_root_key = (kind == TI_CmdKind_OpenUser    ? str8_lit("user") :
                                                     kind == TI_CmdKind_OpenProject ? str8_lit("project") :
                                                     str8_lit("other"));
                            CFG_Node *file_root = cfg_node_child_from_string(cfg_node_root(), file_root_key);

                            // load the new file's data
                            String8 file_path = ti_regs()->file_path;
                            String8 file_data = data_from_file_path(scratch.arena, file_path);
                            File_Properties file_props = properties_from_file_path(file_path);

                            // determine if the file is good
                            bool32 file_is_okay = (file_data.size == 0 || str8_match(str8_prefix(file_data, 7), s("// tide"), 0));

                            // determine file's version
                            String8 file_version = {0};
                            if (file_is_okay && file_props.size != 0)
                            {
                                file_version = str8_skip(file_data, 10);
                                u64 line_end = str8_find_needle(file_version, 0, str8_lit("\n"), 0);
                                file_version = str8_prefix(file_version, line_end);
                                u64 first_space = str8_find_needle(file_version, 0, str8_lit(" "), 0);
                                file_version = str8_prefix(file_version, first_space);
                                file_version = str8_skip_chop_whitespace(file_version);
                            }

                            // bad file -> alert user
                            if (!file_is_okay)
                            {
                                log_user_errorf("\"%S\" appears to refer to an existing file which is not a TIDE config file. This would overwrite the file.", file_path);
                            }

                            // eliminate all old state under this file tree
                            if (file_is_okay)
                            {
                                cfg_node_release_all_children(ti_state->cfg, file_root);
                            }

                            // parse the new file, generate cfg entities for it
                            CFG_Node_Ptr_List file_cfg_list = {0};
                            if (file_is_okay)
                            {
                                u64 file_version_code = version_from_str8(file_version);
                                // legacy file handling when applicable
                                file_cfg_list = cfg_node_ptr_list_from_string(scratch.arena, ti_state->cfg, ti_state->cfg_schema_table, str8_chop_last_slash(file_path), file_data);
                            }

                            if (file_is_okay)
                            {
                                switch(kind)
                                {
                                    default:{}break;
                                    case TI_CmdKind_OpenUser: {
                                        arena_clear(ti_state->user_path_arena);
                                        ti_state->user_path = str8_copy(ti_state->user_path_arena, file_path);
                                    } break;
                                    case TI_CmdKind_OpenProject: {
                                        arena_clear(ti_state->project_path_arena);
                                        ti_state->project_path = str8_copy(ti_state->project_path_arena, file_path);
                                    } break;
                                }
                            }
                            
                            // insert the new cfg entities into this file tree
                            if (file_is_okay)
                            {
                                for (CFG_Node_Ptr_Node *n = file_cfg_list.first; n != 0; n = n->next)
                                {
                                    cfg_node_insert_child(ti_state->cfg, file_root, file_root->last, n->v);
                                }
                            }
                            
                            // if config did not open any windows for the user, then we need to open a sensible default
                            if (file_is_okay && kind == TI_CmdKind_OpenUser)
                            {
                                CFG_Node_Ptr_List all_user_windows = cfg_node_child_list_from_string(scratch.arena, file_root, str8_lit("window"));
                                if (all_user_windows.count == 0)
                                {
                                    WM_Monitor monitor   = wm_primary_monitor();
                                    String8 monitor_name = wm_name_from_monitor(scratch.arena, monitor);
                                    Vec2f32 monitor_dim  = wm_dim_from_monitor(monitor);
                                    f32 monitor_dpi      = wm_dpi_from_monitor(monitor);
                                    Vec2f32 window_dim   = v2f32(monitor_dim.x*4/5, monitor_dim.y*4/5);
                                    if (window_dim.x == 0 || window_dim.y == 0)
                                    {
                                        window_dim = v2f32(1280, 720);
                                    }
                                    CFG_Node *new_window = cfg_node_new(ti_state->cfg, file_root, str8_lit("window"));
                                    CFG_Node *size = cfg_node_new(ti_state->cfg, new_window, str8_lit("size"));
                                    cfg_node_newf(ti_state->cfg, size, "%f", window_dim.x);
                                    cfg_node_newf(ti_state->cfg, size, "%f", window_dim.y);
                                    ti_cmd(TI_CmdKind_ResetToDefaultPanels, .window = new_window->id);
                                }
                            }
                            
                            // if config did not define any keybindings for the user, then we need to build a sensible default
                            /*
                              if (file_is_okay && kind == TI_CmdKind_OpenUser)
                              {
                              CFG_Node_Ptr_List all_keybindings = cfg_node_child_list_from_string(scratch.arena, file_root, str8_lit("keybindings"));
                              if (all_keybindings.count == 0)
                              {
                              ti_cmd(TI_CmdKind_ResetToDefaultBindings);
                              }
                              }
                            */
                            
                            // record last-opened user in config directory
                            
                            // record recently-opened projects in the user
                            
                            // eliminate all project-filtered tab focuses
                            
                            // if just opened project -> set new current path
                            
                            // if just oepned user -> load last project, is enabled
                        }
                    } break;
                    case TI_CmdKind_NewUser: {
                        {
                            ti_cmd(TI_CmdKind_OpenUser, .file_path = str8_zero());
                        }
                    } break;
                    case TI_CmdKind_NewProject: {
                        {
                            ti_cmd(TI_CmdKind_OpenProject, .file_path = str8_zero());
                        }
                    } break;
                    case TI_CmdKind_IncWindowFontSize: cfg = cfg_node_from_id(ti_regs()->window); ti_regs()->view = 0; ti_regs()->tab = 0; goto inc_font_size;
                    case TI_CmdKind_IncViewFontSize:   cfg = cfg_node_from_id(ti_regs()->view); goto inc_font_size;
                    inc_font_size:;
                    if (cfg != &cfg_nil_node)
                    {
                        fnt_reset();
                        f32 current_font_size = ti_font_size();
                        f32 new_font_size = current_font_size+1;
                        new_font_size = Clamp(6.f, new_font_size, 72.f);
                        CFG_Node *font_size_cfg = cfg_node_child_from_string_or_alloc(ti_state->cfg, cfg, str8_lit("font_size"));
                        cfg_node_new_replacef(ti_state->cfg, font_size_cfg, "%I64u", (u64)new_font_size);
                    } break;
                    case TI_CmdKind_DecWindowFontSize: cfg = cfg_node_from_id(ti_regs()->window); ti_regs()->view = 0; ti_regs()->tab = 0; goto dec_font_size;
                    case TI_CmdKind_DecViewFontSize:   cfg = cfg_node_from_id(ti_regs()->view); goto dec_font_size;
                    dec_font_size:;
                    if (cfg != &cfg_nil_node)
                    {
                        fnt_reset();
                        f32 current_font_size = ti_font_size();
                        f32 new_font_size = current_font_size-1;
                        new_font_size = Clamp(6.f, new_font_size, 72.f);
                        CFG_Node *font_size_cfg = cfg_node_child_from_string_or_alloc(ti_state->cfg, cfg, str8_lit("font_size"));
                        cfg_node_new_replacef(ti_state->cfg, font_size_cfg, "%I64u", (u64)new_font_size);
                    } break;
                    // panel creation
                    case TI_CmdKind_NewPanelLeft: {split_dir = Dir2_Left;}goto split;
                    case TI_CmdKind_NewPanelUp:   {split_dir = Dir2_Up;}goto split;
                    case TI_CmdKind_NewPanelRight:{split_dir = Dir2_Right;}goto split;
                    case TI_CmdKind_NewPanelDown: {split_dir = Dir2_Down;}goto split;
                    case TI_CmdKind_SplitPanel:
                        {
                            split_dir = ti_regs()->dir2;
                            split_panel = cfg_node_from_id(ti_regs()->dst_panel);
                        } goto split;
                    split:;
                    if (split_dir != Dir2_Invalid)
                    {
                        Axis2 split_axis = axis2_from_dir2(split_dir);
                        Side split_side = side_from_dir2(split_dir);
                        if (split_panel == &cfg_nil_node)
                        {
                            split_panel = cfg_node_from_id(ti_regs()->panel);
                        }
                        CFG_Node *new_panel_cfg = &cfg_nil_node;
                        CFG_Panel_Tree panel_tree = cfg_panel_tree_from_cfg(scratch.arena, split_panel);
                        CFG_Panel_Node *panel_root = panel_tree.root;
                        CFG_Panel_Node *panel = cfg_panel_node_from_tree_cfg(panel_root, split_panel);
                        CFG_Panel_Node *parent = panel->parent;

                        // splitting on same axis as parent -> insert new sibling on same axis, adjust sizes
                        if (parent != &cfg_nil_panel_node && parent->split_axis == split_axis)
                        {
                            CFG_Node *parent_cfg = parent->cfg;
                            CFG_Node *panel_cfg = panel->cfg;
                            CFG_Node *new_cfg = cfg_node_alloc(ti_state->cfg);
                            cfg_node_insert_child(ti_state->cfg, parent_cfg, split_side == Side_Max ? panel_cfg : panel_cfg->prev, new_cfg);
                            cfg_node_equip_stringf(ti_state->cfg, new_cfg, "%f", 1.f/(parent->child_count+1));
                            for (CFG_Panel_Node *child = parent->first; child != &cfg_nil_panel_node; child = child->next)
                            {
                                f32 old_pct = child->pct_of_parent;
                                f32 new_pct = old_pct * ((f32)(parent->child_count) / (parent->child_count+1));
                                cfg_node_equip_stringf(ti_state->cfg, child->cfg, "%f", new_pct);
                            }
                            new_panel_cfg = new_cfg;
                        }

                        // splitting on opposite axis as parent - need to create new replacement node, + new sibling
                        else
                        {
                            CFG_Node *split_panel_prev = panel->prev->cfg;
                            CFG_Node *new_parent = cfg_node_alloc(ti_state->cfg);
                            CFG_Node *new_sibling = cfg_node_alloc(ti_state->cfg);
                            cfg_node_equip_string(ti_state->cfg, new_parent, split_panel->string);
                            cfg_node_equip_string(ti_state->cfg, split_panel, str8_lit("0.5"));
                            cfg_node_equip_string(ti_state->cfg, new_sibling, str8_lit("0.5"));
                            if (parent->cfg != &cfg_nil_node)
                            {
                                cfg_node_unhook(ti_state->cfg, parent->cfg, split_panel);
                                cfg_node_insert_child(ti_state->cfg, parent->cfg, split_panel_prev, new_parent);
                            }
                            else
                            {
                                cfg_node_equip_string(ti_state->cfg, new_parent, str8_lit("panels"));
                                CFG_Node *window_cfg = ti_window_from_cfg(split_panel);
                                cfg_node_insert_child(ti_state->cfg, window_cfg, window_cfg->last, new_parent);
                                if (split_axis == Axis2_X)
                                {
                                    cfg_node_child_from_string_or_alloc(ti_state->cfg, window_cfg, str8_lit("split_x"));
                                }
                                else
                                {
                                    cfg_node_release(ti_state->cfg, cfg_node_child_from_string(window_cfg, str8_lit("split_x")));
                                }
                            }
                            CFG_Node *min = split_panel;
                            CFG_Node *max = new_sibling;
                            if (split_side == Side_Min)
                            {
                                Swap(CFG_Node *, min, max);
                            }
                            cfg_node_insert_child(ti_state->cfg, new_parent, new_parent->last, min);
                            cfg_node_insert_child(ti_state->cfg, new_parent, new_parent->last, max);
                            new_panel_cfg = new_sibling;
                        }

                        // pre-emptively set up the animation rectangle, depending on where
                        // the new panel was inserted
                        {
                            TI_Window_State *ws = ti_window_state_from_cfg(new_panel_cfg);
                            if (ws != &ti_nil_window_state)
                            {
                                ui_select_state(ws->ui);
                                CFG_Panel_Tree new_panel_tree = cfg_panel_tree_from_cfg(scratch.arena, new_panel_cfg);
                                CFG_Panel_Node *new_panel = cfg_panel_node_from_tree_cfg(new_panel_tree.root, new_panel_cfg);
                                Rng2f32 stub_content_rect = r2f32p(0, 0, 1000, 1000);
                                Vec2f32 stub_content_rect_dim = dim_2f32(stub_content_rect);
                                Rng2f32 new_rect_px  = cfg_target_rect_from_panel_node(stub_content_rect, new_panel_tree.root, new_panel);
                                Rng2f32 new_rect_pct = r2f32p(new_rect_px.x0/stub_content_rect_dim.x,
                                                              new_rect_px.y0/stub_content_rect_dim.y,
                                                              new_rect_px.x1/stub_content_rect_dim.x,
                                                              new_rect_px.y1/stub_content_rect_dim.y);
                                if (new_panel->prev != &cfg_nil_panel_node)
                                {
                                    Rng2f32 target_prev_rect_px  = cfg_target_rect_from_panel_node(stub_content_rect, panel_tree.root, cfg_panel_node_from_tree_cfg(panel_tree.root, new_panel->prev->cfg));
                                    Rng2f32 target_prev_rect_pct = r2f32p(target_prev_rect_px.x0/stub_content_rect_dim.x,
                                                                          target_prev_rect_px.y0/stub_content_rect_dim.y,
                                                                          target_prev_rect_px.x1/stub_content_rect_dim.x,
                                                                          target_prev_rect_px.y1/stub_content_rect_dim.y);
                                    Rng2f32 prev_rect_pct = r2f32p(ui_anim(ui_key_from_stringf(ui_key_zero(), "panel_%p_x0", new_panel->prev->cfg), target_prev_rect_pct.x0, .initial = target_prev_rect_pct.x0, .rate = ti_state->menu_animation_rate),
                                                                   ui_anim(ui_key_from_stringf(ui_key_zero(), "panel_%p_y0", new_panel->prev->cfg), target_prev_rect_pct.y0, .initial = target_prev_rect_pct.y0, .rate = ti_state->menu_animation_rate),
                                                                   ui_anim(ui_key_from_stringf(ui_key_zero(), "panel_%p_x1", new_panel->prev->cfg), target_prev_rect_pct.x1, .initial = target_prev_rect_pct.x1, .rate = ti_state->menu_animation_rate),
                                                                   ui_anim(ui_key_from_stringf(ui_key_zero(), "panel_%p_y1", new_panel->prev->cfg), target_prev_rect_pct.y1, .initial = target_prev_rect_pct.y1, .rate = ti_state->menu_animation_rate));
                                    new_rect_pct = prev_rect_pct;
                                    new_rect_pct.p0.v[split_axis] = new_rect_pct.p1.v[split_axis];
                                }
                                if (new_panel->next != &cfg_nil_panel_node)
                                {
                                    Rng2f32 target_next_rect_px  = cfg_target_rect_from_panel_node(stub_content_rect, panel_tree.root, cfg_panel_node_from_tree_cfg(panel_tree.root, new_panel->next->cfg));
                                    Rng2f32 target_next_rect_pct = r2f32p(target_next_rect_px.x0/stub_content_rect_dim.x,
                                                                          target_next_rect_px.y0/stub_content_rect_dim.y,
                                                                          target_next_rect_px.x1/stub_content_rect_dim.x,
                                                                          target_next_rect_px.y1/stub_content_rect_dim.y);
                                    Rng2f32 next_rect_pct = r2f32p(ui_anim(ui_key_from_stringf(ui_key_zero(), "panel_%p_x0", new_panel->next->cfg), target_next_rect_pct.x0, .initial = target_next_rect_pct.x0, .rate = ti_state->menu_animation_rate),
                                                                   ui_anim(ui_key_from_stringf(ui_key_zero(), "panel_%p_y0", new_panel->next->cfg), target_next_rect_pct.y0, .initial = target_next_rect_pct.y0, .rate = ti_state->menu_animation_rate),
                                                                   ui_anim(ui_key_from_stringf(ui_key_zero(), "panel_%p_x1", new_panel->next->cfg), target_next_rect_pct.x1, .initial = target_next_rect_pct.x1, .rate = ti_state->menu_animation_rate),
                                                                   ui_anim(ui_key_from_stringf(ui_key_zero(), "panel_%p_y1", new_panel->next->cfg), target_next_rect_pct.y1, .initial = target_next_rect_pct.y1, .rate = ti_state->menu_animation_rate));
                                    new_rect_pct = next_rect_pct;
                                    new_rect_pct.p1.v[split_axis] = new_rect_pct.p0.v[split_axis];
                                }
                                ui_anim(ui_key_from_stringf(ui_key_zero(), "panel_%p_x0", new_panel->cfg), new_rect_pct.x0, .initial = new_rect_pct.x0, .reset = 1, .rate = ti_state->menu_animation_rate);
                                ui_anim(ui_key_from_stringf(ui_key_zero(), "panel_%p_x1", new_panel->cfg), new_rect_pct.x1, .initial = new_rect_pct.x1, .reset = 1, .rate = ti_state->menu_animation_rate);
                                ui_anim(ui_key_from_stringf(ui_key_zero(), "panel_%p_y0", new_panel->cfg), new_rect_pct.y0, .initial = new_rect_pct.y0, .reset = 1, .rate = ti_state->menu_animation_rate);
                                ui_anim(ui_key_from_stringf(ui_key_zero(), "panel_%p_y1", new_panel->cfg), new_rect_pct.y1, .initial = new_rect_pct.y1, .reset = 1, .rate = ti_state->menu_animation_rate);
                            }
                        }

                        // if this split was caused by drag/dropping a tab, and the originating panel
                        // has no further tabs, then close the originating panel
                        CFG_Node *dragdrop_origin_panel_cfg = cfg_node_from_id(ti_regs()->panel);
                        CFG_Node *dragdrop_tab = cfg_node_from_id(ti_regs()->view);
                        if(kind == TI_CmdKind_SplitPanel &&
                           new_panel_cfg != &cfg_nil_node && dragdrop_tab != &cfg_nil_node && dragdrop_origin_panel_cfg != &cfg_nil_node)
                        {
                            cfg_node_unhook(ti_state->cfg, dragdrop_origin_panel_cfg, dragdrop_tab);
                            cfg_node_insert_child(ti_state->cfg, new_panel_cfg, new_panel_cfg->last, dragdrop_tab);
                            CFG_Panel_Tree origin_panel_tree = cfg_panel_tree_from_cfg(scratch.arena, dragdrop_origin_panel_cfg);
                            CFG_Panel_Node *origin_panel = cfg_panel_node_from_tree_cfg(origin_panel_tree.root, dragdrop_origin_panel_cfg);
                            if(origin_panel->selected_tab == &cfg_nil_node)
                            {
                                for(CFG_Node_Ptr_Node *n = origin_panel->tabs.first; n != 0; n = n->next)
                                {
                                    if(!ti_cfg_is_project_filtered(n->v))
                                    {
                                        ti_cmd(TI_CmdKind_FocusTab, .panel = origin_panel->cfg->id, .tab = n->v->id);
                                        break;
                                    }
                                }
                            }
                            if(origin_panel->cfg != split_panel && origin_panel->tabs.count == 0)
                            {
                                ti_cmd(TI_CmdKind_ClosePanel);
                            }
                            ti_cmd(TI_CmdKind_FocusTab, .panel = new_panel_cfg->id, .tab = dragdrop_tab->id);
                        }
                        
                        // focus new panel
                        if(new_panel_cfg != &cfg_nil_node)
                        {
                            ti_cmd(TI_CmdKind_FocusPanel, .panel = new_panel_cfg->id);
                        }
                        
                        // tabs on bottom on split panel? -> tabs on bottom on new panel
                        if(panel->tab_side == Side_Max && split_axis == Axis2_X)
                        {
                            ti_cmd(TI_CmdKind_TabBarBottom, .panel = new_panel_cfg->id);
                        }
                    } break;
                    case TI_CmdKind_NextPanel: panel_sib_off = OffsetOf(CFG_Panel_Node, next); panel_child_off = OffsetOf(CFG_Panel_Node, first); goto cycle;
                    case TI_CmdKind_PrevPanel: panel_sib_off = OffsetOf(CFG_Panel_Node, prev); panel_child_off = OffsetOf(CFG_Panel_Node, last); goto cycle;
                    cycle:;
                    {
                        CFG_Panel_Tree panel_tree = cfg_panel_tree_from_cfg(scratch.arena, cfg_node_from_id(ti_regs()->window));
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
                            for (CFG_Panel_Node *p = panel_tree.root;
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
                        ti_cmd(TI_CmdKind_FocusPanel, .panel = next_focused->cfg->id);
                    } break;
                    case TI_CmdKind_FocusPanel: {
                        {
                            CFG_Node *panel = cfg_node_from_id(ti_regs()->panel);
                            CFG_Panel_Tree panel_tree = cfg_panel_tree_from_cfg(scratch.arena, panel);
                            CFG_Node *selection_cfg = &cfg_nil_node;
                            for (CFG_Panel_Node *p = panel_tree.root;
                                 p != &cfg_nil_panel_node;
                                 p = cfg_panel_node_rec__depth_first_pre(panel_tree.root, p).next)
                            {
                                CFG_Node *p_cfg = p->cfg;
                                CFG_Node *p_selection = cfg_node_child_from_string(p_cfg, str8_lit("selected"));
                                if (selection_cfg == &cfg_nil_node)
                                {
                                    selection_cfg = p_selection;
                                }
                                else for (CFG_Node *s = p_selection; s != &cfg_nil_node; s = cfg_node_child_from_string(p_cfg, str8_lit("selected")))
                                {
                                    cfg_node_release(ti_state->cfg, s);
                                }
                            }
                            if (selection_cfg == &cfg_nil_node)
                            {
                                selection_cfg = cfg_node_alloc(ti_state->cfg);
                                cfg_node_equip_string(ti_state->cfg, selection_cfg, str8_lit("selected"));
                            }
                            if (panel != &cfg_nil_node)
                            {
                                cfg_node_insert_child(ti_state->cfg, panel, &cfg_nil_node, selection_cfg);
                                CFG_Node *window = ti_window_from_cfg(panel);
                                TI_Window_State *ws = ti_window_state_from_cfg(window);
                                ws->menu_bar_focused = 0;
                            }
                        }
                    } break;

                        // directional panel focus changing
                    case TI_CmdKind_FocusPanelRight: panel_change_dir = v2s32(+1, +0); goto focus_panel_dir;
                    case TI_CmdKind_FocusPanelLeft:  panel_change_dir = v2s32(-1, +0); goto focus_panel_dir;
                    case TI_CmdKind_FocusPanelUp:    panel_change_dir = v2s32(+0, -1); goto focus_panel_dir;
                    case TI_CmdKind_FocusPanelDown:  panel_change_dir = v2s32(+0, +1); goto focus_panel_dir;
                    focus_panel_dir:;
                    {
                        CFG_Node *window = cfg_node_from_id(ti_regs()->window);
                        CFG_Panel_Tree panel_tree = cfg_panel_tree_from_cfg(scratch.arena, window);
                        CFG_Panel_Node *src_panel = panel_tree.focused;
                        Rng2f32 src_panel_rect = cfg_target_rect_from_panel_node(r2f32(v2f32(0, 0), v2f32(1000, 1000)), panel_tree.root, src_panel);
                        Vec2f32 src_panel_center = center_2f32(src_panel_rect);
                        Vec2f32 src_panel_half_dim = scale_2f32(dim_2f32(src_panel_rect), 0.5f);
                        Vec2f32 travel_dim = add_2f32(src_panel_half_dim, v2f32(10.f, 10.f));
                        Vec2f32 travel_dst = add_2f32(src_panel_center, mul_2f32(travel_dim, v2f32((f32)panel_change_dir.x, (f32)panel_change_dir.y)));
                        CFG_Panel_Node *dst_root = &cfg_nil_panel_node;
                        for(CFG_Panel_Node *p = panel_tree.root; p != &cfg_nil_panel_node; p = cfg_panel_node_rec__depth_first_pre(panel_tree.root, p).next)
                        {
                            if(p == src_panel || p->first != &cfg_nil_panel_node)
                            {
                                continue;
                            }
                            Rng2f32 p_rect = cfg_target_rect_from_panel_node(r2f32(v2f32(0, 0), v2f32(1000, 1000)), panel_tree.root, p);
                            if(contains_2f32(p_rect, travel_dst))
                            {
                                dst_root = p;
                                break;
                            }
                        }
                        if(dst_root != &cfg_nil_panel_node)
                        {
                            CFG_Panel_Node *dst_panel = &cfg_nil_panel_node;
                            for(CFG_Panel_Node *p = dst_root; p != &cfg_nil_panel_node; p = cfg_panel_node_rec__depth_first_pre(dst_root, p).next)
                            {
                                if(p->first == &cfg_nil_panel_node && p != src_panel)
                                {
                                    dst_panel = p;
                                    break;
                                }
                            }
                            ti_cmd(TI_CmdKind_FocusPanel, .panel = dst_panel->cfg->id);
                        }
                    } break;
                    // panel removal
                    case TI_CmdKind_ClosePanel: {
                        {
                            CFG_Node *window = cfg_node_from_id(ti_regs()->window);
                            CFG_Panel_Tree panel_tree = cfg_panel_tree_from_cfg(scratch.arena, window);
                            CFG_Panel_Node *panel = cfg_panel_node_from_tree_cfg(panel_tree.root, cfg_node_from_id(ti_regs()->panel));
                            CFG_Panel_Node *parent = panel->parent;
                            if(parent != &cfg_nil_panel_node)
                            {
                                Axis2 split_axis = parent->split_axis;
                                
                                // NOTE(rjf): If we're removing all but the last child of this parent,
                                // we should just remove both children.
                                if(parent->child_count == 2)
                                {
                                    CFG_Panel_Node *discard_child = panel;
                                    CFG_Panel_Node *keep_child = (panel == parent->first ? parent->last : parent->first);
                                    CFG_Panel_Node *grandparent = parent->parent;
                                    CFG_Panel_Node *parent_prev = parent->prev;
                                    f32 pct_of_parent = parent->pct_of_parent;
                                    
                                    // rjf: unhook kept child
                                    cfg_node_unhook(ti_state->cfg, parent->cfg, keep_child->cfg);
                                    
                                    // rjf: unhook this subtree
                                    if(grandparent != &cfg_nil_panel_node)
                                    {
                                        cfg_node_unhook(ti_state->cfg, grandparent->cfg, parent->cfg);
                                    }
                                    
                                    // rjf: release the containing tree
                                    {
                                        cfg_node_release(ti_state->cfg, parent->cfg);
                                    }
                                    
                                    // rjf: re-hook our kept child into the overall tree
                                    if(grandparent == &cfg_nil_panel_node)
                                    {
                                        if(keep_child->split_axis == Axis2_X)
                                        {
                                            cfg_node_child_from_string_or_alloc(ti_state->cfg, window, str8_lit("split_x"));
                                        }
                                        else
                                        {
                                            cfg_node_release(ti_state->cfg, cfg_node_child_from_string(window, str8_lit("split_x")));
                                        }
                                        cfg_node_equip_string(ti_state->cfg, keep_child->cfg, str8_lit("panels"));
                                        cfg_node_insert_child(ti_state->cfg, window, window->last, keep_child->cfg);
                                    }
                                    else
                                    {
                                        cfg_node_insert_child(ti_state->cfg, grandparent->cfg, parent_prev->cfg, keep_child->cfg);
                                        cfg_node_equip_stringf(ti_state->cfg, keep_child->cfg, "%f", pct_of_parent);
                                    }
                                    
                                    // rjf: keep-child split-axis == grandparent split-axis? bubble keep-child up into grandparent's children
                                    if(grandparent != &cfg_nil_panel_node && grandparent->split_axis == keep_child->split_axis && keep_child->first != &cfg_nil_panel_node)
                                    {
                                        cfg_node_unhook(ti_state->cfg, grandparent->cfg, keep_child->cfg);
                                        CFG_Node *prev = parent_prev->cfg;
                                        for(CFG_Panel_Node *child = keep_child->first, *next = &cfg_nil_panel_node; child != &cfg_nil_panel_node; child = next)
                                        {
                                            next = child->next;
                                            cfg_node_unhook(ti_state->cfg, keep_child->cfg, child->cfg);
                                            cfg_node_insert_child(ti_state->cfg, grandparent->cfg, prev, child->cfg);
                                            prev = child->cfg;
                                            f32 old_pct = child->pct_of_parent;
                                            f32 new_pct = old_pct * pct_of_parent;
                                            cfg_node_equip_stringf(ti_state->cfg, child->cfg, "%f", new_pct);
                                        }
                                        cfg_node_release(ti_state->cfg, keep_child->cfg);
                                    }
                                    
                                    // rjf: reset focus, if needed
                                    if(panel_tree.focused == discard_child)
                                    {
                                        CFG_Panel_Tree new_panel_tree = cfg_panel_tree_from_cfg(scratch.arena, window);
                                        CFG_Panel_Node *new_focused = cfg_panel_node_from_tree_cfg(panel_tree.root, keep_child->cfg);
                                        for(CFG_Panel_Node *grandchild = new_focused; grandchild != &cfg_nil_panel_node; grandchild = grandchild->first)
                                        {
                                            new_focused = grandchild;
                                        }
                                        ti_cmd(TI_CmdKind_FocusPanel, .panel = new_focused->cfg->id);
                                    }
                                }
                                // NOTE(rjf): Otherwise we can just remove this child.
                                else
                                {
                                    // rjf: remove
                                    CFG_Panel_Node *next = &cfg_nil_panel_node;
                                    f32 removed_size_pct = panel->pct_of_parent;
                                    if(next == &cfg_nil_panel_node) { next = panel->prev; }
                                    if(next == &cfg_nil_panel_node) { next = panel->next; }
                                    cfg_node_unhook(ti_state->cfg, parent->cfg, panel->cfg);
                                    cfg_node_release(ti_state->cfg, panel->cfg);
                                    
                                    // rjf: resize siblings to this node
                                    {
                                        CFG_Panel_Tree new_panel_tree = cfg_panel_tree_from_cfg(scratch.arena, window);
                                        CFG_Panel_Node *new_parent = cfg_panel_node_from_tree_cfg(new_panel_tree.root, parent->cfg);
                                        for(CFG_Panel_Node *child = new_parent->first; child != &cfg_nil_panel_node; child = child->next)
                                        {
                                            CFG_Node *cfg = child->cfg;
                                            f32 old_pct = child->pct_of_parent;
                                            f32 new_pct = old_pct / (1.f-removed_size_pct);
                                            cfg_node_equip_stringf(ti_state->cfg, cfg, "%f", new_pct);
                                        }
                                    }
                                    
                                    // rjf: reset focus, if needed
                                    if(panel_tree.focused == panel)
                                    {
                                        CFG_Panel_Tree new_panel_tree = cfg_panel_tree_from_cfg(scratch.arena, window);
                                        CFG_Panel_Node *new_focused = cfg_panel_node_from_tree_cfg(panel_tree.root, next->cfg);
                                        for(CFG_Panel_Node *grandchild = new_focused; grandchild != &cfg_nil_panel_node; grandchild = grandchild->first)
                                        {
                                            new_focused = grandchild;
                                        }
                                        ti_cmd(TI_CmdKind_FocusPanel, .panel = new_focused->cfg->id);
                                    }
                                }
                            }
                        }
                    } break;
                    case TI_CmdKind_FocusTab: {
                        {
                            CFG_Node *tab = cfg_node_from_id(ti_regs()->tab);
                            CFG_Node *panel = tab->parent;
                            if(panel == &cfg_nil_node)
                            {
                                panel = cfg_node_from_id(ti_regs()->panel);
                            }
                            CFG_Panel_Tree panel_tree = cfg_panel_tree_from_cfg(scratch.arena, panel);
                            CFG_Panel_Node *panel_node = cfg_panel_node_from_tree_cfg(panel_tree.root, panel);
                            CFG_Node *selection_cfg = &cfg_nil_node;
                            for(CFG_Node_Ptr_Node *n = panel_node->tabs.first; n != 0; n = n->next)
                            {
                                CFG_Node *tab_selection_cfg = cfg_node_child_from_string(n->v, str8_lit("selected"));
                                if(selection_cfg == &cfg_nil_node)
                                {
                                    selection_cfg = tab_selection_cfg;
                                    cfg_node_unhook(ti_state->cfg, n->v, selection_cfg);
                                }
                                else for(CFG_Node *s = tab_selection_cfg; s != &cfg_nil_node; s = cfg_node_child_from_string(n->v, str8_lit("selected")))
                                     {
                                         cfg_node_release(ti_state->cfg, s);
                                     }
                            }
                            if(selection_cfg == &cfg_nil_node)
                            {
                                selection_cfg = cfg_node_alloc(ti_state->cfg);
                                cfg_node_equip_string(ti_state->cfg, selection_cfg, str8_lit("selected"));
                            }
                            if(tab != &cfg_nil_node)
                            {
                                cfg_node_insert_child(ti_state->cfg, tab, &cfg_nil_node, selection_cfg);
                            }
                            else
                            {
                                cfg_node_release(ti_state->cfg, selection_cfg);
                            }
                        }
                    } break;
                    case TI_CmdKind_NextTab: {
                        {
                            CFG_Node *window = cfg_node_from_id(ti_regs()->window);
                            CFG_Panel_Tree panel_tree = cfg_panel_tree_from_cfg(scratch.arena, window);
                            CFG_Panel_Node *focused = panel_tree.focused;
                            CFG_Node_Ptr_Node *selected_tab_n = 0;
                            for(CFG_Node_Ptr_Node *n = focused->tabs.first; n != 0; n = n->next)
                            {
                                if(n->v == focused->selected_tab)
                                {
                                    selected_tab_n = n;
                                    break;
                                }
                            }
                            CFG_Node *next_selected_tab = &cfg_nil_node;
                            u64 idx = 0;
                            for(CFG_Node_Ptr_Node *tab_n = selected_tab_n;
                                tab_n != 0 && (tab_n != selected_tab_n || idx == 0);
                                ((tab_n->next == 0) ? (tab_n = focused->tabs.first) : (tab_n = tab_n->next)), idx += 1)
                            {
                                if(!ti_cfg_is_project_filtered(tab_n->v) && tab_n != selected_tab_n)
                                {
                                    next_selected_tab = tab_n->v;
                                    break;
                                }
                            }
                            if(next_selected_tab != &cfg_nil_node)
                            {
                                ti_cmd(TI_CmdKind_FocusTab, .tab = next_selected_tab->id);
                            }
                        }
                    } break;
                    case TI_CmdKind_PrevTab: {
                        {
                            CFG_Node *window = cfg_node_from_id(ti_regs()->window);
                            CFG_Panel_Tree panel_tree = cfg_panel_tree_from_cfg(scratch.arena, window);
                            CFG_Panel_Node *focused = panel_tree.focused;
                            CFG_Node_Ptr_Node *selected_tab_n = 0;
                            for(CFG_Node_Ptr_Node *n = focused->tabs.last; n != 0; n = n->prev)
                            {
                                if(n->v == focused->selected_tab)
                                {
                                    selected_tab_n = n;
                                    break;
                                }
                            }
                            CFG_Node *next_selected_tab = &cfg_nil_node;
                            u64 idx = 0;
                            for(CFG_Node_Ptr_Node *tab_n = selected_tab_n;
                                tab_n != 0 && (tab_n != selected_tab_n || idx == 0);
                                ((tab_n->prev == 0) ? (tab_n = focused->tabs.last) : (tab_n = tab_n->prev)), idx += 1)
                            {
                                if(!ti_cfg_is_project_filtered(tab_n->v) && tab_n != selected_tab_n)
                                {
                                    next_selected_tab = tab_n->v;
                                    break;
                                }
                            }
                            if(next_selected_tab != &cfg_nil_node)
                            {
                                ti_cmd(TI_CmdKind_FocusTab, .tab = next_selected_tab->id);
                            }
                        }
                    } break;
                    case TI_CmdKind_MoveTabRight:
                    case TI_CmdKind_MoveTabLeft: {
                        {
                            CFG_Node *tab = cfg_node_from_id(ti_regs()->tab);
                            CFG_Node *window = ti_window_from_cfg(tab);
                            CFG_Panel_Tree panel_tree = cfg_panel_tree_from_cfg(scratch.arena, window);
                            CFG_Panel_Node *panel = cfg_panel_node_from_tree_cfg(panel_tree.root, tab->parent);
                            CFG_Node_Ptr_List filtered_tabs = {0};
                            for(CFG_Node_Ptr_Node *n = panel->tabs.first; n != 0; n = n->next)
                            {
                                if(ti_cfg_is_project_filtered(n->v))
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
                                for(CFG_Node_Ptr_Node *n = filtered_tabs.first; n != 0; (prev2 = prev, prev = n->v, n = n->next))
                                {
                                    next = n->next ? n->next->v : &cfg_nil_node;
                                    if(n->v == tab)
                                    {
                                        tab_prev2 = prev2;
                                        tab_prev = prev;
                                        tab_next = next;
                                        break;
                                    }
                                }
                            }
                            CFG_Node *new_prev = (kind == TI_CmdKind_MoveTabRight ? tab_next : tab_prev2);
                            if(new_prev == tab_prev && filtered_tabs.last)
                            {
                                new_prev = filtered_tabs.last->v;
                            }
                            ti_cmd(TI_CmdKind_MoveView,
                                   .dst_panel = panel->cfg->id,
                                   .view     = tab->id,
                                   .prev_tab  = new_prev->id);
                        }
                    } break;
                    case TI_CmdKind_BuildTab: {
                        {
                            String8 expr_file_path = s("test tab");
                            CFG_Node *panel = cfg_node_from_id(ti_regs()->panel);
                            CFG_Node *tab = cfg_node_new(ti_state->cfg, panel, ti_regs()->string);
                            CFG_Node *expr = cfg_node_new(ti_state->cfg, tab, str8_lit("expression"));
                            cfg_node_new(ti_state->cfg, expr, s("test tab"));
                            if (expr_file_path.size != 0)
                            {
                                CFG_Node *project = cfg_node_new(ti_state->cfg, tab, str8_lit("project"));
                                cfg_node_new(ti_state->cfg, project, ti_state->project_path);
                            }
                            ti_cmd(TI_CmdKind_FocusTab, .tab = tab->id);
                        }
                    } break;
                    case TI_CmdKind_DuplicateTab: {
                        {
                            CFG_Node *src = cfg_node_from_id(ti_regs()->tab);
                            CFG_Node *dst = cfg_node_deep_copy(ti_state->cfg, src);
                            cfg_node_insert_child(ti_state->cfg, src->parent, src, dst);
                            ti_cmd(TI_CmdKind_FocusTab, .tab = dst->id);
                        }
                    } break;
                    case TI_CmdKind_CopyTabFullPath: {
                        {
                            CFG_Node *tab = cfg_node_from_id(ti_regs()->tab);
                            String8 expr = s("test expr");
                            String8 full_path = s("test expr full path");
                            wm_set_clipboard_text(full_path);
                        }
                    } break;
                    case TI_CmdKind_CloseTab: {
                        {
                            CFG_Node *tab = cfg_node_from_id(ti_regs()->tab);
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
                                    else if (!ti_cfg_is_project_filtered(n->v))
                                    {
                                        next_selected_tab = n->v;
                                        if (found_selected)
                                        {
                                            break;
                                        }
                                    }
                                }
                                ti_cmd(TI_CmdKind_FocusTab, .tab = next_selected_tab->id);
                            }
                            cfg_node_release(ti_state->cfg, tab);
                        }
                    } break;
                    case TI_CmdKind_MoveView: {
                        {
                            CFG_Node *view = cfg_node_from_id(ti_regs()->view);
                            CFG_Node *prev_tab = cfg_node_from_id(ti_regs()->prev_tab);
                            CFG_Node *src_panel = view->parent;
                            CFG_Node *dst_panel = cfg_node_from_id(ti_regs()->dst_panel);
                            if(dst_panel != &cfg_nil_node && prev_tab != view)
                            {
                                cfg_node_unhook(ti_state->cfg, src_panel, view);
                                cfg_node_insert_child(ti_state->cfg, dst_panel, prev_tab, view);
                                ti_cmd(TI_CmdKind_FocusTab, .panel = dst_panel->id, .tab = view->id);
                                ti_cmd(TI_CmdKind_FocusPanel, .panel = dst_panel->id);
                                CFG_Panel_Tree src_panel_tree = cfg_panel_tree_from_cfg(scratch.arena, src_panel);
                                CFG_Panel_Node *src_panel_node = cfg_panel_node_from_tree_cfg(src_panel_tree.root, src_panel);
                                bool32 src_panel_is_empty = 0;
                                if(src_panel != dst_panel)
                                {
                                    src_panel_is_empty = 1;
                                    for(CFG_Node_Ptr_Node *n = src_panel_node->tabs.first; n != 0; n = n->next)
                                    {
                                        if(!ti_cfg_is_project_filtered(n->v))
                                        {
                                            ti_cmd(TI_CmdKind_FocusTab, .panel = src_panel->id, .tab = n->v->id);
                                            src_panel_is_empty = 0;
                                            break;
                                        }
                                    }
                                }
                                if(src_panel_is_empty)
                                {
                                    ti_cmd(TI_CmdKind_ClosePanel, .panel = src_panel->id);
                                }
                            }
                        }
                    } break;
                    case TI_CmdKind_TabBarTop: {
                        {
                            CFG_Node *panel = cfg_node_from_id(ti_regs()->panel);
                            cfg_node_release(ti_state->cfg, cfg_node_child_from_string(panel, str8_lit("tabs_on_bottom")));
                        }
                    } break;
                    case TI_CmdKind_TabBarBottom: {
                        {
                            CFG_Node *panel = cfg_node_from_id(ti_regs()->panel);
                            cfg_node_child_from_string_or_alloc(ti_state->cfg, panel, str8_lit("tabs_on_bottom"));
                        }
                    } break;
                    case TI_CmdKind_TabSettings: {
                        {
                            String8 expr = str8f(scratch.arena, "query:config.$%I64x", ti_regs()->tab);
                            ti_cmd(TI_CmdKind_CancelAllQueries);
                            ti_cmd(TI_CmdKind_PushQuery, .expr = expr, .do_implicit_root = 1, .do_big_rows = 1, .do_lister = 1);
                        }
                    } break;
                    case TI_CmdKind_ResetToDefaultPanels: {
                        {
                            CFG_Node *window = cfg_node_from_id(ti_regs()->window);
                            CFG_Node *panels = cfg_node_child_from_string(window, str8_lit("panels"));
                            CFG_Panel_Tree panel_tree = cfg_panel_tree_from_cfg(scratch.arena, window);

                            // defined all of the "fixed" tabs we care about
#define X(name) CFG_Node *name = &cfg_nil_node;
#define Y(name, rule, expr) CFG_Node *name = &cfg_nil_node;
#define Z(name) CFG_Node *name = &cfg_nil_node;
                            TI_FixedTabXList
#undef X
#undef Y
#undef Z
                            // find all the fixed tabs, and all text viewers
                            bool32 any_fixed_tabs_found = 0;
                            CFG_Node_Ptr_List texts = {0};
                            for (CFG_Panel_Node *panel = panel_tree.root;
                                 panel != &cfg_nil_panel_node;
                                 panel = cfg_panel_node_rec__depth_first_pre(panel_tree.root, panel).next)
                            {
                                for (CFG_Node_Ptr_Node *n = panel->tabs.first; n != 0; n = n->next)
                                {
                                    CFG_Node *tab = n->v;
                                    bool32 need_unhook = true;
                                    if (0){}
                                    // @here
                                }
                            }
                        }
                    } break;
                        // queries
                    case TI_CmdKind_PushQuery: {
                        {
                            String8 cmd_name = ti_regs()->cmd_name;
                            TI_Cmd_Kind_Info *cmd_kind_info = ti_cmd_kind_info_from_string(cmd_name);

                            // close existing context menus
                            {
                                CFG_Node *window = cfg_node_from_id(ti_regs()->window);
                                TI_Window_State *ws = ti_window_state_from_cfg(window);
                                ui_ctx_menu_close();
                                ws->menu_bar_focused = 0;
                            }

                            // floating queries -> set up window to build immediate-mode top-level query
                            CFG_Node *view = &cfg_nil_node;
                            bool32 is_floating = (cmd_name.size == 0 || cmd_kind_info->query.flags & TI_QueryFlag_Floating);
                            if (is_floating)
                            {
                                CFG_Node *window = cfg_node_from_id(ti_regs()->window);
                                TI_Window_State *ws = ti_window_state_from_cfg(window);
                                if (ws != &ti_nil_window_state)
                                {
                                    // push onto window's query stack
                                    u64 pre_query_arena_pos = arena_pos(ws->query_arena);
                                    TI_Query_View *query_view = push_array(ws->query_arena, TI_Query_View, 1);
                                    SLLStackPush(ws->query_top, query_view);
                                    query_view->q_arena_pos = pre_query_arena_pos;
                                    query_view->regs = ti_regs_copy(ws->query_arena, ti_regs());

                                    // init cfg tree for query view
                                    CFG_Node *window_query = ti_immediate_cfg_from_keyf("window_query_%p_%I64u", window, query_view->q_arena_pos);
                                    cfg_node_release_all_children(ti_state->cfg, window_query);
                                    view = cfg_node_child_from_string_or_alloc(ti_state->cfg, window_query, s("watch"));
                                    CFG_Node *expr = cfg_node_child_from_string_or_alloc(ti_state->cfg, view, s("expression"));
                                    cfg_node_new_replace(ti_state->cfg, expr, ti_regs()->expr);
                                    CFG_Node *ctx_expr = cfg_node_child_from_string_or_alloc(ti_state->cfg, view, s("context_expression"));
                                    cfg_node_new_replace(ti_state->cfg, ctx_expr, ti_regs()->ctx_expr);
                                }
                            }

                            // non-floating -> embed in view
                            else
                            {
                                view = cfg_node_from_id(ti_regs()->view);
                            }

                            // determine if the target view is a lister (and thus already has a command)
                            bool32 view_is_lister = (cfg_node_child_from_string(view, s("lister")) != &cfg_nil_node);

                            // target view is a lister -> do not do anything - cannot replace the command
                            if (!view_is_lister)
                            {
                                // unpack view's query info
                                CFG_Node *query = cfg_node_child_from_string_or_alloc(ti_state->cfg, view, s("query"));
                                CFG_Node *cmd = cfg_node_child_from_string_or_alloc(ti_state->cfg, query, s("cmd"));
                                CFG_Node *input = cfg_node_child_from_string_or_alloc(ti_state->cfg, query, s("input"));
                                if (is_floating)
                                {
#define Opt(cnd, key) if(!(cnd)) { cfg_node_release(ti_state->cfg, cfg_node_child_from_string(view, s(key))); } else { cfg_node_child_from_string_or_alloc(ti_state->cfg, view, s(key)); }
                Opt(!ti_regs()->do_implicit_root, "explicit_root");
                Opt(ti_regs()->do_lister, "lister");
                Opt(ti_regs()->small_size, "small");
                Opt(ti_regs()->activate_with_single_click, "activate_with_single_click");
                Opt(ti_regs()->prefer_new_tab, "prefer_new_tab");
                Opt(ti_regs()->create_new, "create_new");
#undef Opt
                                }

                                // choose initial input string
                                String8 initial_input = {0};
                                if (cmd_name.size != 0)
                                {
                                    if (cmd_kind_info->query.slot = TI_RegSlot_FilePath)
                                    {
                                        CFG_Node *user = cfg_node_child_from_string(cfg_node_root(), str8_lit("user"));
                                        CFG_Node *current_path = cfg_node_child_from_string(user, str8_lit("current_path"));
                                        String8 current_path_string = current_path->first->string;
                                        if (current_path_string.size == 0)
                                        {
                                            current_path_string = path_normalised_from_string(scratch.arena, get_current_path(scratch.arena));
                                        }
                                        initial_input = current_path_string;
                                        initial_input = push_str8f(scratch.arena, "%S/", initial_input);
                                    }
                                    else if (cmd_kind_info->query.flags & TI_QueryFlag_KeepOldInput)
                                    {
                                        initial_input = input->first->string;
                                    }
                                }

                                // build query state
                                String8 current_query_cmd_name = cmd->first->string;
                                cfg_node_new_replace(ti_state->cfg, input, initial_input);
                                cfg_node_new_replace(ti_state->cfg, cmd, cmd_name);
                                TI_View_State *vs = ti_view_state_from_cfg(view);
                                if (cmd_name.size != 0)
                                {
                                    if (!vs->query_is_open && cmd_kind_info->query.flags & TI_QueryFlag_SelectOldInput)
                                    {
                                        vs->query_cursor = input->first->string.size;
                                        vs->query_mark = 0;
                                    }
                                    else
                                    {
                                        vs->query_cursor = input->first->string.size;
                                        vs->query_mark = vs->query_cursor;
                                    }
                                    if (!str8_match(current_query_cmd_name, cmd_name, 0))
                                    {
                                        vs->query_is_open = 1;
                                    }
                                    else
                                    {
                                        vs->query_is_open ^= 1;
                                    }
                                }
                                if (ti_regs()->do_lister)
                                {
                                    vs->query_is_open = 1;
                                }
                                vs->contents_are_focused = 0;
                            }
                        }
                    } break;
                    case TI_CmdKind_CompleteQuery: {
                        {
                            // unpack params
                            CFG_Node *window = cfg_node_from_id(ti_regs()->window);
                            TI_Window_State *ws = ti_window_state_from_cfg(window);
                            CFG_Node *view = cfg_node_from_id(ti_regs()->view);
                            String8 cmd_name = ti_view_query_cmd();
                            TI_Query_View *qv = ws->query_top;

                            // find out if this view is a lister
                            bool32 is_lister = (cfg_node_child_from_string(view, s("lister")) != &cfg_nil_node);

                            // push command
                            if (cmd_name.size != 0) TI_RegsScope()
                            {
                                if (is_lister && qv)
                                {
                                    ti_regs()->view = qv->regs->view;
                                }
                                ti_push_cmd(cmd_name, ti_regs());
                            }

                            // complete query, either by closing the query popup, or closing the
                            // tab-embedded query edit
                            TI_Cmd_Kind_Info *cmd_kind_info = ti_cmd_kind_info_from_string(cmd_name);
                            if (is_lister && qv && ws != &ti_nil_window_state)
                            {
                                u64 pop_pos = ws->query_top->q_arena_pos;
                                SLLStackPop(ws->query_top);
                                arena_pop_to(ws->query_arena, pop_pos);
                            }
                            else if (!(cmd_kind_info->query.flags & TI_QueryFlag_KeepOldInput))
                            {
                                TI_View_State *vs = ti_view_state_from_cfg(view);
                                vs->query_is_open = 0;
                                vs->query_string_size = 0;
                            }
                        }
                    } break;
                    case TI_CmdKind_CancelQuery: {
                        {
                            CFG_Node *window = cfg_node_from_id(ti_regs()->window);
                            TI_Window_State *ws = ti_window_state_from_cfg(window);
                            if (ws != &ti_nil_window_state && ws->query_top != 0)
                            {
                                u64 pop_pos = ws->query_top->q_arena_pos;
                                SLLStackPop(ws->query_top);
                                arena_pop_to(ws->query_arena, pop_pos);
                            }
                        }
                    } break;
                    case TI_CmdKind_CancelAllQueries: {
                        {
                            CFG_Node *window = cfg_node_from_id(ti_regs()->window);
                            TI_Window_State *ws = ti_window_state_from_cfg(window);
                            if (ws != &ti_nil_window_state)
                            {
                                arena_clear(ws->query_arena);
                                ws->query_top = 0;
                            }
                        }
                    } break;
                    case TI_CmdKind_UpdateQuery: {
                        {
                            CFG_Node *view = cfg_node_from_id(ti_regs()->view);
                            CFG_Node *query = cfg_node_child_from_string_or_alloc(ti_state->cfg, view, str8_lit("query"));
                            CFG_Node *input = cfg_node_child_from_string_or_alloc(ti_state->cfg, query, str8_lit("input"));
                            cfg_node_new_replace(ti_state->cfg, input, ti_regs()->string);
                            TI_View_State *vs = ti_view_state_from_cfg(view);
                            vs->query_string_size = Min(sizeof(vs->query_buffer), ti_regs()->string.size);
                            vs->query_cursor = vs->query_mark = vs->query_string_size;
                            MemoryCopy(vs->query_buffer, ti_regs()->string.str, vs->query_string_size);
                        }
                    } break;
                    case TI_CmdKind_WMEvent: {
                        {
                            WM_Event *wm_event = ti_regs()->wm_event;
                            TI_Window_State *ws = ti_window_state_from_os_handle(wm_event->window);
                            if (wm_event != 0 && ws != &ti_nil_window_state)
                            {
                                UI_Event ui_event = zero_struct;
                                UI_EventKind kind = UI_EventKind_Null;
                                {
                                    switch (wm_event->kind)
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
                }
            }
        }

        //////////////
        // tick measurement engine
        // TODO
        u64 cmd_count_pre_tick = ti_state->cmds[0].count;

        //////////////////////////
        // process measurement engine events
        // TODO

        
        /////////////////////////
        // early out if no new commands
        //
        if (ti_state->cmds[0].count == cmd_count_pre_tick)
        {
            break;
        }
    }

    ///////////////////////
    // get fonts from the config
    //
    ProfScope("get fonts from the config")
    {
        /*
        bool32 use_alternative_font_for_ui = ti_setting_bool32_from_name(s("use_alternative_font_for_ui"));
        if (use_alternative_font_for_ui)
        {
            String8 main_font_name = ti_setting_from_name(s("main_font"));
            String8 code_font_name = ti_setting_from_name(s("code_font"));
            ti_state->font_slot_table[TI_FontSlot_Main]  = fnt_tag_from_path(main_font_name);
            ti_state->font_slot_table[TI_FontSlot_Code]  = fnt_tag_from_path(code_font_name);
            if (fnt_tag_match(ti_state->font_slot_table[TI_FontSlot_Main], fnt_tag_zero()))
            {
                ti_state->font_slot_table[TI_FontSlot_Main] = fnt_tag_from_static_data_string(&ti_default_main_font_bytes);
            }
        }
        else
        {
            String8 font_name = ti_setting_from_name(s("font"));
            ti_state->font_slot_table[TI_FontSlot_Code] = fnt_tag_from_path(font_name);
            ti_state->font_slot_table[TI_FontSlot_Main] = ti_state->font_slot_table[TI_FontSlot_Code];
            if (fnt_tag_match(ti_state->font_slot_table[TI_FontSlot_Main], fnt_tag_zero()))
            {
                ti_state->font_slot_table[TI_FontSlot_Main] = fnt_tag_from_static_data_string(&ti_default_code_font_bytes);
            }
        }
        if (fnt_tag_match(ti_state->font_slot_table[TI_FontSlot_Code], fnt_tag_zero()))
        {
            ti_state->font_slot_table[TI_FontSlot_Code] = fnt_tag_from_static_data_string(&ti_default_code_font_bytes);
        }
        */
        ti_state->font_slot_table[TI_FontSlot_Main]  = fnt_tag_from_static_data_string(&ti_default_main_font_bytes);
        ti_state->font_slot_table[TI_FontSlot_Code]  = fnt_tag_from_static_data_string(&ti_default_code_font_bytes);
        ti_state->font_slot_table[TI_FontSlot_Icons] = fnt_tag_from_static_data_string(&ti_icon_font_bytes);
    }

    //////////////////////////
    // update window titles
    if (ti_state->frame_depth == 1)
    {
        Temp scratch = scratch_begin(0, 0);
        String8 window_title = ti_push_window_title(scratch.arena);
        if (!str8_match(window_title, ti_state->last_window_title, 0))
        {
            for (TI_Window_State *ws = ti_state->first_window_state; ws != &ti_nil_window_state; ws = ws->order_next)
            {
                wm_window_set_title(ws->os, window_title);
            }
        }
        ti_state->last_window_title = str8_copy(ti_frame_arena(), window_title);
        scratch_end(scratch);
    }

    ////////////////////////////
    //- rjf: rotate command slots, bump command gen counter
    //
    // in this step, we rotate the ring buffer of command batches (command
    // arenas & lists). when the cmds_gen (the position of the ring buffer)
    // is even, the command queue is in a "read/write" mode, and this is uniquely
    // usable by the core - this is done so that commands in the core can push
    // other commands, and have those other commands processed on the same frame.
    //
    // in view code, however, they can only use the current command queue in a
    // "read only" mode, because new commands pushed by those views must be
    // processed first by the core. so, before calling into view code, the
    // cmds_gen is incremented to be *odd*. this way, the views will *write*
    // commands into the 0 slot, but *read* from the 1 slot (which will contain
    // this frame's commands).
    //
    // after view code runs, the generation number is incremented back to even.
    // the commands pushed by the view will be in the queue, and the core can
    // treat that queue as r/w again.
    //
    if (ti_state->frame_depth == 1)
    {
        // rotate
        {
            Arena *first_arena = ti_state->cmds_arenas[0];
            TI_Cmd_List first_cmds = ti_state->cmds[0];
            MemoryCopy(ti_state->cmds_arenas,
                       ti_state->cmds_arenas + 1,
                       sizeof(ti_state->cmds_arenas[0]) * (ArrayCount(ti_state->cmds_arenas) - 1));
            MemoryCopy(ti_state->cmds,
                       ti_state->cmds + 1,
                       sizeof(ti_state->cmds[0]) * (ArrayCount(ti_state->cmds) - 1));
            ti_state->cmds_arenas[ArrayCount(ti_state->cmds_arenas) - 1] = first_arena;
            ti_state->cmds[ArrayCount(ti_state->cmds_arenas) - 1] = first_cmds;
        }

        // clear next batch
        {
            arena_clear(ti_state->cmds_arenas[0]);
            MemoryZeroStruct(&ti_state->cmds[0]);
        }

        // bump
        {
            ti_state->cmds_gen += 1;
        }
    }

    ////////////////////////////////////
    // compute amimation rates, given config
    //
    {
        f32 master_animations_f    = 1.f;
        f32 scrolling_animations_f = 1.f;
        f32 tooltip_animations_f   = 1.f;
        f32 menu_animations_f      = 1.f;

        ti_state->catchall_animation_rate     = 1 - master_animations_f*pow_f32(2, (-60.f * ti_state->frame_dt));
        ti_state->menu_animation_rate         = 1 - master_animations_f*menu_animations_f*pow_f32(2, (-70.f * ti_state->frame_dt));
        ti_state->menu_animation_rate__slow   = 1 - master_animations_f*menu_animations_f*pow_f32(2, (-50.f * ti_state->frame_dt));
        ti_state->entity_alive_animation_rate = 1 - master_animations_f*menu_animations_f*pow_f32(2, (-30.f * ti_state->frame_dt));
        ti_state->rich_hover_animation_rate   = 1 - master_animations_f*menu_animations_f*pow_f32(2, (-50.f * ti_state->frame_dt));
        ti_state->scrolling_animation_rate    = 1 - master_animations_f*scrolling_animations_f*pow_f32(2, (-60.f * ti_state->frame_dt));
        ti_state->tooltip_animation_rate      = 1 - master_animations_f*tooltip_animations_f*pow_f32(2, (-60.f * ti_state->frame_dt));
    }

    ///////////////////////////////
    // update/render all windows
    {
        dr_begin_frame(fnt_tag_from_static_data_string(&ti_default_main_font_bytes));
        CFG_Node_Ptr_List windows = cfg_node_top_level_list_from_string(scratch.arena, str8_lit("window"));
        for (CFG_Node_Ptr_Node *n = windows.first; n != 0; n = n->next)
        {
            CFG_Node *window = n->v;
            TI_Window_State *w = ti_window_state_from_cfg(window);
            bool32 window_is_focused = wm_window_is_focused(w->os);
            if (window_is_focused)
            {
                ti_state->last_focused_window = w->cfg_id;
            }
            ti_push_regs();
            ti_regs()->window = w->cfg_id;
            ti_window_frame();
            MemoryZeroStruct(&w->ui_events);
            TI_Regs *window_regs = ti_pop_regs();
            if (ti_state->last_focused_window == w->cfg_id)
            {
                MemoryCopyStruct(ti_regs(), window_regs);
            }
        }
    }

    /////////////////////////////////
    // garbage collect untouched window states
    //
    {
        for EachIndex(slot_idx, ti_state->window_state_slots_count)
        {
            for (TI_Window_State *ws = ti_state->window_state_slots[slot_idx].first, *next = 0; ws != 0; ws = next)
            {
                next = ws->hash_next;
                CFG_Node *cfg = cfg_node_from_id(ws->cfg_id);
                if (cfg == &cfg_nil_node || ws->last_frame_index_touched < ti_state->frame_index || ti_state->quit)
                {
                    ui_state_release(ws->ui);
                    r_window_unequip(ws->os, ws->r);
                    wm_window_close(ws->os);
                    arena_release(ws->arena);
                    DLLRemove_NPZ(&ti_nil_window_state, ti_state->first_window_state, ti_state->last_window_state, ws, order_next, order_prev);
                    DLLRemove_NP(ti_state->window_state_slots[slot_idx].first, ti_state->window_state_slots[slot_idx].last, ws, hash_next, hash_prev);
                    SLLStackPush_N(ti_state->free_window_state, ws, order_next);
                }
            }
        }
    }

    ///////////////////
    // simulate lag
    //
    if (DEV_simulate_lag)
    {
        sleep_ms(300);
    }

    //////////////////////////////
    // clear frame request state
    if (ti_state->num_frames_requested > 0)
    {
        ti_state->num_frames_requested -= 1;
    }

    //////////////////////////
    // close frame scopes
    //
    // NOTE: this always must happen before the refresh, since that
    // will sleep for vsync, and we do not want to hold handles for long,
    // since eviction threads may be wariing to get rid of stuff.
    //
    access_close(ti_state->frame_access);
    ti_state->frame_access = frame_access_restore;

    /////////////////////////////
    // submit rendering to all windows
    ProfScope("submit rendering to all windows")
    {
        r_begin_frame();
        for (TI_Window_State *w = ti_state->first_window_state; w != &ti_nil_window_state; w = w->order_next)
        {
            r_window_begin_frame(w->os, w->r);
            dr_submit_bucket(w->os, w->r, w->draw_bucket);
            r_window_end_frame(w->os, w->r);
        }
        r_end_frame();
    }

    //////////////////////////
    // show windows after first frame
    if (ti_state->frame_depth == 1)
    {
        CFG_ID_List windows_to_show = {0};
        for (TI_Window_State *w = ti_state->first_window_state; w != &ti_nil_window_state; w = w->order_next)
        {
            if (w->frames_alive == 1)
            {
                cfg_id_list_push(scratch.arena, &windows_to_show, w->cfg_id);
            }
        }
        for (CFG_ID_Node *n = windows_to_show.first; n != 0; n = n->next)
        {
            CFG_Node *window = cfg_node_from_id(n->v);
            TI_Window_State *ws = ti_window_state_from_cfg(window);
            wm_window_first_paint(ws->os);
        }
    }

    //////////////////////////////////////
    // determine frame time, record into history
    u64 end_time_us = now_time_us();
    u64 frame_time_us = end_time_us - begin_time_us;
    ti_state->frame_time_us_history[ti_state->frame_index % ArrayCount(ti_state->frame_time_us_history)] = frame_time_us;

    ////////////////////////////
    // [windows] clear pages from working set shortly after startup, many of which will not be needed
#if OS_WINDOWS
    if (ti_state->frame_index == 15) ProfScope("SetProcessWorkingSetSize")
    {
        SetProcessWorkingSetSize(GetCurrentProcess(), max_u64, max_u64);
    }
#endif

    ///////////////////////////
    // bump frame time counters
    //
    ti_state->frame_index += 1;
    ti_state->time_in_seconds += ti_state->frame_dt;
    ti_state->time_in_us += frame_time_us;

    //////////////////////////
    // bump command batch ring buffer generation
    //
    if (ti_state->frame_depth == 1)
    {
        ti_state->cmds_gen += 1;
    }

    /////////////////
    // collect logs
    ProfScope("collect logs")
    {
        Log_Scope_Result log = log_scope_end(scratch.arena);
        append_data_to_file_path(ti_state->log_path, log.strings[LogMsgKind_Info]);
        if (log.strings[LogMsgKind_UserError].size != 0)
        {
            String8 error_log = log.strings[LogMsgKind_UserError];
            String8_List error_log_lines = str8_split(scratch.arena, error_log, (u8 *)"\n", 1, 0);
            String8 error_log_string = str8_list_join(scratch.arena,
                                                      &error_log_lines,
                                                      &(String_Join){.sep = str8_lit(" ")});
            for (TI_Window_State *ws = ti_state->first_window_state; ws != &ti_nil_window_state; ws = ws->order_next)
            {
                ws->error_string_size = Min(sizeof(ws->error_buffer), error_log_string.size);
                MemoryCopy(ws->error_buffer, error_log_string.str, ws->error_string_size);
                ws->error_t = 1.f;
            }
        }
    }

    ti_state->frame_depth -= 1;
    scratch_end(scratch);
    ProfEnd();
}
