
Test(get_local_dns)
{
    Temp scratch = scratch_begin(0, 0);

    String8_List local_dns_addresses = dns_get_local_nameservers(scratch.arena);
    for (String8_Node *n = local_dns_addresses.first; n != 0; n = n->next) {
        T_Ok(net_str8_to_ipv4(0, n->string) || net_str8_to_ipv6(0, n->string));
        //printf("%.*s\n", str8_varg(n->string));
    }

    scratch_end(scratch);
}

internal void print_msg_data(Dns_Msg *msg)
{
    Temp scratch = scratch_begin(0, 0);

    String8 header_string = dns_msg_header_to_str8(scratch.arena, msg->header);
    fprintf(stderr, "%.*s\n", str8_varg(header_string));

    for (u64 i = 0; i < msg->header.answer_count; i++) {
        switch (msg->answer[i].type) {
            case Dns_Type_A: {   
                String8 ip = net_ipv4_to_str8(scratch.arena, msg->answer[i].rdata.A.addr);
                printf("%.*s\n", str8_varg(ip));
            } break;
        }
    }
    for (u64 i = 0; i < msg->header.nameserver_count; i++) {
        switch (msg->ns[i].type) {
            case Dns_Type_NS: {
                fprintf(stderr, "%.*s\n", str8_varg(msg->ns[i].rdata.NS.ns));
            } break;
            case Dns_Type_SOA: {
                fprintf(stderr, "%.*s\n", str8_varg(msg->ns[i].rdata.SOA.master_name));
                fprintf(stderr, "%.*s\n", str8_varg(msg->ns[i].rdata.SOA.responsible_name));
                fprintf(stderr, "Serial: %u\n", msg->ns[i].rdata.SOA.serial);
                fprintf(stderr, "Refresh: %u\n", msg->ns[i].rdata.SOA.refresh);
                fprintf(stderr, "Retry: %u\n", msg->ns[i].rdata.SOA.retry);
                fprintf(stderr, "Expire: %u\n", msg->ns[i].rdata.SOA.expire);
                fprintf(stderr, "Minimum: %u\n", msg->ns[i].rdata.SOA.minimum);
            } break;
        }
    }
    scratch_end(scratch);
}

Test(stub_client_exchange_with_address)
{
    // @TODO: Make this work with TCP
    Temp scratch = scratch_begin(0, 0);
    Dns_Type question_type = Dns_Type_A;

    Dns_Msg msg = dns_msg_alloc(scratch.arena, str8_lit("www.example.org"), question_type);
    Dns_Client client = dns_client_alloc(scratch.arena, Net_AddressFamily_IPv4, Dns_TransportProtocol_UDP);
    Net_Address address = {0};
    (void)net_str8_to_address(&address, str8_lit("8.8.8.8:53"));

    Dns_Msg response = dns_client_exchange_with_address(scratch.arena, client, msg, address);
    
    T_Ok(msg.header.id == response.header.id);
    
    for (u64 i = 0; i < response.header.answer_count; i++) {
        T_Ok(response.answer[i].type == question_type);
    }
    
    scratch_end(scratch);
}

Test(stub_client_exchange_with_address_nxdomain)
{
    Temp scratch = scratch_begin(0, 0);
    Dns_Type question_type = Dns_Type_A;

    Dns_Msg msg = dns_msg_alloc(scratch.arena, str8_lit("iasldjkosajdf"), question_type);
    Dns_Client client = dns_client_alloc(scratch.arena, Net_AddressFamily_IPv4, Dns_TransportProtocol_UDP);
    Net_Address address = {0};
    (void)net_str8_to_address(&address, str8_lit("8.8.8.8:53"));

    Dns_Msg response = dns_client_exchange_with_address(scratch.arena, client, msg, address);

    T_Ok(msg.header.id == response.header.id);
    T_Ok(response.header.rcode == Dns_RCode_NameError);

    // Authority section should contain the SOA record.
    for (u64 i = 0; i < response.header.nameserver_count; i++) {
        T_Ok(response.ns[i].type == Dns_Type_SOA);
    }
    
    scratch_end(scratch);
}

Test(iterative_lookup)
{
    Temp scratch = scratch_begin(0, 0);

    Dns_Type question_type = Dns_Type_A;

    Dns_Msg msg = dns_msg_alloc(scratch.arena, str8_lit("www.auckland.ac.nz"), question_type);
    msg.header.recursion_desired = false;
    
    Dns_Client client = dns_client_alloc(scratch.arena, Net_AddressFamily_IPv4, Dns_TransportProtocol_UDP);
    

    String8 root_ip = str8_cat(scratch.arena,
                               net_ipv4_to_str8(scratch.arena, dns_root_server_to_ipv4[Dns_RootServer_A]),
                               s(":53"));
    Net_Address address = {0};
    (void)net_str8_to_address(&address, root_ip);

    Dns_Msg response = dns_client_exchange_with_address(scratch.arena, client, msg, address);

    T_Ok(msg.header.id == response.header.id);
    T_Ok(response.header.rcode == Dns_RCode_Success);

    scratch_end(scratch);
}

Test(server)
{
    struct {
        String8 name;
        Dns_TransportProtocol network;
        String8 addr;
    } test_server[] = {
        {str8_lit("udp"), Dns_TransportProtocol_UDP, str8_lit(":0")},
        //{str8_lit("tcp"), Dns_TransportProtocol_TCP, str8_lit(":0")},
    };
    for (u64 ts; ts < ArrayCount(test_server); ts += 1)
    {
        
    }
}
