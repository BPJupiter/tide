
///////////////////////////////////
// Networking Conversion Helpers

internal void w32_sockaddr_storage_to_net_address(Net_Address *out, SOCKADDR_STORAGE *in)
{
    switch (in->ss_family)
    {
        case AF_INET: {
            SOCKADDR_IN *addr = (SOCKADDR_IN *)in;
            
            out->address_type = Net_AddressType_Ipv4;
            out->port = net_to_host_u16(addr->sin_port);
            out->ip.v4 = net_to_host_u32(addr->sin_addr.S_un.S_addr);
        } break;
        case AF_INET6: {
            // The msdn documentation states that every field of the
            // SOCKADDR_IN6_LH (LH is for "longhorn", and old vista version...)
            // should be in network byte order, except for the family.
            SOCKADDR_IN6_LH *addr = (SOCKADDR_IN6_LH *)in;

            out->address_type = Net_AddressType_Ipv6;
            out->port = net_to_host_u16(addr->sin6_port);
            MemoryCopyArray(out->ip.v6.u8, addr->sin6_addr.u.Byte);
        } break;
        default: {
            // TODO: Error handling
            MemoryZero(out, sizeof(*out));
        } break;
    }
}

internal void w32_net_address_to_sockaddr_storage(SOCKADDR_STORAGE *out, Net_Address *in)
{
    switch (in->address_type)
    {
        case Net_AddressType_Ipv4: {
            SOCKADDR_IN *addr = (SOCKADDR_IN *)out;

            addr->sin_family = AF_INET;
            addr->sin_port = host_to_net_u16(in->port);
            addr->sin_addr.S_un.S_addr = host_to_net_u32(in->ip.v4);
        } break;
        case Net_AddressType_Ipv6: {
            SOCKADDR_IN6_LH *addr = (SOCKADDR_IN6_LH *)out;

            addr->sin6_family = AF_INET6;
            addr->sin6_port = host_to_net_u16(in->port);
            addr->sin6_flowinfo = 0;
            MemoryCopyArray(addr->sin6_addr.u.Byte, in->ip.v6.u8);
            addr->sin6_scope_id = 0;
        } break;
        default: {
            // TODO: Error handling
            MemoryZero(out, sizeof(*out));
        } break;
    }
}

/////////////////////////////////////
// @per_os_impl Networking Primitives

internal Net_Socket net_socket_alloc(Net_AddressType type, Net_TransportProtocol protocol)
{
    W32_Entity *entity = w32_entity_alloc(W32_EntityKind_Socket);
    
    u16 family = 0;
    switch(type) {
        default:
        case Net_AddressType_Any:
        case Net_AddressType_Ipv4: {
            family = AF_INET;
        } break;
        case Net_AddressType_Ipv6: {
            family = AF_INET6;
        } break;
    }
    
    switch (protocol) {
        default:
        case Net_TransportProtocol_RAW: {
            entity->socket = socket(family, SOCK_RAW, 0);
        } break;
        case Net_TransportProtocol_TCP: {
            entity->socket = socket(family, SOCK_STREAM, 0);
        } break;
        case Net_TransportProtocol_UDP: {
            entity->socket = socket(family, SOCK_DGRAM, 0);
        } break;
    }
    
    Net_Socket socket = {IntFromPtr(entity)};
    return socket;
}

internal void net_socket_release(Net_Socket socket)
{
    W32_Entity *entity = (W32_Entity *)PtrFromInt(socket.u64[0]);
    closesocket(entity->socket);
    w32_entity_release(entity);
}

/////////////////////////////////////////////
// @per_os_impl Network Listener Functions

internal Net_Listener net_listener_alloc(Net_AddressType type, Net_TransportProtocol protocol, u16 port)
{
    SOCKADDR_STORAGE storage = {0};
    Net_Address address = {0};
    address.address_type = type;
    address.port = port;
    w32_net_address_to_sockaddr_storage(&storage, &address);
    Net_Listener listener = {0};
    {
        listener.port = port;
        listener.type = type;
        listener.protocol = protocol;
        listener.socket = net_socket_alloc(type, protocol);
    }
    W32_Entity *entity = (W32_Entity *)PtrFromInt(listener.socket.u64[0]);
    if (INVALID_SOCKET == entity->socket) {
        // TODO: Error handling
    }
    if (0 > bind(entity->socket, (SOCKADDR *)&storage, sizeof(storage))) {
        // TODO: Error handling
    }
    if (0 > listen(entity->socket, SOMAXCONN)) {
        // TODO: Error handling
    }
    return listener;
}

internal Net_Client net_listener_accept(Arena *arena, Net_Listener listener)
{
    SOCKADDR_STORAGE storage = {0};
    int storagelen = sizeof(storage);

    Net_Socket accept_socket = net_socket_alloc(listener.type, listener.protocol);
    W32_Entity *accept_entity = (W32_Entity *)PtrFromInt(accept_socket.u64[0]);
    W32_Entity *listen_entity = (W32_Entity *)PtrFromInt(listener.socket.u64[0]);
    accept_entity->socket = accept(listen_entity->socket, (SOCKADDR *)&storage, &storagelen);
    if (INVALID_SOCKET == accept_entity->socket) {
        // TODO: Error handling
    }
    // This is yuck and currently creates a dummy socket that we have to release.
    // Might be worth duplicating the logic of net_client_alloc
    // if this ends up being a lot of overhead.
    Net_Client client = net_client_alloc(arena, listener.type, listener.protocol);
    net_socket_release(client.socket);
    client.socket = accept_socket;
    w32_sockaddr_storage_to_net_address(&client.address, &storage);
    client.connected = true;
    
    return client;
}

internal void net_listener_close(Net_Listener listener)
{
    net_socket_release(listener.socket);
}

///////////////////////////////////////////
// @per_os_impl Network Client Functions

internal Net_Client net_client_alloc(Arena *arena, Net_AddressType type, Net_TransportProtocol protocol)
{
    Net_Socket client_socket = net_socket_alloc(type, protocol);
    W32_Entity *entity = (W32_Entity *)PtrFromInt(client_socket.u64[0]);
    if (INVALID_SOCKET == entity->socket) {
        // TODO: Error handling
    }

    Net_Client client = {0};
    client.arena = arena;
    client.type = type;
    client.protocol = protocol;
    client.socket = client_socket; // We could make this call the responsibility of the caller?
    
    // TODO: I need some sort of compass for how large to make these buffers.
    // Currently 16kB is a wild guess.
    client.recv_buffer = make_ring(arena, Kilobytes(16));
    client.send_buffer = make_ring(arena, Kilobytes(16));
    return client;
}

internal Net_Client net_client_connect(Net_Client client, Net_Address target)
{
    SOCKADDR_STORAGE storage = {0};
    w32_net_address_to_sockaddr_storage(&storage, &target);

    W32_Entity *entity = (W32_Entity *)PtrFromInt(client.socket.u64[0]);
    if (SOCKET_ERROR == connect(entity->socket, (SOCKADDR *)&storage, sizeof(storage))) {
        // TODO: Error handling
    }
    client.address = target;
    client.connected = true;
    return client;
}

internal s64 net_client_send_raw(Net_Client *client, u32 size, void *data)
{
    s64 result = -1;
    
    switch (client->protocol)
    {
        case Net_TransportProtocol_TCP: {
            W32_Entity *entity = (W32_Entity *)PtrFromInt(client->socket.u64[0]);
            u64 total = 0;
            u64 remaining = size;
            s64 n = 0;
            while (total < size) {
                n = send(entity->socket, (u8 *)data + total, remaining, 0);
                if (SOCKET_ERROR == n) {
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
        case Net_TransportProtocol_UDP: {
            W32_Entity *entity = (W32_Entity *)PtrFromInt(client->socket.u64[0]);
            SOCKADDR_STORAGE dest = {0};
            s64 n = sendto(entity->socket, data, size, 0, (SOCKADDR *)&dest, sizeof(dest));
            w32_sockaddr_storage_to_net_address(&client->address, (SOCKADDR_STORAGE *)&dest);
            if (SOCKET_ERROR == n) {
                result = -1;
            }
            else if (n != size) {
                // TODO: Error handling
                //       this should only happen if the message is truncated,
                //       which theoretically shouldn't happen
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

internal s64 net_client_recv_raw(Net_Client *client, u32 size, void *out)
{
    s64 result = -1;

    switch (client->protocol)
    {
        case Net_TransportProtocol_TCP: {
            W32_Entity *entity = (W32_Entity *)PtrFromInt(client->socket.u64[0]);
            u64 total = 0;
            u64 remaining = size;
            s64 n = 0;

            while (total < size) {
                n = recv(entity->socket, (u8 *)out + total, remaining, 0);
                if (0 == n) {
                    // peer closed the connection
                    break;
                }
                else if (SOCKET_ERROR == n) {
                    // network error
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
        case Net_TransportProtocol_UDP: {
            W32_Entity *entity =(W32_Entity *)PtrFromInt(client->socket.u64[0]);
            SOCKADDR_STORAGE from = {0};
            int fromsize = sizeof(from);

            s64 n = recvfrom(entity->socket, out, size, 0, (SOCKADDR *)&from, &fromsize);
            if (SOCKET_ERROR == n) {
                result = -1;
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

internal bool32 net_client_send_from_ring(Net_Client *client)
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

internal bool32 net_client_recv_to_ring(Net_Client *client)
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

internal void net_client_close(Net_Client client)
{
    net_socket_release(client.socket);
}
