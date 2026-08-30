// Copyright (c) Epic Games Tools
// Licensed under the MIT license (https://opensource.org/license/mit/)

#ifndef WIN32_WINDOW_MANAGER_H
#define WIN32_WINDOW_MANAGER_H

/////////////////////////
// Incluldes/Libraries

#include <uxtheme.h>
#include <dwmapi.h>
#include <shellscalingapi.h>
#pragma comment(lib, "gdi32")
#pragma comment(lib, "dwmapi")
#pragma comment(lib, "UxTheme")
#pragma comment(lib, "comdlg32")
#ifndef WM_NCUAHDRAWCAPTION
# define WM_NCUAHDRAWCAPTION (0x00AE)
#endif
#ifndef WM_NCUAHDRAWFRAME
# define WM_NCUAHDRAWFRAME (0x00AF)
#endif

/////////////
// Windows

typedef struct W32_WM_Title_Bar_Client_Area W32_WM_Title_Bar_Client_Area;
struct W32_WM_Title_Bar_Client_Area {
    W32_WM_Title_Bar_Client_Area *next;
    Rng2f32 rect;
};

typedef struct W32_WM_Window W32_WM_Window;
struct W32_WM_Window {
    W32_WM_Window *next;
    W32_WM_Window *prev;
    HWND hwnd;
    HDC hdc;
    WINDOWPLACEMENT last_window_placement;
    f32 dpi;
    bool32 first_paint_done;
    bool32 maximized;
    bool32 custom_border;
    f32 custom_border_title_thickness;
    f32 custom_border_edge_thickness;
    bool32 custom_border_composition_enabled;
    Arena *paint_arena;
    W32_WM_Title_Bar_Client_Area *first_title_bar_client_area;
    W32_WM_Title_Bar_Client_Area *last_title_bar_client_area;
};

//////////////////////////////
// Monitor Gathering Bundle

typedef struct W32_WM_Monitor_Gather_Node W32_WM_Monitor_Gather_Node;
struct W32_WM_Monitor_Gather_Node {
    W32_WM_Monitor_Gather_Node *next;
    WM_Monitor v;
};

typedef struct W32_WM_Monitor_Gather_Bundle W32_WM_Monitor_Gather_Bundle;
struct W32_WM_Monitor_Gather_Bundle {
    Arena *arena;
    W32_WM_Monitor_Gather_Node *first_monitor;
    W32_WM_Monitor_Gather_Node *last_monitor;
    u64 monitor_count;
};

//////////////////
// Global State

typedef struct W32_WM_State W32_WM_State;
struct W32_WM_State {
    Arena *arena;
    u32 gfx_thread_tid;
    HINSTANCE hInstance;
    HCURSOR hCursor;
    WM_System_Info gfx_info;
    W32_WM_Window *first_window;
    W32_WM_Window *last_window;
    W32_WM_Window *free_window;
    WM_Key key_from_vkey_table[256];
};

/////////////
// Globals

global W32_WM_State *w32_wm_state = 0;
global WM_Event_List w32_wm_event_list = {0};
global Arena *w32_wm_event_arena = 0;
global bool32 w32_wm_resising = false;
global bool32 w32_wm_new_window_custom_border = false;

///////////////////
// Basic Helpers

internal Rng2f32 w32_wm_rng2f32_from_rect(RECT rect);

/////////////
// Windows

internal WM_Window      w32_wm_handle_from_window(W32_WM_Window *window);
internal W32_WM_Window *w32_wm_window_from_handle(WM_Window window);
internal W32_WM_Window *w32_wm_window_from_hwnnd(HWND hwnd);
internal HWND           w32_wm_hwnd_from_window(W32_WM_Window *window);
internal W32_WM_Window *w32_wm_window_alloc(void);
internal void           w32_wm_window_release(W32_WM_Window *window);
internal WM_Event      *w32_wm_push_event(WM_EventKind kind, W32_WM_Window *window);
internal WM_Key         w32_wm_os_key_from_vkey(WPARAM vkey);
internal WPARAM         w32_wm_vkey_from_os_key(WM_Key key);
internal LRESULT        w32_wm_wnd_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

//////////////
// Monitors

internal BOOL w32_wm_monitor_gather_enum_proc(HMONITOR monitor, HDC hdc, LPRECT rect, LPARAM bundle_ptr);

#endif // WIN32_WINDOW_MANAGER_H
