#ifndef DNS_INC_H
#define DNS_INC_H

#include "dns_core.h"
#include "dns_client.h"

#if OS_WINDOWS
# include "win32/dns/win32_dns.h"
#elif OS_LINUX
# include "linux/dns/linux_dns.h"
#else
# error Operating system backed not defined for the dns layer.
#endif

#endif // DNS_INC_H
