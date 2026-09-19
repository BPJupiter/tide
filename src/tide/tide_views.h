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

////////////////
// View Hooks

// TODO: eliminate once we are predeclaring these with metacode

TI_VIEW_UI_FUNCTION_DEF(null);

TI_VIEW_UI_FUNCTION_DEF(text);
TI_VIEW_UI_FUNCTION_DEF(geo3d);

#endif // TIDE_VIEWS_H
