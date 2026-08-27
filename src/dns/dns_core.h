// This DNS implementation takes inspiration from
// https://codeberg.org/miekg/dns where I thought
// appropriate!

#ifndef DNS_CORE_H
#define DNS_CORE_H

///////////////////
// Generated Code

#include "generated/dns.meta.h"

// This is a temporary array for me to easily keep track of
// what is an isn't supported at this time.
global bool32 supported_dns_types[1ULL << 16] = {
    [DNS_Type_A]     = true,
    [DNS_Type_NS]    = true,
    [DNS_Type_CNAME] = true,
    [DNS_Type_SOA]   = true,
    [DNS_Type_AAAA]  = true,
};

typedef enum DNS_TransportProtocol DNS_TransportProtocol;
enum DNS_TransportProtocol {
    DNS_TransportProtocol_UDP,
    DNS_TransportProtocol_TCP,
    DNS_TransportProtocol_TLS,
    DNS_TransportProtocol_HTTPS,
    DNS_TransportProtocol_COUNT
};

////////////////////////
// Message Structures

typedef struct DNS_RR DNS_RR;
struct DNS_RR {
    String8   name;
    DNS_Type  type;
    DNS_Class class;
    u32       ttl;
    
    union
    {
        struct {
            u32 addr;
        } A;
        struct {
            String8 ns;
        } NS;
        struct {
            String8 target;
        } CNAME;
        struct {
            String8 master_name;
            String8 responsible_name;
            u32 serial;
            u32 refresh;
            u32 retry;
            u32 expire;
            u32 minimum;
        } SOA;
        struct {
            u128 addr;
        } AAAA;
    } rdata;
};

typedef struct DNS_Msg_Header DNS_Msg_Header;
struct DNS_Msg_Header {
    u16 id;
    
    bool32 query_response;
    DNS_OpCode opcode;
    bool32 authoritative;
    bool32 truncated;
    bool32 recursion_desired;
    bool32 recursion_available;
    bool32 zero;
    bool32 authenticated_data;
    bool32 checking_disabled;
    DNS_RCode rcode;

    u16 question_count;
    u16 answer_count;
    u16 nameserver_count;
    u16 additional_count;
};

typedef struct DNS_Msg DNS_Msg;
struct DNS_Msg {
    DNS_Msg_Header header;
    DNS_RR *question; // the only reason this is an array is because some clients MAY send more than one question.
                      // we will only ever write one ourselves.
    DNS_RR *answer;
    DNS_RR *ns;
    DNS_RR *extra;
};

///////////////////////
// Client Structures

typedef struct DNS_Client DNS_Client;
struct DNS_Client {
    NET_Client dialer;
    DNS_TransportProtocol dns_protocol;
};

/////////////////////////
// Server Structures

typedef struct DNS_Server DNS_Server;
struct DNS_Server {
    NET_Listener listener;
    DNS_TransportProtocol dns_protocol;
};

global String8 dns_root_server_to_domain_name[] = {
    [DNS_RootServer_A] = str8_lit_comp("a.root-servers.net"),
    [DNS_RootServer_B] = str8_lit_comp("b.root-servers.net"),
    [DNS_RootServer_C] = str8_lit_comp("c.root-servers.net"),
    [DNS_RootServer_D] = str8_lit_comp("d.root-servers.net"),
    [DNS_RootServer_E] = str8_lit_comp("e.root-servers.net"),
    [DNS_RootServer_F] = str8_lit_comp("f.root-servers.net"),
    [DNS_RootServer_G] = str8_lit_comp("g.root-servers.net"),
    [DNS_RootServer_H] = str8_lit_comp("h.root-servers.net"),
    [DNS_RootServer_I] = str8_lit_comp("i.root-servers.net"),
    [DNS_RootServer_J] = str8_lit_comp("j.root-servers.net"),
    [DNS_RootServer_K] = str8_lit_comp("k.root-servers.net"),
    [DNS_RootServer_L] = str8_lit_comp("l.root-servers.net"),
    [DNS_RootServer_M] = str8_lit_comp("m.root-servers.net"),
};

global u32 dns_root_server_to_ipv4[] = {
    [DNS_RootServer_A] = 0xC6290004, // 198.41.0.4
    [DNS_RootServer_B] = 0xAAF7AA02, // 170.247.170.2
    [DNS_RootServer_C] = 0xC021040C, // 192.33.4.12
    [DNS_RootServer_D] = 0xC7075B0D, // 199.7.91.13
    [DNS_RootServer_E] = 0xC0CBE60A, // 192.203.230.10
    [DNS_RootServer_F] = 0xC00505F1, // 192.5.5.241
    [DNS_RootServer_G] = 0xC0702404, // 192.112.36.4
    [DNS_RootServer_H] = 0xC661BE35, // 198.97.190.53
    [DNS_RootServer_I] = 0xC0249411, // 192.36.148.17
    [DNS_RootServer_J] = 0xC03A801E, // 192.58.128.30
    [DNS_RootServer_K] = 0xC1000E81, // 193.0.14.129
    [DNS_RootServer_L] = 0xC707532A, // 199.7.83.42
    [DNS_RootServer_M] = 0xCA0C1B21, // 202.12.27.33
};

global u128 dns_root_server_to_ipv6[] = {
    [DNS_RootServer_A] = u128_lit64(0x20010503BA3E0000, 0x0000000000020030), // 2001:503:ba3e::2:30
    [DNS_RootServer_B] = u128_lit64(0x280101B800100000, 0x000000000000000B), // 2801:1b8:10::b
    [DNS_RootServer_C] = u128_lit64(0x2001050000020000, 0x000000000000000C), // 2001:500:2::c
    [DNS_RootServer_D] = u128_lit64(0x20010500002D0000, 0x000000000000000D), // 2001:500:2d::d
    [DNS_RootServer_E] = u128_lit64(0x2001050000A80000, 0x000000000000000E), // 2001:500:a8::e
    [DNS_RootServer_F] = u128_lit64(0x20010500002F0000, 0x000000000000000F), // 2001:500:2f::f
    [DNS_RootServer_G] = u128_lit64(0x2001050000120000, 0x0000000000000D0D), // 2001:500:12::d0d
    [DNS_RootServer_H] = u128_lit64(0x2001050000010000, 0x0000000000000053), // 2001:500:1::53
    [DNS_RootServer_I] = u128_lit64(0x200107FE00000000, 0x0000000000000053), // 2001:7fe::53
    [DNS_RootServer_J] = u128_lit64(0x200105030C270000, 0x0000000000020030), // 2001:503:c27::2:30
    [DNS_RootServer_K] = u128_lit64(0x200107FD00000000, 0x0000000000000001), // 2001:7fd::1
    [DNS_RootServer_L] = u128_lit64(0x20010500009F0000, 0x0000000000000042), // 2001:500:9f::42
    [DNS_RootServer_M] = u128_lit64(0x20010DC300000000, 0x0000000000000035), // 2001:dc3::35
};

///////////////
// Constants

// DEFAULT_MSG_SIZE is the default for messages larger than 512 bytes.
// this limit is the recommendation from rfc 9715
#define DNS_DEFAULT_MSG_SIZE     1400
#define DNS_MIN_MSG_SIZE         512
#define DNS_MAX_MSG_SIZE         max_u16
#define DNS_MSG_HEADER_SIZE      12
#define DNS_MAX_SERIAL_INCREMENT max_u32

#define DNS_MAX_COMPRESSION_JUMPS 16 // arbitrary
#define DNS_MAX_LABEL_LEN 63
#define DNS_MAX_NAME_LEN  255
#define DNS_MAX_RDATA_LEN (Kilobytes(4096)) // rfc 6891

//////////////////////////////////
// @REMOVE: Supported DNS Check

#define DNS_CRASH_THE_PROGRAM_IF_THIS_TYPE_IS_SUPPORTED(type)                                \
    do {                                                                                     \
        if (supported_dns_types[type]) {                                                     \
            fprintf(stderr, "////////////////////////////////////////////////////////\n");   \
            fprintf(stderr, " SUPPORTED DNS TYPE NOT IMPLEMENTED! : %.*s\n ",                \
                    str8_varg(dns_string_from_type(type)));                                  \
            fprintf(stderr, "////////////////////////////////////////////////////////\n");   \
            u64 *SUPPORTED_DNS_TYPE_NOT_IMPLEMENTED = 0;                                     \
            SUPPORTED_DNS_TYPE_NOT_IMPLEMENTED[0] = 1;                                       \
        }                                                                                    \
    } while(0)

///////////////////////
// Message Functions

internal DNS_Msg dns_msg_alloc(Arena *arena, String8 domain, DNS_Type type);
internal String8 dns_msg_header_to_str8(Arena *arena, DNS_Msg_Header h);

//////////////////////
// Client Functions

internal DNS_Client dns_client_alloc(Arena *arena, NET_AddressFamily family, DNS_TransportProtocol protocol);
internal void       dns_client_release(DNS_Client client);
internal DNS_Msg    dns_exchange(Arena *arena, DNS_Msg msg, DNS_TransportProtocol protocol, String8 target); // TODO
internal DNS_Msg    dns_client_exchange(Arena *arena, DNS_Client client, DNS_Msg msg, String8 target); // TODO
internal DNS_Msg    dns_client_exchange_with_address(Arena *arena, DNS_Client client, DNS_Msg msg, NET_Address address);

////////////////////////
// Server Functions

internal DNS_Server dns_server_alloc(Arena *arena, NET_AddressFamily family, DNS_TransportProtocol protocol); // TODO
internal void       dns_server_release(DNS_Server server);
internal void       dns_listen_and_serve(String8 address, DNS_TransportProtocol protocol); // TODO
internal void       dns_server_listen_and_serve(DNS_Server server);
internal void       dns_server_shutdown(DNS_Server *server); // TODO
internal void       dns_server_shutdown_and_release(DNS_Server *server); // TODO

///////////////////////
// Utility Functions

internal String8 str8_to_fqdn(Arena *arena, String8 s);
internal bool32  str8_is_fqdn(String8 s);
internal String8 str8_to_canonical(Arena *arena, String8 s);
internal String8 str8_to_name_labels(Arena *arena, String8 s);
internal bool32  str8_is_name_labels(String8 s);

//////////////////
// Wire Lengths

internal u64 dns_rdata_wire_length(DNS_RR *rr);
internal u64 dns_rr_wire_length(DNS_RR *rr);
internal u64 dns_msg_wire_length(DNS_Msg *msg);

//////////////////////////////
// DNS Diagnostic Functions

//~ fbt: This function pings each root server A-M and then attemps a single DNS query to each one
//       if servers respond to a ping, but NONE respond to our DNS query, then we figure
//       our local network blocks outbound DNS not headed for the local resolver.
internal bool32 dns_is_blocked_on_this_network(DNS_TransportProtocol protocol);

////////////////////////////////////////
// @per_os_impl Sytem DNS Info

internal String8_List dns_get_local_nameservers(Arena *arena);

#endif // DNS_CORE_H
