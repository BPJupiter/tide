
//////////////////
// Build Options

#define BUILD_TITLE "CRITTERS"
#define OS_FEATURE_GRAPHICAL 1

#define ARENA_TABLE_DEBUG BUILD_DEBUG

////////////////
// Includes


#include "base/base_inc.h"
#include "win32/win32_inc.h"
#include "artifact_cache/artifact_cache.h"
#include "mdesk/mdesk.h"
#include "net/net_inc.h"
#include "dns/dns_inc.h"
#include "window_manager/window_manager_inc.h"
#include "shell/shell_inc.h"
#include "config/config_inc.h"
#include "content/content.h"
#include "file_stream/file_stream.h"
#include "text/text.h"
#include "mutable_text/mutable_text.h"
#include "font_provider/font_provider_inc.h"
#include "render/render_inc.h"
#include "font_cache/font_cache.h"
#include "draw/draw.h"
#include "ui/ui_inc.h"

#include "base/base_inc.c"
#include "artifact_cache/artifact_cache.c"
#include "mdesk/mdesk.c"
#include "net/net_inc.c"
#include "dns/dns_inc.c"
#include "window_manager/window_manager_inc.c"
#include "shell/shell_inc.c"
#include "config/config_inc.c"
#include "content/content.c"
#include "file_stream/file_stream.c"
#include "text/text.c"
#include "mutable_text/mutable_text.c"
#include "font_provider/font_provider_inc.c"
#include "render/render_inc.c"
#include "font_cache/font_cache.c"
#include "draw/draw.c"
#include "ui/ui_inc.c"

#if !defined(STBI_INCLUDE_STB_IMAGE_H)
# define STB_IMAGE_IMPLEMENTATION
# include "third_party/stb/stb_image.h"
#endif

///////////////
// Constants

#define SCREEN_WIDTH 1600
#define SCREEN_HEIGHT 900

#define MAX_THINGS 1000
#define CRITTER_COUNT 800
#define MAX_VELOCITY 80
#define DSTROYER_RADIUS 20
#define CRITTER_RADIUS 12

#define GRID_CELL_SIZE (CRITTER_RADIUS * 2)
#define GRID_COLS ((SCREEN_WIDTH * GRID_CELL_SIZE - 1) / GRID_CELL_SIZE)
#define GRID_ROWS ((SCREEN_HEIGHT * GRID_CELL_SIZE - 1) / GRID_CELL_SIZE)
#define GRID_CELL_COUNT (GRID_COLS * GRID_ROWS)

typedef enum Kind {
    Kind_Nil,
    Kind_Destroyer,
    Kind_Critter,
    Kind_COUNT
} Kind;

typedef struct Thing Thing;
struct Thing {
    Kind kind;
    Vec2f32 position;
    Vec2f32 velocity;
    f32 radius;
    bool32 is_dirty;
};

typedef struct Things Things;
struct Things {
    Thing things[MAX_THINGS];
    u32 count;
};

typedef struct Texture Texture;
struct Texture {
    u8 *data;
    int width;
    int height;
    int channels;
};

typedef struct Grid Grid;
struct Grid {
    u32 cell_head[GRID_CELL_COUNT];
    u32 thing_next[MAX_THINGS];
};

///////////
// Globals

global char *texture_paths[] = {
    "MISSING.png",
    "destroyer.png",
    "critter.png",
};
global Texture textures[Kind_COUNT] = {0};
global Things things                = { .things = {0}, .count = 1 };
global u32 destroyer_idx            = 0;
global Grid grid                    = {0};

global WM_Window os_window = {0};
global R_Handle r_window = {0};

////////////
// Methods

// Things
internal u32 thing_add(Kind kind)
{
    if (things.count < MAX_THINGS)
    {
        u32 slot = things.count++;
        MemoryZeroStruct(&things.things[slot]);
        things.things[slot].kind = kind;
        return slot;
    }
    return 0;
}

internal Thing *thing_get(u32 idx)
{
    if (idx > 0 && idx < things.count)
    {
        return &things.things[idx];
    }
    return &things.things[0];
}

internal void thing_remove(u32 idx)
{
    if (idx > 0 && idx < things.count)
    {
        things.things[idx] = things.things[things.count - 1];
        things.count--;
    }
}

// Grid
internal void grid_clear(void)
{
    MemoryZeroStruct(grid.cell_head);
}

internal Vec2s32 grid_cell_coords(Vec2f32 pos)
{
    Vec2s32 result = {0};
    result.x = (s32)(pos.x / GRID_CELL_SIZE);
    result.y = (s32)(pos.y / GRID_CELL_SIZE);
    result.x = Clamp(0, result.x, GRID_COLS - 1);
    result.y = Clamp(0, result.y, GRID_ROWS - 1);
    return result;
}

internal void grid_insert(u32 idx, Vec2f32 position)
{
    Vec2s32 coords = grid_cell_coords(position);
    s32 ci = coords.y * GRID_COLS + coords.x;
    grid.thing_next[idx] = grid.cell_head[idx];
    grid.cell_head[ci] = idx;
}

////////////
// Helpers

internal f64 update_fps(void)
{
    local_persist u64 frame_count = 0;
    local_persist f64 last_time = 0.f;
    local_persist f64 fps = 0.f;

    f64 now_time = now_time_us() / 1000000.f;

    if (last_time == 0.0)
    {
        last_time = now_time;
    }

    frame_count++;

    if (now_time - last_time >= 1.0)
    {
        fps = (f64)frame_count / (now_time - last_time);
        frame_count = 0;
        last_time = now_time;
    }
    return fps;
}

///////////////////////////
// Per-Frame Entry Point

internal bool32 frame(void)
{
    ProfBeginFunction();
    bool32 quit = false;
    Temp scratch = scratch_begin(0, 0);
    WM_Event_List events = wm_get_events(scratch.arena, 0);
    for (WM_Event *evt = events.first; evt != 0; evt = evt->next)
    {
        if (evt->kind == WM_EventKind_WindowClose)
        {
            quit = true;
            break;
        }
    }

    // begin frame
    r_begin_frame();
    dr_begin_frame(fnt_tag_zero());
    r_window_begin_frame(os_window, r_window);
    DR_Bucket *bucket = dr_bucket_make();
    DR_BucketScope(bucket) ProfScope("draw")
    {
        local_persist FNT_Tag font = {0};
        local_persist bool32 font_loaded = false;
        if (!font_loaded)
        {
            font = fnt_tag_from_path(str8_lit("P:/brokenProxy/data/Inconsolata-Regular.ttf"));
            font_loaded = true;
        }
        dr_text(font, 16.f, 0, 0,
                FNT_RasterFlag_Smooth|FNT_RasterFlag_Hinted,
                v2f32(30, 30),
                v4f32(1, 1, 1, 1),
                str8f(scratch.arena, "FPS: %f", update_fps()));
    }

    // end frame
    r_window_submit(os_window, r_window, &bucket->passes);
    r_window_end_frame(os_window, r_window);
    r_end_frame();
    scratch_end(scratch);
    ProfEnd();
    return quit;
}

///////////////
// Entry Point

void entry_point(Cmd_Line *cmdline)
{
    (void)cmdline;
    for (u64 i = 0; i < Kind_COUNT; i++)
    {
        textures[i].data = stbi_load(texture_paths[i],
                                     &textures[i].width,
                                     &textures[i].height,
                                     &textures[i].channels, 4);
    }
    os_window = wm_window_open(r2f32p(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT),
                               WM_WindowFlag_UseDefaultPosition,
                               str8_lit("CRITTERS"));
    r_window = r_window_equip(os_window);
    wm_window_first_paint(os_window);
    for (;!update(););
}
