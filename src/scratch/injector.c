// Copyright (c) Frances Telfar
// Licensed under the PolyForm Noncommercial License (https://polyformproject.org/licenses/noncommercial/1.0.0)

////////////////////
// Build Options

#define BUILD_TITLE "INJECTOR"
#define BUILD_CONSOLE_INTERFACE 1

//////////////
// Includes

#include "base/base_inc.h"
#include "hooks/hooks_inc.h"
#include "shell/shell_inc.h"

#include "base/base_inc.c"
#include "hooks/hooks_inc.c"
#include "shell/shell_inc.c"


internal void
entry_point(Cmd_Line *cmdline)
{
    Temp scratch = scratch_begin(0, 0);

    if(cmdline->inputs.node_count == 0)
    {
        fprintf(stderr, "ERROR: No exe given\n");
        goto exit;
    }
    String8 exe = cmdline->inputs.first->string;

    String8 dll_path = s("P:/tide/build/tidelib.dll");
    Process process = hk_parent_create_process_with_dll(exe, str8_zero(), str8_zero(), dll_path);
    hk_parent_start_process_with_hooks(process);

 exit:;
    scratch_end(scratch);
}
