// Copyright (c) Frances Telfar
// Licensed under the PolyForm Noncommercial License (https://polyformproject.org/licenses/noncommercial/1.0.0)

///////////////////////////////////
// Conversion Helpers

internal bool32
lnx_net_endpoint_is_mapped_v4(NET_Endpoint ep)
{
    return ep.address.u64[0] == 0 && ep.address.u16[4] == 0 && ep.address.u16[5] == 0xffff;
}

internal struct sockaddr_storage
lnx_net_sockaddr_storage_from_endpoint(NET_Endpoint ep)
{
    struct sockaddr_storage result = {0};
    if(lnx_net_endpoint_is_mapped_v4(ep))
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
            v6->sin6_addr.s6_addr[i] = ep.address.u8[15-i];
        }
    }
    return result;
}

internal NET_Endpoint
lnx_net_endpoint_from_sockaddr_storage(struct sockaddr_storage addr)
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
            ep.address.u8[15-i] = v6->sin6_addr.s6_addr[i];
        }
    }
    ep.kind = lnx_net_endpoint_is_mapped_v4(ep) ? NET_EndpointKind_IPv4 : NET_EndpointKind_IPv6;
    return ep;
}

/////////////////////////
// Listener Thread

internal void
lnx_net_listener_thread_entry_point(void *p)
{
    ThreadNameF("lnx_net_listener_thread_%I64x", p);
    LNX_NET_Session *session = (LNX_NET_Session *)p;
    for(;;)
    {
        // wait for next event
        struct epoll_event evts[1] = {0};
        int wait_result = LNX_RETRY_ON_EINTR(epoll_wait(session->epoll_fd, evts, ArrayCount(evts), -1));

        // code is 0 -> listener is ready for accept
        if(evts[0].data.u64 == 0)
        {
            // accept new connection
            struct sockaddr_storage addr = {0};
            socklen_t addr_size = sizeof(addr);
            int new_socket = LNX_RETRY_ON_EINTR(accept(session->tcp_listen_socket, (struct sockaddr *)&addr, &addr_size));

            // unpack socket's endpoint info
            NET_Endpoint endpoint = lnx_net_endpoint_from_sockaddr_storage(addr);

            // unpack endpoint
            u64 hash = u64_hash_from_str8(str8_struct(&endpoint));
            u64 slot_idx = hash%session->connection_slots_count;
            LNX_NET_Connection_Slot *slot = &session->connection_slots[slot_idx];
            Stripe *stripe = stripe_from_slot_idx(&session->connection_stripes, slot_idx);

            // store new connection
            u64 con_ptr_u64 = 0;
            RWMutexScope(stripe->rw_mutex, 1)
            {
                LNX_NET_Connection *con = (LNX_NET_Connection *)stripe->free;
                if(con != 0)
                {
                    stripe->free = con->next;
                }
                else
                {
                    con = push_array(stripe->arena, LNX_NET_Connection, 1);
                }
                con->endpoint = endpoint;
                con->protocol = NET_Protocol_TCP;
                con->socket = new_socket;
                DLLPushBack(slot->first, slot->last, con);
                con_ptr_u64 = (u64)con;
            }

            // add socket to epoll
            struct epoll_event evt = {0};
            evt.events = EPOLLIN;
            evt.data.u64 = con_ptr_u64;
            LNX_RETRY_ON_EINTR(epoll_ctl(session->epoll_fd, EPOLL_CTL_ADD, new_socket, &evt));
        }

        // code is nonzero -> ready to read on a connection
        else
        {
            // unpack associated connection
            LNX_NET_Connection *con = (LNX_NET_Connection *)evts[0].data.u64;
            NET_Endpoint endpoint = con->endpoint;
            u64 hash = u64_hash_from_str8(str8_struct(&endpoint));
            u64 slot_idx = hash%session->connection_slots_count;
            LNX_NET_Connection_Slot *slot = &session->connection_slots[slot_idx];
            Stripe *stripe = stripe_from_slot_idx(&session->connection_stripes, slot_idx);

            // do read
            u8 buffer[4096] = {0};
            ssize_t recv_result = LNX_RETRY_ON_EINTR(recv(con->socket, buffer, sizeof(buffer), MSG_DONTWAIT));

            // bytes received -> push result to user.
            if(recv_result >= 0)
            {
                Ring_Guard g = guarded_ring_open(session->s2u_ring);
                u64 header[5] =
                {
                    (u64)NET_Protocol_TCP,
                    (u64)endpoint.port,
                    endpoint.address.u64[0],
                    endpoint.address.u64[1],
                    (u64)recv_result,
                };
                guarded_ring_write_or_wait(&g, sizeof(header), header, max_u64);
                guarded_ring_write_or_wait(&g, recv_result, buffer, max_u64);
                guarded_ring_close(&g);
            }

            // error -> socket was disconnected
            else RWMutexScope(stripe->rw_mutex, 1)
            {
                close(con->socket);
                LNX_RETRY_ON_EINTR(epoll_ctl(session->epoll_fd, EPOLL_CTL_DEL, con->socket, 0));
                DLLRemove(slot->first, slot->last, con);
                con->next = stripe->free;
                stripe->free = con;
            }
        }
    }
}

//////////////////////
// @per_os_impl Top-Level Layer Calls

internal void
net_init(void)
{
    Arena *arena = arena_alloc();
    lnx_net_state = push_array(arena, LNX_NET_State, 1);
    lnx_net_state->arena = arena;
    lnx_net_state->session_rw_mutex = rw_mutex_alloc();
}

internal void
net_async_tick(void)
{
    Temp scratch = scratch_begin(0, 0);

    /////////////////////////
    // gather send tasks
    typedef struct Send_Task Send_Task;
    struct Send_Task
    {
        Send_Task *next;
        LNX_NET_Session *session;
        NET_Endpoint endpoint;
        String8 data;
    };
    Send_Task *first_tcp_send = 0;
    Send_Task *last_tcp_send = 0;
    if(lane_idx() == 0)
    {
        for(;;)
        {
            bool32 got_more = 0;
            RWMutexScope(lnx_net_state->session_rw_mutex, 0)
            {
                for EachNode(s, LNX_NET_Session, lnx_net_state->first_session)
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
    //
    for(Send_Task *t = first_tcp_send; t != 0; t = t->next)
    {
        LNX_NET_Session *session = t->session;

        // unpack endpoint
        u64 hash = u64_hash_from_str8(str8_struct(&t->endpoint));
        u64 slot_idx = hash%session->connection_slots_count;
        LNX_NET_Connection_Slot *slot = &session->connection_slots[slot_idx];
        Stripe *stripe = stripe_from_slot_idx(&session->connection_stripes, slot_idx);


        // get existing socket for this endpoint
        int ep_socket = -1;
        RWMutexScope(stripe->rw_mutex, 0)
        {
            for(LNX_NET_Connection *c = slot->first; c != 0; c = c->next)
            {
                if(MemoryMatchStruct(&c->endpoint, &t->endpoint))
                {
                    ep_socket = c->socket;
                    break;
                }
            }
        }

        // didn't get a socket? open socket
        if(ep_socket == -1) RWMutexScope(stripe->rw_mutex, 1)
        {
            // try to get socket again, now that we have the write lock
            LNX_NET_Connection *con = 0;
            for(LNX_NET_Connection *c = slot->first; c != 0; c = c->next)
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
                struct sockaddr_storage endpoint_sockaddr = lnx_net_sockaddr_storage_from_endpoint(t->endpoint);
                int endpoint_sockaddr_size = endpoint_sockaddr.ss_family == AF_INET ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6);

                // create
                int new_socket = socket(AF_INET, SOCK_STREAM, 0);
                connect(new_socket, (struct sockaddr *)&endpoint_sockaddr, endpoint_sockaddr_size);

                // store in cache
                con = (LNX_NET_Connection *)stripe->free;
                if(con != 0)
                {
                    stripe->free = con->next;
                }
                else
                {
                    con = push_array(stripe->arena, LNX_NET_Connection, 1);
                }
                con->endpoint = t->endpoint;
                con->protocol = NET_Protocol_TCP;
                con->socket = new_socket;
                DLLPushBack(slot->first, slot->last, con);

                // hook up to session epoll
                struct epoll_event evt = {0};
                evt.events = EPOLLIN;
                evt.data.u64 = (u64)con;
                LNX_RETRY_ON_EINTR(epoll_ctl(session->epoll_fd, EPOLL_CTL_ADD, new_socket, &evt));
            }

            // get socket from cache
            ep_socket = con->socket;
        }

        // got socket? -> send
        bool32 send_failed = 0;
        if(ep_socket != -1 && send(ep_socket, t->data.str, t->data.size, 0) == -1)
        {
            send_failed = 1;
        }

        // got a socket, but send failed? -> connection closed
        if(ep_socket != -1 && send_failed)
        {
            RWMutexScope(stripe->rw_mutex, 1)
            {
                for(LNX_NET_Connection *c = slot->first; c != 0; c = c->next)
                {
                    if(MemoryMatchStruct(&c->endpoint, &t->endpoint))
                    {
                        close(c->socket);
                        LNX_RETRY_ON_EINTR(epoll_ctl(session->epoll_fd, EPOLL_CTL_DEL, c->socket, 0));
                        DLLRemove(slot->first, slot->last, c);
                        c->next = stripe->free;
                        stripe->free = c;
                    }
                }
            }
        }
    }

    scratch_end(scratch);
}

///////////////////////////
// @per_os_impl Session Creating/Closing

internal NET_Session
net_session_open(u16 listener_port, NET_Wakeup_Function_Type *wakeup_hook)
{
    // set up state
    Arena *arena = arena_alloc();
    LNX_NET_Session *session = push_array(arena, LNX_NET_Session, 1);
    session->arena = arena;
    session->u2s_ring = guarded_ring_alloc(arena, Kilobytes(256));
    session->s2u_ring = guarded_ring_alloc(arena, Kilobytes(256));
    session->epoll_fd = epoll_create(1);

    // set up listener
    {
        session->tcp_listen_socket = socket(AF_INET, SOCK_STREAM, 0);
        u32 ipv6only = 0;
        setsockopt(session->tcp_listen_socket, IPPROTO_IPV6, IPV6_V6ONLY, (char *)&ipv6only, sizeof(ipv6only));
        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(listener_port);
        bind(session->tcp_listen_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
        listen(session->tcp_listen_socket, SOMAXCONN);
        struct epoll_event evt = {0};
        evt.events = EPOLLIN;
        epoll_ctl(session->epoll_fd, EPOLL_CTL_ADD, session->tcp_listen_socket, &evt);
    }

    // set up connection cache
    session->connection_slots_count = 8;
    session->connection_stripes = stripe_array_alloc(arena);
    session->connection_slots = push_array(arena, LNX_NET_Connection_Slot, session->connection_slots_count);

    // launch listener thread
    session->listener_thread = thread_launch(lnx_net_listener_thread_entry_point, session);

    // link into top-level storage
    RWMutexScope(lnx_net_state->session_rw_mutex, 1)
    {
        DLLPushBack(lnx_net_state->first_session, lnx_net_state->last_session, session);
    }

    // bundle as handle
    NET_Session s = {(u64)session};
    return s;
}

internal void
net_session_close(NET_Session session)
{
    // TODO
}

//////////////////////
// @per_os_impl Sends

internal bool32
net_send(NET_Session session, NET_Protocol protocol, NET_Endpoint endpoint, String8 data, u64 endt_us)
{
    bool32 result = 0;
    LNX_NET_Session *s = (LNX_NET_Session *)session.u64[0];
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

internal bool32
net_recv(Arena *arena, NET_Session session, NET_Protocol *protocol_out, NET_Endpoint *endpoint_out, String8 *data_out, u64 endt_us)
{
    bool32 result = 0;
    u64 header_size = sizeof(*protocol_out) + sizeof(*endpoint_out) + sizeof(u64);
    {
        LNX_NET_Session *s = (LNX_NET_Session *)session.u64[0];
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
