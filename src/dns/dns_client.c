

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

internal Dns_Msg *dns_client_exchange(Dns_Client client, Dns_Msg *msg, Net_TransportProtocol protocol, Net_Address address)
{
    Temp scratch = scratch_begin(0, 0);

    u64 off = dns_pack_msg(msg);
    net_client_send_raw(&client.dialer, off, msg->wire);
    /*
    Dns_Msg *response = push_array(scratch.arena, Dns_Msg, 1);
    MemoryCopyStruct(response, msg);
    // make sure we have enough space here probably....

    respone->wire =
    */
    net_client_recv_to_ring(&client.dialer);
    

    scratch_end(scratch);
}


