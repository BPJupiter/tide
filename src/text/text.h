// Copyright (c) Epic Games Tools
// Licensed under the MIT license (https://opensource.org/license/mit/)

#ifndef TEXT_H
#define TEXT_H

////////////////////////////////
//~ rjf: Value Types

typedef enum TXT_LineEndKind
{
  TXT_LineEndKind_Null,
  TXT_LineEndKind_LF,
  TXT_LineEndKind_CRLF,
  TXT_LineEndKind_COUNT
}
TXT_LineEndKind;

typedef enum TXT_TokenKind
{
  TXT_TokenKind_Null,
  TXT_TokenKind_Error,
  TXT_TokenKind_Whitespace,
  TXT_TokenKind_Keyword,
  TXT_TokenKind_Identifier,
  TXT_TokenKind_Numeric,
  TXT_TokenKind_String,
  TXT_TokenKind_Char,
  TXT_TokenKind_Symbol,
  TXT_TokenKind_LineComment,
  TXT_TokenKind_BlockComment,
  TXT_TokenKind_Meta, // preprocessor, etc.
  TXT_TokenKind_COUNT
}
TXT_TokenKind;

typedef struct TXT_Tokeniser_Rule TXT_Tokeniser_Rule;
struct TXT_Tokeniser_Rule
{
  TXT_TokenKind token_kind;
  String8 open_string;
  String8 close_string;
  u32 close_advance;
  bool32 nesting;
  bool32 escaping;
  u32 parent_num;
};

typedef struct TXT_Tokeniser_Rule_Ptr_Node TXT_Tokeniser_Rule_Ptr_Node;
struct TXT_Tokeniser_Rule_Ptr_Node
{
  TXT_Tokeniser_Rule_Ptr_Node *next;
  TXT_Tokeniser_Rule *v;
};

typedef struct TXT_Tokeniser_Rule_Array TXT_Tokeniser_Rule_Array;
struct TXT_Tokeniser_Rule_Array
{
  TXT_Tokeniser_Rule *v;
  u64 count;
};

typedef struct TXT_Token TXT_Token;
struct TXT_Token
{
  TXT_TokenKind kind;
  Rng1u64 range;
};

typedef struct TXT_Token_Pt TXT_Token_Pt;
struct TXT_Token_Pt
{
  TXT_TokenKind kind;
  u64 off;
};

typedef struct TXT_Token_Chunk_Node TXT_Token_Chunk_Node;
struct TXT_Token_Chunk_Node
{
  TXT_Token_Chunk_Node *next;
  u64 count;
  u64 cap;
  TXT_Token *v;
};

typedef struct TXT_Token_Chunk_List TXT_Token_Chunk_List;
struct TXT_Token_Chunk_List
{
  TXT_Token_Chunk_Node *first;
  TXT_Token_Chunk_Node *last;
  u64 chunk_count;
  u64 token_count;
};

typedef struct TXT_Token_Node TXT_Token_Node;
struct TXT_Token_Node
{
  TXT_Token_Node *next;
  TXT_Token v;
};

typedef struct TXT_Token_List TXT_Token_List;
struct TXT_Token_List
{
  TXT_Token_Node *first;
  TXT_Token_Node *last;
  u64 count;
};

typedef struct TXT_Token_Array TXT_Token_Array;
struct TXT_Token_Array
{
  u64 count;
  TXT_Token *v;
};

typedef struct TXT_Token_Array_Array TXT_Token_Array_Array;
struct TXT_Token_Array_Array
{
  u64 count;
  TXT_Token_Array *v;
};

typedef struct TXT_Scope_Node TXT_Scope_Node;
struct TXT_Scope_Node
{
  u64 first_num;
  u64 last_num;
  u64 next_num;
  u64 parent_num;
  Rng1u64 token_idx_range;
};

typedef struct TXT_Scope_Node_Array TXT_Scope_Node_Array;
struct TXT_Scope_Node_Array
{
  TXT_Scope_Node *v;
  u64 count;
};

typedef struct TXT_Scope_Pt TXT_Scope_Pt;
struct TXT_Scope_Pt
{
  u64 token_idx;
  u64 scope_idx;
};

typedef struct TXT_Scope_Pt_Array TXT_Scope_Pt_Array;
struct TXT_Scope_Pt_Array
{
  TXT_Scope_Pt *v;
  u64 count;
};

typedef struct TXT_Text_Info TXT_Text_Info;
struct TXT_Text_Info
{
  u64 lines_count;
  Rng1u64 *lines_ranges;
  u64 lines_max_size;
  TXT_LineEndKind line_end_kind;
  u64 big_token_pts_count;
  TXT_Token_Pt *big_token_pts;
  TXT_Token_Array tokens;
  TXT_Scope_Pt_Array scope_pts;
  TXT_Scope_Node_Array scope_nodes;
  u64 bytes_processed;
  u64 bytes_to_process;
};

typedef struct TXT_Line_Tokens_Slice TXT_Line_Tokens_Slice;
struct TXT_Line_Tokens_Slice
{
  TXT_Token_Array *line_tokens;
};

////////////////////////////////
//~ rjf: Value Modification Patches

typedef struct TXT_Patch TXT_Patch;
struct TXT_Patch
{
  Rng1u64 range;
  String8 replace;
};

typedef struct TXT_Patch_Node TXT_Patch_Node;
struct TXT_Patch_Node
{
  TXT_Patch_Node *next;
  TXT_Patch_Node *prev;
  TXT_Patch v;
};

typedef struct TXT_Patch_List TXT_Patch_List;
struct TXT_Patch_List
{
  TXT_Patch_Node *first;
  TXT_Patch_Node *last;
  u64 count;
};

////////////////////////////////
//~ rjf: Value Reading Types

typedef struct TXT_Line_Map_Range_Node TXT_Line_Map_Range_Node;
struct TXT_Line_Map_Range_Node
{
  TXT_Line_Map_Range_Node *next;
  Rng1u64 num_range;
  Rng1u64 *ranges;
  s64 delta;
};

typedef struct TXT_Line_Map TXT_Line_Map;
struct TXT_Line_Map
{
  TXT_Line_Map_Range_Node *first_range;
  TXT_Line_Map_Range_Node *last_range;
  u64 total_line_count;
};

typedef struct TXT_Token_Pt_Map_Range_Node TXT_Token_Pt_Map_Range_Node;
struct TXT_Token_Pt_Map_Range_Node
{
  TXT_Token_Pt_Map_Range_Node *next;
  Rng1u64 num_range;
  TXT_Token_Pt *pts;
  s64 delta;
};

typedef struct TXT_Token_Pt_Map TXT_Token_Pt_Map;
struct TXT_Token_Pt_Map
{
  TXT_Token_Pt_Map_Range_Node *first_range;
  TXT_Token_Pt_Map_Range_Node *last_range;
  u64 total_pt_count;
};

typedef struct TXT_Patched TXT_Patched;
struct TXT_Patched
{
  Memory_Map memory_map;
  u64 size;
  TXT_Line_Map line_map;
  TXT_Token_Pt_Map token_pt_map;
};

////////////////////////////////
//~ rjf: Generated Code

#include "generated/text.meta.h"

////////////////////////////////
//~ rjf: Language Kind Types

typedef TXT_Token_Array TXT_Lang_Lex_Function_Type(Arena *arena, u64 *bytes_processed_counter, String8 string);

////////////////////////////////
//~ rjf: Globals

read_only global TXT_Scope_Node txt_scope_node_nil = {0};
read_only global Rng1u64 txt_info_line_range_nil = {0};
read_only global TXT_Text_Info txt_info_nil =
{
  1,
  &txt_info_line_range_nil,
  0,
  TXT_LineEndKind_Null,
};

////////////////////////////////
//~ rjf: Basic Helpers

internal TXT_LangKind txt_lang_kind_from_extension(String8 extension);
internal String8 txt_extension_from_lang_kind(TXT_LangKind kind);
internal TXT_LangKind txt_lang_kind_from_arch(Arch arch);
internal TXT_Lang_Lex_Function_Type *txt_lex_function_from_lang_kind(TXT_LangKind kind);

////////////////////////////////
//~ rjf: Token Type Functions

internal void txt_token_chunk_list_push(Arena *arena, TXT_Token_Chunk_List *list, u64 cap, TXT_Token *token);
internal void txt_token_list_push(Arena *arena, TXT_Token_List *list, TXT_Token *token);
internal TXT_Token_Array txt_token_array_from_chunk_list(Arena *arena, TXT_Token_Chunk_List *list);
internal TXT_Token_Array txt_token_array_from_list(Arena *arena, TXT_Token_List *list);

////////////////////////////////
//~ rjf: Patch Functions

internal void txt_patch_list_push_new(Arena *arena, TXT_Patch_List *list, Rng1u64 range, String8 replace);

////////////////////////////////
//~ rjf: Lexing Functions

internal TXT_Token_Array txt_token_array_from_lang_kind_string(Arena *arena, TXT_LangKind lang_kind, String8 string);

internal TXT_Token_Array txt_token_array_from_string__c_cpp(Arena *arena, u64 *bytes_processed_counter, String8 string);
internal TXT_Token_Array txt_token_array_from_string__odin(Arena *arena, u64 *bytes_processed_counter, String8 string);
internal TXT_Token_Array txt_token_array_from_string__jai(Arena *arena, u64 *bytes_processed_counter, String8 string);
internal TXT_Token_Array txt_token_array_from_string__zig(Arena *arena, u64 *bytes_processed_counter, String8 string);
internal TXT_Token_Array txt_token_array_from_string__rust(Arena *arena, u64 *bytes_processed_counter, String8 string);
internal TXT_Token_Array txt_token_array_from_string__disasm_x64_intel(Arena *arena, u64 *bytes_processed_counter, String8 string);

////////////////////////////////
//~ rjf: Text Info Extractor Helpers

internal void txt_line_map_push(Arena *arena, TXT_Line_Map *map, Rng1u64 num_range, Rng1u64 *ranges, s64 delta);
internal u64 txt_line_num_from_off(TXT_Line_Map *map, u64 off);
internal Rng1u64 txt_range_from_line_num(TXT_Line_Map *map, u64 num);
internal void txt_token_pt_map_push(Arena *arena, TXT_Token_Pt_Map *map, Rng1u64 num_range, TXT_Token_Pt *pts, s64 delta);
internal u64 txt_token_pt_num_from_off(TXT_Token_Pt_Map *map, u64 off);
internal TXT_Token_Pt txt_token_pt_from_num(TXT_Token_Pt_Map *map, u64 num);
internal TXT_Token_Array txt_token_array_from_data(Arena *arena, TXT_LangKind lang_kind, TXT_Token_Pt ctx_token_pt, String8 data, u64 base_off, u64 limit);
internal TXT_Patched txt_patched_from_info_data_patches(Arena *arena, TXT_Text_Info *info, String8 data, TXT_Patch_List *patches);

//~ TODO(rjf): old unpatched text viz code:

internal u64 txt_off_from_pt(TXT_Text_Info *info, TXT_Patch_List *patches, Txt_Pt pt);
internal Txt_Pt txt_pt_from_off__linear_scan(TXT_Text_Info *info, TXT_Patch_List *patches, u64 off);
internal TXT_Token_Array txt_token_array_from_info_line_num__linear_scan(TXT_Text_Info *info, s64 line_num);
internal Rng1u64 txt_expr_off_range_from_line_off_range_string_tokens(u64 off, Rng1u64 line_range, String8 line_text, TXT_Token_Array *line_tokens);
internal Rng1u64 txt_expr_off_range_from_info_data_pt(TXT_Text_Info *info, String8 data, Txt_Pt pt);
internal String8 txt_string_from_info_data_txt_rng(TXT_Text_Info *info, String8 data, TXT_Patch_List *patches, Txt_Rng rng);
internal String8 txt_string_from_info_data_line_num(TXT_Text_Info *info, String8 data, s64 line_num);
internal TXT_Line_Tokens_Slice txt_line_tokens_slice_from_info_data_line_range(Arena *arena, TXT_Text_Info *info, String8 data, Rng1s64 line_range);
internal TXT_Scope_Node *txt_scope_node_from_info_num(TXT_Text_Info *info, u64 num);
internal TXT_Scope_Node *txt_scope_node_from_info_off(TXT_Text_Info *info, u64 off);
internal TXT_Scope_Node *txt_scope_node_from_info_pt(TXT_Text_Info *info, TXT_Patch_List *patches, Txt_Pt pt);

////////////////////////////////
//~ rjf: Artifact Cache Hooks / Lookups

internal AC_Artifact txt_artifact_create(String8 key, bool32 *cancel_signal, AC_Status *status_out, u64 *gen_out);
internal void txt_artifact_destroy(AC_Artifact artifact);
internal TXT_Text_Info txt_text_info_from_hash_lang(Access *access, u128 hash, TXT_LangKind lang);
internal TXT_Text_Info txt_text_info_from_key_lang(Access *access, C_Key key, TXT_LangKind lang, u128 *hash_out);

#endif // TEXT_H
