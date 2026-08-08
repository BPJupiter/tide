#ifndef DNS_MSG_H
#define DNS_MSG_H

////////////////////////////////
// Enum -> String conversions

String8 class_to_string[] = {
    [Dns_Class_INET]   = str8_lit_comp("IN"),
    [Dns_Class_CSNET]  = str8_lit_comp("CS"),
    [Dns_Class_CHAOS]  = str8_lit_comp("CH"),
    [Dns_Class_HESIOD] = str8_lit_comp("HS"),
    [Dns_QClass_NONE]  = str8_lit_comp("NONE"),
    [Dns_QClass_ANY]   = str8_lit_comp("ANY"),
};

String8 opcode_to_string[] = {
    [Dns_OpCode_Query]    = str8_lit_comp("QUERY"),
    [Dns_OpCode_IQuery]   = str8_lit_comp("IQUERY"),
    [Dns_OpCode_Status]   = str8_lit_comp("STATUS"),
    [Dns_OpCode_Notify]   = str8_lit_comp("NOTIFY"),
    [Dns_OpCode_Update]   = str8_lit_comp("NOTIFY"),
    [Dns_OpCode_Stateful] = str8_lit_comp("STATEFUL"),
};

String8 rcode_to_string[] = {
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

//////////////////////////
// Wire packing for RRs

// TODO: Implement
u32 wire_pack_rr(void);
// TODO: Implement
Dns_RR wire_unpack_rr(u8 *wire);

///////////////////////////
// DNS Message Functions

Dns_Msg *dns_msg_alloc(Arena *arena, String8 domain, Dns_Type type);


#endif // DNS_MSG_H
