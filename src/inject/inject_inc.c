// Copyright (c) Frances Telfar
// Licensed under the PolyForm Noncommercial License (https://polyformproject.org/licenses/noncommercial/1.0.0)

#include "inject.c"

#if OS_WINDOWS
# include "win32/inject/win32_inject.c"
#elif OS_LINUX
# include "linux/inject/linux_inject.c"
#else
# error Operating system backed not defined for the inject layer.
#endif
