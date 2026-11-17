// Copyright (c) Frances Telfar
// Licensed under the PolyForm Noncommercial License (https://polyformproject.org/licenses/noncommercial/1.0.0)

#ifndef HOOKS_H
#define HOOKS_H

#define hk_hook C_LINKAGE

//////////////////////
// Backend functions to be called by the parent (hk_parent_)

hk_hook Process hk_parent_create_process_with_dll(String8 application_name, String8 cmd_line, String8 working_directory, String8 dll_name);
hk_hook Process hk_parent_create_process_with_dlls(String8 application_name, String8 cmd_line, String8 working_directory, String8_List dll_names);
hk_hook bool32 hk_parent_start_process_with_hooks(Process process);

//////////////////////
// Backend function to be called by the library (hk_lib_)

hk_hook void hk_lib_init(void);

hk_hook bool32 hk_lib_transaction_begin(void);
hk_hook bool32 hk_lib_transaction_commit(void);
hk_hook bool32 hk_lib_transaction_abort(void);

hk_hook bool32 hk_lib_install_function(void **real_func, void *hook_func);
hk_hook bool32 hk_lib_uninstall_function(void **real_func, void *hook_func);
hk_hook void *hk_lib_find_function(String8 dll_name, String8 function_name);
hk_hook bool32 hk_lib_register_all_threads_for_transaction(void);
hk_hook bool32 hk_lib_restore_import_table(void);

/////////////////
// Macro Loop Wrappers

#define HK_LIB_TransactionScope() DeferLoop(hk_lib_transaction_begin(), hk_lib_transaction_commit())


#endif // HOOKS_H
