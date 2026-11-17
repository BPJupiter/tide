
/////////////////////////
// WindowsNT functions

typedef NTSTATUS W32_NtGetNextThread_Type(HANDLE ProcessHandle, HANDLE ThreadHandle, ACCESS_MASK DesiredAccess, ULONG HandleAttributes, ULONG Flags, PHANDLE NewThreadHandle);

global W32_NtGetNextThread_Type *NtGetNextThread = 0;

//////////////////////////
// Callbacks for DLL validation

typedef struct HK_DTR_Export_Context HK_DTR_Export_Context;
struct HK_DTR_Export_Context
{
    BOOL has_ordinal1;
    ULONG export_count;
};

internal BOOL
hk_dtr_export_callback(PVOID context, ULONG ordinal, LPCSTR symbol, PVOID target)
{
    HK_DTR_Export_Context *ec = (HK_DTR_Export_Context *)context;
    if(ordinal == 1)
    {
        ec->has_ordinal1 = TRUE;
    }
    ec->export_count += 1;

    return TRUE;
}

////////////////////////////
// Export stub
//
// NOTE(fbt): Detours requires that we export at least a single function.
// We export a stub here so we can more easily write test DLLs

#if BUILD_DYNAMIC_LIBRARY
shared_function void
hk_dtr_satisfy_single_export_requirement(void)
{}
#endif

/////////////////////////
// Functions to be called by the parent

hk_hook Process
hk_parent_create_process_with_dll(String8 application_name, String8 cmd_line, String8 working_directory, String8 dll_name)
{
    // Validate DLL
    HMODULE dll = LoadLibraryExA(dll_name.str, 0, DONT_RESOLVE_DLL_REFERENCES);
    HK_DTR_Export_Context ec = {0};
    DetourEnumerateExports(dll, &ec, hk_dtr_export_callback);
    FreeLibrary(dll);

    if(!ec.has_ordinal1)
    {
        sh_message(0, s("HOOK"), s("DLL does not export ordinal #1"));
    }
    
    // Create process with dll
    STARTUPINFOA si = {0};
    PROCESS_INFORMATION pi = {0};
    
    bool32 ok = DetourCreateProcessWithDlls(application_name.str,
                                            cmd_line.size ? cmd_line.str : 0,
                                            0, 0, 1, 0, 0,
                                            working_directory.size ? working_directory.str : 0,
                                            &si, &pi, 1, &dll_name.str, 0);

    Process result = {0};
    result.u64[0] = (u64)pi.hThread;
    return result;
}

hk_hook Process
hk_parent_create_process_with_dlls(String8 application_name, String8 cmd_line, String8 working_directory, String8_List dll_names)
{
    Process result = {0};
    return result;
}

hk_hook bool32
hk_parent_start_process_with_hooks(Process process)
{
    ResumeThread((HANDLE)process.u64[0]);
    return 1;
}



/////////////////////////
// Functions to be called bt the library

hk_hook void
hk_lib_init(void)
{
    // we use some NT functions in this layer.
    HMODULE module = LoadLibraryA("ntdll.dll");
    NtGetNextThread = (void *)GetProcAddress(module, "NtGetNextThread");
    FreeLibrary(module);
}

hk_hook bool32
hk_lib_transaction_begin(void)
{
    LONG err = DetourTransactionBegin();
    return err == NO_ERROR;
}

hk_hook bool32
hk_lib_transaction_commit(void)
{
    LONG err = DetourTransactionCommit();
    return err == NO_ERROR;
}

hk_hook bool32
hk_lib_transaction_abort(void)
{
    LONG err = DetourTransactionAbort();
    return err == NO_ERROR;
}

hk_hook bool32
hk_lib_install_function(void **real_func, void *hook_func)
{
    LONG err = DetourAttach(real_func, hook_func);
    return err == NO_ERROR;
}

hk_hook bool32
hk_lib_uninstall_function(void **real_func, void *hook_func)
{
    LONG err = DetourDetach(real_func, hook_func);
    return err == NO_ERROR;
}

hk_hook void *
hk_lib_find_function(String8 dll_name, String8 function_name)
{
    void *func_addr = DetourFindFunction(dll_name.str, function_name.str);
    return func_addr;
}

hk_hook bool32
hk_lib_register_all_threads_for_transaction(void)
{
    // NOTE(fbt): !!WARNING!! use of undocumented kernel
    // functions incoming. Because microsoft
    // doesn't want to offer reasonable thread enumeration
    // through regular APIs for some reason.

    Temp scratch = scratch_begin(0, 0);

    typedef struct Thread_Task Thread_Task;
    struct Thread_Task
    {
        Thread_Task *next;
        HANDLE handle;
    };
    Thread_Task *first_thread_task = 0;
    Thread_Task *last_thread_task  = 0;

    NTSTATUS status = 0;
    for(HANDLE thread = 0, next_thread = 0; status == 0;)
    {
        status = NtGetNextThread(GetCurrentProcess(), thread, THREAD_QUERY_INFORMATION, 0, 0, &next_thread);
        thread = next_thread;
        
        Thread_Task *task = push_array(scratch.arena, Thread_Task, 1);
        task->handle = thread;
        SLLQueuePush(first_thread_task, last_thread_task, task);
    }
    if(status == ERROR_NO_MORE_ITEMS)
    {
        HANDLE our_thread = GetCurrentThread();
        DWORD our_thread_id = GetThreadId(our_thread);
        for(Thread_Task *t = first_thread_task; t != 0; t = t->next)
        {
            // we must pass the pseudo-handle for our current thread who is executing this code
            if(GetThreadId(t->handle) == our_thread_id)
            {
                DetourUpdateThread(our_thread);
            }
            else
            {
                DetourUpdateThread(t->handle);
            }
        }
    }

    scratch_end(scratch);
    
    return status == ERROR_NO_MORE_ITEMS;
}


hk_hook bool32
hk_lib_restore_import_table(void)
{
    return DetourRestoreAfterWith();
}


