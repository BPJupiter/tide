// Copyright (c) Epic Games Tools
// Licensed under the MIT license (https://opensource.org/license/mit/)

#ifndef CONFIG_PANELS_H
#define CONFIG_PANELS_H

typedef struct CFG_Panel_Node CFG_Panel_Node;
struct CFG_Panel_Node
{
  // rjf: links data
  CFG_Panel_Node *first;
  CFG_Panel_Node *last;
  CFG_Panel_Node *next;
  CFG_Panel_Node *prev;
  CFG_Panel_Node *parent;
  u64 child_count;
  CFG_Node *cfg;
  
  // rjf: split data
  Axis2 split_axis;
  f32 pct_of_parent;
  
  // rjf: tab params
  Side tab_side;
  
  // rjf: which tabs are attached
  CFG_Node_Ptr_List tabs;
  CFG_Node *selected_tab;
};

typedef struct CFG_Panel_Tree CFG_Panel_Tree;
struct CFG_Panel_Tree
{
  CFG_Panel_Node *root;
  CFG_Panel_Node *focused;
};

typedef struct CFG_Panel_Node_Rec CFG_Panel_Node_Rec;
struct CFG_Panel_Node_Rec
{
  CFG_Panel_Node *next;
  s32 push_count;
  s32 pop_count;
};

read_only global CFG_Panel_Node cfg_nil_panel_node =
{
  &cfg_nil_panel_node,
  &cfg_nil_panel_node,
  &cfg_nil_panel_node,
  &cfg_nil_panel_node,
  &cfg_nil_panel_node,
  0,
  &cfg_nil_node,
  .selected_tab = &cfg_nil_node,
};

internal CFG_Node *cfg_window_from_cfg(CFG_Node *cfg);
internal CFG_Panel_Tree cfg_panel_tree_from_cfg(Arena *arena, CFG_Node *cfg_root);
internal CFG_Panel_Node_Rec cfg_panel_node_rec__depth_first(CFG_Panel_Node *root, CFG_Panel_Node *panel, u64 sib_off, u64 child_off);
#define cfg_panel_node_rec__depth_first_pre(root, p)     cfg_panel_node_rec__depth_first((root), (p), OffsetOf(CFG_Panel_Node, next), OffsetOf(CFG_Panel_Node, first))
#define cfg_panel_node_rec__depth_first_pre_rev(root, p) cfg_panel_node_rec__depth_first((root), (p), OffsetOf(CFG_Panel_Node, prev), OffsetOf(CFG_Panel_Node, last))
internal CFG_Panel_Node *cfg_panel_node_from_tree_cfg(CFG_Panel_Node *root, CFG_Node *cfg);
internal Rng2f32 cfg_target_rect_from_panel_node_child(Rng2f32 parent_rect, CFG_Panel_Node *parent, CFG_Panel_Node *panel);
internal Rng2f32 cfg_target_rect_from_panel_node(Rng2f32 root_rect, CFG_Panel_Node *root, CFG_Panel_Node *panel);

#endif // CONFIG_PANELS_H
