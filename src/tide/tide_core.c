
/////////////////////
// Generated Code

#include "generated/tide.meta.c"

/////////////////////////////
// Main Layer Top-Level Calls

#if !defined(STBI_INCLUDE_STB_IMAGE_H)
# define STB_IMAGE_IMPLEMENTATION
# define STBI_ONLY_PNG
# define STBI_ONLY_BMP
# include "third_party/stb/stb_image.h"
#endif

///////////////////////
// Window Functions

internal void ti_window_frame(void)
{
    Temp scratch = scratch_begin(0, 0);
    ProfBeginFunction();

    /////////////////////////////////////
    // @window_frame_part unpack context
    //
    TI_Window_State *ws = ti_state->window_state;

    /////////////////////////////////////
    // @window_frame_part compute window's frame
    //
    {
        Access *access = access_open();

        typedef struct TI_Color_Def TI_Color_Def;
        struct TI_Color_Def
        {
            char *tags[2];
            u32   tag_count;
            u32   srgba;
        };
        TI_Color_Def defs[] = {
            { {"background"},           1, 0x14161AFF }, // window clear color - near-black
            { {"border"},                1, 0x4A4E58FF }, // window border / box borders - mid gray
            { {"drop_shadow"},           1, 0x000000AA }, // shadow under floating/hot boxes
            { {"text"},                  1, 0xF2F2F2FF }, // default text - near-white
            { {"text", "weak"},          2, 0x9AA0AAFF }, // de-emphasized text/icons
            { {"hover"},                 1, 0x3C82F6FF }, // hot/hover highlight - blue
            { {"focus", "overlay"},      2, 0x3C82F633 }, // focus fill overlay - translucent blue
            { {"focus", "border"},       2, 0x3C82F6FF }, // focus outline - solid blue
            { {"bad", "text"},           2, 0xFF5C5CFF }, // error state text/border - red
            { {"match", "background"},   2, 0xF6C64555 }, // fuzzy-match highlight - amber
        };
        
        // convert to final pattern array
        ws->theme = push_array(ti_frame_arena(), UI_Theme, 1);
        ws->theme->patterns = push_array(ti_frame_arena(), UI_Theme_Pattern, ArrayCount(defs));
        ws->theme->patterns_count = ArrayCount(defs);
        for (u64 i = 0; i < ArrayCount(defs); i += 1)
        {
            TI_Color_Def *def = &defs[i];
            String8 *tag_strs = push_array(ti_frame_arena(), String8, def->tag_count);
            for (u32 t = 0; t < def->tag_count; t += 1)
            {
                tag_strs[t] = str8_cstring(def->tags[t]);
            }
            ws->theme->patterns[i].tags  = (String8_Array){tag_strs, def->tag_count};
            ws->theme->patterns[i].linear = linear_from_srgba(rgba_from_u32(def->srgba));
        }

        access_close(access);
    }
    
    //////////////////////////////
    // @window_frame_part build UI
    ProfScope("build UI")
    {
        //////////////////////////
        // @window_ui_part set up
        {
            // get font size info
            f32 font_size = 16.f;

            // build icon info
            UI_Icon_Info icon_info = {0};
            {
            }

            // build animation info
            UI_Animation_Info animation_info = {0};
            {
                animation_info.hot_animation_rate      = 5.f;
                animation_info.active_animation_rate   = 5.f;
                animation_info.focus_animation_rate    = 1.f;
                animation_info.tooltip_animation_rate  = 4.f;
                animation_info.menu_animation_rate     = 3.f;
                animation_info.scroll_animation_rate   = 6.f;
            }

            // begin & push initial stack values
            ui_begin_build(ws->os,
                           &ws->ui_events,
                           &icon_info,
                           ws->theme,
                           &animation_info,
                           ti_state->frame_dt,
                           ti_state->frame_dt);
            ui_push_font(fnt_tag_from_static_data_string(&ti_default_main_font_bytes));
            ui_push_font_size(font_size);
        }

        // @window_ui_part calculate top-level rectangles/sizes
        Rng2f32 window_rect = wm_client_rect_from_window(ws->os);
        Vec2f32 window_rect_dim = dim_2f32(window_rect);

        //////////////////////////////////////////
        // @window_ui_part button test
        {
            // absolute
            f32 width = 200.f;
            f32 height = ui_top_font_size() * 2.f;
            Vec2f32 center = center_2f32(window_rect);
            Rng2f32 button_rect = r2f32p(center.x - width * 0.5f, center.y - height * 0.5f,
                                         center.x + width * 0.5f, center.y + height * 0.5f);
            ui_set_next_fixed_x(button_rect.x0);
            ui_set_next_fixed_y(button_rect.y0);
            ui_set_next_fixed_width(dim_2f32(button_rect).x);
            ui_set_next_fixed_height(dim_2f32(button_rect).y);
            UI_Box *pane = ui_build_box_from_stringf(UI_BoxFlag_Floating, "###button_test");
            UI_Parent(pane)
            {
                UI_Signal sig = ui_button(str8_lit("Click me"));
                if (ui_clicked(sig))
                {
                    sh_message(0, s("button clicked"), s("I have been clicked"));
                }
            }

            // in div
            {
                UI_Box *center_row;
                UI_WidthFill UI_HeightFill
                {
                    center_row = ui_build_box_from_stringf(0, "center_row");
                    center_row->child_layout_axis = Axis2_X;
                }
                UI_Parent(center_row)
                {
                    ui_spacer(ui_pct(1, 0));

                    UI_Box *center_col = ui_build_box_from_stringf(0, "center_col");
                    center_col->child_layout_axis = Axis2_Y;
                    UI_Parent(center_col)
                        UI_PrefWidth(ui_px(200, 1.f))
                    {
                        ui_spacer(ui_pct(1, 0));

                        UI_Box *container = ui_build_box_from_stringf(0, "container");
                        UI_Parent(container)
                            UI_PrefWidth(ui_px(200, 1.f))
                            UI_PrefHeight(ui_em(1.f, 1.f))
                            UI_BackgroundColor(v4f32(0.1f, 0.1f, 0.1f, 1.f))
                        {
                            UI_Signal sig = ui_button(str8_lit("Click me"));
                            if (ui_clicked(sig))
                            {
                                sh_message(0, s("button clicked"), s("I have been clicked"));
                            }
                        }

                        ui_spacer(ui_pct(1, 0));
                    }

                    ui_spacer(ui_pct(1, 0));
                }
            }
        }

        ui_end_build();
    }

    ///////////////////////////////
    // @window_frame_part draw UI
    //
    ws->draw_bucket = dr_bucket_make();
    DR_BucketScope(ws->draw_bucket)
        ProfScope("draw UI")
    {
        Temp scratch = scratch_begin(0, 0);
        f32 box_squish_epsilon = 0.001f;
        Rng2f32 window_rect = wm_client_rect_from_window(ws->os);

        // unpack settings
        f32 rounded_corner_amount = 0.5f;
        f32 border_softness       = 1.f;
        bool32 do_background_blur                = true;
        bool32 force_opaque_floating_backgrounds = true;
        bool32 do_drop_shadows                   = true;
        Vec4f32 base_background_color = ui_color_from_name(str8_lit("background"));
        Vec4f32 base_border_color     = ui_color_from_name(str8_lit("border"));
        Vec4f32 drop_shadow_color     = ui_color_from_name(str8_lit("drop_shadow"));

        // set up heatmap buckets
        f32 heatmap_bucket_size = 32.f;
        u64 *heatmap_buckets = 0;
        u64 heatmap_bucket_pitch = 0;
        u64 heatmap_bucket_count = 0;
        if (DEV_draw_ui_box_heatmap)
        {
            Rng2f32 rect = wm_client_rect_from_window(ws->os);
            Vec2f32 size = dim_2f32(rect);
            Vec2s32 buckets_dim = {(s32)(size.x/heatmap_bucket_size), (s32)(size.y/heatmap_bucket_size)};
            heatmap_bucket_pitch = buckets_dim.x;
            heatmap_bucket_count = buckets_dim.x * buckets_dim.y;
            heatmap_buckets = push_array(scratch.arena, u64, heatmap_bucket_count);
        }

        // draw background color
        {
            dr_rect(window_rect, base_background_color, 0, 0, 0);
        }

        // draw window border
        {
            dr_rect(window_rect, base_border_color, 0, 1.f, border_softness * 0.5f);
        }

        // recurse & draw
        u64 total_heatmap_sum_count = 0;
        UI_Box *hover_debug_box = &ui_nil_box;
        for (UI_Box *box = ui_root_from_state(ws->ui); !ui_box_is_nil(box);)
        {
            // get corner radii
            f32 box_corner_radii[Corner_COUNT] = {
                box->corner_radii[Corner_00] * rounded_corner_amount,
                box->corner_radii[Corner_01] * rounded_corner_amount,
                box->corner_radii[Corner_10] * rounded_corner_amount,
                box->corner_radii[Corner_11] * rounded_corner_amount,
            };

            // get recursion
            UI_Box_Rec rec = ui_box_rec_df_post(box, &ui_nil_box);

            // sum to box heatmap
            if (DEV_draw_ui_box_heatmap)
            {
                Vec2f32 center = center_2f32(box->rect);
                Vec2s32 p = v2s32(center.x / heatmap_bucket_size, center.y / heatmap_bucket_size);
                u64 bucket_idx = p.y * heatmap_bucket_pitch + p.x;
                if (bucket_idx < heatmap_bucket_count)
                {
                    heatmap_buckets[bucket_idx] += 1;
                    total_heatmap_sum_count += 1;
                }
            }

            // grab if debug
            if (box->flags & UI_BoxFlag_Debug && contains_2f32(box->rect, ui_mouse()))
            {
                hover_debug_box = box;
            }
            
            // push transparency
            if (box->transparency != 0)
            {
                dr_push_transparency(box->transparency);
            }

            // push squish
            if (box->squish > box_squish_epsilon)
            {
                Vec2f32 box_dim = dim_2f32(box->rect);
                Vec2f32 anchor_off = {0};
                if (box->flags & UI_BoxFlag_SquishAnchored)
                {
                    anchor_off.x = box_dim.x/2.f;
                }
                else
                {
                    anchor_off.y = -box_dim.y/8.f;
                }
                Mat3x3f32 box2origin_xform = make_translate_3x3f32(v2f32(-box->rect.x0 - box_dim.x/2 + anchor_off.x,
                                                                         -box->rect.y0               + anchor_off.y));
                Mat3x3f32 scale_xform = make_scale_3x3f32(v2f32(1 - box->squish, 1 - box->squish));
                Mat3x3f32 origin2box_xform = make_translate_3x3f32(v2f32(box->rect.x0 + box_dim.x/2 - anchor_off.x,
                                                                         box->rect.y0               - anchor_off.y));
                Mat3x3f32 xform = mul_3x3f32(origin2box_xform, mul_3x3f32(scale_xform, box2origin_xform));
                dr_push_xform2d(xform);
                dr_push_tex2d_sample_kind(R_Tex2DSampleKind_Linear);
            }

            // draw drop shadow
            if (do_drop_shadows && box->flags & UI_BoxFlag_DrawDropShadow)
            {
                Rng2f32 drop_shadow_rect = shift_2f32(pad_2f32(box->rect, 8), v2f32(4, 4));
                R_Rect2D_Inst *inst = dr_rect(drop_shadow_rect, drop_shadow_color, 0.8f, 0, 8.f);
                MemoryCopyArray(inst->corner_radii, box_corner_radii);
            }

            // blur background
            if (do_background_blur && box->flags & UI_BoxFlag_DrawBackgroundBlur)
            {
                R_Pass_Params_Blur *params = dr_blur(pad_2f32(box->rect, 1.f),
                                                     box->blur_size * (1 - box->transparency),
                                                     0);
                MemoryCopyArray(params->corner_radii, box_corner_radii);
            }

            // compute effective active t
            f32 effective_active_t = box->active_t;
            if (!(box->flags & UI_BoxFlag_DrawActiveEffects))
            {
                effective_active_t = 0;
            }
            f32 t = box->hot_t * (1 - effective_active_t);

            // compute background color
            Vec4f32 box_background_color = box->background_color;
            if (force_opaque_floating_backgrounds &&
                box->flags & UI_BoxFlag_Floating && box->flags & UI_BoxFlag_DrawDropShadow)
            {
                box_background_color.w = 1.f;
            }

            // draw background
            if (box->flags & UI_BoxFlag_DrawBackground)
            {
                Rng2f32 box_bg_rect = r2f32p(box->rect.x0 + box->visual_margin.x,
                                             box->rect.y0 + box->visual_margin.y,
                                             box->rect.x1 - box->visual_margin.x,
                                             box->rect.y1 - box->visual_margin.y);

                // hot effect extension (drop shadow)
                if (box->flags & UI_BoxFlag_DrawHotEffects)
                {
                    Rng2f32 drop_shadow_rect = shift_2f32(pad_2f32(box_bg_rect, 8), v2f32(4, 4));
                    Vec4f32 color = drop_shadow_color;
                    color.w *=t * box_background_color.w;
                    dr_rect(drop_shadow_rect, color, 0.8f, 0, 8.f);
                }

                // draw background
                R_Rect2D_Inst *inst = dr_rect(pad_2f32(box_bg_rect, 1.f),
                                              box_background_color, 0, 0, border_softness * 1.f);
                MemoryCopyArray(inst->corner_radii, box_corner_radii);

                // hot effect extension
                if (box->flags & UI_BoxFlag_DrawHotEffects)
                {
                    bool32 is_hot = !ui_key_match(box->key, ui_key_zero()) && ui_key_match(box->key, ui_hot_key());
                    Vec4f32 hover_color = ui_color_from_tags_key_name(box->tags_key, str8_lit("hover"));

                    // brighten
                    if (is_hot)
                    {
                        Vec4f32 color = hover_color;
                        color.w *= 0.015f;
                        R_Rect2D_Inst *inst = dr_rect(pad_2f32(box_bg_rect, 1.f),
                                                      v4f32(0, 0, 0, 0), 0, 0, border_softness * 1.f);
                        inst->colors[Corner_00] = color;
                        inst->colors[Corner_10] = color;
                        inst->colors[Corner_01] = color;
                        inst->colors[Corner_11] = color;
                        MemoryCopyArray(inst->corner_radii, box_corner_radii);
                    }

                    // soft circle around mouse
                    if (box->hot_t > 0.01f && dim_2f32(box->rect).x > box->font_size * 8.f) DR_ClipScope(intersect_2f32(box_bg_rect, dr_top_clip()))
                    {
                        Vec4f32 color = hover_color;
                        color.w *= 0.025f;
                        if (!is_hot)
                        {
                            color.w *= t;
                        }
                        Vec2f32 center = ui_mouse();
                        Vec2f32 box_dim = dim_2f32(box->rect);
                        f32 max_dim = Max(box_dim.x, box_dim.y);
                        f32 radius = box->font_size * 24.f;
                        radius = Min(max_dim, radius);
                        dr_rect(pad_2f32(r2f32(center, center), radius * 2.f), color, radius, 0, radius / 2.f);
                    }
                }

                // active effect extension
                if (box->flags & UI_BoxFlag_DrawActiveEffects)
                {
                    Vec4f32 shadow_color = drop_shadow_color;
                    shadow_color.w *= 0.5 * box->active_t;
                    Vec2f32 shadow_size = {
                        (box_bg_rect.x1 - box_bg_rect.x0) * 0.60f * box->active_t,
                        (box_bg_rect.y1 - box_bg_rect.y0) * 0.60f * box->active_t,
                    };
                    shadow_size.x = Clamp(0, shadow_size.x, box->font_size * 2.f);
                    shadow_size.y = Clamp(0, shadow_size.y, box->font_size * 2.f);

                    // top -> bottom dark effect
                    {
                        R_Rect2D_Inst *inst = dr_rect(r2f32p(box_bg_rect.x0,
                                                             box_bg_rect.y0,
                                                             box_bg_rect.x1,
                                                             box_bg_rect.y0 + shadow_size.y),
                                                      v4f32(0, 0, 0, 0),
                                                      0, 0, 1.f);
                        inst->colors[Corner_00] = inst->colors[Corner_10] = shadow_color;
                        inst->colors[Corner_01] = inst->colors[Corner_11] = v4f32(0.f, 0.f, 0.f, 0.0f);
                        MemoryCopyArray(inst->corner_radii, box_corner_radii);
                    }
                    
                    // rjf: bottom -> top light effect
                    {
                        R_Rect2D_Inst *inst = dr_rect(r2f32p(box_bg_rect.x0,
                                                             box_bg_rect.y1 - shadow_size.y,
                                                             box_bg_rect.x1,
                                                             box_bg_rect.y1),
                                                      v4f32(0, 0, 0, 0),
                                                      0, 0, 1.f);
                        inst->colors[Corner_00] = inst->colors[Corner_10] = v4f32(0, 0, 0, 0);
                        inst->colors[Corner_01] = inst->colors[Corner_11] = v4f32(1.0f, 1.0f, 1.0f, 0.08f*box->active_t);
                        MemoryCopyArray(inst->corner_radii, box_corner_radii);
                    }
          
                    // rjf: left -> right dark effect
                    {
                        R_Rect2D_Inst *inst = dr_rect(r2f32p(box_bg_rect.x0,
                                                             box_bg_rect.y0,
                                                             box_bg_rect.x0 + shadow_size.x,
                                                             box_bg_rect.y1),
                                                      v4f32(0, 0, 0, 0),
                                                      0, 0, 1.f);
                        inst->colors[Corner_10] = inst->colors[Corner_11] = v4f32(0.f, 0.f, 0.f, 0.f);
                        inst->colors[Corner_00] = shadow_color;
                        inst->colors[Corner_01] = shadow_color;
                        MemoryCopyArray(inst->corner_radii, box_corner_radii);
                    }
                    
                    // rjf: right -> left dark effect
                    {
                        R_Rect2D_Inst *inst = dr_rect(r2f32p(box_bg_rect.x1 - shadow_size.x,
                                                             box_bg_rect.y0,
                                                             box_bg_rect.x1,
                                                             box_bg_rect.y1),
                                                      v4f32(0, 0, 0, 0),
                                                      0, 0, 1.f);
                        inst->colors[Corner_00] = inst->colors[Corner_01] = v4f32(0.f, 0.f, 0.f, 0.f);
                        inst->colors[Corner_10] = shadow_color;
                        inst->colors[Corner_11] = shadow_color;
                        MemoryCopyArray(inst->corner_radii, box_corner_radii);
                    }
                }
            }

            // draw string
            if(box->flags & UI_BoxFlag_DrawText)
            {
                Vec2f32 text_position = ui_box_text_position(box);
                if(DEV_draw_ui_text_pos)
                {
                    dr_rect(r2f32p(text_position.x-4,
                                   text_position.y-4,
                                   text_position.x+4,
                                   text_position.y+4),
                            v4f32(1, 0, 1, 1), 1, 0, 1);
                }
                f32 max_x = 100000.f;
                FNT_Run ellipses_run = {0};
                if(!(box->flags & UI_BoxFlag_DisableTextTrunc))
                {
                    FNT_Tag ellipses_font = box->font;
                    f32 ellipses_size = box->font_size;
                    FNT_RasterFlags ellipses_raster_flags = box->text_raster_flags;
                    if(box->display_fstrs.last)
                    {
                        ellipses_font = box->display_fstrs.last->v.params.font;
                        ellipses_size = box->display_fstrs.last->v.params.size;
                        ellipses_raster_flags = box->display_fstrs.last->v.params.raster_flags;
                    }
                    max_x = (box->rect.x1-text_position.x);
                    ellipses_run = fnt_run_from_string(ellipses_font,
                                                       ellipses_size,
                                                       0,
                                                       box->tab_size,
                                                       ellipses_raster_flags,
                                                       str8_lit("..."));
                }
                if(box->flags & UI_BoxFlag_HasFuzzyMatchRanges) UI_TagF("match")
                {
                    Vec4f32 match_color = ui_color_from_tags_key_name(ui_top_tags_key(), str8_lit("background"));
                    dr_truncated_fancy_run_fuzzy_matches(text_position,
                                                         &box->display_fruns,
                                                         max_x,
                                                         &box->fuzzy_match_ranges,
                                                         match_color);
                }
                dr_truncated_fancy_run_list(text_position, &box->display_fruns, max_x, ellipses_run);
            }

            // draw focus viz
            if(DEV_draw_ui_focus_debug)
            {
                bool32 focused = (box->flags & (UI_BoxFlag_FocusHot|UI_BoxFlag_FocusActive) &&
                               box->flags & UI_BoxFlag_Clickable);
                bool32 disabled = 0;
                for(UI_Box *p = box; !ui_box_is_nil(p); p = p->parent)
                {
                    if(p->flags & (UI_BoxFlag_FocusHotDisabled|UI_BoxFlag_FocusActiveDisabled))
                    {
                        disabled = 1;
                        break;
                    }
                }
                if(focused)
                {
                    Vec4f32 color = v4f32(0.3f, 0.8f, 0.3f, 1.f);
                    if(disabled)
                    {
                        color = v4f32(0.8f, 0.3f, 0.3f, 1.f);
                    }
                    dr_rect(r2f32p(box->rect.x0-6,
                                   box->rect.y0-6,
                                   box->rect.x0+6,
                                   box->rect.y0+6),
                            color, 2, 0, 1);
                    dr_rect(box->rect, color, 2, 2, 1);
                }
                if(box->flags & (UI_BoxFlag_FocusHot|UI_BoxFlag_FocusActive))
                {
                    if(box->flags & (UI_BoxFlag_FocusHotDisabled|UI_BoxFlag_FocusActiveDisabled))
                    {
                        dr_rect(r2f32p(box->rect.x0-6,
                                       box->rect.y0-6,
                                       box->rect.x0+6,
                                       box->rect.y0+6),
                                v4f32(1, 0, 0, 0.2f), 2, 0, 1);
                    }
                    else
                    {
                        dr_rect(r2f32p(box->rect.x0-6,
                                       box->rect.y0-6,
                                       box->rect.x0+6,
                                       box->rect.y0+6),
                                v4f32(0, 1, 0, 0.2f),
                                2, 0, 1);
                    }
                }
            }

            // rjf: push clip
            if(box->flags & UI_BoxFlag_Clip)
            {
                Rng2f32 top_clip = dr_top_clip();
                Rng2f32 new_clip = pad_2f32(box->rect, -1);
                if(top_clip.x1 != 0 || top_clip.y1 != 0)
                {
                    new_clip = intersect_2f32(new_clip, top_clip);
                }
                dr_push_clip(new_clip);
            }
            
            // rjf: custom draw list
            if(box->flags & UI_BoxFlag_DrawBucket)
            {
                Mat3x3f32 xform = make_translate_3x3f32(box->position_delta);
                DR_XForm2DScope(xform)
                {
                    dr_sub_bucket(box->draw_bucket);
                }
            }
            
            // rjf: call custom draw callback
            if(box->custom_draw != 0)
            {
                box->custom_draw(box, box->custom_draw_user_data);
            }
            
            // rjf: pop
            {
                s32 pop_idx = 0;
                for(UI_Box *b = box; !ui_box_is_nil(b) && pop_idx <= rec.pop_count; b = b->parent)
                {
                    pop_idx += 1;
                    if(b == box && rec.push_count != 0)
                    {
                        continue;
                    }
                    
                    // rjf: pop clips
                    if(b->flags & UI_BoxFlag_Clip)
                    {
                        dr_pop_clip();
                    }
                    
                    // rjf: get corner radii
                    f32 b_corner_radii[Corner_COUNT] =
                        {
                            b->corner_radii[Corner_00]*rounded_corner_amount,
                            b->corner_radii[Corner_01]*rounded_corner_amount,
                            b->corner_radii[Corner_10]*rounded_corner_amount,
                            b->corner_radii[Corner_11]*rounded_corner_amount,
                        };
                    
                    // rjf: draw border
                    if(b->flags & UI_BoxFlag_DrawBorder)
                    {
                        Vec4f32 border_color = b->border_color;
                        Rng2f32 b_border_rect = r2f32p(b->rect.x0 - 1.f + b->visual_margin.x,
                                                       b->rect.y0 - 1.f + b->visual_margin.y,
                                                       b->rect.x1 + 1.f - b->visual_margin.x,
                                                       b->rect.y1 + 1.f - b->visual_margin.y);
                        R_Rect2D_Inst *inst = dr_rect(b_border_rect, border_color, 0, 1.f, border_softness*1.f);
                        MemoryCopyArray(inst->corner_radii, b_corner_radii);
                        
                        // rjf: hover effect
                        if(b->flags & UI_BoxFlag_DrawHotEffects)
                        {
                            Vec4f32 color = ui_color_from_tags_key_name(box->tags_key, str8_lit("hover"));
                            if(ui_key_match(b->key, ui_key_zero()) || !ui_key_match(b->key, ui_hot_key()))
                            {
                                color.w *= b->hot_t;
                            }
                            color.w *= 0.01f;
                            R_Rect2D_Inst *inst = dr_rect(b_border_rect, color, 0, 1.f, 1.f);
                            MemoryCopyArray(inst->corner_radii, b_corner_radii);
                        }
                    }
                    
                    // rjf: draw scroll fade
                    if(b->flags & (UI_BoxFlag_DrawFadeTop|UI_BoxFlag_DrawFadeBottom|UI_BoxFlag_DrawFadeLeft|UI_BoxFlag_DrawFadeRight))
                    {
                        Vec2f32 fade_dim = scale_2f32(dim_2f32(b->rect), 0.05f);
                        Vec4f32 fade_color = drop_shadow_color;
                        if(b->flags & UI_BoxFlag_DrawFadeTop)
                        {
                            f32 t = ui_anim(ui_key_from_string(b->key, s("fade_top")), 1.f, .rate = ti_state->catchall_animation_rate);
                            Rng2f32 rect = r2f32p(b->rect.x0, b->rect.y0, b->rect.x1, b->rect.y0 + fade_dim.y*t);
                            R_Rect2D_Inst *r = dr_rect(rect, fade_color, 0, 0, 0);
                            MemoryCopyArray(r->corner_radii, b_corner_radii);
                            r->colors[Corner_01] = r->colors[Corner_11] = v4f32(0, 0, 0, 0);
                        }
                        if(b->flags & UI_BoxFlag_DrawFadeBottom)
                        {
                            f32 t = ui_anim(ui_key_from_string(b->key, s("fade_bottom")), 1.f, .rate = ti_state->catchall_animation_rate);
                            Rng2f32 rect = r2f32p(b->rect.x0, b->rect.y1 - fade_dim.y*t, b->rect.x1, b->rect.y1);
                            R_Rect2D_Inst *r = dr_rect(rect, fade_color, 0, 0, 0);
                            MemoryCopyArray(r->corner_radii, b_corner_radii);
                            r->colors[Corner_00] = r->colors[Corner_10] = v4f32(0, 0, 0, 0);
                        }
                        if(b->flags & UI_BoxFlag_DrawFadeLeft)
                        {
                            f32 t = ui_anim(ui_key_from_string(b->key, s("fade_left")), 1.f, .rate = ti_state->catchall_animation_rate);
                            Rng2f32 rect = r2f32p(b->rect.x0, b->rect.y0, b->rect.x1+fade_dim.x*t, b->rect.y1);
                            R_Rect2D_Inst *r = dr_rect(rect, fade_color, 0, 0, 0);
                            MemoryCopyArray(r->corner_radii, b_corner_radii);
                            r->colors[Corner_11] = r->colors[Corner_10] = v4f32(0, 0, 0, 0);
                        }
                        if(b->flags & UI_BoxFlag_DrawFadeRight)
                        {
                            f32 t = ui_anim(ui_key_from_string(b->key, s("fade_right")), 1.f, .rate = ti_state->catchall_animation_rate);
                            Rng2f32 rect = r2f32p(b->rect.x1 - fade_dim.x*t, b->rect.y0, b->rect.x1, b->rect.y1);
                            R_Rect2D_Inst *r = dr_rect(rect, fade_color, 0, 0, 0);
                            MemoryCopyArray(r->corner_radii, b_corner_radii);
                            r->colors[Corner_00] = r->colors[Corner_01] = v4f32(0, 0, 0, 0);
                        }
                    }
                    
                    // rjf: debug border rendering
                    if(b->flags & UI_BoxFlag_Debug)
                    {
                        R_Rect2D_Inst *inst = dr_rect(b->rect, v4f32(1*box->pref_size[Axis2_X].strictness, 0, 1, 0.25f), 0, 1.f, 0);
                        MemoryCopyArray(inst->corner_radii, b_corner_radii);
                    }
                    
                    // rjf: draw sides
                    if(b->flags & (UI_BoxFlag_DrawSideTop|UI_BoxFlag_DrawSideBottom|UI_BoxFlag_DrawSideLeft|UI_BoxFlag_DrawSideRight))
                    {
                        Vec4f32 border_color = b->border_color;
                        Rng2f32 r = b->rect;
                        f32 half_thickness = 1.f;
                        f32 softness = 0.f;
                        if(b->flags & UI_BoxFlag_DrawSideTop)
                        {
                            dr_rect(r2f32p(r.x0, r.y0, r.x1, r.y0+2*half_thickness), border_color, 0, 0, softness);
                        }
                        if(b->flags & UI_BoxFlag_DrawSideBottom)
                        {
                            dr_rect(r2f32p(r.x0, r.y1-2*half_thickness, r.x1, r.y1), border_color, 0, 0, softness);
                        }
                        if(b->flags & UI_BoxFlag_DrawSideLeft)
                        {
                            dr_rect(r2f32p(r.x0, r.y0, r.x0+2*half_thickness, r.y1), border_color, 0, 0, softness);
                        }
                        if(b->flags & UI_BoxFlag_DrawSideRight)
                        {
                            dr_rect(r2f32p(r.x1-2*half_thickness, r.y0, r.x1, r.y1), border_color, 0, 0, softness);
                        }
                    }
                    
                    // rjf: draw focus overlay
                    if(b->flags & UI_BoxFlag_Clickable && !(b->flags & UI_BoxFlag_DisableFocusOverlay) && b->focus_hot_t > 0.01f)
                    {
                        String8 extras[] = {str8_lit("focus"), str8_lit("overlay")};
                        String8_Array extras_array = {extras, ArrayCount(extras)};
                        Vec4f32 color = ui_color_from_tags_key_extras(b->tags_key, extras_array);
                        color.w *= b->focus_hot_t;
                        R_Rect2D_Inst *inst = dr_rect(b->rect, color, 0, 0, 0.f);
                        MemoryCopyArray(inst->corner_radii, b_corner_radii);
                    }
                    
                    // rjf: draw focus border
                    if(b->flags & UI_BoxFlag_Clickable && !(b->flags & UI_BoxFlag_DisableFocusBorder) && b->focus_active_t > 0.01f)
                    {
                        Rng2f32 rect = b->rect;
                        if(b->flags & UI_BoxFlag_Floating)
                        {
                            rect = pad_2f32(rect, 1.f);
                            rect = intersect_2f32(window_rect, rect);
                        }
                        String8 extras[] = {str8_lit("focus"), str8_lit("border")};
                        String8_Array extras_array = {extras, ArrayCount(extras)};
                        Vec4f32 color = ui_color_from_tags_key_extras(b->tags_key, extras_array);
                        color.w *= b->focus_active_t;
                        R_Rect2D_Inst *inst = dr_rect(rect, color, 0, 1.f, border_softness*1.f);
                        MemoryCopyArray(inst->corner_radii, b_corner_radii);
                    }
                    
                    // rjf: disabled overlay
                    if(b->disabled_t >= 0.005f)
                    {
                        Vec4f32 disabled_overlay_color = v4f32(base_background_color.x, base_background_color.y, base_background_color.z, b->disabled_t*0.3f);
                        R_Rect2D_Inst *inst = dr_rect(b->rect, disabled_overlay_color, 0, 0, 1);
                        MemoryCopyArray(inst->corner_radii, b_corner_radii);
                    }
                    
                    // rjf: pop squish
                    if(b->squish > box_squish_epsilon)
                    {
                        dr_pop_xform2d();
                        dr_pop_tex2d_sample_kind();
                    }
                    
                    // rjf: pop transparency
                    if(b->transparency != 0)
                    {
                        dr_pop_transparency();
                    }
                }
            }

            // next
            box = rec.next;
        }

        //- rjf: draw heatmap
        if(DEV_draw_ui_box_heatmap)
        {
            u64 uniform_dist_count = total_heatmap_sum_count / heatmap_bucket_count;
            uniform_dist_count = ClampBot(uniform_dist_count, 10);
            for(u64 bucket_idx = 0; bucket_idx < heatmap_bucket_count; bucket_idx += 1)
            {
                u64 x = bucket_idx % heatmap_bucket_pitch;
                u64 y = bucket_idx / heatmap_bucket_pitch;
                u64 bucket = heatmap_buckets[bucket_idx];
                f32 pct = (f32)bucket / uniform_dist_count;
                pct = Clamp(0, pct, 1);
                Vec3f32 hsv = v3f32((1-pct) * 0.9411f, 1, 0.5f);
                Vec3f32 rgb = rgb_from_hsv(hsv);
                Rng2f32 rect = r2f32p(x*heatmap_bucket_size,
                                      y*heatmap_bucket_size,
                                      (x+1)*heatmap_bucket_size,
                                      (y+1)*heatmap_bucket_size);
                dr_rect(rect, v4f32(rgb.x, rgb.y, rgb.z, 0.3f), 0, 0, 0);
            }
        }

        // draw hover debug box
        if (hover_debug_box != &ui_nil_box)
        {
            FNT_Tag font = fnt_tag_from_static_data_string(&ti_default_main_font_bytes);
            Vec2f32 p = ui_mouse();
            dr_rect(hover_debug_box->rect, v4f32(1, 1, 1, 0.2f), 0, 0, 0);
            R_Rect2D_Inst *border = dr_rect(hover_debug_box->rect, v4f32(1, 0, 0, 1.f), 0, 0, 0);
            MemoryCopyArray(border->corner_radii, hover_debug_box->corner_radii);
            dr_text(font, 12.f, 0, 0, FNT_RasterFlag_Hinted, p, v4f32(1, 1, 1, 1), push_str8f(scratch.arena, "key: 0x%I64x", hover_debug_box->key.u64[0]));
            p.y += 20.f;
            dr_text(font, 12.f, 0, 0, FNT_RasterFlag_Hinted, p, v4f32(1, 1, 1, 1), push_str8f(scratch.arena, "string: '%S'", hover_debug_box->string));
            p.y += 20.f;
        }

        //- rjf: draw border/overlay color to signify error
        if(ws->error_t > 0.01f) UI_TagF("bad")
        {
            Vec4f32 color = ui_color_from_name(str8_lit("text"));
            color.w *= ws->error_t;
            Rng2f32 rect = wm_client_rect_from_window(ws->os);
            dr_rect(pad_2f32(rect, 24.f), color, 0, 16.f, 12.f);
            dr_rect(rect, v4f32(color.x, color.y, color.z, color.w*0.025f), 0, 0, 0);
        }

        scratch_end(scratch);
    }

    ProfEnd();
    scratch_end(scratch);
}

//////////////////////////////
// Continuous Frame Requests

internal void ti_request_frame(void)
{
    ti_state->num_frames_requested = 4;
}

////////////////////////////
// Main State Accessors

// per-frame arena
internal Arena *ti_frame_arena(void)
{
    return ti_state->frame_arenas[ti_state->frame_index % ArrayCount(ti_state->frame_arenas)];
}

//////////////////////////////
// Main Layer Top-Level Calls

internal void ti_init(Cmd_Line *cmdline)
{
    ProfBeginFunction();
    Temp scratch = scratch_begin(0, 0);

    Arena *arena = arena_alloc();
    ti_state = push_array(arena, TI_State, 1);
    ti_state->arena = arena;
    for (u64 idx = 0; idx < ArrayCount(ti_state->frame_arenas); idx += 1)
    {
        ti_state->frame_arenas[idx] = arena_alloc();
    }
    ti_state->log = log_alloc();
    log_select(ti_state->log);
    {
        Temp scratch = scratch_begin(0, 0);
        ti_state->log_path = push_str8f(ti_state->arena, "%S/ui_thread.tide_log", g_logs_folder);
        write_data_to_file_path(ti_state->log_path, str8_zero());
        scratch_end(scratch);
    }
    ti_state->num_frames_requested = 2;

    // set up window cache
    {
        ti_state->window_state = push_array(arena, TI_Window_State, 1);
        ti_state->window_state->os = wm_window_open(r2f32p(0, 0, 1600, 900),
                                                   WM_WindowFlag_UseDefaultPosition,
                                                   str8_lit("TIDE"));
        ti_state->window_state->r = r_window_equip(ti_state->window_state->os);
        ti_state->window_state->ui = ui_state_alloc();
        ui_select_state(ti_state->window_state->ui);
        wm_window_first_paint(ti_state->window_state->os);
    }

    ProfEnd();
    scratch_end(scratch);
}
                         
internal void ti_frame(void)
{
    ProfBeginFunction();
    Temp scratch = scratch_begin(0, 0);
    log_scope_begin();
    ti_state->frame_depth += 1;

    ////////////////////////////
    // get events from the OS
    WM_Event_List events = {0};
    if (ti_state->frame_depth == 1)
    {
        events = wm_get_events(scratch.arena, ti_state->num_frames_requested == 0);// make a DEV_always_refresh here
    }

    ////////////////////
    // push frame scopes
    Access *frame_access_restore = ti_state->frame_access;
    ti_state->frame_access = access_open();

    ///////////////////////
    // pick target hz
    f32 target_hz = wm_get_system_info()->default_refresh_rate;

    //////////////////////////
    // target Hz -> delta time
    ti_state->frame_dt = 1.f / target_hz;

    /////////////////////////////////////////
    // megin measuring actual per-frame work
    u64 begin_time_us = now_time_us();

    ////////////////////
    // consume events
    ProfScope("consume events")
    {
        TI_Window_State *ws = ti_state->window_state;
        for (WM_Event *wm_event = events.first;
             wm_event != 0;
             wm_event = wm_event->next)
        {
            if (wm_event->kind == WM_EventKind_WindowClose)
            {
                ti_state->quit = true;
                break;
            }
            bool32 fallthrough_event = false;
            if (wm_event->kind == WM_EventKind_Press) fallthrough_event = true;
            if (wm_event->kind == WM_EventKind_Release) fallthrough_event = true;
            if (wm_event->kind == WM_EventKind_Text) fallthrough_event = true;
            if (wm_event->kind == WM_EventKind_Scroll) fallthrough_event = true;
            if (wm_event->kind == WM_EventKind_FileDrop) fallthrough_event = true;
            if (fallthrough_event) {
                UI_Event ui_event = zero_struct;
                UI_EventKind kind = UI_EventKind_Null;
                {
                    switch(wm_event->kind)
                    {
                        case WM_EventKind_Press:     {kind = UI_EventKind_Press;}break;
                        case WM_EventKind_Release:   {kind = UI_EventKind_Release;}break;
                        case WM_EventKind_Text:      {kind = UI_EventKind_Text;}break;
                        case WM_EventKind_Scroll:    {kind = UI_EventKind_Scroll;}break;
                        case WM_EventKind_FileDrop:  {kind = UI_EventKind_FileDrop;}break;
                    }
                }
                ui_event.kind         = kind;
                ui_event.key          = wm_event->key;
                ui_event.modifiers    = wm_event->modifiers;
                ui_event.string       = wm_event->character ? str8_from_32(ui_build_arena(), str32(&wm_event->character, 1)) : str8_zero();
                ui_event.paths        = str8_list_copy(ui_build_arena(), &wm_event->strings);
                ui_event.pos          = wm_event->pos;
                ui_event.delta_2f32   = wm_event->delta;
                ui_event.timestamp_us = wm_event->timestamp_us;
                ui_event_list_push(scratch.arena, &ws->ui_events, &ui_event);
            }
        }
    }

    ////////////////////////////////////
    // compute amimation rates, given config
    {
        f32 master_animations_f    = 1.f;
        f32 scrolling_animations_f = 1.f;
        f32 tooltip_animations_f   = 1.f;
        f32 menu_animations_f      = 1.f;

        ti_state->catchall_animation_rate     = 1 - master_animations_f*pow_f32(2, (-60.f * ti_state->frame_dt));
        ti_state->menu_animation_rate         = 1 - master_animations_f*menu_animations_f*pow_f32(2, (-70.f * ti_state->frame_dt));
        ti_state->menu_animation_rate__slow   = 1 - master_animations_f*menu_animations_f*pow_f32(2, (-50.f * ti_state->frame_dt));
        ti_state->entity_alive_animation_rate = 1 - master_animations_f*menu_animations_f*pow_f32(2, (-30.f * ti_state->frame_dt));
        ti_state->rich_hover_animation_rate   = 1 - master_animations_f*menu_animations_f*pow_f32(2, (-50.f * ti_state->frame_dt));
        ti_state->scrolling_animation_rate    = 1 - master_animations_f*scrolling_animations_f*pow_f32(2, (-60.f * ti_state->frame_dt));
        ti_state->tooltip_animation_rate      = 1 - master_animations_f*tooltip_animations_f*pow_f32(2, (-60.f * ti_state->frame_dt));
    }

    ///////////////////////////////
    // update/render all windows
    {
        dr_begin_frame(fnt_tag_from_static_data_string(&ti_default_main_font_bytes));
        {
            ti_window_frame();
        }
    }

    //////////////////////////////
    // clear frame request state
    if (ti_state->num_frames_requested > 0)
    {
        ti_state->num_frames_requested -= 1;
    }

    //////////////////////////
    // close frame scopes
    //
    // NOTE: this always must happen before the refresh, since that
    // will sleep for vsync, and we do not want to hold handles for long,
    // since eviction threads may be wariing to get rid of stuff.
    //
    access_close(ti_state->frame_access);
    ti_state->frame_access = frame_access_restore;

    /////////////////////////////
    // submit rendering to all windows
    ProfScope("submit rendering to all windows")
    {
        r_begin_frame();
        {
            TI_Window_State *w = ti_state->window_state;
            r_window_begin_frame(w->os, w->r);
            dr_submit_bucket(w->os, w->r, w->draw_bucket);
            r_window_end_frame(w->os, w->r);
        }
        r_end_frame();
    }

    //////////////////////////
    // show windows after first frame
    if (ti_state->frame_depth == 1)
    {
        TI_Window_State *ws = ti_state->window_state;
        wm_window_first_paint(ws->os);
    }

    ////////////////////////
    // determine frame time
    u64 end_time_us = now_time_us();
    u64 frame_time_us = end_time_us - begin_time_us;

    ///////////////////////////
    // bump frame time counters
    ti_state->frame_index += 1;
    ti_state->time_in_seconds += ti_state->frame_dt;
    ti_state->time_in_us += frame_time_us;

    /////////////////
    // collect logs
    ProfScope("collect logs")
    {
        Log_Scope_Result log = log_scope_end(scratch.arena);
        append_data_to_file_path(ti_state->log_path, log.strings[LogMsgKind_Info]);
        if (log.strings[LogMsgKind_UserError].size != 0)
        {
            String8 error_log = log.strings[LogMsgKind_UserError];
            String8_List error_log_lines = str8_split(scratch.arena, error_log, (u8 *)"\n", 1, 0);
            String8 error_log_string = str8_list_join(scratch.arena,
                                                      &error_log_lines,
                                                      &(String_Join){.sep = str8_lit(" ")});
            TI_Window_State *ws = ti_state->window_state;
            {
                ws->error_string_size = Min(sizeof(ws->error_buffer), error_log_string.size);
                MemoryCopy(ws->error_buffer, error_log_string.str, ws->error_string_size);
                ws->error_t = 1.f;
            }
        }
    }

    ti_state->frame_depth -= 1;
    scratch_end(scratch);
    ProfEnd();
}
