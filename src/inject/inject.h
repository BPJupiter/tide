// Copyright (c) Frances Telfar
// Licensed under the PolyForm Noncommercial License (https://polyformproject.org/licenses/noncommercial/1.0.0)

#ifndef INJECT_H
#define INJECT_H

///////////////////////
// @per_os_impl Process Memory Allocation

internal void *inj_reserve_memory(Process process, u64 size);
internal bool32 inj_commit_memory(Process process, void *ptr, u64 size);

///////////////////////
// @per_os_impl DLL Inject

internal bool32 inj_dll_into_pid(u64 process_id, String8 dll_path);

#endif // INJECT_H
