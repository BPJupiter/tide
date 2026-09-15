// Copyright (c) Frances Telfar
// Licensed under the PolyForm Noncommercial License (https://polyformproject.org/licenses/noncommercial/1.0.0)


#ifndef DNS_INC_H
#define DNS_INC_H

#include "dns_core.h"
#include "dns_pack.h"

#if OS_WINDOWS
# include "win32/dns/win32_dns.h"
#elif OS_LINUX
# include "linux/dns/linux_dns.h"
#else
# error Operating system backed not defined for the dns layer.
#endif

#endif // DNS_INC_H
