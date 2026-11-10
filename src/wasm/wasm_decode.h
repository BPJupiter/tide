#ifndef WASM_DECODE_H
#define WASM_DECODE_H

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


internal WASM_Module *wasm_module_decode(Arena *arena, String8 bytes);

#endif // WASM_DECODE_H
