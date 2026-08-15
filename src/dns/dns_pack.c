
//////////////////
// Wire Packing

internal u64 dns_pack_u8(u8 i, u8 *wire, u64 off)
{
    MemoryCopy(wire + off, &i, sizeof(i));
    return off + sizeof(i);
}

internal u64 dns_pack_u16(u16 i, u8 *wire, u64 off)
{
    i = host_to_net_u16(i);
    MemoryCopy(wire + off, &i, sizeof(i));
    return off + sizeof(i);
}

internal u64 dns_pack_u32(u32 i, u8 *wire, u64 off)
{
    i = host_to_net_u32(i);
    MemoryCopy(wire + off, &i, sizeof(i));
    return off + sizeof(i);
}

internal u64 dns_pack_u64(u64 i, u8 *wire, u64 off)
{
    i = host_to_net_u64(i);
    MemoryCopy(wire + off, &i, sizeof(i));
    return off + sizeof(i);
}

internal u64 dns_pack_u128(u128 i, u8 *wire, u64 off)
{
    i = host_to_net_u128(i);
    MemoryCopy(wire + off, &i, sizeof(i));
    return off + sizeof(i);
}

internal u64 dns_pack_str8(String8 s, u8 *wire, u64 off)
{
    MemoryCopy(wire + off, s.str, s.size);
    return off + s.size;
}

internal u64 dns_pack_rdata(Dns_RR rr, u8 *wire, u64 off)
{
    Temp scratch = scratch_begin(0, 0);
    switch (rr.type) {
        case Dns_Type_A: {
            off = dns_pack_u32(rr.rdata.A.addr, wire, off);
        } break;
        case Dns_Type_NS: {
            String8 ns = str8_to_domain_name(scratch.arena, rr.rdata.NS.ns);
            off = dns_pack_str8(ns, wire, off);
        } break;
        case Dns_Type_CNAME: {
            String8 cname = str8_to_domain_name(scratch.arena, rr.rdata.CNAME.target);
            off = dns_pack_str8(cname, wire, off);
        } break;
        case Dns_Type_AAAA: {
            off = dns_pack_u128(rr.rdata.AAAA.addr, wire, off);
        } break;
        default: {
            DNS_CRASH_THE_PROGRAM_IF_THIS_TYPE_IS_SUPPORTED(rr.type);
        } break;
    }

    scratch_end(scratch);
    return off;
}

internal u64 dns_pack_question(Dns_RR rr, u8* wire, u64 off)
{
    /*
                                    1  1  1  1  1  1
      0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                                               |
    /                     QNAME                     /
    /                                               /
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                     QTYPE                     |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                     QCLASS                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    
    */

    Temp scratch = scratch_begin(0, 0);

    String8 name = str8_to_domain_name(scratch.arena, rr.name);
    u16 qtype = (u16)rr.type;
    u16 qclass = (u16)rr.class;

    off = dns_pack_str8(name, wire, off);
    off = dns_pack_u16(qtype, wire, off);
    off = dns_pack_u16(qclass, wire, off);

    scratch_end(scratch);
    return off;
}

internal u64 dns_pack_rr(Dns_RR rr, u8 *wire, u64 off)
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

    Temp scratch = scratch_begin(0, 0);

    String8 name = str8_to_domain_name(scratch.arena, rr.name);
    u16 type = (u16)rr.type;
    u16 class = (u16)rr.class;
    u32 ttl = rr.ttl;

    u64 header_end = dns_pack_str8(name, wire, off);
    header_end = dns_pack_u16(type, wire, header_end);
    header_end = dns_pack_u16(class, wire, header_end);
    header_end = dns_pack_u32(ttl, wire, header_end);
    header_end += 2; // rdlength written to in a second...

    off = dns_pack_rdata(rr, wire, header_end);
    u16 rdlength = off - header_end;
    if (rdlength <= DNS_MAX_MSG_SIZE) { // overflow
        (void)dns_pack_u16(rdlength, wire, header_end - 2);
    }
    else {
        // @TODO: Handle inconsistent rdata length...
        // do something with `off`
    }

    scratch_end(scratch);
    return off;
}

internal u64 dns_pack_msg(Dns_Msg *msg)
{
    /*

                                    1  1  1  1  1  1
      0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                      ID                       |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |QR|   Opcode  |AA|TC|RD|RA|   Z    |   RCODE   |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    QDCOUNT                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    ANCOUNT                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    NSCOUNT                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    ARCOUNT                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+

     */
    
    u64 l = dns_msg_wire_length(msg);
    msg->wire = arena_push(msg->wire_arena, l, 1, 0);

    u16 id = host_to_net_u16(msg->header.id);
    u64 off = dns_pack_u16(msg->header.id, msg->wire, 0);

    u16 bits = (u16)msg->header.opcode << 11 | (u16)msg->header.rcode & 0xF;
    if (msg->header.query_response)      bits |= _QR;
    if (msg->header.authoritative)       bits |= _AA;
    if (msg->header.truncated)           bits |= _TC;
    if (msg->header.recursion_desired)   bits |= _RD;
    if (msg->header.recursion_available) bits |= _RA;
    if (msg->header.zero)                bits |= _Z;
    if (msg->header.authenticated_data)  bits |= _AD;
    if (msg->header.checking_disabled)   bits |= _CD;
    
    off = dns_pack_u16(bits, msg->wire, off);

    off = dns_pack_u16((u16)1,                       msg->wire, off);
    off = dns_pack_u16(msg->header.answer_count,     msg->wire, off);
    off = dns_pack_u16(msg->header.nameserver_count, msg->wire, off);
    off = dns_pack_u16(msg->header.additional_count, msg->wire, off);

    // @TODO: Message compression

    // @TODO: Error checking for the below block
    off = dns_pack_question(msg->question, msg->wire, off);
    
    for (u64 i = 0; i < msg->header.answer_count; i++) {
        off = dns_pack_rr(msg->answer[i], msg->wire, off);
    }
    for (u64 i = 0; i < msg->header.nameserver_count; i++) {
        off = dns_pack_rr(msg->ns[i], msg->wire, off);
    }
    for (u64 i = 0; i < msg->header.additional_count; i++) {
        off = dns_pack_rr(msg->extra[i], msg->wire, off);
    }

    // @TODO: OPT RR

    return off;
}
