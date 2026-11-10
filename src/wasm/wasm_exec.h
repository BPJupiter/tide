#ifndef WASM_EXEC_H
#define WASM_EXEC_H

/////////////
// Runtime Type Definitions

typedef u8 WASM_ExternKind;
enum WASM_ExternKindEnum
{
    WASM_ExternKind_Func   = 0,
    WASM_ExternKind_Table  = 1,
    WASM_ExternKind_Mem    = 2,
    WASM_ExternKind_Global = 3,
};

typedef struct WASM_Extern_Val WASM_Extern_Val;
struct WASM_Extern_Val
{
    WASM_ExternKind kind;
    union
    {
        u32 func_addr;
        u32 table_addr;
        u32 mem_addr;
        u32 global_addr;
    };
};
DefineTypeArray(WASM_Extern_Val);

typedef struct WASM_Export_Inst WASM_Export_Inst;
struct WASM_Export_Inst
{
    u8 *name;
    WASM_Extern_Val value;
};
DefineTypeArray(WASM_Export_Inst);

typedef struct WASM_Module_Inst WASM_Module_Inst;
struct WASM_Module_Inst
{
    WASM_Func_Type *func_types;
    u32            *func_addrs;
    u32            *table_addrs;
    u32            *mem_addrs;
    u32            *global_addrs;
    u32            *elem_addrs;
    u32            *data_addrs;
    WASM_Export_Inst_Array exports;
};

typedef struct WASM_Func_Inst WASM_Func_Inst;
struct WASM_Func_Inst
{
    WASM_Func_Type *type;
    WASM_Module_Inst *module;
    WASM_Func *code;
};
DefineTypeArray(WASM_Func_Inst);

typedef struct WASM_Table_Inst WASM_Table_Inst;
struct WASM_Table_Inst
{
    WASM_Table_Type type;
    u32_Array elems;
};
DefineTypeArray(WASM_Table_Inst);

typedef union WASM_Num WASM_Num;
union WASM_Num
{
    u32 i32;
    u64 i64;
    f32 f32;
    f64 f64;
};

typedef union WASM_Val WASM_Val;
struct WASM_Val
{
    WASM_Num num;
    u32 ref;
};

typedef struct WASM_Label WASM_Label;
struct WASM_Label
{
    u32 arity;
    WASM_Instruction *parent;
    WASM_Instruction *continuation;
};
DefineTypeNode_DL(WASM_Label);
DefineTypeList(WASM_Label);

typedef struct WASM_Frame WASM_Frame;
struct WASM_Frame
{
    u32 arity;
    WASM_Val *locals;
    WASM_Module_Inst *module;
};
DefineTypeNode_DL(WASM_Frame);
DefineTypeList(WASM_Frame);

// stack

typedef u8 WASM_ObjKind;
enum WASM_ObjKindEnum
{
    WASM_TypeKind_Val   = 0,
    WASM_TypeKind_Label = 1,
    WASM_TypeKind_Frame = 2,
};

typedef struct WASM_Obj WASM_Obj;
struct WASM_Obj
{
    WASM_ObjKind kind; // identifier
    union
    {
        WASM_Val val;
        WASM_Label label;
        WASM_Frame frame;
    };
};

typedef struct WASM_Stack WASM_Stack;
struct WASM_Stack
{
    // NOTE: probably want to let the stack own its own memory here.
    //Arena *arena;
    WASM_Frame_List frames;
    WASM_Label_List labels;
    u32 cap;
    u32 idx;
    WASM_Obj *obj_pool;
};

typedef struct WASM_Mem_Inst WASM_Mem_Inst;
struct WASM_Mem_Inst
{
    WASM_Limits type;
    u32 page_count;
    u8 ***table2[4]; // @FIX ??????????????????????
};
DefineTypeArray(WASM_Mem_Inst);

typedef struct WASM_Global_Inst WASM_Global_Inst;
struct WASM_Global_Inst
{
    WASM_Global_Type global_type;
    WASM_Val val;
};
DefineTypeArray(WASM_Global_Inst);

typedef struct WASM_Elem_Inst WASM_Elem_Inst;
struct WASM_Elem_Inst
{
    u8 ref_type;
    u32_Array elem;
};
DefineTypeArray(WASM_Elem_Inst);

typedef struct WASM_Data_Inst WASM_Data_Inst;
struct WASM_Data_Inst
{
    String8 data;
};
DefineTypeArray(WASM_Data_Inst);

typedef struct WASM_Store WASM_Store;
struct WASM_Store
{
    WASM_Func_Inst_Array   funcs;
    WASM_Table_Inst_Array  tables;
    WASM_Mem_Inst_Array    mems;
    WASM_Global_Inst_Array globals;
    WASM_Elem_Inst_Array   elems;
    WASM_Data_Inst_Array   datas;
    WASM_Stack *stack;
};

typedef struct WASM_Arg WASM_Arg;
struct WASM_Arg
{
    u8 type;
    WASM_Val val;
};
DefineTypeArray(WASM_Arg);

////////////////
// Stack Functions

internal WASM_Stack *wasm_stack_make(Arena *arena, u32 size_in_bytes);
internal bool32 wasm_stack_val_push(WASM_Stack *stack, WASM_Val val);
internal bool32 wasm_stack_label_push(WASM_Stack *stack, WASM_Label label);
internal bool32 wasm_stack_frame_push(WASM_Stack *stack, WASM_Frame frame);
internal WASM_Val *wasm_stack_val_pop(Arena *arena, WASM_Stack *stack);
internal WASM_Val *wasm_stack_vals_pop(Arena *arena, WASM_Stack *stack);
internal WASM_Label *wasm_stack_label_pop(Arena *arena, WASM_Stack *stack);
internal WASM_Frame *wasm_stack_frame_pop(Arena *arena, WASM_Stack *stack);

/////////////
// Store Functions

internal WASM_Store *wasm_store_make(Arena *arena);
internal bool32 wasm_store_instantiate(WASM_Store *store, WASM_Module *module, WASM_Extern_Val_Array *extern_vals, WASM_Module_Inst **inst);
internal bool32 wasm_store_invoke(WASM_Store *store, u32 func_addr, WASM_Arg_Array *args);

#endif // WASM_EXEC_H
