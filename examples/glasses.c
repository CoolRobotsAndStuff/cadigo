#include "../cadigo.h"
#include <bits/pthreadtypes.h>
#include <stdio.h>

CAD_Array range(size_t beg, size_t end, size_t step) {
    size_t count = end - beg;
    int* array = (int*)malloc(count*sizeof(int));  

    for (size_t i = 0; i < count; i += step) {
        array[i] = (int)i + (int)beg;
    }
    CAD_Array ret = {.count=count, .element_size=sizeof(int), .values=array};
    return ret;
}

void make_little_faces_around(CAD_Array* faces) {
    CAD_Array* vals = (CAD_Array*)faces->values;
    CAD_Array face1 = vals[0];
    CAD_Array face2 = vals[1];
    size_t point_count = face2.count;

    printf("hello from make make_little_faces_around");
    for (size_t i=0; i < face1.count; ++i) {
        CAD_Array* new_face = malloc(sizeof(CAD_Array));
        new_face->count = 4;
        new_face->element_size = sizeof(int);
        int* vals = (int*)malloc(new_face->element_size * new_face->count);
        if (i != face1.count - 1) {
            vals[3] = i;
            vals[2] = i + 1;
            vals[1] = i + 1 + point_count;
            vals[0] = i + point_count;
            new_face->values = vals; 
        } else {
            vals[3] = i;
            vals[2] = 0;
            vals[1] = point_count;
            vals[0] = i + point_count;
            new_face->values = vals; 
        }
        cad_array_append(faces, new_face);
    }
}

int main() {
    CAD_Array points = vectors3(
            // x    y    z
        vec3(-0.3 , 25  , 1), 
        vec3(0.5  , 20  , 1),
        vec3(1.5  , 15  , 1),
        vec3(3    , 10  , 1),
        vec3(5.5  , 5.5 , 1),
        vec3(8    , 2.75, 1),
        vec3(10   , 2   , 1),
        vec3(15   , .5  , 1),
        vec3(20   , 0   , 1),
        vec3(25   , -.2 , 1),
        vec3(30   , 0   , 1),
        vec3(35   , 1   , 1),
        vec3(40   , 2.5 , 1),
        vec3(42   , 3   , 1),
        vec3(45   , 5   , 1),
        vec3(47.5 , 7.5 , 1),
        vec3(49   , 10  , 1),
        vec3(50   , 12  , 1),
        vec3(50   , 12  , 1),
        vec3(51   , 15  , 1),
        vec3(52.5 , 20  , 1),
        vec3(53.5 , 25  , 1),
        vec3(53.55, 33  , 1),
        vec3(53   , 35  , 1),
        vec3(52   , 37.5, 1),
        vec3(50   , 39  , 1),
        vec3(48.5 , 40  , 1),
        vec3(45   , 42  , 1),
        vec3(40   , 43  , 1),
        vec3(35   , 43.5, 1),
        vec3(30   , 43.5, 1),
        vec3(30   , 43.5, 1),
        vec3(25   , 43  , 1),
        vec3(20   , 42.5, 1),
        vec3(15   , 41.5, 1),
        vec3(10   , 40.2, 1),
        vec3(5    , 38.5, 1),
        vec3(3.5  , 37.5, 1),
        vec3(1.5  , 35  , 1),
        vec3(0    , 32.75,1),
        vec3(-0.5 , 30  , 1),
    );

    CAD_Array second_face;
    second_face = cad_array_map(points, Vec3, vec3_translate, 0, 0, 1);
    points = cad_array_concat(points, second_face);

    //     vec3(-0.3 , 25  , 2), 
    //     vec3(0.5  , 20  , 2),
    //     vec3(1.5  , 15  , 2),
    //     vec3(3    , 10  , 2),
    //     vec3(5.5  , 5.5 , 2),
    //     vec3(8    , 2.75, 2),
    //     vec3(10   , 2   , 2),
    //     vec3(15   , .5  , 2),
    //     vec3(20   , 0   , 2),
    //     vec3(25   , -.2 , 2),
    //     vec3(30   , 0   , 2),
    //     vec3(35   , 1   , 2),
    //     vec3(40   , 2.5 , 2),
    //     vec3(42   , 3   , 2),
    //     vec3(45   , 5   , 2),
    //     vec3(47.5 , 7.5 , 2),
    //     vec3(49   , 10  , 2),
    //     vec3(50   , 12  , 2),
    //     vec3(50   , 12  , 2),
    //     vec3(51   , 15  , 2),
    //     vec3(52.5 , 20  , 2),
    //     vec3(53.5 , 25  , 2),
    //     vec3(53.55, 33  , 2),
    //     vec3(53   , 35  , 2),
    //     vec3(52   , 37.5, 2),
    //     vec3(50   , 39  , 2),
    //     vec3(48.5 , 40  , 2),
    //     vec3(45   , 42  , 2),
    //     vec3(40   , 43  , 2),
    //     vec3(35   , 43.5, 2),
    //     vec3(30   , 43.5, 2),
    //     vec3(30   , 43.5, 2),
    //     vec3(25   , 43  , 2),
    //     vec3(20   , 42.5, 2),
    //     vec3(15   , 41.5, 2),
    //     vec3(10   , 40.2, 2),
    //     vec3(5    , 38.5, 2),
    //     vec3(3.5  , 37.5, 2),
    //     vec3(1.5  , 35  , 2),
    //     vec3(0    , 32.75,2),
    //     vec3(-0.5 , 30  , 2),
    // );

    CAD_Array faces = cad_faces(
        range(0, points.count / 2, 1),
        cad_array_reverse(range(points.count/2, points.count, 1))
    );

    make_little_faces_around(&faces);

    // lens 

    double curve_side1_x = 80;
    double curve_side1_y = 5;

    double curve_side2_x = 70;
    double curve_side2_y = 2;
    double curve_length = 200;

    double lens_tickness = 2;

    const double EPS = 0.1;

    CAD_Object* lens_shape = cad_polyhedron(points, faces);
    

    // CAD_Object* curve_pos = cad_frustum(curve_side1_x, curve_side2_x);
    // cad_resize(curve_pos, curve_side1_x, curve_side1_y, curve_length);
    //
    // CAD_Object* curve_neg = cad_object_copy(curve_pos);
    // cad_translate(curve_neg, 0, - lens_tickness, -EPS / 2);
    // cad_resize(curve_neg, 
    //            curve_side1_x - lens_tickness, 
    //            curve_side1_y, 
    //            curve_length + EPS);
    //
    // CAD_Object*   curve = cad_difference(curve_pos, curve_neg);
    // cad_rotate   (curve, 90, 0, 0);
    // cad_rotate   (curve, 0, 0, -20);
    // cad_translate(curve, curve_side2_x*0.3, 0, 3);
    //
    // CAD_Object* lens = cad_intersection(lens_shape, curve); 
    //
    // // frame
    // 
    // CAD_Object* cutout = cad_object_copy(lens_shape);
    //
    // CAD_Object* frame = cad_object_copy(lens);
    // cad_resize(frame, 70, 60, 2);
    // cad_translate(frame, -8, -8, 0);
    //
    // CAD_Object* final_frame = cad_difference(frame, cutout);
    //
    // cad_point_size = 1;
    // CAD_Object* reference_points[] = {
    //     cad_reference_point(0, 0, 0),
    // };
    //
    CAD_Object*  c = cad_cube(3, 3, 3);
    cad_translate(c, 4, 4, 0);

    CAD_Object* glasses = cad_union_multi(
        cad_difference(lens_shape, c)
    );
    // cad_program_register(glasses, lens);
    // cad_program_register(glasses, frame);
    // cad_program_register(glasses, curve);

    cad_program_save("examples/glasses.scad", glasses);
}
