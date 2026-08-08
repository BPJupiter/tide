

////////////////////
// Util Functions

internal String8 str8_to_fqdn(Arena *arena, String8 s)
{
    if (!str8_is_fqdn(s)) {
        s = str8_cat(arena, s, str8_lit("."));
    }
    return s;
}

internal bool32 str8_is_fqdn(String8 s)
{
    return str8_ends_with(s, '.', 0);
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
