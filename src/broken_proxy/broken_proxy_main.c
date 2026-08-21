
///////////////////
// Build Options

#define BUILD_TITLE "BROKEN PROXY"

#define OS_FEATURE_GRAPHICAL 1

//////////////
// Includes

#include "base/base_inc.h"
#include "mdesk/mdesk.h"
#include "net/net_inc.h"
#include "dns/dns_inc.h"
#include "window_manager/window_manager_inc.h"
#include "text/text.h"
#include "mutable_text/mutable_text.h"
#include "font_provider/font_provider_inc.h"
#include "render/render_inc.h"
#include "font_cache/font_cache.h"
#include "draw/draw.h"
#include "ui/ui_inc.h"
#include "broken_proxy_inc.h"

#include "base/base_inc.c"
#include "mdesk/mdesk.c"
#include "net/net_inc.c"
#include "dns/dns_inc.c"
#include "window_manager/window_manager_inc.c"
#include "text/text.c"
#include "mutable_text/mutable_text.c"
#include "font_provider/font_provider_inc.c"
#include "render/render_inc.c"
#include "font_cache/font_cache.c"
#include "draw/draw.c"
#include "ui/ui_inc.c"
#include "broken_proxy_inc.c"

internal bool32 frame(void) { return 0; }

/////////////////
// Entry Point

void entry_point(Cmd_Line *cmdline)
{
}
