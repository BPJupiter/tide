// Copyright (c) Frances Telfar
// Licensed under the PolyForm Noncommercial License (https://polyformproject.org/licenses/noncommercial/1.0.0)

#include "detours.c"

#if OS_WINDOWS
# include "win32/detours/win32_detours.c"
#elif OS_LINUX
# include "linux/detours/linux_detours.c"
#else
# error Operating system backed not defined for the detours layer.
#endif
