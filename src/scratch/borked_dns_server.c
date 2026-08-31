
////////////////////
// Build Options

#define BUILD_TITLE "BORKED_DNS_SERVER"
#define BUILD_CONSOLE_INTERFACE 1

//////////////
// Includes

#include "base/base_inc.h"
#include "net/net_inc.h"
#include "dns/dns_inc.h"

#include "base/base_inc.c"
#include "net/net_inc.c"
#include "dns/dns_inc.c"

#define LISTENING_PORT 1225

internal void bdns_g01(void)
{
    printf("g01\n");

    
}

internal void bdns_g02(void)
{
    printf("g02\n");
}

internal void bdns_g03(void)
{
    printf("g03\n");
}

internal void bdns_c01(void)
{
    printf("c01\n");
}

internal void bdns_c02(void)
{
    printf("c02\n");
}

internal void bdns_d01(void)
{
    /* dnspython */
    /* UDP only */
    Temp scratch = scratch_begin(0, 0);
    printf("d01\n");

    DNS_Server server = dns_server_alloc(NET_AddressFamily_IPv4, DNS_TransportProtocol_UDP, LISTENING_PORT);
    NET_Client client = net_listener_accept(scratch.arena, server.listener);
    DNS_Msg query = {0};
    if (dns_unpack_msg(scratch.arena, client.recv_buffer, &query))
    {
        // TODO: Waiting for group mate to write python script that actually drops
        //       malformed packets.
        
        // bad message
        {
            net_client_send_from_ring(&client); // empty message
        }
        
        // good message
        {
            DNS_Msg msg = query; // shallow copy
            
            msg.header.query_response = true;
            msg.header.authoritative = true;
            msg.header.rcode  = DNS_RCode_NoError;
            
            msg.header.answer_count = 1;
            msg.answer = push_array(scratch.arena, DNS_RR, 1);
            
            msg.answer[0].name  = query.question[0].name;
            msg.answer[0].type  = query.question[0].type;
            msg.answer[0].class = query.question[0].class;
            msg.answer[0].ttl   = 300;
            
            msg.answer[0].rdata.A.addr = 0x0b16212c;
            
            dns_pack_msg(client.send_buffer, &msg);
            net_client_send_from_ring(&client);
        }
    }
    

    scratch_end(scratch);
}

internal void bdns_go1(void)
{
    printf("go1\n");
}

internal void bdns_help(void)
{
    fprintf(stderr, "--- Help -------------------------------------------------------\n");
    fprintf(stderr, " %s\n\n", BUILD_TITLE_STRING_LITERAL);
    fprintf(stderr, " Usage: borked_dns_server [Options]\n\n");
    fprintf(stderr, " Options:\n");
    fprintf(stderr, "   -g01 : CVE-2023-4527\n");
    fprintf(stderr, "   -g02 : CVE-2026-4427\n");
    fprintf(stderr, "   -g03 : CVE-2026-4438\n");
    fprintf(stderr, "   -c01 : CVE-2026-33630\n");
    fprintf(stderr, "   -c02 : CVE-2025-62408\n");
    fprintf(stderr, "   -d01 : CVE-2023-29483\n");
    fprintf(stderr, "   -go1 : CVE-2024-24788\n");
}

internal void entry_point(Cmd_Line *cmdline)
{
    Temp scratch = scratch_begin(0, 0);
    u64 exit_code = max_u64;

    if      (cmd_line_has_flag(cmdline, str8_lit("g01"))) { bdns_g01(); }
    else if (cmd_line_has_flag(cmdline, str8_lit("g02"))) { bdns_g02(); }
    else if (cmd_line_has_flag(cmdline, str8_lit("g03"))) { bdns_g03(); }
    else if (cmd_line_has_flag(cmdline, str8_lit("c01"))) { bdns_c01(); }
    else if (cmd_line_has_flag(cmdline, str8_lit("c02"))) { bdns_c02(); }
    else if (cmd_line_has_flag(cmdline, str8_lit("d01"))) { bdns_d01(); }
    else if (cmd_line_has_flag(cmdline, str8_lit("go1"))) { bdns_go1(); }
    else                                                  { bdns_help(); }
    
    scratch_end(scratch);
    exit(exit_code);
}
