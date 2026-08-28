
#ifndef TIDE_CORE_H
#define TIDE_CORE_H

///////////////////
// Generated Code

#include "generated/tide.meta.h"

/////////////////////
// Per-Window State

typedef struct TI_Window_State TI_Window_State;
struct TI_Window_State {
    // top-level info & handles
    Arena *arena;
    WM_Window os;
    R_Handle r;
    UI_State *ui;

    // theme (recomputed each frame)
    UI_Theme *theme;

    // error state
    u8 error_buffer[512];
    u64 error_string_size;
    f32 error_t;

    // per-frame ui events state
    UI_Event_List ui_events;

    // per-frame drawing state
    DR_Bucket *draw_bucket;
};

////////////////////////////////////
// Main Per-Process Graphical State

typedef struct TI_State TI_State;
struct TI_State {
    // basics
    Arena *arena;
    bool32 quit;
    s32 frame_depth;

    // animation rates
    f32 catchall_animation_rate;
    f32 menu_animation_rate;
    f32 menu_animation_rate__slow;
    f32 entity_alive_animation_rate;
    f32 rich_hover_animation_rate;
    f32 scrolling_animation_rate;
    f32 tooltip_animation_rate;

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

    // window state cache
    TI_Window_State *window_state;
};

//////////////
// Globals

read_only global TI_Window_State ti_nil_window_state = {0};
global TI_State *ti_state = 0;

// Dev flags
global bool32 DEV_draw_ui_text_pos = false;
global bool32 DEV_draw_ui_focus_debug = false;
global bool32 DEV_draw_ui_box_heatmap = false;

///////////////////
// Window Functions

internal void ti_window_frame(void);

////////////////////////////
// Continuous Frame Requests

internal void ti_request_frame(void);

///////////////////////
// Main State Accessors

internal Arena *ti_frame_arena(void);

//////////////////////////////
// Main Layer Top-Level Calls

internal void ti_init(Cmd_Line *cmdline);
internal void ti_frame(void);

#endif // TIDE_CORE_H
