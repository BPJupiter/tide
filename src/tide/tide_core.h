
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
};

////////////////////////////////////
// Main Per-Process Graphical State

typedef struct TI_State TI_State;
struct TI_State {
    // basics
    Arena *arena;
    bool32 quit;
    s32 frame_depth;

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

//////////////////////////////
// Main Layer Top-Level Calls

internal void ti_init(Cmd_Line *cmdline);
internal void ti_frame(void);

#endif // TIDE_CORE_H
