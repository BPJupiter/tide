// Copyright (c) Frances Telfar
// Licensed under the PolyForm Noncommercial License (https://polyformproject.org/licenses/noncommercial/1.0.0)

/////////////////
// Build Options

#define BUILD_TITLE "WASM_INTERPRETER"
#define BUILD_CONSOLE_INTERFACE 1

//////////////
// Includes

#include "base/base_inc.h"
#include "wasm/wasm_inc.h"

#include "base/base_inc.c"
#include "wasm/wasm_inc.c"

internal void
printf_with_indent(u64 indent_count, char *fmt, ...)
{
    Temp scratch = scratch_begin(0, 0);
    String8 indent = {0};
    indent.size = indent_count*2;
    indent.str = push_array(scratch.arena, u8, indent.size);
    MemorySet(indent.str, ' ', indent.size);
    fprintf(stdout, "%.*s", str8_varg(indent));
    
    va_list args;
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    va_end(args);
    
    scratch_end(scratch);
}

internal void
entry_point(Cmd_Line *cmdline)
{
    Temp scratch = scratch_begin(0, 0);
    u64 exit_code = 0;

    String8 data = data_from_file_path(scratch.arena, cmdline->inputs.first->string);
    if(data.size == 0)
    {
        fprintf(stderr, "ERROR: input file was not read %.*s\n", str8_varg(cmdline->inputs.first->string));
        goto exit;
    }
    WASM_Module *module = wasm_module_decode(scratch.arena, data);

    printf_with_indent(0, "(module\n");
    for(u64 i = 0; i < module->type_count; i += 1)
    {
        String8_List param_list = {0};
        String8_List result_list = {0};
        for(u64 j = 0; j < module->types[i].params.size; j += 1)
        {
            String8 type_identifier = wasm_string_from_type_kind(module->types[i].params.str[j]);
            str8_list_push(scratch.arena, &param_list, type_identifier);
        }
        for(u64 j = 0; j < module->types[i].results.size; j += 1)
        {
            String8 type_identifier = wasm_string_from_type_kind(module->types[i].results.str[j]);
            str8_list_push(scratch.arena, &result_list, type_identifier);
        }
        String_Join param_join  = {.pre = s(" (param "),  .sep = s(" "), .post = s(")")};
        String_Join result_join = {.pre = s(" (result "), .sep = s(" "), .post = s(")")};
        String8 params  = param_list.node_count  > 0 ? str8_list_join(scratch.arena, &param_list,  &param_join)  : str8_zero();
        String8 results = result_list.node_count > 0 ? str8_list_join(scratch.arena, &result_list, &result_join) : str8_zero();
        printf_with_indent(1, "(type (;%I64u;) (func%.*s%.*s))\n", i, str8_varg(params), str8_varg(results));
    }
    for(u64 i = 0; i < module->import_count; i += 1)
    {
        String8 import_string = str8f(scratch.arena, "(import \"%S\" \"%S\" (%S (;%I64u;) (type %I64u)", module->imports[i].module_name, module->imports[i].field_name, wasm_string_from_import_kind(module->imports[i].kind), i, module->imports[i].function.func_type_index);
        printf_with_indent(1, "%.*s\n", str8_varg(import_string));
    }
    fprintf(stdout, "Function Count: %I64u\n", module->function_count);
    fprintf(stdout, "Table Count:    %I64u\n", module->table_count);
    fprintf(stdout, "Memory Count:   %I64u\n", module->memory_count);
    fprintf(stdout, "Tag Count:      %I64u\n", module->tag_count);
    fprintf(stdout, "Global Count:   %I64u\n", module->global_count);
    fprintf(stdout, "Export Count:   %I64u\n", module->export_count);
    fprintf(stdout, "Element Count:  %I64u\n", module->element_count);
    fprintf(stdout, "Data Count:     %I64u\n", module->data_count);

 exit:;
    scratch_end(scratch);
}
