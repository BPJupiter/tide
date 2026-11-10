// Copyright (c) Epic Games Tools
// Licensed under the MIT license (https://opensource.org/license/mit/)

// Copyright (c) Frances Telfar
// Licensed under the PolyForm Noncommercial License (https://polyformproject.org/licenses/noncommercial/1.0.0)

//- GENERATED CODE

internal String8
wasm_string_from_type_kind(WASM_TypeKind v)
{
String8 result = str8_lit("<Unknown WASM_TypeKind>");
switch(v)
{
default:{}break;
case WASM_TypeKind_I32:{result = str8_lit("i32");}break;
case WASM_TypeKind_I64:{result = str8_lit("i64");}break;
case WASM_TypeKind_F32:{result = str8_lit("f32");}break;
case WASM_TypeKind_F64:{result = str8_lit("f64");}break;
case WASM_TypeKind_V128:{result = str8_lit("v128");}break;
case WASM_TypeKind_I8:{result = str8_lit("");}break;
case WASM_TypeKind_I16:{result = str8_lit("");}break;
case WASM_TypeKind_NullExceptionRef:{result = str8_lit("");}break;
case WASM_TypeKind_NullFuncRef:{result = str8_lit("");}break;
case WASM_TypeKind_NullExternRef:{result = str8_lit("");}break;
case WASM_TypeKind_NullRef:{result = str8_lit("");}break;
case WASM_TypeKind_FuncRef:{result = str8_lit("");}break;
case WASM_TypeKind_ExternRef:{result = str8_lit("");}break;
case WASM_TypeKind_AnyRef:{result = str8_lit("");}break;
case WASM_TypeKind_EqRef:{result = str8_lit("");}break;
case WASM_TypeKind_I31Ref:{result = str8_lit("");}break;
case WASM_TypeKind_StructRef:{result = str8_lit("");}break;
case WASM_TypeKind_ArrayRef:{result = str8_lit("");}break;
case WASM_TypeKind_ExceptionRef:{result = str8_lit("");}break;
case WASM_TypeKind_HTNonNullable:{result = str8_lit("");}break;
case WASM_TypeKind_HTNullable:{result = str8_lit("");}break;
case WASM_TypeKind_Function:{result = str8_lit("");}break;
case WASM_TypeKind_Struct:{result = str8_lit("");}break;
case WASM_TypeKind_Array:{result = str8_lit("");}break;
case WASM_TypeKind_Sub:{result = str8_lit("");}break;
case WASM_TypeKind_SubFinal:{result = str8_lit("");}break;
case WASM_TypeKind_Recursive:{result = str8_lit("");}break;
case WASM_TypeKind_Void:{result = str8_lit("");}break;
}
return result;
}

