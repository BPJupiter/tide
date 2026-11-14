// Copyright (c) Frances Telfar
// Licensed under the PolyForm Noncommercial License (https://polyformproject.org/licenses/noncommercial/1.0.0)

#ifndef DETOURS_INC_H
#define DETOURS_INC_H

#include "detours.h"

#if OS_WINDOWS
# include "win32/detours/win32_detours.h"
#elif OS_LINUX
# include "linux/detours/linux_detours.h"
#else
# error Operating system backed not defined for the detours layer.
#endif

#endif // DETOURS_INC_H
