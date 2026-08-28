
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

internal void print_msg_data(DNS_Msg *msg)
{
    Temp scratch = scratch_begin(0, 0);

    String8 header_string = dns_msg_header_to_str8(scratch.arena, msg->header);
    fprintf(stderr, "%.*s\n", str8_varg(header_string));

    for (u64 i = 0; i < msg->header.answer_count; i++) {
        switch (msg->answer[i].type) {
            case DNS_Type_A: {   
                String8 ip = net_ipv4_to_str8(scratch.arena, msg->answer[i].rdata.A.addr);
                printf("%.*s\n", str8_varg(ip));
            } break;
        }
    }
    for (u64 i = 0; i < msg->header.nameserver_count; i++) {
        switch (msg->ns[i].type) {
            case DNS_Type_NS: {
                fprintf(stderr, "%.*s\n", str8_varg(msg->ns[i].rdata.NS.ns));
            } break;
            case DNS_Type_SOA: {
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
    DNS_Type question_type = DNS_Type_A;

    DNS_Msg msg = dns_msg_alloc(scratch.arena, str8_lit("www.example.org"), question_type);
    DNS_Client client = dns_client_alloc(scratch.arena, NET_AddressFamily_IPv4, DNS_TransportProtocol_UDP);
    NET_Address address = {0};
    (void)net_str8_to_address(&address, str8_lit("8.8.8.8:53"));

    DNS_Msg response = dns_client_exchange_with_address(scratch.arena, client, msg, address);
    
    T_Ok(msg.header.id == response.header.id);
    
    for (u64 i = 0; i < response.header.answer_count; i++) {
        T_Ok(response.answer[i].type == question_type);
    }
    
    scratch_end(scratch);
}

Test(stub_client_exchange_with_address_nxdomain)
{
    Temp scratch = scratch_begin(0, 0);
    DNS_Type question_type = DNS_Type_A;

    DNS_Msg msg = dns_msg_alloc(scratch.arena, str8_lit("iasldjkosajdf"), question_type);
    DNS_Client client = dns_client_alloc(scratch.arena, NET_AddressFamily_IPv4, DNS_TransportProtocol_UDP);
    NET_Address address = {0};
    (void)net_str8_to_address(&address, str8_lit("8.8.8.8:53"));

    DNS_Msg response = dns_client_exchange_with_address(scratch.arena, client, msg, address);

    T_Ok(msg.header.id == response.header.id);
    T_Ok(response.header.rcode == DNS_RCode_NXDomain);

    // Authority section should contain the SOA record.
    for (u64 i = 0; i < response.header.nameserver_count; i++) {
        T_Ok(response.ns[i].type == DNS_Type_SOA);
    }
    
    scratch_end(scratch);
}

Test(iterative_lookup)
{
    Temp scratch = scratch_begin(0, 0);

    DNS_Type question_type = DNS_Type_A;

    DNS_Msg msg = dns_msg_alloc(scratch.arena, str8_lit("www.auckland.ac.nz"), question_type);
    msg.header.recursion_desired = false;
    
    DNS_Client client = dns_client_alloc(scratch.arena, NET_AddressFamily_IPv4, DNS_TransportProtocol_UDP);
    

    NET_Address address = {0};
    (void)net_str8_to_address(&address,
                              str8_cat(scratch.arena, dns_ipv4_string_of_root_server(DNS_RootServer_A), s(":53")));

    DNS_Msg response = dns_client_exchange_with_address(scratch.arena, client, msg, address);

    T_Ok(msg.header.id == response.header.id);
    T_Ok(response.header.rcode == DNS_RCode_NoError);

    scratch_end(scratch);
}

internal void server_thread_func(void *params)
{
    ThreadNameF("test_server_thread_func");
    DNS_Server s = *(DNS_Server *)params;
    dns_server_listen_and_serve(s);
}

internal DNS_Server *dns_test_server(Arena *arena, NET_AddressFamily family, DNS_TransportProtocol protocol)
{
    DNS_Server *s = push_array(arena, DNS_Server, 1);
    *s = dns_server_alloc(family, protocol, 0);
    Thread server_thread = thread_launch(server_thread_func, s);
    
    return s;
}

Test(server)
{
    Temp scratch = scratch_begin(0, 0);
    
    struct {
        String8 name;
        NET_AddressFamily family;
        DNS_TransportProtocol network;
    } ts[] = {
        {str8_lit("udp"), NET_AddressFamily_IPv4, DNS_TransportProtocol_UDP},
        //{str8_lit("tcp"), DNS_TransportProtocol_TCP},
    };
    
    for (u64 idx = 0; idx < ArrayCount(ts); idx += 1)
    {
        DNS_Server *async_server = dns_test_server(scratch.arena, ts[idx].family, ts[idx].network);
        u16 port = async_server->listener.port;

        DNS_Msg msg = dns_msg_alloc(scratch.arena, str8_lit("www.example.org"), DNS_Type_A);
        DNS_Client client = dns_client_alloc(scratch.arena, NET_AddressFamily_IPv4, ts[idx].network);
        NET_Address address = {0};
        String8 target = str8_cat(scratch.arena, str8_lit("127.0.0.1"), str8f(scratch.arena, ":%hu", port));
        (void)net_str8_to_address(&address, target);
        DNS_Msg response = dns_client_exchange_with_address(scratch.arena, client, msg, address);
    }
    
    scratch_end(scratch);
}
