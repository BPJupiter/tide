

internal Dns_Client dns_client_alloc(Arena *arena, Net_AddressType type)
{
    // @TODO: Handle DNS over TCP and all that.
    Dns_Client client = {0};
    client.dialer = net_client_alloc(arena, type, Net_TransportProtocol_UDP);
    return client;
}

internal void dns_client_release(Dns_Client client)
{
    net_client_close(client.dialer);
}

internal Dns_Msg dns_client_exchange(Arena *arena, Dns_Client client, Dns_Msg msg, Net_Address address)
{
    Dns_Msg result = {0};

    bool32 ok = dns_pack_msg(client.dialer.send_buffer, &msg);
    if (ok) {
        client.dialer.address = address;
        ok = net_client_send_from_ring(&client.dialer);
        if (ok) {
            ok = net_client_recv_to_ring(&client.dialer);
            if (ok) {
                ok = dns_unpack_msg(arena, client.dialer.recv_buffer, &result);
            }
        }
    }
    if (!ok) {
        MemoryZeroStruct(&result);
    }
    return result;
}

