
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
