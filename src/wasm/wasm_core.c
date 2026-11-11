
//////////////
// Generated Code

#include "generated/wasm.meta.c"

//////////////////
// X-List Helpers

internal u64
wasm_size_from_type_kind(WASM_TypeKind k)
{
    u64 result = 0;
    switch(k)
    {
        default:{}break;
#define X(name, id, size) case WASM_TypeKind_##name:{result = size;}break;
        WASM_TypeKind_XList
#undef X
    }
    return result;
}

internal u64
wasm_cell_num_from_type(WASM_TypeKind type)
{
    return wasm_size_from_type_kind(type)/4;
}

internal u64
wasm_cell_num_from_types(String8 types)
{
    u64 result = 0;
    for(u64 i = 0; i < types.size; i += 1)
    {
        result += wasm_cell_num_from_type(types.str[i]);
    }
    return result;
}

/////////////////
// Comparison Helpers

internal bool32
wasm_type_match(WASM_Type *a, WASM_Type *b)
{
    // NOTE: refence_count is ignored.
    return (a->param_cell_count == b->param_cell_count &&
            a->result_cell_count == b->result_cell_count &&
            str8_match(a->params, b->params, 0) &&
            str8_match(a->results, b->results, 0));
}


///////////////
// String Helpers

internal String8
wasm_string_from_import_kind(WASM_ImportKind k)
{
    String8 result = s("<Unknown WASM_ImportKind>");
    switch(k)
    {
        default:{}break;
        case WASM_ImportKind_Func:  {result = s("func");}break;
        case WASM_ImportKind_Table: {result = s("table");}break;
        case WASM_ImportKind_Memory:{result = s("memory");}break;
        case WASM_ImportKind_Global:{result = s("global");}break;
    }
    return result;
}
