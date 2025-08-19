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

    cad_repeat(4) cad_catmull_clark(s);

    CAD* c = cad_translate_x(cad_cube(10), 20);

    cad_translate_x(cad_cube(10), 20);

    CAD* wii = cad_cube(2);
    cad_translate_y(wii, 10);

    CAD_Viz* viz = cad_viz_init();

    while(cad_viz_keep_rendenring(viz)) {
        cad_rotate_y(c, 1);
        cad_rotate_z(wii, 10);

        cad_viz_begin(viz);
            for (size_t i=0; i < objs_to_render.count; ++i) {
                cad_viz_render(viz, *objs_to_render.items[i]);
            }
        cad_viz_end(viz);
    }
    return 0;
}
