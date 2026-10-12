// Copyright (c) Frances Telfar
// Licensed under the PolyForm Noncommercial License (https://polyformproject.org/licenses/noncommercial/1.0.0)

#ifndef INJECTION_INC_H
#define INJECTION_INC_H

#include "injection.h"

#if OS_WINDOWS
# include "win32/injection/win32_injection.h"
#elif OS_LINUX
# include "linux/injection/linux_injection.h"
#else
# error Operating system backed not defined for the injection layer.
#endif

#endif // INJECTION_INC_H
