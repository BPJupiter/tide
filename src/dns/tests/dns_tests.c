
Test(get_local_dns)
{
    Temp scratch = scratch_begin(0, 0);

    String8_List local_dns_addresses = dns_get_local_nameservers(scratch.arena);
    for (String8_Node *n = local_dns_addresses.first; n != 0; n = n->next) {
        T_Ok(net_str8_to_ipv4(0, n->string));
        printf("%.*s\n", str8_varg(n->string));
    }

    scratch_end(scratch);
}

Test(example_client_exchange)
{
    Temp scratch = scratch_begin(0, 0);
    
    Dns_Msg *msg = dns_msg_alloc(scratch.arena, str8_lit("www.example.org"), Dns_Type_A);
    Dns_Client client = dns_client_alloc(scratch.arena, Net_AddressType_Ipv4);
    Net_Address address = {0};
    (void)net_str8_to_address(&address, str8_lit("8.8.8.8:53"));

    // @REMOVE
    u64 off = dns_pack_msg(msg);
    String8 hexdump = hexdump_str8(scratch.arena,
                                   msg->wire,
                                   off);
    printf("\n%.*s\n", str8_varg(hexdump));
    client.dialer.address = address;
    T_Ok(0 < net_client_send_raw(&client.dialer, off, msg->wire));
    T_Ok(net_client_recv_to_ring(&client.dialer));
    hexdump = hexdump_str8(scratch.arena,
                                   client.dialer.recv_buffer->base + client.dialer.recv_buffer->read_pos,
                                   ring_peek_unread_quantity(client.dialer.recv_buffer));
    printf("\n%.*s\n", str8_varg(hexdump));
    /*
    Dns_Msg *response = dns_client_exchange(client, msg, Net_TransportProtocol_UDP, address);

    for (u64 i = 0; i < msg->header.answer_count; i++) {
        Dns_RR rr = response->answer[i];
        if (rr.type == Dns_Type_A) {
            String8 address = net_ipv4_to_str8(scratch.arena, rr.rdata.A.addr);
            fprintf(stderr, "%.*s\n", str8_varg(address));
            T_Ok(rr.rdata.A.addr);
        }
    }
    */
    
    scratch_end(scratch);
}


