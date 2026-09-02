
///////////////////
// Generated Code

#include "generated/dns.meta.c"

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

internal DNS_Msg dns_msg_alloc(Arena *arena, String8 domain, DNS_Type type)
{
    DNS_Msg msg = {0};
    msg.header.id = dns_id_func();
    msg.header.recursion_desired = true;
    msg.header.question_count = 1;
    msg.question = push_array(arena, DNS_RR, 1);
    msg.question[0].name = str8_to_fqdn(arena, domain);
    msg.question[0].class = DNS_Class_IN;
    msg.question[0].type = type;
    return msg;
}

internal String8 dns_msg_header_to_str8(Arena *arena, DNS_Msg_Header h)
{
    Temp scratch = scratch_begin(&arena, 1);
    
    String8_List sb;
    str8_serial_begin(scratch.arena, &sb);
    (void)str8_serial_push_string(scratch.arena, &sb, s(";; "));
    (void)str8_serial_push_string(scratch.arena, &sb, dns_string_from_opcode(h.opcode));
    (void)str8_serial_push_string(scratch.arena, &sb, s(", rcode: "));
    (void)str8_serial_push_string(scratch.arena, &sb, dns_string_from_rcode(h.rcode));
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

internal DNS_Client dns_client_alloc(Arena *arena, NET_AddressFamily family, DNS_TransportProtocol protocol)
{
    DNS_Client client = {0};
    
    client.dns_protocol = protocol;
    NET_TransportProtocol ipproto = 0;
    switch (protocol)
    {
        default:{}break;
        case DNS_TransportProtocol_TLS:
        case DNS_TransportProtocol_HTTPS:
        case DNS_TransportProtocol_TCP: {
            ipproto = NET_TransportProtocol_TCP;
        } break;
        case DNS_TransportProtocol_UDP: {
            ipproto = NET_TransportProtocol_UDP;
        } break;
    }

    client.dialer = net_client_alloc(arena, family, ipproto);
    
    return client;
}

internal void dns_client_release(DNS_Client client)
{
    net_client_close(client.dialer);
}

internal DNS_Msg dns_exchange(Arena *arena, DNS_Msg msg, DNS_TransportProtocol protocol, String8 target)
{
    // TODO
    DNS_Msg result = {0};
    return result;
}

internal DNS_Msg dns_client_exchange(Arena *arena, DNS_Client client, DNS_Msg msg, String8 target)
{
    // TODO
    DNS_Msg result = {0};
    return result;
}

internal DNS_Msg dns_client_exchange_with_address(Arena *arena, DNS_Client client, DNS_Msg msg, NET_Address address)
{
    // @Cleanup: make this not have 1 million dns_protocol checks.
    //           probably dispatch instead.
    DNS_Msg result = {0};
    bool32 ok = true;

    switch (client.dns_protocol)
    {
        default:{}break;
        case DNS_TransportProtocol_UDP: {
            {
                client.dialer.address = address;
                ok = dns_pack_msg(client.dialer.send_buffer, &msg);
                if (!ok) goto end;
                ok = net_client_send_from_ring(&client.dialer);
                if (!ok) goto end;
                ok = net_client_recv_to_ring(&client.dialer);
                if (!ok) goto end;
                ok = dns_unpack_msg(arena, client.dialer.recv_buffer, &result);
            }
        } break;
        case DNS_TransportProtocol_TCP: {
            {
                net_client_connect(client.dialer, address);
                u64 length64 = dns_msg_wire_length(&msg);
                u16 length16 = host_to_net_u16(safe_cast_u16(safe_cast_u32(length64)));
                ok = ring_try_write_struct(client.dialer.send_buffer, &length16);
                if (!ok) goto end;
                ok = dns_pack_msg(client.dialer.send_buffer, &msg);
                if (!ok) goto end;
                ok = net_client_send_from_ring(&client.dialer);
                if (!ok) goto end;
                ok = net_client_recv_to_ring(&client.dialer);
                if (!ok) goto end;
                u16 unpacklen = 0;
                ok = ring_try_read_struct(client.dialer.recv_buffer, &unpacklen);
                unpacklen = net_to_host_u16(unpacklen);
                if (!ok) goto end;
                u16 unread = ring_peek_unread_quantity(client.dialer.recv_buffer);
                ok = dns_unpack_msg(arena, client.dialer.recv_buffer, &result);
                ok = (unpacklen == unread);
            }
        } break;
    }

 end:;
    if (!ok)
    {
        MemoryZeroStruct(&result);
    }

    return result;
}

////////////////////////
// Server Functions

internal DNS_Server dns_server_alloc(NET_AddressFamily family, DNS_TransportProtocol protocol, u16 port)
{
    DNS_Server server = {0};

    server.dns_protocol = protocol;
    NET_TransportProtocol ipproto = 0;
    switch (protocol)
    {
        default:{}break;
        case DNS_TransportProtocol_TLS:
        case DNS_TransportProtocol_HTTPS:
        case DNS_TransportProtocol_TCP: {
            ipproto = NET_TransportProtocol_TCP;
        } break;
        case DNS_TransportProtocol_UDP: {
            ipproto = NET_TransportProtocol_UDP;
        } break;
    }
    server.listener = net_listener_alloc(family, ipproto, port);
    
    return server;
}

internal void dns_server_release(DNS_Server server)
{
    NOTIMPL_WARNING(dns_server_release);
    // TODO
}

internal void dns_listen_and_serve(String8 address, DNS_TransportProtocol protocol)
{
    NOTIMPL_WARNING(dns_listen_and_serve);
    // TODO
}

internal void dns_server_listen_and_serve(DNS_Server server)
{
    // TODO: Sanity checking on server.dns_protocol and server.listener.protocol
    Temp scratch = scratch_begin(0, 0);
    switch(server.dns_protocol)
    {
        default:{}break;
        case DNS_TransportProtocol_UDP: {
            {
                NET_Client client = net_listener_accept(scratch.arena, server.listener);
                u8 foo = 255;
                ring_try_write_struct(client.send_buffer, &foo);
                net_client_send_from_ring(&client);
            }
        } break;
    }
    scratch_end(scratch);
}

internal void dns_server_shutdown(DNS_Server *server)
{
    NOTIMPL_WARNING(dns_server_shutdown);
}

internal void dns_server_shutdown_and_release(DNS_Server *server)
{
    NOTIMPL_WARNING(dns_server_shutdown_and_release);
    // TODO
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

internal u64 dns_rdata_wire_length(DNS_RR *rr)
{
    u64 l = 0;

    switch (rr->type) {
        case DNS_Type_A: {
            l += sizeof(rr->rdata.A.addr);
        } break;
        case DNS_Type_NS: {
            l += rr->rdata.NS.ns.size + 1;
        } break;
        case DNS_Type_CNAME: {
            l += rr->rdata.CNAME.target.size + 1;
        } break;
        case DNS_Type_AAAA: {
            l += sizeof(rr->rdata.AAAA.addr);
        } break;
        default: {
            DNS_CRASH_THE_PROGRAM_IF_THIS_TYPE_IS_SUPPORTED(rr->type);
        } break;
    }

    return l;
}

internal u64 dns_rr_wire_length(DNS_RR *rr)
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

internal u64 dns_msg_wire_length(DNS_Msg *msg)
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

internal bool32 dns_is_blocked_on_this_network(DNS_TransportProtocol protocol)
{
    bool32 result = true;
    Temp scratch = scratch_begin(0, 0);

    DNS_Client udp_client = dns_client_alloc(scratch.arena, NET_AddressFamily_IPv4, DNS_TransportProtocol_UDP);
    DNS_Client tcp_client = dns_client_alloc(scratch.arena, NET_AddressFamily_IPv4, DNS_TransportProtocol_TCP);
    for (u64 i = 0; i < DNS_RootServer_COUNT; i++) {
        DNS_Msg msg = dns_msg_alloc(scratch.arena, str8_lit("www.example.org"), DNS_Type_A);
        NET_Address address;
        (void)net_str8_to_address(&address, str8_cat(scratch.arena, dns_dname_of_root_server(i), str8_lit(":53")));
        bool32 ok = dns_pack_msg(udp_client.dialer.send_buffer, &msg);
        ok &= dns_pack_msg(tcp_client.dialer.send_buffer, &msg);
    }

    scratch_end(scratch);
    return result;
}
