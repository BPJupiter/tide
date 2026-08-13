
/////////////////////////////////////////
// @per_os_impl System DNS Info

internal String8_List dns_get_local_nameservers(Arena *arena)
{
    String8_List result = {0};
    Temp scratch = scratch_begin(&arena, 1);

    u32 i = 0;
    DWORD ret = 0;

    ULONG flags = GAA_FLAG_SKIP_UNICAST | GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST;
    ULONG family = AF_UNSPEC;
    LPVOID lpMsgBuf = 0;
    PIP_ADAPTER_ADDRESSES pAddresses = 0;
    ULONG bufferSize = Kilobytes(16);
    ULONG Iterations = 0;

    PIP_ADAPTER_ADDRESSES pCurrAddresses = 0;
    PIP_ADAPTER_UNICAST_ADDRESS pUnicast = 0;
    PIP_ADAPTER_ANYCAST_ADDRESS pAnycast = 0;
    PIP_ADAPTER_MULTICAST_ADDRESS pMulticast = 0;
    PIP_ADAPTER_DNS_SERVER_ADDRESS pDns = 0;
    PIP_ADAPTER_PREFIX pPrefix = 0;

    do {
        pAddresses = (PIP_ADAPTER_ADDRESSES)push_array(scratch.arena, u8, bufferSize);
        ret = GetAdaptersAddresses(family, flags, 0, pAddresses, &bufferSize);
        Iterations++;
    } while ((ERROR_BUFFER_OVERFLOW == ret) && (Iterations < 3));

    if (NO_ERROR == ret) {
        pCurrAddresses = pAddresses;
        while (pCurrAddresses) {
            // This suffix length check is to filter out the loopback
            // which is always active and has no suffix.
            // This MAY break because I have no idea what standard
            // practice is around dns suffix advertisement.
            if ((pCurrAddresses->OperStatus == IfOperStatusUp)
                && (0 != wcslen(pCurrAddresses->DnsSuffix))) {

                pDns = pCurrAddresses->FirstDnsServerAddress;
                if (pDns) {
                    for (i = 0; pDns != 0; i++) {
                        SOCKADDR *sa = pDns->Address.lpSockaddr;
                        char ipStr[INET6_ADDRSTRLEN] = {0};
                            
                        if (sa->sa_family == AF_INET) {
                            SOCKADDR_IN *sa_in = (SOCKADDR_IN *)sa;
                            inet_ntop(AF_INET, &(sa_in->sin_addr), ipStr, sizeof(ipStr));
                            str8_list_pushf(arena, &result, "%s", ipStr);
                        }
                        else if (sa->sa_family == AF_INET6) {
                            SOCKADDR_IN6 *sa_in6 = (SOCKADDR_IN6 *)sa;
                            inet_ntop(AF_INET6, &(sa_in6->sin6_addr), ipStr, sizeof(ipStr));
                            str8_list_pushf(arena, &result, "%s", ipStr);
                        }
                        pDns = pDns->Next;
                    }
                }
            }
            pCurrAddresses = pCurrAddresses->Next;
        }
    }

    scratch_end(scratch);
    return result;
}
