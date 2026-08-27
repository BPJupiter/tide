
////////////////////
// Build Options

#define BUILD_TITLE "BORKED_DNS_SERVER"
#define BUILD_CONSOLE_INTERFACE 1

//////////////
// Includes

#include "base/base_inc.h"
#include "net/net_inc.h"
#include "dns/dns_inc.h"

#include "base/base_inc.c"
#include "net/net_inc.c"
#include "dns/dns_inc.c"

internal void
bdns_help(void)
{
    fprintf(stderr, "--- Help -------------------------------------------------------\n");
    fprintf(stderr, " %s\n\n", BUILD_TITLE_STRING_LITERAL);
    fprintf(stderr, " Usage: borked_dns_server [Options]\n\n");
    fprintf(stderr, " Options:\n");
}

internal void
entry_point(Cmd_Line *cmdline)
{
    Temp scratch = scratch_begin(0, 0);
    u64 exit_code = max_u64;
    
    String8_List inputs = {0};
    str8_list_concat_in_place(&inputs, &cmdline->inputs);
    if (inputs.node_count == 0)
    {
        bdns_help();
        goto exit;
    }
    
 exit:;
    scratch_end(scratch);
    exit(exit_code);
}
