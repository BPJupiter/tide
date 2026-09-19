// Copyright (c) Frances Telfar
// Licensed under the PolyForm Noncommercial License (https://polyformproject.org/licenses/noncommercial/1.0.0)

#ifndef TIDE_CORE_H
#define TIDE_CORE_H

///////////////////
// View UI Hook Types

#define TI_VIEW_UI_FUNCTION_SIG(name) void name(Rng2f32 rect)
#define TI_VIEW_UI_FUNCTION_NAME(name) ti_view_ui__##name
#define TI_VIEW_UI_FUNCTION_DEF(name) internal TI_VIEW_UI_FUNCTION_SIG(TI_VIEW_UI_FUNCTION_NAME(name))
typedef TI_VIEW_UI_FUNCTION_SIG(TI_View_UI_Function_Type);

typedef struct TI_View_UI_Rule TI_View_UI_Rule;
struct TI_View_UI_Rule
{
    String8 name;
    TI_View_UI_Function_Type *ui;
};

typedef struct TI_View_UI_Rule_Node TI_View_UI_Rule_Node;
struct TI_View_UI_Rule_Node
{
    TI_View_UI_Rule_Node *next;
    TI_View_UI_Rule v;
};

typedef struct TI_View_UI_Rule_Slot TI_View_UI_Rule_Slot;
struct TI_View_UI_Rule_Slot
{
    TI_View_UI_Rule_Node *first;
    TI_View_UI_Rule_Node *last;
};

typedef struct TI_View_UI_Rule_Map TI_View_UI_Rule_Map;
struct TI_View_UI_Rule_Map
{
    TI_View_UI_Rule_Slot *slots;
    u64 slots_count;
};

/////////////////////
// Drag/Drop Types
//
typedef enum TI_DragDropState {
    TI_DragDropState_Null,
    TI_DragDropState_Dragging,
    TI_DragDropState_Dropping,
    TI_DragDropState_COUNT
} TI_DragDropState;

////////////////////////
// Command Kind Types

typedef u32 TI_QueryFlags;
enum {
    TI_QueryFlag_AllowFiles     = (1 << 0),
    TI_QueryFlag_AllowFolders   = (1 << 1),
    TI_QueryFlag_CodeInput      = (1 << 2),
    TI_QueryFlag_KeepOldInput   = (1 << 3),
    TI_QueryFlag_SelectOldInput = (1 << 4),
    TI_QueryFlag_Floating       = (1 << 5),
    TI_QueryFlag_Required       = (1 << 6),
};

typedef u32 TI_CmdKindFlags;
enum {
    TI_CmdKindFlag_ListInUI            = (1 << 0),
};

////////////////////
// Autocompletion Cursor Info Type

typedef struct TI_Autocomp_Cursor_Info TI_Autocomp_Cursor_Info;
struct TI_Autocomp_Cursor_Info
{
    String8 list_expr;
    String8 filter;
    Rng1u64 replaced_range;
    String8 callee_expr;
    MD_Node *arg_schema;
};

///////////////////
// Generated Code

#include "generated/tide.meta.h"

////////////////
// View State Types

typedef struct TI_Arena_Ext TI_Arena_Ext;
struct TI_Arena_Ext
{
    TI_Arena_Ext *next;
    Arena *arena;
};

typedef struct TI_View_State TI_View_State;
struct TI_View_State
{
    // hash links & key
    TI_View_State *hash_next;
    TI_View_State *hash_prev;
    CFG_ID cfg_id;

    // touch info
    u64 last_frame_index_touched;
    u64 last_frame_index_built;

    // loading indicator info
    f32 loading_t;
    f32 loading_t_target;
    u64 loading_progress_v;
    u64 loading_progress_v_target;

    // scroll position
    UI_Scroll_Pt2 scroll_pos;

    // view-lifetime allocation & user data extensions
    Arena *arena;
    u64 arena_reset_pos;
    TI_Arena_Ext *first_arena_ext;
    TI_Arena_Ext *last_arena_ext;
    void *user_data;

    // query state
    bool32 query_is_open;
    u64 query_cursor;
    u64 query_mark;
    u8 query_buffer[Kilobytes(1)];
    u64 query_string_size;

    // contents are focused (disables query focus)
    bool32 contents_are_focused;
};

typedef struct TI_View_State_Slot TI_View_State_Slot;
struct TI_View_State_Slot
{
    TI_View_State *first;
    TI_View_State *last;
};

///////////////////
// Vocabulary Map

typedef struct TI_Vocab_Info_Map_Node TI_Vocab_Info_Map_Node;
struct TI_Vocab_Info_Map_Node
{
    TI_Vocab_Info_Map_Node *single_next;
    TI_Vocab_Info_Map_Node *plural_next;
    TI_Vocab_Info v;
};

typedef struct TI_Vocab_Info_Map_Slot TI_Vocab_Info_Map_Slot;
struct TI_Vocab_Info_Map_Slot
{
    TI_Vocab_Info_Map_Node *first;
    TI_Vocab_Info_Map_Node *last;
};

typedef struct TI_Vocab_Info_Map TI_Vocab_Info_Map;
struct TI_Vocab_Info_Map
{
    u64 single_slots_count;
    TI_Vocab_Info_Map_Slot *single_slots;
    u64 plural_slots_count;
    TI_Vocab_Info_Map_Slot *plural_slots;
};

///////////////////
// Command Types

typedef struct TI_Cmd TI_Cmd;
struct TI_Cmd {
    String8 name;
    TI_Regs *regs;
};

typedef struct TI_Cmd_Node TI_Cmd_Node;
struct TI_Cmd_Node {
    TI_Cmd_Node *next;
    TI_Cmd_Node *prev;
    TI_Cmd cmd;
};

typedef struct TI_Cmd_List TI_Cmd_List;
struct TI_Cmd_List {
    TI_Cmd_Node *first;
    TI_Cmd_Node *last;
    u64 count;
};

///////////////////////
// Context Register Types

typedef struct TI_Regs_Node TI_Regs_Node;
struct TI_Regs_Node {
    TI_Regs_Node *next;
    TI_Regs v;
};

//////////////////////////////////
// Structured Theme Types, Parsed From Config

typedef enum TI_FontSlot
{
    TI_FontSlot_Main,
    TI_FontSlot_Code,
    TI_FontSlot_Icons,
    TI_FontSlot_COUNT,
} TI_FontSlot;

/////////////////////
// Per-Window State

typedef struct TI_Drop_Completion_Task TI_Drop_Completion_Task;
struct TI_Drop_Completion_Task
{
    TI_Drop_Completion_Task *next;
    String8_List paths;
};

typedef struct TI_Query_View TI_Query_View;
struct TI_Query_View
{
    TI_Query_View *next;
    TI_Regs *regs;
    u64 q_arena_pos;
};

typedef struct TI_Window_State TI_Window_State;
struct TI_Window_State {
    // links & metadata
    TI_Window_State *order_next;
    TI_Window_State *order_prev;
    TI_Window_State *hash_next;
    TI_Window_State *hash_prev;
    CFG_ID cfg_id;
    u64 frames_alive;
    u64 last_frame_index_touched;
    
    // top-level info & handles
    Arena *arena;
    WM_Window os;
    R_Handle r;
    UI_State *ui;
    f32 last_dpi;
    bool32 window_layout_reset;
    Rng2f32 last_window_rect;

    // theme (recomputed each frame)
    UI_Theme *theme;
    Vec4f32 theme_code_colors[TI_CodeColorSlot_COUNT];

    // font raster flags (recomputed each frame)
    FNT_RasterFlags font_slot_raster_flags[TI_FontSlot_COUNT];

    // dev interface state
    bool32 dev_menu_is_open;

    // menu bar state
    bool32 menu_bar_focused;
    bool32 menu_bar_focused_on_press;
    bool32 menu_bar_key_held;
    bool32 menu_bar_focus_press_started;

    // drop-completion state
    Arena *drop_completion_arena;
    CFG_ID drop_completion_panel;
    TI_Drop_Completion_Task *top_drop_completion_task;

    // query stack state
    Arena *query_arena;
    CFG_ID query_last_bottom_view_id;
    TI_Query_View *query_top;

    // hover eval state

    // autocompletion state
    u64 autocomp_last_frame_index;
    Arena *autocomp_arena;
    TI_Regs *autocomp_regs;
    TI_Autocomp_Cursor_Info autocomp_cursor_info;

    // error state
    u8 error_buffer[512];
    u64 error_string_size;
    f32 error_t;

    // per-frame ui events state
    UI_Event_List ui_events;

    // per-frame drawing state
    DR_Bucket *draw_bucket;
};

typedef struct TI_Window_State_Slot TI_Window_State_Slot;
struct TI_Window_State_Slot {
    TI_Window_State *first;
    TI_Window_State *last;
};

////////////////////////////////////
// Main Per-Process Graphical State

typedef struct TI_Ambiguous_Path_Node TI_Ambiguous_Path_Node;
struct TI_Ambiguous_Path_Node
{
    TI_Ambiguous_Path_Node *next;
    String8 name;
    String8_List paths;
};

typedef struct TI_State TI_State;
struct TI_State {
    // basics
    Arena *arena;
    bool32 quit;
    s32 frame_depth;

    // installation setting state
    bool32 installed;

    // config bucket paths
    Arena *user_path_arena;
    String8 user_path;
    Arena *project_path_arena;
    String8 project_path;
    Arena *theme_path_arena;
    String8 theme_path;

    // unpacked settings (cached, because they need to be used
    // earlier than setting evaluation is legal in a frame)
    bool32 alt_menu_bar_enabled;

    // animation rates
    f32 catchall_animation_rate;
    f32 menu_animation_rate;
    f32 menu_animation_rate__slow;
    f32 entity_alive_animation_rate;
    f32 rich_hover_animation_rate;
    f32 scrolling_animation_rate;
    f32 tooltip_animation_rate;

    // default theme table
    MD_Node *theme_preset_trees[TI_ThemePreset_COUNT];

    // vocab table
    TI_Vocab_Info_Map vocab_info_map;

    // log
    Log *log;
    String8 log_path;

    // frame history info
    u64 frame_index;
    Arena *frame_arenas[2];
    u64 frame_time_us_history[64];
    u64 num_frames_requested;
    f64 time_in_seconds;
    u64 time_in_us;

    // frame parameters
    f32 frame_dt;
    Access *frame_access;
    String8 last_window_title;

    // ambiguous path table (constructed from-scratch each frame)
    u64 ambiguous_path_slots_count;
    TI_Ambiguous_Path_Node **ambiguous_path_slots;

    // key map (constructed from-scratch each frame)
    CFG_Key_Map *key_map;

    // slot -> font tag map (constructed from-scratch each frame)
    FNT_Tag font_slot_table[TI_FontSlot_COUNT];

    // name -> view ui map (constructed from-scratch each frame)
    TI_View_UI_Rule_Map *view_ui_rule_map;

    // Registers stack
    TI_Regs_Node base_regs;
    TI_Regs_Node *top_regs;

    // autosave state
    f32 seconds_until_autosave;

    // commands
    Arena *cmds_arenas[2];
    TI_Cmd_List cmds[2];
    u64 cmds_gen;
    Arena *cmd_output_arena;
    String8_List cmd_outputs;

    // popup state
    UI_Key popup_key;
    bool32 popup_active;
    f32 popup_t;
    Arena *popup_arena;
    TI_Cmd_List popup_cmds;
    String8 popup_title;
    String8 popup_desc;

    // text editing mode state
    bool32 text_edit_mode;
    bool32 text_edit_mode_multiline;

    // contextual hover info
    TI_Regs *hover_regs;
    TI_RegSlot hover_regs_slot;
    TI_Regs *next_hover_regs;
    TI_RegSlot next_hover_regs_slot;

    // icon texture
    R_Handle icon_texture;

    // fixed ui keys
    UI_Key drop_completion_key;
    UI_Key ctx_menu_key;

    // drag/drop state
    Arena *drag_drop_arena;
    TI_Regs *drag_drop_regs;
    TI_RegSlot drag_drop_regs_slot;
    TI_DragDropState drag_drop_state;

    // cfg state
    CFG_State *cfg;
    CFG_Schema_Table *cfg_schema_table;

    // window state cache
    u64 window_state_slots_count;
    TI_Window_State_Slot *window_state_slots;
    TI_Window_State *free_window_state;
    CFG_ID last_focused_window;
    TI_Window_State *first_window_state;
    TI_Window_State *last_window_state;
    CFG_ID window_state_last_accessed_id;
    TI_Window_State *window_state_last_accessed;

    // view state cache
    u64 view_state_slots_count;
    TI_View_State_Slot *view_state_slots;
    TI_View_State *free_view_state;
    CFG_ID view_state_last_accessed_id;
    TI_View_State *view_state_last_accessed;

    // bind change
    Arena *bind_change_arena;
    bool32 bind_change_active;
    CFG_ID bind_change_binding_id;
    String8 bind_change_cmd_name;
};

//////////////
// Globals

read_only global TI_Vocab_Info ti_nil_vocab_info = {0};

read_only global TI_Cmd_Kind_Info ti_nil_cmd_kind_info = {0};

TI_VIEW_UI_FUNCTION_DEF(null);
read_only global TI_View_UI_Rule ti_nil_view_ui_rule = {
    {0},
    TI_VIEW_UI_FUNCTION_NAME(null),
};

read_only global TI_View_State ti_nil_view_state = {
    &ti_nil_view_state,
    &ti_nil_view_state,
};

read_only global TI_Window_State ti_nil_window_state = {
    &ti_nil_window_state,
    &ti_nil_window_state,
    &ti_nil_window_state,
    &ti_nil_window_state,
};

global TI_State *ti_state = 0;

// Dev flags
global bool32 DEV_simulate_lag        = false;
global bool32 DEV_always_refresh      = false;
global bool32 DEV_draw_ui_text_pos    = false;
global bool32 DEV_draw_ui_focus_debug = false;
global bool32 DEV_draw_ui_box_heatmap = false;

/////////////////////////////
// Registers Type Functions

internal void ti_regs_copy_contents(Arena *arena, TI_Regs *dst, TI_Regs *src);
internal TI_Regs *ti_regs_copy(Arena *arena, TI_Regs *src);

///////////////////////////
// Commands type functions

internal void ti_cmd_list_push_new(Arena *arena, TI_Cmd_List *cmds, String8 name, TI_Regs *regs);

/////////////////////
// View UI Rule Functions

internal TI_View_UI_Rule_Map *ti_view_ui_rule_map_make(Arena *arena, u64 slots_count);
internal void ti_view_ui_rule_map_insert(Arena *arena, TI_View_UI_Rule_Map *map, String8 string, TI_View_UI_Function_Type *ui);

internal TI_View_UI_Rule *ti_view_ui_rule_from_string(String8 string);

////////////////////
// Config Functions

internal bool32 ti_cfg_is_project_filtered(CFG_Node *cfg);

internal Vec4f32 ti_hsva_from_cfg(CFG_Node *cfg);
internal Vec4f32 ti_color_from_cfg(CFG_Node *cfg);

internal bool32 ti_disabled_from_cfg(CFG_Node *cfg);
internal String8 ti_name_from_cfg(CFG_Node *cfg);
internal String8 ti_label_from_cfg(CFG_Node *cfg);
internal String8 ti_expr_from_cfg(CFG_Node *cfg);
internal String8 ti_path_from_cfg(CFG_Node *cfg);

internal String8 ti_default_setting_from_names(String8 schema_name, String8 setting_name);
internal String8 ti_setting_from_name(String8 name);
internal bool32 ti_setting_bool32_from_name(String8 name);
internal u64 ti_setting_u64_from_name(String8 name);
internal f32 ti_setting_f32_from_name(String8 name);

internal CFG_Node *ti_immediate_cfg_from_key(String8 string);
internal CFG_Node *ti_immediate_cfg_from_keyf(char *fmt, ...);


////////////////////////
// Evaluation Visualization
//

// eval <-> file path
internal String8 ti_file_path_from_eval_string(Arena *arena, String8 string);


/////////////////
// View Functions

/*
internal CFG_Node *ti_view_from_string(CFG_Node *parent, String8 string);
*/
internal TI_View_State *ti_view_state_from_cfg(CFG_Node *cfg);
internal void ti_view_ui(Rng2f32 rect);

/////////////////
// View Building API

// view info extraction
internal Arena *ti_view_arena(void);
internal UI_Scroll_Pt2 ti_view_scroll_pos(void);
internal String8 ti_view_query_cmd(void);
internal String8 ti_view_query_input(void);
internal String8 ti_view_setting_from_name(String8 string);
internal bool32 ti_view_setting_bool32_from_name(String8 string);
internal u64 ti_view_setting_u64_from_name(String8 string);
internal f32 ti_view_setting_f32_from_name(String8 string);
internal u64 ti_view_setting_addr_from_name(String8 string);

// pushing/attaching view resources
internal void *ti_view_state_by_size(u64 size);
#define ti_view_state(T) (T *)ti_view_state_by_size(sizeof(T))
internal Arena *ti_push_view_arena(void);

// storing view-attached state
internal void ti_store_view_loading_info(bool32 is_loading, u64 progress_u64, u64 progress_u64_target);
internal void ti_store_view_scroll_pos(UI_Scroll_Pt2 pos);
internal void ti_store_view_param(String8 key, String8 value);
internal void ti_store_view_paramf(String8 key, char *fmt, ...);
#define ti_store_view_param_f32(key, f32) ti_store_view_paramf((key), "%ff", (f32))
#define ti_store_view_param_s64(key, s64) ti_store_view_paramf((key), "%I64d", (s64))
#define ti_store_view_param_u64(key, u64) ti_store_view_paramf((key), "0x%I64x", (u64))

///////////////////
// Window Functions

internal String8 ti_push_window_title(Arena *arena);
internal CFG_Node *ti_window_from_cfg(CFG_Node *cfg);
internal TI_Window_State *ti_window_state_from_cfg(CFG_Node *cfg);
internal TI_Window_State *ti_window_state_from_os_handle(WM_Window os);
internal void ti_window_frame(void);

///////////////////////
// Colors, Fonts, Config

// colors
internal MD_Node *ti_theme_tree_from_name(Arena *arena, Access *access, String8 theme_name);
internal Vec4f32 ti_rgba_from_code_color_slot(TI_CodeColorSlot slot);
internal TI_CodeColorSlot ti_code_color_slot_from_txt_token_kind(TXT_TokenKind kind);
internal TI_CodeColorSlot ti_code_color_slot_from_txt_token_kind_lookup_string(TXT_TokenKind kind, String8 string, bool32 allow_macros, bool32 is_called);

// fonts
internal f32 ti_font_size(void);
internal FNT_Tag ti_font_from_slot(TI_FontSlot slot);
internal FNT_RasterFlags ti_raster_flags_from_slot(TI_FontSlot slot);

////////////////////
// Vocab Info Lookups

internal TI_Vocab_Info *ti_vocab_info_from_code_name(String8 code_name);
internal TI_Vocab_Info *ti_vocab_info_from_code_name_plural(String8 code_name_plural);
#define ti_plural_from_code_name(code_name) (ti_vocab_info_from_code_name(code_name)->code_name_plural)
#define ti_display_from_code_name(code_name) (ti_vocab_info_from_code_name(code_name)->display_name)
#define ti_display_plural_from_code_name(code_name) (ti_vocab_info_from_code_name(code_name)->display_name_plural)
#define ti_icon_kind_from_code_name(code_name) (ti_vocab_info_from_code_name(code_name)->icon_kind)
#define ti_singular_from_code_name_plural(code_name_plural) (ti_vocab_info_from_code_name_plural(code_name_plural)->code_name)


////////////////////////////
// Continuous Frame Requests

internal void ti_request_frame(void);

///////////////////////
// Main State Accessors

// per-frame arena
internal Arena *ti_frame_arena(void);

////////////////
// Registers

#define ti_regs() (&ti_state->top_regs->v)
#define ti_base_regs() (&ti_state->base_regs.v)
internal TI_Regs *ti_push_regs_(TI_Regs *regs);
#define ti_push_regs(...) ti_push_regs_(&(TI_Regs){ti_regs_lit_init_top __VA_ARGS__})
internal TI_Regs *ti_pop_regs(void);
#define TI_RegsScope(...) DeferLoop(ti_push_regs(__VA_ARGS__), ti_pop_regs())
internal void ti_regs_fill_slot_from_string(TI_RegSlot slot, String8 query_expr, String8 string);

//////////////
// Commands

// name -> info
internal TI_CmdKind ti_cmd_kind_from_string(String8 string);
internal TI_Cmd_Kind_Info *ti_cmd_kind_info_from_string(String8 string);

// pushing
internal void ti_push_cmd(String8 name, TI_Regs *regs);
#define ti_cmd(kind, ...) ti_push_cmd(ti_cmd_kind_info_table[kind].string, &(TI_Regs){ti_regs_lit_init_top __VA_ARGS__})

// iterating
internal bool32 ti_next_cmd(TI_Cmd **cmd);
/*
internal bool32 ti_next_view_cmd(TI_Cmd **cmd);
*/

//////////////////////////////
// Main Layer Top-Level Calls

internal void ti_init(Cmd_Line *cmdline);
internal void ti_frame(void);

#endif // TIDE_CORE_H
