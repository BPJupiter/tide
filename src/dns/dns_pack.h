
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

//////////////////
// Wire Packing

// @TODO: Redo packing with a ring buffer.
internal u64 dns_pack_u8(u8 i, u8 *wire, u64 off);
internal u64 dns_pack_u16(u16 i, u8 *wire, u64 off);
internal u64 dns_pack_u32(u32 i, u8 *wire, u64 off);
internal u64 dns_pack_u64(u64 i, u8 *wire, u64 off);
internal u64 dns_pack_u128(u128 i, u8 *wire, u64 off);
internal u64 dns_pack_str8(String8 s, u8 *wire, u64 off);

internal u64 dns_pack_question(Dns_RR rr, u8* wire, u64 off);
internal u64 dns_pack_rdata(Dns_RR rr, u8 *wire, u64 off);
internal u64 dns_pack_rr(Dns_RR rr, u8 *wire, u64 off);
internal u64 dns_pack_msg(Dns_Msg *msg);

