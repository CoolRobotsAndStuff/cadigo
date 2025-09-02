#define CADIGO_IMPLEMENTATION
#define CADIGO_CREATION_HOOKS
#include "cadigo.h"
#include "extensions/visualizer.h"

CADs objs_to_render = {0};

void cad_on_alloc(CAD* obj) {
    da_append(&objs_to_render, obj);
};

void cad_on_free(CAD* obj)  { 
    for (size_t i = 0; i < objs_to_render.count; ++i) {
        if (objs_to_render.items[i] == obj) da_delete(&objs_to_render, i);
    }
};


int main() {
    CAD* s = cad_cube(10);
    s->color = vec3(1, 0.2, 0.2);

    cad_repeat(4) cad_catmull_clark(s);

    CAD* c = cad_translate_x(cad_cube(10), 20);
    c->color = vec3(0.8, 0.8, 0);

    cad_translate_x(cad_cube(10), 20)->color = vec3(0, 0.8, 0);

    CAD* wii = cad_cube(2);
    cad_translate_y(wii, 10);
    wii->color = vec3(0.5, 0.5, 0.8);

    CAD_Viz* viz = cad_viz_init();

    Vec3 vector = vec3(5, 0, 5);
    while(cad_viz_keep_rendenring(viz)) {
        cad_rotate_y(c, 1);
        cad_rotate_z(wii, 10);

        cad_viz_begin(viz);

            for (size_t i=0; i < objs_to_render.count; ++i) {
                cad_viz_render(viz, *objs_to_render.items[i]);
            }
            cad_viz_render_vec3_at(viz, vec3(10, 0, 0),  vector);
            vector = vec3_rotate_pitch(0.001, vector);
            vector = vec3_rotate_yaw(0.001, vector);
        cad_viz_end(viz);
    }
    return 0;
}
