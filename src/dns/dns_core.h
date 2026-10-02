// Copyright (c) Frances Telfar
// Licensed under the PolyForm Noncommercial License (https://polyformproject.org/licenses/noncommercial/1.0.0)

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
    [DNS_Type_PTR]   = true,
    [DNS_Type_AAAA]  = true,
};

////////////////////////
// Message Structures

typedef struct DNS_RR DNS_RR;
struct DNS_RR {
    String8   name;
    DNS_Type  type;
    DNS_Class class;
    u32       ttl;

    DNS_RData rdata;
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

/////////////////////////
// Server Structures

typedef struct DNS_Session DNS_Session;
struct DNS_Session {
    NET_Session net_session;
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

internal DNS_Msg dns_msg_make(Arena *arena, String8 domain, DNS_Type type);
internal String8 dns_string_from_msg_header(Arena *arena, DNS_Msg_Header h);

//////////////////
// Wire Lengths

internal u64 dns_rdata_wire_length(DNS_RR *rr);
internal u64 dns_rr_wire_length(DNS_RR *rr);
internal u64 dns_msg_wire_length(DNS_Msg *msg);

////////////////////////////
// Wire Packing/Unpacking

internal String8 dns_pack_msg     (Arena *arena, DNS_Msg msg, bool32 pack_tcp_length);

internal String8 dns_unpack_labels(Arena *arena, String8 wire, u64 *cursor);
internal DNS_Msg dns_unpack_msg     (Arena *arena, String8 wire, bool32 unpack_tcp_length);

///////////////////////
// Utility Functions

internal String8 dns_fqdn_from_string(Arena *arena, String8 s);
internal bool32  dns_string_is_fqdn(String8 s);
internal String8 dns_canonical_from_string(Arena *arena, String8 s);
internal String8 dns_name_labels_from_string(Arena *arena, String8 s);
internal bool32  dns_string_is_name_labels(String8 s);

internal String8 dns_inverse_query_name_from_endpoint(Arena *arena, NET_Endpoint endpoint);

//////////////////////////////
// Network Diagnosis

internal bool32 dns_is_blocked_on_this_network(void);

////////////////////////////////////////
// @per_os_impl Sytem DNS Info

internal String8_List dns_get_local_nameservers(Arena *arena);

#endif // DNS_CORE_H
