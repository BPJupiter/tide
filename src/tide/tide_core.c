
/////////////////////
// Generated Code

#include "generated/tide.meta.c"

/////////////////////////////
// Main Layer Top-Level Calls

#if !defined(STBI_INCLUDE_STB_IMAGE_H)
# define STB_IMAGE_IMPLEMENTATION
# define STBI_ONLY_PNG
# define STBI_ONLY_BMP
# include "third_party/stb/stb_image.h"
#endif

internal void ti_init(Cmd_Line *cmdline)
{
    ProfBeginFunction();
    Temp scratch = scratch_begin(0, 0);

    Arena *arena = arena_alloc();
    ti_state = push_array(arena, TI_State, 1);
    ti_state->arena = arena;
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

    // set up window cache
    {
        ti_state->window_state = push_array(arena, TI_Window_State, 1);
        ti_state->window_state->os = wm_window_open(r2f32p(0, 0, 1600, 900),
                                                   WM_WindowFlag_UseDefaultPosition,
                                                   str8_lit("TIDE"));
        ti_state->window_state->r = r_window_equip(ti_state->window_state->os);
        ti_state->window_state->ui = ui_state_alloc();
        ui_select_state(ti_state->window_state->ui);
        wm_window_first_paint(ti_state->window_state->os);
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

    WM_Event_List events = wm_get_events(scratch.arena, 0);
    for (WM_Event *evt = events.first; evt != 0; evt = evt->next)
    {
        if (evt->kind == WM_EventKind_WindowClose)
        {
            ti_state->quit = true;
            break;
        }
    }
    r_begin_frame();
    dr_begin_frame(fnt_tag_zero());
    r_window_begin_frame(ti_state->window_state->os, ti_state->window_state->r);
    DR_Bucket *bucket = dr_bucket_make();
    DR_BucketScope(bucket) ProfScope("draw")
    {
        Vec2f32 mouse = wm_mouse_from_window(ti_state->window_state->os);
        /*
        UI_Box *container_box = ui_build_box_from_stringf(0, "container");
        UI_Parent(container_box)
            UI_PrefWidth(ui_px(200, 1.f))
            UI_PrefHeight(ui_em(2.f, 1.f))
            UI_BackgroundColor(v4f32(0.1f, 0.1f, 0.1f, 1.f))
        {
            UI_Signal sig = ui_button(str8_lit("Click me"));
        }
        Vec2f32 mouse = ui_mouse();
        */
        FNT_Tag font = fnt_tag_from_static_data_string(&ti_default_main_font_bytes);
        dr_text(font, 16.f, 0, 0,
                FNT_RasterFlag_Smooth|FNT_RasterFlag_Hinted,
                v2f32(30 + mouse.x, 30 + mouse.y),
                v4f32(1, 1, 1, 1),
                str8f(scratch.arena, "%.2f, %.2f", mouse.x, mouse.y));
    }
    r_window_submit(ti_state->window_state->os, ti_state->window_state->r, &bucket->passes);
    r_window_end_frame(ti_state->window_state->os, ti_state->window_state->r);
    r_end_frame();

    ProfEnd();
    scratch_end(scratch);
}
