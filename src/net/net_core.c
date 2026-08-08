
internal bool32 net_str8_to_ipv4(String8 string, u32 *out)
{
    bool32 result = false;
    u32 address = 0;
    Temp scratch = scratch_begin(0, 0);

    String8_List parts = str8_split(scratch.arena, string, (u8 *)".", 1, StringSplitFlag_KeepEmpties);
    if (parts.node_count == 4) {
        result = true;
        u32 i = 3;
        for (String8_Node *n = parts.first; n != 0; n = n->next) {
            String8 part = n->string;
            if (part.size == 0 || part.size > 3 || !str8_is_integer(part, 10)) {
                result = false;
                break;
            }

            if (part.size > 1 && part.str[0] == '0') {
                result = false;
                break;
            }

            u64 byte = u64_from_str8(part, 10);
            if (byte > max_u8) {
                result = false;
                break;
            }
            address |= (byte << (8 * i--));
        }
    }

    if (result && out != 0) {
        *out = address;
    }
    
    scratch_end(scratch);
    return result;
}


internal bool32 net__ipv6_group_is_valid(String8 group)
{
    return (group.size >= 1 && group.size <= 4 && str8_is_integer(group, 16));
}

internal bool32 net_str8_to_ipv6(String8 string, u128 *out)
{
    bool32 ok = true;
    u128 address = {0};
    
    Temp scratch = scratch_begin(0, 0);

    bool32 good_length = true;
    if (string.size < 1)  good_length = false;
    if (string.size > 45) good_length = false;

    ok = good_length;
    if (good_length) {
        bool32 good_colons = true;
        if (string.str[0] == ':' && string.str[1] != ':') good_colons = false;
        if (string.str[string.size - 1] == ':' && string.str[string.size - 2] != ':') good_colons = false;
        if (str8_find_needle(string, 0, str8_lit(":::"), 0) < string.size) good_colons = false;
        
        // Locate "::"; reject if it appears more than once.
        u64 dc_pos = str8_find_needle(string, 0, str8_lit("::"), 0);
        bool32 has_dc = (dc_pos < string.size);
    
        if (has_dc) {
            u64 dc_pos2 = str8_find_needle(string, dc_pos + 2, str8_lit("::"), 0);
            if (dc_pos2 < string.size) good_colons = false;
        }

        ok = good_colons;
        if (good_colons) {
            String8 left = has_dc ? str8_prefix(string, dc_pos) : string;
            String8 right = has_dc ? str8_skip(string, dc_pos + 2) : str8_zero();
            
            String8_List left_groups = str8_split(scratch.arena, left, (u8 *)":", 1, StringSplitFlag_KeepEmpties);
            String8_List right_groups = str8_split(scratch.arena, right, (u8 *)":", 1, StringSplitFlag_KeepEmpties);
            
            // the final group of the address may be an embedded ipv4 literal
            String8_Node *last_node = has_dc ? right_groups.last : left_groups.last;
            bool32 last_is_ipv4 = (last_node != 0) && net_str8_to_ipv4(last_node->string, 0);
            // NOTE: embedded ipv4 is two hextets but shows up as one node.
            u64 num_hextets = left_groups.node_count + right_groups.node_count + (last_is_ipv4 ? 1 : 0);

            bool32 good_hextet_count = true;
            if (has_dc) {
                if (num_hextets > 7) good_hextet_count = false;
            }
            else {
                if (num_hextets != 8) good_hextet_count = false;
            }

            ok = good_hextet_count;
            if (good_hextet_count) {
                u64 i = 0; // we have 8 2-byte groups to fill with data
                for (String8_Node *n = left_groups.first; ok && n != 0; n = n->next) {
                    String8 group = n->string;
                    if (n == last_node && last_is_ipv4) {
                        if (i > 6) {
                            ok = false;
                        }
                        else {
                            net_str8_to_ipv4(group, &address.u32[0]);
                            i += 2;
                        }
                    }
                    else if (net__ipv6_group_is_valid(group)) {
                        if (i > 7) {
                            ok = false;
                        }
                        else {
                            address.u16[7 - i++] = (u16)u32_from_str8(group, 16);
                        }
                    }
                    else {
                        ok = false;
                    }
                }
                
                i += has_dc ? (8 - num_hextets) : 0;
                
                for (String8_Node *n = right_groups.first; ok && n != 0; n = n->next) {
                    String8 group = n->string;
                    if (n == last_node && last_is_ipv4) {
                        if (i > 6) {
                            ok = false;
                        }
                        else {
                            net_str8_to_ipv4(group, &address.u32[0]);
                            i += 2;
                        }
                    }
                    else if (net__ipv6_group_is_valid(group)) {
                        if (i > 7) {
                            ok = false;
                        }
                        else {
                            address.u16[7 - i++] = (u16)u32_from_str8(group, 16);
                        }
                    }
                    else {
                        ok = false;
                    }
                }
            } // good_hextet_count
        } // good_colons
    } // good_length
    
    if (ok && out != 0) {
#if ARCH_LITTLE_ENDIAN
        *out = address;
#else
        
#endif
    }
    
    scratch_end(scratch);
    return ok;
}

internal Net_Address net_str8_to_net_address(String8 string)
{
    Net_Address result = {0};
    Temp scratch = scratch_begin(0, 0);

    String8_List parts = str8_split(scratch.arena, string, (u8 *)":", 1, StringSplitFlag_KeepEmpties);

    if (parts.node_count == 2) {
        String8 port_part = parts.last->string;
        u32 port = u32_from_str8(port_part, 10);
        if (port <= max_u16) {
            result.port = (u16)port;
        }

        String8 ip_part = parts.first->string;
        /*
        if (net_str8_is_ipv4(ip_part)) {
            
        }
        else if (net_str8_is_ipv6(ip_part)) {
        }
        else {
        }
        */
    }
    
    scratch_end(scratch);
    return result;
}
