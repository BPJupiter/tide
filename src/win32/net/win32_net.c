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
        SOCKADDR_STORAGE client_addr = {0};
        u32 client_addr_size = sizeof(client_addr);
        W32_NET_Connection *conn = push_array(w32_net_state->arena, W32_NET_Connection, 1);
        // block to wait for a new connection
        conn->socket = accept(w32_net_state->tcp_listen_socket, (SOCKADDR *)&client_addr, &client_addr_size);
        
        // add the new connection to the connection list
        // 
        // there is a race condition when accessing the connection list
        // as net_async_tick can also create new connections.
        if (conn->socket != INVALID_SOCKET)
        {
            conn->protocol = NET_Protocol_TCP;
            conn->endpoint = w32_net_endpoint_from_sockaddr_storage(client_addr);
            SLLQueuePush(w32_net_state->conn_list.first, w32_net_state->conn_list.last, conn); // RACE CONDITION!!!!
        }
    }
}

internal void w32_net_udp_listeners_thread_entry_point(void *p)
{
    ThreadNameF("w32_net_udp_listeners_thread");
    Temp scratch = scratch_begin(0, 0);
    for(;;)
    {
        arena_clear(scratch.arena);
        String8 data = {0};
        data.size = (1 << 16); // maximum UDP payload
        data.str = push_array(scratch.arena, u8, data.size);
        
        SOCKADDR_STORAGE client_addr = {0};
        u32 client_addr_size = sizeof(client_addr);
        u64 bytes_got = recvfrom(w32_net_state->udp_listen_socket, data.str, data.size - 1, 0, (SOCKADDR *)&client_addr, &client_addr_size);
        data.str[bytes_got] = 0;

        NET_Protocol pr = NET_Protocol_UDP;
        NET_Endpoint ep = w32_net_endpoint_from_sockaddr_storage(client_addr);
        u64 needed_size = sizeof(pr) + sizeof(ep) + sizeof(u64) + bytes_got;
        Ring_Guard guard = guarded_ring_open(w32_net_state->s2u_ring);
        {
            void *dst = guarded_ring_push_or_wait(&guard, needed_size, max_u64);
            if (dst != 0)
            {
                MemoryCopy((u8 *)dst + 0, &pr, sizeof(pr));
                MemoryCopy((u8 *)dst + sizeof(pr), &ep, sizeof(ep));
                MemoryCopy((u8 *)dst + sizeof(pr) + sizeof(ep), &bytes_got, sizeof(bytes_got));
                MemoryCopy((u8 *)dst + sizeof(pr) + sizeof(ep) + sizeof(bytes_got), data.str, bytes_got);
            }
        }
        guarded_ring_close(&guard);
    }
    scratch_end(scratch);
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

    // create listener sockets
    w32_net_state->tcp_listen_socket = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
    w32_net_state->udp_listen_socket = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
    {
        DWORD ipv6only = 0;
        setsockopt(w32_net_state->tcp_listen_socket, IPPROTO_IPV6, IPV6_V6ONLY, (char *)&ipv6only, sizeof(ipv6only));
        setsockopt(w32_net_state->udp_listen_socket, IPPROTO_IPV6, IPV6_V6ONLY, (char *)&ipv6only, sizeof(ipv6only));
    }

    // bind listener sockets
    {
        NET_Endpoint server_ep = net_endpoint_from_string_port(s("::ffff:127.0.0.1"), NET_PORT);
        SOCKADDR_STORAGE server_storage = w32_net_sockaddr_storage_from_endpoint(server_ep);
        bind(w32_net_state->tcp_listen_socket, (SOCKADDR *)&server_storage, sizeof(server_storage));
        bind(w32_net_state->udp_listen_socket, (SOCKADDR *)&server_storage, sizeof(server_storage));
    }

    // start listening
    {
        listen(w32_net_state->tcp_listen_socket, SOMAXCONN);
        listen(w32_net_state->udp_listen_socket, SOMAXCONN);
    }

    // launch one-off listeners threads to block & accept connections
    w32_net_state->tcp_listeners_thread = thread_launch(w32_net_tcp_listeners_thread_entry_point, 0);
    w32_net_state->udp_listeners_thread = thread_launch(w32_net_udp_listeners_thread_entry_point, 0);
}

internal void net_async_tick(void)
{
    // pop sends
    if (lane_idx() == 0)
    {
        Temp scratch = scratch_begin(0, 0);
            
        Ring_Guard guard = guarded_ring_open(w32_net_state->u2s_ring);
        for (;guard.r->ring->write_pos != guard.r->ring->read_pos;)
        {
            // there is data we need to enqueue to send to the network
            W32_NET_Connection *connection = push_array(scratch.arena, W32_NET_Connection, 1);
            String8 data = {0};
            guarded_ring_try_read_struct(&guard, &connection->protocol);
            guarded_ring_try_read_struct(&guard, &connection->endpoint);
            guarded_ring_try_read_struct(&guard, &data.size);
            if (data.size != 0)
            {
                data.str = push_array(scratch.arena, u8, data.size);
                guarded_ring_try_read(&guard, data.size, data.str);
            }

            switch(connection->protocol)
            {
                default:{}break;
                case NET_Protocol_TCP: {
                    // find active connection, if it exists
                    for (W32_NET_Connection *conn = w32_net_state->conn_list.first; conn != 0; conn = conn->next)
                    {
                        if (MemoryMatchStruct(&connection->endpoint, &conn->endpoint))
                        {
                            MemoryCopyStruct(connection, conn);
                            break;
                        }
                    }
                    
                    // if no connection was found, construct socket & push new connection to list
                    if (connection->protocol == NET_Protocol_TCP && connection->next == 0)
                    {
                        // create new socket
                        connection->socket = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
                        {
                            DWORD ipv6only = 0;
                            setsockopt(connection->socket, IPPROTO_IPV6, IPV6_V6ONLY, (char *)&ipv6only, sizeof(ipv6only));
                        }
                        // connect to server
                        SOCKADDR_STORAGE storage = w32_net_sockaddr_storage_from_endpoint(connection->endpoint);
                        if (SOCKET_ERROR != connect(connection->socket, (SOCKADDR *)&storage, sizeof(storage)))
                        {
                            SLLQueuePush(w32_net_state->conn_list.first, w32_net_state->conn_list.last, connection);
                        }
                    }

                    // send data
                    send(connection->socket, data.str, data.size, 0);
                } break;
                case NET_Protocol_UDP: {
                    SOCKADDR_STORAGE dest = w32_net_sockaddr_storage_from_endpoint(connection->endpoint);
                    // sendto from our bound listening socket such that replies will arrive there.
                    sendto(w32_net_state->udp_listen_socket, data.str, data.size, 0, (SOCKADDR *)&dest, sizeof(dest));
                } break;
            }
        }
        guarded_ring_close(&guard);
        
        scratch_end(scratch);
    }
    lane_sync();

    // pop receives
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

/////////////////////////
// @per_os_impl Receives

internal u64 net_recv(u8 *ptr, u64 size, NET_Protocol *protocol_out, NET_Endpoint *endpoint_out, u64 endt_us)
{
    u64 received_size = 0;
    u64 header_size = sizeof(*protocol_out) + sizeof(*endpoint_out) + sizeof(u64);
    {
        Ring_Guard guard = guarded_ring_open(w32_net_state->s2u_ring);
        {
            void *src = guarded_ring_pop_or_wait(&guard, header_size, endt_us);
            if(src != 0)
            {
                u64 payload_size = 0;
                MemoryCopy(protocol_out,  (u8 *)src + 0, sizeof(*protocol_out));
                MemoryCopy(endpoint_out,  (u8 *)src + sizeof(*protocol_out), sizeof(*endpoint_out));
                MemoryCopy(&payload_size, (u8 *)src + sizeof(*protocol_out) + sizeof(*endpoint_out), sizeof(u64));
                void *payload = guarded_ring_pop_or_wait(&guard, payload_size, max_u64);
                MemoryCopy(ptr, payload, payload_size);
                received_size = payload_size;
            }
        }
        guarded_ring_close(&guard);
    }
    return received_size;
}

