#ifndef WASM_DECODE_H
#define WASM_DECODE_H

///////////////////
// Section Kinds

typedef u8 WASM_SectionKind;
enum WASM_SectionKindEnum
{
    WASM_SectionKind_Custom,
    WASM_SectionKind_Type,
    WASM_SectionKind_Import,
    WASM_SectionKind_Function,
    WASM_SectionKind_Table,
    WASM_SectionKind_Memory,
    WASM_SectionKind_Global,
    WASM_SectionKind_Export,
    WASM_SectionKind_Start,
    WASM_SectionKind_Element,
    WASM_SectionKind_Code,
    WASM_SectionKind_Data,
    WASM_SectionKind_DataCount,
    WASM_SectionKind_Tag,
    WASM_SectionKind_COUNT,
};

/////////////////
// Decoding Functions

global u32 wasm_default_max_pages    = 0x0000ffff;
global u32 wasm_default_max_pages_64 = 0xffffffff;

global u32 wasm_default_num_bytes_per_page = 0x0000ffff;


internal WASM_Module *wasm_module_decode(Arena *arena, String8 bytes);


#endif // WASM_DECODE_H
