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

#define CAD_INFINTY HUGE_VALL

typedef struct {
    val_t m;
    val_t b;
} Line;

typedef struct {
    val_t min_x;
    val_t max_x;
    Line line;
} Segment;

val_t slope2d(Vec3 p1, Vec3 p2) {
    if (p1.x - p2.x == 0) return CAD_INFINTY;

    return (p1.y - p2.y)/
           (p1.x - p2.x);

}

val_t y_intercept2d(Vec3 p1, val_t slope) {
    return p1.y - p1.x * slope;
}

Line line_from_points2d(Vec3 p1, Vec3 p2) {
    Line ret;
    ret.m = slope2d(p1, p2);
    ret.b = y_intercept2d(p1, ret.m);
    return ret;
}

Segment segment_from_points2d(Vec3 p1, Vec3 p2) {
    Segment ret ;
    ret.line = line_from_points2d(p1, p2);
    ret.min_x = min(p1.x, p2.x);
    ret.max_x = max(p1.x, p2.x);
    return ret;
}

Vec3 lines_intersect2d(Line l1, Line l2) {
    val_t x = (l1.b - l2.b)/
              (l2.m - l1.m);
    
    val_t y = l1.m * x + l1.b;

    return vec3(x, y, 0);
}


Vec3 face_segment_intersection(CAD obj, size_t face_index, Segment segment, size_t* out_idx) {
    Face f = obj.faces.items[face_index];
    for (size_t i = 0; i < f.count; ++i) {
        size_t i2 = (i+1)%f.count;

        val_t max_x = max(obj.points.items[f.items[i]].x, obj.points.items[f.items[i2]].x);
        val_t min_x = min(obj.points.items[f.items[i]].x, obj.points.items[f.items[i2]].x);

        Line l = line_from_points2d(obj.points.items[f.items[i]], obj.points.items[f.items[i2]]);
        //printf("Line: { slope: %"VAL_FMT", y_intercept:%"VAL_FMT"\n", l.m, l.b);
        Vec3 itsc = lines_intersect2d(l, segment.line);
        if (itsc.x >= CAD_INFINTY || itsc.y >= CAD_INFINTY) continue;

        if (itsc.x >= segment.min_x && itsc.x <= segment.max_x
        &&  itsc.x >= min_x         && itsc.x <= max_x        ) {
            *out_idx = i2;
            return itsc;
        }
    }
    assert(false && "face_segment_intersection");
    *out_idx = 100000000;
    return vec3(69, CAD_INFINTY, 0);
}

void cut(CAD* c1, CAD cutter_) {
    CAD* cutter_ptr = cad_clone(cutter_);
    CAD cutter = *cutter_ptr;

    Face orig_face = cad_copy_face(c1->faces.items[0]);
    puts("orig_face:");
    print_face(orig_face);

    size_t sep = c1->points.count;

    //Face cutter_face = cutter.faces.items[0];
    Face cutter_face = {0};
    
    size_t cutter_face_count = 0;
    for (size_t i = 0; i < cutter.points.count; ++i) {
        if (point_inside_face2D(cutter.points.items[i], 0, *c1)) {
            cutter.points.items[i].mark = 0;
            da_append(&c1->points, cutter.points.items[i]);
            da_append(&cutter_face, cutter_face_count);
            cutter_face_count += 1;
        } else {
            size_t i_next = (i+1)%cutter.points.count;
            size_t i_prev;
            if (i == 0) i_prev = cutter.points.count-1;
            else        i_prev = (i-1)%cutter.points.count;

            bool next_inside = point_inside_face2D(cutter.points.items[i_next], 0, *c1);
            bool prev_inside = point_inside_face2D(cutter.points.items[i_prev], 0, *c1);

            if (prev_inside && next_inside) {
                Segment seg;
                size_t idx;
                Vec3 itc;

                seg = segment_from_points2d(cutter.points.items[i_prev], cutter.points.items[i]);

                itc = face_segment_intersection(*c1, 0, seg, &idx);
                itc.mark = 1;

                da_append(&c1->points, itc);
                da_insert(&orig_face         , idx, c1->points.count-1);
                da_insert(&c1->faces.items[0], idx, c1->points.count-1);
                da_append(&cutter_face, cutter_face_count++);

                seg = segment_from_points2d(cutter.points.items[i], cutter.points.items[i_next]);

                itc = face_segment_intersection(*c1, 0, seg, &idx);
                itc.mark = 1;

                da_append(&c1->points, itc);
                da_insert(&orig_face         , idx, c1->points.count-1);
                da_insert(&c1->faces.items[0], idx, c1->points.count-1);
                da_append(&cutter_face, cutter_face_count++);

                da_append(&c1->points, cutter.points.items[i_next]);
                da_append(&cutter_face, cutter_face_count++);
                i++;

            } else if (prev_inside || next_inside) {
                Segment seg;
                if (prev_inside) {
                    seg = segment_from_points2d(cutter.points.items[i_prev], cutter.points.items[i]);
                } else if (next_inside) {
                    seg = segment_from_points2d(cutter.points.items[i], cutter.points.items[i_next]);
                }

                size_t idx;
                Vec3 itc = face_segment_intersection(*c1, 0, seg, &idx);
                itc.mark = 1;

                da_append(&c1->points, itc);
                da_insert(&orig_face         , idx, c1->points.count-1);
                da_insert(&c1->faces.items[0], idx, c1->points.count-1);
                da_append(&cutter_face, cutter_face_count++);
                if (next_inside) {
                    da_append(&c1->points, cutter.points.items[i_next]);
                    da_append(&cutter_face, cutter_face_count++);
                    i++;
                }
            }


            //assert(!(prev_inside && next_inside));


            //assert((prev_inside || next_inside) && "not both outside");

            // if (!point_inside_face2D(cutter.points.items[i2], 0, *c1)) {
            //     i2 = (i-1)%cutter.points.count;
            // }
            // assert(point_inside_face2D(cutter.points.items[i2], 0, *c1) && "point_inside_face2D");
            // 
            // Segment seg = segment_from_points2d(cutter.points.items[i2], cutter.points.items[i]);
            // //cutter.points.items[i] = face_segment_intersection(*c1, 0, seg);
            // Vec3 itc = face_segment_intersection(*c1, 0, seg);
            // //itc.x -= 0.5;
            // //printf("intersection: ");
            // //vec3_print(itc);
            // da_append(&c1->points, itc);
        }
    }

    // free_face(orig_face);
    // cad_free(cutter_ptr);
    // return;
    
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


            if (prev_closest_cfi != SIZE_MAX) {
                printf("going from previous : %zu to next: %zu\n", prev_closest_cfi, closest_cfi);
                for (size_t h = prev_closest_cfi; h != closest_cfi; h=(h+1)%cutter_face.count) {
                    size_t k = sep + cutter_face.items[h];

                    if (!new_face.count || new_face.items[0] != k) {
                        da_insert(&new_face, 0, k);
                        if (c1->points.items[k].mark == 1) {
                            printf("we shoudl break here, at face = %zu, point = %zu\n", c1->faces.count, k);
                            goto append_face_and_begin_next;
                        } else {
                            puts("we shouldn't break here");
                        }
                    }
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
        append_face_and_begin_next:
        da_append(&c1->faces, new_face);
    }
    out:

    da_delete(&c1->faces, 0);
    
    bool continue_deleting;
    do {
        continue_deleting = false;
        for (size_t i=0; i < c1->faces.count; ++i) {
            Face* f = &c1->faces.items[i];

            printf("--------------------------------------------------\n");
            for (size_t j=0; j < f->count; ++j) {
                size_t jplus1  = (j+1)%f->count;
                size_t jminus1 = (j-1)%f->count;
                Vec3 p    = c1->points.items[f->items[j]];
                Vec3 prev = c1->points.items[f->items[jminus1]];
                Vec3 next = c1->points.items[f->items[jplus1]];

                if (fabs(slope2d(prev, p) - slope2d(next, p)) < 0.01) {
                    da_delete(f, j);
                    continue_deleting = true;
                }
            }
        }
    } while(continue_deleting);


    print_faces(c1->faces);
    print_zu(c1->faces.count);
    free_face(orig_face);
    cad_free(cutter_ptr);
}


int main() {
    CAD* c1 = cad_square(20);
    cad_rotate_z(c1, 10);
    CAD* c2 = cad_square(5);
    cad_rotate_z(c2, -10);
    CAD* c3 = cad_alloc();

    CAD_Viz* viz = cad_viz_init();
    viz->fps = 60;
    viz->show_vertices = true;

    val_t pos = 0;
    val_t v = 0.1;
    //cad_translate_x(c2, -9);
    while (cad_viz_keep_rendenring(viz)) {
        cad_clone_into(*c1, c3);
        cad_rotate_in_place_z(c2, 1);
        cad_translate_x(c2, v);
        pos += v;
        if (pos > 15 || pos < -15) v = -v;
        cut(c3, *c2);
        print_zu(c3->faces.count);
        //cad_rotate_z(c3, 180);

        cad_viz_begin(viz);
            cad_viz_render(viz, *c3);
        cad_viz_end(viz);
    }

    return 0;
}
