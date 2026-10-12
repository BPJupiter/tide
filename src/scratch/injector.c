// Copyright (c) Frances Telfar
// Licensed under the PolyForm Noncommercial License (https://polyformproject.org/licenses/noncommercial/1.0.0)

////////////////////
// Build Options

#define BUILD_TITLE "INJECTOR"
#define BUILD_CONSOLE_INTERFACE 1

//////////////
// Includes

#include "base/base_inc.h"
#include "injection/injection_inc.h"

#include "base/base_inc.c"
#include "injection/injection_inc.c"


internal void
entry_point(Cmd_Line *cmdline)
{
    Temp scratch = scratch_begin(0, 0);
    
    fprintf(stdout, "Enter Target Process EXE name: ");
    String8 process_name = {0};
    char input[32] = {0};
    u64 process_id = 0;
    if(fgets(input, sizeof(input), stdin) != 0)
    {
        input[strchr(input, '\n')-input] = 0;
        process_name = str8_cstring(input);
        process_id = pid_from_name(process_name);
    }

    String8 dll_path = full_path_from_path(scratch.arena, s("build\\typing_sound.dll"));
    if(process_id)
    {
        fprintf(stdout, "Process %.*s: %I64u\n", str8_varg(process_name), process_id);
        fprintf(stdout, "DLL to inject : %.*s\n", str8_varg(dll_path));
        if(!inj_dll_into_pid(process_id, dll_path))
        {
            printf("FUCK\n");
        }
    }
    else
    {
        fprintf(stderr, "No such process with the name %.*s!\n", str8_varg(process_name));
    }
    
    scratch_end(scratch);
}
