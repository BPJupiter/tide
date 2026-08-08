// This DNS implementation takes inspiration from
// https://codeberg.org/miekg/dns where I thought
// appropriate!

#ifndef DNS_CORE_H
#define DNS_CORE_H

#define DNS_MAX_LABEL_LEN 63
#define DNS_MAX_NAME_LEN  255
#define DNS_MAX_RDATA_LEN (Kilobytes(4096)) // rfc 6891

typedef enum Dns_RRType {
    /* RESERVED       =   0,    rfc 6895 */
    Dns_RRType_A      =   1, /* rfc 1035 */
    Dns_RRType_NS     =   2, /* rfc 1035 */
    Dns_RRType_MD     =   3, /* OBSOLETE: rfc 883, 973, 1035 */
    Dns_RRType_MF     =   4, /* OBSOLETE: rfc 883, 973, 1035 */
    Dns_RRType_CNAME  =   5, /* rfc 1035 */
    Dns_RRType_SOA    =   6, /* rfc 1035, 2308 */
    Dns_RRType_MB     =   7, /* EXPERIMENTAL: rfc 883, 1035, 2505 */
    Dns_RRType_MG     =   8, /* EXPERIMENTAL: rfc 883, 1035, 2505 */
    Dns_RRType_MR     =   9, /* EXPERIMENTAL: rfc 883, 1035, 2505 */
    Dns_RRType_NULL   =  10, /* EXPERIMENTAL: rfc 883, 1035 */
    Dns_RRType_WKS    =  11, /* OBSOLETE: rfc 883, 1035, 1123, 1127 */
    Dns_RRType_PTR    =  12, /* rfc 1035 */
    Dns_RRType_HINFO  =  13, /* rfc 883, 1035, 8482 */
    Dns_RRType_MINFO  =  14, /* EXPERIMENTAL: rfc 883, 1035, 2505 */
    Dns_RRType_MX     =  15, /* rfc 1035, 7505 */
    Dns_RRType_TXT    =  16, /* rfc 1035 */
    Dns_RRType_RP     =  17, /* OBSOLETE: rfc 1183 */
    Dns_RRType_AFSDB  =  18, /* rfc 1183 */
    Dns_RRType_X25    =  19, /* OBSOLETE: rfc 1183 */
    Dns_RRType_ISDN   =  20, /* OBSOLETE: rfc 1183 */
    Dns_RRType_RT     =  21, /* OBSOLETE: rfc 1183 */
    Dns_RRType_NSAP   =  22, /* OBSOLETE: rfc 1706 */
    Dns_RRType_NSAPPTR = 23, /* OBSOLETE: rfc 1706 */
    Dns_RRType_SIG    =  24, /* OBSOLETE: rfc 2065, 2535, 3755 */
    Dns_RRType_KEY    =  25, /* OBSOLETE: rfc 2065, 2535, 2930, 3755 */
    Dns_RRType_PX     =  26, /* OBSOLETE: rfc 2163 */
    Dns_RRType_GPOS   =  27, /* OBSOLETE: rfc 1712 */
    Dns_RRType_AAAA   =  28, /* rfc 3596 */
    Dns_RRType_LOC    =  29, /* rfc 1876 */
    Dns_RRType_NXT    =  30, /* OBSOLETE: rfc 2065, 3755 */
    Dns_RRType_EID    =  31, /* OBSOLETE: - */
    Dns_RRType_NIMLOC =  32, /* OBSOLETE: - */
    Dns_RRType_NB     =  32, /* OBSOLETE: rfc 1002 */
    Dns_RRType_NBSTAT =  33, /* OBSOLETE: rfc 1002 */
    Dns_RRType_SRV    =  33, /* rfc 2782 */
    Dns_RRType_ATMA   =  34, /* OBSOLETE: - */
    Dns_RRType_NAPTR  =  35, /* rfc 3403 */
    Dns_RRType_KX     =  36, /* rfc 2230 */
    Dns_RRType_CERT   =  37, /* rfc 4398 */
    Dns_RRType_A6     =  38, /* OBSOLETE: rfc 2874, 3363, 6563 */
    Dns_RRType_DNAME  =  39, /* rfc 6672 */
    Dns_RRType_SINK   =  40, /* OBSOLETE: - */
    Dns_RRType_OPT    =  41, /* PSEUDO: rfc 3225, 6891 */
    Dns_RRType_APL    =  42, /* EXPERIMENTAL: rfc 3123 */
    Dns_RRType_DS     =  43, /* rfc 4034 */
    Dns_RRType_SSHFP  =  44, /* rfc 4255 */
    Dns_RRType_IPSECKEY = 45, /* rfc 4025 */
    Dns_RRType_RRSIG  =  46, /* rfc 4034 */
    Dns_RRType_NSEC   =  47, /* rfc 4034 */
    Dns_RRType_DNSKEY =  48, /* rfc 4034 */
    Dns_RRType_DHCID  =  49, /* rfc 4701 */
    Dns_RRType_NSEC3  =  50, /* rfc 5155 */
    Dns_RRType_NSEC3PARAM = 51, /* rfc 5155 */
    Dns_RRType_TLSA   =  52, /* rfc 6698 */
    Dns_RRType_SMIMEA =  53, /* rfc 8162 */
    /* UNASSIGNED     =  54,             */
    Dns_RRType_HIP    =  55, /* rfc 8005 */
    Dns_RRType_NINFO  =  56, /* OBSOLETE: - */
    Dns_RRType_RKEY   =  57, /* OBSOLETE: - */
    Dns_RRType_TALINK =  58, /* OBSOLETE: - */
    Dns_RRType_CDS    =  59, /* rfc 7344 */
    Dns_RRType_CDNSKEY = 60, /* rfc 7344 */
    Dns_RRType_OPENPGPKEY = 61, /* rfc 7929 */
    Dns_RRType_CSYNC  =  62, /* rfc 7477 */
    Dns_RRType_ZONEMD =  63, /* rfc 8976 */
    Dns_RRType_SVCB   =  64, /* rfc 9460 */
    Dns_RRType_HTTPS  =  65, /* rfc 9460 */
    Dns_RRType_DSYNC  =  66, /* rfc 9859 */
    Dns_RRType_HHIT   =  67, /* rfc 9886 */
    Dns_RRType_BRID   =  68, /* rfc 9886 */
    /* UNASSIGNED     =  69,
                      =  98,             */
    Dns_RRType_SPF    =  99, /* OBSOLETE: rfc 4408, 7208 */
    Dns_RRType_UINFO  = 100, /* OBSOLETE: - */
    Dns_RRType_UID    = 101, /* OBSOLETE: - */
    Dns_RRType_GID    = 102, /* OBSOLETE: - */
    Dns_RRType_UNSPEC = 103, /* OBSOLETE: - */
    Dns_RRType_NID    = 104, /* EXPERIMENTAL: rfc 6742 */
    Dns_RRType_L32    = 105, /* EXPERIMENTAL: rfc 6742 */
    Dns_RRType_L64    = 106, /* EXPERIMENTAL: rfc 6742 */
    Dns_RRType_LP     = 107, /* EXPERIMENTAL: rfc 6742 */
    Dns_RRType_EUI48  = 108, /* rfc 7043 */
    Dns_RRType_EUI64  = 109, /* rfc 7043 */
    /* UNASSIGNED     = 110,
                      = 127,             */
    Dns_RRType_NXNAME = 128, /* rfc 9824 */
    /* UNASSIGNED     = 129,
                      = 248,             */
    Dns_RRType_TKEY   = 249, /* rfc 2930 */
    Dns_RRType_TSIG   = 250, /* rfc 2845 */
    Dns_QRRType_IXFR  = 251, /* rfc 1996 */
    Dns_QRRType_AXFR  = 252, /* rfc 1035 */
    Dns_QRRType_MAILB = 253, /* EXPERIMENTAL: rfc 883, 1035, 2505 */
    Dns_QRRType_MAILA = 254, /* OBSOLETE: rfc 883, 973, 1035 */
    Dns_QRRType_ANY   = 255, /* rfc 1035 */
    Dns_RRType_URI    = 256, /* rfc 7553 */
    Dns_RRType_CAA    = 257, /* rfc 8659 */
    Dns_RRType_DOA    = 259, /* OBSOLETE: - */
    Dns_RRType_AMTRELAY = 260, /* rfc 8777 */
    Dns_RRType_RESINFO = 261, /* rfc 9606 */
    Dns_RRType_WALLET = 262, /* https://www.iana.org/assignments/dns-parameters/WALLET/wallet-completed-template */
    Dns_RRType_CLA    = 263, /* - */
    Dns_RRType_IPN    = 264, /* - */
    /* UNASSIGNED     = 265,
                      = 32767       */
    Dns_RRType_TA     = 32768, /* - */
    Dns_RRType_DLV    = 32769, /* OBSOLETE: rfc 4431, 8749 */
    /* UNASSIGNED     = 32770,
                      = 65279,      */
    /* PRIVATE        = 65280,
                      = 65534,      */
    /* RESERVED       = 65535       */
} Dns_Type;

bool32 g_supported_dns_types[1ULL << 16] = {
    [Dns_RRType_A]     = true,
    [Dns_RRType_NS]    = true,
    [Dns_RRType_CNAME] = true,
    [Dns_RRType_AAAA]  = true,
};

typedef enum Dns_Class {
    /* RESERVED         =   0,    rfc 6895 */
    Dns_Class_INET      =   1, /* rfc 1035 */
    Dns_Class_CSNET     =   2, /* OBSOLETE: rfc 1035 */
    Dns_Class_CHAOS     =   3, /* rfc 1035 */
    Dns_Class_HESIOD    =   4, /* rfc 1035 */
    /* UNASSIGNED       =   5,
                        = 253,             */
    Dns_QClass_NONE     = 254, /* rfc 2136 */
    Dns_QClass_ANY      = 255  /* rfc 1035 */
    /* UNASSIGNED       = 256,
                        = 65279,           */
    /* PRIVATE          = 65280,
                        = 65534,           */
    /* RESERVED         = 65535            */
} Dns_Class;

typedef enum Dns_OpCode {
    Dns_OpCode_Quqery    = 0, /* rfc 1035 */
    Dns_OpCode_IQuery   = 1, /* rfc 1035 */
    Dns_OpCode_Status   = 2, /* rfc 1035 */
    /* UNASSIGNED       = 3,             */
    Dns_OpCode_Notify   = 4, /* rfc 1996 */
    Dns_OpCode_Update   = 5, /* rfc 2136 */
    Dns_OpCode_Stateful = 6  /* rfc 8490 */
} Dns_OpCode;


typedef enum Dns_RCode {
    Dns_RCode_Success           =   0, /* rfc 1035 */
    Dns_RCode_FormatError       =   1, /* rfc 1035 */
    Dns_RCode_ServerFailure     =   2, /* rfc 1035 */
    Dns_RCode_NameError         =   3, /* rfc 1035 */
    Dns_RCode_NotImplemented    =   4, /* rfc 1035 */
    Dns_RCode_Refused           =   5, /* rfc 1035 */
    Dns_RCode_YXDomain          =   6, /* rfc 2136, 6672 */
    Dns_RCode_YXRRSet           =   7, /* rfc 2136 */
    Dns_RCode_NXRRSet           =   8, /* rfc 2136 */
    Dns_RCode_NotAuth           =   9, /* rfc 2136, 8945 */
    Dns_RCode_NotZone           =  10, /* rfc 2136 */
    Dns_RCode_StatefulNotImplemented =  11, /* rfc 8490 */
    /* UNASSIGNED       =  12,
                        =  15,             */
    Dns_RCode_BadVersion        =  16, /* rfc 6891 */
    Dns_RCode_BadSig            =  16, /* rfc 8945 */
    Dns_RCode_BadKey            =  17, /* rfc 8945 */
    Dns_RCode_BadTime           =  18, /* rfc 8945 */
    Dns_RCode_BadMode           =  19, /* rfc 2930 */
    Dns_RCode_BadName           =  20, /* rfc 2930 */
    Dns_RCode_BadAlg            =  21, /* rfc 2930 */
    Dns_RCode_BadTrunc          =  22, /* rfc 8945 */
    Dns_RCode_BadCookie         =  23  /* rfc 7873 */
    /* UNASSIGNED       =  24,
                       = 3840,             */
    /* PRIVATE         = 3841,
                       = 4095,             */
    /* UNASSIGNED      = 4096,
                      = 65534,             */
    /* RESERVED       = 65535              */
} Dns_RCode;

typedef struct Dns_RR_Header {
    String8 name;
    u32 ttl;
    Dns_Class class;
} Dns_RR_Header;

typedef struct Dns_RR {
    Dns_RR *next;
    Dns_RR_Header header;
    Dns_RRType type;
    union
    {
        u32 A;
        String8 NS;
        String8 CNAME;
        u128 AAAA;
    } rdata;
} Dns_RR;

typedef struct Dns_Msg_Header {
    u16 id;
    
    bool32 query_response;
    Dns_OpCode opcode;
    bool32 authoritative;
    bool32 truncated;
    bool32 recursion_desired;
    bool32 recursion_available;
    bool32 zero;
    bool32 authenticated_data;
    bool32 checking_disabled;
    Dns_RCode rcode;
    
    u16 answer_count;
    u16 nameserver_count;
    u16 additional_count;
} Dns_Msg_Header;

typedef struct Dns_Msg {
    Dns_Msg_Header header;
    Dns_RR question;
    Dns_RR *answer;
    Dns_RR *ns;
    Dns_RR *extra;
    u8 *wire;
} Dns_Msg;

///////////////////////
// Utility Functions

internal String8 str8_to_fqdn(Arena *arena, String8 s);
internal bool32  str8_is_fqdn(String8 s);
internal String8 str8_to_canonical(Arena *arena, String8 s);
internal bool32  str8_is_domain_name(String8 s);

#endif // DNS_CORE_H
