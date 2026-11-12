/////////////////////
// null @view_hook_impl

TI_VIEW_UI_FUNCTION_DEF(null) {}

////////////////////
// text @view_hook_impl

TI_VIEW_UI_FUNCTION_DEF(text)
{
}

////////////////////
// worldmap @view_hook_impl

typedef struct TI_World_Map_View_State TI_World_Map_View_State;
struct TI_World_Map_View_State
{
    f32 center_x;
    f32 center_y;
    f32 distance;
};

typedef struct TI_World_Map_Box_Draw_Data TI_World_Map_Box_Draw_Data;
struct TI_World_Map_Box_Draw_Data
{
    f32 center_x;
    f32 center_y;
    f32 distance;
    R_Handle vertex_buffer;
    R_Handle index_buffer;
    R_Handle texture;
};

internal UI_BOX_CUSTOM_DRAW(ti_world_map_box_draw)
{
    TI_World_Map_Box_Draw_Data *draw_data = (TI_World_Map_Box_Draw_Data *)user_data;

    // get clip
    Rng2f32 clip = box->rect;
    for (UI_Box *b = box->parent; !ui_box_is_nil(b); b = b->parent)
    {
        if (b->flags & UI_BoxFlag_Clip)
        {
            clip = intersect_2f32(b->rect, clip);
        }
    }

    // eye/target
    Vec3f32 target = {draw_data->center_x, draw_data->center_y, 0.f};
    Vec3f32 eye = v3f32(draw_data->center_x, draw_data->center_y, draw_data->distance);
    Vec3f32 up = v3f32(0.f, 1.f, 0.f);

    Vec2f32 box_dim = dim_2f32(box->rect);
    R_Pass_Params_Geo3D *pass = dr_geo3d_begin(box->rect,
                                               make_look_at_4x4f32(eye, target, up),
                                               make_perspective_4x4f32(0.25, box_dim.x/box_dim.y, 1.f, 4000.f));
    pass->clip = clip;
    dr_mesh(draw_data->vertex_buffer,
            draw_data->index_buffer,
            R_GeoTopologyKind_Triangles,
            R_GeoVertexFlag_TexCoord|R_GeoVertexFlag_Normals|R_GeoVertexFlag_RGB,
            draw_data->texture,
            mat_4x4f32(1.f));
}

TI_VIEW_UI_FUNCTION_DEF(worldmap)
{
   String8 vtx = ti_map_vertex_bytes;
   String8 idx = ti_map_index_bytes;

   Temp scratch = scratch_begin(0, 0);
   Access *access = access_open();
   TI_World_Map_View_State *state = ti_view_state(TI_World_Map_View_State);

   ///////////////////
   // unpack parameters
   //
   f32 center_x_target  = ti_view_setting_f32_from_name(str8_lit("center_x"));
   f32 center_y_target  = ti_view_setting_f32_from_name(str8_lit("center_y"));
   f32 distance_target  = ti_view_setting_f32_from_name(str8_lit("distance"));

   //////////////////
   // do first-time camera initialization, if needed
   if (distance_target == 0)
   {
       center_x_target = 500.f;
       center_y_target = 500.f;
       distance_target = 600.f;
   }
   if (state->distance == 0)
   {
       state->center_x = center_x_target;
       state->center_y = center_y_target;
       state->distance = distance_target;
   }

   // animate camera
   {
       f32 fast_rate = 1 - pow_f32(2, (-60.f * ti_state->frame_dt));
       f32 slow_rate = 1 - pow_f32(2, (-30.f * ti_state->frame_dt));
       state->center_x += (center_x_target - state->center_x) * fast_rate;
       state->center_y += (center_y_target - state->center_y) * fast_rate;
       state->distance += (distance_target - state->distance) * slow_rate;
       if (abs_f32(state->center_x - center_x_target) > 0.01f ||
           abs_f32(state->center_y - center_y_target) > 0.01f ||
           abs_f32(state->distance - distance_target) > 0.01f)
       {
           ti_request_frame();
       }
   }

   //////////////
   // build
   //
   if (!r_handle_match(ti_state->world_map_index_buffer, r_handle_zero())
       && !r_handle_match(ti_state->world_map_vertex_buffer, r_handle_zero()))
   {
       Vec2f32 dim = dim_2f32(rect);
       UI_Box *box = &ui_nil_box;
       UI_FixedSize(dim)
       {
           box = ui_build_box_from_stringf(UI_BoxFlag_DrawBorder|
                                           UI_BoxFlag_DrawBackground|
                                           UI_BoxFlag_Clickable|
                                           UI_BoxFlag_Scroll,
                                           "geo_box");
       }
       UI_Signal sig = ui_signal_from_box(box);
       if(ui_dragging(sig))
       {
           if(ui_pressed(sig))
           {
               ti_cmd(TI_CmdKind_FocusPanel);
               Vec2f32 data = v2f32(center_x_target, center_y_target);
               ui_store_drag_struct(&data);
           }
           Vec2f32 drag_delta       = ui_drag_delta();
           Vec2f32 drag_start_delta = *ui_get_drag_struct(Vec2f32);
           f32 visible_height  = 2.f * distance_target * tan_f32(0.25 * 0.5f);
           f32 world_units_per_pixel = visible_height / dim.y;
           center_x_target   = drag_start_delta.x + drag_delta.x * world_units_per_pixel;
           center_y_target   = drag_start_delta.y + drag_delta.y * world_units_per_pixel;
       }
       distance_target *= pow_f32(1.1f, sig.scroll.y);
       distance_target = Clamp(5.f, distance_target, 2000.f);
       TI_World_Map_Box_Draw_Data *draw_data = push_array(ui_build_arena(), TI_World_Map_Box_Draw_Data, 1);
       draw_data->center_x   = state->center_x;
       draw_data->center_y   = state->center_y;
       draw_data->distance   = state->distance;
       draw_data->vertex_buffer = ti_state->world_map_vertex_buffer;
       draw_data->index_buffer  = ti_state->world_map_index_buffer;
       draw_data->texture       = ti_state->world_map_texture;
       ui_box_equip_custom_draw(box, ti_world_map_box_draw, draw_data);
   }

   ///////////////////
   // commit parameters
   //
   ti_store_view_param_f32(str8_lit("center_x"),   center_x_target);
   ti_store_view_param_f32(str8_lit("center_y"),   center_y_target);
   ti_store_view_param_f32(str8_lit("distance"),   distance_target);

   access_close(access);
   scratch_end(scratch);
}
