// Copyright (c) Frances Telfar
// Licensed under the PolyForm Noncommercial License (https://polyformproject.org/licenses/noncommercial/1.0.0)

#ifndef HOOKS_H
#define HOOKS_H

#define hk_hook C_LINKAGE

//////////////////////
// Backend Hooks

hk_hook bool32 hk_transaction_begin(void);
hk_hook bool32 hk_transaction_commit(void);
hk_hook bool32 hk_transaction_abort(void);

hk_hook bool32 hk_install_function(void **real_func, void *hook_func);
hk_hook bool32 hk_uninstall_function(void **real_func, void *hook_func);
hk_hook void *hk_find_function(String8 dll_name, String8 function_name);
hk_hook bool32 hk_register_thread_for_transaction(Thread thread);
hk_hook Process hk_create_process_with_dll(String8 application_name, String8 cmd_line, String8 current_directory, String8 dll_name);
hk_hook Process hk_create_process_with_dlls(String8 application_name, String8 cmd_line, String8 current_directory, String8_List dll_names);
hk_hook bool32 hk_restore_import_table(void);

#endif // HOOKS_H
