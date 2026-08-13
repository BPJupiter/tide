

#include "dns_core.c"
#include "dns_client.c"

#if OS_WINDOWS
# include "win32/dns/win32_dns.c"
#elif OS_LINUX
# include "linux/dns/linux_dns.c"
#else
# error Opearting system backend not found for the dns layer.
#endif
