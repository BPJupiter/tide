// Copyright (c) Frances Telfar
// Licensed under the PolyForm Noncommercial License (https://polyformproject.org/licenses/noncommercial/1.0.0)

////////////////////
// Build Options

#define BUILD_TITLE "TYPING_SOUND_DLL"
#define BUILD_DYNAMIC_LIBRARY 1

////////////////
// Includes

#include "base/base_inc.h"
#include "shell/shell_inc.h"

#include "base/base_inc.c"
#include "shell/shell_inc.c"

internal void
entry_point(Cmd_Line *cmdline)
{}

internal void
library_init_entry_point(void)
{
    MessageBoxA(0, "The library has loaded successfully.", "DLL INJECTED!", MB_OK);
    //sh_message(0, s("The library has loaded successfully."), s("MESSAGE_BOX"));
}

internal void
library_close_exit_point(void)
{
    
}
