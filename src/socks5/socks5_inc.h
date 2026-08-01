#ifndef SOCKS5_INC_H
#define SOCKS5_INC_H

//////////////
// Includes

#include "socks5_core.h"

//////////////////////////
// Third-Party Includes

#if OS_WINDOWS
# include "third_party/wepoll/wepoll.h"
#elif OS_LINUX
# include <sys/epoll.h>
#endif

#endif // SOCKS5_INC_H
