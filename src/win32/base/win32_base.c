
///////////////////////////////////////////////////////////////////////////////////
//  Modern Windows SDK functions
//
// (we must dynamically link to them, since they can be missing in older SDKs)

typedef HRESULT W32_SetThreadDescription_Type(HANDLE hThread, PCWSTR lpThreadDescription);
global          W32_SetThreadDescription_Type *w32_SetThreadDescription_func = 0;
typedef BOOL    W32_InitializeSynchronizationBarrier_Type(W32_SYNCHRONIZATION_BARRIER *lpBarrier, LONG lTotalThreads, LONG lSpinCount);
global          W32_InitializeSynchronizationBarrier_Type *w32_InitializeSynchronizationBarrier_func = 0;
typedef BOOL    W32_DeleteSynchronizationBarrier_Type(W32_SYNCHRONIZATION_BARRIER *lpBarrier);
global          W32_DeleteSynchronizationBarrier_Type *w32_DeleteSynchronizationBarrier_func = 0;
typedef BOOL    W32_EnterSynchronizationBarrier_Type(W32_SYNCHRONIZATION_BARRIER *lpBarrier, DWORD dwFlags);
global          W32_EnterSynchronizationBarrier_Type *w32_EnterSynchronizationBarrier_func = 0;

global RIO_EXTENSION_FUNCTION_TABLE w32_rio_functions = {0};

////////////////////////////////////
// File Info Conversaion Helpers

internal FilePropertyFlags w32_file_property_flags_from_dwFileAttributes(DWORD dwFileAttributes)
{
    FilePropertyFlags flags = 0;
    if (dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        flags |= FilePropertyFlag_IsFolder;
    }
    return flags;
}

internal void w32_file_properties_from_attribute_data(File_Properties *properties, WIN32_FILE_ATTRIBUTE_DATA *attributes)
{
    properties->size = Compose64Bit(attributes->nFileSizeHigh, attributes->nFileSizeLow);
    w32_dense_time_from_file_time(&properties->created, &attributes->ftCreationTime);
    w32_dense_time_from_file_time(&properties->modified, &attributes->ftLastWriteTime);
    properties->flags = w32_file_property_flags_from_dwFileAttributes(attributes->dwFileAttributes);
}

//////////////////////////////
// Time Conversion Helpers

internal void w32_date_time_from_system_time(Date_Time *out, SYSTEMTIME *in)
{
    out->year = in->wYear;
    out->mon  = in->wMonth - 1;
    out->wday = in->wDayOfWeek;
    out->day  = in->wDay;
    out->hour = in->wHour;
    out->min  = in->wMinute;
    out->sec  = in->wSecond;
    out->msec = in->wMilliseconds;
}

internal void w32_system_time_from_date_time(SYSTEMTIME *out, Date_Time *in)
{
    out->wYear         = (WORD)(in->year);
    out->wMonth        = in->min + 1;
    out->wDayOfWeek    = in->wday;
    out->wDay          = in->day;
    out->wHour         = in->hour;
    out->wMinute       = in->min;
    out->wSecond       = in->sec;
    out->wMilliseconds = in->msec;
}

internal void w32_dense_time_from_file_time(Dense_Time *out, FILETIME *in)
{
    SYSTEMTIME systime = {0};
    FileTimeToSystemTime(in, &systime);
    Date_Time date_time = {0};
    w32_date_time_from_system_time(&date_time, &systime);
    *out = dense_time_from_date_time(date_time);
}

internal u32 w32_sleep_ms_from_endt_us(u64 endt_us)
{
    u32 sleep_ms = 0;
    if (endt_us == max_u64) {
        sleep_ms = INFINITE;
    }
    else {
        u64 begint = now_time_us();
        if (begint < endt_us) {
            u64 sleep_us = endt_us - begint;
            sleep_ms = (u32)((sleep_us + 999) / 1000);
        }
    }

    return sleep_ms;
}

internal u32 w32_unix_time_from_file_time(FILETIME file_time)
{
    u64 win32_time = ((u64)file_time.dwHighDateTime << 32) | file_time.dwLowDateTime;
    u64 unix_time64 = ((win32_time - 0x19DB1DED53E8000ULL) / 10000000);

    Assert(unix_time64 <= max_u32);
    u32 unix_time32 = (u32)unix_time64;

    return unix_time32;
}

///////////////////////
// Entity Functions

internal W32_Entity *w32_entity_alloc(W32_EntityKind kind)
{
    W32_Entity *result = 0;
    EnterCriticalSection(&w32_state.entity_mutex);
    {
        result = w32_state.entity_free;
        if(result) {
            SLLStackPop(w32_state.entity_free);
        }
        else {
            result = push_array_no_zero(w32_state.entity_arena, W32_Entity, 1);
        }
        MemoryZeroStruct(result);
    }
    LeaveCriticalSection(&w32_state.entity_mutex);
    result->kind = kind;
    return result;
}

internal void w32_entity_release(W32_Entity *entity)
{
    entity->kind = W32_EntityKind_Null;
    EnterCriticalSection(&w32_state.entity_mutex);
    SLLStackPush(w32_state.entity_free, entity);
    LeaveCriticalSection(&w32_state.entity_mutex);
}

/////////////////////////
// Thread Entry Point

internal DWORD w32_thread_entry_point(void *ptr)
{
    W32_Entity *entity = (W32_Entity *)ptr;
    Thread_Entry_Point_Function_Type *func = entity->thread.func;
    void *thread_ptr = entity->thread.ptr;
    supplement_thread_base_entry_point(func, thread_ptr);
    return 0;
}

////////////////////////////////////////////////
// @per_os_impl Debugger Attachment Checking

internal bool32 debugger_is_attached(void)
{
    bool32 result = IsDebuggerPresent();
    return result;
}

///////////////////////////////////////////
// @per_os_impl Platform Time Functions

internal u64 now_time_us(void)
{
    u64 result = 0;
    LARGE_INTEGER large_int_counter;
    if (QueryPerformanceCounter(&large_int_counter)) {
        result = (large_int_counter.QuadPart * Million(1)) / w32_state.microsecond_resolution;
    }
    return result;
}

internal u32 now_time_unix(void)
{
    FILETIME file_time;
    GetSystemTimeAsFileTime(&file_time);
    u32 unix_time = w32_unix_time_from_file_time(file_time);
    return unix_time;
}

internal Date_Time now_time_universal(void)
{
    SYSTEMTIME systime = {0};
    GetSystemTime(&systime);
    Date_Time result;
    w32_date_time_from_system_time(&result, &systime);
    return result;
}

internal Date_Time universal_from_local_time(Date_Time *date_time)
{
    SYSTEMTIME systime = {0};
    w32_system_time_from_date_time(&systime, date_time);
    FILETIME ftime = {0};
    SystemTimeToFileTime(&systime, &ftime);
    FILETIME ftime_local = {0};
    LocalFileTimeToFileTime(&ftime, &ftime_local);
    FileTimeToSystemTime(&ftime_local, &systime);
    Date_Time result;
    w32_date_time_from_system_time(&result, &systime);
    return result;
}

internal Date_Time local_from_universal_time(Date_Time *date_time)
{
    SYSTEMTIME systime = {0};
    w32_system_time_from_date_time(&systime, date_time);
    FILETIME ftime = {0};
    SystemTimeToFileTime(&systime, &ftime);
    FILETIME ftime_local = {0};
    FileTimeToLocalFileTime(&ftime, &ftime_local);
    FileTimeToSystemTime(&ftime_local, &systime);
    Date_Time result = {0};
    w32_date_time_from_system_time(&result, &systime);
    return result;
}

internal void sleep_ms(u32 msec)
{
    Sleep(msec);
}

///////////////////////////////////////////
// @per_os_impl Platform GUID Functions

internal Guid make_guid(void)
{
    Guid result;
    MemoryZeroStruct(&result);
    UUID uuid;
    RPC_STATUS rpc_status = UuidCreate(&uuid);
    if(rpc_status == RPC_S_OK) {
        result.data1 = uuid.Data1;
        result.data2 = uuid.Data2;
        result.data3 = uuid.Data3;
        MemoryCopyArray(result.data4, uuid.Data4);
    }
    return result;
}

//////////////////////////////////////////////
// @per_os_impl Platform Memory Allocation

// basic
internal void *reserve_memory(u64 size)
{
    void *result = VirtualAlloc(0, size, MEM_RESERVE, PAGE_READWRITE);
    return result;
}

internal bool32 commit_memory(void *ptr, u64 size)
{
    bool32 result = (VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE) != 0);
    if (w32_rio_functions.RIORegisterBuffer) {
        // wine does not implement these functions
        w32_rio_functions.RIODeregisterBuffer(w32_rio_functions.RIORegisterBuffer(ptr, size));
    }
#if PROFILE_TELEMETRY
    tmAlloc(0, ptr, size / 1024, "Win32 Commit");
#endif
    return result;
}

internal void decommit_memory(void *ptr, u64 size)
{
    VirtualFree(ptr, size, MEM_DECOMMIT);
#if PROFILE_TELEMETRY
    tmFree(0, ptr);
#endif
}

internal void release_memory(void *ptr, u64 size)
{
    // Size not used. Not necessary on Windows, but necessary for other OSes
    VirtualFree(ptr, 0, MEM_RELEASE);
}

// large pages
internal void *reserve_memory_large(u64 size)
{
    // We commit on reserve because Windows.
    void *result = VirtualAlloc(0, size, MEM_RESERVE|MEM_COMMIT|MEM_LARGE_PAGES, PAGE_READWRITE);
    return result;
}

internal bool32 commit_memory_large(void *ptr, u64 size)
{
    return true;
}

/////////////////////////////////
// @per_os_impl Shared Memory

internal Shared_Memory shared_memory_alloc(u64 size, String8 name) {
    Temp scratch = scratch_begin(0, 0);
    String16 name16 = str16_from_8(scratch.arena, name);
    HANDLE file = CreateFileMappingW(INVALID_HANDLE_VALUE,
                                     0,
                                     PAGE_READWRITE,
                                     (u32)((size & 0xffffffff00000000) >> 32),
                                     (u32)((size & 0x00000000ffffffff)),
                                     (WCHAR *)name16.str);
    Shared_Memory result = {(u64)file};
    scratch_end(scratch);
    return result;
}

internal Shared_Memory shared_memory_open(String8 name)
{
    Temp scratch = scratch_begin(0, 0);
    String16 name16 = str16_from_8(scratch.arena, name);
    HANDLE file = OpenFileMappingW(FILE_MAP_ALL_ACCESS, 0, (WCHAR *)name16.str);
    Shared_Memory result = {(u64)file};
    scratch_end(scratch);
    return result;
}

internal void shared_memory_close(Shared_Memory handle)
{
    HANDLE file = (HANDLE)(handle.u64[0]);
    CloseHandle(file);
}

internal void *shared_memory_view_open(Shared_Memory handle, Rng1u64 range)
{
    HANDLE file = (HANDLE)(handle.u64[0]);
    u64 offset = range.min;
    u64 size = range.max-range.min;
    void *ptr = MapViewOfFile(file, FILE_MAP_ALL_ACCESS,
                              (u32)((offset & 0xffffffff00000000) >> 32),
                              (u32)((offset & 0x00000000ffffffff)),
                              size);
    return ptr;
}

internal void shared_memory_view_close(Shared_Memory handle, void *ptr, Rng1u64 range)
{
    UnmapViewOfFile(ptr);
}

///////////////////////////////
// @per_os_impl System Info

internal System_Info *get_system_info(void)
{
    return &w32_state.system_info;
}

///////////////////////////////////////
// @per_os_impl Current Thread Info

internal u32 tid(void)
{
    DWORD id = GetCurrentThreadId();
    return (u32)id;
}

internal void set_platform_thread_name(String8 name)
{
    Temp scratch = scratch_begin(0, 0);
    
    // rjf: windows 10 style
    if(w32_SetThreadDescription_func) {
        String16 name16 = str16_from_8(scratch.arena, name);
        HRESULT hr = w32_SetThreadDescription_func(GetCurrentThread(), (WCHAR*)name16.str);
    }
    
    // rjf: raise-exception style
    {
        String8 name_copy = push_str8_copy(scratch.arena, name);
#pragma pack(push,8)
        typedef struct THREADNAME_INFO THREADNAME_INFO;
        struct THREADNAME_INFO {
            u32 dwType;     // Must be 0x1000.
            char *szName;   // Pointer to name (in user addr space).
            u32 dwThreadID; // Thread ID (-1=caller thread).
            u32 dwFlags;    // Reserved for future use, must be zero.
        };
#pragma pack(pop)
        THREADNAME_INFO info;
        info.dwType = 0x1000;
        info.szName = (char *)name_copy.str;
        info.dwThreadID = tid();
        info.dwFlags = 0;
#pragma warning(push)
#pragma warning(disable: 6320 6322)
        __try
        {
            RaiseException(0x406D1388, 0, sizeof(info) / sizeof(void *), (const ULONG_PTR *)&info);
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
        }
#pragma warning(pop)
    }
  
    scratch_end(scratch);
}

////////////////////////////////////
// @per_os_impl Thread Functions

internal Thread thread_launch(Thread_Entry_Point_Function_Type *f, void *p)
{
    W32_Entity *entity = w32_entity_alloc(W32_EntityKind_Thread);
    entity->thread.func = f;
    entity->thread.ptr = p;
    entity->thread.handle = CreateThread(0, 0, w32_thread_entry_point, entity, 0, &entity->thread.tid);
    Thread result = {IntFromPtr(entity)};
    return result;
}

internal bool32 thread_join(Thread thread, u64 endt_us) {
    DWORD sleep_ms = w32_sleep_ms_from_endt_us(endt_us);
    W32_Entity *entity = (W32_Entity *)PtrFromInt(thread.u64[0]);
    DWORD wait_result = WAIT_OBJECT_0;
    if(entity != 0) {
        wait_result = WaitForSingleObject(entity->thread.handle, sleep_ms);
        CloseHandle(entity->thread.handle);
        w32_entity_release(entity);
    }
    return (wait_result == WAIT_OBJECT_0);
}

internal void thread_detach(Thread thread)
{
    W32_Entity *entity = (W32_Entity*)PtrFromInt(thread.u64[0]);
    if(entity != 0) {
        CloseHandle(entity->thread.handle);
        w32_entity_release(entity);
    }
}

//////////////////////////////
// @per_os_impl Safe Calls

internal void safe_call(Thread_Entry_Point_Function_Type *func, Thread_Entry_Point_Function_Type *fail_handler, void *ptr)
{
    __try {
        func(ptr);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        if(fail_handler != 0)
        {
            fail_handler(ptr);
        }
    }
}

///////////////////////////////////////////////////////
// @per_os_impl Synchronization Primitive Functions

//- rjf: recursive mutexes

internal Mutex mutex_alloc(void)
{
    W32_Entity *entity = w32_entity_alloc(W32_EntityKind_Mutex);
    InitializeCriticalSection(&entity->mutex);
    Mutex result = {IntFromPtr(entity)};
    return result;
}

internal void mutex_release(Mutex mutex)
{
    W32_Entity *entity = (W32_Entity*)PtrFromInt(mutex.u64[0]);
    DeleteCriticalSection(&entity->mutex);
    w32_entity_release(entity);
}

internal void mutex_take(Mutex mutex)
{
    W32_Entity *entity = (W32_Entity*)PtrFromInt(mutex.u64[0]);
    EnterCriticalSection(&entity->mutex);
}

internal void mutex_drop(Mutex mutex)
{
    W32_Entity *entity = (W32_Entity*)PtrFromInt(mutex.u64[0]);
    LeaveCriticalSection(&entity->mutex);
}

//- rjf: reader/writer mutexes

internal RWMutex rw_mutex_alloc(void)
{
    W32_Entity *entity = w32_entity_alloc(W32_EntityKind_RWMutex);
    InitializeSRWLock(&entity->rw_mutex);
    RWMutex result = {IntFromPtr(entity)};
    return result;
}

internal void rw_mutex_release(RWMutex rw_mutex)
{
    W32_Entity *entity = (W32_Entity*)PtrFromInt(rw_mutex.u64[0]);
    w32_entity_release(entity);
}

internal void rw_mutex_take(RWMutex rw_mutex, bool32 write_mode)
{
    W32_Entity *entity = (W32_Entity*)PtrFromInt(rw_mutex.u64[0]);
    if(write_mode) {
        AcquireSRWLockExclusive(&entity->rw_mutex);
    }
    else {
        AcquireSRWLockShared(&entity->rw_mutex);
    }
}

internal void rw_mutex_drop(RWMutex rw_mutex, bool32 write_mode)
{
    W32_Entity *entity = (W32_Entity*)PtrFromInt(rw_mutex.u64[0]);
    if(write_mode) {
        ReleaseSRWLockExclusive(&entity->rw_mutex);
    }
    else {
        ReleaseSRWLockShared(&entity->rw_mutex);
    }
}

//- rjf: condition variables

internal CondVar cond_var_alloc(void)
{
    W32_Entity *entity = w32_entity_alloc(W32_EntityKind_ConditionVariable);
    InitializeConditionVariable(&entity->cv);
    CondVar result = {IntFromPtr(entity)};
    return result;
}

internal void cond_var_release(CondVar cv)
{
    W32_Entity *entity = (W32_Entity*)PtrFromInt(cv.u64[0]);
    w32_entity_release(entity);
}

internal bool32 cond_var_wait(CondVar cv, Mutex mutex, u64 endt_us)
{
    u32 sleep_ms = w32_sleep_ms_from_endt_us(endt_us);
    BOOL result = 0;
    if(sleep_ms > 0) {
        W32_Entity *entity = (W32_Entity*)PtrFromInt(cv.u64[0]);
        W32_Entity *mutex_entity = (W32_Entity*)PtrFromInt(mutex.u64[0]);
        result = SleepConditionVariableCS(&entity->cv, &mutex_entity->mutex, sleep_ms);
    }
    return result;
}

internal bool32 cond_var_wait_rw(CondVar cv, RWMutex mutex_rw, bool32 write_mode, u64 endt_us)
{
    u32 sleep_ms = w32_sleep_ms_from_endt_us(endt_us);
    BOOL result = 0;
    if(sleep_ms > 0) {
        W32_Entity *entity = (W32_Entity*)PtrFromInt(cv.u64[0]);
        W32_Entity *mutex_entity = (W32_Entity*)PtrFromInt(mutex_rw.u64[0]);
        result = SleepConditionVariableSRW(&entity->cv, &mutex_entity->rw_mutex, sleep_ms,
                                           write_mode ? 0 : CONDITION_VARIABLE_LOCKMODE_SHARED);
    }
    return result;
}

internal void cond_var_signal(CondVar cv)
{
    W32_Entity *entity = (W32_Entity*)PtrFromInt(cv.u64[0]);
    WakeConditionVariable(&entity->cv);
}

internal void cond_var_broadcast(CondVar cv)
{
    W32_Entity *entity = (W32_Entity*)PtrFromInt(cv.u64[0]);
    WakeAllConditionVariable(&entity->cv);
}

//- rjf: cross-process semaphores

internal Semaphore semaphore_alloc(u32 initial_count, u32 max_count, String8 name)
{
    Temp scratch = scratch_begin(0, 0);
    String16 name16 = str16_from_8(scratch.arena, name);
    HANDLE handle = CreateSemaphoreW(0, initial_count, max_count, (WCHAR *)name16.str);
    Semaphore result = {(u64)handle};
    scratch_end(scratch);
    return result;
}

internal void semaphore_release(Semaphore semaphore)
{
    HANDLE handle = (HANDLE)semaphore.u64[0];
    CloseHandle(handle);
}

internal Semaphore semaphore_open(String8 name)
{
    Temp scratch = scratch_begin(0, 0);
    String16 name16 = str16_from_8(scratch.arena, name);
    HANDLE handle = OpenSemaphoreW(SEMAPHORE_ALL_ACCESS, 0, (WCHAR *)name16.str);
    Semaphore result = {(u64)handle};
    scratch_end(scratch);
    return result;
}

internal void semaphore_close(Semaphore semaphore)
{
    HANDLE handle = (HANDLE)semaphore.u64[0];
    CloseHandle(handle);
}

internal bool32 semaphore_take(Semaphore semaphore, u64 endt_us)
{
    u32 sleep_ms = w32_sleep_ms_from_endt_us(endt_us);
    HANDLE handle = (HANDLE)semaphore.u64[0];
    DWORD wait_result = WaitForSingleObject(handle, sleep_ms);
    bool32 result = (wait_result == WAIT_OBJECT_0);
    return result;
}

internal void semaphore_drop_count(Semaphore semaphore, u64 drop_count)
{
    ReleaseSemaphore((HANDLE)*semaphore.u64, drop_count, 0);
}

//- rjf: barriers

internal Barrier barrier_alloc(u64 count)
{
    Barrier result = {0};
    if(w32_InitializeSynchronizationBarrier_func != 0) {
        W32_Entity *entity = w32_entity_alloc(W32_EntityKind_Barrier);
        if(entity != 0) {
            BOOL init_good = w32_InitializeSynchronizationBarrier_func(&entity->sb, count, -1);
            (void)init_good;
        }
        result.u64[0] = IntFromPtr(entity);
    }
    else {
        result = slow_barrier_alloc(count);
    }
    return result;
}

internal void barrier_release(Barrier barrier)
{
    if(w32_InitializeSynchronizationBarrier_func != 0) {
        W32_Entity *entity = (W32_Entity*)PtrFromInt(barrier.u64[0]);
        if(entity != 0) {
            w32_DeleteSynchronizationBarrier_func(&entity->sb);
            w32_entity_release(entity);
        }
    }
    else {
        slow_barrier_release(barrier);
    }
}

internal void barrier_wait(Barrier barrier)
{
    if(w32_InitializeSynchronizationBarrier_func != 0) {
        W32_Entity *entity = (W32_Entity*)PtrFromInt(barrier.u64[0]);
        if(entity != 0) {
            w32_EnterSynchronizationBarrier_func(&entity->sb, 0);
        }
    }
    else {
        slow_barrier_wait(barrier);
    }
}

///////////////////////////////
// @per_os_impl File System

internal File
file_open(AccessFlags flags, String8 path)
{
    File result = {0};
    Temp scratch = scratch_begin(0, 0);
    String16 path16 = str16_from_8(scratch.arena, path);
    DWORD access_flags = 0;
    DWORD share_mode = 0;
    DWORD creation_disposition = OPEN_EXISTING;
    SECURITY_ATTRIBUTES security_attributes = {sizeof(security_attributes), 0, 0};
    if(flags & AccessFlag_Read)        {access_flags |= GENERIC_READ;}
    if(flags & AccessFlag_Write)       {access_flags |= GENERIC_WRITE;}
    if(flags & AccessFlag_Execute)     {access_flags |= GENERIC_EXECUTE;}
    if(flags & AccessFlag_ShareRead)   {share_mode |= FILE_SHARE_READ;}
    if(flags & AccessFlag_ShareWrite)  {share_mode |= FILE_SHARE_WRITE|FILE_SHARE_DELETE;}
    if(flags & AccessFlag_Write)       {creation_disposition = CREATE_ALWAYS;}
    if(flags & AccessFlag_Append)      {creation_disposition = OPEN_ALWAYS; access_flags |= FILE_APPEND_DATA; }
    if(flags & AccessFlag_Inherited) {
        security_attributes.bInheritHandle = 1;
    }
    HANDLE file = CreateFileW((WCHAR *)path16.str, access_flags, share_mode, &security_attributes, creation_disposition, FILE_ATTRIBUTE_NORMAL, 0);
    if(file != INVALID_HANDLE_VALUE) {
        result.u64[0] = (u64)file;
    }
    else {
        DWORD err = GetLastError();
        (void)err;
    }
    scratch_end(scratch);
    return result;
}

internal void file_close(File file)
{
    if(file_match(file, file_zero())) { return; }
    HANDLE handle = (HANDLE)file.u64[0];
    BOOL result = CloseHandle(handle);
    (void)result;
}

internal u64 file_read(File file, Rng1u64 rng, void *out_data)
{
    if(file_match(file, file_zero())) { return 0; }
  
    HANDLE  handle = (HANDLE)file.u64[0];
    u8     *ptr    = out_data;
    u64     off    = rng.min;
    while(off != rng.max) {
        u64        amt64      = rng.max - off;
        u32        amt32      = (u32)Min(MB(32), amt64);
        DWORD      read_size  = 0;
        OVERLAPPED overlapped = { .Offset = (u32)off, .OffsetHigh = (u32)(off >> 32) };
        if( ! ReadFile(handle, ptr, amt32, &read_size, &overlapped)) {
            break;
        }
        ptr += read_size;
        off += read_size;
    }
    
    u64 total_read_size = off - rng.min;
    return total_read_size;
}

internal u64 file_write(File file, Rng1u64 rng, void *data)
{
    if(file_match(file, file_zero())) { return 0; }
    HANDLE win_handle = (HANDLE)file.u64[0];
    u64 src_off = 0;
    u64 dst_off = rng.min;
    u64 total_write_size = dim_1u64(rng);
    for(;;) {
        void *bytes_src = (u8 *)data + src_off;
        u64 bytes_left = total_write_size - src_off;
        DWORD write_size = Min(MB(1), bytes_left);
        DWORD bytes_written = 0;
        OVERLAPPED overlapped = {0};
        overlapped.Offset = (dst_off&0x00000000ffffffffull);
        overlapped.OffsetHigh = (dst_off&0xffffffff00000000ull) >> 32;
        BOOL success = WriteFile(win_handle, bytes_src, write_size, &bytes_written, &overlapped);
        if(success == 0)
        {
            break;
        }
        src_off += bytes_written;
        dst_off += bytes_written;
        if(bytes_left == 0) {
            break;
        }
    }
    return src_off;
}

internal bool32 file_set_time(File file, Date_Time time)
{
    if(file_match(file, file_zero())) { return 0; }
    bool32 result = 0;
    HANDLE handle = (HANDLE)file.u64[0];
    SYSTEMTIME system_time = {0};
    w32_system_time_from_date_time(&system_time, &time);
    FILETIME file_time = {0};
    result = (SystemTimeToFileTime(&system_time, &file_time) &&
              SetFileTime(handle, &file_time, &file_time, &file_time));
    return result;
}

internal File_Properties properties_from_file(File file)
{
    if(file_match(file, file_zero())) { File_Properties r = {0}; return r; }
    File_Properties props = {0};
    HANDLE handle = (HANDLE)file.u64[0];
    BY_HANDLE_FILE_INFORMATION info;
    BOOL info_good = GetFileInformationByHandle(handle, &info);
    if(info_good) {
        u32 size_lo = info.nFileSizeLow;
        u32 size_hi = info.nFileSizeHigh;
        props.size     = (u64)size_lo | (((u64)size_hi)<<32);
        w32_dense_time_from_file_time(&props.modified, &info.ftLastWriteTime);
        w32_dense_time_from_file_time(&props.created, &info.ftCreationTime);
        props.flags = w32_file_property_flags_from_dwFileAttributes(info.dwFileAttributes);
    }
    return props;
}

internal File_ID id_from_file(File file)
{
    if(file_match(file, file_zero())) { File_ID r = {0}; return r; }
    File_ID result = {0};
    HANDLE handle = (HANDLE)file.u64[0];
    BY_HANDLE_FILE_INFORMATION info;
    BOOL is_ok = GetFileInformationByHandle(handle, &info);
    if(is_ok) {
        result.v[0] = info.dwVolumeSerialNumber;
        result.v[1] = info.nFileIndexLow;
        result.v[2] = info.nFileIndexHigh;
    }
    return result;
}

internal bool32 file_reserve_size(File file, u64 size)
{
    HANDLE handle = (HANDLE)file.u64[0];
    
    FILE_ALLOCATION_INFO alloc_info    = {0};
    alloc_info.AllocationSize.LowPart  = size & max_u32;
    alloc_info.AllocationSize.HighPart = (size >> 32) & max_u32;
    
    BOOL is_reserved = SetFileInformationByHandle(handle, FileAllocationInfo, &alloc_info, sizeof(alloc_info));
    return is_reserved;
}

internal bool32 delete_file_at_path(String8 path)
{
    Temp scratch = scratch_begin(0, 0);
    String16 path16 = str16_from_8(scratch.arena, path);
    bool32 result = DeleteFileW((WCHAR*)path16.str);
    scratch_end(scratch);
    return result;
}

internal bool32 copy_file_path(String8 dst, String8 src)
{
    Temp scratch = scratch_begin(0, 0);
    String16 dst16 = str16_from_8(scratch.arena, dst);
    String16 src16 = str16_from_8(scratch.arena, src);
    bool32 result = CopyFileW((WCHAR*)src16.str, (WCHAR*)dst16.str, 0);
    scratch_end(scratch);
    return result;
}

internal bool32 move_file_path(String8 dst, String8 src)
{
    Temp scratch = scratch_begin(0, 0);
    String16 dst16 = str16_from_8(scratch.arena, dst);
    String16 src16 = str16_from_8(scratch.arena, src);
    bool32 result = MoveFileW((WCHAR*)src16.str, (WCHAR*)dst16.str);
    scratch_end(scratch);
    return result;
}

internal String8 full_path_from_path(Arena *arena, String8 path)
{
    Temp scratch = scratch_begin(&arena, 1);
    DWORD     buffer_size = Max(MAX_PATH, path.size * 2) + 1;
    String16  path16      = str16_from_8(scratch.arena, path);
    WCHAR    *buffer      = push_array_no_zero(scratch.arena, WCHAR, buffer_size);
    DWORD     path16_size = GetFullPathNameW((WCHAR*)path16.str, buffer_size, buffer, NULL);
    if(path16_size > buffer_size) {
        arena_pop(scratch.arena, buffer_size);
        buffer_size = path16_size + 1;
        buffer      = push_array_no_zero(scratch.arena, WCHAR, buffer_size);
        path16_size = GetFullPathNameW((WCHAR*)path16.str, buffer_size, buffer, NULL);
    }
    String8 full_path = str8_from_16(arena, str16((u16*)buffer, path16_size));
    scratch_end(scratch);
    return full_path;
}

internal bool32 file_path_exists(String8 path)
{
    Temp scratch = scratch_begin(0,0);
    String16 path16 = str16_from_8(scratch.arena, path);
    DWORD attributes = GetFileAttributesW((WCHAR *)path16.str);
    bool32 exists = (attributes != INVALID_FILE_ATTRIBUTES) && !!(~attributes & FILE_ATTRIBUTE_DIRECTORY);
    scratch_end(scratch);
    return exists;
}

internal bool32 folder_path_exists(String8 path)
{
    Temp scratch = scratch_begin(0,0);
    String16 path16     = str16_from_8(scratch.arena, path);
    DWORD    attributes = GetFileAttributesW((WCHAR *)path16.str);
    bool32      exists     = (attributes != INVALID_FILE_ATTRIBUTES) && (attributes & FILE_ATTRIBUTE_DIRECTORY);
    scratch_end(scratch);
    return exists;
}

internal File_Properties properties_from_file_path(String8 path)
{
    WIN32_FIND_DATAW find_data = {0};
    Temp scratch = scratch_begin(0, 0);
    String16 path16 = str16_from_8(scratch.arena, path);
    HANDLE handle = FindFirstFileW((WCHAR *)path16.str, &find_data);
    File_Properties props = {0};
    if(handle != INVALID_HANDLE_VALUE) {
        props.size = Compose64Bit(find_data.nFileSizeHigh, find_data.nFileSizeLow);
        w32_dense_time_from_file_time(&props.created, &find_data.ftCreationTime);
        w32_dense_time_from_file_time(&props.modified, &find_data.ftLastWriteTime);
        props.flags = w32_file_property_flags_from_dwFileAttributes(find_data.dwFileAttributes);
    }
    else {
        Temp scratch = scratch_begin(0, 0);
        WCHAR buffer[512] = {0};
        DWORD length = GetLogicalDriveStringsW(sizeof(buffer), buffer);
        u64 last_slash_pos = 0;
        for(;last_slash_pos < path.size; last_slash_pos = str8_find_needle(path, last_slash_pos+1, str8_lit("/"), StringMatchFlag_SlashInsensitive));
        String8 path_trimmed = str8_prefix(path, last_slash_pos);
        for(u64 off = 0; off < (u64)length;) {
            String16 next_drive_string_16 = str16_cstring((u16 *)buffer+off);
            off += next_drive_string_16.size+1;
            String8 next_drive_string = str8_from_16(scratch.arena, next_drive_string_16);
            next_drive_string = str8_chop_last_slash(next_drive_string);
            if(str8_match(path_trimmed, next_drive_string, StringMatchFlag_CaseInsensitive)) {
                props.flags |= FilePropertyFlag_IsFolder;
                break;
            }
        }
        scratch_end(scratch);
    }
    FindClose(handle);
    scratch_end(scratch);
    return props;
}

//- rjf: file maps

internal File_Map file_map_open(AccessFlags flags, File file)
{
    File_Map map = {0};
    {
        HANDLE file_handle = (HANDLE)file.u64[0];
        DWORD protect_flags = 0;
        {
            switch(flags)
            {
                default:{}break;
                case AccessFlag_Read:
                    {protect_flags = PAGE_READONLY;}break;
                case AccessFlag_Write:
                case AccessFlag_Read|AccessFlag_Write:
                    {protect_flags = PAGE_READWRITE;}break;
                case AccessFlag_Execute:
                case AccessFlag_Read|AccessFlag_Execute:
                    {protect_flags = PAGE_EXECUTE_READ;}break;
                case AccessFlag_Execute|AccessFlag_Write|AccessFlag_Read:
                case AccessFlag_Execute|AccessFlag_Write:
                    {protect_flags = PAGE_EXECUTE_READWRITE;}break;
            }
        }
        HANDLE map_handle = CreateFileMappingA(file_handle, 0, protect_flags, 0, 0, 0);
        map.u64[0] = (u64)map_handle;
    }
    return map;
}

internal void file_map_close(File_Map map)
{
    HANDLE handle = (HANDLE)map.u64[0];
    BOOL result = CloseHandle(handle);
    (void)result;
}

internal void *file_map_view_open(File_Map map, AccessFlags flags, Rng1u64 range)
{
    HANDLE handle = (HANDLE)map.u64[0];
    u32 off_lo = (u32)((range.min&0x00000000ffffffffull)>>0);
    u32 off_hi = (u32)((range.min&0xffffffff00000000ull)>>32);
    u64 size = dim_1u64(range);
    DWORD access_flags = 0;
    {
        switch(flags)
        {
            default:{}break;
            case AccessFlag_Read:
                {
                    access_flags = FILE_MAP_READ;
                }break;
            case AccessFlag_Write:
                {
                    access_flags = FILE_MAP_WRITE;
                }break;
            case AccessFlag_Read|AccessFlag_Write:
                {
                    access_flags = FILE_MAP_ALL_ACCESS;
                }break;
            case AccessFlag_Execute:
            case AccessFlag_Read|AccessFlag_Execute:
            case AccessFlag_Write|AccessFlag_Execute:
            case AccessFlag_Read|AccessFlag_Write|AccessFlag_Execute:
                {
                    access_flags = FILE_MAP_ALL_ACCESS|FILE_MAP_EXECUTE;
                }break;
        }
    }
    void *result = MapViewOfFile(handle, access_flags, off_hi, off_lo, size);
    DWORD err = GetLastError();
    return result;
}

internal void file_map_view_close(File_Map map, void *ptr, Rng1u64 range)
{
    BOOL result = UnmapViewOfFile(ptr);
    (void)result;
}

//- rjf: directory iteration

internal File_Iter *file_iter_begin(Arena *arena, String8 path, FileIterFlags flags)
{
    Temp scratch = scratch_begin(&arena, 1);
    String8 path_with_wildcard = push_str8_cat(scratch.arena, path, str8_lit("\\*"));
    String16 path16 = str16_from_8(scratch.arena, path_with_wildcard);
    File_Iter *iter = push_array(arena, File_Iter, 1);
    iter->flags = flags;
    W32_File_Iter *w32_iter = (W32_File_Iter*)iter->memory;
    if(path.size == 0) {
        w32_iter->is_volume_iter = 1;
        WCHAR buffer[512] = {0};
        DWORD length = GetLogicalDriveStringsW(sizeof(buffer), buffer);
        String8_List drive_strings = {0};
        for(u64 off = 0; off < (u64)length;)
        {
            String16 next_drive_string_16 = str16_cstring((u16 *)buffer+off);
            off += next_drive_string_16.size+1;
            String8 next_drive_string = str8_from_16(arena, next_drive_string_16);
            next_drive_string = str8_chop_last_slash(next_drive_string);
            str8_list_push(scratch.arena, &drive_strings, next_drive_string);
        }
        w32_iter->drive_strings = str8_array_from_list(arena, &drive_strings);
        w32_iter->drive_strings_iter_idx = 0;
    }
  else {
      w32_iter->handle = FindFirstFileExW((WCHAR*)path16.str, FindExInfoBasic, &w32_iter->find_data, FindExSearchNameMatch, 0, FIND_FIRST_EX_LARGE_FETCH);
  }
    scratch_end(scratch);
    return iter;
}

internal bool32 file_iter_next(Arena *arena, File_Iter *iter, File_Info *info_out)
{
    bool32 result = 0;
    FileIterFlags flags = iter->flags;
    W32_File_Iter *w32_iter = (W32_File_Iter*)iter->memory;
    switch(w32_iter->is_volume_iter)
    {
        //- rjf: file iteration
        default:
        case 0: {
            if (!(flags & FileIterFlag_Done) && w32_iter->handle != INVALID_HANDLE_VALUE) {
                do {
                    // check is usable
                    bool32 usable_file = 1;
                    
                    WCHAR *file_name = w32_iter->find_data.cFileName;
                    DWORD attributes = w32_iter->find_data.dwFileAttributes;
                    if (file_name[0] == '.'){
                        if (flags & FileIterFlag_SkipHiddenFiles){
                            usable_file = 0;
                        }
                        else if (file_name[1] == 0){
                            usable_file = 0;
                        }
                        else if (file_name[1] == '.' && file_name[2] == 0){
                            usable_file = 0;
                        }
                    }
                    if (attributes & FILE_ATTRIBUTE_DIRECTORY){
                        if (flags & FileIterFlag_SkipFolders){
                            usable_file = 0;
                        }
                    }
                    else{
                        if (flags & FileIterFlag_SkipFiles){
                            usable_file = 0;
                        }
                    }
                    
                    // emit if usable
                    if (usable_file){
                        info_out->name = str8_from_16(arena, str16_cstring((u16*)file_name));
                        info_out->props.size = (u64)w32_iter->find_data.nFileSizeLow | (((u64)w32_iter->find_data.nFileSizeHigh)<<32);
                        w32_dense_time_from_file_time(&info_out->props.created,  &w32_iter->find_data.ftCreationTime);
                        w32_dense_time_from_file_time(&info_out->props.modified, &w32_iter->find_data.ftLastWriteTime);
                        info_out->props.flags = w32_file_property_flags_from_dwFileAttributes(attributes);
                        result = 1;
                        if (!FindNextFileW(w32_iter->handle, &w32_iter->find_data)){
                            iter->flags |= FileIterFlag_Done;
                        }
                        break;
                    }
                }while(FindNextFileW(w32_iter->handle, &w32_iter->find_data));
            }
        }break;
            
            //- rjf: volume iteration
        case 1: {
            result = w32_iter->drive_strings_iter_idx < w32_iter->drive_strings.count;
            if(result != 0)
            {
                MemoryZeroStruct(info_out);
                info_out->name = w32_iter->drive_strings.v[w32_iter->drive_strings_iter_idx];
                info_out->props.flags |= FilePropertyFlag_IsFolder;
                w32_iter->drive_strings_iter_idx += 1;
            }
        }break;
    }
    if(!result) {
        iter->flags |= FileIterFlag_Done;
    }
    return result;
}

internal void file_iter_end(File_Iter *iter)
{
    W32_File_Iter *w32_iter = (W32_File_Iter*)iter->memory;
    HANDLE zero_handle;
    MemoryZeroStruct(&zero_handle);
    if(!MemoryMatchStruct(&zero_handle, &w32_iter->handle)) {
        FindClose(w32_iter->handle);
    }
}

//- rjf: directory creation

internal bool32
make_directory(String8 path)
{
  bool32 result = 0;
  Temp scratch = scratch_begin(0, 0);
  String16 name16 = str16_from_8(scratch.arena, path);
  WIN32_FILE_ATTRIBUTE_DATA attributes = {0};
  GetFileAttributesExW((WCHAR*)name16.str, GetFileExInfoStandard, &attributes);
  if(attributes.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
  {
    result = 1;
  }
  else if(CreateDirectoryW((WCHAR*)name16.str, 0))
  {
    result = 1;
  }
  scratch_end(scratch);
  return(result);
}

////////////////////////////
// @per_os_impl Aborting

internal void abort_self(u64 exit_code)
{
    ExitProcess((int)exit_code);
}

////////////////////////////////
// @per_os_impl Process Info

internal Process_Info *get_process_info(void)
{
    return &w32_state.process_info;
}

internal String8 get_current_path(Arena *arena)
{
    Temp scratch = scratch_begin(&arena, 1);
    DWORD length = GetCurrentDirectoryW(0, 0);
    u16 *memory = push_array_no_zero(scratch.arena, u16, length + 1);
    length = GetCurrentDirectoryW(length + 1, (WCHAR *)memory);
    String8 name = str8_from_16(arena, str16(memory, length));
    scratch_end(scratch);
    return name;
}

internal u32 get_process_start_time_unix(void)
{
    u32 result = 0;
    HANDLE handle = GetCurrentProcess();
    FILETIME start_time = {0};
    FILETIME exit_time;
    FILETIME kernel_time;
    FILETIME user_time;
    if (GetProcessTimes(handle, &start_time, &exit_time, &kernel_time, &user_time)) {
        result = w32_unix_time_from_file_time(start_time);
    }
    return result;
}

///////////////////////////////////
// @per_os_impl Child Processes

internal Process process_launch(Process_Launch_Params *params)
{
    Process result = {0};
    Temp scratch = scratch_begin(0, 0);
    
    //- rjf: form full command string
    String8 cmd = {0};
    {
        String_Join join_params = {0};
        join_params.pre = str8_lit("\"");
        join_params.sep = str8_lit("\" \"");
        join_params.post = str8_lit("\"");
        cmd = str8_list_join(scratch.arena, &params->cmd_line, &join_params);
    }
    
  //- rjf: form environment
    bool32 use_null_env_arg = 0;
    String8 env = {0};
    {
        String_Join join_params2 = {0};
        join_params2.sep = str8_lit("\0");
        join_params2.post = str8_lit("\0");
        String8_List all_opts = params->env;
        if(params->inherit_env != 0) {
            if(all_opts.node_count != 0) {
                MemoryZeroStruct(&all_opts);
                for(String8_Node *n = params->env.first; n != 0; n = n->next) {
                    str8_list_push(scratch.arena, &all_opts, n->string);
                }
                for(String8_Node *n = w32_state.process_info.environment.first; n != 0; n = n->next) {
                    str8_list_push(scratch.arena, &all_opts, n->string);
                }
            }
            else {
                use_null_env_arg = 1;
            }
        }
        if(use_null_env_arg == 0) {
            env = str8_list_join(scratch.arena, &all_opts, &join_params2);
        }
    }
    
    //- rjf: utf-8 -> utf-16
    String16 cmd16 = str16_from_8(scratch.arena, cmd);
    String16 dir16 = str16_from_8(scratch.arena, params->path);
    String16 env16 = {0};
    if(use_null_env_arg == 0) {
        env16 = str16_from_8(scratch.arena, env);
    }
    
    //- rjf: determine creation flags
    DWORD creation_flags = CREATE_UNICODE_ENVIRONMENT;
    if(params->consoleless) {
        creation_flags |= CREATE_NO_WINDOW;
    }
    
    //- rjf: launch
    BOOL inherit_handles = 0;
    STARTUPINFOW startup_info = {sizeof(startup_info)};
    if(!file_match(params->stdout_file, file_zero())) {
        HANDLE stdout_handle = (HANDLE)params->stdout_file.u64[0];
        startup_info.hStdOutput = stdout_handle;
        startup_info.dwFlags |= STARTF_USESTDHANDLES;
        inherit_handles = 1;
    }
    if(!file_match(params->stderr_file, file_zero())) {
        HANDLE stderr_handle = (HANDLE)params->stderr_file.u64[0];
        startup_info.hStdError = stderr_handle;
        startup_info.dwFlags |= STARTF_USESTDHANDLES;
        inherit_handles = 1;
    }
    if(!file_match(params->stdin_file, file_zero())) {
        HANDLE stdin_handle = (HANDLE)params->stdin_file.u64[0];
        startup_info.hStdInput = stdin_handle;
        startup_info.dwFlags |= STARTF_USESTDHANDLES;
        inherit_handles = 1;
    }
    PROCESS_INFORMATION process_info = {0};
    if(CreateProcessW(0, (WCHAR*)cmd16.str, 0, 0, inherit_handles, creation_flags, use_null_env_arg ? 0 : (WCHAR*)env16.str, (WCHAR*)dir16.str, &startup_info, &process_info)) {
        result.u64[0] = (u64)process_info.hProcess;
        CloseHandle(process_info.hThread);
    }
    
    scratch_end(scratch);
    return result;
}

internal u64 pid_from_process(Process process)
{
    HANDLE process_handle = (HANDLE)process.u64[0];
    u64 result = (u64)GetProcessId(process_handle);
    return result;
}

internal bool32 process_join(Process process, u64 endt_us, u64 *exit_code_out)
{
    HANDLE process_handle = (HANDLE)(process.u64[0]);
    DWORD sleep_ms = w32_sleep_ms_from_endt_us(endt_us);
    DWORD result = WaitForSingleObject(process_handle, sleep_ms);
    bool32 process_joined = (result == WAIT_OBJECT_0);
    if(process_joined && exit_code_out) {
        DWORD exit_code = 0;
        if(GetExitCodeProcess(process_handle, &exit_code)) {
            *exit_code_out = exit_code;
        }
    }
    if(process_joined) {
        CloseHandle(process_handle);
    }
    return process_joined;
}

internal void process_detach(Process process)
{
    HANDLE process_handle = (HANDLE)(process.u64[0]);
    CloseHandle(process_handle);
}

internal bool32 process_kill(Process process)
{
    HANDLE process_handle = (HANDLE)process.u64[0];
    BOOL was_terminated = TerminateProcess(process_handle, 999);
    return was_terminated;
}

////////////////////////////////////////////////
// @per_os_impl Dynamically-Loaded Libraries

internal Library library_open(String8 path)
{
    Temp scratch = scratch_begin(0, 0);
    String16 path16 = str16_from_8(scratch.arena, path);
    HMODULE mod = LoadLibraryW((LPCWSTR)path16.str);
    Library result = { (u64)mod };
    scratch_end(scratch);
    return result;
}

internal void library_close(Library lib)
{
    HMODULE mod = (HMODULE)lib.u64[0];
    FreeLibrary(mod);
}

internal Void_Proc *library_load_proc(Library lib, String8 name)
{
    Temp scratch = scratch_begin(0, 0);
    HMODULE mod = (HMODULE)lib.u64[0];
    name = push_str8_copy(scratch.arena, name);
    Void_Proc *result = (Void_Proc*)GetProcAddress(mod, (LPCSTR)name.str);
    scratch_end(scratch);
    return result;
}

/////////////////////////////////////
// @per_os_impl Networking Primitives

internal Net_Socket net_socket_alloc(Net_TransportProtocol protocol)
{
    W32_Entity *entity = w32_entity_alloc(W32_EntityKind_Socket);
    switch (protocol) {
        case Net_TransportProtocol_RAW: {
            entity->socket = socket(AF_INET, SOCK_RAW, 0);
        } break;
        case Net_TransportProtocol_TCP: {
            entity->socket = socket(AF_INET, SOCK_STREAM, 0);
        } break;
        case Net_TransportProtocol_UDP: {
            entity->socket = socket(AF_INET, SOCK_DGRAM, 0);
        } break;
        default: {
            entity->socket = socket(AF_INET, SOCK_RAW, 0);
        }
    }
    Net_Socket socket = {IntFromPtr(entity)};
    return socket;
}

internal void net_socket_release(Net_Socket socket)
{
    W32_Entity *entity = (W32_Entity *)PtrFromInt(socket.u64[0]);
    closesocket(entity->socket);
    w32_entity_release(entity);
}

/////////////////////////////////////////////
// @per_os_impl Network Listener Functions

internal Net_Listener net_listener_alloc(Net_TransportProtocol protocol, u16 port)
{
    struct sockaddr_in addr = {0};
    {
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port);
    }
    Net_Listener listener = {0};
    {
        listener.port = port;
        listener.protocol = protocol;
        listener.socket = net_socket_alloc(protocol);
    }
    W32_Entity *entity = (W32_Entity *)PtrFromInt(listener.socket.u64[0]);
    if (INVALID_SOCKET == entity->socket) {
        // TODO: Error handling
    }
    if (0 > bind(entity->socket, (struct sockaddr *)&addr, sizeof(addr))) {
        // TODO: Error handling
    }
    if (0 > listen(entity->socket, SOMAXCONN)) {
        // TODO: Error handling
    }
    return listener;
}

internal Net_Client net_listener_accept(Arena *arena, Net_Listener listener)
{
    struct sockaddr_in addr = {0};
    int addrlen = sizeof(addr);

    Net_Socket accept_socket = net_socket_alloc(listener.protocol);
    W32_Entity *accept_entity = (W32_Entity *)PtrFromInt(accept_socket.u64[0]);
    W32_Entity *listen_entity = (W32_Entity *)PtrFromInt(listener.socket.u64[0]);
    accept_entity->socket = accept(listen_entity->socket, (struct sockaddr *)&addr, &addrlen);
    if (INVALID_SOCKET == accept_entity->socket) {
        // TODO: Error handling
    }
    // This is yuck and currently creates a dummy socket that we have to release.
    // Might be worth duplicating the logic of net_client_alloc
    // if this ends up being a lot of overhead.
    Net_Client client = net_client_alloc(arena, listener.protocol);
    net_socket_release(client.socket);
    client.socket = accept_socket;
    
    client.address.ip.v4 = ntohl(addr.sin_addr.s_addr);
    client.address.address_type = Net_AddressType_Ipv4;
    client.address.port = ntohs(addr.sin_port);
    
    client.connected = true;
    return client;
}

internal void net_listener_close(Net_Listener listener)
{
    net_socket_release(listener.socket);
}

///////////////////////////////////////////
// @per_os_impl Network Client Functions

internal Net_Client net_client_alloc(Arena *arena, Net_TransportProtocol protocol)
{
    Net_Socket client_socket = net_socket_alloc(protocol);
    W32_Entity *entity = (W32_Entity *)PtrFromInt(client_socket.u64[0]);
    if (INVALID_SOCKET == entity->socket) {
        // TODO: Error handling
    }

    Net_Client client = {0};
    client.arena = arena;
    client.protocol = protocol;
    client.socket = client_socket; // Make this call the responsibility of the caller?
    
    // TODO: I need some sort of compass for how large to make these buffers.
    // Currently 16kB is a wild guess.
    client.read_buffer = make_ring(arena, Kilobytes(16));
    client.write_buffer = make_ring(arena, Kilobytes(16));
    return client;
}

internal Net_Client net_client_connect(Net_Client client, Net_Address target)
{
    struct sockaddr_in server_address = {0};
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(target.port);
    server_address.sin_addr.s_addr = htonl(target.ip.v4);

    W32_Entity *entity = (W32_Entity *)PtrFromInt(client.socket.u64[0]);
    if (SOCKET_ERROR == connect(entity->socket, (struct sockaddr *)&server_address, sizeof(server_address))) {
        // TODO: Error handling
    }
    client.address = target;
    client.connected = true;
    return client;
}

internal bool32 net_client_pack_raw(Net_Client client, u64 size, void *data)
{
    return ring_try_write(client.write_buffer, size, data);
}

internal bool32 net_client_unpack_raw(Net_Client client, u64 size, void *out)
{
    return ring_try_read(client.read_buffer, size, out);
}

internal void net_client_close(Net_Client client)
{
    net_socket_release(client.socket);
}

//////////////////
// Entry Point

#include <dbghelp.h>
#undef OS_WINDOWS // shlwapi uses its own OS_WINDOWS include inside
#include <shlwapi.h>

internal bool32 win32_g_is_quiet = false;
internal bool32 win32_g_gen_dump = false;

internal HRESULT WINAPI win32_dialog_callback(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam, LONG_PTR data)
{
    if(msg == TDN_HYPERLINK_CLICKED) {
        ShellExecuteW(NULL, L"open", (LPWSTR)lparam, NULL, NULL, SW_SHOWNORMAL);
    }
    return S_OK;
}

internal LONG WINAPI win32_exception_filter(EXCEPTION_POINTERS *exception_ptrs)
{
    if (win32_g_is_quiet) {
        ExitProcess(1);
    }

    static volatile LONG first = 0;
    if (InterlockedCompareExchange(&first, 1, 0) != 0) {
        // prevent failures in other threads to popup same message box
        // this handler just shows first thread that crashes
        // we are terminating afterwards anyway.
        for (;;) Sleep(1000);
    }

    WCHAR buffer[4096] = {0};
    int buflen = 0;

    DWORD exception_code = exception_ptrs->ExceptionRecord->ExceptionCode;
    buflen += wnsprintfW(buffer + buflen, ArrayCount(buffer) - buflen, L"A fatal exception (code 0x%x) occurred. The process is terminating.\n", exception_code);

    // load dbghelp dynamically just in case if it is missing
    BOOL (WINAPI *dbg_MiniDumpWriteDump)(HANDLE hProcess, WORD ProcessId, HANDLE hFile, MINIDUMP_TYPE DumpType, PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam, PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam, PMINIDUMP_CALLBACK_INFORMATION CallbackParam) = 0;
    HMODULE dbghelp = LoadLibrary("dbghelp.dll");
    if (dbghelp) {
        DWORD (WINAPI *dbg_SymSetOptions)(DWORD SymOptions);
        BOOL (WINAPI *dbg_SymInitializeW)(HANDLE hProcess, PCWSTR UserSearchPath, BOOL fInvadeProcess);
        BOOL (WINAPI *dbg_StackWalk64)(DWORD MachineType, HANDLE hProcess, HANDLE hThread,
                                       LPSTACKFRAME64 StackFrame, PVOID ContextRecord, PREAD_PROCESS_MEMORY_ROUTINE64 ReadMemoryRoutine,
                                       PFUNCTION_TABLE_ACCESS_ROUTINE64 FunctionTableAccessRoutine, PGET_MODULE_BASE_ROUTINE64 GetModuleBaseRoutine,
                                       PTRANSLATE_ADDRESS_ROUTINE64 TranslateAddress);
        PVOID (WINAPI *dbg_SymFunctionTableAccess64)(HANDLE hProcess, DWORD64 AddrBase);
        DWORD64 (WINAPI *dbg_SymGetModuleBase64)(HANDLE hProcess, DWORD64 qwAddr);
        BOOL (WINAPI *dbg_SymFromAddrW)(HANDLE hProcess, DWORD64 Address, PDWORD64 Displacement, PSYMBOL_INFOW Symbol);
        BOOL (WINAPI *dbg_SymGetLineFromAddrW64)(HANDLE hProcess, DWORD64 dwAddr, PDWORD pdwDisplacement, PIMAGEHLP_LINEW64 Line);
        BOOL (WINAPI *dbg_SymGetModuleInfoW64)(HANDLE hProcess, DWORD64 qwAddr, PIMAGEHLP_MODULEW64 ModuleInfo);

        *(FARPROC *)&dbg_SymSetOptions            = GetProcAddress(dbghelp, "SymSetOptions");
        *(FARPROC *)&dbg_SymInitializeW           = GetProcAddress(dbghelp, "SymInitializeW");
        *(FARPROC *)&dbg_StackWalk64              = GetProcAddress(dbghelp, "StackWalk64");
        *(FARPROC *)&dbg_SymFunctionTableAccess64 = GetProcAddress(dbghelp, "SymFunctionTableAccess64");
        *(FARPROC *)&dbg_SymGetModuleBase64       = GetProcAddress(dbghelp, "SymGetModuleBase64");
        *(FARPROC *)&dbg_SymFromAddrW             = GetProcAddress(dbghelp, "SymFromAddrW");
        *(FARPROC *)&dbg_SymGetLineFromAddrW64    = GetProcAddress(dbghelp, "SymGetLineFromAddrW64");
        *(FARPROC *)&dbg_SymGetModuleInfoW64      = GetProcAddress(dbghelp, "SymGetModuleInfoW64");

        if (dbg_SymSetOptions
            && dbg_SymInitializeW
            && dbg_StackWalk64
            && dbg_SymFunctionTableAccess64
            && dbg_SymGetModuleBase64
            && dbg_SymFromAddrW
            && dbg_SymGetLineFromAddrW64
            && dbg_SymGetModuleInfoW64) {
            HANDLE process = GetCurrentProcess();
            HANDLE thread = GetCurrentThread();
            CONTEXT context = *exception_ptrs->ContextRecord;

            WCHAR module_path[MAX_PATH];
            GetModuleFileNameW(NULL, module_path, ArrayCount(module_path));
            PathRemoveFileSpecW(module_path);

            dbg_SymSetOptions(SYMOPT_EXACT_SYMBOLS|SYMOPT_FAIL_CRITICAL_ERRORS|SYMOPT_LOAD_LINES|SYMOPT_UNDNAME);
            if (dbg_SymInitializeW(process, module_path, TRUE)) {
                // check that raddbg.pdb file is good
                bool32 raddbg_pdb_valid = 0;
                {
                    IMAGEHLP_MODULEW64 module = {0};
                    module.SizeOfStruct = sizeof(module);
                    if(dbg_SymGetModuleInfoW64(process, (DWORD64)&win32_exception_filter, &module))
                    {
                        raddbg_pdb_valid = (module.SymType == SymPdb);
                    }
                }
        
                if(!raddbg_pdb_valid)
                {
                    buflen += wnsprintfW(buffer + buflen, sizeof(buffer) - buflen,
                                         L"\nThe PDB debug information file for this executable is not valid or was not found. Please rebuild binary to get the call stack.\n");
                }
                else
                {
                    STACKFRAME64 frame = {0};
                    DWORD image_type;
#if defined(_M_AMD64)
                    image_type = IMAGE_FILE_MACHINE_AMD64;
                    frame.AddrPC.Offset = context.Rip;
                    frame.AddrPC.Mode = AddrModeFlat;
                    frame.AddrFrame.Offset = context.Rbp;
                    frame.AddrFrame.Mode = AddrModeFlat;
                    frame.AddrStack.Offset = context.Rsp;
                    frame.AddrStack.Mode = AddrModeFlat;
#elif defined(_M_ARM64)
                    image_type = IMAGE_FILE_MACHINE_ARM64;
                    frame.AddrPC.Offset = context.Pc;
                    frame.AddrPC.Mode = AddrModeFlat;
                    frame.AddrFrame.Offset = context.Fp;
                    frame.AddrFrame.Mode = AddrModeFlat;
                    frame.AddrStack.Offset = context.Sp;
                    frame.AddrStack.Mode = AddrModeFlat;
#else
#  error Arch not supported!
#endif
          
#if BUILD_CONSOLE_INTERFACE
                    buflen += wnsprintfW(buffer + buflen, ArrayCount(buffer) - buflen, L"\nCreate a new issue with this report at %S.\n\n", BUILD_ISSUES_LINK_STRING_LITERAL);
#else
                    buflen += wnsprintfW(buffer + buflen, ArrayCount(buffer) - buflen,
                                         L"\nPress Ctrl+C to copy this text to clipboard, then create a new issue at\n"
                                         L"<a href=\"%S\">%S</a>\n\n", BUILD_ISSUES_LINK_STRING_LITERAL, BUILD_ISSUES_LINK_STRING_LITERAL);
#endif
                    buflen += wnsprintfW(buffer + buflen, ArrayCount(buffer) - buflen, L"Call stack:\n");
          
                    u64 frame_offset = 0;
          
                    if (frame.AddrPC.Offset == 0)
                    {
                        // if IP address is 0 then most likely we have called indirectly on NULL function pointer
                        // which means no useful stack unwinding will happen, because there's no unwind info for address 0
                        // but we can try reading 8 bytes of return address from stack, and start unwinding there
            
                        ULONG_PTR hi, lo;
                        GetCurrentThreadStackLimits(&lo, &hi);
                        if (frame.AddrStack.Offset >= lo && frame.AddrStack.Offset <= hi - sizeof(void*))
                        {
                            frame.AddrPC.Offset = *(DWORD64*)frame.AddrStack.Offset - 1;
                            frame.AddrStack.Offset += sizeof(void*);
#if defined(_M_AMD64)
                            context.Rip = frame.AddrPC.Offset;
                            context.Rsp = frame.AddrStack.Offset;
#elif defined(_M_ARM64)
                            context.Pc = frame.AddrPC.Offset;
                            context.Sp = frame.AddrStack.Offset;
#endif
                        }
            
                        buflen += wnsprintfW(buffer + buflen, ArrayCount(buffer) - buflen, L"1. [NULL]\n");
                        frame_offset = 1;
                    }
          
                    for(u32 idx=0; ;idx++)
                    {
                        const u32 max_frames = 32;
                        if(idx == max_frames)
                        {
                            buflen += wnsprintfW(buffer + buflen, ArrayCount(buffer) - buflen, L"...");
                            break;
                        }
            
                        if(!dbg_StackWalk64(image_type, process, thread, &frame, &context, 0, dbg_SymFunctionTableAccess64, dbg_SymGetModuleBase64, 0))
                        {
                            break;
                        }
            
                        u64 address = frame.AddrPC.Offset;
                        if(address == 0)
                        {
                            break;
                        }
            
                        buflen += wnsprintfW(buffer + buflen, ArrayCount(buffer) - buflen, L"%u. [0x%I64x]", frame_offset + idx + 1, address);
            
                        struct {
                            SYMBOL_INFOW info;
                            WCHAR name[MAX_SYM_NAME];
                        } symbol = {0};
            
                        symbol.info.SizeOfStruct = sizeof(symbol.info);
                        symbol.info.MaxNameLen = MAX_SYM_NAME;
            
                        DWORD64 displacement = 0;
                        if(dbg_SymFromAddrW(process, address, &displacement, &symbol.info))
                        {
                            buflen += wnsprintfW(buffer + buflen, ArrayCount(buffer) - buflen, L" %s +%u", symbol.info.Name, (DWORD)displacement);
              
                            IMAGEHLP_LINEW64 line = {0};
                            line.SizeOfStruct = sizeof(line);
              
                            DWORD line_displacement = 0;
                            if(dbg_SymGetLineFromAddrW64(process, address, &line_displacement, &line))
                            {
                                buflen += wnsprintfW(buffer + buflen, ArrayCount(buffer) - buflen, L", %s line %u", PathFindFileNameW(line.FileName), line.LineNumber);
                            }
                        }
                        else
                        {
                            IMAGEHLP_MODULEW64 module = {0};
                            module.SizeOfStruct = sizeof(module);
                            if(dbg_SymGetModuleInfoW64(process, address, &module))
                            {
                                buflen += wnsprintfW(buffer + buflen, ArrayCount(buffer) - buflen, L" %s", module.ModuleName);
                            }
                        }
            
                        buflen += wnsprintfW(buffer + buflen, ArrayCount(buffer) - buflen, L"\n");
                    }
                }
            }
        }
    }
  
    buflen += wnsprintfW(buffer + buflen, ArrayCount(buffer) - buflen, L"\nVersion: %S%S", BUILD_VERSION_STRING_LITERAL, BUILD_GIT_HASH_STRING_LITERAL_APPEND);
  
    bool32 generate_crash_dump = win32_g_gen_dump;
#if BUILD_CONSOLE_INTERFACE
    fwprintf(stderr, L"\n--- Fatal Exception ---\n");
    fwprintf(stderr, L"%s\n\n", buffer);
#else
    int selected_button = 0;
    TASKDIALOG_BUTTON generate_dump = {1, L"Generate Crash Dump File"};
    TASKDIALOGCONFIG dialog = {0};
    dialog.cbSize = sizeof(dialog);
    dialog.dwFlags = TDF_SIZE_TO_CONTENT | TDF_ENABLE_HYPERLINKS | TDF_ALLOW_DIALOG_CANCELLATION;
    dialog.pszMainIcon = TD_ERROR_ICON;
    dialog.dwCommonButtons = TDCBF_CLOSE_BUTTON;
    dialog.pszWindowTitle = L"Fatal Exception";
    dialog.pszContent = buffer;
    dialog.pfCallback = &win32_dialog_callback;
    dialog.cButtons = 1;
    dialog.pButtons = &generate_dump;
    TaskDialogIndirect(&dialog, &selected_button, 0, 0);
    generate_crash_dump = (selected_button == generate_dump.nButtonID);
#endif
  
    if(dbg_MiniDumpWriteDump && generate_crash_dump)
    {
        WCHAR dump_file_path[MAX_PATH] = {0};
        SHGetFolderPathW(0, CSIDL_DESKTOP, 0, 0, dump_file_path);
        PathAppendW(dump_file_path, L"bplib_crash_dump.dmp");
        HANDLE file = CreateFileW(dump_file_path, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
        if(file != INVALID_HANDLE_VALUE)
        {
            MINIDUMP_EXCEPTION_INFORMATION info = {0};
            info.ThreadId = GetCurrentThreadId();
            info.ExceptionPointers = exception_ptrs;
            info.ClientPointers = FALSE;
            BOOL dump_successful = dbg_MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), file, MiniDumpNormal, &info, 0, 0);
            CloseHandle(file);
            if(dump_successful)
            {
#if !BUILD_CONSOLE_INTERFACE
                // opens explorer and selects file
                SFGAOF flags = 0;
                PIDLIST_ABSOLUTE list = 0;
                if (SUCCEEDED(SHParseDisplayName(dump_file_path, NULL, &list, 0, &flags)))
                {
                    SHOpenFolderAndSelectItems(list, 0, NULL, 0);
                    CoTaskMemFree(list);
                }
#endif
            }
        }
    }
  
    ExitProcess(1);
}

#undef OS_WINDOWS // shlwapi uses its own OS_WINDOWS include inside
#define OS_WINDOWS 1

internal void w32_entry_point_caller(int argc, WCHAR **wargv)
{
    SetUnhandledExceptionFilter(&win32_exception_filter);

    // Dynamically load windows functions which are not garuanteed in all SDKs
    {
        HMODULE module = LoadLibraryA("kernel32.dll");
        w32_SetThreadDescription_func = (W32_SetThreadDescription_Type *)GetProcAddress(module, "SetThreadDescription");
        w32_InitializeSynchronizationBarrier_func = (W32_InitializeSynchronizationBarrier_Type *)GetProcAddress(module, "InitializeSynchronizationBarrier");
        w32_DeleteSynchronizationBarrier_func = (W32_DeleteSynchronizationBarrier_Type *)GetProcAddress(module, "DeleteSynchronizationBarrier");
        w32_EnterSynchronizationBarrier_func = (W32_EnterSynchronizationBarrier_Type *)GetProcAddress(module, "EnterSynchronizationBarrier");
        if(w32_InitializeSynchronizationBarrier_func == 0) {
            w32_DeleteSynchronizationBarrier_func = 0;
            w32_EnterSynchronizationBarrier_func = 0;
        }
        FreeLibrary(module);
    }

    // Try to allow large pages if we can
    bool32 large_pages_allowed = false;
    {
        HANDLE token;
        if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY, &token)) {
            LUID luid;
            if (LookupPrivilegeValue(0, SE_LOCK_MEMORY_NAME, &luid)) {
                TOKEN_PRIVILEGES priv;
                priv.PrivilegeCount = 1;
                priv.Privileges[0].Luid = luid;
                priv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
                large_pages_allowed = !!AdjustTokenPrivileges(token, 0, &priv, sizeof(priv), 0, 0);
            }
            CloseHandle(token);
        }
    }

    // get RIO extension function table
    {
        // NOTE: need to get functions pointers to RIO functions, and that requires a dummy socket.
        WSADATA WinSockData;
        WSAStartup(MAKEWORD(2, 2), &WinSockData);
        GUID guid = WSAID_MULTIPLE_RIO;
        DWORD rio_byte;
        SOCKET Sock = socket(AF_UNSPEC, SOCK_STREAM, IPPROTO_TCP);
        WSAIoctl(Sock, SIO_GET_MULTIPLE_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), (void **)&w32_rio_functions, sizeof(w32_rio_functions), &rio_byte, 0, 0);
        closesocket(Sock);
    }

    // Get system info
    SYSTEM_INFO sysinfo = {0};
    GetSystemInfo(&sysinfo);

    // Set up non-dynamically-alloc'd state
    //
    // (we need to set up some basics before this layer can supply
    // memory allocation primitives)
    {
        w32_state.microsecond_resolution = 1;
        LARGE_INTEGER large_int_resolution;
        if (QueryPerformanceFrequency(&large_int_resolution)) {
            w32_state.microsecond_resolution = large_int_resolution.QuadPart;
        }
    }

    {
        System_Info *info = &w32_state.system_info;
        info->logical_processor_count  = (u64)sysinfo.dwNumberOfProcessors;
        info->page_size                = sysinfo.dwPageSize;
        info->large_page_size          = GetLargePageMinimum();
        info->allocation_granularity   = sysinfo.dwAllocationGranularity;
    }

    {
        Process_Info *info = &w32_state.process_info;
        info->large_pages_allowed = large_pages_allowed;
        info->pid                 = GetCurrentProcessId();
    }

    // extract arguments
    Arena *args_arena = arena_alloc(.reserve_size = Megabytes(1), .commit_size = Kilobytes(32));
    char **argv = push_array(args_arena, char *, argc);
    for (int i = 0; i < argc; i++) {
        String16 arg16 = str16_cstring((u16 *)wargv[i]);
        String8 arg8 = str8_from_16(args_arena, arg16);
        if (str8_match(arg8, str8_lit("--quiet"), StringMatchFlag_CaseInsensitive) ||
            str8_match(arg8, str8_lit("-quit"), StringMatchFlag_CaseInsensitive)) {
            win32_g_is_quiet = true;
        }

        if (str8_match(arg8, str8_lit("--large_pages"), StringMatchFlag_CaseInsensitive) ||
            str8_match(arg8, str8_lit("-large_pages"), StringMatchFlag_CaseInsensitive)) {
            arena_default_flags        = ArenaFlag_LargePages;
            arena_default_reserve_size = Max(Megabytes(64), w32_state.system_info.large_page_size);
            arena_default_commit_size  = arena_default_reserve_size;
        }

        if (str8_match(arg8, str8_lit("--gen_crash_dump"), StringMatchFlag_CaseInsensitive) ||
            str8_match(arg8, str8_lit("-gen_crash_dump"), StringMatchFlag_CaseInsensitive)) {
            win32_g_gen_dump = true;
        }
        argv[i] = (char *)arg8.str;
    }

    // Set up thread context
    TCTX *tctx = tctx_alloc();
    tctx_select(tctx);

    // Set up dynamically alloc'd state
    Arena *arena = arena_alloc();
    {
        w32_state.arena = arena;
        {
            System_Info *info = &w32_state.system_info;
            u8 buffer[MAX_COMPUTERNAME_LENGTH + 1] = {0};
            DWORD size = MAX_COMPUTERNAME_LENGTH + 1;
            if (GetComputerNameA((char *)buffer, &size)) {
                info->machine_name = push_str8_copy(arena, str8(buffer, size));
            }
        }
    }

    {
        Process_Info *info = &w32_state.process_info;
        {
            Temp scratch = scratch_begin(0, 0);
            DWORD size = Kilobytes(32);
            u16 *buffer = push_array_no_zero(scratch.arena, u16, size);
            DWORD length = GetModuleFileNameW(0, (WCHAR *)buffer, size);
            String8 name8 = str8_from_16(scratch.arena, str16(buffer, length));
            info->binary_file_path = push_str8_copy(arena, name8);
            info->binary_path = str8_chop_last_slash(info->binary_file_path);
            scratch_end(scratch);
        }
        info->initial_path = get_current_path(arena);
        {
            Temp scratch = scratch_begin(0, 0);
            u64 size = Kilobytes(32);
            u16 *buffer = push_array_no_zero(scratch.arena, u16, size);
            if (SUCCEEDED(SHGetFolderPathW(0, CSIDL_APPDATA, 0, 0, (WCHAR *)buffer))) {
                info->user_program_config_data_path = str8_from_16(arena, str16_cstring(buffer));
                info->user_program_cache_data_path = info->user_program_logs_data_path = info->user_program_config_data_path;
            }
            scratch_end(scratch);
        }
        
        {
            WCHAR *this_proc_env = GetEnvironmentStringsW();
            u64 start_idx = 0;
            for (u64 idx = 0;; idx += 1) {
                if (this_proc_env[idx] == 0) {
                    if (start_idx == idx) {
                        break;
                    }
                    else {
                        String16 string16 = str16((u16 *)this_proc_env + start_idx, idx - start_idx);
                        String8 string = str8_from_16(arena, string16);
                        str8_list_push(arena, &info->environment, string);
                        start_idx = idx + 1;
                    }
                }
            }
        }
    }

    // Set up entity storage
    InitializeCriticalSection(&w32_state.entity_mutex);
    w32_state.entity_arena = arena_alloc();

    // call into "real" entry point
    main_thread_base_entry_point(argc, argv);
}

#if BUILD_CONSOLE_INTERFACE
int wmain(int argc, WCHAR **argv)
{
    w32_entry_point_caller(argc, argv);
    return 0;
}
#else
int wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd)
{
    CoInitializeEx(0, COINIT_APARTMENTTHREADED);
    w32_entry_point_caller(__argc, __wargv);
    return 0;
}
#endif
