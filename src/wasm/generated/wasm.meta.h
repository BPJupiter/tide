// Copyright (c) Epic Games Tools
// Licensed under the MIT license (https://opensource.org/license/mit/)

// Copyright (c) Frances Telfar
// Licensed under the PolyForm Noncommercial License (https://polyformproject.org/licenses/noncommercial/1.0.0)

//- GENERATED CODE

#ifndef WASM_META_H
#define WASM_META_H

typedef u8 WASM_TypeKind;
typedef enum WASM_TypeKindEnum
{
WASM_TypeKind_I32 = 0x7F,
WASM_TypeKind_I64 = 0x7E,
WASM_TypeKind_F32 = 0x7D,
WASM_TypeKind_F64 = 0x7C,
WASM_TypeKind_V128 = 0x7B,
WASM_TypeKind_I8 = 0x78,
WASM_TypeKind_I16 = 0x77,
WASM_TypeKind_NullExceptionRef = 0x74,
WASM_TypeKind_NullFuncRef = 0x73,
WASM_TypeKind_NullExternRef = 0x72,
WASM_TypeKind_NullRef = 0x71,
WASM_TypeKind_FuncRef = 0x70,
WASM_TypeKind_ExternRef = 0x6F,
WASM_TypeKind_AnyRef = 0x6E,
WASM_TypeKind_EqRef = 0x6D,
WASM_TypeKind_I31Ref = 0x6C,
WASM_TypeKind_StructRef = 0x6B,
WASM_TypeKind_ArrayRef = 0x6A,
WASM_TypeKind_ExceptionRef = 0x69,
WASM_TypeKind_HTNonNullable = 0x64,
WASM_TypeKind_HTNullable = 0x63,
WASM_TypeKind_Function = 0x60,
WASM_TypeKind_Struct = 0x5F,
WASM_TypeKind_Array = 0x5E,
WASM_TypeKind_Sub = 0x50,
WASM_TypeKind_SubFinal = 0x4F,
WASM_TypeKind_Recursive = 0x4E,
WASM_TypeKind_Void = 0x40,
} WASM_TypeKindEnum;

#define WASM_TypeKind_XList \
X(I32, 0x7F, 4)\
X(I64, 0x7E, 8)\
X(F32, 0x7D, 4)\
X(F64, 0x7C, 8)\
X(V128, 0x7B, 16)\
X(I8, 0x78, 1)\
X(I16, 0x77, 1)\
X(NullExceptionRef, 0x74, 4)\
X(NullFuncRef, 0x73, 4)\
X(NullExternRef, 0x72, 4)\
X(NullRef, 0x71, 4)\
X(FuncRef, 0x70, 4)\
X(ExternRef, 0x6F, 4)\
X(AnyRef, 0x6E, 4)\
X(EqRef, 0x6D, 4)\
X(I31Ref, 0x6C, 4)\
X(StructRef, 0x6B, 4)\
X(ArrayRef, 0x6A, 4)\
X(ExceptionRef, 0x69, 4)\
X(HTNonNullable, 0x64, 4)\
X(HTNullable, 0x63, 4)\
X(Function, 0x60, 4)\
X(Struct, 0x5F, 4)\
X(Array, 0x5E, 4)\
X(Sub, 0x50, 4)\
X(SubFinal, 0x4F, 4)\
X(Recursive, 0x4E, 4)\
X(Void, 0x40, 0)\

internal String8 wasm_string_from_type_kind(WASM_TypeKind v);
#endif // WASM_META_H
