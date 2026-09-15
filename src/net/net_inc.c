// Copyright (c) Frances Telfar
// Licensed under the PolyForm Noncommercial License (https://polyformproject.org/licenses/noncommercial/1.0.0)

//////////////
// Includes

#include "net_core.c"

#if OS_WINDOWS
# include "win32/net/win32_net.c"
#elif OS_LINUX
# include "linux/net/linux_net.c"
#else
# error Operating system backend not found for the net layer.
#endif
