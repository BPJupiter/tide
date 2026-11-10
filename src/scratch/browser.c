
// Copyright (c) Frances Telfar
// Licensed under the PolyForm Noncommercial License (https://polyformproject.org/licenses/noncommercial/1.0.0)

/////////////////
// Build Options

#define BUILD_TITLE "BROWSER"
#define OS_FEATURE_GRAPHICAL 1

#define ARENA_TABLE_DEBUG BUILD_DEBUG

///////////////
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

///////////////////
// Top Level Execution Types

typedef enum ExecMode
{
    ExecMode_Browser,
    ExecMode_Renderer,
    ExecMode_GPUProcess,
    ExecMode_NetworkProcess,
} ExecMode;

///////////////////////
// Per-Frame Entry Point

internal bool32
frame(void)
{
    return 0;
}

