
///////////////////////////////////
// Networking Conversion Helpers

internal void lnx_sockaddr_storage_to_net_address(Net_Address *out, struct sockaddr_storage *in)
{
    switch (in->sa_family) {
        case AF_INET: {
            struct sockaddr_in *addr = (struct sockaddr_in *)in;

            out->address_type = Net_AddressType_Ipv4;
            out->port = net_to_host_u16(addr->sin_port);
            out->ip.v4 = net_to_host_u32(addr->sin_addr.s_addr);
        } break;
        case AF_INET6: {
            struct sockaddr_in6 *addr = (struct sockaddr_in6 *)in;

            out->address_type = Net_AddressType_Ipv6;
            out->port = net_to_host_u16(addr->sin6_port);
            MemoryCopyArray(out->ip.v6.u8, addr->sin6_addr.s6_addr);
        } break;
        default: {
            MemoryZero(out);
        } break;
    }
}

internal void lnx_net_address_to_sockaddr_storage(struct sockaddr_storage *out, Net_Address *in)
{
    switch (in->address_type)
    {
        case Net_AddressType_Ipv4: {
            struct sockaddr_in *addr = (struct sockaddr_in *)out;

            addr->sin_family = AF_INET;
            addr->sin_port = host_to_net_u16(in->port);
            addr->sin_addr.s_addr = host_to_net_u32(in->ip.v4);
        } break;
        case Net_AddressType_Ipv6: {
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
// @per_os_impl Networking Primitives

internal Net_Socket net_socket_alloc(Net_AddressType type, Net_TransportProtocol protocol)
{
    LNX_Entity *entity = lnx_entity_alloc(LNX_EntityKind_Socket);

    // @TODO: Finish
}
