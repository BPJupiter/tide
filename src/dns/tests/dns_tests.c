
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
    Dns_Msg *response = dns_client_exchange(client, msg, Net_TransportProtocol_UDP, address);

    for (Dns_RR *n = msg->answer; n != 0; n = n->next) {
        if (n->type == Dns_Type_A) {
            String8 address = net_ipv4_to_str8(scratch.arena, n->rdata.A);
            fprintf(stderr, "%.*s\n", str8_varg(address));
            T_Ok(n->rdata.A);
        }
    }
    
    scratch_end(scratch);
}
