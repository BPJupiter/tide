
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
    fprintf(stderr, "\nID: %hu\n",                  msg->header.id);
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


