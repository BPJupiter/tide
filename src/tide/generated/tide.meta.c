// Copyright (c) Epic Games Tools
// Licensed under the MIT license (https://opensource.org/license/mit/)

//- GENERATED CODE

C_LINKAGE_BEGIN
String8 ti_reg_slot_code_name_table[3] =
{
{0},
str8_lit_comp("window"),
str8_lit_comp("wm_event"),
};

Rng1u64 ti_reg_slot_range_table[3] =
{
{0},
{OffsetOf(TI_Regs, window), OffsetOf(TI_Regs, window) + sizeof(CFG_ID)},
{OffsetOf(TI_Regs, wm_event), OffsetOf(TI_Regs, wm_event) + sizeof(WM_Event *)},
};

TI_Cmd_Kind_Info ti_cmd_kind_info_table[5] =
{
{0},
{ str8_lit_comp("exit"), str8_lit_comp("Exits the TIDE program."), TI_CmdKindFlag_ListInUI*1},
{ str8_lit_comp("wm_event"), str8_lit_comp(""), TI_CmdKindFlag_ListInUI*0},
{ str8_lit_comp("open_window"), str8_lit_comp("Opens a new window."), TI_CmdKindFlag_ListInUI*1},
{ str8_lit_comp("close_window"), str8_lit_comp("Closes an opened window."), TI_CmdKindFlag_ListInUI*1},
};

String8 ti_icon_kind_text_table[75] =
{
str8_lit_comp(""),
str8_lit_comp("b"),
str8_lit_comp("c"),
str8_lit_comp("B"),
str8_lit_comp("C"),
str8_lit_comp("f"),
str8_lit_comp("F"),
str8_lit_comp("g"),
str8_lit_comp("h"),
str8_lit_comp("r"),
str8_lit_comp("s"),
str8_lit_comp("i"),
str8_lit_comp("w"),
str8_lit_comp("W"),
str8_lit_comp("k"),
str8_lit_comp("K"),
str8_lit_comp("L"),
str8_lit_comp("R"),
str8_lit_comp("U"),
str8_lit_comp("D"),
str8_lit_comp("G"),
str8_lit_comp("P"),
str8_lit_comp("3"),
str8_lit_comp("p"),
str8_lit_comp("O"),
str8_lit_comp("o"),
str8_lit_comp("!"),
str8_lit_comp("1"),
str8_lit_comp("V"),
str8_lit_comp("<"),
str8_lit_comp(">"),
str8_lit_comp("^"),
str8_lit_comp("v"),
str8_lit_comp("9"),
str8_lit_comp("0"),
str8_lit_comp("7"),
str8_lit_comp("8"),
str8_lit_comp("+"),
str8_lit_comp("-"),
str8_lit_comp("'"),
str8_lit_comp("\""),
str8_lit_comp("M"),
str8_lit_comp("."),
str8_lit_comp("x"),
str8_lit_comp("q"),
str8_lit_comp("j"),
str8_lit_comp("u"),
str8_lit_comp("m"),
str8_lit_comp("n"),
str8_lit_comp("l"),
str8_lit_comp("a"),
str8_lit_comp("z"),
str8_lit_comp("y"),
str8_lit_comp("X"),
str8_lit_comp("Y"),
str8_lit_comp("S"),
str8_lit_comp("T"),
str8_lit_comp("Z"),
str8_lit_comp("d"),
str8_lit_comp("N"),
str8_lit_comp("E"),
str8_lit_comp("H"),
str8_lit_comp("e"),
str8_lit_comp("I"),
str8_lit_comp("J"),
str8_lit_comp("A"),
str8_lit_comp("?"),
str8_lit_comp("4"),
str8_lit_comp("5"),
str8_lit_comp("6"),
str8_lit_comp("&"),
str8_lit_comp("*"),
str8_lit_comp("("),
str8_lit_comp(")"),
str8_lit_comp("#"),
};

C_LINKAGE_END

