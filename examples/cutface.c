#include "extensions/visualizer.h"
#define CADIGO_IMPLEMENTATION
#include "cadigo.h"


val_t vec3_dist(Vec3 a, Vec3 b) {
    return sqrt( 
        (a.x - b.x) * (a.x - b.x) +
        (a.y - b.y) * (a.y - b.y) +
        (a.z - b.z) * (a.z - b.z)
    );
}

#define print_zu(var) printf(#var " = %zu", (var)); 

void cut(CAD* c1, CAD cutter) {
    da_delete(&c1->faces, 0);

    size_t sep = c1->points.count;

    for (size_t i = 0; i < cutter.points.count; ++i) {
        da_append(&c1->points, cutter.points.items[i]);
    }

    size_t j = 0;
    while (j < sep) {
        Face new_face = face();
        size_t prev_closest = SIZE_MAX;
        for (; j <= sep; ++j) {
            size_t i;
            if (j == c1->points.count) i = 0;
            else i = j;
            da_append(&new_face, i);
            val_t min_dist = 1000000000000.0;
            size_t closest = 0;
            for (size_t k = sep; k < c1->points.count; ++k) {
                val_t d = vec3_dist(c1->points.items[i], c1->points.items[k]);
                // printf("d = %"VAL_FMT"\n", d);
                // printf("min_dist = %"VAL_FMT"\n", min_dist);
                if (d < min_dist) {
                    min_dist = d;
                    closest = k;
                }
            }
            da_insert(&new_face, 0, closest);
            if ((prev_closest != closest) && (prev_closest != SIZE_MAX)) {
                break;
            }
            prev_closest = closest;
        }
        da_append(&c1->faces, new_face);
    }
    
    bool last_in_face = false;
    bool first_in_face = false;
    for (size_t i = 0; i < c1->faces.count; ++i) {
        last_in_face = false;
        first_in_face = false;
        Face face = c1->faces.items[i];
        for (size_t j = 0; j < face.count; ++j) {
            if (face.items[j] == 0) first_in_face = true;
            if (face.items[j] == sep-1) last_in_face = true;
        }
        if (last_in_face && first_in_face) break;
    }

    if (!(last_in_face && first_in_face)) {
        Face new_face = face();
        da_append(&new_face, sep-1); 
        da_append(&new_face, 0); 
        val_t min_dist = 1000000000000.0;
        size_t closest = 0;
        for (size_t k = sep; k < c1->points.count; ++k) {
            val_t d = vec3_dist(c1->points.items[0], c1->points.items[sep]);
            if (d < min_dist) {
                min_dist = d;
                closest = k;
            }
        }
        da_append(&new_face, closest);
        da_append(&c1->faces, new_face);

    }

    print_zu(c1->faces.count);

}

int main() {
    CAD c1 = cad_square(10);
    CAD c2 = cad_square(5);
    // c2.faces.items[0].count -= 1;
    // c2.points.count -= 1;
    //cad_translate(&c2, vec3(7, 0, 0));
    cad_hotpoints_subdivision(&c2);
    cad_hotpoints_subdivision(&c2);
    print_faces(c2.faces);
    print_points(c2.points);
    cad_rotate_z(&c2, 30);

    cut(&c1, c2);
    float zoom = 0.05;
    ASCII_Screen screen = alloc_ascii_screen();
    cad_clear_ascii_screen(&screen);
    cad_render_to_ascii_screen(&screen, zoom, c1);
    //cad_render_to_ascii_screen(&screen, zoom, c2);
    cad_print_ascii_screen(screen);

    free_ascii_screen(screen);

    cad_rotate_z(&c1, 180);
    cad_visualize(c1);


    return 0;
}
