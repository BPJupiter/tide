// Copyright (c) Frances Telfar
// Licensed under the PolyForm Noncommercial License (https://polyformproject.org/licenses/noncommercial/1.0.0)

#ifndef INJECTION_H
#define INJECTION_H

///////////////////////
// @per_os_impl DLL Injection

internal bool32 inj_dll_into_pid(u64 process_id, String8 dll_path);

#endif // INJECTION_H
