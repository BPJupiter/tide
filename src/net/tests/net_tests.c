Test(str8_ipv4_strings)
{
    Temp scratch = scratch_begin(0, 0);
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

    String8 invalid_seeds[] = {
        str8_lit_comp(""),                  
        str8_lit_comp("ImInvalid"),         
        str8_lit_comp("::1"),                

        str8_lit_comp("1"),                 
        str8_lit_comp("1.1"),                
        str8_lit_comp("1.1.1"),              
        str8_lit_comp("1.1.1.1.1"),          
        str8_lit_comp("1.1.1.1.1.1"),        
 
        str8_lit_comp("1.1.1.256"),          
        str8_lit_comp("999.999.999.999"),    
        str8_lit_comp("4294967296.1.1.1"),
        str8_lit_comp("18446744073709551616.1.1.1"),
 
        str8_lit_comp("196.169.01.1"),       
        str8_lit_comp("01.1.1.1"),           
        str8_lit_comp("1.01.1.1"),           
        str8_lit_comp("1.1.1.01"),           
        str8_lit_comp("010.010.010.010"),    
        str8_lit_comp("00.0.0.0"),           
 
        str8_lit_comp("1.2.3.a"),            
        str8_lit_comp("a.b.c.d"),            
        str8_lit_comp("1.2.3.4x"),           
        str8_lit_comp("0x1.0x2.0x3.0x4"),    
 
        str8_lit_comp("-1.1.1.1"),           
        str8_lit_comp("1.-1.1.1"),           
        str8_lit_comp("+1.1.1.1"),           
 
        str8_lit_comp(".1.1.1"),             
        str8_lit_comp("1.1.1."),             
        str8_lit_comp(".1.1.1.1"),           
        str8_lit_comp("1.1.1.1."),           
        str8_lit_comp("1..1.1"),             
        str8_lit_comp("1.1..1"),             
        str8_lit_comp("...."),               
        str8_lit_comp("."),                  
 
        str8_lit_comp(" 1.1.1.1"),           
        str8_lit_comp("1.1.1.1 "),           
        str8_lit_comp("1. 1.1.1"),           
        str8_lit_comp("1.1.1.1\t"),          
 
        str8_lit_comp("1.1.1.1/24"),         
        str8_lit_comp("1.1.1.1:80"),
    };
 
    // Test invalid
    for (u64 i = 0; i < ArrayCount(invalid_seeds); i++) {
        T_Ok(!net_str8_to_ipv4(invalid_seeds[i], 0));
    }

    // Test valid
    for (u64 i = 0; i < ArrayCount(valid_seeds); i++) {
        u32 ip = 0;
        T_Ok(net_str8_to_ipv4(valid_seeds[i], &ip));
        T_Ok(seed_to_u32[i] == ip);

        String8 string = net_ipv4_to_str8(scratch.arena, seed_to_u32[i]);
        T_Ok(str8_match(valid_seeds[i], string, 0));
    }
    scratch_end(scratch);
}

Test(str8_ipv6_strings)
{
    Temp scratch = scratch_begin(0, 0);
    String8 valid_seeds[] = {
        str8_lit_comp("2001:0db8:85a3:0000:0000:8a2e:0370:7334"),
        str8_lit_comp("2001:DB8:85A3:0:0:8A2E:370:7334"),
        str8_lit_comp("2001:0db8:0000:0000:0000:0000:0000:0001"),
        str8_lit_comp("0:0:0:0:0:0:0:1"),
        str8_lit_comp("0:0:0:0:0:0:0:0"),
        str8_lit_comp("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff"),
        str8_lit_comp("ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255"),
        
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
        u128_lit64(0x20010DB800000000, 0x0000000000000001),
        u128_lit64(0x0000000000000000, 0x0000000000000001),
        u128_lit64(0x0000000000000000, 0x0000000000000000),
        u128_lit64(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF),
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

    String8 invalid_seeds[] = {
        str8_lit_comp(""),
        str8_lit_comp("ImInvalid"),
        
        str8_lit_comp("1:2:3:4:5:6:7"),      
        str8_lit_comp("1:2:3:4:5:6:7:8:9"),  
        str8_lit_comp("1:2:3:4:5:6:7::8"),   
        str8_lit_comp("12345::"),            

        str8_lit_comp("2001:db8:xyz::1"),    
        str8_lit_comp("1g::"),               

        str8_lit_comp("1::2::3"),            
        str8_lit_comp(":::1"),               
        str8_lit_comp("1:::"),               
        str8_lit_comp(":1:2:3:4:5:6:7:8"),   
        str8_lit_comp("1:2:3:4:5:6:7:8:"),   

        str8_lit_comp("1.1.1"),              
        str8_lit_comp("1.1.1.1.1"),          
        str8_lit_comp("256.1.1.1"),          
        str8_lit_comp("196.169.01.1"),       
        str8_lit_comp("1.2.3.a"),            
        str8_lit_comp("127.0.0.1::"),        
        str8_lit_comp("1:2:3:4:5:6:127.0.0.1::8"),  
        str8_lit_comp("1:2:3:4:5:6:7:192.168.1.1"), 
        str8_lit_comp("1:2:3:4:5:192.168.1.1"),     
        str8_lit_comp("::ffff:192.168.1"),          
        str8_lit_comp("::ffff:192.168.1.1.1"),      
    };

    // Test invalid
    for (u64 i = 0; i < ArrayCount(invalid_seeds); i++) {
        T_Ok(!net_str8_to_ipv6(invalid_seeds[i], 0));
    }

    // Test valid
    for (u64 i = 0; i < ArrayCount(valid_seeds); i++) {
        u128 ip = {0};
        T_Ok(net_str8_to_ipv6(valid_seeds[i], &ip));
        T_Ok(u128_match(seed_to_u128[i], ip));

        String8 string = net_ipv6_to_str8(scratch.arena, seed_to_u128[i]);
        // @TODO: make a ipv6_str8_match function
        //T_Ok(str8_match(valid_seeds[i], string, 0));
        
        T_Ok(net_str8_to_ipv6(string, &ip));
        T_Ok(u128_match(seed_to_u128[i], ip));
    }

    scratch_end(scratch);
}

Test(str8_to_address)
{
    Temp scratch = scratch_begin(0, 0);
    String8 valid_seeds[] = {
        str8_lit_comp("192.168.1.1:8080"),
        str8_lit_comp("127.0.0.1:80"),
        str8_lit_comp("127.0.0.1:0080"),
        str8_lit_comp("0.0.0.0:443"),
        str8_lit_comp("255.255.255.255:65535"),
        
        str8_lit_comp("[2001:db8::1]:8000"),
        str8_lit_comp("[::1]:443"),
        str8_lit_comp("[::]:0"),
        str8_lit_comp("[2001:0db8:85a3:0000:0000:8a2e:0370:7334]:22"),
        str8_lit_comp("[::ffff:192.168.1.1]:8080"),
    };

    Net_Address seed_to_struct[] = {
        { .ip = { ._padding = {0}, .v4 = 0xC0A80101 }, .address_type = Net_AddressType_Ipv4, .port = 8080 },
        { .ip = { ._padding = {0}, .v4 = 0x7F000001 }, .address_type = Net_AddressType_Ipv4, .port = 80 },
        { .ip = { ._padding = {0}, .v4 = 0x7F000001 }, .address_type = Net_AddressType_Ipv4, .port = 80 },
        { .ip = { ._padding = {0}, .v4 = 0x00000000 }, .address_type = Net_AddressType_Ipv4, .port = 443 },
        { .ip = { ._padding = {0}, .v4 = 0xFFFFFFFF }, .address_type = Net_AddressType_Ipv4, .port = 65535 },
        
        { .ip = { .v6 = u128_lit64(0x20010DB800000000, 0x0000000000000001) }, .address_type = Net_AddressType_Ipv6, .port = 8000 },
        { .ip = { .v6 = u128_lit64(0x0000000000000000, 0x0000000000000001) }, .address_type = Net_AddressType_Ipv6, .port = 443 },
        { .ip = { .v6 = u128_lit64(0x0000000000000000, 0x0000000000000000) }, .address_type = Net_AddressType_Ipv6, .port = 0 },
        { .ip = { .v6 = u128_lit64(0x20010DB885A30000, 0x00008A2E03707334) }, .address_type = Net_AddressType_Ipv6, .port = 22 },
        { .ip = { .v6 = u128_lit64(0x0000000000000000, 0x0000FFFFC0A80101) }, .address_type = Net_AddressType_Ipv6, .port = 8080 },
    };

    for (u64 i = 0; i < ArrayCount(valid_seeds); i++) {
        Net_Address result = {0};
        T_Ok(net_str8_to_address(valid_seeds[i], &result));
        //String8 hexdump = HexdumpStructStr8(scratch.arena, result);
        //printf("\n%.*s\n", str8_varg(hexdump));
        T_Ok(MemoryMatchStruct(&result, &seed_to_struct[i]));
    }

    String8 invalid_seeds[] = {
        str8_lit_comp(""),
        str8_lit_comp("hello_world"),
        str8_lit_comp(":"),
        str8_lit_comp("0.0.0.0"),
        str8_lit_comp(":80"),

        str8_lit_comp("192.168.1.1"),
        str8_lit_comp("192.168.1.1:"),
        
        str8_lit_comp("256.0.0.1:80"),
        str8_lit_comp("192.168.1.999:80"),
        str8_lit_comp("999.999.999.999:80"),
        
        str8_lit_comp("192.168.1:80"),
        str8_lit_comp("192.168.1.1.1:80"),
        
        str8_lit_comp("-1.0.0.0:80"),
        str8_lit_comp("192.168.1a.1:80"),
        str8_lit_comp("192.168.1. 1:80"),
        str8_lit_comp("[192.168.1.1]:80"),

        str8_lit_comp("[2001:db8::1]"),
        str8_lit_comp("[2001:db8::1]:"),
        
        str8_lit_comp("2001:db8::1:8080"),
        str8_lit_comp("[2001:db8::1:8080"),
        str8_lit_comp("2001:db8::1]:8080"),
        
        str8_lit_comp("[2001:xyz::1]:80"),
        str8_lit_comp("[-2001:db8::1]:80"),
        
        str8_lit_comp("[2001::db8::1]:80"),
        str8_lit_comp("[1:2:3:4:5:6:7:8:9]:80"),
        str8_lit_comp("[1:2:3:4:5:6:7:8::]:80"),
        str8_lit_comp("[::::]:80"),

        str8_lit_comp("127.0.0.1:65536"),
        str8_lit_comp("[::1]:999999"),
        
        str8_lit_comp("127.0.0.1:-80"),
        str8_lit_comp("127.0.0.1:80a"),
        str8_lit_comp("127.0.0.1:http"),
        
        str8_lit_comp(" 127.0.0.1:80"),
        str8_lit_comp("127.0.0.1:80 "),
        str8_lit_comp("127.0.0.1 :80"),
        str8_lit_comp("127.0.0.1: 80"),
    };

    for (u64 i = 0; i < ArrayCount(invalid_seeds); i++) {
        T_Ok(!net_str8_to_address(invalid_seeds[i], 0));
    }
    scratch_end(scratch);
}
