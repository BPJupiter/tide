#ifndef TIDE_VIEWS_H
#define TIDE_VIEWS_H

///////////
// Watch View Types
typedef struct TI_Lister_View_State TI_Lister_View_State;
struct TI_Lister_View_State
{
    u64 cursor;
    u64 filter_hash;
};

typedef struct TI_Watch_View_Text_Edit_State TI_Watch_View_Text_Edit_State;
struct TI_Watch_View_Text_Edit_State
{
    TI_Watch_View_Text_Edit_State *pt_hash_next;
    //TI_Watch_Pt pt;
    u64 cursor;
    u64 mark;
    u8 input_buffer[1024];
    u64 input_size;
    u8 initial_buffer[1024];
    u64 initial_size;
};

typedef struct TI_Watch_View_State TI_Watch_View_State;
struct TI_Watch_View_State
{
    bool32 initialized;

    // filter history
    Arena *filter_arena;
    String8 last_filter;

    // table cursor info
/*
    TI_Watch_Pt cursor;
    TI_Watch_Pt mark;
    TI_Watch_Pt next_cursor;
    TI_Watch_Pt next_mark;
*/

    // text input state
    Arena *text_edit_arena;
    u64 text_edit_state_slots_count;
    TI_Watch_View_Text_Edit_State dummy_text_edit_state;
    TI_Watch_View_Text_Edit_State **text_edit_state_slots;
    bool32 text_editing;
};

////////////////
// View Hooks

// TODO: eliminate once we are predeclaring these with metacode

TI_VIEW_UI_FUNCTION_DEF(null);

TI_VIEW_UI_FUNCTION_DEF(text);
TI_VIEW_UI_FUNCTION_DEF(worldmap);

#endif // TIDE_VIEWS_H
