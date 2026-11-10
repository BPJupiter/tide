// Copyright (c) Frances Telfar
// Licensed under the PolyForm Noncommercial License (https://polyformproject.org/licenses/noncommercial/1.0.0)

#ifndef INJECT_INC_H
#define INJECT_INC_H

#include "inject.h"

#if OS_WINDOWS
# include "win32/inject/win32_inject.h"
#elif OS_LINUX
# include "linux/inject/linux_inject.h"
#else
# error Operating system backed not defined for the inject layer.
#endif

#endif // INJECT_INC_H
