
#ifndef WASM_CORE_H
#define WASM_CORE_H

//////////////////
// Generated Code

#include "generated/wasm.meta.h"

typedef u128 v128;

////////////////
// Top-Level Types

typedef union WASM_Value WASM_Value;
union WASM_Value
{
    s32 s32;
    u32 u32;
    u32 i32;
    u32 global_index;
    u32 ref_index;
    s64 s64;
    u64 u64;
    u64 i64;
    f32 f32;
    f64 f64;
    u128 v128;
};

////////////////
// Memory Flag

typedef u8 WASM_MemoryFlags;
enum WASM_MemoryFlagsEnum
{
    WASM_MemoryFlag_HasMaxSize = (1<<0),
    WASM_MemoryFlag_IsShared   = (1<<1),
    WASM_MemoryFlag_Is64Bit    = (1<<2),
};

/////////////
// Common Types

typedef struct WASM_Function_Type WASM_Function_Type;
struct WASM_Function_Type
{
    u16 param_cell_count;
    u16 result_cell_count;
    u16 reference_count;
    String8 params;
    String8 results;
};

typedef WASM_Function_Type WASM_Type;

////////////////
// Global Types

typedef struct WASM_Init_Expr WASM_Init_Expr;
struct WASM_Init_Expr
{
    WASM_TypeKind type_kind;
    union
    {
        struct
        {
            WASM_Value v;
        } unary;
        struct
        {
            WASM_Init_Expr *l_expr;
            WASM_Init_Expr *r_expr;
        } binary;
    };
};

typedef struct WASM_Global_Type WASM_Global_Type;
struct WASM_Global_Type
{
    WASM_TypeKind kind;
    bool32 is_mutable;
};

typedef struct WASM_Global WASM_Global;
struct WASM_Global
{
    WASM_Global_Type type;
    WASM_Init_Expr init_expr;
};

////////////
// Function

typedef struct WASM_Function WASM_Function;
struct WASM_Function
{
    // signature info
    WASM_Function_Type *func_type;
    u32 func_type_index;
    
    String8 locals;
    u16 param_cell_count;
    u16 result_cell_count;
    u16 local_cell_count;
    String16 stack_cell_offsets;

    String8 code;
};

//////////
// Table

typedef struct WASM_Table_Type WASM_Table_Type;
struct WASM_Table_Type
{
    u8 element_kind;
    WASM_MemoryFlags flags;
    bool32 possible_grow;
    u32 init_size;
    u32 max_size;
};

typedef struct WASM_Table WASM_Table;
struct WASM_Table
{
    WASM_Table_Type table_type;
};

//////////
// Memory

typedef struct WASM_Memory WASM_Memory;
struct WASM_Memory
{
    WASM_MemoryFlags flags;
    u32 num_bytes_per_page;
    u32 init_page_count;
    u32 max_page_count;
};

////////////
// Element

typedef struct WASM_Elem WASM_Elem;
struct WASM_Elem
{
    u64 foo;
    // @TODO
};

//////////
// Data

typedef struct WASM_Data WASM_Data;
struct WASM_Data
{
    u64 foo;
    // @TODO
};

////////////////
// Export Types

typedef u8 WASM_ExportKind;
enum WASM_ExportKindEnum
{
    WASM_ExportDescKind_Func   = 0,
    WASM_ExportDescKind_Table  = 1,
    WASM_ExportDescKind_Mem    = 2,
    WASM_ExportDescKind_Global = 3,
};

typedef struct WASM_Export WASM_Export;
struct WASM_Export
{
    u8 *name;
    WASM_ExportKind kind;
    u32 idx;
};

///////////
// Import Types

typedef u8 WASM_ImportKind;
enum WASM_ImportKindEnum
{
    WASM_ImportKind_Func   = 0,
    WASM_ImportKind_Table  = 1,
    WASM_ImportKind_Memory = 2,
    WASM_ImportKind_Global = 3,
};

typedef struct WASM_Function_Import WASM_Function_Import;
struct WASM_Function_Import
{
    // base names
    String8 module_name;
    String8 field_name;
    
    WASM_Function_Type *func_type;
    u32                 func_type_index;
};

typedef struct WASM_Table_Import WASM_Table_Import;
struct WASM_Table_Import
{
    // base names
    String8 module_name;
    String8 field_name;
    // @TODO
};

typedef struct WASM_Memory_Import WASM_Memory_Import;
struct WASM_Memory_Import
{
    // base names
    String8 module_name;
    String8 field_name;
    // @TODO
};

typedef struct WASM_Global_Import WASM_Global_Import;
struct WASM_Global_Import
{
    // base names
    String8 module_name;
    String8 field_name;
    
    WASM_Global base_global;
    u32 data_offset;
};

typedef struct WASM_Import WASM_Import;
struct WASM_Import
{
    WASM_ImportKind kind;
    union
    {
        WASM_Function_Import function_import;
        WASM_Table_Import table_import;
        WASM_Memory_Import memory_import;
        WASM_Global_Import global_import;
        // base (present as first two fields of all union members)
        struct
        {
            String8 module_name;
            String8 field_name;
        } names;
    };
};

/////////////
// Top-Level Module Bundle

typedef u32 WASM_ModuleKind;
enum WASM_ModuleKindEnum
{
    WASM_ModuleKind_Bytecode,
    WASM_ModuleKind_COUNT,
};

typedef struct WASM_Module WASM_Module;
struct WASM_Module
{
    // metadata
    WASM_ModuleKind      kind;
    u32                  package_version;

    // section counts
    u64 type_count;
    u64 import_count;
    u64 function_count;
    u64 table_count;
    u64 memory_count;
    u64 tag_count;
    u64 global_count;
    u64 export_count;
    u64 element_count;
    u64 data_count;
    
    // section elements
    WASM_Type           *types;
    WASM_Import         *imports;
    struct
    {
        u64 function_count;
        u64 table_count;
        u64 memory_count;
        u64 global_count;
        WASM_Import *functions;
        WASM_Import *tables;
        WASM_Import *memories;
        WASM_Import *globals;
    } import_views;
    WASM_Function       *functions;
    WASM_Table          *tables;
    WASM_Memory         *memories;
    // tags?
    WASM_Global         *globals;
    WASM_Export         *exports;
    WASM_Elem           *elements;
    WASM_Data           *datas;

    // entry point information
    bool32               has_start;
    u32                  start_function_idx;
};

//////////////////
// X-List Helpers

internal u64 wasm_size_from_type_kind(WASM_TypeKind k);
internal u64 wasm_cell_num_from_type(WASM_TypeKind type);
internal u64 wasm_cell_num_from_types(String8 types);

/////////////////
// Comparison Helpers

internal bool32 wasm_type_match(WASM_Type *a, WASM_Type *b);

///////////////
// String Helpers

internal String8 wasm_string_from_import_kind(WASM_ImportKind k);


#endif // WASM_CORE_H

