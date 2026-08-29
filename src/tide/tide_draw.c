
//////////////////
// Line Drawing

internal void ti_draw_line(Vec2f32 p0, Vec2f32 p1, f32 thickness, Vec4f32 color)
{
    Vec2f32 delta = sub_2f32(p1, p0);
    f32 len = length_2f32(delta);
    if(len > 0.001f)
    {
        Vec2f32 mid = mix_2f32(p0, p1, 0.5f);
        f32 angle_rad = atan2f(delta.y, delta.x);
        
        Mat3x3f32 rotate = mat_3x3f32(1.f);
        rotate.v[0][0] =  cosf(angle_rad); rotate.v[0][1] = sinf(angle_rad);
        rotate.v[1][0] = -sinf(angle_rad); rotate.v[1][1] = cosf(angle_rad);
        
        Mat3x3f32 translate = make_translate_3x3f32(mid);
        
        Mat3x3f32 xform = mul_3x3f32(translate, rotate);
        
        dr_push_xform2d(xform);
        dr_rect(r2f32p(-len*0.5f, -thickness*0.5f, len*0.5f, thickness*0.5f),
                color, thickness*0.5f, 0.f, 1.f);
        dr_pop_xform2d();
    }
}

/////////////
// Testing
internal void ti_test_draw_box(Vec4f32 color)
{

}
