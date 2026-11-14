// Copyright (c) Frances Telfar
// Licensed under the PolyForm Noncommercial License (https://polyformproject.org/licenses/noncommercial/1.0.0)

#ifndef DETOURS_H
#define DETOURS_H

///////////////////////
// @per_os_impl Process Memory Allocation

internal void *dtr_reserve_memory(Process process, u64 size);
internal bool32 dtr_commit_memory(Process process, void *ptr, u64 size);

///////////////////////
// @per_os_impl DLL Detours

internal bool32 dtr_dll_into_pid(u64 process_id, String8 dll_path);

#endif // DETOURS_H
