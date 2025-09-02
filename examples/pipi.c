#define CADIGO_IMPLEMENTATION
//#define CADIGO_CREATION_HOOKS
#include "cadigo.h"
#include "extensions/visualizer.h"

//CADs objs_to_render = {0};

// void cad_on_alloc(CAD* obj) {
//     da_append(&objs_to_render, obj);
// };
//
// void cad_on_free(CAD* obj)  { 
//     for (size_t i = 0; i < objs_to_render.count; ++i) {
//         if (objs_to_render.items[i] == obj) da_delete(&objs_to_render, i);
//     }
// };

CAD* cad_regular_polygon(val_t radius, size_t sides) {
    Points ps = {0};
    for (size_t i =0; i<sides; ++i) {
        val_t angle = i * (M_PI*2/sides);
        Vec3 v = vec3(
            cos(angle)*radius,
            sin(angle)*radius,
            0
        );
        da_append(&ps, v);
    }
    CAD* ret = cad_polygon(ps);
    return ret;
}

Vec3 color_wood;
Vec3 color_yellow;
Vec3 color_blue;
Vec3 color_black;

CAD* wheel_model(float side) {
    CAD* ret = cad_regular_polygon(6.5/2, 20);
    cad_extrude(ret, 2.5);
    cad_rotate_y(ret, 90*side);
    ret->color = color_black;
    cad_translate_x(ret, -3.5*side);
    cad_translate_z(ret, -6);
    cad_translate_y(ret, 0.5+1);
    return ret;
}

CAD* motor_model(float side) {
    CAD* ret = cad_cube(1);
    cad_scale(ret, vec3(2, 2, 6));
    cad_translate_x(ret, side*(6.5/2-1));
    cad_translate_y(ret, 0.5+1);
    cad_translate_z(ret, -4);
    ret->color = color_yellow;
    return ret;
}

int main() {
    color_wood = vec3(189/255.0, 148/255.0, 89/255.0);
    color_yellow = vec3(0.8, 0.8, 0.1);
    color_blue = vec3(0.1, 0.1, 0.8);
    color_black = vec3(0.1, 0.1, 0.1);

    CAD_Viz* viz = cad_viz_init();
    viz->camZ = 30;

    while(cad_viz_keep_rendenring(viz)) {
        cad_viz_begin(viz);

        CAD* model;
        model = wheel_model(1);
        cad_viz_render(viz, *model);
        cad_free(model);

        model = wheel_model(-1);
        cad_viz_render(viz, *model);
        cad_free(model);

        model = motor_model(1);
        cad_viz_render(viz, *model);
        cad_free(model);

        model = motor_model(-1);
        cad_viz_render(viz, *model);
        cad_free(model);
        
        // chasis_main
        model = cad_cube(1);
        cad_scale(model, vec3(8.8, 1, 5));
        model->color = color_wood;
        cad_viz_render(viz, *model);
        cad_free(model);
        
        // chasis_tail
        model = cad_cube(1);
        cad_scale(model, vec3(6.5, 1, 7));
        cad_translate_z(model, -6);
        model->color = color_wood;
        cad_viz_render(viz, *model);
        cad_free(model);
        
        // chasis_nose
        model = cad_cube(1);
        cad_scale(model, vec3(3, 1, 6));
        cad_translate_z(model, 5.5);
        model->color = color_wood;
        cad_viz_render(viz, *model);
        cad_free(model);

        // arduino
        model = cad_cube(1);
        cad_scale(model, vec3(7, 0.2, 5));
        cad_translate_y(model, 0.6);
        cad_translate_z(model, 3);
        model->color = color_blue;
        cad_viz_render(viz, *model);
        cad_free(model);
        
        // sensor
        model = cad_cube(1);
        cad_scale(model, vec3(1, 0.2, 3.5));
        cad_translate_y(model, 0.5);
        cad_translate_z(model, 8);
        model->color = color_black;
        cad_viz_render(viz, *model);
        cad_free(model);

        cad_viz_end(viz);
    }
    return 0;
}
