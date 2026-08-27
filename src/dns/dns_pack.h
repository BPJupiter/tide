
///////////////////////
// Bit Packing Flags

#define _QR (1 << 15)
#define _AA (1 << 10)
#define _TC (1 << 9)
#define _RD (1 << 8)
#define _RA (1 << 7)
#define _Z  (1 << 6)
#define _AD (1 << 5)
#define _CD (1 << 4)

// ENDS0 OPT
#define _DO (1 << 15)
#define _CO (1 << 14)
#define _DE (1 << 13)

////////////////////////////
// Wire Packing/Unpacking

internal bool32 dns_pack_rdata   (Ring *ring, DNS_RR *rr);
internal bool32 dns_pack_question(Ring *ring, DNS_RR *rr);
internal bool32 dns_pack_rr      (Ring *ring, DNS_RR *rr);
internal bool32 dns_pack_msg     (Ring *ring, DNS_Msg *msg);

internal bool32 dns_unpack_labels  (Arena *arena, Ring *ring, String8 *out);
internal bool32 dns_unpack_rdata   (Arena *arena, Ring *ring, DNS_RR *rr, u16 rdlength);
internal bool32 dns_unpack_question(Arena *arena, Ring *ring, DNS_RR *rr);
internal bool32 dns_unpack_rr      (Arena *arena, Ring *ring, DNS_RR *rr);
internal bool32 dns_unpack_msg     (Arena *arena, Ring *ring, DNS_Msg *msg);

