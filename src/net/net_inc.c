// Copyright (c) Frances Telfar
// Licensed under the PolyForm Noncommercial License (https://polyformproject.org/licenses/noncommercial/1.0.0)

//////////////
// Includes

#include "net.c"

#if OS_WINDOWS
# include "win32/net/win32_net.c"
#elif OS_LINUX
# include "linux/net/linux_net.c"
#else
# include "net_stub.c"
#endif
