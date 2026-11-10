
////////////////
// Stack Functions

internal WASM_Stack *
wasm_stack_make(Arena *arena, u32 size_in_bytes)
{
    WASM_Stack *stack = push_array(arena, WASM_Stack, 1);
    stack->cap = (size_in_bytes/sizeof(obj_pool[0]))-1;
    stack->obj_pool = push_array(arena, WASM_Obj, stack->cap);
}

internal bool32
wasm_stack_val_push(WASM_Stack *stack, WASM_Val val)
{
    bool32 result = 0;
    if(stack->idx < stack->cap)
    {
        WASM_Obj *obj = &stack->pool[stack->idx];
        obj->type = WASM_TypeKind_Val;
        obj->val  = val;
        stack->idx += 1;
        result = 1;
    }
    return result;
}

internal bool32
wasm_stack_label_push(WASM_Stack *stack, WASM_Label label)
{
    bool32 result = 0;
    if(stack->idx < stack->cap)
    {
        WASM_Obj *obj = &stack->pool[stack->idx];
        obj->type  = WASM_TypeKind_Label;
        obj->label = label;
        // how to handle allocation strategy here?
        //DLLPushBack(stack->labels.first, stack->labels.last, obj);
        stack->idx += 1;
        result = 1;
    }
    return result;
}

internal bool32
wasm_stack_frame_push(WASM_Stack *stack, WASM_Frame frame)
{
    bool32 result = 0;
    if(stack->idx < stack->cap)
    {
        WASM_Obj *obj = &stack->pool[stack->idx];
        obj->type  = WASM_TypeKind_Frame;
        obj->frame = frame;
        //DLLPushBack(stack->labels.first, stack->labels.last, obj);
        stack->idx += 1;
        result = 1;
    }
    return result;
}

internal WASM_Val *
wasm_stack_val_pop(Arena *arena, WASM_Stack *stack)
{
    WASM_Val *val = push_array(arena, WASM_Val, 1);
    MemoryCopyStruct(val, &stack->pool[stack->idx].val);
    stack->idx -= 1;
    return val;
}

internal WASM_Val *
wasm_stack_vals_pop(Arena *arena, WASM_Stack *stack)
{
    // TODO
    return 0;
}

internal WASM_Label *
wasm_stack_label_pop(Arena *arena, WASM_Stack *stack)
{
    WASM_Label *label = push_array(arena, WASM_Label, 1);
    MemoryCopyStruct(label, &stack->pool[stack->idx].label);
    stack->idx -= 1;
    // remove label from list
    return label;
}

internal WASM_Frame *
wasm_stack_frame_pop(Arena *arena, WASM_Stack *stack)
{
    WASM_Frame *frame = push_array(arena, WASM_Frame, 1);
    MemoryCopyStruct(frame, &stack->pool[stack->idx].frame);
    stack->idx -= 1;
    // remove frame from list
    return frame;
}

/////////////
// Store Functions

internal WASM_Store *
wasm_store_make(Arena *arena)
{
    WASM_Store *store = push_array(arena, WASM_Store, 1);
    store->stack = wasm_stack_make(arena);
    return store;
}

internal bool32
wasm_store_instantiate(Arena *arena, WASM_Store *store, WASM_Module *module, WASM_Extern_vals *extern_vals, WASM_Module_Inst **inst)
{
    // allocate space within store
    store->funcs.count   = module->funcs.count;
    store->tables.count  = module->tables.count;
    store->mems.count    = module->mems.count;
    store->globals.count = module->globals.count;
    store->elems.count   = module->elems.count;
    store->datas.count   = module->datas.count;
    
    store->funcs.v   = push_array(arena, WASM_Func_Inst, store->funcs.count);
    store->tables.v  = push_array(arena, WASM_Table_Inst, store->tables.count);
    store->mems.v    = push_array(arena, WASM_Mem_Inst, store->mems.count);
    store->globals.v = push_array(arena, WASM_Global_Inst, store->globals.count);
    store->elems.v   = push_array(arena, WASM_Elem_Inst, store->elems.count);
    store->datas.v   = push_array(arena, WASM_Data_Inst, store->datas.count);

    // create new module inst
    

}

internal bool32
wasm_store_invoke(WASM_Store *store, u32 func_addr, WASM_Arg_Array *args)
{
}

