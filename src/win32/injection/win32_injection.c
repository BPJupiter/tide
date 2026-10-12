
///////////////////////
// @per_os_impl DLL Injection

internal bool32
inj_dll_into_pid(u64 process_id, String8 dll_path)
{
    // open target process with access right
    HANDLE process_handle = OpenProcess(PROCESS_ALL_ACCESS, 0, process_id);
    if(!process_handle)
    {
        return 0;
    }

    // alloc memory within the target process for our DLL path
    LPVOID dll_path_mem = VirtualAllocEx(process_handle, 0, dll_path.size+1, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
    if(!dll_path_mem)
    {
        CloseHandle(process_handle);
        return 0;
    }

    if(!WriteProcessMemory(process_handle, dll_path_mem, (LPVOID)dll_path.str, dll_path.size+1, 0))
    {
        VirtualFreeEx(process_handle, dll_path_mem, 0, MEM_RELEASE);
        CloseHandle(process_handle);
        return 0;
    }

    // get the address of LoadLibraryA for kernel32.dll
    LPTHREAD_START_ROUTINE load_library_a = (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
    if(!load_library_a)
    {
        VirtualFreeEx(process_handle, dll_path_mem, 0, MEM_RELEASE);
        CloseHandle(process_handle);
        return 0;
    }

    // create remote thread within the target process that runs LoadLibraryA(dll_path);
    HANDLE thread_handle = CreateRemoteThread(process_handle, 0, 0, load_library_a, dll_path_mem, 0, 0);
    if(!thread_handle)
    {
        VirtualFreeEx(process_handle, dll_path_mem, 0, MEM_RELEASE);
        CloseHandle(process_handle);
        return 0;
    }

    CloseHandle(thread_handle);
    CloseHandle(process_handle);
    return 1;
}
