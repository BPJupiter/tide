// Copyright (c) Frances Telfar
// Licensed under the PolyForm Noncommercial License (https://polyformproject.org/licenses/noncommercial/1.0.0)

///////////////////////////////
// String <-> Binary Formats

internal NET_Endpoint net_endpoint_from_string_port(String8 address, u16 port)
{
    // detect kind
    NET_EndpointKind kind = NET_EndpointKind_IPv4;
    {
        if (str8_find_needle(address, 0, s(":"), 0) < address.size)
        {
            kind = NET_EndpointKind_IPv6;
        }
    }

    // fill based on format
    NET_Endpoint ep = {0};
    ep.port = port;
    ep.kind = kind;
    switch(kind)
    {
        default:
        case NET_EndpointKind_IPv4: {
            {
                u64 off = 0;
                u64 part_start_off = 0;
                u64 bad = 0;
                ep.address.u8[10] = 0xff;
                ep.address.u8[11] = 0xff;
                for(;off < 4;)
                {
                    u64 part_end_off = str8_find_needle(address, part_start_off+1, s("."), 0);
                    String8 part = str8_substr(address, r1u64(part_start_off, part_end_off));
                    u64 part_val = u64_from_str8(part, 10);
                    bad += (part.size == 0 || part.size > 3 || part_val > 255);
                    ep.address.u8[12 + 3 - off] = (u8)part_val;
                    part_start_off = part_end_off+1;
                    off += 1;
                    if (part_end_off >= address.size)
                    {
                        break;
                    }
                }

                if (bad || off != 4 || part_start_off <= address.size)
                {
                    ep.kind = NET_EndpointKind_Null;
                }
            }
        } break;
        case NET_EndpointKind_IPv6: {
            {
                u64 off = 0;
                u64 part_start_off = 0;
                u64 part_end_off = 0;
                u64 double_colon_off = max_u64;
                u64 double_colon_count = 0;
                u64 bad = 0;
                for (;off < 8;)
                {
                    if (part_start_off < address.size && address.str[part_start_off] == ':')
                    {
                        double_colon_off = off;
                        double_colon_count += 1;
                        part_start_off = part_end_off+2;
                    }
                    if (part_end_off+1 >= address.size)
                    {
                        break;
                    } 
                    part_end_off = str8_find_needle(address, part_start_off+1, s(":"), 0);
                    String8 part = str8_substr(address, r1u64(part_start_off, part_end_off));
                    u64 part_val = u64_from_str8(part, 16);
                    bad += (part_val > 0xffff);
                    ep.address.u16[7 - off] = (u16)part_val;
                    part_start_off = part_end_off+1;
                    off += (part.size != 0);
                }
                if (bad || double_colon_count > 1 || part_start_off <= address.size || (double_colon_off == max_u64 ? off != 8 : off >= 8))
                {
                    ep.kind = NET_EndpointKind_Null;
                }
                if (ep.kind != NET_EndpointKind_Null && off < 8 && double_colon_off < 8)
                {
                    u64 shift_amt = 8 - off;
                    for (u64 move_off = 0; move_off < off - double_colon_off; move_off += 1)
                    {
                        ep.address.u16[move_off] = ep.address.u16[move_off+shift_amt];
                        ep.address.u16[move_off+shift_amt] = 0;
                    }
                }
            }
        } break;
    }
    return ep;
}

internal NET_Endpoint net_endpoint_from_string(String8 address_and_port)
{
    String8 addr = {0};
    u64 port_off = 0;
    if (address_and_port.size > 0 && address_and_port.str[0] == '[')
    {
        u64 addr_end_off = str8_find_needle(address_and_port, 1, s("]"), 0);
        addr = str8_substr(address_and_port, r1u64(1, addr_end_off));
        port_off = str8_find_needle(address_and_port, addr_end_off+1, s(":"), 0)+1;
    }
    else
    {
        u64 colon_off = str8_find_needle(address_and_port, 0, s(":"), 0);
        port_off = colon_off + 1;
        addr = str8_prefix(address_and_port, colon_off);
    }
    String8 port_string = str8_skip(address_and_port, port_off);
    u16 port = u64_from_str8(port_string, 10);
    NET_Endpoint ep = net_endpoint_from_string_port(addr, port);
    return ep;
}

internal String8 net_string_from_endpoint(Arena *arena, NET_Endpoint endpoint)
{
    String8 result = {0};
    NOTIMPL_WARNING(net_string_from_endpoint);
    return result;
}

