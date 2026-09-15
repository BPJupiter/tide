// Copyright (c) Frances Telfar
// Licensed under the PolyForm Noncommercial License (https://polyformproject.org/licenses/noncommercial/1.0.0)

#ifndef NET_INC_H
#define NET_INC_H

//////////////
// Includes

#include "net_core.h"

#if OS_WINDOWS
# include "win32/net/win32_net.h"
#elif OS_LINUX
# include "linux/net/linux_net.h"
#else
# error Operating system backend not defined for the net layer.
#endif

#endif // NET_INC_H
