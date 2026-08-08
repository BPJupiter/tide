Test(str8_ipv4_strings)
{
    String8 valid_seeds[] = {
        str8_lit_comp("0.0.0.0"),
        str8_lit_comp("255.255.255.255"),
        str8_lit_comp("192.168.1.1"),
        str8_lit_comp("127.0.0.1"),
        str8_lit_comp("10.0.0.1"),
        str8_lit_comp("1.2.3.4"),
        str8_lit_comp("100.100.100.100"),
        str8_lit_comp("8.8.8.8"),
    };

    u32 seed_to_u32[] = {
        0x00000000,
        0xFFFFFFFF,
        0xC0A80101,
        0x7F000001,
        0x0A000001,
        0x01020304,
        0x64646464,
        0x08080808,
    };

    // Test invalid
    {
        T_Ok(!net_str8_to_ipv4(str8_lit("ImInvalid"),    0));
        T_Ok(!net_str8_to_ipv4(str8_lit("1.1.1"),        0));
        T_Ok(!net_str8_to_ipv4(str8_lit("1.1.1.1.1"),    0));
        T_Ok(!net_str8_to_ipv4(str8_lit("256.1.1.1"),    0));
        T_Ok(!net_str8_to_ipv4(str8_lit("196.169.01.1"), 0));
        T_Ok(!net_str8_to_ipv4(str8_lit("1.2.3.a"),      0));
        T_Ok(!net_str8_to_ipv4(str8_lit(""),             0));
    }

    // Test valid
    {
        for (u64 i = 0; i < ArrayCount(valid_seeds); i++) {
            u32 result = 0;
            T_Ok(net_str8_to_ipv4(valid_seeds[i], &result));
            T_Ok(seed_to_u32[i] == result);
        }
    }
}

Test(str8_ipv6_strings)
{
    String8 valid_seeds[] = {
        str8_lit_comp("2001:0db8:85a3:0000:0000:8a2e:0370:7334"),
        str8_lit_comp("2001:DB8:85A3:0:0:8A2E:370:7334"),
        str8_lit_comp("0:0:0:0:0:0:0:1"),
        str8_lit_comp("0:0:0:0:0:0:0:0"),
        str8_lit_comp("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff"),
        
        str8_lit_comp("2001:db8:85a3::8a2e:370:7334"),
        str8_lit_comp("2001:db8::"),
        str8_lit_comp("::1"),
        str8_lit_comp("::"),
        str8_lit_comp("1:2:3:4:5:6:7::"),
        str8_lit_comp("::1:2:3:4:5:6:7"),
        str8_lit_comp("2001:db8::1:0:0:1"),
        str8_lit_comp("fe80::200:5aee:feaa:20a2"),
        str8_lit_comp("aBcD:eF01::"),

        str8_lit_comp("0:0:0:0:0:ffff:192.168.1.1"),
        str8_lit_comp("::ffff:192.168.1.1"),
        str8_lit_comp("::192.168.1.1"),
        str8_lit_comp("::ffff:127.0.0.1"),
        str8_lit_comp("::ffff:192.0.2.128"),
        str8_lit_comp("1::127.0.0.1"),
    };

    u128 seed_to_u128[] = {
        u128_lit64(0x20010DB885A30000, 0x00008A2E03707334),
        u128_lit64(0x20010DB885A30000, 0x00008A2E03707334),
        u128_lit64(0x0000000000000000, 0x0000000000000001),
        u128_lit64(0x0000000000000000, 0x0000000000000000),
        u128_lit64(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF),

        u128_lit64(0x20010DB885A30000, 0x00008A2E03707334),
        u128_lit64(0x20010DB800000000, 0x0000000000000000),
        u128_lit64(0x0000000000000000, 0x0000000000000001),
        u128_lit64(0x0000000000000000, 0x0000000000000000),
        u128_lit64(0x0001000200030004, 0x0005000600070000),
        u128_lit64(0x0000000100020003, 0x0004000500060007),
        u128_lit64(0x20010DB800000000, 0x0001000000000001),
        u128_lit64(0xFE80000000000000, 0x02005AEEFEAA20A2),
        u128_lit64(0xABCDEF0100000000, 0x0000000000000000),

        u128_lit64(0x0000000000000000, 0x0000FFFFC0A80101),
        u128_lit64(0x0000000000000000, 0x0000FFFFC0A80101),
        u128_lit64(0x0000000000000000, 0x00000000C0A80101),
        u128_lit64(0x0000000000000000, 0x0000FFFF7F000001),
        u128_lit64(0x0000000000000000, 0x0000FFFFC0000280),
        u128_lit64(0x0001000000000000, 0x000000007F000001),
    };

    // Test invalid
    {
        T_Ok(!net_str8_to_ipv6(str8_lit(""),                   0));
        T_Ok(!net_str8_to_ipv6(str8_lit("ImInvalid"),          0));
        
        T_Ok(!net_str8_to_ipv6(str8_lit("1:2:3:4:5:6:7"),      0));
        T_Ok(!net_str8_to_ipv6(str8_lit("1:2:3:4:5:6:7:8:9"),  0));
        T_Ok(!net_str8_to_ipv6(str8_lit("1:2:3:4:5:6:7::8"),   0));
        T_Ok(!net_str8_to_ipv6(str8_lit("12345::"),            0));

        T_Ok(!net_str8_to_ipv6(str8_lit("2001:db8:xyz::1"),    0));
        T_Ok(!net_str8_to_ipv6(str8_lit("1g::"),               0));

        T_Ok(!net_str8_to_ipv6(str8_lit("1::2::3"),            0));
        T_Ok(!net_str8_to_ipv6(str8_lit(":::1"),               0));
        T_Ok(!net_str8_to_ipv6(str8_lit("1:::"),               0));
        T_Ok(!net_str8_to_ipv6(str8_lit(":1:2:3:4:5:6:7:8"),   0));
        T_Ok(!net_str8_to_ipv6(str8_lit("1:2:3:4:5:6:7:8:"),   0));

        T_Ok(!net_str8_to_ipv6(str8_lit("1.1.1"),              0));
        T_Ok(!net_str8_to_ipv6(str8_lit("1.1.1.1.1"),          0));
        T_Ok(!net_str8_to_ipv6(str8_lit("256.1.1.1"),          0));
        T_Ok(!net_str8_to_ipv6(str8_lit("196.169.01.1"),       0));
        T_Ok(!net_str8_to_ipv6(str8_lit("1.2.3.a"),            0));
        T_Ok(!net_str8_to_ipv6(str8_lit("127.0.0.1::"),        0));
        T_Ok(!net_str8_to_ipv6(str8_lit("1:2:3:4:5:6:127.0.0.1::8"),  0));
        T_Ok(!net_str8_to_ipv6(str8_lit("1:2:3:4:5:6:7:192.168.1.1"), 0));
        T_Ok(!net_str8_to_ipv6(str8_lit("1:2:3:4:5:192.168.1.1"),     0));
        T_Ok(!net_str8_to_ipv6(str8_lit("::ffff:192.168.1"),          0));
        T_Ok(!net_str8_to_ipv6(str8_lit("::ffff:192.168.1.1.1"),      0));
    }

    // Test valid
    {
        for (u64 i = 0; i < ArrayCount(valid_seeds); i++) {
            u128 result = {0};
            T_Ok(net_str8_to_ipv6(valid_seeds[i], &result));
            T_Ok(u128_match(seed_to_u128[i], result));
        }
    }
}

