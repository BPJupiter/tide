
internal WASM_Module *
wasm_module_decode(Arena *arena, String8 data)
{
    Temp scratch = scratch_begin(&arena, 1);
    WASM_Module *module = 0;
    u64 module_cursor = 0;

    ////////////////
    // check magic number & version
    //
    {
        u32 magic = 0;
        u32 version = 0;
        module_cursor += str8_deserial_read_struct(data, module_cursor, &magic);
        module_cursor += str8_deserial_read_struct(data, module_cursor, &version);
        
        if(magic != 0x6d736100 || version != 0x00000001)
        {
            return module;
        }
    }

    /////////////////
    // initialise module
    //
    module = push_array(arena, WASM_Module, 1);

    WASM_SectionKind expected_section_order[WASM_SectionKind_COUNT] =
    {
        WASM_SectionKind_Type,
        WASM_SectionKind_Import,
        WASM_SectionKind_Function,
        WASM_SectionKind_Table,
        WASM_SectionKind_Memory,
        WASM_SectionKind_Tag,
        WASM_SectionKind_Global,
        WASM_SectionKind_Export,
        WASM_SectionKind_Start,
        WASM_SectionKind_Element,
        WASM_SectionKind_DataCount,
        WASM_SectionKind_Code,
        WASM_SectionKind_Data,
    };
    u8 expected_section_idx = 0;

    String8 sections[WASM_SectionKind_COUNT] = {0};

    ////////////////////////
    // fill sections array with data
    //
    for(;module_cursor < data.size;)
    {
        u8 section_kind = 0;
        u64 section_size = 0;

        module_cursor += str8_deserial_read_struct(data, module_cursor, &section_kind);
        module_cursor += str8_deserial_read_uleb128(data, module_cursor, &section_size);

        if(section_kind >= WASM_SectionKind_COUNT)
        {
            // TODO: malformed section ID
        }

        ////////////////
        // ensure correct section ordering and no repeated sections
        //
        if(section_kind != WASM_SectionKind_Custom)
        {
            for(;expected_section_order[expected_section_idx] != section_kind; expected_section_idx += 1)
            {
                if(expected_section_idx >= WASM_SectionKind_COUNT)
                {
                    // unexpected content after last section
                }
            }
        }

        /////////////////////
        // gather section infromation
        //
        {
            String8 *section = &sections[section_kind];
            section->size = section_size;
            section->str = push_array(scratch.arena, u8, section->size);
            module_cursor += str8_deserial_read_array(data, module_cursor, section->str, section->size);
        }
    }

    ///////////////
    // decode sections
    //
    
    // @Todo: custom sections

    ////////////////////
    // decode type section
    //
    if(sections[WASM_SectionKind_Type].size > 0)
    {
        String8 section = sections[WASM_SectionKind_Type];
        u64 section_cursor = 0;
        u64 type_count = 0;
        section_cursor += str8_deserial_read_uleb128(section, section_cursor, &type_count);
        if(type_count)
        {
            module->type_count = type_count;
            module->types = push_array(arena, WASM_Type, module->type_count);

            for(u64 i = 0; i < module->type_count; i += 1)
            {
                WASM_Function_Type *type = &module->types[i];
                u64 param_count = 0;
                u64 result_count = 0;

                u8 kind = 0;
                section_cursor += str8_deserial_read_struct(section, section_cursor, &kind);
                if(kind != WASM_SectionKind_Function)
                {
                    // @TODO parse error
                }

                // resolve parameter count and result count
                section_cursor += str8_deserial_read_uleb128(section, section_cursor,               &param_count);
                (void)            str8_deserial_read_uleb128(section, section_cursor + param_count, &result_count);

                if(param_count > max_u16 || result_count > max_u16)
                {
                    // @TODO parse error
                }

                type->reference_count = 1;
                type->params.size  = param_count;
                type->params.str   = push_array(arena, u8, param_count);
                type->results.size = result_count;
                type->results.str  = push_array(arena, u8, result_count);
                section_cursor += str8_deserial_read_array(section, section_cursor, type->params.str, type->params.size);
                section_cursor += 1;
                section_cursor += str8_deserial_read_array(section, section_cursor, type->results.str, type->results.size);

                // @TODO validate all parameter & result types.

                u64 param_cell_num = wasm_cell_num_from_types(type->params);
                u64 return_cell_num = wasm_cell_num_from_types(type->results);
                if(param_cell_num > max_u16 || return_cell_num > max_u16)
                {
                    // @TODO parse error
                }
                type->param_cell_count = param_cell_num;
                type->return_cell_count = return_cell_num;

                // if there is already a type of this signature, use that instead.
                for(u64 j = 0; j < i; j += 1)
                {
                    if(wasm_type_match(type, &module->types[j]))
                    {
                        if(module->types[j].reference_count == max_u16)
                        {
                            // @TODO ref overflow
                        }
                        // @TODO
                    }
                }
            }
        }
    }

    ///////////////////////
    // decode import section
    //
    if(sections[WASM_SectionKind_Import].size > 0)
    {
        String8 section = sections[WASM_SectionKind_Import];
        u64 section_cursor = 0;
        u64 import_count = 0;
        section_cursor += str8_deserial_read_uleb128(section, section_cursor, &import_count);
        if(import_count)
        {
            module->import_count = import_count;
            module->imports = push_array(arena, WASM_Import, module->import_count);

            for(u64 i = 0; i < module->import_count; i += 1)
            {
                WASM_Import *import = &module->imports[i];
                        
                // load names
                section_cursor += str8_deserial_read_uleb128(section, section_cursor, &import->names.module_name.size);
                import->names.module_name.str = push_array(arena, u8, import->names.module_name.size);
                section_cursor += str8_deserial_read_array(section, section_cursor, import->names.module_name.str, import->names.module_name.size);
                section_cursor += str8_deserial_read_uleb128(section, section_cursor, &import->names.field_name.size);
                import->names.field_name.str = push_array(arena, u8, import->names.field_name.size);
                section_cursor += str8_deserial_read_array(section, section_cursor, import->names.field_name.str, import->names.field_name.size);
                section_cursor += str8_deserial_read_struct(section, section_cursor, &import->kind);
                switch(import->kind)
                {
                    default:
                    {
                        // @TODO: invalid import kind
                    }break;
                    case WASM_ImportKind_Func:
                    {
                        section_cursor += str8_deserial_read_struct(section, section_cursor, &import->function.func_type_index);
                    }break;
                    case WASM_ImportKind_Table:
                    {
                        // @TODO
                    }break;
                    case WASM_ImportKind_Memory:
                    {
                        // @TODO
                    }break;
                    case WASM_ImportKind_Global:
                    {
                        // @TODO
                    }break;
                }
            }
        }
        
        // ensure we've reached eof
        if(section_cursor != section.size)
        {
            // @TODO: handle section size mismatch
        }
    }

    //////////////////
    // decode function & code section
    //
    // NOTE: we load both the function and code sections here in a single
    //       pass.
    if(sections[WASM_SectionKind_Function].size > 0 &&
       sections[WASM_SectionKind_Code].size > 0)
    {
        // we grab the code section here because the equivalent code at
        // https://github.com/wasm-micro-runtime/wasm-micro-runtime/blob/main/core/iwasm/interpreter/wasm_loader.c
        // line 3857 `load_function_section`
        // wants access to the code buffer.
        String8 func_section = sections[WASM_SectionKind_Function];
        String8 code_section = sections[WASM_SectionKind_Code];
        u64 func_section_cursor = 0;
        u64 code_section_cursor = 0;
        u64 func_count = 0;
        u64 code_count = 0;
        func_section_cursor += str8_deserial_read_uleb128(func_section, func_section_cursor, &func_count);
        code_section_cursor += str8_deserial_read_uleb128(code_section, code_section_cursor, &code_count);
        if(func_count == code_count)
        {
            module->function_count = func_count;
            module->functions = push_array(arena, WASM_Function, module->function_count);

            for(u64 i = 0; i <= module->function_count; i += 1)
            {
                // resolve function type
                u64 func_type_index = 0;
                func_section_cursor += str8_deserial_read_uleb128(func_section, func_section_cursor, &func_type_index);
#if 0
                if(module->types[type_index].kind != WASM_TypeKind_Function)
                {
                    // @TODO error handling
                }
#endif
                // @TODO get the smallest func type index

                u64 code_size = 0;
                code_section_cursor += str8_deserial_read_uleb128(code_section, code_section_cursor, &code_size);

                // @HERE
            }
        }
    }

    if(sections[WASM_SectionKind_Table].size > 0)
    {
        String8 section = sections[WASM_SectionKind_Table];
        u64 section_cursor = 0;
        u64 table_count = 0;
        section_cursor += str8_deserial_read_uleb128(section, section_cursor, &table_count);
        if(table_count)
        {
            module->table_count = table_count;
        }
    }

    if(sections[WASM_SectionKind_Memory].size > 0)
    {
        String8 section = sections[WASM_SectionKind_Memory];
        u64 section_cursor = 0;
        u64 memory_count = 0;
        section_cursor += str8_deserial_read_uleb128(section, section_cursor, &memory_count);
        if(memory_count)
        {
            module->memory_count = memory_count;
        }
    }

    if(sections[WASM_SectionKind_Tag].size > 0)
    {
        String8 section = sections[WASM_SectionKind_Tag];
        u64 section_cursor = 0;
        u64 tag_count = 0;
        section_cursor += str8_deserial_read_uleb128(section, section_cursor, &tag_count);
        if(tag_count)
        {
            module->tag_count = tag_count;
        }
    }

    if(sections[WASM_SectionKind_Global].size > 0)
    {
        String8 section = sections[WASM_SectionKind_Global];
        u64 section_cursor = 0;
        u64 global_count = 0;
        section_cursor += str8_deserial_read_uleb128(section, section_cursor, &global_count);
        if(global_count)
        {
            module->global_count = global_count;
        }
    }

    if(sections[WASM_SectionKind_Export].size > 0)
    {
        String8 section = sections[WASM_SectionKind_Export];
        u64 section_cursor = 0;
        u64 export_count = 0;
        section_cursor += str8_deserial_read_uleb128(section, section_cursor, &export_count);
        if(export_count)
        {
            module->export_count = export_count;
        }
    }

    if(sections[WASM_SectionKind_Start].size > 0)
    {
        String8 section = sections[WASM_SectionKind_Start];
        u64 section_cursor = 0;
        u64 start_function_idx = max_u64;
        section_cursor += str8_deserial_read_uleb128(section, section_cursor, &start_function_idx);
        if(start_function_idx != max_u64)
        {
            module->has_start = 1;
            module->start_function_idx = start_function_idx;
        }
    }

    if(sections[WASM_SectionKind_Element].size > 0)
    {
        String8 section = sections[WASM_SectionKind_Element];
        u64 section_cursor = 0;
        u64 element_count = 0;
        section_cursor += str8_deserial_read_uleb128(section, section_cursor, &element_count);
        if(element_count)
        {
            module->element_count = element_count;
        }
    }

    if(sections[WASM_SectionKind_DataCount].size > 0)
    {
        String8 section = sections[WASM_SectionKind_DataCount];
        u64 section_cursor = 0;
        u64 datacount = 0;
        section_cursor += str8_deserial_read_uleb128(section, section_cursor, &datacount);
    }

    if(sections[WASM_SectionKind_Data].size > 0)
    {
        String8 section = sections[WASM_SectionKind_Data];
        u64 section_cursor = 0;
        u64 data_count = 0;
        section_cursor += str8_deserial_read_uleb128(section, section_cursor, &data_count);
        if(data_count)
        {
            module->data_count = data_count;
        }
    }
    
    scratch_end(scratch);
    return module;
}
