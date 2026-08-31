
//////////////////
// Wire Packing

internal bool32 dns_pack_rdata(Ring *ring, DNS_RR *rr)
{
    bool32 result = true;
    Temp scratch = scratch_begin(0, 0);
    switch (rr->type) {
        case DNS_Type_A: {
            u32 addr = host_to_net_u32(rr->rdata.A.addr);
            result &= ring_try_write_struct(ring, &addr);
        } break;
        case DNS_Type_NS: {
            String8 ns = str8_to_name_labels(scratch.arena, rr->rdata.NS.ns);
            result &= ring_try_write(ring, ns.size, ns.str);
        } break;
        case DNS_Type_CNAME: {
            String8 cname = str8_to_name_labels(scratch.arena, rr->rdata.CNAME.target);
            result &= ring_try_write(ring, cname.size, cname.str);
        } break;
        case DNS_Type_SOA: {
            String8 mname = str8_to_name_labels(scratch.arena, rr->rdata.SOA.master_name);
            String8 rname = str8_to_name_labels(scratch.arena, rr->rdata.SOA.responsible_name);
            u32 serial  = host_to_net_u32(rr->rdata.SOA.serial);
            u32 refresh = host_to_net_u32(rr->rdata.SOA.refresh);
            u32 retry   = host_to_net_u32(rr->rdata.SOA.retry);
            u32 expire  = host_to_net_u32(rr->rdata.SOA.expire);
            u32 minimum = host_to_net_u32(rr->rdata.SOA.minimum);
            result &= ring_try_write(ring, mname.size, mname.str);
            result &= ring_try_write(ring, rname.size, rname.str);
            result &= ring_try_write_struct(ring, &serial);
            result &= ring_try_write_struct(ring, &refresh);
            result &= ring_try_write_struct(ring, &retry);
            result &= ring_try_write_struct(ring, &expire);
            result &= ring_try_write_struct(ring, &minimum);
        } break;
        case DNS_Type_AAAA: {
            u128 addr = host_to_net_u128(rr->rdata.AAAA.addr);
            result &= ring_try_write_struct(ring, &addr);
        } break;
        default: {
            result = false;
            DNS_CRASH_THE_PROGRAM_IF_THIS_TYPE_IS_SUPPORTED(rr->type);
        } break;
    }

    scratch_end(scratch);
    return result;
}

internal bool32 dns_pack_question(Ring *ring, DNS_RR *rr)
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

    bool32 result = true;
    Temp scratch = scratch_begin(0, 0);

    String8 name = str8_to_name_labels(scratch.arena, rr->name);
    u16 qtype = host_to_net_u16(rr->type);
    u16 qclass = host_to_net_u16(rr->class);
    
    result &= ring_try_write(ring, name.size, name.str);
    result &= ring_try_write_struct(ring, &qtype);
    result &= ring_try_write_struct(ring, &qclass);

    scratch_end(scratch);
    return result;
}

internal bool32 dns_pack_rr(Ring *ring, DNS_RR *rr)
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

    bool32 result = true;
    Temp scratch = scratch_begin(0, 0);

    String8 name = str8_to_name_labels(scratch.arena, rr->name);
    u16 type =  host_to_net_u16(rr->type);
    u16 class = host_to_net_u16(rr->class);
    u32 ttl =   host_to_net_u32(rr->ttl);
    u16 rdlength = 0; // dummy rdlength;

    result &= ring_try_write(ring, name.size, name.str);
    result &= ring_try_write_struct(ring, &type);
    result &= ring_try_write_struct(ring, &class);
    result &= ring_try_write_struct(ring, &ttl);
    
    u64 rdlength_offset = ring->write_pos; // save offset to write to rdlength later
    result &= ring_try_write_struct(ring, &rdlength);

    result &= dns_pack_rdata(ring, rr);
    rdlength = ring->write_pos - rdlength_offset - sizeof(rdlength);
    if (rdlength <= DNS_MAX_MSG_SIZE) { // overflow
        rdlength = host_to_net_u16(rdlength);
        MemoryCopy(&ring->base[rdlength_offset % ring->size], &rdlength, sizeof(rdlength));
    }
    else {
        // @TODO: Handle inconsistent rdata length...
    }

    scratch_end(scratch);
    return result;
}

internal bool32 dns_pack_msg(Ring *ring, DNS_Msg *msg)
{
    /*

                                    1  1  1  1  1  1
      0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                      ID                       |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |QR|   Opcode  |AA|TC|RD|RA| Z|AD|CD|   RCODE   |
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

    bool32 result = true;
    
    u64 l = dns_msg_wire_length(msg);

    u16 id = host_to_net_u16(msg->header.id);
    result &= ring_try_write_struct(ring, &id);

    u16 bits = (u16)msg->header.opcode << 11 | (u16)msg->header.rcode & 0xF;
    if (msg->header.query_response)      bits |= _QR;
    if (msg->header.authoritative)       bits |= _AA;
    if (msg->header.truncated)           bits |= _TC;
    if (msg->header.recursion_desired)   bits |= _RD;
    if (msg->header.recursion_available) bits |= _RA;
    if (msg->header.zero)                bits |= _Z;
    if (msg->header.authenticated_data)  bits |= _AD;
    if (msg->header.checking_disabled)   bits |= _CD;

    bits = host_to_net_u16(bits);
    result &= ring_try_write_struct(ring, &bits);

    u16 qdcount = host_to_net_u16(msg->header.question_count);; // this SHOULDDDD be 1 according to rfc 9619
    u16 ancount = host_to_net_u16(msg->header.answer_count);
    u16 nscount = host_to_net_u16(msg->header.nameserver_count);
    u16 arcount = host_to_net_u16(msg->header.additional_count);
    result &= ring_try_write_struct(ring, &qdcount);
    result &= ring_try_write_struct(ring, &ancount);
    result &= ring_try_write_struct(ring, &nscount);
    result &= ring_try_write_struct(ring, &arcount);

    // @TODO: Message compression

    // @TODO: Error checking for the below block
    for (u64 i = 0; i < msg->header.question_count; i++) {
        result &= dns_pack_question(ring, &msg->question[i]);
    }
    for (u64 i = 0; i < msg->header.answer_count; i++) {
        result &= dns_pack_rr(ring, &msg->answer[i]);
    }
    for (u64 i = 0; i < msg->header.nameserver_count; i++) {
        result &= dns_pack_rr(ring, &msg->ns[i]);
    }
    for (u64 i = 0; i < msg->header.additional_count; i++) {
        result &= dns_pack_rr(ring, &msg->extra[i]);
    }

    // @TODO: OPT RR

    return result;
}

internal bool32 dns_unpack_labels(Arena *arena, Ring *ring, String8 *out)
{
    bool32 result = true;
    Temp scratch = scratch_begin(&arena, 1);

    String8 domain_name = str8_zero();
    u32 total_length = 0;

    u64 cursor = ring->read_pos;
    u64 end = 0;
    
    bool32 jumped = false;
    u32 jumps = 0;
    while (true) {
        bool32 out_of_bounds = false;
        if (cursor + 1 > ring->write_pos) out_of_bounds = true;
        if (out_of_bounds) {
            result = false;
            break;
        }
        u8 length = ring->base[cursor % ring->size];

        if ((length & 0xC0) == 0xC0) {
            bool32 bad_pointer = false;
            if (++jumps > DNS_MAX_COMPRESSION_JUMPS) bad_pointer = true;
            if (cursor + 2 > ring->write_pos)        bad_pointer = true;
            if (bad_pointer) {
                result = false;
                break;
            }

            u8 lo = ring->base[(cursor + 1) % ring->size];
            if (!jumped) {
                end = cursor + 2;
                jumped = true;
            }

            u16 offset = ((u16)(length & 0x3F) << 8) | lo;
            bool32 bad_offset = false;
            if (offset >= cursor) bad_offset = true;
            if (bad_offset) {
                result = false;
                break;
            }
            cursor = offset;
            continue;
        }

        if (length == 0) {
            // terminating byte
            cursor += 1;
            if (!jumped) {
                end = cursor;
            }
            break;
        }

        bool32 bad_name = false;
        if (length > DNS_MAX_LABEL_LEN)               bad_name = true;
        if (total_length + length > DNS_MAX_NAME_LEN) bad_name = true;
        if (cursor + 1 + length > ring->write_pos)    bad_name = true;
        if (bad_name) {
            result = false;
            break;
        }

        String8 label = str8_zero();
        label.size = length;
        label.str = push_array(scratch.arena, u8, label.size);
        for (u64 k = 0; k < label.size; k++) {
            label.str[k] = ring->base[(cursor + 1 + k) % ring->size];
        }

        total_length += label.size;
        domain_name = str8_cat(scratch.arena, domain_name, label);
        domain_name = str8_cat(scratch.arena, domain_name, s("."));
        cursor += label.size + 1;
    }

    if (result) {
        ring->read_pos = end;
        *out = str8_copy(arena, domain_name);
    }
    
    scratch_end(scratch);
    return result;
}

internal bool32 dns_unpack_rdata(Arena *arena, Ring *ring, DNS_RR *rr, u16 rdlength)
{
    bool32 result = true;
    Temp scratch = scratch_begin(&arena, 1);

    switch(rr->type) {
        case DNS_Type_A: {
            u32 addr;
            result &= ring_try_read_struct(ring, &addr);
            rr->rdata.A.addr = net_to_host_u32(addr);
        } break;
        case DNS_Type_NS: {
            result &= dns_unpack_labels(arena, ring, &rr->rdata.NS.ns);
        } break;
        case DNS_Type_CNAME: {
            result &= dns_unpack_labels(arena, ring, &rr->rdata.CNAME.target);
        } break;
        case DNS_Type_SOA: {
            result &= dns_unpack_labels(arena, ring, &rr->rdata.SOA.master_name);
            result &= dns_unpack_labels(arena, ring, &rr->rdata.SOA.responsible_name);
            u32 serial, refresh, retry, expire, minimum;
            result &= ring_try_read_struct(ring, &serial);
            result &= ring_try_read_struct(ring, &refresh);
            result &= ring_try_read_struct(ring, &retry);
            result &= ring_try_read_struct(ring, &expire);
            result &= ring_try_read_struct(ring, &minimum);
            rr->rdata.SOA.serial  = net_to_host_u32(serial);
            rr->rdata.SOA.refresh = net_to_host_u32(refresh);
            rr->rdata.SOA.retry   = net_to_host_u32(retry);
            rr->rdata.SOA.expire  = net_to_host_u32(expire);
            rr->rdata.SOA.minimum = net_to_host_u32(minimum);
        } break;
        case DNS_Type_AAAA: {
            u128 addr;
            result &= ring_try_read_struct(ring, &addr);
            rr->rdata.AAAA.addr = net_to_host_u128(addr);
        } break;
        default: {
            DNS_CRASH_THE_PROGRAM_IF_THIS_TYPE_IS_SUPPORTED(rr->type);
            u8 *discard = push_array(scratch.arena, u8, rdlength);
            result &= ring_try_read(ring, rdlength, discard);
        } break;
    }

    scratch_end(scratch);
    return result;
}


internal bool32 dns_unpack_question(Arena *arena, Ring *ring, DNS_RR *rr)
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

    bool32 result = true;

    u16 qtype, qclass;
    result &= dns_unpack_labels(arena, ring, &rr->name);
    result &= ring_try_read_struct(ring, &qtype);
    result &= ring_try_read_struct(ring, &qclass);
    rr->type = net_to_host_u16(qtype);
    rr->class = net_to_host_u16(qclass);

    return result;
}

internal bool32 dns_unpack_rr(Arena *arena, Ring *ring, DNS_RR *rr)
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

    bool32 result = true;

    u16 type, class, rdlength;
    u32 ttl;
    result &= dns_unpack_labels(arena, ring, &rr->name);
    result &= ring_try_read_struct(ring, &type);
    result &= ring_try_read_struct(ring, &class);
    result &= ring_try_read_struct(ring, &ttl);
    result &= ring_try_read_struct(ring, &rdlength);
    rr->type     = net_to_host_u16(type);
    rr->class    = net_to_host_u16(class);
    rr->ttl      = net_to_host_u32(ttl);
    rdlength = net_to_host_u16(rdlength);

    u64 rdata_start = ring->read_pos;
    result &= dns_unpack_rdata(arena, ring, rr, rdlength);
    u64 consumed = ring->read_pos - rdata_start;
    if (consumed != rdlength) {
        result = false;
    }
    
    return result;
}

internal bool32 dns_unpack_msg(Arena *arena, Ring *ring, DNS_Msg *msg)
{
    /*

                                    1  1  1  1  1  1
      0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                      ID                       |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |QR|   Opcode  |AA|TC|RD|RA| Z|AD|CD|   RCODE   |
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

    bool32 result = true;
    Temp scratch = scratch_begin(&arena, 1);

    // It may be faster to instead pass around the base octet of the recv_buffer
    // to use for name pointer offsets, instead of making a ring buffer for the message.
    u64 msg_size = ring_peek_unread_quantity(ring);
    u8 *msg_bytes = push_array(scratch.arena, u8, msg_size);
    result &= ring_try_read(ring, msg_size, msg_bytes);
    if (result) {
        Ring msg_ring = {0};
        msg_ring.base = msg_bytes;
        msg_ring.size = msg_size;
        msg_ring.write_pos = msg_size;
        msg_ring.read_pos = 0;
        
        u16 id;
        result &= ring_try_read_struct(&msg_ring, &id);
        msg->header.id = net_to_host_u16(id);

        u16 bits;
        result &= ring_try_read_struct(&msg_ring, &bits);
        bits = net_to_host_u16(bits);

        msg->header.opcode = (bits >> 11) & 0xF;
        msg->header.rcode = (bits & 0xF);
        if (bits & _QR) msg->header.query_response      = true;
        if (bits & _AA) msg->header.authoritative       = true;
        if (bits & _TC) msg->header.truncated           = true;
        if (bits & _RD) msg->header.recursion_desired   = true;
        if (bits & _RA) msg->header.recursion_available = true;
        if (bits & _Z)  msg->header.zero                = true;
        if (bits & _AD) msg->header.authenticated_data  = true;
        if (bits & _CD) msg->header.checking_disabled   = true;

        u16 qdcount, ancount, nscount, arcount;
        result &= ring_try_read_struct(&msg_ring, &qdcount);
        result &= ring_try_read_struct(&msg_ring, &ancount);
        result &= ring_try_read_struct(&msg_ring, &nscount);
        result &= ring_try_read_struct(&msg_ring, &arcount);
        msg->header.question_count   = net_to_host_u16(qdcount);
        msg->header.answer_count     = net_to_host_u16(ancount);
        msg->header.nameserver_count = net_to_host_u16(nscount);
        msg->header.additional_count = net_to_host_u16(arcount);

        msg->question = push_array(arena, DNS_RR, msg->header.question_count);
        msg->answer   = push_array(arena, DNS_RR, msg->header.answer_count);
        msg->ns       = push_array(arena, DNS_RR, msg->header.nameserver_count);
        msg->extra    = push_array(arena, DNS_RR, msg->header.additional_count);

        u64 i = 0;
        for (i = 0; i < msg->header.question_count; i++) {
            result &= dns_unpack_question(arena, &msg_ring, &msg->question[i]);
        }
        for (i = 0; i < msg->header.answer_count; i++) {
            result &= dns_unpack_rr(arena, &msg_ring, &msg->answer[i]);
        }
        for (i = 0; i < msg->header.nameserver_count; i++) {
            result &= dns_unpack_rr(arena, &msg_ring, &msg->ns[i]);
        }
        for (i = 0; i < msg->header.additional_count; i++) {
            result &= dns_unpack_rr(arena, &msg_ring, &msg->extra[i]);
        }
    }

    scratch_end(scratch);
    return result;
}
