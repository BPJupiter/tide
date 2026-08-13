

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
    return 0xCAFE;
}

///////////////////////////
// DNS Message Functions

Dns_Msg *dns_msg_alloc(Arena *arena, String8 domain, Dns_Type type)
{
    Dns_Msg *msg = push_array(arena, Dns_Msg, 1);
    msg->header.id = dns_id_func();
    msg->header.recursion_desired = true;
    msg->question.name = str8_to_fqdn(arena, domain);
    msg->question.class = Dns_Class_INET;
    msg->question.type = type;
    return msg;
}


////////////////////
// Utility Functions

internal String8 str8_to_fqdn(Arena *arena, String8 s)
{
    if (!str8_is_fqdn(s)) {
        s = str8_cat(arena, s, str8_lit("."));
    }
    return s;
}

internal bool32 str8_is_fqdn(String8 s)
{
    return str8_ends_with(s, str8_lit("."), 0);
}

internal String8 str8_to_canonical(Arena *arena, String8 s)
{
    s = lower_from_str8(arena, s);
    s = str8_to_fqdn(arena, s);
    return s;
}

internal bool32 str8_is_domain_name(String8 s)
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

/////////////////////
// Wire <-> Struct

internal u64 dns_pack_rr(Dns_RR rr, u8 *wire)
{
    u64 offset = 0;

    return offset;
}

internal u8 *dns_rr_to_bytes(Arena *arena, Dns_RR rr)
{
    u8 *result = push_array(arena, u8, 
    return 0;
}

internal u64 dns_bytes_to_rr(Dns_RR *rr, u8 *bytes)
{
    return 0;
}
