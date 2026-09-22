// Copyright (c) Frances Telfar
// Licensed under the PolyForm Noncommercial License (https://polyformproject.org/licenses/noncommercial/1.0.0)

////////////////////////////
// Error Printing Helpers

internal void w32_net_print_winsock_error(const char *msg)
{
    int errorCode = WSAGetLastError();
    LPSTR errorString = NULL;
    DWORD size =
        FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,
                       NULL,
                       errorCode,
                       MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                       (LPSTR)&errorString,
                       0,
                       NULL);

    if (size > 0 && errorString != NULL) {
        fprintf(stderr, "%s: %s (Error Code: %d)\n", msg, errorString, errorCode);
        LocalFree(errorString);
    } else {
        printf("%s (Failed to retrieve string. Error code: %d)\n", msg, errorCode);
    }
}


/////////////////////////////////////////////
// Networking Conversion Helpers

internal NET_Endpoint w32_net_endpoint_from_sockaddr_storage(SOCKADDR_STORAGE storage)
{
    NET_Endpoint ep = {0};
    switch (storage.ss_family)
    {
        default:
        case AF_INET: {
            SOCKADDR_IN *addr = (SOCKADDR_IN *)&storage;
            
            ep.kind = NET_EndpointKind_IPv4;
            ep.port = net_to_host_u16(addr->sin_port);
            ep.address.u16[5] = 0xffff;
            ep.address.u32[3] = net_to_host_u32(addr->sin_addr.S_un.S_addr);
        } break;
        case AF_INET6: {
            // The msdn documentation states that every field of the
            // SOCKADDR_IN6_LH (LH is for "longhorn", and old vista version)
            // should be in network byte order, except for the family.
            SOCKADDR_IN6_LH *addr = (SOCKADDR_IN6_LH *)&storage;

            ep.kind = NET_EndpointKind_IPv4;
            ep.port = net_to_host_u16(addr->sin6_port);
            MemoryCopyArray(ep.address.u8, addr->sin6_addr.u.Byte);
        } break;
    }
    return ep;
}

internal SOCKADDR_STORAGE w32_net_sockaddr_storage_from_endpoint(NET_Endpoint ep)
{
    SOCKADDR_STORAGE storage = {0};
    switch (ep.kind)
    {
        default:
        case NET_EndpointKind_IPv4: {
            SOCKADDR_IN *addr = (SOCKADDR_IN *)&storage;

            addr->sin_family = AF_INET;
            addr->sin_port = host_to_net_u16(ep.port);
            addr->sin_addr.S_un.S_addr = host_to_net_u32(ep.address.u32[3]);
        } break;
        case NET_EndpointKind_IPv6: {
            SOCKADDR_IN6_LH *addr = (SOCKADDR_IN6_LH *)&storage;

            addr->sin6_family = AF_INET6;
            addr->sin6_port = host_to_net_u16(ep.port);
            addr->sin6_flowinfo = 0;
            MemoryCopyArray(addr->sin6_addr.u.Byte, ep.address.u8);
            addr->sin6_scope_id = 0;
        } break;
    }
    return storage;
}

internal void w32_net_tcp_listeners_thread_entry_point(void *p)
{
    ThreadnameF("w32_net_tcp_listeners_thread");
    for(;;)
    {
        // TODO
    }
}

internal void w32_net_udp_listeners_thread_entry_point(void *p)
{
    ThreadNameF("w32_net_udp_listeners_thread");
    for(;;)
    {
        // TODO
    }
}

/////////////////////////////////////////////
// @per_os_impl Network Listener Functions

internal u16 net_listener_alloc(u16 port)
{
    NOTIMPL_WARNING(net_listener_alloc);
}

internal u16 net_listener_close(u16 port)
{
    NOTIMPL_WARNING(net_listener_close);
}

////////////////////////////
// @per_os_impl Top-Level Layer Calls

internal void net_init(void)
{
    // NOTE: winsock2 is already initialized by the base layer for RIO function grabbing.

    // set up state
    Arena *arena = arena_alloc();
    w32_net_state = push_array(arena, W32_NET_State, 1);
    w32_net_state->arena = arena;
    w32_net_state->u2s_ring = guarded_ring_alloc(arena, Kilobytes(256));
    w32_net_state->s2u_ring = guarded_ring_alloc(arena, Kilobytes(256));

    // launch one-off listeners threads to block & accept connections
    w32_net_state->tcp_listeners_thread = thread_launch(w32_net_tcp_listeners_thread_entry_point, 0);
    w32_net_state->udp_listeners_thread = thread_launch(w32_net_udp_listeners_thread_entry_point, 0);
}

internal void net_async_tick(void)
{
    NOTIMPL_WANRING(net_async_tick);
}

//////////////////////////
// @per_os_impl Sends

internal u64 net_send(u8 *ptr, u64 size, NET_Protocol *protocol_in, NET_Endpoint *endpoint_in, u64 endt_us)
{
    u64 result = 0;
    u64 needed_size = sizeof(*protocol_in) + sizeof(endpoint_in) + sizeof(u64) + size;
    Ring_Guard guard = guarded_ring_open(w32_net_state->u2s_ring);
    {
        void *dst = guarded_ring_push_or_wait(&guard, needed_size, endt_us);
        if(dst != 0)
        {
            MemoryCopy((u8 *)dst + 0, protocol_in, sizeof(*protocol_in));
            MemoryCopy((u8 *)dst + sizeof(*protocol_in), endpoint_in, sizeof(*endpoint_in));
            MemoryCopy((u8 *)dst + sizeof(*protocol_in) + sizeof(*endpoint_in), &size, sizeof(size));
            MemoryCopy((u8 *)dst + sizeof(*protocol_in) + sizeof(*endpoint_in) + sizeof(u64), ptr, size);
            result = needed_size;
        }
    }
    guarded_ring_close(&guard);
    return result;
}
