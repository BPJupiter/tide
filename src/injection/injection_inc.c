// Copyright (c) Frances Telfar
// Licensed under the PolyForm Noncommercial License (https://polyformproject.org/licenses/noncommercial/1.0.0)

#include "injection.c"

#if OS_WINDOWS
# include "win32/injection/win32_injection.c"
#elif OS_LINUX
# include "linux/injection/linux_injection.c"
#else
# error Operating system backed not defined for the injection layer.
#endif
