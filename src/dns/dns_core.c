

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

Dns_Msg dns_msg_alloc(Arena *arena, String8 domain, Dns_Type type)
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
