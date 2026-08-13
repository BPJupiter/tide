// This DNS implementation takes inspiration from
// https://codeberg.org/miekg/dns where I thought
// appropriate!

#ifndef DNS_CORE_H
#define DNS_CORE_H

#define DNS_MAX_LABEL_LEN 63
#define DNS_MAX_NAME_LEN  255
#define DNS_MAX_RDATA_LEN (Kilobytes(4096)) // rfc 6891

typedef enum Dns_Type Dns_Type;
enum Dns_Type {
    /* RESERVED       =   0,    rfc 6895 */
    Dns_Type_A      =   1, /* rfc 1035 */
    Dns_Type_NS     =   2, /* rfc 1035 */
    Dns_Type_MD     =   3, /* OBSOLETE: rfc 883, 973, 1035 */
    Dns_Type_MF     =   4, /* OBSOLETE: rfc 883, 973, 1035 */
    Dns_Type_CNAME  =   5, /* rfc 1035 */
    Dns_Type_SOA    =   6, /* rfc 1035, 2308 */
    Dns_Type_MB     =   7, /* EXPERIMENTAL: rfc 883, 1035, 2505 */
    Dns_Type_MG     =   8, /* EXPERIMENTAL: rfc 883, 1035, 2505 */
    Dns_Type_MR     =   9, /* EXPERIMENTAL: rfc 883, 1035, 2505 */
    Dns_Type_NULL   =  10, /* EXPERIMENTAL: rfc 883, 1035 */
    Dns_Type_WKS    =  11, /* OBSOLETE: rfc 883, 1035, 1123, 1127 */
    Dns_Type_PTR    =  12, /* rfc 1035 */
    Dns_Type_HINFO  =  13, /* rfc 883, 1035, 8482 */
    Dns_Type_MINFO  =  14, /* EXPERIMENTAL: rfc 883, 1035, 2505 */
    Dns_Type_MX     =  15, /* rfc 1035, 7505 */
    Dns_Type_TXT    =  16, /* rfc 1035 */
    Dns_Type_RP     =  17, /* OBSOLETE: rfc 1183 */
    Dns_Type_AFSDB  =  18, /* rfc 1183 */
    Dns_Type_X25    =  19, /* OBSOLETE: rfc 1183 */
    Dns_Type_ISDN   =  20, /* OBSOLETE: rfc 1183 */
    Dns_Type_RT     =  21, /* OBSOLETE: rfc 1183 */
    Dns_Type_NSAP   =  22, /* OBSOLETE: rfc 1706 */
    Dns_Type_NSAPPTR = 23, /* OBSOLETE: rfc 1706 */
    Dns_Type_SIG    =  24, /* OBSOLETE: rfc 2065, 2535, 3755 */
    Dns_Type_KEY    =  25, /* OBSOLETE: rfc 2065, 2535, 2930, 3755 */
    Dns_Type_PX     =  26, /* OBSOLETE: rfc 2163 */
    Dns_Type_GPOS   =  27, /* OBSOLETE: rfc 1712 */
    Dns_Type_AAAA   =  28, /* rfc 3596 */
    Dns_Type_LOC    =  29, /* rfc 1876 */
    Dns_Type_NXT    =  30, /* OBSOLETE: rfc 2065, 3755 */
    Dns_Type_EID    =  31, /* OBSOLETE: - */
    Dns_Type_NIMLOC =  32, /* OBSOLETE: - */
    Dns_Type_NB     =  32, /* OBSOLETE: rfc 1002 */
    Dns_Type_NBSTAT =  33, /* OBSOLETE: rfc 1002 */
    Dns_Type_SRV    =  33, /* rfc 2782 */
    Dns_Type_ATMA   =  34, /* OBSOLETE: - */
    Dns_Type_NAPTR  =  35, /* rfc 3403 */
    Dns_Type_KX     =  36, /* rfc 2230 */
    Dns_Type_CERT   =  37, /* rfc 4398 */
    Dns_Type_A6     =  38, /* OBSOLETE: rfc 2874, 3363, 6563 */
    Dns_Type_DNAME  =  39, /* rfc 6672 */
    Dns_Type_SINK   =  40, /* OBSOLETE: - */
    Dns_Type_OPT    =  41, /* PSEUDO: rfc 3225, 6891 */
    Dns_Type_APL    =  42, /* EXPERIMENTAL: rfc 3123 */
    Dns_Type_DS     =  43, /* rfc 4034 */
    Dns_Type_SSHFP  =  44, /* rfc 4255 */
    Dns_Type_IPSECKEY = 45, /* rfc 4025 */
    Dns_Type_RRSIG  =  46, /* rfc 4034 */
    Dns_Type_NSEC   =  47, /* rfc 4034 */
    Dns_Type_DNSKEY =  48, /* rfc 4034 */
    Dns_Type_DHCID  =  49, /* rfc 4701 */
    Dns_Type_NSEC3  =  50, /* rfc 5155 */
    Dns_Type_NSEC3PARAM = 51, /* rfc 5155 */
    Dns_Type_TLSA   =  52, /* rfc 6698 */
    Dns_Type_SMIMEA =  53, /* rfc 8162 */
    /* UNASSIGNED     =  54,             */
    Dns_Type_HIP    =  55, /* rfc 8005 */
    Dns_Type_NINFO  =  56, /* OBSOLETE: - */
    Dns_Type_RKEY   =  57, /* OBSOLETE: - */
    Dns_Type_TALINK =  58, /* OBSOLETE: - */
    Dns_Type_CDS    =  59, /* rfc 7344 */
    Dns_Type_CDNSKEY = 60, /* rfc 7344 */
    Dns_Type_OPENPGPKEY = 61, /* rfc 7929 */
    Dns_Type_CSYNC  =  62, /* rfc 7477 */
    Dns_Type_ZONEMD =  63, /* rfc 8976 */
    Dns_Type_SVCB   =  64, /* rfc 9460 */
    Dns_Type_HTTPS  =  65, /* rfc 9460 */
    Dns_Type_DSYNC  =  66, /* rfc 9859 */
    Dns_Type_HHIT   =  67, /* rfc 9886 */
    Dns_Type_BRID   =  68, /* rfc 9886 */
    /* UNASSIGNED     =  69,
                      =  98,             */
    Dns_Type_SPF    =  99, /* OBSOLETE: rfc 4408, 7208 */
    Dns_Type_UINFO  = 100, /* OBSOLETE: - */
    Dns_Type_UID    = 101, /* OBSOLETE: - */
    Dns_Type_GID    = 102, /* OBSOLETE: - */
    Dns_Type_UNSPEC = 103, /* OBSOLETE: - */
    Dns_Type_NID    = 104, /* EXPERIMENTAL: rfc 6742 */
    Dns_Type_L32    = 105, /* EXPERIMENTAL: rfc 6742 */
    Dns_Type_L64    = 106, /* EXPERIMENTAL: rfc 6742 */
    Dns_Type_LP     = 107, /* EXPERIMENTAL: rfc 6742 */
    Dns_Type_EUI48  = 108, /* rfc 7043 */
    Dns_Type_EUI64  = 109, /* rfc 7043 */
    /* UNASSIGNED     = 110,
                      = 127,             */
    Dns_Type_NXNAME = 128, /* rfc 9824 */
    /* UNASSIGNED     = 129,
                      = 248,             */
    Dns_Type_TKEY   = 249, /* rfc 2930 */
    Dns_Type_TSIG   = 250, /* rfc 2845 */
    Dns_QType_IXFR  = 251, /* rfc 1996 */
    Dns_QType_AXFR  = 252, /* rfc 1035 */
    Dns_QType_MAILB = 253, /* EXPERIMENTAL: rfc 883, 1035, 2505 */
    Dns_QType_MAILA = 254, /* OBSOLETE: rfc 883, 973, 1035 */
    Dns_QType_ANY   = 255, /* rfc 1035 */
    Dns_Type_URI    = 256, /* rfc 7553 */
    Dns_Type_CAA    = 257, /* rfc 8659 */
    Dns_Type_DOA    = 259, /* OBSOLETE: - */
    Dns_Type_AMTRELAY = 260, /* rfc 8777 */
    Dns_Type_RESINFO = 261, /* rfc 9606 */
    Dns_Type_WALLET = 262, /* https://www.iana.org/assignments/dns-parameters/WALLET/wallet-completed-template */
    Dns_Type_CLA    = 263, /* - */
    Dns_Type_IPN    = 264, /* - */
    /* UNASSIGNED     = 265,
                      = 32767       */
    Dns_Type_TA     = 32768, /* - */
    Dns_Type_DLV    = 32769, /* OBSOLETE: rfc 4431, 8749 */
    /* UNASSIGNED     = 32770,
                      = 65279,      */
    /* PRIVATE        = 65280,
                      = 65534,      */
    /* RESERVED       = 65535       */
};

// This is a temporary array for me to easily keep track of
// what is an isn't supported at this time.
bool32 g_supported_dns_types[1ULL << 16] = {
    [Dns_Type_A]     = true,
    [Dns_Type_NS]    = true,
    [Dns_Type_CNAME] = true,
    [Dns_Type_AAAA]  = true,
};

typedef enum Dns_Class Dns_Class;
enum Dns_Class {
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
};

typedef enum Dns_OpCode Dns_OpCode;
enum Dns_OpCode {
    Dns_OpCode_Query    = 0, /* rfc 1035 */
    Dns_OpCode_IQuery   = 1, /* rfc 1035 */
    Dns_OpCode_Status   = 2, /* rfc 1035 */
    /* UNASSIGNED       = 3,             */
    Dns_OpCode_Notify   = 4, /* rfc 1996 */
    Dns_OpCode_Update   = 5, /* rfc 2136 */
    Dns_OpCode_Stateful = 6  /* rfc 8490 */
};

typedef enum Dns_RCode Dns_RCode;
enum Dns_RCode {
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
};

typedef struct Dns_RR Dns_RR;
struct Dns_RR {
    Dns_RR   *next;
    
    String8   name;
    Dns_Type  type;
    Dns_Class class;
    u32       ttl;
    
    union
    {
        u32     A;
        String8 NS;
        String8 CNAME;
        u128    AAAA;
    } rdata;
};

typedef struct Dns_Msg_Header Dns_Msg_Header;
struct Dns_Msg_Header {
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
};

typedef struct Dns_Msg Dns_Msg;
struct Dns_Msg {
    Dns_Msg_Header header;
    Dns_RR question;
    Dns_RR *answer;
    Dns_RR *ns;
    Dns_RR *extra;
    u8 *wire;
};

////////////////////////////////
// Enum -> String conversions

String8 class_to_str8[] = {
    [Dns_Class_INET]   = str8_lit_comp("IN"),
    [Dns_Class_CSNET]  = str8_lit_comp("CS"),
    [Dns_Class_CHAOS]  = str8_lit_comp("CH"),
    [Dns_Class_HESIOD] = str8_lit_comp("HS"),
    [Dns_QClass_NONE]  = str8_lit_comp("NONE"),
    [Dns_QClass_ANY]   = str8_lit_comp("ANY"),
};

String8 opcode_to_str8[] = {
    [Dns_OpCode_Query]    = str8_lit_comp("QUERY"),
    [Dns_OpCode_IQuery]   = str8_lit_comp("IQUERY"),
    [Dns_OpCode_Status]   = str8_lit_comp("STATUS"),
    [Dns_OpCode_Notify]   = str8_lit_comp("NOTIFY"),
    [Dns_OpCode_Update]   = str8_lit_comp("NOTIFY"),
    [Dns_OpCode_Stateful] = str8_lit_comp("STATEFUL"),
};

String8 rcode_to_str8[] = {
    [Dns_RCode_Success]                = str8_lit_comp("NOERROR"),
    [Dns_RCode_FormatError]            = str8_lit_comp("FORMERR"),
    [Dns_RCode_ServerFailure]          = str8_lit_comp("SERVFAIL"),
    [Dns_RCode_NameError]              = str8_lit_comp("NXDOMAIN"),
    [Dns_RCode_NotImplemented]         = str8_lit_comp("NOTIMPL"),
    [Dns_RCode_Refused]                = str8_lit_comp("REFUSED"),
    [Dns_RCode_YXDomain]               = str8_lit_comp("YXDOMAIN"),
    [Dns_RCode_YXRRSet]                = str8_lit_comp("YXRRSET"),
    [Dns_RCode_NXRRSet]                = str8_lit_comp("NXRRSET"),
    [Dns_RCode_NotAuth]                = str8_lit_comp("NOTAUTH"),
    [Dns_RCode_NotZone]                = str8_lit_comp("NOTZONE"),
    [Dns_RCode_StatefulNotImplemented] = str8_lit_comp("DSOTYPENI"),
    [Dns_RCode_BadVersion]             = str8_lit_comp("BADVERS"),
    [Dns_RCode_BadSig]                 = str8_lit_comp("BADSIG"),
    [Dns_RCode_BadTime]                = str8_lit_comp("BADTIME"),
    [Dns_RCode_BadMode]                = str8_lit_comp("BADMODE"),
    [Dns_RCode_BadName]                = str8_lit_comp("BADNAME"),
    [Dns_RCode_BadAlg]                 = str8_lit_comp("BADALG"),
    [Dns_RCode_BadTrunc]               = str8_lit_comp("BADTRUNC"),
    [Dns_RCode_BadCookie]              = str8_lit_comp("BADCOOKIE"),
};

///////////////
// Constants

// DEFAULT_MSG_SIZE is the default for messages larger than 512 bytes.
// this limit is the recommendation from rfc 9715
#define DEFAULT_MSG_SIZE     1400
#define MIN_MSG_SIZE         512
#define MAX_MSG_SIZE         max_u16
#define MSG_HEADER_SIZE      12
#define MAX_SERIAL_INCREMENT max_u32

///////////////////////////
// DNS Message Functions

Dns_Msg *dns_msg_alloc(Arena *arena, String8 domain, Dns_Type type);

///////////////////////
// Utility Functions

internal String8 str8_to_fqdn(Arena *arena, String8 s);
internal bool32  str8_is_fqdn(String8 s);
internal String8 str8_to_canonical(Arena *arena, String8 s);
internal bool32  str8_is_domain_name(String8 s);

////////////////////
// Wire <-> Struct

internal u8 *dns_rr_to_bytes(Arena *arena, Dns_RR rr);
internal u64 dns_bytes_to_rr(Dns_RR *out, u8 *bytes);

////////////////////////////////////////
// @per_os_impl Sytem DNS Info

internal String8_List dns_get_local_nameservers(Arena *arena);

#endif // DNS_CORE_H
