
///////////////////////////////////
// NETworking Conversion Helpers

internal void lnx_sockaddr_storage_to_net_address(NET_Address *out, struct sockaddr_storage *in)
{
    switch (in->ss_family) {
        case AF_INET: {
            struct sockaddr_in *addr = (struct sockaddr_in *)in;

            out->family = NET_AddressFamily_IPv4;
            out->port = net_to_host_u16(addr->sin_port);
            out->ip.v4 = net_to_host_u32(addr->sin_addr.s_addr);
        } break;
        case AF_INET6: {
            struct sockaddr_in6 *addr = (struct sockaddr_in6 *)in;

            out->family = NET_AddressFamily_IPv6;
            out->port = net_to_host_u16(addr->sin6_port);
            MemoryCopyArray(out->ip.v6.u8, addr->sin6_addr.s6_addr);
        } break;
        default: {
            MemoryZeroStruct(out);
        } break;
    }
}

internal void lnx_net_address_to_sockaddr_storage(struct sockaddr_storage *out, NET_Address *in)
{
    switch (in->family)
    {
        case NET_AddressFamily_IPv4: {
            struct sockaddr_in *addr = (struct sockaddr_in *)out;

            addr->sin_family = AF_INET;
            addr->sin_port = host_to_net_u16(in->port);
            addr->sin_addr.s_addr = host_to_net_u32(in->ip.v4);
        } break;
        case NET_AddressFamily_IPv6: {
            struct sockaddr_in6 *addr = (struct sockaddr_in6 *)out;

            addr->sin6_family = AF_INET6;
            addr->sin6_port = host_to_net_u16(in->port);
            addr->sin6_flowinfo = 0;
            MemoryCopyArray(addr->sin6_addr.s6_addr, in->ip.v6.u8);
            addr->sin6_scope_id = 0;
        } break;
        default: {
            MemoryZeroStruct(out);
        } break;
    }
}

////////////////////////////////////////
// @per_os_impl NETworking Primitives

internal NET_Socket net_socket_alloc(NET_AddressFamily family, NET_TransportProtocol protocol)
{
    LNX_Entity *entity = lnx_entity_alloc(LNX_EntityKind_Socket);

    u16 af = 0;
    switch (family) {
        default:
        case NET_AddressFamily_Any:
        case NET_AddressFamily_IPv4: {
            af = AF_INET;
        } break;
        case NET_AddressFamily_IPv6: {
            af = AF_INET6;
        } break;
    }

    switch (protocol) {
        default:
        case NET_TransportProtocol_RAW: {
            entity->socket = socket(af, SOCK_RAW, 0);
        } break;
        case NET_TransportProtocol_TCP: {
            entity->socket = socket(af, SOCK_STREAM, 0);
        } break;
        case NET_TransportProtocol_UDP: {
            entity->socket = socket(af, SOCK_DGRAM, 0);
        } break;
    }

    NET_Socket socket = {IntFromPtr(entity)};
    return socket;
}

internal void net_socket_release(NET_Socket socket)
{
    LNX_Entity *entity = (LNX_Entity *)PtrFromInt(socket.u64[0]);
    close(entity->socket);
    lnx_entity_release(entity);
}

/////////////////////////////////////////////
// @per_os_impl NETwork Listener Functions

internal NET_Listener net_listener_alloc(NET_AddressFamily family, NET_TransportProtocol protocol, u16 port)
{
    struct sockaddr_storage storage = {0};
    NET_Address address = {0};
    address.family = family;
    address.port = port;
    lnx_net_address_to_sockaddr_storage(&storage, &address);
    NET_Listener listener = {0};
    listener.socket = net_socket_alloc(family, protocol);
    
    LNX_Entity *entity = (LNX_Entity *)PtrFromInt(listener.socket.u64[0]);
    if (-1 == entity->socket) {
        // @TODO: Error handling
        perror("socket");
        fprintf(stderr, "LNX NET ERROR AT %s %d\n", __FILE__, __LINE__);
    }
    // Do some linux-specific options to make life easier
    {
        int opt = 1;
        if (0 > setsockopt(entity->socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
            // @TODO: Error handling
            perror("setsockopt");
            fprintf(stderr, "LNX NET ERROR AT %s %d\n", __FILE__, __LINE__);        
        }
        if (0 > setsockopt(entity->socket, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt))) {
            // @TODO: Error handling
            perror("setsockopt");
            fprintf(stderr, "LNX NET ERROR AT %s %d\n", __FILE__, __LINE__);
        }
    }
    if (0 > bind(entity->socket, (struct sockaddr *)&storage, sizeof(storage))) {
        // @TODO: Error handling
        perror("bind");
        fprintf(stderr, "LNX NET ERROR AT %s %d\n", __FILE__, __LINE__);
    }
    if (protocol == NET_TransportProtocol_TCP)
    {
        if (0 > listen(entity->socket, SOMAXCONN))
        {
            // TODO
            perror("listen");
            fprintf(stderr, "LNX NET ERROR AT %s %d\n", __FILE__, __LINE__);
        }
    }
    {
        socklen_t storagelen = sizeof(storage);
        if (0 > getsockname(entity->socket, (struct sockaddr *)&storage, &storagelen))
        {
            perror("getsockname");
            fprintf(stderr, "LNX NET ERROR AT %s %d\n", __FILE__, __LINE__);
        }
        switch (storage.ss_family)
        {
            case AF_INET: {
                struct sockaddr_in *in = (struct sockaddr_in *)&storage;
                listener.port = ntohs(in->sin_port);
            } break;
            case AF_INET6: {
                struct sockaddr_in6 *in = (struct sockaddr_in6 *)&storage;
                listener.port = ntohs(in->sin6_port);
            } break;
        }
        listener.family = family;
        listener.protocol = protocol;
    }
    return listener;
}

internal NET_Client net_listener_accept(Arena *arena, NET_Listener listener)
{
    struct sockaddr_storage storage = {0};
    socklen_t storagelen = sizeof(storage);
    NET_Client client = net_client_alloc(arena, listener.family, listener.protocol);

    switch (listener.protocol)
    {
        default:{}break;
        case NET_TransportProtocol_TCP: {
            {
                LNX_Entity *listen_entity = (LNX_Entity *)PtrFromInt(listener.socket.u64[0]);
                int socket = accept(listen_entity->socket, (struct sockaddr *)&storage, &storagelen);
                if (-1 == socket) {
                    // @TODO: Error handling
                    perror("accept");
                    fprintf(stderr, "LNX NET ERROR AT %s %d\n", __FILE__, __LINE__);
                }
                LNX_Entity *accept_entity = lnx_entity_alloc(LNX_EntityKind_Socket);
                accept_entity->socket = socket;
                NET_Socket accept_socket = {IntFromPtr(accept_entity)};
                // This is yuck and currently creates a dummy socket that we have to release.
                // Might be worth duplicating the logic of net_client_alloc
                // if this ends up being a lot of overhead.
                net_socket_release(client.socket);
                client.socket = accept_socket;
                lnx_sockaddr_storage_to_net_address(&client.address, &storage);
                client.connected = true;
            }
        } break;
        case NET_TransportProtocol_UDP: {
            {
                client.socket = listener.socket;
                net_client_recv_to_ring(&client);
            }
        } break;
    }

    return client;
}

internal void net_listener_close(NET_Listener listener)
{
    net_socket_release(listener.socket);
}

///////////////////////////////////////////
// @per_os_impl NETwork Client Functions

internal NET_Client net_client_alloc(Arena *arena, NET_AddressFamily family, NET_TransportProtocol protocol)
{
    NET_Socket client_socket = net_socket_alloc(family, protocol);
    LNX_Entity *entity = (LNX_Entity *)PtrFromInt(client_socket.u64[0]);
    if (-1 == entity->socket) {
        // @TODO: Error handling
        perror("socket");
        fprintf(stderr, "LNX NET ERROR AT %s %d\n", __FILE__, __LINE__);
    }

    NET_Client client = {0};
    client.arena = arena;
    client.family = family;
    client.protocol = protocol;
    client.socket = client_socket;
    client.recv_buffer = make_ring(arena, NET_CLIENT_DEFAULT_BUFFER_SIZE);
    client.send_buffer = make_ring(arena, NET_CLIENT_DEFAULT_BUFFER_SIZE);
    return client;
}

internal NET_Client net_client_connect(NET_Client client, NET_Address target)
{
    struct sockaddr_storage storage = {0};
    lnx_net_address_to_sockaddr_storage(&storage, &target);

    LNX_Entity *entity = (LNX_Entity *)PtrFromInt(client.socket.u64[0]);
    if (-1 == connect(entity->socket, (struct sockaddr *)&storage, sizeof(storage))) {
        // @TODO: Error handling
        perror("connect");
        fprintf(stderr, "LNX NET ERROR AT %s %d\n", __FILE__, __LINE__);
    }
    client.address = target;
    client.connected = true;
    return client;
}

internal s64 net_client_send_raw(NET_Client *client, u32 size, void *data)
{
    s64 result = -1;

    switch (client->protocol)
    {
        case NET_TransportProtocol_TCP: {
            LNX_Entity *entity = (LNX_Entity *)PtrFromInt(client->socket.u64[0]);
            u64 total = 0;
            u64 remaining = size;
            s64 n = 0;
            while (total < size) {
                n = send(entity->socket, (u8 *)data + total, remaining, 0);
                if (-1 == n) {
                    perror("send");
                    result = -1;
                    break;
                }
                total += n;
                remaining -= n;
            }
            if (-1 != n) {
                result = total;
            }
        } break;
        case NET_TransportProtocol_UDP: {
            LNX_Entity *entity = (LNX_Entity *)PtrFromInt(client->socket.u64[0]);
            struct sockaddr_storage dest = {0};
            lnx_net_address_to_sockaddr_storage(&dest, &client->address);
            s64 n = sendto(entity->socket, data, size, 0, (struct sockaddr *)&dest, sizeof(dest));
            if (-1 == n) {
                perror("sendto");
                result = -1;
            }
            else if (n != size) {
                // truncation branch
                // @TODO: Error handling
                //        this should only happen if the message is truncated,
                //        which theoretically shouldn't happen.
            }
            else {
                result = n;
            }
        } break;
        default: {
        } break;
    }

    return result;
}

internal s64 net_client_recv_raw(NET_Client *client, u32 size, void *out)
{
    s64 result = -1;
    LNX_Entity *entity = (LNX_Entity *)PtrFromInt(client->socket.u64[0]);

    switch (client->protocol)
    {
        case NET_TransportProtocol_TCP: {
            int n = recv(entity->socket, (u8 *)out, (int)size, 0);
            if (0 == n) {
                // peer closed the connection
            }
            else if (-1 == n) {
                perror("recv");
            }
            result = (s64)n;
        } break;
        case NET_TransportProtocol_UDP: {
            struct sockaddr_storage from = {0};
            socklen_t fromsize = sizeof(from);

            s64 n = recvfrom(entity->socket, out, size, 0, (struct sockaddr *)&from, &fromsize);
            if (-1 == n) {
                result = -1;
            }
            else {
                result = n;
                lnx_sockaddr_storage_to_net_address(&client->address, &from);
            }
        } break;
        default: {
        } break;
    }
    return result;
}

internal bool32 net_client_send_from_ring(NET_Client *client)
{
    bool32 result = false;
    Temp scratch = scratch_begin(0, 0);

    u64 size = client->send_buffer->write_pos - client->send_buffer->read_pos;
    u8 *data = push_array(scratch.arena, u8, size);
    result = ring_try_read(client->send_buffer, size, data);
    if (result) {
        s64 amount = net_client_send_raw(client, size, data);
        if (-1 == amount) {
            result = false;
        }
    }

    scratch_end(scratch);
    return result;
}

internal bool32 net_client_recv_to_ring(NET_Client *client)
{
    bool32 result = false;
    Temp scratch = scratch_begin(0, 0);

    u64 size = client->recv_buffer->size;
    u8 *data = push_array(scratch.arena, u8, size);
    s64 amount = net_client_recv_raw(client, size, data);
    if (-1 != amount) {
        result = ring_try_write(client->recv_buffer, amount, data);
    }

    scratch_end(scratch);
    return result;
}

internal void net_client_close(NET_Client client)
{
    net_socket_release(client.socket);
}
