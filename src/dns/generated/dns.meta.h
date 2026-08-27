// Copyright (c) Epic Games Tools
// Licensed under the MIT license (https://opensource.org/license/mit/)

//- GENERATED CODE

#ifndef DNS_META_H
#define DNS_META_H

typedef enum DNS_Class
{
DNS_Class_IN           = 1,
DNS_Class_CS           = 2,
DNS_Class_CH           = 3,
DNS_Class_HS           = 4,
DNS_Class_NONE         = 254,
DNS_Class_ANY          = 255,
} DNS_Class;

typedef enum DNS_Type
{
DNS_Type_A            = 1,
DNS_Type_NS           = 2,
DNS_Type_MD           = 3,
DNS_Type_MF           = 4,
DNS_Type_CNAME        = 5,
DNS_Type_SOA          = 6,
DNS_Type_MB           = 7,
DNS_Type_MG           = 8,
DNS_Type_MR           = 9,
DNS_Type_NULL         = 10,
DNS_Type_WKS          = 11,
DNS_Type_PTR          = 12,
DNS_Type_HINFO        = 13,
DNS_Type_MINFO        = 14,
DNS_Type_MX           = 15,
DNS_Type_TXT          = 16,
DNS_Type_RP           = 17,
DNS_Type_AFSDB        = 18,
DNS_Type_X25          = 19,
DNS_Type_ISDN         = 20,
DNS_Type_RT           = 21,
DNS_Type_NSAP         = 22,
DNS_Type_NSAPPTR      = 23,
DNS_Type_SIG          = 24,
DNS_Type_KEY          = 25,
DNS_Type_PX           = 26,
DNS_Type_GPOS         = 27,
DNS_Type_AAAA         = 28,
DNS_Type_LOC          = 29,
DNS_Type_NXT          = 30,
DNS_Type_EID          = 31,
DNS_Type_NIMLOC       = 32,
DNS_Type_SRV          = 33,
DNS_Type_ATMA         = 34,
DNS_Type_NAPTR        = 35,
DNS_Type_KX           = 36,
DNS_Type_CERT         = 37,
DNS_Type_A6           = 38,
DNS_Type_DNAME        = 39,
DNS_Type_SINK         = 40,
DNS_Type_OPT          = 41,
DNS_Type_APL          = 42,
DNS_Type_DS           = 43,
DNS_Type_SSHFP        = 44,
DNS_Type_IPSECKEY     = 45,
DNS_Type_RRSIG        = 46,
DNS_Type_NSEC         = 47,
DNS_Type_DNSKEY       = 48,
DNS_Type_DHCID        = 49,
DNS_Type_NSEC3        = 50,
DNS_Type_NSEC3PARAM   = 51,
DNS_Type_TLSA         = 52,
DNS_Type_SMIMEA       = 53,
DNS_Type_HIP          = 55,
DNS_Type_NINFO        = 56,
DNS_Type_RKEY         = 57,
DNS_Type_TALINK       = 58,
DNS_Type_CDS          = 59,
DNS_Type_CDNSKEY      = 60,
DNS_Type_OPENPGPKEY   = 61,
DNS_Type_CSYNC        = 62,
DNS_Type_ZONEMD       = 63,
DNS_Type_SVCB         = 64,
DNS_Type_HTTPS        = 65,
DNS_Type_DSYNC        = 66,
DNS_Type_HHIT         = 67,
DNS_Type_BRID         = 68,
DNS_Type_SPF          = 99,
DNS_Type_UINFO        = 100,
DNS_Type_UID          = 101,
DNS_Type_GID          = 102,
DNS_Type_UNSPEC       = 103,
DNS_Type_NID          = 104,
DNS_Type_L32          = 105,
DNS_Type_L64          = 106,
DNS_Type_LP           = 107,
DNS_Type_EUI48        = 108,
DNS_Type_EUI64        = 109,
DNS_Type_NXNAME       = 128,
DNS_Type_TKEY         = 249,
DNS_Type_TSIG         = 250,
DNS_Type_IXFR         = 251,
DNS_Type_AXFR         = 252,
DNS_Type_MAILB        = 253,
DNS_Type_MAILA        = 254,
DNS_Type_ANY          = 255,
DNS_Type_URI          = 256,
DNS_Type_CAA          = 257,
DNS_Type_AVC          = 258,
DNS_Type_DOA          = 259,
DNS_Type_AMTRELAY     = 260,
DNS_Type_RESINFO      = 261,
DNS_Type_WALLET       = 262,
DNS_Type_CLA          = 263,
DNS_Type_IPN          = 264,
DNS_Type_TA           = 32768,
DNS_Type_DLV          = 32769,
} DNS_Type;

typedef enum DNS_OpCode
{
DNS_OpCode_Query        = 0,
DNS_OpCode_IQuery       = 1,
DNS_OpCode_Status       = 2,
DNS_OpCode_Notify       = 4,
DNS_OpCode_Update       = 5,
DNS_OpCode_DSO          = 6,
} DNS_OpCode;

typedef enum DNS_RCode
{
DNS_RCode_NoError      = 0,
DNS_RCode_FormErr      = 1,
DNS_RCode_ServFail     = 2,
DNS_RCode_NXDomain     = 3,
DNS_RCode_NotImp       = 4,
DNS_RCode_Refused      = 5,
DNS_RCode_YXDomain     = 6,
DNS_RCode_YXRRSet      = 7,
DNS_RCode_NXRRSet      = 8,
DNS_RCode_NotAuth      = 9,
DNS_RCode_NotZone      = 10,
DNS_RCode_DSOTYPENI    = 11,
DNS_RCode_BADSIG       = 16,
DNS_RCode_BADKEY       = 17,
DNS_RCode_BADTIME      = 18,
DNS_RCode_BADMODE      = 19,
DNS_RCode_BADNAME      = 20,
DNS_RCode_BADALG       = 21,
DNS_RCode_BADTRUNC     = 22,
DNS_RCode_BADCOOKIE    = 23,
} DNS_RCode;

typedef enum DNS_RootServer
{
DNS_RootServer_A,
DNS_RootServer_B,
DNS_RootServer_C,
DNS_RootServer_D,
DNS_RootServer_E,
DNS_RootServer_F,
DNS_RootServer_G,
DNS_RootServer_H,
DNS_RootServer_I,
DNS_RootServer_J,
DNS_RootServer_K,
DNS_RootServer_L,
DNS_RootServer_M,
DNS_RootServer_COUNT,
} DNS_RootServer;

typedef struct DNS_RData_A DNS_RData_A;
struct DNS_RData_A
{
u32 address;
};
typedef struct DNS_RData_NS DNS_RData_NS;
struct DNS_RData_NS
{
String8 nsdname;
};
typedef struct DNS_RData_MD DNS_RData_MD;
struct DNS_RData_MD
{
String8 madname;
};
typedef struct DNS_RData_MF DNS_RData_MF;
struct DNS_RData_MF
{
String8 madname;
};
typedef struct DNS_RData_CNAME DNS_RData_CNAME;
struct DNS_RData_CNAME
{
String8 cname;
};
typedef struct DNS_RData_SOA DNS_RData_SOA;
struct DNS_RData_SOA
{
String8 mname;
String8 rname;
u32 serial;
u32 refresh;
u32 retry;
u32 expire;
u32 minimum;
};
typedef struct DNS_RData_MB DNS_RData_MB;
struct DNS_RData_MB
{
String8 madname;
};
typedef struct DNS_RData_MG DNS_RData_MG;
struct DNS_RData_MG
{
String8 mgmname;
};
typedef struct DNS_RData_MR DNS_RData_MR;
struct DNS_RData_MR
{
String8 newname;
};
typedef struct DNS_RData_NULL DNS_RData_NULL;
struct DNS_RData_NULL
{
String8 data;
};
typedef struct DNS_RData_WKS DNS_RData_WKS;
struct DNS_RData_WKS
{
u32 address;
u8 protocol;
String8 bitmap;
};
typedef struct DNS_RData_PTR DNS_RData_PTR;
struct DNS_RData_PTR
{
String8 ptrdname;
};
typedef struct DNS_RData_HINFO DNS_RData_HINFO;
struct DNS_RData_HINFO
{
String8 cpu;
String8 os;
};
typedef struct DNS_RData_MINFO DNS_RData_MINFO;
struct DNS_RData_MINFO
{
String8 rmailbx;
String8 emailbx;
};
typedef struct DNS_RData_MX DNS_RData_MX;
struct DNS_RData_MX
{
u16 preference;
String8 exchange;
};
typedef struct DNS_RData_TXT DNS_RData_TXT;
struct DNS_RData_TXT
{
String8 txt_data;
};
typedef struct DNS_RData_RP DNS_RData_RP;
struct DNS_RData_RP
{
String8 mbox_dname;
String8 txt_dname;
};
typedef struct DNS_RData_AFSDB DNS_RData_AFSDB;
struct DNS_RData_AFSDB
{
u16 subtype;
String8 hostname;
};
typedef struct DNS_RData_X25 DNS_RData_X25;
struct DNS_RData_X25
{
String8 psdn_addr;
};
typedef struct DNS_RData_ISDN DNS_RData_ISDN;
struct DNS_RData_ISDN
{
String8 isdn_addr;
String8 sub_addr;
};
typedef struct DNS_RData_RT DNS_RData_RT;
struct DNS_RData_RT
{
u16 preference;
String8 intrm_host;
};
typedef struct DNS_RData_NSAP DNS_RData_NSAP;
struct DNS_RData_NSAP
{
String8 data;
};
typedef struct DNS_RData_NSAPPTR DNS_RData_NSAPPTR;
struct DNS_RData_NSAPPTR
{
String8 ptrdname;
};
typedef struct DNS_RData_SIG DNS_RData_SIG;
struct DNS_RData_SIG
{
u16 type_covered;
u8 algorithm;
u8 labels;
u32 ttl;
u32 sig_expire;
u32 time_signd;
u16 key_ftprnt;
String8 signr_name;
String8 signature;
};
typedef struct DNS_RData_KEY DNS_RData_KEY;
struct DNS_RData_KEY
{
u16 flags;
u8 protocol;
u8 algorithm;
String8 pubkey;
};
typedef struct DNS_RData_PX DNS_RData_PX;
struct DNS_RData_PX
{
u16 preference;
String8 map_822;
String8 map_x400;
};
typedef struct DNS_RData_GPOS DNS_RData_GPOS;
struct DNS_RData_GPOS
{
String8 longitude;
String8 latitude;
String8 altitude;
};
typedef struct DNS_RData_AAAA DNS_RData_AAAA;
struct DNS_RData_AAAA
{
u128 address;
};
typedef struct DNS_RData_LOC DNS_RData_LOC;
struct DNS_RData_LOC
{
u8 version;
u8 size;
u8 horiz_pre;
u8 vert_pre;
u32 latitude;
u32 longitude;
u32 altitude;
};
typedef struct DNS_RData_NXT DNS_RData_NXT;
struct DNS_RData_NXT
{
String8 next_dname;
String8 type_bitmap;
};
typedef struct DNS_RData_EID DNS_RData_EID;
struct DNS_RData_EID
{
String8 endpoint_id;
};
typedef struct DNS_RData_NIMLOC DNS_RData_NIMLOC;
struct DNS_RData_NIMLOC
{
String8 nimrod_loc;
};
typedef struct DNS_RData_SRV DNS_RData_SRV;
struct DNS_RData_SRV
{
u16 priority;
u16 weight;
u16 port;
String8 target;
};
typedef struct DNS_RData_ATMA DNS_RData_ATMA;
struct DNS_RData_ATMA
{
String8 mctx;
String8 format;
String8 atma;
u16 atma_len;
};
typedef struct DNS_RData_NAPTR DNS_RData_NAPTR;
struct DNS_RData_NAPTR
{
u16 order;
u16 preference;
String8 flags;
String8 service;
String8 regexp;
String8 replacement;
};
typedef struct DNS_RData_KX DNS_RData_KX;
struct DNS_RData_KX
{
u16 preference;
String8 exchanger;
};
typedef struct DNS_RData_CERT DNS_RData_CERT;
struct DNS_RData_CERT
{
u16 type;
u16 key_tag;
u8 algorithm;
String8 certificate;
};
typedef struct DNS_RData_A6 DNS_RData_A6;
struct DNS_RData_A6
{
u8 prefix_len;
String8 addr_suffx;
String8 prefix_name;
};
typedef struct DNS_RData_DNAME DNS_RData_DNAME;
struct DNS_RData_DNAME
{
String8 target;
};
typedef struct DNS_RData_SINK DNS_RData_SINK;
struct DNS_RData_SINK
{
u8 coding;
u8 subcoding;
String8 data;
};
typedef struct DNS_RData_OPT DNS_RData_OPT;
struct DNS_RData_OPT
{
String8 data;
};
typedef struct DNS_RData_APL DNS_RData_APL;
struct DNS_RData_APL
{
u16 addr_fam;
u8 prefix;
bool32 n;
u8 afdlength;
union{u32 v4; u128 v6;} afdpart;
};
typedef struct DNS_RData_DS DNS_RData_DS;
struct DNS_RData_DS
{
u16 key_tag;
u8 algorithm;
u8 digest_t;
String8 digest;
};
typedef struct DNS_RData_SSHFP DNS_RData_SSHFP;
struct DNS_RData_SSHFP
{
u8 algorithm;
u8 fp_type;
String8 fingerprint;
};
typedef struct DNS_RData_IPSECKEY DNS_RData_IPSECKEY;
struct DNS_RData_IPSECKEY
{
u8 precedence;
u8 gateway_type;
u8 algorithm;
union{u32 v4; u128 v6; String8 dname;} gateway;
String8 public_key;
};
typedef struct DNS_RData_RRSIG DNS_RData_RRSIG;
struct DNS_RData_RRSIG
{
u16 type_covered;
u8 algorithm;
u8 labels;
u32 orig_ttl;
u32 expiration;
u32 inception;
u16 key_tag;
String8 signr_name;
String8 signature;
};
typedef struct DNS_RData_NSEC DNS_RData_NSEC;
struct DNS_RData_NSEC
{
String8 nxt_domain;
String8 type_bitmap;
};
typedef struct DNS_RData_DNSKEY DNS_RData_DNSKEY;
struct DNS_RData_DNSKEY
{
u16 flags;
u8 protocol;
u8 algorithm;
String8 public_key;
};
typedef struct DNS_RData_DHCID DNS_RData_DHCID;
struct DNS_RData_DHCID
{
u16 id_type_code;
u8 digest_type_code;
String8 digest;
};
typedef struct DNS_RData_NSEC3 DNS_RData_NSEC3;
struct DNS_RData_NSEC3
{
u8 hash;
u8 flags;
u16 iterations;
u8 salt_len;
String8 salt;
u8 hash_len;
String8 nxt_domain;
String8 type_bitmap;
};
typedef struct DNS_RData_NSEC3PARAM DNS_RData_NSEC3PARAM;
struct DNS_RData_NSEC3PARAM
{
u8 hash;
u8 flags;
u16 iterations;
u8 salt_len;
String8 salt;
};
typedef struct DNS_RData_TLSA DNS_RData_TLSA;
struct DNS_RData_TLSA
{
u8 usage;
u8 selector;
u8 matching_t;
String8 certificate;
};
typedef struct DNS_RData_SMIMEA DNS_RData_SMIMEA;
struct DNS_RData_SMIMEA
{
u8 usage;
u8 selector;
u8 matching_t;
String8 certificate;
};
typedef struct DNS_RData_HIP DNS_RData_HIP;
struct DNS_RData_HIP
{
u8 hit_len;
u8 pub_key_alg;
u16 pub_key_len;
String8 hit;
String8 pub_key;
String8 rendezvous_servers;
};
typedef struct DNS_RData_NINFO DNS_RData_NINFO;
struct DNS_RData_NINFO
{
String8 zsdata;
};
typedef struct DNS_RData_RKEY DNS_RData_RKEY;
struct DNS_RData_RKEY
{
u16 flags;
u8 protocol;
u8 algorithm;
String8 pub_key;
};
typedef struct DNS_RData_TALINK DNS_RData_TALINK;
struct DNS_RData_TALINK
{
String8 prev_name;
String8 next_name;
};
typedef struct DNS_RData_CDS DNS_RData_CDS;
struct DNS_RData_CDS
{
u16 key_tag;
u8 algorithm;
u8 digest_t;
String8 digest;
};
typedef struct DNS_RData_CDNSKEY DNS_RData_CDNSKEY;
struct DNS_RData_CDNSKEY
{
u16 flags;
u8 protocol;
u8 algorithm;
String8 pub_key;
};
typedef struct DNS_RData_OPENPGPKEY DNS_RData_OPENPGPKEY;
struct DNS_RData_OPENPGPKEY
{
String8 pub_key;
};
typedef struct DNS_RData_CSYNC DNS_RData_CSYNC;
struct DNS_RData_CSYNC
{
u32 serial;
u16 flags;
String8 type_bitmap;
};
typedef struct DNS_RData_ZONEMD DNS_RData_ZONEMD;
struct DNS_RData_ZONEMD
{
u32 serial;
u8 scheme;
u8 hash;
String8 digest;
};
typedef struct DNS_RData_SVCB DNS_RData_SVCB;
struct DNS_RData_SVCB
{
u16 priority;
String8 target;
String8 pairs;
};
typedef struct DNS_RData_HTTPS DNS_RData_HTTPS;
struct DNS_RData_HTTPS
{
u16 priority;
String8 target;
String8 pairs;
};
typedef struct DNS_RData_DSYNC DNS_RData_DSYNC;
struct DNS_RData_DSYNC
{
u16 type;
u8 scheme;
u16 port;
String8 target;
};
typedef struct DNS_RData_HHIT DNS_RData_HHIT;
struct DNS_RData_HHIT
{
u128 prefix;
u32 hid;
u8 hssi;
String8 orchid_hash;
};
typedef struct DNS_RData_BRID DNS_RData_BRID;
struct DNS_RData_BRID
{
String8 UNKNOWN;
};
typedef struct DNS_RData_SPF DNS_RData_SPF;
struct DNS_RData_SPF
{
String8 text;
};
typedef struct DNS_RData_UINFO DNS_RData_UINFO;
struct DNS_RData_UINFO
{
String8 uinfo;
};
typedef struct DNS_RData_UID DNS_RData_UID;
struct DNS_RData_UID
{
u32 uid;
};
typedef struct DNS_RData_GID DNS_RData_GID;
struct DNS_RData_GID
{
u32 gid;
};
typedef struct DNS_RData_UNSPEC DNS_RData_UNSPEC;
struct DNS_RData_UNSPEC
{
String8 UNKNOWN;
};
typedef struct DNS_RData_NID DNS_RData_NID;
struct DNS_RData_NID
{
u16 preference;
u64 node_id;
};
typedef struct DNS_RData_L32 DNS_RData_L32;
struct DNS_RData_L32
{
u16 preference;
u32 locator_32;
};
typedef struct DNS_RData_L64 DNS_RData_L64;
struct DNS_RData_L64
{
u16 preference;
u64 locator_64;
};
typedef struct DNS_RData_LP DNS_RData_LP;
struct DNS_RData_LP
{
u16 preference;
String8 fqdn;
};
typedef struct DNS_RData_EUI48 DNS_RData_EUI48;
struct DNS_RData_EUI48
{
String8 address;
};
typedef struct DNS_RData_EUI64 DNS_RData_EUI64;
struct DNS_RData_EUI64
{
String8 address;
};
typedef struct DNS_RData_NXNAME DNS_RData_NXNAME;
struct DNS_RData_NXNAME
{
String8 EMPTY;
};
typedef struct DNS_RData_TKEY DNS_RData_TKEY;
struct DNS_RData_TKEY
{
String8 algorithm;
u32 inception;
u32 expiration;
u16 mode;
u16 error;
u16 key_size;
String8 key;
u16 other_len;
String8 other_data;
};
typedef struct DNS_RData_TSIG DNS_RData_TSIG;
struct DNS_RData_TSIG
{
String8 algorithm;
u64 time_signed;
u16 fudge;
u16 mac_size;
String8 mac;
u16 orig_id;
u16 error;
u16 other_len;
String8 other_data;
};
typedef struct DNS_RData_IXFR DNS_RData_IXFR;
struct DNS_RData_IXFR
{
String8 EMPTY;
};
typedef struct DNS_RData_AXFR DNS_RData_AXFR;
struct DNS_RData_AXFR
{
String8 EMPTY;
};
typedef struct DNS_RData_MAILB DNS_RData_MAILB;
struct DNS_RData_MAILB
{
String8 EMPTY;
};
typedef struct DNS_RData_MAILA DNS_RData_MAILA;
struct DNS_RData_MAILA
{
String8 EMPTY;
};
typedef struct DNS_RData_ANY DNS_RData_ANY;
struct DNS_RData_ANY
{
String8 EMPTY;
};
typedef struct DNS_RData_URI DNS_RData_URI;
struct DNS_RData_URI
{
u16 priority;
u16 weight;
String8 target;
};
typedef struct DNS_RData_CAA DNS_RData_CAA;
struct DNS_RData_CAA
{
u8 flag;
String8 tag;
String8 value;
};
typedef struct DNS_RData_AVC DNS_RData_AVC;
struct DNS_RData_AVC
{
String8 text;
};
typedef struct DNS_RData_DOA DNS_RData_DOA;
struct DNS_RData_DOA
{
String8 UNKNOWN;
};
typedef struct DNS_RData_AMTRELAY DNS_RData_AMTRELAY;
struct DNS_RData_AMTRELAY
{
String8 UNKOWN;
};
typedef struct DNS_RData_RESINFO DNS_RData_RESINFO;
struct DNS_RData_RESINFO
{
String8 text;
};
typedef struct DNS_RData_WALLET DNS_RData_WALLET;
struct DNS_RData_WALLET
{
String8 text;
};
typedef struct DNS_RData_CLA DNS_RData_CLA;
struct DNS_RData_CLA
{
String8 text;
};
typedef struct DNS_RData_IPN DNS_RData_IPN;
struct DNS_RData_IPN
{
u64 node;
};
typedef struct DNS_RData_TA DNS_RData_TA;
struct DNS_RData_TA
{
u16 key_tag;
u8 algorithm;
u8 digest_t;
String8 digest;
};
typedef struct DNS_RData_DLV DNS_RData_DLV;
struct DNS_RData_DLV
{
u16 key_tag;
u8 algorithm;
u8 digest_t;
String8 digest;
};
internal String8 dns_string_from_class(DNS_Class v);
internal String8 dns_friendly_name_from_class(DNS_Class v);
internal String8 dns_rfc_references_from_class(DNS_Class v);
internal String8 dns_string_from_type(DNS_Type v);
internal String8 dns_string_from_opcode(DNS_OpCode v);
internal String8 dns_string_from_rcode(DNS_RCode v);
internal String8 dns_dname_of_rootserver(DNS_RootServer v);
internal String8 dns_ipv4_string_of_rootserver(DNS_RootServer v);
internal String8 dns_ipv6_string_of_rootserver(DNS_RootServer v);
internal String8 dns_operator_of_rootserver(DNS_RootServer v);
internal bool32 dns_is_obsolete_class(DNS_Class v);
internal bool32 dns_is_query_exclusive_class(DNS_Class v);
internal bool32 dns_is_obsolete_type(DNS_Type v);
internal bool32 dns_is_query_exclusive_type(DNS_Type v);
#endif // DNS_META_H
