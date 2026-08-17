
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
    fprintf(stderr, "\nID: 0x%hX\n",                msg->header.id);
    fprintf(stderr, "Is Query Response: %d\n",      msg->header.query_response);
    fprintf(stderr, "OpCode: %.*s\n",               str8_varg(dns_opcode_to_str8[msg->header.opcode]));
    fprintf(stderr, "Is Authoritative: %d\n",       msg->header.authoritative);
    fprintf(stderr, "Is Truncated: %d\n",           msg->header.truncated);
    fprintf(stderr, "Is Recursion Desired: %d\n",   msg->header.recursion_desired);
    fprintf(stderr, "Is Recursion Available: %d\n", msg->header.recursion_available);
    fprintf(stderr, "Is Authenticated Data: %d\n",  msg->header.authenticated_data);
    fprintf(stderr, "Is Checking Disabled: %d\n",   msg->header.checking_disabled);
    fprintf(stderr, "Response Code: %.*s\n",        str8_varg(dns_rcode_to_str8[msg->header.rcode]));
    fprintf(stderr, "Question Count: %hu\n",        msg->header.question_count);
    fprintf(stderr, "Answer Count: %hu\n",          msg->header.answer_count);
    fprintf(stderr, "Nameserver Count: %hu\n",      msg->header.nameserver_count);
    fprintf(stderr, "Additional Count: %hu\n",      msg->header.additional_count);
    fprintf(stderr, "\n");


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

Test(example_client_exchange)
{
    Temp scratch = scratch_begin(0, 0);
    Dns_Type question_type = Dns_Type_A;

    Dns_Msg msg = dns_msg_alloc(scratch.arena, str8_lit("www.example.org"), question_type);
    Dns_Client client = dns_client_alloc(scratch.arena, Net_AddressType_Ipv4);
    Net_Address address = {0};
    (void)net_str8_to_address(&address, str8_lit("8.8.8.8:53"));

    Dns_Msg response = dns_client_exchange(scratch.arena, client, msg, address);
    
    T_Ok(msg.header.id == response.header.id);
    
    for (u64 i = 0; i < response.header.answer_count; i++) {
        T_Ok(response.answer[i].type == question_type);
    }
    
    scratch_end(scratch);
}

Test(example_client_exchange_nxdomain)
{
    Temp scratch = scratch_begin(0, 0);
    Dns_Type question_type = Dns_Type_A;

    Dns_Msg msg = dns_msg_alloc(scratch.arena, str8_lit("iasldjkosajdf"), question_type);
    Dns_Client client = dns_client_alloc(scratch.arena, Net_AddressType_Ipv4);
    Net_Address address = {0};
    (void)net_str8_to_address(&address, str8_lit("8.8.8.8:53"));

    Dns_Msg response = dns_client_exchange(scratch.arena, client, msg, address);

    T_Ok(msg.header.id == response.header.id);
    T_Ok(response.header.rcode == Dns_RCode_NameError);

    // Authority section should contain the SOA record.
    for (u64 i = 0; i < response.header.nameserver_count; i++) {
        T_Ok(response.ns[i].type == Dns_Type_SOA);
    }
    
    scratch_end(scratch);
}
