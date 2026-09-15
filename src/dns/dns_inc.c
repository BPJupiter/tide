// Copyright (c) Frances Telfar
// Licensed under the PolyForm Noncommercial License (https://polyformproject.org/licenses/noncommercial/1.0.0)


#include "dns_core.c"
#include "dns_pack.c"

#if OS_WINDOWS
# include "win32/dns/win32_dns.c"
#elif OS_LINUX
# include "linux/dns/linux_dns.c"
#else
# error Opearting system backend not found for the dns layer.
#endif
