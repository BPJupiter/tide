// Copyright (c) Frances Telfar
// Licensed under the PolyForm Noncommercial License (https://polyformproject.org/licenses/noncommercial/1.0.0)

///////////////////
// Generated Code

#include "generated/dns.meta.c"

///////////////////////
// Bit Packing Flags

#define _QR (1 << 15)
#define _AA (1 << 10)
#define _TC (1 << 9)
#define _RD (1 << 8)
#define _RA (1 << 7)
#define _Z  (1 << 6)
#define _AD (1 << 5)
#define _CD (1 << 4)

// ENDS0 OPT
#define _DO (1 << 15)
#define _CO (1 << 14)
#define _DE (1 << 1)3

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

internal DNS_Msg dns_msg_make(Arena *arena, String8 domain, DNS_Type type)
{
    DNS_Msg msg = {0};
    msg.header.id = dns_id_func();
    msg.header.recursion_desired = true;
    msg.header.question_count = 1;
    msg.question = push_array(arena, DNS_RR, 1);
    msg.question[0].name = dns_fqdn_from_string(arena, domain);
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

//////////////////
// Wire Lengths

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
        case DNS_Type_PTR: {
            l += rr->rdata.PTR.ptrdname.size + 1;
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

//////////////////////
// Wire Packing/Unpacking

internal String8 dns_pack_msg(Arena *arena, DNS_Msg msg, bool32 pack_tcp_length)
{
    Temp scratch = scratch_begin(&arena, 1);
    String8_List serial = {0};
    str8_serial_begin(scratch.arena, &serial);
    {
        if(pack_tcp_length)
        {
            u64 l = dns_msg_wire_length(&msg);
            l = net_hton_u16(l);
            str8_serial_push_u16(scratch.arena, &serial, l);
        }
        
        // pack header
        {
            u16 id = net_hton_u16(msg.header.id);
            str8_serial_push_u16(scratch.arena, &serial, id);
            
            u16 bits = (u16)msg.header.opcode << 11 | ((u16)msg.header.rcode & 0xF);
            if (msg.header.query_response)      bits |= _QR;
            if (msg.header.authoritative)       bits |= _AA;
            if (msg.header.truncated)           bits |= _TC;
            if (msg.header.recursion_desired)   bits |= _RD;
            if (msg.header.recursion_available) bits |= _RA;
            if (msg.header.zero)                bits |= _Z;
            if (msg.header.authenticated_data)  bits |= _AD;
            if (msg.header.checking_disabled)   bits |= _CD;
        
            bits = net_hton_u16(bits);
            str8_serial_push_u16(scratch.arena, &serial, bits);

            // NOTE: rfc 9619 states that qdcount should always be 1.
            u16 qdcount = net_hton_u16(msg.header.question_count);
            u16 ancount = net_hton_u16(msg.header.answer_count);
            u16 nscount = net_hton_u16(msg.header.nameserver_count);
            u16 arcount = net_hton_u16(msg.header.additional_count);
            str8_serial_push_u16(scratch.arena, &serial, qdcount);
            str8_serial_push_u16(scratch.arena, &serial, ancount);
            str8_serial_push_u16(scratch.arena, &serial, nscount);
            str8_serial_push_u16(scratch.arena, &serial, arcount);
        }

        // @TODO: Message compression
        
        // @TODO: Error checking for the below block
        
        // pack questions
        for (u64 i = 0; i < msg.header.question_count; i++)
        {
            String8 name = dns_name_labels_from_string(scratch.arena, msg.question[i].name);
            u16 qtype    = net_hton_u16(msg.question[i].type);
            u16 qclass   = net_hton_u16(msg.question[i].class);
            str8_serial_push_string(scratch.arena, &serial, name);
            str8_serial_push_u16(scratch.arena, &serial, qtype);
            str8_serial_push_u16(scratch.arena, &serial, qclass);
        }

        // pack RRs from the answer, nameserver, and additional sections
        struct
        {
            DNS_RR *rr;
            u64 count;
        } rrs[] =
        {
            {msg.answer, msg.header.answer_count},
            {msg.ns,     msg.header.nameserver_count},
            {msg.extra,  msg.header.additional_count},
        };
        for(u64 section = 0; section < ArrayCount(rrs); section++)
        {
            for(u64 i = 0; i < rrs[section].count; i++)
            {
                DNS_RR rr = rrs[section].rr[i];
                // pack the RR header
                String8 name = dns_name_labels_from_string(scratch.arena, rr.name);
                u16 type =  net_hton_u16(rr.type);
                u16 class = net_hton_u16(rr.class);
                u32 ttl =   net_hton_u32(rr.ttl);
                str8_serial_push_string(scratch.arena, &serial, name);
                str8_serial_push_u16(scratch.arena, &serial, type);
                str8_serial_push_u16(scratch.arena, &serial, class);
                str8_serial_push_u32(scratch.arena, &serial, ttl);
                
                // the rdlength is written to later with the length of the rdata that get serialized.
                u16 *rdlength_ptr = str8_serial_push_u16(scratch.arena, &serial, 0);
                
                // pack rdata
                switch (rr.type)
                {
                    case DNS_Type_A:
                    {
                        u32 addr = net_hton_u32(rr.rdata.A.addr);
                        str8_serial_push_u32(scratch.arena, &serial, addr);
                    } break;
                    case DNS_Type_NS:
                    {
                        String8 ns = dns_name_labels_from_string(scratch.arena, rr.rdata.NS.ns);
                        str8_serial_push_string(scratch.arena, &serial, ns);
                    } break;
                    case DNS_Type_CNAME:
                    {
                        String8 cname = dns_name_labels_from_string(scratch.arena, rr.rdata.CNAME.target);
                        str8_serial_push_string(scratch.arena, &serial, cname);
                    } break;
                    case DNS_Type_SOA:
                    {
                        String8 mname = dns_name_labels_from_string(scratch.arena, rr.rdata.SOA.master_name);
                        String8 rname = dns_name_labels_from_string(scratch.arena, rr.rdata.SOA.responsible_name);
                        u32 serial_n  = net_hton_u32(rr.rdata.SOA.serial);
                        u32 refresh   = net_hton_u32(rr.rdata.SOA.refresh);
                        u32 retry     = net_hton_u32(rr.rdata.SOA.retry);
                        u32 expire    = net_hton_u32(rr.rdata.SOA.expire);
                        u32 minimum   = net_hton_u32(rr.rdata.SOA.minimum);
                        str8_serial_push_string(scratch.arena, &serial, mname);
                        str8_serial_push_string(scratch.arena, &serial, rname);
                        str8_serial_push_u32(scratch.arena, &serial, serial_n);
                        str8_serial_push_u32(scratch.arena, &serial, refresh);
                        str8_serial_push_u32(scratch.arena, &serial, retry);
                        str8_serial_push_u32(scratch.arena, &serial, expire);
                        str8_serial_push_u32(scratch.arena, &serial, minimum);
                    } break;
                    case DNS_Type_PTR:
                    {
                        String8 ptrdname = dns_name_labels_from_string(scratch.arena, rr.rdata.PTR.ptrdname);
                        str8_serial_push_string(scratch.arena, &serial, ptrdname);
                    } break;
                    case DNS_Type_AAAA:
                    {
                        u128 addr = net_hton_u128(rr.rdata.AAAA.addr);
                        str8_serial_push_data(scratch.arena, &serial, &addr, sizeof(addr));
                    } break;
                    default:
                    {
                        DNS_CRASH_THE_PROGRAM_IF_THIS_TYPE_IS_SUPPORTED(rr.type);
                    } break;
                }
            
                rdlength_ptr[0] = safe_cast_u16(safe_cast_u32(serial.last->string.size));
                if (rdlength_ptr[0] <= DNS_MAX_MSG_SIZE)
                { // overflow
                    rdlength_ptr[0] = net_hton_u16(rdlength_ptr[0]);
                }
                else
                {
                    // @TODO: Handle inconsistent rdata length...
                }
            }
        }
        // @TODO: OPT RR
    }
    String8 result = str8_serial_end(arena, &serial);
    scratch_end(scratch);

    return result;
}

internal String8 dns_unpack_labels(Arena *arena, String8 wire, u64 *cursor)
{
    // @TODO this function fucking sucks and needs to not suck
    Temp scratch = scratch_begin(&arena, 1);

    bool32 good_domain = 1;
    String8 domain_name = str8_zero();
    u32 total_length = 0;

    u64 pos = *cursor;
    u64 end = 0;
    
    bool32 jumped = 0;
    u32 jumps = 0;
    for(;;)
    {
        if(pos + 1 > wire.size)
        {
            good_domain = 0;
            break;
        }
        
        u8 length = 0;
        str8_deserial_read_struct(wire, pos, &length);
        if ((length & 0xC0) == 0xC0) {
            if(++jumps > DNS_MAX_COMPRESSION_JUMPS || pos + 2 > wire.size)
            {
                good_domain = 0;
                break;
            }

            u8 lo = 0;
            str8_deserial_read_struct(wire, pos+1, &lo);
            if (!jumped) {
                jumped = 1;
                end = pos+2;
            }

            u16 offset = ((u16)(length & 0x3F) << 8) | lo;
            if(offset >= pos)
            {
                good_domain = 0;
                break;
            }
            pos = offset;
            continue;
        }

        if (length == 0) {
            // terminating byte
            pos += 1;
            if(!jumped)
            {
                end = pos;
            }
            break;
        }

        bool32 bad_name = 0;
        if (length > DNS_MAX_LABEL_LEN)               bad_name = 1;
        if (total_length + length > DNS_MAX_NAME_LEN) bad_name = 1;
        if (pos + 1 + length > wire.size)             bad_name = 1;
        if (bad_name) {
            good_domain = 0;
            break;
        }

        String8 label = str8_substr(wire, r1u64(pos+1, pos+1+length));
        total_length += label.size;
        domain_name = str8_cat(scratch.arena, domain_name, label);
        domain_name = str8_cat(scratch.arena, domain_name, s("."));
        pos += label.size + 1;
    }

    String8 result = {0};
    if (good_domain)
    {
        if(domain_name.size == 0)
        {
            domain_name = s(".");
        }
        result = str8_copy(arena, domain_name);
        *cursor = end;
    }
    
    scratch_end(scratch);
    return result;
}

internal DNS_Msg dns_unpack_msg(Arena *arena, String8 wire, bool32 unpack_tcp_length)
{
    Temp scratch = scratch_begin(&arena, 1);
    DNS_Msg msg = {0};
    u64 cursor = 0;

    if(unpack_tcp_length)
    {
        u16 total_length = 0;
        cursor += str8_deserial_read_struct(wire, cursor, &total_length);
        // @TODO verify/do something with this total length. probably.
    }

    u16 id;
    cursor += str8_deserial_read_struct(wire, cursor, &id);
    msg.header.id = net_ntoh_u16(id);
    
    u16 bits;
    cursor += str8_deserial_read_struct(wire, cursor, &bits);
    bits = net_ntoh_u16(bits);
    
    msg.header.opcode = (bits >> 11) & 0xF;
    msg.header.rcode = (bits & 0xF);
    if (bits & _QR) msg.header.query_response      = 1;
    if (bits & _AA) msg.header.authoritative       = 1;
    if (bits & _TC) msg.header.truncated           = 1;
    if (bits & _RD) msg.header.recursion_desired   = 1;
    if (bits & _RA) msg.header.recursion_available = 1;
    if (bits & _Z)  msg.header.zero                = 1;
    if (bits & _AD) msg.header.authenticated_data  = 1;
    if (bits & _CD) msg.header.checking_disabled   = 1;

    u16 qdcount, ancount, nscount, arcount;
    cursor += str8_deserial_read_struct(wire, cursor, &qdcount);
    cursor += str8_deserial_read_struct(wire, cursor, &ancount);
    cursor += str8_deserial_read_struct(wire, cursor, &nscount);
    cursor += str8_deserial_read_struct(wire, cursor, &arcount);
    msg.header.question_count   = net_ntoh_u16(qdcount);
    msg.header.answer_count     = net_ntoh_u16(ancount);
    msg.header.nameserver_count = net_ntoh_u16(nscount);
    msg.header.additional_count = net_ntoh_u16(arcount);

    msg.question = push_array(arena, DNS_RR, msg.header.question_count);
    msg.answer   = push_array(arena, DNS_RR, msg.header.answer_count);
    msg.ns       = push_array(arena, DNS_RR, msg.header.nameserver_count);
    msg.extra    = push_array(arena, DNS_RR, msg.header.additional_count);

    // unpack question
    for (u64 i = 0; i < msg.header.question_count; i++) {
        u16 qtype, qclass;
        msg.question[i].name = dns_unpack_labels(arena, wire, &cursor);
        cursor += str8_deserial_read_struct(wire, cursor, &qtype);
        cursor += str8_deserial_read_struct(wire, cursor, &qclass);
        msg.question[i].type = net_ntoh_u16(qtype);
        msg.question[i].class = net_ntoh_u16(qclass);
    }

    // unpack RRs from the answer, nameserver, and additional sections
    struct
    {
        DNS_RR *rr;
        u64 count;
    } rrs[] =
    {
        {msg.answer, msg.header.answer_count},
        {msg.ns,     msg.header.nameserver_count},
        {msg.extra,  msg.header.additional_count},
    };
    for(u64 section = 0; section < ArrayCount(rrs); section++)
    {
        for(u64 i = 0; i < rrs[section].count; i++)
        {
            DNS_RR *rr = &rrs[section].rr[i];
            
            u16 type, class, rdlength;
            u32 ttl;
            rr->name = dns_unpack_labels(arena, wire, &cursor);
            cursor += str8_deserial_read_struct(wire, cursor, &type);
            cursor += str8_deserial_read_struct(wire, cursor, &class);
            cursor += str8_deserial_read_struct(wire, cursor, &ttl);
            cursor += str8_deserial_read_struct(wire, cursor, &rdlength);
            rr->type     = net_ntoh_u16(type);
            rr->class    = net_ntoh_u16(class);
            rr->ttl      = net_ntoh_u32(ttl);
            rdlength = net_ntoh_u16(rdlength);

            u64 rdata_start = cursor;
            // unpack rdata
            switch(rr->type)
            {
                case DNS_Type_A:
                {
                    u32 addr;
                    cursor += str8_deserial_read_struct(wire, cursor, &addr);
                    rr->rdata.A.addr = net_ntoh_u32(addr);
                } break;
                case DNS_Type_NS:
                {
                    rr->rdata.NS.ns                = dns_unpack_labels(arena, wire, &cursor);
                } break;
                case DNS_Type_CNAME:
                {
                    rr->rdata.CNAME.target         = dns_unpack_labels(arena, wire, &cursor);
                } break;
                case DNS_Type_SOA:
                {
                    rr->rdata.SOA.master_name      = dns_unpack_labels(arena, wire, &cursor);
                    rr->rdata.SOA.responsible_name = dns_unpack_labels(arena, wire, &cursor);
                    u32 serial_n, refresh, retry, expire, minimum;
                    cursor += str8_deserial_read_struct(wire, cursor, &serial_n);
                    cursor += str8_deserial_read_struct(wire, cursor, &refresh);
                    cursor += str8_deserial_read_struct(wire, cursor, &retry);
                    cursor += str8_deserial_read_struct(wire, cursor, &expire);
                    cursor += str8_deserial_read_struct(wire, cursor, &minimum);
                    rr->rdata.SOA.serial  = net_ntoh_u32(serial_n);
                    rr->rdata.SOA.refresh = net_ntoh_u32(refresh);
                    rr->rdata.SOA.retry   = net_ntoh_u32(retry);
                    rr->rdata.SOA.expire  = net_ntoh_u32(expire);
                    rr->rdata.SOA.minimum = net_ntoh_u32(minimum);
                } break;
                case DNS_Type_PTR:
                {
                    rr->rdata.PTR.ptrdname = dns_unpack_labels(arena, wire, &cursor);
                } break;
                case DNS_Type_AAAA:
                {
                   u128 addr;
                   cursor += str8_deserial_read_struct(wire, cursor, &addr);
                   rr->rdata.AAAA.addr = net_ntoh_u128(addr);
                } break;
                default:
                {
                    DNS_CRASH_THE_PROGRAM_IF_THIS_TYPE_IS_SUPPORTED(rr->type);
                    cursor += rdlength;
                } break;
            }

            u64 consumed = cursor - rdata_start;
            if (consumed != rdlength) {
                // @TODO something! this is quite bad and shouldn't happen!
            }
        }
    }

    scratch_end(scratch);
    return msg;
}

////////////////////
// Utility Functions

internal String8 dns_fqdn_from_string(Arena *arena, String8 s)
{
    if (!dns_string_is_fqdn(s)) {
        s = str8_cat(arena, s, str8_lit("."));
    }
    return str8_copy(arena, s);
}

internal bool32 dns_string_is_fqdn(String8 s)
{
    return str8_ends_with(s, str8_lit("."), 0);
}

internal String8 dns_canonical_from_string(Arena *arena, String8 s)
{
    Temp scratch = scratch_begin(&arena, 1);
    s = lower_from_str8(scratch.arena, s);
    String8 result = dns_fqdn_from_string(arena, s);
    scratch_end(scratch);
    return result;
}

internal String8 dns_name_labels_from_string(Arena *arena, String8 s)
{
    Temp scratch = scratch_begin(&arena, 1);

    s = dns_canonical_from_string(scratch.arena, s);

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

internal bool32 dns_string_is_name_labels(String8 s)
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

internal String8 dns_inverse_query_name_from_address(Arena *arena, NET_Endpoint endpoint)
{
    Temp scratch = scratch_begin(&arena, 1);
    String8 result = {0};
    
    switch (endpoint.kind)
    {
        default:{}break;
        case NET_EndpointKind_IPv4:
        {
            String8 ip_str = net_string_from_endpoint(scratch.arena, endpoint);
            result = str8_cat(arena, ip_str, s(".in-addr.arpa"));
        } break;
        case NET_EndpointKind_IPv6:
        {
        } break;
    }
    
    scratch_end(scratch);
    return result;
}

internal bool32 dns_is_blocked_on_this_network(void)
{
    // @TODO
    return 0;
}
