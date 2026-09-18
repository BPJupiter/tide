#ifndef TIDE_QUERY_H
#define TIDE_QUERY_H

typedef struct TI_Query_Item TI_Query_Item;
struct TI_Query_Item
{
    TI_Query_Item *next;
    String8 display;
    String8 value;
    CFG_ID cfg;
    TI_IconKind icon;
};

typedef struct TI_Query_Item_List TI_Query_Item_List;
struct TI_Query_Item_List
{
    TI_Query_Item *first;
    TI_Query_Item *last;
    u64 count;
};

internal TI_Query_Item_List ti_query_items_from_string(Arena *arena, String8 string, String8 filter);

#endif // TIDE_QUERY_H
