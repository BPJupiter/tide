
#ifndef BROKEN_PROXY_CORE_H
#define BROKEN_PROXY_CORE_H

////////////////////////
// View UI Hook Types

#define BP_VIEW_UI_FUNCTION_SIG(name) void name(CFG_Node *view, Rng2f32 rect)
#define BP_VIEW_UI_FUNCTION_NAME(name) bp_view_ui__##name
#define BP_VIEW_UI_FUNCTION_DEF(name) internal BP_VIEW_UI_FUNCTION_SIG(BP_VIEW_UI_FUNCTION_NAME(name))
typedef BP_VIEW_UI_FUNCTION_SIG(BP_View_UI_Function_Type);

typedef struct BP_View_UI_Rule BP_View_UI_Rule;
struct BP_View_UI_Rule {
    String8 name;
    BP_View_UI_Function_Type *ui;
};

typedef struct BP_View_UI_Rule_Node BP_View_UI_Rule_Node;
struct BP_View_UI_Rule_Node {
    BP_View_UI_Rule_Node *next;
    BP_View_UI_Rule v;
};

typedef struct BP_View_UI_Rule_Slot BP_View_UI_Rule_Slot;
struct BP_View_UI_Rule_Slot {
    BP_View_UI_Rule_Node *first;
    BP_View_UI_Rule_Node *last;
};

typedef struct BP_View_UI_Rule_Map BP_View_UI_Rule_Map;
struct BP_View_UI_Rule_Map {
    BP_View_UI_Rule_Slot *slots;
    u64 slots_count;
};

/////////////////////
// Drag/Drop Types

typedef enum BP_DragDropState {
    BP_DragDropState_Null,
    BP_DragDropState_Dragging,
    BP_DragDropState_Dropping,
    BP_DragDropState_COUNT
} BP_DragDropState;

////////////////////////
// Command Kind Types

typedef u32 BP_QueryFlags;
enum {
    BP_QueryFlag_AllowFiles     = (1 << 0),
    BP_QueryFlag_AllowFolders   = (1 << 1),
    BP_QueryFlag_CodeInput      = (1 << 2),
    BP_QueryFlag_KeepOldInput   = (1 << 3),
    BP_QueryFlag_SelectOldInput = (1 << 4),
    BP_QueryFlag_Floating       = (1 << 5),
    BP_QueryFlag_Required       = (1 << 6),
};

typedef u32 BP_CmdKindFlags;
enum {
    BP_CmdKindFlag_ListInUI = (1 << 0),
};

/////////////////////////////////////
// Autocompletion Cursor Info Type

typedef struct BP_Autocomp_Cursor_Info BP_Autocomp_Cursor_Info;
struct BP_Autocomp_Cursor_Info {
    String8 filter;
    Rng1u64 replaced_range;
};

////////////////////
// Generated Code

#include "generated/broken_proxy.meta.h"

//////////////////////
// View State Types

typedef struct BP_Arena_Ext BP_Arena_Ext;
struct BP_Arena_Ext {
    BP_Arena_Ext *next;
    Arena *arena;
};

typedef struct BP_View_State BP_View_State;
struct BP_View_State {
    // hash links & key
    BP_View_State *hash_next;
    BP_View_State *hash_prev;
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
    BP_Arena_Ext *first_arena_ext;
    BP_Arena_Ext *last_arena_ext;
    void *user_data;

    // query state
    bool32 query_is_open;
    u64 query_cursor;
    u64 query_mark;
    u8 query_buffer[Kilobytes(1)];
    u64 query_string_size;

    // contents are focused (disabled query focus)
    bool32 contents_are_focused;
};

typedef struct BP_View_State_Slot BP_View_State_Slot;
struct BP_View_State_Slot {
    BP_View_State *first;
    BP_View_State *last;
};

////////////////////
// Vocabulary Map

typedef struct BP_Vocab_Info_Map_Node BP_Vocab_Info_Map_Node;
struct BP_Vocab_Info_Map_Node {
    BP_Vocab_Info_Map_Node *single_next;
    BP_Vocab_Info_Map_Node *plural_next;
    BP_Vocab_Info v;
};

typedef struct BP_Vocab_Info_Map_Slot BP_Vocab_Info_Map_Slot;
struct BP_Vocab_Info_Map_Slot {
    BP_Vocab_Info_Map_Node *first;
    BP_Vocab_Info_Map_Node *last;
};

typedef struct BP_Vocab_Info_Map BP_Vocab_Info_Map;
struct BP_Vocab_Info_Map {
    u64 single_slots_count;
    BP_Vocab_Info_Map_Slot *single_slots;
    u64 plural_slots_count;
    BP_Vocab_Info_Map_Slot *plural_slots;
};

//////////////////
// Command Types

typedef struct BP_Cmd BP_Cmd;
struct BP_Cmd {
    String8 name;
    BP_Regs *regs;
};

typedef struct BP_Cmd_Node BP_Cmd_Node;
struct BP_Cmd_Node {
    BP_Cmd_Node *next;
    BP_Cmd_Node *prev;
    BP_Cmd cmd;
};

typedef struct BP_Cmd_List BP_Cmd_List;
struct BP_Cmd_List {
    BP_Cmd_Node *first;
    BP_Cmd_Node *last;
    u64 count;
};

////////////////////////////
// Context Register Types

typedef struct BP_Regs_Node BP_Regs_Node;
struct BP_Regs_Node {
    BP_Regs_Node *next;
    BP_Regs v;
};

/////////////////////////////////////////////////
// Structured Theme Types, Parsed From Config

typedef enum BP_FontSlot {
    BP_FontSlot_Main,
    BP_FontSlot_Code,
    BP_FontSlot_Icons,
    BP_FontSlot_COUNT
} BP_FontSlot;

/////////////////////
// Per-Window State

typedef struct BP_Drop_Completion_Task BP_Drop_Completion_Task;
struct BP_Drop_Completion_Task {
    BP_Drop_Completion_Task *next;
    bool32 cfg;
    String8_List paths;
};

typedef struct BP_Window_State BP_Window_State;
struct BP_Window_State
{
    // links & metadata
    BP_Window_State *order_next;
    BP_Window_State *order_prev;
    BP_Window_State *hash_next;
    BP_Window_State *hash_prev;
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
    Vec4f32 theme_code_colors[BP_CodeColorSlot_COUNT];

    // font raster flags (recomputed each frame)
    FNT_RasterFlags font_slot_raster_flags[BP_FontSlot_COUNT];

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
    BP_Drop_Completion_Task *top_drop_completion_task;

    // query state
    bool32 query_is_active;
    Arena *query_arena;
    BP_Regs *query_regs;
    CFG_ID query_view_id;
    CFG_ID query_last_view_id;

    // autocompletion state
    u64 autocomp_last_frame_index;
    Arena *autocomp_arena;
    BP_Regs *autocomp_regs;
    BP_Autocomp_Cursor_Info autocomp_cursor_info;

    // error state
    u8 error_buffer[512];
    u64 error_string_size;
    f32 error_t;

    // per-frame ui events state
    UI_Event_List ui_events;

    // per-frame drawing state
    DR_Bucket *draw_bucket;
};

typedef struct BP_Window_State_Slot BP_Window_State_Slot;
struct BP_Window_State_Slot {
    BP_Window_State *first;
    BP_Window_State *last;
};

///////////////////////////////////////
// Main Per-Process Graphical State

typedef struct BP_Ambiguous_Path_Node BP_Ambiguous_Path_Node;
struct BP_Ambiguous_Path_Node
{
    BP_Ambiguous_Path_Node *next;
    String8 name;
    String8_List paths;
};

typedef struct BP_State BP_State;
struct BP_State {
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
    f32 scrolling_animation_rate;
    f32 tooltip_animation_rate;

    // default theme table
    MD_Node *theme_preset_trees[BP_ThemePreset_COUNT];

    // vocab table
    BP_Vocab_Info_Map vocab_info_map;

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

    // ambiguous path table (consutrcted from-scratch each frame)
    u64 ambiguous_path_slots_count;
    BP_Ambiguous_Path_Node **ambiguous_path_slots;

    // key map (constructed from-scratch each frame)
    CFG_Key_Map *key_map;

    // slot -> font tag map (constructed from-scratch each frame)
    FNT_Tag font_slot_table[BP_FontSlot_COUNT];

    // name -> view ui map (constructed form-scratch each frame)
    BP_View_UI_Rule_Map *view_ui_rule_map;

    // registers stack
    BP_Regs_Node base_regs;
    BP_Regs_Node *top_regs;

    // autosave state
    f32 seconds_until_autosave;

    // commands
    Arena *cmds_arenas[2];
    BP_Cmd_List cmds[2];
    u64 cmds_gen;
    Arena *cmd_output_arena;
    String8_List cmd_outputs;

    // popup state
    UI_Key popup_key;
    bool32 popup_active;
    f32 popup_t;
    Arena *popup_arena;
    BP_Cmd_List popup_cmds;
    String8 popup_title;
    String8 popup_desc;

    // text editing mode state
    bool32 text_edit_mode;
    bool32 text_edit_mode_multiline;

    // contextual hover info
    BP_Regs *hover_regs;
    BP_RegSlot hover_regs_slot;
    BP_Regs *next_hover_regs;
    BP_RegSlot next_hover_regs_slot;

    // icon texture
    R_Handle icon_texture;

    // fixed ui keys
    UI_Key drop_completion_key;
    UI_Key ctx_menu_key;

    // drag/drop state
    Arena *drag_drop_arena;
    BP_Regs *drag_drop_regs;
    BP_RegSlot drag_drop_regs_slot;
    BP_DragDropState drag_drop_state;

    // cfg state
    CFG_State *cfg;
    CFG_Schema_Table *cfg_schema_table;

    // window sate cache
    u64 window_state_slots_count;
    BP_Window_State_Slot *window_state_slots;
    BP_Window_State *free_window_state;
    CFG_ID last_focused_window;
    BP_Window_State *first_window_state;
    BP_Window_State *last_window_state;
    CFG_ID window_state_last_accessed_id;
    BP_Window_State *window_state_last_accessed;

    // view state cache
    u64 view_state_slots_count;
    BP_View_State_Slot *view_state_slots;
    BP_View_State *free_view_state;
    CFG_ID view_state_last_accessed_id;
    BP_View_State *view_state_last_accessed;

    // bind change
    Arena *bind_change_arena;
    bool32 bind_change_active;
    CFG_ID bind_change_binding_id;
    String8 bind_change_cmd_name;
};

/////////////
// Globals

read_only global BP_Vocab_Info    bp_nil_vocab_info    = {0};
read_only global BP_Cmd_Kind_Info bp_nil_cmd_kind_info = {0};

BP_VIEW_UI_FUNCTION_DEF(null);
read_only global BP_View_UI_Rule bp_nil_view_ui_rule = {
    {0},
    BP_VIEW_UI_FUNCTION_NAME(null),
};

read_only global BP_View_State bp_nil_view_state = {
    &bp_nil_view_state,
    &bp_nil_view_state,
};

read_only global BP_Window_State bp_nil_window_state = {
    &bp_nil_window_state,
    &bp_nil_window_state,
    &bp_nil_window_state,
    &bp_nil_window_state,
};

global BP_State *bp_state = 0;
global CFG_ID bp_last_drag_drop_panel = 0;
global CFG_ID bp_last_drag_drop_prev_tab = 0;

//////////////////////////////
// Registers Type Functions

internal void bp_regs_copy_contents(Arena *arena, BP_Regs *dst, BP_Regs *src);
internal BP_Regs *bp_regs_copy(Arena *arena, BP_Regs *src);

/////////////////////////////
// Commands Type Functions

internal void bp_cmd_list_push_new(Arena *arena, BP_Cmd_List *cmds, String8 name, BP_Regs *regs);

////////////////////////////
// View UI Rule Functions

internal BP_View_UI_Rule_Map *bp_view_ui_rule_map_make(Arena *arena, u64 slots_count);
internal void                 bp_view_ui_rule_map_insert(Arena *arena, BP_View_UI_Rule_Map *map, String8 string, BP_View_UI_Function_Type *ui);

internal BP_View_UI_Rule      *bp_view_ui_rule_from_string(String8 string);

////////////////////////////////////////////////////////
// Global Cross-Window UI Interaction State Functions

internal bool32 bp_drag_is_active(void);
internal void bp_drag_begin(BP_RegSlot slot);
internal bool32 bp_drag_drop(void);
internal void bp_drag_kill(void);

internal void bp_set_hover_regs(BP_RegSlot slot);
internal BP_Regs *bp_get_hover_regs(void);

//////////////////////
// Config Functions

internal bool32 bp_cfg_is_project_filtered(CFG_Node *cfg);

internal Vec4f32 bp_hsva_from_cfg(CFG_Node *cfg);
internal Vec4f32 bp_color_from_cfg(CFG_Node *cfg);

internal bool32 bp_disabled_from_cfg(CFG_Node *cfg);
internal String8 bp_name_from_cfg(CFG_Node *cfg);
internal String8 bp_label_from_cfg(CFG_Node *cfg);
internal String8 bp_path_from_cfg(CFG_Node *cfg);

internal String8 bp_default_setting_from_names(String8 schema_name, String8 setting_name);

internal String8 bp_setting_from_name(String8 name);
internal bool32 bp_setting_bool32_from_name(String8 name);
internal u64 bp_setting_u64_from_name(String8 name);
internal f32 bp_setting_f32_from_name(String8 name);

internal CFG_Node *bp_immediate_cfg_from_key(String8 string);
internal CFG_Node *bp_immediate_cfg_from_keyf(char *fmt, ...);

////////////////////
// View Functions

internal BP_View_State *bp_view_state_from_cfg(CFG_Node *cfg);
internal void bp_view_ui(Rng2f32 rect);

///////////////////////
// View Building API

// view info extraction
internal Arena *bp_view_arena(void);
internal UI_Scroll_Pt2 bp_view_scroll_pos(void);
internal String8 bp_view_query_cmd(void);
internal String8 bp_view_query_input(void);
internal String8 bp_view_setting_from_name(String8 string);
internal bool32 bp_view_setting_bool32_from_name(String8 string);
internal u64 bp_view_setting_u64_from_name(String8 string);
internal f32 bp_view_setting_f32_from_name(String8 string);

// language kind for tab, from its file path (user for syntax highlighting)
internal TXT_LangKind bp_lang_kind_from_file_path(String8 file_path);

// pushing/attaching view resources
internal void *bp_view_state_by_size(u64 size);
#define bp_view_state(T) (T *)bp_view_state_by_size(sizeof(T))
internal Arena *bp_push_view_arena(void);

// storing view-attached state
internal void bp_store_view_loading_info(bool32 is_loading, u64 progress_u64, u64 progress_u64_target);
internal void bp_store_view_scroll_pos(UI_Scroll_Pt2 pos);
internal void bp_store_view_param(String8 key, String8 value);
internal void bp_store_view_paramf(String8 key, char *fmt, ...);
#define bp_store_view_param_f32(key, f32) bp_store_view_paramf((key), "%ff", (f32))
#define bp_store_view_param_s64(key, s64) bp_store_view_paramf((key), "%I64d", (s64))
#define bp_store_view_param_u64(key, u64) bp_store_view_paramf((key), "0x%I64x", (u64))

/////////////////////
// Window Functions

internal String8 bp_push_window_title(Arena *arena);
internal CFG_Node *bp_window_from_cfg(CFG_Node *cfg);
internal BP_Window_State *bp_window_state_from_cfg(CFG_Node *cfg);
internal BP_Window_State *bp_window_state_from_os_handle(WM_Window os);
internal void bp_window_frame(void);

///////////////////////////
// Autocompletion Lister

internal void bp_set_autocomp_regs_(BP_Regs *regs);
#define bp_set_autocomp_regs(...) bp_set_autocomp_regs_(&(BP_Regs){bp_regs_lit_init_top __VA_ARGS__})

///////////////////////////
// Colors, Fonts, Config

// colors
internal MD_Node *bp_theme_tree_from_name(Arena *arena, Access *access, String8 theme_name);
internal Vec4f32 bp_rgba_from_code_color_slot(BP_CodeColorSlot slot);
internal BP_CodeColorSlot bp_code_color_slot_from_txt_token_kind(TXT_TokenKind kind);
internal BP_CodeColorSlot bp_code_color_slot_from_txt_token_kind_lookup_string(TXT_TokenKind kind, String8 string, bool32 allow_macros, bool32 is_called);

// fonts
internal f32 bp_font_size(void);
internal FNT_Tag bp_font_from_slot(BP_FontSlot slot);
internal FNT_RasterFlags bp_raster_flags_from_slot(BP_FontSlot slot);

////////////////////////
// Vocab Info Lookups

internal BP_Vocab_Info *bp_vocab_info_from_code_name(String8 code_name);
internal BP_Vocab_Info *bp_vocab_info_from_code_name_plural(String8 code_name_plural);
#define bp_plural_from_code_name(code_name) (bp_vocab_info_from_code_name(code_name)->code_name_plural)
#define bp_display_from_code_name(code_name) (bp_vocab_info_from_code_name(code_name)->display_name)
#define bp_display_plural_from_code_name(code_name) (bp_vocab_info_from_code_name(code_name)->display_name_plural)
#define bp_icon_kind_from_code_name(code_name) (bp_vocab_info_from_code_name(code_name)->icon_kind)
#define bp_singular_from_code_name_plural(code_name_plural) (bp_vocab_info_from_code_name_plural(code_name_plural)->code_name)

///////////////////////////////
// Continuous Frame Requests

internal void bp_request_frame(void);

//////////////////////////
// Main State Accessors

// per-frame arena
internal Arena *bp_frame_arena(void);

///////////////
// Registers

#define bp_regs() (&bp_state->top_regs->v)
#define bp_base_regs() (&bp_state->base_regs.v)
internal BP_Regs *bp_push_regs_(BP_Regs *regs);
#define bp_push_regs(...) bp_push_regs_(&(BP_Regs){bp_regs_lit_init_top __VA_ARGS__})
internal BP_Regs *bp_pop_regs(void);
#define BP_RegsScope(...) DeferLoop(bp_push_regs(__VA_ARGS__), bp_pop_regs())

//////////////
// Commands

// name -> info
internal BP_CmdKind bp_cmd_kind_from_string(String8 string);
internal BP_Cmd_Kind_Info *bp_cmd_kind_info_from_string(String8 string);

// pushing
internal void bp_push_cmd(String8 name, BP_Regs *regs);
#define bp_cmd(kind, ...) bp_push_cmd(bp_cmd_kind_info_table[kind].string, &(BP_Regs){bp_regs_lit_init_top __VA_ARGS__})

// iterating
internal bool32 bp_next_cmd(BP_Cmd **cmd);
internal bool32 bp_next_view_cmd(BP_Cmd **cmd);

////////////////////////////////
// Main Layer Top-Level Calls

internal void bp_init(Cmd_Line *cmdline);
internal void bp_frame(void);

#endif // BROKEN_PROXY_CORE_H
