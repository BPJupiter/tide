// Copyright (c) Frances Telfar
// Licensed under the PolyForm Noncommercial License (https://polyformproject.org/licenses/noncommercial/1.0.0)

Test(str8_ipv4_strings)
{
    Temp scratch = scratch_begin(0, 0);
    struct
    {
        String8 string;
        u32 u32;
    } valid_addresses[] = {
        {str8_lit_comp("0.0.0.0"),         0x00000000},
        {str8_lit_comp("255.255.255.255"), 0xFFFFFFFF},
        {str8_lit_comp("192.168.1.1"),     0xC0A80101},
        {str8_lit_comp("127.0.0.1"),       0x7F000001},
        {str8_lit_comp("10.0.0.1"),        0x0A000001},
        {str8_lit_comp("1.2.3.4"),         0x01020304},
        {str8_lit_comp("100.100.100.100"), 0x64646464},
        {str8_lit_comp("8.8.8.8"),         0x08080808},
    };
        
    String8 invalid_addresses[] = {
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
 
        //str8_lit_comp("196.169.01.1"),       
        //str8_lit_comp("01.1.1.1"),           
        //str8_lit_comp("1.01.1.1"),           
        //str8_lit_comp("1.1.1.01"),           
        //str8_lit_comp("010.010.010.010"),    
        //str8_lit_comp("00.0.0.0"),           
 
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
    for (u64 i = 0; i < ArrayCount(invalid_addresses); i++) {
        NET_Endpoint ep = net_endpoint_from_string_port(invalid_addresses[i], 0);
        T_Ok(ep.kind != NET_EndpointKind_IPv4);
    }

    // Test valid
    for (u64 i = 0; i < ArrayCount(valid_addresses); i++) {
        NET_Endpoint ep = net_endpoint_from_string_port(valid_addresses[i].string, 0);
        T_Ok(ep.kind == NET_EndpointKind_IPv4);
        T_Ok(ep.address.u32[3] == valid_addresses[i].u32);

        String8 string = net_string_from_endpoint(scratch.arena, ep);
        String8 addr_part = str8_prefix(string, str8_find_needle(string, 0, s(":"), 0));
        T_Ok(str8_match(valid_addresses[i].string, addr_part, 0));
    }
    scratch_end(scratch);
}

Test(str8_ipv6_strings)
{
    Temp scratch = scratch_begin(0, 0);
    struct
    {
        String8 string;
        u128 u128;
    } valid_addresses[] = {
        {str8_lit_comp("2001:0db8:85a3:0000:0000:8a2e:0370:7334"),u128_lit64(0x20010DB885A30000, 0x00008A2E03707334)},
        {str8_lit_comp("2001:DB8:85A3:0:0:8A2E:370:7334"),        u128_lit64(0x20010DB885A30000, 0x00008A2E03707334)},
        {str8_lit_comp("2001:0db8:0000:0000:0000:0000:0000:0001"),u128_lit64(0x20010DB800000000, 0x0000000000000001)},
        {str8_lit_comp("0:0:0:0:0:0:0:1"),                        u128_lit64(0x0000000000000000, 0x0000000000000001)},
        {str8_lit_comp("0:0:0:0:0:0:0:0"),                        u128_lit64(0x0000000000000000, 0x0000000000000000)},
        {str8_lit_comp("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff"),u128_lit64(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF)},
        //{str8_lit_comp("ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255"),u128_lit64(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF)},
        
        {str8_lit_comp("2001:db8:85a3::8a2e:370:7334"), u128_lit64(0x20010DB885A30000, 0x00008A2E03707334)},
        {str8_lit_comp("2001:db8::370:7334"),           u128_lit64(0x20010DB800000000, 0x0000000003707334)},
        {str8_lit_comp("2001:db8::8a2e:370:7334"),      u128_lit64(0x20010DB800000000, 0x00008A2E03707334)},
        {str8_lit_comp("2001:db8:85a3::370:7334"),      u128_lit64(0x20010DB885A30000, 0x0000000003707334)},
        {str8_lit_comp("2001:db8::"),                   u128_lit64(0x20010DB800000000, 0x0000000000000000)},
        {str8_lit_comp("::1"),                          u128_lit64(0x0000000000000000, 0x0000000000000001)},
        {str8_lit_comp("::"),                           u128_lit64(0x0000000000000000, 0x0000000000000000)},
        {str8_lit_comp("1:2:3:4:5:6:7::"),              u128_lit64(0x0001000200030004, 0x0005000600070000)},
        {str8_lit_comp("::1:2:3:4:5:6:7"),              u128_lit64(0x0000000100020003, 0x0004000500060007)},
        {str8_lit_comp("2001:db8::1:0:0:1"),            u128_lit64(0x20010DB800000000, 0x0001000000000001)},
        {str8_lit_comp("fe80::200:5aee:feaa:20a2"),     u128_lit64(0xFE80000000000000, 0x02005AEEFEAA20A2)},
        {str8_lit_comp("aBcD:eF01::"),                  u128_lit64(0xABCDEF0100000000, 0x0000000000000000)},

        //{str8_lit_comp("0:0:0:0:0:ffff:192.168.1.1"),   u128_lit64(0x0000000000000000, 0x0000FFFFC0A80101)},
        //{str8_lit_comp("::ffff:192.168.1.1"),           u128_lit64(0x0000000000000000, 0x0000FFFFC0A80101)},
        //{str8_lit_comp("::192.168.1.1"),                u128_lit64(0x0000000000000000, 0x00000000C0A80101)},
        //{str8_lit_comp("::ffff:127.0.0.1"),             u128_lit64(0x0000000000000000, 0x0000FFFF7F000001)},
        //{str8_lit_comp("::ffff:192.0.2.128"),           u128_lit64(0x0000000000000000, 0x0000FFFFC0000280)},
        //{str8_lit_comp("1::127.0.0.1"),                 u128_lit64(0x0001000000000000, 0x000000007F000001)},
    };

    String8 invalid_addresses[] = {
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
        //str8_lit_comp("196.169.01.1"),       
        str8_lit_comp("1.2.3.a"),            
        str8_lit_comp("127.0.0.1::"),        
        str8_lit_comp("1:2:3:4:5:6:127.0.0.1::8"),  
        str8_lit_comp("1:2:3:4:5:6:7:192.168.1.1"), 
        str8_lit_comp("1:2:3:4:5:192.168.1.1"),     
        str8_lit_comp("::ffff:192.168.1"),          
        str8_lit_comp("::ffff:192.168.1.1.1"),      
    };

    // Test invalid
    for (u64 i = 0; i < ArrayCount(invalid_addresses); i++) {
        NET_Endpoint ep = net_endpoint_from_string_port(invalid_addresses[i], 0);
        T_Ok(ep.kind == NET_EndpointKind_Null);
    }

    // Test valid
    for (u64 i = 0; i < ArrayCount(valid_addresses); i++) {
        NET_Endpoint ep = net_endpoint_from_string_port(valid_addresses[i].string, 0);
        T_Ok(ep.kind == NET_EndpointKind_IPv6);
        T_Ok(u128_match(ep.address, valid_addresses[i].u128));

        String8 string = net_string_from_endpoint(scratch.arena, ep);
        ep = net_endpoint_from_string(string);
        T_Ok(u128_match(ep.address, valid_addresses[i].u128));
    }

    scratch_end(scratch);
}

Test(connect_to_server)
{
    Temp scratch = scratch_begin(0, 0);
    {
        NET_Session test_session = net_session_open(0, 0);
        // We can connect to google's public DNS over HTTPS
        // as they offer web-client to explore responses.
        String8 request = str8_lit("GET / HTTP/1.1\r\n"
                                   "Host: 8.8.8.8\r\n"
                                   "\r\n");

        NET_Endpoint ep = net_endpoint_from_string(s("8.8.8.8:443"));
        T_Ok(net_send(test_session, NET_Protocol_TCP, ep, request, max_u64));
        NET_Protocol p_out = {0};
        NET_Endpoint ep_out = {0};
        String8 data_out = {0};
        T_Ok(net_recv(scratch.arena, test_session, &p_out, &ep_out, &data_out, max_u64));
        // We get a FIN & RST response from google because we aren't encrypted :P
        T_Ok(data_out.size == 0);
        net_session_close(test_session);
    }
    
    scratch_end(scratch);
}
