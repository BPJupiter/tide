// Copyright (c) Frances Telfar
// Licensed under the PolyForm Noncommercial License (https://polyformproject.org/licenses/noncommercial/1.0.0)


//////////////////////
// Conversion Helpers

internal bool32
w32_net_endpoint_is_mapped_v4(NET_Endpoint ep)
{
    return ep.address.u64[0] == 0 && ep.address.u16[4] == 0 && ep.address.u16[5] == 0xffff;
}

internal struct sockaddr_storage
w32_net_sockaddr_storage_from_endpoint(NET_Endpoint ep)
{
    struct sockaddr_storage result = {0};
    if(w32_net_endpoint_is_mapped_v4(ep))
    {
        struct sockaddr_in *v4 = (struct sockaddr_in *)&result;
        v4->sin_family = AF_INET;
        v4->sin_port = host_to_net_u16(ep.port);
        v4->sin_addr.s_addr = host_to_net_u32(ep.address.u32[3]);
    }
    else
    {
        struct sockaddr_in6 *v6 = (struct sockaddr_in6 *)&result;
        v6->sin6_family = AF_INET6;
        v6->sin6_port = host_to_net_u16(ep.port);
        for(u64 i = 0; i < 16; i++)
        {
            v6->sin6_addr.u.Byte[i] = ep.address.u8[15-i];
        }
    }
    return result;
}

internal NET_Endpoint
w32_net_endpoint_from_sockaddr_storage(struct sockaddr_storage addr)
{
    NET_Endpoint ep = {0};
    if(addr.ss_family == AF_INET)
    {
        struct sockaddr_in *v4 = (struct sockaddr_in *)&addr;
        ep.port = net_to_host_u16(v4->sin_port);
        ep.address.u16[5] = 0xffff;
        ep.address.u32[3] = net_to_host_u32(v4->sin_addr.s_addr);
    }
    else if(addr.ss_family == AF_INET6)
    {
        struct sockaddr_in6 *v6 = (struct sockaddr_in6 *)&addr;
        ep.port = net_to_host_u16(v6->sin6_port);
        for(u64 i = 0; i < 16; i++)
        {
            ep.address.u8[15-i] = v6->sin6_addr.u.Byte[i];
        }
    }
    ep.kind = w32_net_endpoint_is_mapped_v4(ep) ? NET_EndpointKind_IPv4 : NET_EndpointKind_IPv6;
    return ep;
}


//////////////////////////
// Listener Threads

internal void w32_net_listener_thread_entry_point(void *p)
{
    ThreadNameF("w32_net_listener_thread_%I64x", (u64)p);
    W32_NET_Session *session = (W32_NET_Session *)p;
    for(;;)
    {
        // get next completion from IOCP
        DWORD byte_count = 0;
        u64 completion_key = 0;
        OVERLAPPED *overlapped_ptr = 0;
        bool32 success = GetQueuedCompletionStatus(session->iocp, &byte_count, &completion_key, &overlapped_ptr, INFINITE);

        // call wakeup hook
        if(session->wakeup_hook)
        {
            session->wakeup_hook();
        }

        // overlapped_ptr == accept overlapped? -> new connection
        if(overlapped_ptr == &session->tcp_accept_overlapped)
        {
            // unpack new socket
            SOCKET new_socket = session->tcp_accept_socket;
            struct sockaddr_storage addr = {0};
            MemoryCopy(&addr, session->tcp_accept_buffer + sizeof(struct sockaddr_storage) + 16, sizeof(addr));

            // unpack socket's endpoint info
            NET_Endpoint endpoint = w32_net_endpoint_from_sockaddr_storage(addr);

            // unpack endpoint
            u64 hash = u64_hash_from_str8(str8_struct(&endpoint));
            u64 slot_idx = hash%session->connection_slots_count;
            W32_NET_Connection_Slot *slot = &session->connection_slots[slot_idx];
            Stripe *stripe = stripe_from_slot_idx(&session->connection_stripes, slot_idx);

            // store new connection
            WSABUF buf = {0};
            DWORD *recv_size = 0;
            OVERLAPPED *recv_overlapped = 0;
            RWMutexScope(stripe->rw_mutex, 1)
            {
                W32_NET_Connection *con = (W32_NET_Connection *)stripe->free;
                if(con != 0)
                {
                    stripe->free = con->next;
                }
                else
                {
                    con = push_array(stripe->arena, W32_NET_Connection, 1);
                }
                con->endpoint = endpoint;
                con->protocol = NET_Protocol_TCP;
                con->socket = new_socket;
                MemoryZeroStruct(&con->recv_overlapped);
                DLLPushBack(slot->first, slot->last, con);
                buf.len = sizeof(con->recv_buffer);
                buf.buf = (char *)con->recv_buffer;
                recv_size = &con->recv_size;
                recv_overlapped = &con->recv_overlapped;
            }

            // kick off receive
            DWORD flags = MSG_PUSH_IMMEDIATE;
            WSARecv(new_socket, &buf, 1, recv_size, &flags, recv_overlapped, 0);

            // create new accept socket, associate with iocp, zero overlapped, kick off next accept
            session->tcp_accept_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            CreateIoCompletionPort((HANDLE)session->tcp_accept_socket, session->iocp, 0, 0);
            MemoryZeroStruct(&session->tcp_accept_overlapped);
            session->lpfnAcceptEx(session->tcp_listen_socket, session->tcp_accept_socket, session->tcp_accept_buffer, 0, sizeof(struct sockaddr_storage) + 16, sizeof(struct sockaddr_storage) + 16, &session->tcp_accept_size_out, &session->tcp_accept_overlapped);
        }

        // overlapped ptr anywhere else -> completion of async recv
        else
        {
            // unpack associated connection
            W32_NET_Connection *con = CastFromMember(W32_NET_Connection, recv_overlapped, overlapped_ptr);
            NET_Endpoint endpoint = con->endpoint;
            u64 hash = u64_hash_from_str8(str8_struct(&endpoint));
            u64 slot_idx = hash%session->connection_slots_count;
            W32_NET_Connection_Slot *slot = &session->connection_slots[slot_idx];
            Stripe *stripe = stripe_from_slot_idx(&session->connection_stripes, slot_idx);

            // success? -> push result to user. kick off next recv
            if(success)
            {
                Ring_Guard g = guarded_ring_open(session->s2u_ring);
                u64 header[5] =
                {
                    (u64)NET_Protocol_TCP,
                    (u64)endpoint.port,
                    endpoint.address.u64[0],
                    endpoint.address.u64[1],
                    (u64)byte_count,
                };
                guarded_ring_write_or_wait(&g, sizeof(header), header, max_u64);
                guarded_ring_write_or_wait(&g, byte_count, con->recv_buffer, max_u64);
                MemoryZeroStruct(&con->recv_overlapped);
                WSABUF buf = {sizeof(con->recv_buffer), (char *)con->recv_buffer};
                DWORD flags = MSG_PUSH_IMMEDIATE;
                WSARecv(con->socket, &buf, 1, &con->recv_size, &flags, &con->recv_overlapped, 0);
                guarded_ring_close(&g);
            }

            // no seccess? -> connection closed.
            else RWMutexScope(stripe->rw_mutex, 1)
            {
                closesocket(con->socket);
                DLLRemove(slot->first, slot->last, con);
                con->next = stripe->free;
                stripe->free = con;
            }
        }
    }
}

//////////////////////////
// @per_os_impl Top-Level Layer Calls
//

internal void net_init(void)
{
    // NOTE: winsock2 is already initialized by the base layer for RIO function grabbing.

    // set up top-level state
    Arena *arena = arena_alloc();
    w32_net_state = push_array(arena, W32_NET_State, 1);
    w32_net_state->arena = arena;
    w32_net_state->session_rw_mutex = rw_mutex_alloc();
}

internal void net_async_tick(void)
{
    Temp scratch = scratch_begin(0, 0);

    ///////////////////////
    // gather send tasks
    typedef struct Send_Task Send_Task;
    struct Send_Task
    {
        Send_Task *next;
        W32_NET_Session *session;
        NET_Endpoint endpoint;
        String8 data;
    };
    Send_Task *first_tcp_send = 0;
    Send_Task *last_tcp_send = 0;
    Send_Task *first_udp_send = 0;
    Send_Task *last_udp_send = 0;
    if(lane_idx() == 0)
    {
        for(;;)
        {
            bool32 got_more = 0;
            RWMutexScope(w32_net_state->session_rw_mutex, 0)
            {
                for EachNode(s, W32_NET_Session, w32_net_state->first_session)
                {
                    Ring_Guard g = guarded_ring_open(s->u2s_ring);
                    {
                        u64 header[5] = {0};
                        if(guarded_ring_try_read(&g, sizeof(header), header))
                        {
                            got_more = 1;
                            NET_Protocol protocol = (NET_Protocol)header[0];
                            u16 port = (u16)header[1];
                            NET_Endpoint endpoint = {0};
                            endpoint.address.u64[0] = header[2];
                            endpoint.address.u64[1] = header[3];
                            endpoint.port = port;
                            u64 data_size = header[4];
                            u8 *data = push_array(scratch.arena, u8, data_size);
                            guarded_ring_read_or_wait(&g, data_size, data, max_u64);
                            Send_Task *t = push_array(scratch.arena, Send_Task, 1);
                            t->session = s;
                            t->endpoint = endpoint;
                            t->data = str8(data, data_size);
                            SLLQueuePush(first_tcp_send, last_tcp_send, t);
                        }
                    }
                    guarded_ring_close(&g);
                }
            }
            if(!got_more)
            {
                break;
            }
        }
    }
    lane_sync();

    ////////////////////
    // do TCP sends
    for(Send_Task *t = first_tcp_send; t != 0; t = t->next)
    {
        W32_NET_Session *session = t->session;

        // unpack endpoint
        u64 hash = u64_hash_from_str8(str8_struct(&t->endpoint));
        u64 slot_idx = hash%session->connection_slots_count;
        W32_NET_Connection_Slot *slot = &session->connection_slots[slot_idx];
        Stripe *stripe = stripe_from_slot_idx(&session->connection_stripes, slot_idx);

        // get existing socket for this endpoint
        SOCKET ep_socket = -1;
        RWMutexScope(stripe->rw_mutex, 0)
        {
            for(W32_NET_Connection *c = slot->first; c != 0; c = c->next)
            {
                if(MemoryMatchStruct(&c->endpoint, &t->endpoint))
                {
                    ep_socket = c->socket;
                    break;
                }
            }
        }

        // didn't get a socket? -> open socket
        if(ep_socket == -1) RWMutexScope(stripe->rw_mutex, 1)
        {
            // try to get socket again, now that we have the write lock
            W32_NET_Connection *con = 0;
            for(W32_NET_Connection *c = slot->first; c != 0; c = c->next)
            {
                if(MemoryMatchStruct(&c->endpoint, &t->endpoint))
                {
                    con = c;
                    break;
                }
            }

            // no socket still? -> create
            if(con == 0)
            {
                // convert endpoint -> sockaddr
                struct sockaddr_storage endpoint_sockaddr = w32_net_sockaddr_storage_from_endpoint(t->endpoint);
                int endpoint_sockaddr_size = endpoint_sockaddr.ss_family == AF_INET ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6);
                
                // create
                SOCKET new_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
                connect(new_socket, (struct sockaddr *)&endpoint_sockaddr, endpoint_sockaddr_size);

                // store in cache
                con = (W32_NET_Connection *)stripe->free;
                if(con != 0)
                {
                    stripe->free = con->next;
                }
                else
                {
                    con = push_array(stripe->arena, W32_NET_Connection, 1);
                }

                con->endpoint = t->endpoint;
                con->protocol = NET_Protocol_TCP;
                con->socket = new_socket;
                DLLPushBack(slot->first, slot->last, con);

                // associate this socket with iocp
                CreateIoCompletionPort((HANDLE)new_socket, session->iocp, 0, 0);
                MemoryZeroStruct(&con->recv_overlapped);

                // kick off receive on this socket
                WSABUF buf = {sizeof(con->recv_buffer), (char *)con->recv_buffer};
                DWORD flags = MSG_PUSH_IMMEDIATE;
                WSARecv(con->socket, &buf, 1, &con->recv_size, &flags, &con->recv_overlapped, 0);
            }

            // get socket from cache
            ep_socket = con->socket;
        }

        // got socket? -> send
        bool32 send_failed = 0;
        if(ep_socket != -1 && send(ep_socket, (char *)t->data.str, t->data.size, 0) == SOCKET_ERROR)
        {
            int error = WSAGetLastError();
            if(error == WSAECONNRESET)
            {
                send_failed = 1;
            }
        }

        // got a socket, but send failed? -> connection closed
        if(ep_socket != -1 && send_failed)
        {
            RWMutexScope(stripe->rw_mutex, 1)
            {
                for(W32_NET_Connection *c = slot->first; c != 0; c = c->next)
                {
                    if(MemoryMatchStruct(&c->endpoint, &t->endpoint))
                    {
                        DLLRemove(slot->first, slot->last, c);
                        c->next = stripe->free;
                        stripe->free =c;
                    }
                }
            }
        }
    }

    scratch_end(scratch);
}

////////////////////////////
// Session Creation/Closing
//

internal NET_Session net_session_open(u16 listener_port, NET_Wakeup_Function_Type *wakeup_hook)
{
    // set up state
    Arena *arena = arena_alloc();
    W32_NET_Session *session = push_array(arena, W32_NET_Session, 1);
    session->arena = arena;
    session->u2s_ring = guarded_ring_alloc(arena, Kilobytes(256));
    session->s2u_ring = guarded_ring_alloc(arena, Kilobytes(256));

    // set up IOCP
    session->iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);

    // set up listener(s)
    {
        session->tcp_listen_socket = WSASocketA(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
        DWORD ipv6only = 0;
        setsockopt(session->tcp_listen_socket, IPPROTO_IPV6, IPV6_V6ONLY, (char *)&ipv6only, sizeof(ipv6only));
        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(listener_port);
        bind(session->tcp_listen_socket, (SOCKADDR *)&server_addr, sizeof(server_addr));
        listen(session->tcp_listen_socket, SOMAXCONN);
    }

    // assocate listener sockets with IOCP
    {
        CreateIoCompletionPort((HANDLE)session->tcp_listen_socket, session->iocp, 0, 0);
    }

    // load AcceptEx function
    DWORD dwBytes = 0;
    GUID AcceptEx_guid = WSAID_ACCEPTEX;
    WSAIoctl(session->tcp_listen_socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &AcceptEx_guid, sizeof(AcceptEx_guid), &session->lpfnAcceptEx, sizeof(session->lpfnAcceptEx), &dwBytes, 0, 0);

    // create accepting socket
    {
        session->tcp_accept_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    }

    // associate accepting socket with IOCP
    {
        CreateIoCompletionPort((HANDLE)session->tcp_accept_socket, session->iocp, 0, 0);
    }

    // kick off accept
    {
        session->lpfnAcceptEx(session->tcp_listen_socket, session->tcp_accept_socket, session->tcp_accept_buffer, 0, sizeof(struct sockaddr_storage) + 16, sizeof(struct sockaddr_storage) + 16, &session->tcp_accept_size_out, &session->tcp_accept_overlapped);
    }

    // set up connection cache
    session->connection_slots_count = 8;
    session->connection_slots = push_array(arena, W32_NET_Connection_Slot, session->connection_slots_count);
    session->connection_stripes = stripe_array_alloc(arena);

    // set wakeup hook
    session->wakeup_hook = wakeup_hook;

    // launch one-off listener thread to block & accept connections
    session->tcp_listener_thread = thread_launch(w32_net_listener_thread_entry_point, session);

    // connection to top-level state
    RWMutexScope(w32_net_state->session_rw_mutex, 1)
    {
        DLLPushBack(w32_net_state->first_session, w32_net_state->last_session, session);
    }

    // bundle as handle
    NET_Session result = {(u64)session};
    return result;
}

internal void net_session_close(NET_Session session)
{
    // TODO
}

////////////////////////
// @per_os_impl Sends

internal bool32 net_send(NET_Session session, NET_Protocol protocol, NET_Endpoint endpoint, String8 data, u64 endt_us)
{
    bool32 result = 0;
    W32_NET_Session *s = (W32_NET_Session *)session.u64[0];
    Ring_Guard guard = guarded_ring_open(s->u2s_ring);
    {
        u64 header[5] = {0};
        u64 size_cap = s->u2s_ring->ring->size - sizeof(header);
        u64 size = Min(data.size, size_cap);
        {
            header[0] = (u64)protocol;
            header[1] = (u64)endpoint.port;
            header[2] = endpoint.address.u64[0];
            header[3] = endpoint.address.u64[1];
            header[4] = size;
        }
        if(guarded_ring_write_or_wait(&guard, sizeof(header), header, endt_us))
        {
            guarded_ring_write_or_wait(&guard, size, data.str, max_u64);
            result = 1;
        }
    }
    guarded_ring_close(&guard);
    if(result)
    {
        ins_atomic_u32_eval_assign(&async_loop_again, 1);
        cond_var_broadcast(async_tick_start_cond_var);
    }
    return result;
}

////////////////////////////
// @per_os_impl Receives
//

internal bool32 net_recv(Arena *arena, NET_Session session, NET_Protocol *protocol_out, NET_Endpoint *endpoint_out, String8 *data_out, u64 endt_us)
{
    bool32 result = 0;
    u64 header_size = sizeof(*protocol_out) + sizeof(*endpoint_out) + sizeof(u64);
    {
        W32_NET_Session *s = (W32_NET_Session *)session.u64[0];
        Ring_Guard guard = guarded_ring_open(s->s2u_ring);
        {
            u64 header[5] = {0};
            if(guarded_ring_read_or_wait(&guard, sizeof(header), header, endt_us))
            {
                u64 size_cap = s->s2u_ring->ring->size - sizeof(header);
                if(protocol_out)
                {
                    protocol_out[0] = (NET_Protocol)header[0];
                }
                if(endpoint_out)
                {
                    endpoint_out->port = (u16)header[1];
                    endpoint_out->address.u64[0] = header[2];
                    endpoint_out->address.u64[1] = header[3];
                }
                if(data_out)
                {
                    data_out->size = Min(size_cap, header[4]);
                    data_out->str = push_array(arena, u8, data_out->size);
                }
                guarded_ring_read_or_wait(&guard, data_out->size, data_out->str, max_u64);
                result = 1;
            }
        }
        guarded_ring_close(&guard);
    }
    return result;
}
