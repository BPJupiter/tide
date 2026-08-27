// This DNS implementation takes inspiration from
// https://codeberg.org/miekg/dns where I thought
// appropriate!

#ifndef DNS_CORE_H
#define DNS_CORE_H

///////////////
// DNS Enums

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
global bool32 supported_dns_types[1ULL << 16] = {
    [Dns_Type_A]     = true,
    [Dns_Type_NS]    = true,
    [Dns_Type_CNAME] = true,
    [Dns_Type_SOA]   = true,
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

typedef enum Dns_TransportProtocol Dns_TransportProtocol;
enum Dns_TransportProtocol {
    Dns_TransportProtocol_UDP,
    Dns_TransportProtocol_TCP,
    Dns_TransportProtocol_TLS,
    Dns_TransportProtocol_HTTPS,
    Dns_TransportProtocol_COUNT
};

typedef enum Dns_RootServer Dns_RootServer;
enum Dns_RootServer {
    Dns_RootServer_A,
    Dns_RootServer_B,
    Dns_RootServer_C,
    Dns_RootServer_D,
    Dns_RootServer_E,
    Dns_RootServer_F,
    Dns_RootServer_G,
    Dns_RootServer_H,
    Dns_RootServer_I,
    Dns_RootServer_J,
    Dns_RootServer_K,
    Dns_RootServer_L,
    Dns_RootServer_M,
    Dns_RootServer_COUNT
};

////////////////////////
// Message Structures

typedef struct Dns_RR Dns_RR;
struct Dns_RR {
    String8   name;
    Dns_Type  type;
    Dns_Class class;
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

    u16 question_count;
    u16 answer_count;
    u16 nameserver_count;
    u16 additional_count;
};

typedef struct Dns_Msg Dns_Msg;
struct Dns_Msg {
    Dns_Msg_Header header;
    Dns_RR *question; // the only reason this is an array is because some clients MAY send more than one question.
                      // we will only ever write one ourselves.
    Dns_RR *answer;
    Dns_RR *ns;
    Dns_RR *extra;
};

///////////////////////
// Client Structures

typedef struct Dns_Client Dns_Client;
struct Dns_Client {
    Net_Client dialer;
    Dns_TransportProtocol dns_protocol;
};

/////////////////////////
// Server Structures

typedef struct Dns_Server Dns_Server;
struct Dns_Server {
    Net_Listener listener;
    Dns_TransportProtocol dns_protocol;
};

////////////////////////////////
// Enum -> String conversions

global String8 dns_type_to_str8[] = {
#if COMPILER_CLANG || COMPILER_GCC
    [0 ... 0xffff]   = str8_lit_comp("UNKNOWN_DNS_TYPE"),
#endif
    /* [0] IS RESERVED */
    [Dns_Type_A]      = str8_lit_comp("A"),
    [Dns_Type_NS]     = str8_lit_comp("NS"),
    [Dns_Type_MD]     = str8_lit_comp("MD"),
    [Dns_Type_MF]     = str8_lit_comp("MF"),
    [Dns_Type_CNAME]  = str8_lit_comp("CNAME"),
    [Dns_Type_SOA]    = str8_lit_comp("SOA"),
    [Dns_Type_MB]     = str8_lit_comp("MB"),
    [Dns_Type_MG]     = str8_lit_comp("MG"),
    [Dns_Type_MR]     = str8_lit_comp("MR"),
    [Dns_Type_NULL]   = str8_lit_comp("NULL"),
    [Dns_Type_WKS]    = str8_lit_comp("WKS"),
    [Dns_Type_PTR]    = str8_lit_comp("PTR"),
    [Dns_Type_HINFO]  = str8_lit_comp("HINFO"),
    [Dns_Type_MINFO]  = str8_lit_comp("MINFO"),
    [Dns_Type_MX]     = str8_lit_comp("MX"),
    [Dns_Type_TXT]    = str8_lit_comp("TXT"),
    [Dns_Type_RP]     = str8_lit_comp("RP"),
    [Dns_Type_AFSDB]  = str8_lit_comp("AFSDB"),
    [Dns_Type_X25]    = str8_lit_comp("X25"),
    [Dns_Type_ISDN]   = str8_lit_comp("ISDN"),
    [Dns_Type_RT]     = str8_lit_comp("RT"),
    [Dns_Type_NSAP]   = str8_lit_comp("NSAP"),
    [Dns_Type_NSAPPTR] = str8_lit_comp("NSAPPTR"),
    [Dns_Type_SIG]    = str8_lit_comp("SIG"),
    [Dns_Type_KEY]    = str8_lit_comp("KEY"),
    [Dns_Type_PX]     = str8_lit_comp("PX"),
    [Dns_Type_GPOS]   = str8_lit_comp("GPOS"),
    [Dns_Type_AAAA]   = str8_lit_comp("AAAA"),
    [Dns_Type_LOC]    = str8_lit_comp("LOC"),
    [Dns_Type_NXT]    = str8_lit_comp("NXT"),
    [Dns_Type_EID]    = str8_lit_comp("EID"),
    [Dns_Type_NIMLOC] = str8_lit_comp("NIMLOC"),
    [Dns_Type_NB]     = str8_lit_comp("NB"),
    [Dns_Type_NBSTAT] = str8_lit_comp("NBSTAT"),
    [Dns_Type_SRV]    = str8_lit_comp("SRV"),
    [Dns_Type_ATMA]   = str8_lit_comp("ATMA"),
    [Dns_Type_NAPTR]  = str8_lit_comp("NAPTR"),
    [Dns_Type_KX]     = str8_lit_comp("KX"),
    [Dns_Type_CERT]   = str8_lit_comp("CERT"),
    [Dns_Type_A6]     = str8_lit_comp("A6"),
    [Dns_Type_DNAME]  = str8_lit_comp("DNAME"),
    [Dns_Type_SINK]   = str8_lit_comp("SINK"),
    [Dns_Type_OPT]    = str8_lit_comp("OPT"),
    [Dns_Type_APL]    = str8_lit_comp("APL"),
    [Dns_Type_DS]     = str8_lit_comp("DS"),
    [Dns_Type_SSHFP]  = str8_lit_comp("SSHFP"),
    [Dns_Type_IPSECKEY] = str8_lit_comp("IPSECKEY"),
    [Dns_Type_RRSIG]  = str8_lit_comp("RRSIG"),
    [Dns_Type_NSEC]   = str8_lit_comp("NSEC"),
    [Dns_Type_DNSKEY] = str8_lit_comp("DNSKEY"),
    [Dns_Type_DHCID]  = str8_lit_comp("DHCID"),
    [Dns_Type_NSEC3]  = str8_lit_comp("NSEC3"),
    [Dns_Type_NSEC3PARAM] = str8_lit_comp("NSEC3PARAM"),
    [Dns_Type_TLSA]   = str8_lit_comp("TLSA"),
    [Dns_Type_SMIMEA] = str8_lit_comp("SMIMEA"),
    /* UNASSIGNED     =  54,             */
    [Dns_Type_HIP]    = str8_lit_comp("HIP"),
    [Dns_Type_NINFO]  = str8_lit_comp("NINFO"),
    [Dns_Type_RKEY]   = str8_lit_comp("RKEY"),
    [Dns_Type_TALINK] = str8_lit_comp("TALINK"),
    [Dns_Type_CDS]    = str8_lit_comp("CDS"),
    [Dns_Type_CDNSKEY] = str8_lit_comp("CDNSKEY"),
    [Dns_Type_OPENPGPKEY] = str8_lit_comp("OPENPGPKEY"),
    [Dns_Type_CSYNC]  = str8_lit_comp("CSYNC"),
    [Dns_Type_ZONEMD] = str8_lit_comp("ZONEMD"),
    [Dns_Type_SVCB]   = str8_lit_comp("SVCB"),
    [Dns_Type_HTTPS]  = str8_lit_comp("HTTPS"),
    [Dns_Type_DSYNC]  = str8_lit_comp("DSYNC"),
    [Dns_Type_HHIT]   = str8_lit_comp("HHIT"),
    [Dns_Type_BRID]   = str8_lit_comp("BRID"),
    /* UNASSIGNED     =  69,
                      =  98,             */
    [Dns_Type_SPF]    = str8_lit_comp("SPF"),
    [Dns_Type_UINFO]  = str8_lit_comp("UINFO"),
    [Dns_Type_UID]    = str8_lit_comp("UID"),
    [Dns_Type_GID]    = str8_lit_comp("GID"),
    [Dns_Type_UNSPEC] = str8_lit_comp("UNSPEC"),
    [Dns_Type_NID]    = str8_lit_comp("NID"),
    [Dns_Type_L32]    = str8_lit_comp("L32"),
    [Dns_Type_L64]    = str8_lit_comp("L64"),
    [Dns_Type_LP]     = str8_lit_comp("LP"),
    [Dns_Type_EUI48]  = str8_lit_comp("EUI48"),
    [Dns_Type_EUI64]  = str8_lit_comp("EUI64"),
    /* UNASSIGNED     = 110,
                      = 127,             */
    [Dns_Type_NXNAME] = str8_lit_comp("NXNAME"),
    /* UNASSIGNED     = 129,
                      = 248,             */
    [Dns_Type_TKEY]   = str8_lit_comp("TKEY"),
    [Dns_Type_TSIG]   = str8_lit_comp("TSIG"),
    [Dns_QType_IXFR]  = str8_lit_comp("IXFR"),
    [Dns_QType_AXFR]  = str8_lit_comp("AXFR"),
    [Dns_QType_MAILB] = str8_lit_comp("MAILB"),
    [Dns_QType_MAILA] = str8_lit_comp("MAILA"),
    [Dns_QType_ANY]   = str8_lit_comp("ANY"),
    [Dns_Type_URI]    = str8_lit_comp("URI"),
    [Dns_Type_CAA]    = str8_lit_comp("CAA"),
    [Dns_Type_DOA]    = str8_lit_comp("DOA"),
    [Dns_Type_AMTRELAY] = str8_lit_comp("AMTRELAY"),
    [Dns_Type_RESINFO] = str8_lit_comp("RESINFO"),
    [Dns_Type_WALLET] = str8_lit_comp("WALLET"),
    [Dns_Type_CLA]    = str8_lit_comp("CLA"),
    [Dns_Type_IPN]    = str8_lit_comp("IPN"),
    /* UNASSIGNED     = 265,
                      = 32767       */
    [Dns_Type_TA]     = str8_lit_comp("TA"),
    [Dns_Type_DLV]    = str8_lit_comp("DLV"),
};

global String8 dns_class_to_str8[] = {
    [Dns_Class_INET]   = str8_lit_comp("IN"),
    [Dns_Class_CSNET]  = str8_lit_comp("CS"),
    [Dns_Class_CHAOS]  = str8_lit_comp("CH"),
    [Dns_Class_HESIOD] = str8_lit_comp("HS"),
    [Dns_QClass_NONE]  = str8_lit_comp("NONE"),
    [Dns_QClass_ANY]   = str8_lit_comp("ANY"),
};

global String8 dns_opcode_to_str8[] = {
    [Dns_OpCode_Query]    = str8_lit_comp("QUERY"),
    [Dns_OpCode_IQuery]   = str8_lit_comp("IQUERY"),
    [Dns_OpCode_Status]   = str8_lit_comp("STATUS"),
    [Dns_OpCode_Notify]   = str8_lit_comp("NOTIFY"),
    [Dns_OpCode_Update]   = str8_lit_comp("NOTIFY"),
    [Dns_OpCode_Stateful] = str8_lit_comp("STATEFUL"),
};

global String8 dns_rcode_to_str8[] = {
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

global String8 dns_root_server_to_domain_name[] = {
    [Dns_RootServer_A] = str8_lit_comp("a.root-servers.net"),
    [Dns_RootServer_B] = str8_lit_comp("b.root-servers.net"),
    [Dns_RootServer_C] = str8_lit_comp("c.root-servers.net"),
    [Dns_RootServer_D] = str8_lit_comp("d.root-servers.net"),
    [Dns_RootServer_E] = str8_lit_comp("e.root-servers.net"),
    [Dns_RootServer_F] = str8_lit_comp("f.root-servers.net"),
    [Dns_RootServer_G] = str8_lit_comp("g.root-servers.net"),
    [Dns_RootServer_H] = str8_lit_comp("h.root-servers.net"),
    [Dns_RootServer_I] = str8_lit_comp("i.root-servers.net"),
    [Dns_RootServer_J] = str8_lit_comp("j.root-servers.net"),
    [Dns_RootServer_K] = str8_lit_comp("k.root-servers.net"),
    [Dns_RootServer_L] = str8_lit_comp("l.root-servers.net"),
    [Dns_RootServer_M] = str8_lit_comp("m.root-servers.net"),
};

global u32 dns_root_server_to_ipv4[] = {
    [Dns_RootServer_A] = 0xC6290004, // 198.41.0.4
    [Dns_RootServer_B] = 0xAAF7AA02, // 170.247.170.2
    [Dns_RootServer_C] = 0xC021040C, // 192.33.4.12
    [Dns_RootServer_D] = 0xC7075B0D, // 199.7.91.13
    [Dns_RootServer_E] = 0xC0CBE60A, // 192.203.230.10
    [Dns_RootServer_F] = 0xC00505F1, // 192.5.5.241
    [Dns_RootServer_G] = 0xC0702404, // 192.112.36.4
    [Dns_RootServer_H] = 0xC661BE35, // 198.97.190.53
    [Dns_RootServer_I] = 0xC0249411, // 192.36.148.17
    [Dns_RootServer_J] = 0xC03A801E, // 192.58.128.30
    [Dns_RootServer_K] = 0xC1000E81, // 193.0.14.129
    [Dns_RootServer_L] = 0xC707532A, // 199.7.83.42
    [Dns_RootServer_M] = 0xCA0C1B21, // 202.12.27.33
};

global u128 dns_root_server_to_ipv6[] = {
    [Dns_RootServer_A] = u128_lit64(0x20010503BA3E0000, 0x0000000000020030), // 2001:503:ba3e::2:30
    [Dns_RootServer_B] = u128_lit64(0x280101B800100000, 0x000000000000000B), // 2801:1b8:10::b
    [Dns_RootServer_C] = u128_lit64(0x2001050000020000, 0x000000000000000C), // 2001:500:2::c
    [Dns_RootServer_D] = u128_lit64(0x20010500002D0000, 0x000000000000000D), // 2001:500:2d::d
    [Dns_RootServer_E] = u128_lit64(0x2001050000A80000, 0x000000000000000E), // 2001:500:a8::e
    [Dns_RootServer_F] = u128_lit64(0x20010500002F0000, 0x000000000000000F), // 2001:500:2f::f
    [Dns_RootServer_G] = u128_lit64(0x2001050000120000, 0x0000000000000D0D), // 2001:500:12::d0d
    [Dns_RootServer_H] = u128_lit64(0x2001050000010000, 0x0000000000000053), // 2001:500:1::53
    [Dns_RootServer_I] = u128_lit64(0x200107FE00000000, 0x0000000000000053), // 2001:7fe::53
    [Dns_RootServer_J] = u128_lit64(0x200105030C270000, 0x0000000000020030), // 2001:503:c27::2:30
    [Dns_RootServer_K] = u128_lit64(0x200107FD00000000, 0x0000000000000001), // 2001:7fd::1
    [Dns_RootServer_L] = u128_lit64(0x20010500009F0000, 0x0000000000000042), // 2001:500:9f::42
    [Dns_RootServer_M] = u128_lit64(0x20010DC300000000, 0x0000000000000035), // 2001:dc3::35
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
// @REMOVE: Supported Dns Check

#define DNS_CRASH_THE_PROGRAM_IF_THIS_TYPE_IS_SUPPORTED(type)                                \
    do {                                                                                     \
        if (supported_dns_types[type]) {                                                     \
            fprintf(stderr, "////////////////////////////////////////////////////////\n");   \
            fprintf(stderr, " SUPPORTED DNS TYPE NOT IMPLEMENTED! : %.*s\n ",                \
                    str8_varg(dns_type_to_str8[type]));                                      \
            fprintf(stderr, "////////////////////////////////////////////////////////\n");   \
            u64 *SUPPORTED_DNS_TYPE_NOT_IMPLEMENTED = 0;                                     \
            SUPPORTED_DNS_TYPE_NOT_IMPLEMENTED[0] = 1;                                       \
        }                                                                                    \
    } while(0)

///////////////////////
// Message Functions

internal Dns_Msg dns_msg_alloc(Arena *arena, String8 domain, Dns_Type type);
internal String8 dns_msg_header_to_str8(Arena *arena, Dns_Msg_Header h);

//////////////////////
// Client Functions

internal Dns_Client dns_client_alloc(Arena *arena, Net_AddressFamily family, Dns_TransportProtocol protocol);
internal void       dns_client_release(Dns_Client client);
internal Dns_Msg    dns_exchange(Arena *arena, Dns_Msg msg, Dns_TransportProtocol protocol, String8 target); // TODO
internal Dns_Msg    dns_client_exchange(Arena *arena, Dns_Client client, Dns_Msg msg, String8 target); // TODO
internal Dns_Msg    dns_client_exchange_with_address(Arena *arena, Dns_Client client, Dns_Msg msg, Net_Address address);

////////////////////////
// Server Functions

internal Dns_Server dns_server_alloc(Arena *arena, Net_AddressFamily family, Dns_TransportProtocol protocol); // TODO
internal void       dns_server_release(Dns_Server server);
internal void       dns_listen_and_serve(String8 address, Dns_TransportProtocol protocol); // TODO
internal void       dns_server_listen_and_serve(Dns_Server server);
internal void       dns_server_shutdown(Dns_Server *server); // TODO
internal void       dns_server_shutdown_and_release(Dns_Server *server); // TODO

///////////////////////
// Utility Functions

internal String8 str8_to_fqdn(Arena *arena, String8 s);
internal bool32  str8_is_fqdn(String8 s);
internal String8 str8_to_canonical(Arena *arena, String8 s);
internal String8 str8_to_name_labels(Arena *arena, String8 s);
internal bool32  str8_is_name_labels(String8 s);

//////////////////
// Wire Lengths

internal u64 dns_rdata_wire_length(Dns_RR *rr);
internal u64 dns_rr_wire_length(Dns_RR *rr);
internal u64 dns_msg_wire_length(Dns_Msg *msg);

//////////////////////////////
// DNS Diagnostic Functions

//~ fbt: This function pings each root server A-M and then attemps a single DNS query to each one
//       if servers respond to a ping, but NONE respond to our DNS query, then we figure
//       our local network blocks outbound DNS not headed for the local resolver.
internal bool32 dns_is_blocked_on_this_network(Dns_TransportProtocol protocol);

////////////////////////////////////////
// @per_os_impl Sytem DNS Info

internal String8_List dns_get_local_nameservers(Arena *arena);

#endif // DNS_CORE_H
