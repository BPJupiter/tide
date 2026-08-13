

internal Dns_Client dns_client_alloc(Arena *arena, Net_AddressType type)
{
    Dns_Client client = {0};
    client.dialer = net_client_alloc(arena, type, Net_TransportProtocol_UDP);
    return client;
}

internal void dns_client_release(Dns_Client client)
{
    net_client_close(client.dialer);
}

internal Dns_Msg *dns_client_exchange(Dns_Client client, Dns_Msg *msg, Net_TransportProtocol protocol, Net_Address address)
{
    Temp scratch = scratch_begin(0, 0);

    

    scratch_end(scratch);
}
