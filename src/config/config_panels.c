// Copyright (c) Epic Games Tools
// Licensed under the MIT license (https://opensource.org/license/mit/)

internal CFG_Node *
cfg_window_from_cfg(CFG_Node *cfg)
{
  CFG_Node *result = &cfg_nil_node;
  for(CFG_Node *c = cfg; c != &cfg_nil_node; c = c->parent)
  {
    if(c->parent->parent == cfg_node_root() && str8_match(c->string, str8_lit("window"), 0))
    {
      result = c;
      break;
    }
  }
  return result;
}

internal CFG_Panel_Tree
cfg_panel_tree_from_cfg(Arena *arena, CFG_Node *cfg_root)
{
  Temp scratch = scratch_begin(&arena, 1);
  CFG_Node *wcfg = cfg_window_from_cfg(cfg_root);
  CFG_Node *src_root = cfg_node_child_from_string(wcfg, str8_lit("panels"));
  CFG_Panel_Node *dst_root = &cfg_nil_panel_node;
  CFG_Panel_Node *dst_focused = &cfg_nil_panel_node;
  {
    Axis2 active_split_axis = cfg_node_child_from_string(wcfg, str8_lit("split_x")) != &cfg_nil_node ? Axis2_X : Axis2_Y;
    CFG_Node_Rec rec = {0};
    CFG_Panel_Node *dst_active_parent = &cfg_nil_panel_node;
    for(CFG_Node *src = src_root; src != &cfg_nil_node; src = rec.next)
    {
      // rjf: build a panel node
      CFG_Panel_Node *dst = push_array(arena, CFG_Panel_Node, 1);
      MemoryCopyStruct(dst, &cfg_nil_panel_node);
      dst->parent = dst_active_parent;
      if(dst_active_parent != &cfg_nil_panel_node)
      {
        DLLPushBack_NPZ(&cfg_nil_panel_node, dst_active_parent->first, dst_active_parent->last, dst, next, prev);
        dst_active_parent->child_count += 1;
      }
      if(dst_root == &cfg_nil_panel_node)
      {
        dst_root = dst;
      }
      
      // rjf: extract cfg info
      bool32 panel_has_children = 0;
      dst->cfg = src;
      dst->pct_of_parent = (src == src_root ? 1.f : (f32)f64_from_str8(src->string));
      dst->tab_side = (cfg_node_child_from_string(src, str8_lit("tabs_on_bottom")) != &cfg_nil_node ? Side_Max : Side_Min);
      dst->split_axis = active_split_axis;
      for(CFG_Node *src_child = src->first; src_child != &cfg_nil_node; src_child = src_child->next)
      {
        MD_Tokenise_Result tokenise = md_tokenise_from_text(scratch.arena, src_child->string);
        if(tokenise.tokens.count == 1 && tokenise.tokens.v[0].flags & MD_TokenFlag_Numeric)
        {
          panel_has_children = 1;
        }
        else if(str8_match(src_child->string, str8_lit("tabs_on_bottom"), 0))
        {
          // NOTE(rjf): skip - this is a panel option.
        }
        else if(str8_match(src_child->string, str8_lit("selected"), 0))
        {
          dst_focused = dst;
        }
        else if(tokenise.tokens.count == 1 && tokenise.tokens.v[0].flags & MD_TokenFlag_Identifier)
        {
          cfg_node_ptr_list_push(arena, &dst->tabs, src_child);
          if(cfg_node_child_from_string(src_child, str8_lit("selected")) != &cfg_nil_node)
          {
            dst->selected_tab = src_child;
          }
        }
      }
      
      // rjf: recurse
      rec = cfg_node_rec__depth_first(src_root, src);
      if(!panel_has_children)
      {
        MemoryZeroStruct(&rec);
        rec.next = &cfg_nil_node;
        for(CFG_Node *p = src; p != src_root && p != &cfg_nil_node; p = p->parent, rec.pop_count += 1)
        {
          if(p->next != &cfg_nil_node)
          {
            rec.next = p->next;
            break;
          }
        }
      }
      if(rec.push_count > 0)
      {
        dst_active_parent = dst;
        active_split_axis = axis2_flip(active_split_axis);
      }
      else for(s32 pop_idx = 0; pop_idx < rec.pop_count; pop_idx += 1)
      {
        dst_active_parent = dst_active_parent->parent;
        active_split_axis = axis2_flip(active_split_axis);
      }
    }
  }
  scratch_end(scratch);
  CFG_Panel_Tree tree = {dst_root, dst_focused};
  return tree;
}

internal CFG_Panel_Node_Rec
cfg_panel_node_rec__depth_first(CFG_Panel_Node *root, CFG_Panel_Node *panel, u64 sib_off, u64 child_off)
{
  CFG_Panel_Node_Rec rec = {&cfg_nil_panel_node};
  if(*MemberFromOffset(CFG_Panel_Node **, panel, child_off) != &cfg_nil_panel_node)
  {
    rec.next = *MemberFromOffset(CFG_Panel_Node **, panel, child_off);
    rec.push_count += 1;
  }
  else for(CFG_Panel_Node *p = panel; p != &cfg_nil_panel_node && p != root; p = p->parent, rec.pop_count += 1)
  {
    if(*MemberFromOffset(CFG_Panel_Node **, p, sib_off) != &cfg_nil_panel_node)
    {
      rec.next = *MemberFromOffset(CFG_Panel_Node **, p, sib_off);
      break;
    }
  }
  return rec;
}

internal CFG_Panel_Node *
cfg_panel_node_from_tree_cfg(CFG_Panel_Node *root, CFG_Node *cfg)
{
  CFG_Panel_Node *result = &cfg_nil_panel_node;
  for(CFG_Panel_Node *p = root;
      p != &cfg_nil_panel_node;
      p = cfg_panel_node_rec__depth_first_pre(root, p).next)
  {
    if(p->cfg == cfg)
    {
      result = p;
      break;
    }
  }
  return result;
}

internal Rng2f32
cfg_target_rect_from_panel_node_child(Rng2f32 parent_rect, CFG_Panel_Node *parent, CFG_Panel_Node *panel)
{
  Rng2f32 rect = parent_rect;
  if(parent != &cfg_nil_panel_node)
  {
    Vec2f32 parent_rect_size = dim_2f32(parent_rect);
    Axis2 axis = parent->split_axis;
    rect.p1.v[axis] = rect.p0.v[axis];
    for(CFG_Panel_Node *child = parent->first; child != &cfg_nil_panel_node; child = child->next)
    {
      rect.p1.v[axis] += parent_rect_size.v[axis] * child->pct_of_parent;
      if(child == panel)
      {
        break;
      }
      rect.p0.v[axis] = rect.p1.v[axis];
    }
    //rect.p0.v[axis] += parent_rect_size.v[axis] * panel->off_pct_of_parent.v[axis];
    //rect.p0.v[axis2_flip(axis)] += parent_rect_size.v[axis2_flip(axis)] * panel->off_pct_of_parent.v[axis2_flip(axis)];
  }
  rect.x0 = round_f32(rect.x0);
  rect.x1 = round_f32(rect.x1);
  rect.y0 = round_f32(rect.y0);
  rect.y1 = round_f32(rect.y1);
  return rect;
}

internal Rng2f32
cfg_target_rect_from_panel_node(Rng2f32 root_rect, CFG_Panel_Node *root, CFG_Panel_Node *panel)
{
  Temp scratch = scratch_begin(0, 0);
  
  // rjf: count ancestors
  u64 ancestor_count = 0;
  for(CFG_Panel_Node *p = panel->parent; p != &cfg_nil_panel_node; p = p->parent)
  {
    ancestor_count += 1;
  }
  
  // rjf: gather ancestors
  CFG_Panel_Node **ancestors = push_array(scratch.arena, CFG_Panel_Node *, ancestor_count);
  {
    u64 ancestor_idx = 0;
    for(CFG_Panel_Node *p = panel->parent; p != &cfg_nil_panel_node; p = p->parent)
    {
      ancestors[ancestor_idx] = p;
      ancestor_idx += 1;
    }
  }
  
  // rjf: go from highest ancestor => panel and calculate rect
  Rng2f32 parent_rect = root_rect;
  for(s64 ancestor_idx = (s64)ancestor_count-1;
      0 <= ancestor_idx && ancestor_idx < ancestor_count;
      ancestor_idx -= 1)
  {
    CFG_Panel_Node *ancestor = ancestors[ancestor_idx];
    CFG_Panel_Node *parent = ancestor->parent;
    if(parent != &cfg_nil_panel_node)
    {
      parent_rect = cfg_target_rect_from_panel_node_child(parent_rect, parent, ancestor);
    }
  }
  
  // rjf: calculate final rect
  Rng2f32 rect = cfg_target_rect_from_panel_node_child(parent_rect, panel->parent, panel);
  
  scratch_end(scratch);
  return rect;
}
