#define CADIGO_IMPLEMENTATION
#include "cadigo.h"
#include "extensions/visualizer.h"

#include <math.h>
#include <stdlib.h>
#include <stdbool.h>

val_t vec3_dist(Vec3 a, Vec3 b) {
    return sqrt( 
        (a.x - b.x) * (a.x - b.x) +
        (a.y - b.y) * (a.y - b.y) +
        (a.z - b.z) * (a.z - b.z)
    );
}

#define print_zu(var) printf(#var " = %zu\n", (var)); 

void cut(CAD* c1, CAD cutter) {
    Face orig_face = cad_copy_face(c1->faces.items[0]);
    puts("orig_face:");
    print_face(orig_face);

    size_t sep = c1->points.count;

    for (size_t i = 0; i < cutter.points.count; ++i) {
        da_append(&c1->points, cutter.points.items[i]);
    }

    Face cutter_face = cutter.faces.items[0];
    puts("cutter_face:");
    print_face(cutter_face);

    size_t prev_closest_cfi;
    size_t fi = 0;
    for (;;) {
        print_faces(c1->faces);
        print_zu(fi);

        Face new_face = face();
        size_t prev_closest = SIZE_MAX;
        prev_closest_cfi = SIZE_MAX;

        for (;;) {
            size_t i = orig_face.items[fi];
            if (fi == orig_face.count) i = orig_face.items[0];
            
            da_append(&new_face, i);
            val_t min_dist = 1000000000000.0;
            size_t closest = 0;
            size_t closest_cfi = 0;

            for (size_t cfi = 0; cfi < cutter_face.count; ++cfi) {
                size_t k = sep + cutter_face.items[cfi];
                val_t d = vec3_dist(c1->points.items[i], c1->points.items[k]);
                if (d < min_dist) {
                    min_dist = d;
                    closest = k;
                    closest_cfi = cfi;
                }
            }
            printf("going from previous : %zu to next: %zu\n", prev_closest_cfi, closest_cfi);

            if (prev_closest_cfi != SIZE_MAX)
                for (size_t h = prev_closest_cfi; h != closest_cfi; h=(h+1)%cutter_face.count) {
                    size_t k = sep + cutter_face.items[h];

                    if (!new_face.count || new_face.items[0] != k) {
                        da_insert(&new_face, 0, k);
                    }
                }

            if (!new_face.count || new_face.items[0] != closest) {
                da_insert(&new_face, 0, closest);
            }
            if ((prev_closest != closest) && (prev_closest != SIZE_MAX)) break;
            prev_closest     = closest;
            prev_closest_cfi = closest_cfi;
            if (++fi > orig_face.count) {
                if (new_face.count >= 3) {
                    da_append(&c1->faces, new_face);
                }
                goto out;
            }
        }
        da_append(&c1->faces, new_face);
    }
    out:

    da_delete(&c1->faces, 0);

    print_faces(c1->faces);
    print_zu(c1->faces.count);
    free_face(orig_face);
}


int main() {
    CAD c1 = cad_square(20);
    CAD c2 = cad_square(5);
    CAD c3 = {0};

    CAD_Viz* viz = cad_viz_init();
    viz->fps = 60;

    val_t pos = 0;
    val_t v = 0.1;

    while (cad_viz_keep_rendenring(viz)) {
        cad_clone_into(c1, &c3);
        cad_translate_x(&c2, v);
        pos += v;
        if (pos > 10 || pos < -10) v = -v;
        cut(&c3, c2);
        print_zu(c3.faces.count);
        cad_rotate_z(&c3, 180);

        cad_viz_begin(viz);
            cad_viz_render(viz, c3);
        cad_viz_end(viz);
    }

    return 0;
}
