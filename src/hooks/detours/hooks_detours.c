
//////////////////////////
// Backend Implementations

hk_hook bool32
hk_transaction_begin(void)
{
    LONG err = DetourTransactionBegin();
    return err == NO_ERROR;
}

hk_hook bool32
hk_transaction_commit(void)
{
    LONG err = DetourTransactionCommit();
    return err == NO_ERROR;
}

hk_hook bool32
hk_transaction_abort(void)
{
    LONG err = DetourTransactionAbort();
    return err == NO_ERROR;
}

hk_hook bool32
hk_install_function(void **real_func, void *hook_func)
{
    LONG err = DetourAttach(real_func, hook_func);
    return err == NO_ERROR;
}

hk_hook bool32
hk_uninstall_function(void **real_func, void *hook_func)
{
    LONG err = DetourDetach(real_func, hook_func);
    return err == NO_ERROR;
}

hk_hook void *
hk_find_function(String8 dll_name, String8 function_name)
{
    void *func_addr = DetourFindFunction(dll_name.str, function_name.str);
    return func_addr;
}

hk_hook bool32
hk_register_thread_for_transaction(Thread thread)
{
    LONG err = DetourUpdateThread((HANDLE)thread.u64[0]);
    return err == NO_ERROR;
}

hk_hook Process
hk_create_process_with_dll(String8 application_name, String8 cmd_line, String8 current_directory, String8 dll_name)
{
    STARTUPINFOA si = {0};
    PROCESS_INFORMATION pi = {0};
    
    bool32 ok = DetourCreateProcessWithDlls(application_name.str,
                                            current_directory.str,
                                            0, 0, 1, 0, 0, 0,
                                            &si, &pi, 1, &dll_name.str, 0);

    Process result = {0};
    return result;
}

hk_hook Process
hk_create_process_with_dlls(String8 application_name, String8 cmd_line, String8 current_directory, String8_List dll_names)
{
    Process result = {0};
    return result;
}

hk_hook bool32
hk_restore_import_table(void)
{
    return 0;
}


