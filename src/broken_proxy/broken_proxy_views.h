
#ifndef BROKEN_PROXY_VIEWS_H
#define BROKEN_PROXY_VIEWS_H

typedef struct BP_Code_View_TLine_Split_Node BP_Code_View_TLine_Split_Node;
struct BP_Code_View_TLine_Split_Node {
    BP_Code_View_TLine_Split_Node *next;
    u64 off;
};

typedef struct BP_Code_View_TLine_Wrap_Cache_Node BP_Code_View_TLine_Wrap_Cache_Node;
struct BP_Code_View_TLine_Wrap_Cache_Node {
    BP_Code_View_TLine_Wrap_Cache_Node *hash_next;
    BP_Code_View_TLine_Wrap_Cache_Node *hash_prev;
    s64 line_num;
    BP_Code_View_TLine_Split_Node *first_split;
    BP_Code_View_TLine_Split_Node *last_split;
};

typedef struct BP_Code_View_TLine_Wrap_Cache_Slot BP_Code_View_TLine_Wrap_Cache_Slot;
struct BP_Code_View_TLine_Wrap_Cache_Slot {
    BP_Code_View_TLine_Wrap_Cache_Node *first;
    BP_Code_View_TLine_Wrap_Cache_Node *last;
};

typedef struct BP_Code_View_State BP_Code_View_State;
struct BP_Code_View_State {
    // stable state
    bool32 initialised;
    s64 preferred_column;
    bool32 drifted_for_search;
    u128 last_hash;

    // patch state
    Arena *patch_arena;
    TXT_Patch_List patches;

    // per-frame command info
    s64 goto_line_num;
    bool32 center_cursor;
    bool32 contain_cursor;
    bool32 force_contain_only;
    Arena *find_text_arena;
    String8 find_text_fwd;
    String8 find_text_bwd;

    // line wrapping cache & info
    Arena *wrap_arena;
    BP_Code_View_TLine_Wrap_Cache_Slot *wrap_cache_slots;
    u64 wrap_cache_slots_count;
    u64 wrap_total_vline_count;
};

////////////////////////
// Code View Functions

internal void bp_code_view_init(BP_Code_View_State *cv);
internal void bp_code_view_build(Arena *arena, BP_Code_View_State *cv, Rng2f32 rect, String8 text_data, TXT_Text_Info *text_info);

///////////////
// View Hooks

BP_VIEW_UI_FUNCTION_DEF(null);
BP_VIEW_UI_FUNCTION_DEF(text);
//BP_VIEW_UI_FUNCTION_DEF(geo3d);
BP_VIEW_UI_FUNCTION_DEF(getting_started);

#endif // BROKEN_PROXY_VIEWS_H
