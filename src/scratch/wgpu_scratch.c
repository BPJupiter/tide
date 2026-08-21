
#define BUILD_TITLE "wgpu_scratch"
#define OS_FEATURE_GRAPHICAL 1

#include "base/base_inc.h"
#include "render/render_inc.h"
#include "draw/draw.h"

#include "base/base_inc.c"
#include "render/render_inc.c"
#include "draw/draw.c"

internal bool32 frame(void)
{
    bool32 quit = false;
    Temp scratch = scratch_begin(0, 0);
    WM_Event_List events = wm_get_events(scratch.arena, 0);
    for (WM_Event *evt = events.first; evt != 0; evt = evt->next) {
        if (evt->kind == WM_EventKind_WindowClose) {
            quit = true;
            break;
        }
    }
    r_begin_frame();
    dr_begin_frame(fnt_tag_zero());
    r_window_begin_frame(os_window, r_window);
}
