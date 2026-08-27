
/////////////
// Globals

// dns_id_func() by default returns a 16-bit random number to be used as a message id.
// the number is planned to be drawn from a cryptographically secure random number
// generator, but for now just returns a static value.
// This being a variable the function can be reassigned to a custom function.
// For instance, to make it return a static value for testing.

internal u16 dns_id_func_default(void);
internal u16 (*dns_id_func)(void) = dns_id_func_default;

internal u16 dns_id_func_default(void)
{
    // @TODO: Make this real
    return 0xCAFE;
}

///////////////////////////
// DNS Message Functions

internal Dns_Msg dns_msg_alloc(Arena *arena, String8 domain, Dns_Type type)
{
    Dns_Msg msg = {0};
    msg.header.id = dns_id_func();
    msg.header.recursion_desired = true;
    msg.header.question_count = 1;
    msg.question = push_array(arena, Dns_RR, 1);
    msg.question[0].name = str8_to_fqdn(arena, domain);
    msg.question[0].class = Dns_Class_INET;
    msg.question[0].type = type;
    return msg;
}

internal String8 dns_msg_header_to_str8(Arena *arena, Dns_Msg_Header h)
{
    Temp scratch = scratch_begin(&arena, 1);
    
    String8_List sb;
    str8_serial_begin(scratch.arena, &sb);
    (void)str8_serial_push_string(scratch.arena, &sb, s(";; "));
    (void)str8_serial_push_string(scratch.arena, &sb, dns_opcode_to_str8[h.opcode]);
    (void)str8_serial_push_string(scratch.arena, &sb, s(", rcode: "));
    (void)str8_serial_push_string(scratch.arena, &sb, dns_rcode_to_str8[h.rcode]);
    (void)str8_serial_push_string(scratch.arena, &sb, s(", id: "));
    (void)str8_serial_push_string(scratch.arena, &sb, str8f(scratch.arena, "%hu", h.id));
    (void)str8_serial_push_string(scratch.arena, &sb, s(","));

    (void)str8_serial_push_string(scratch.arena, &sb, s(" flags:"));
    if (h.query_response)      (void)str8_serial_push_string(scratch.arena, &sb, s(" qr"));
    if (h.authoritative)       (void)str8_serial_push_string(scratch.arena, &sb, s(" aa"));
    if (h.truncated)           (void)str8_serial_push_string(scratch.arena, &sb, s(" tc"));
    if (h.recursion_desired)   (void)str8_serial_push_string(scratch.arena, &sb, s(" rd"));
    if (h.recursion_available) (void)str8_serial_push_string(scratch.arena, &sb, s(" ra"));
    if (h.zero)                (void)str8_serial_push_string(scratch.arena, &sb, s(" z"));
    if (h.authenticated_data)  (void)str8_serial_push_string(scratch.arena, &sb, s(" ad"));
    if (h.checking_disabled)   (void)str8_serial_push_string(scratch.arena, &sb, s(" cd"));

    (void)str8_serial_push_string(scratch.arena, &sb, s("\n"));
    (void)str8_serial_push_string(scratch.arena, &sb, s(";; "));
    (void)str8_serial_push_string(scratch.arena, &sb, s("QUESTION: "));
    (void)str8_serial_push_string(scratch.arena, &sb, str8f(scratch.arena, "%hu", h.question_count));
    (void)str8_serial_push_string(scratch.arena, &sb, s(", ANSWER: "));
    (void)str8_serial_push_string(scratch.arena, &sb, str8f(scratch.arena, "%hu", h.answer_count));
    (void)str8_serial_push_string(scratch.arena, &sb, s(", AUTHORTIY: "));
    (void)str8_serial_push_string(scratch.arena, &sb, str8f(scratch.arena, "%hu", h.nameserver_count));
    (void)str8_serial_push_string(scratch.arena, &sb, s(", ADDITIONAL: "));
    (void)str8_serial_push_string(scratch.arena, &sb, str8f(scratch.arena, "%hu", h.additional_count));
    (void)str8_serial_push_string(scratch.arena, &sb, s("\n"));
    String8 result = str8_serial_end(arena, &sb);
    
    scratch_end(scratch);
    return result;
}

//////////////////////
// Client Functions

internal Dns_Client dns_client_alloc(Arena *arena, Net_AddressFamily family, Dns_TransportProtocol protocol)
{
    Dns_Client client = {0};
    
    client.dns_protocol = protocol;
    Net_TransportProtocol ipproto = 0;
    switch (protocol)
    {
        case Dns_TransportProtocol_TLS:
        case Dns_TransportProtocol_HTTPS:
        case Dns_TransportProtocol_TCP: {
            ipproto = Net_TransportProtocol_TCP;
        } break;
        case Dns_TransportProtocol_UDP: {
            ipproto = Net_TransportProtocol_UDP;
        } break;
    }

    client.dialer = net_client_alloc(arena, family, ipproto);
    
    return client;
}

internal void dns_client_release(Dns_Client client)
{
    net_client_close(client.dialer);
}

internal Dns_Msg dns_client_exchange_with_address(Arena *arena, Dns_Client client, Dns_Msg msg, Net_Address address)
{
    // @Cleanup: make this not have 1 million dns_protocol checks.
    //           probably dispatch instead.
    Dns_Msg result = {0};
    
    bool32 ok = true;
    if (client.dns_protocol == Dns_TransportProtocol_TCP) {
        (void)net_client_connect(client.dialer, address);
        u64 length64 = dns_msg_wire_length(&msg);
        u16 length16 = safe_cast_u16(safe_cast_u32(length64));
        ok &= ring_try_write(client.dialer.send_buffer, 2, &length16);
    }
    ok &= dns_pack_msg(client.dialer.send_buffer, &msg);
    if (ok) {
        client.dialer.address = address;
        ok = net_client_send_from_ring(&client.dialer);
        if (ok) {
            ok = net_client_recv_to_ring(&client.dialer);
            if (ok) {
                u16 unpacklen = 0;
                if (client.dns_protocol == Dns_TransportProtocol_TCP) {
                    ok &= ring_try_read(client.dialer.recv_buffer, 2, &unpacklen);
                }
                u16 unread = ring_peek_unread_quantity(client.dialer.recv_buffer);
                ok = dns_unpack_msg(arena, client.dialer.recv_buffer, &result);
                if (client.dns_protocol == Dns_TransportProtocol_TCP) {
                    //ok &= (unpacklen == unread);
                    //printf("%hu, %hu\n", unpacklen, unread);
                }
            }
        }
    }
    if (!ok) {
        MemoryZeroStruct(&result);
    }

    return result;
}

////////////////////
// Utility Functions

internal String8 str8_to_fqdn(Arena *arena, String8 s)
{
    if (!str8_is_fqdn(s)) {
        s = str8_cat(arena, s, str8_lit("."));
    }
    return str8_copy(arena, s);
}

internal bool32 str8_is_fqdn(String8 s)
{
    return str8_ends_with(s, str8_lit("."), 0);
}

internal String8 str8_to_canonical(Arena *arena, String8 s)
{
    Temp scratch = scratch_begin(&arena, 1);
    s = lower_from_str8(scratch.arena, s);
    String8 result = str8_to_fqdn(arena, s);
    scratch_end(scratch);
    return result;
}

internal String8 str8_to_name_labels(Arena *arena, String8 s)
{
    Temp scratch = scratch_begin(&arena, 1);

    s = str8_to_canonical(scratch.arena, s);

    u64 out_cap = s.size + 1;
    u8 *out = push_array(arena, u8, out_cap);
    u64 out_len = 0;

    if (s.size == 1 && s.str[0] == '.') {
        out[out_len++] = 0; // root domain
    }
    else {
        u64 begin = 0;
        while (begin < s.size) {

            u64 i = str8_find_needle(s, begin, s("."), 0);
            if (i == s.size) {
                break;
            }

            u64 label_len = i - begin;

            if (label_len > 0) {
                if (label_len > DNS_MAX_LABEL_LEN) {
                    label_len = DNS_MAX_LABEL_LEN;
                }

                out[out_len++] = (u8)label_len;

                for (u64 j = 0; j < label_len; j++) {
                    out[out_len++] = s.str[begin + j];
                }
            }

            begin = i + 1;
        }

        out[out_len++] = 0;
    }

    String8 result;
    result.str = out;
    result.size = out_len;
    
    scratch_end(scratch);
    return result;
}

internal bool32 str8_is_name_labels(String8 s)
{
    bool32 result = true;
    const u64 msg_len = 256;
    u64 ls = s.size;

    bool32 root_string = true;
    if (ls != 1)         root_string = false;
    if (s.str[0] != '.') root_string = false;
    
    if (!root_string) {
        u64 off = 0;
        u64 begin = 0;
        while (begin < ls) {
            u32 i = str8_find_needle(s, begin, str8_lit("."), 0);
            if (i == s.size) {
                break;
            }

            u64 label_len = i - begin;

            // top two bits of length must be clear and two dots back to back
            // is not legal
            if (label_len == 0 || label_len >= (1 << 6)) {
                result = false;
                break;
            }

            off += 1 + label_len;
            if (off > msg_len) {
                result = false;
                break;
            }
            begin = i + 1;
        }
    }

    return result;
}

//////////////////
// Wire Legnths

internal u64 dns_rdata_wire_length(Dns_RR *rr)
{
    u64 l = 0;

    switch (rr->type) {
        case Dns_Type_A: {
            l += sizeof(rr->rdata.A.addr);
        } break;
        case Dns_Type_NS: {
            l += rr->rdata.NS.ns.size + 1;
        } break;
        case Dns_Type_CNAME: {
            l += rr->rdata.CNAME.target.size + 1;
        } break;
        case Dns_Type_AAAA: {
            l += sizeof(rr->rdata.AAAA.addr);
        } break;
        default: {
            DNS_CRASH_THE_PROGRAM_IF_THIS_TYPE_IS_SUPPORTED(rr->type);
        } break;
    }

    return l;
}

internal u64 dns_rr_wire_length(Dns_RR *rr)
{
    /*
                                    1  1  1  1  1  1
      0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                                               |
    /                                               /
    /                      NAME                     /
    |                                               |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                      TYPE                     |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                     CLASS                     |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                      TTL                      |
    |                                               |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                   RDLENGTH                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
    /                     RDATA                     /
    /                                               /
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    
    */

    u64 l = rr->name.size + 1 + 10; // +1 because example.com is actually .example.com
    l += dns_rdata_wire_length(rr);

    return l;
}

internal u64 dns_msg_wire_length(Dns_Msg *msg)
{
    u64 i = 0;
    u64 l = DNS_MSG_HEADER_SIZE;

    // we always add a +1, even if the name is a root label.
    // 4 is for the type and class.

    for (i = 0; i < msg->header.question_count; i++) {
        l += msg->question[i].name.size + 1;
    }
    l += 4;

    for (i = 0; i < msg->header.answer_count; i++) {
        l += dns_rr_wire_length(&msg->answer[i]);
    }

    for (i = 0; i < msg->header.nameserver_count; i++) {
        l += dns_rr_wire_length(&msg->ns[i]);
    }

    for (i = 0; i < msg->header.additional_count; i++) {
        l += dns_rr_wire_length(&msg->extra[i]);
    }

    return Min(l, DNS_MAX_MSG_SIZE);
}

internal bool32 dns_is_blocked_on_this_network(Dns_TransportProtocol protocol)
{
    bool32 result = true;
    Temp scratch = scratch_begin(0, 0);

    Dns_Client udp_client = dns_client_alloc(scratch.arena, Net_AddressFamily_IPv4, Dns_TransportProtocol_UDP);
    Dns_Client tcp_client = dns_client_alloc(scratch.arena, Net_AddressFamily_IPv4, Dns_TransportProtocol_TCP);
    for (u64 i = 0; i < Dns_RootServer_COUNT; i++) {
        Dns_Msg msg = dns_msg_alloc(scratch.arena, str8_lit("www.example.org"), Dns_Type_A);
        Net_Address address;
        String8 root_ip = net_ipv4_to_str8(scratch.arena, dns_root_server_to_ipv4[i]);
        (void)net_str8_to_address(&address, str8_cat(scratch.arena, root_ip, str8_lit(":53")));
        bool32 ok = dns_pack_msg(udp_client.dialer.send_buffer, &msg);
        ok &= dns_pack_msg(tcp_client.dialer.send_buffer, &msg);
    }

    scratch_end(scratch);
    return result;
}
