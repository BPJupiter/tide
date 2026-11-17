// Copyright (c) Frances Telfar
// Licensed under the PolyForm Noncommercial License (https://polyformproject.org/licenses/noncommercial/1.0.0)

///////////////////
// Build Options

#define BUILD_TITLE "TIDELIB"
#define BUILD_DYNAMIC_LIBRARY 1

//////////////////
// Includes

#include "base/base_inc.h"
#include "shell/shell_inc.h"
#include "hooks/hooks_inc.h"

#include "base/base_inc.c"
#include "shell/shell_inc.c"
#include "hooks/hooks_inc.c"

///////////////////
// Hooks

global BOOL (*TrueTranslateMessage)(const MSG *msg) = TranslateMessage;
global VOID (*TrueSleep)(DWORD ms) = Sleep;

BOOL
HookedTranslateMessage(const MSG *msg)
{
    //sh_message(0, s("HOOKED!"), s("TranslateMessage"));
    //MessageBoxA(0, "TranslateMessage", "HOOKED", MB_OK);
    return TrueTranslateMessage(msg);
}

VOID
HookedSleep(DWORD ms)
{
    MessageBoxA(0, "Sleep", "HOOKED", MB_OK);
    TrueSleep(ms);
}

///////////////////
// On Load/Unload Points

internal void
library_load_point(void)
{
    hk_lib_init();
    hk_lib_restore_import_table();
    hk_lib_transaction_begin();
    {
        hk_lib_register_all_threads_for_transaction();
        hk_lib_install_function(&(void *)TrueTranslateMessage, HookedTranslateMessage);
        hk_lib_install_function(&(void *)TrueSleep, HookedSleep);
    }
    bool32 ok = hk_lib_transaction_commit();
#if 0
    if(ok)
    {
        sh_message(0, s("HOOK"), s("Did it!"));
    }
    else
    {
        sh_message(0, s("HOOK"), s("Error trying to hook"));
    }
#endif
}

internal void
library_unload_point(void)
{
    
}
