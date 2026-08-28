
////////////////////////////
// Error Printing Helpers

internal void w32_print_winsock_error(const char *msg)
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

///////////////////////////////////
// NETworking Conversion Helpers

internal void w32_sockaddr_storage_to_net_address(NET_Address *out, SOCKADDR_STORAGE *in)
{
    switch (in->ss_family)
    {
        case AF_INET: {
            SOCKADDR_IN *addr = (SOCKADDR_IN *)in;
            
            out->family = NET_AddressFamily_IPv4;
            out->port = net_to_host_u16(addr->sin_port);
            out->ip.v4 = net_to_host_u32(addr->sin_addr.S_un.S_addr);
        } break;
        case AF_INET6: {
            // The msdn documentation states that every field of the
            // SOCKADDR_IN6_LH (LH is for "longhorn", and old vista version...)
            // should be in network byte order, except for the family.
            SOCKADDR_IN6_LH *addr = (SOCKADDR_IN6_LH *)in;

            out->family = NET_AddressFamily_IPv6;
            out->port = net_to_host_u16(addr->sin6_port);
            MemoryCopyArray(out->ip.v6.u8, addr->sin6_addr.u.Byte);
        } break;
        default: {
            MemoryZeroStruct(out);
        } break;
    }
}

internal void w32_net_address_to_sockaddr_storage(SOCKADDR_STORAGE *out, NET_Address *in)
{
    switch (in->family)
    {
        case NET_AddressFamily_IPv4: {
            SOCKADDR_IN *addr = (SOCKADDR_IN *)out;

            addr->sin_family = AF_INET;
            addr->sin_port = host_to_net_u16(in->port);
            addr->sin_addr.S_un.S_addr = host_to_net_u32(in->ip.v4);
        } break;
        case NET_AddressFamily_IPv6: {
            SOCKADDR_IN6_LH *addr = (SOCKADDR_IN6_LH *)out;

            addr->sin6_family = AF_INET6;
            addr->sin6_port = host_to_net_u16(in->port);
            addr->sin6_flowinfo = 0;
            MemoryCopyArray(addr->sin6_addr.u.Byte, in->ip.v6.u8);
            addr->sin6_scope_id = 0;
        } break;
        default: {
            MemoryZeroStruct(out);
        } break;
    }
}

/////////////////////////////////////
// @per_os_impl NETworking Primitives

internal NET_Socket net_socket_alloc(NET_AddressFamily family, NET_TransportProtocol protocol)
{
    W32_Entity *entity = w32_entity_alloc(W32_EntityKind_Socket);
    
    u16 af = 0;
    switch(family) {
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
    W32_Entity *entity = (W32_Entity *)PtrFromInt(socket.u64[0]);
    closesocket(entity->socket);
    w32_entity_release(entity);
}

/////////////////////////////////////////////
// @per_os_impl NETwork Listener Functions

internal NET_Listener net_listener_alloc(NET_AddressFamily family, NET_TransportProtocol protocol, u16 port)
{
    SOCKADDR_STORAGE storage = {0};
    NET_Address address = {0};
    address.family = family;
    address.port = port;
    w32_net_address_to_sockaddr_storage(&storage, &address);
    NET_Listener listener = {0};
    listener.socket = net_socket_alloc(family, protocol);

    W32_Entity *entity = (W32_Entity *)PtrFromInt(listener.socket.u64[0]);
    if (INVALID_SOCKET == entity->socket) {
        w32_print_winsock_error("socket");
        // @TODO: Error handling
    }
    if (0 > bind(entity->socket, (SOCKADDR *)&storage, sizeof(storage))) {
        w32_print_winsock_error("bind");
        // @TODO: Error handling
    }
    if (protocol == NET_TransportProtocol_TCP) {
        if (0 > listen(entity->socket, SOMAXCONN)) {
            w32_print_winsock_error("listen");
            // @TODO: Error handling
        }
    }
    {
        int storagelen = sizeof(storage);
        if (0 > getsockname(entity->socket, (SOCKADDR *)&storage, &storagelen))
        {
            w32_print_winsock_error("getsockname");
            // TODO
        }
        switch (storage.ss_family)
        {
            case AF_INET: {
                SOCKADDR_IN *in = (SOCKADDR_IN *)&storage;
                listener.port = ntohs(in->sin_port);
            } break;
            case AF_INET6: {
                SOCKADDR_IN6_LH *in = (SOCKADDR_IN6_LH *)&storage;
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
    SOCKADDR_STORAGE storage = {0};
    int storagelen = sizeof(storage);
    NET_Client client = net_client_alloc(arena, listener.family, listener.protocol);

    switch(listener.protocol)
    {
        case NET_TransportProtocol_TCP: {
            {
                W32_Entity *listen_entity = (W32_Entity *)PtrFromInt(listener.socket.u64[0]);
                SOCKET socket = accept(listen_entity->socket, (SOCKADDR *)&storage, &storagelen);
                if (INVALID_SOCKET == socket) {
                    w32_print_winsock_error("accept");
                    // @TODO: Error handling
                }
                W32_Entity *accept_entity = w32_entity_alloc(W32_EntityKind_Socket);
                accept_entity->socket = socket;
                NET_Socket accept_socket = {IntFromPtr(accept_entity)};
                // This is yuck and currently creates a dummy socket that we have to release.
                // Might be worth duplicating the logic of net_client_alloc
                // if this ends up being a lot of overhead.
                net_socket_release(client.socket);
                client.socket = accept_socket;
                w32_sockaddr_storage_to_net_address(&client.address, &storage);
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
    W32_Entity *entity = (W32_Entity *)PtrFromInt(client_socket.u64[0]);
    if (INVALID_SOCKET == entity->socket) {
        w32_print_winsock_error("socket");
        // @TODO: Error handling
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
    SOCKADDR_STORAGE storage = {0};
    w32_net_address_to_sockaddr_storage(&storage, &target);

    W32_Entity *entity = (W32_Entity *)PtrFromInt(client.socket.u64[0]);
    if (SOCKET_ERROR == connect(entity->socket, (SOCKADDR *)&storage, sizeof(storage))) {
        w32_print_winsock_error("connect");
        // @TODO: Error handling
    }
    client.address = target;
    client.connected = true;
    return client;
}

internal s64 net_client_send_raw(NET_Client *client, u32 size, void *data)
{
    s64 result = -1;
    W32_Entity *entity = (W32_Entity *)PtrFromInt(client->socket.u64[0]);
    
    switch (client->protocol)
    {
        case NET_TransportProtocol_TCP: {
            u64 total = 0;
            u64 remaining = size;
            s64 n = 0;
            while (total < size) {
                n = send(entity->socket, (u8 *)data + total, remaining, 0);
                if (SOCKET_ERROR == n) {
                    w32_print_winsock_error("send");
                    result = -1;
                    break;
                }
                total += n;
                remaining -= n;
            }
            if (SOCKET_ERROR != n) {
                result = total;
            }
        } break;
        case NET_TransportProtocol_UDP: {
            SOCKADDR_STORAGE dest = {0};
            w32_net_address_to_sockaddr_storage(&dest, &client->address);
            int n = sendto(entity->socket, data, size, 0, (SOCKADDR *)&dest, sizeof(dest));
            if (SOCKET_ERROR == n) {
                w32_print_winsock_error("sendto");
                result = -1;
            }
            else if (n != size) {
                result = -1;
                // @TODO: Error handling
                //        this should only happen if the message is truncated,
                //        which theoretically shouldn't happen
            }
            else {
                result = (s64)n;
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
    W32_Entity *entity = (W32_Entity *)PtrFromInt(client->socket.u64[0]);

    switch (client->protocol)
    {
        case NET_TransportProtocol_TCP: {
            int n = recv(entity->socket, (char *)out, (int)size, 0);
            if (0 == n) {
                // peer closed the connection
            }
            else if (SOCKET_ERROR == n) {
                w32_print_winsock_error("recv");
            }
            result = (s64)n;
        } break;
        case NET_TransportProtocol_UDP: {
            SOCKADDR_STORAGE from = {0};
            int fromsize = sizeof(from);

            s64 n = recvfrom(entity->socket, out, size, 0, (SOCKADDR *)&from, &fromsize);
            if (SOCKET_ERROR == n) {
                result = -1;
            }
            else {
                result = n;
                w32_sockaddr_storage_to_net_address(&client->address, &from);
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
