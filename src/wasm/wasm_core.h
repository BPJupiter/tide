
#ifndef WASM_CORE_H
#define WASM_CORE_H

//////////////////
// Generated Code

#include "generated/wasm.meta.h"

/////////////
// Common Types

typedef struct WASM_Function_Type WASM_Function_Type;
struct WASM_Function_Type
{
    u16 param_cell_count;
    u16 return_cell_count;
    u16 reference_count;
    String8 params;
    String8 results;
};

typedef WASM_Function_Type WASM_Type;

////////////////
// Global Types

typedef struct WASM_Global WASM_Global;
struct WASM_Global
{
    u64 foo;
    // @TODO
};

////////////
// Function

typedef struct WASM_Function WASM_Function;
struct WASM_Function
{
    // signature info
    WASM_Function_Type *func_type;
    u32 func_type_index;
    
    String8 local_types;
    u16 param_cell_count;
    u16 result_cell_count;
    u16 local_cell_count;

    String8 code;
};

//////////
// Table

typedef struct WASM_Table WASM_Table;
struct WASM_Table
{
    u64 foo;
    // @TODO
};

//////////
// Memory

typedef struct WASM_Memory WASM_Memory;
struct WASM_Memory
{
    u64 foo;
    // @TODO
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
    u8                  func_type_index;
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
    // @TODO
};

typedef struct WASM_Import WASM_Import;
struct WASM_Import
{
    WASM_ImportKind kind;
    union
    {
        WASM_Function_Import function;
        WASM_Table_Import table;
        WASM_Memory_Import memory;
        WASM_Global_Import global;
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

typedef enum WASM_ModuleKind
{
    WASM_ModuleKind_Bytecode,
    WASM_ModuleKind_COUNT,
} WASM_ModuleKind;

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

