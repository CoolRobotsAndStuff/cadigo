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
        vec3(-0.3 , 25  , 0.5), 
        vec3(0.5  , 20  , 0.5),
        vec3(1.5  , 15  , 0.5),
        vec3(3    , 10  , 0.5),
        vec3(5.5  , 5.5 , 0.3),
        vec3(7.75 , 2.75, 0),
        vec3(10   , 1.5 , 0.2),
        vec3(15   , 0.5 , 1),
        vec3(20   , 0.2 , 1.4),
        vec3(25   , 0   , 1.70),
        vec3(30   , 0.2 , 1.75),
        vec3(35   , 0.9   , 1.75),
        vec3(40   , 2.1 , 1.5),
        vec3(42   , 3   , 1.4),
        vec3(45   , 5   , 1.3),
        vec3(47.5 , 7.5 , 1.5),
        vec3(49   , 10  , 1.5),
        vec3(50   , 12  , 1.5),
        vec3(51   , 15  , 1.5),
        vec3(52.5 , 20  , 1.3),
        vec3(53.5 , 25  , 1),
        vec3(54   , 30  , 0.5),
        vec3(53.55, 33  , 0.1),
        vec3(53   , 35  , 0.03),
        vec3(52   , 37.5, 0),
        vec3(50   , 39  , 0.05),
        vec3(48.5 , 40  , .1),
        vec3(45   , 41.5, 0.75),
        vec3(40   , 42  , 1.4),
        vec3(35   , 42.2, 1.75),
        vec3(30   , 42.2, 2),
        vec3(25   , 42.1  , 2.2),
        vec3(20   , 41.7, 2.1),
        vec3(15   , 41  , 2),
        vec3(10   , 40.2, 1.5),
        vec3(5    , 38.5, 0.5),
        vec3(3.5  , 37.5, 0.2),
        vec3(1    , 35  , 0),
        vec3(0    , 32.75,0),
        vec3(-0.5 , 30  , 0.1),
    );
    
    points = vec3_array_scale(vec3(1, 1, 0.7), points);

    points = vec3_array_subdivide(points);

    Vec3 max_point = vec3_array_max(points);
    Vec3 min_point = vec3_array_min(points);
    Vec3 lens_size = {
        .x = max_point.x - min_point.x,
        .y = max_point.y - min_point.y,
        .z = max_point.z - min_point.z
    };


    CAD_Array second_face;
    second_face = cad_array_map(points, Vec3, vec3_translate, 0, 0, 0.1);
    points = cad_array_concat(points, second_face);

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

    double lens_thickness = 2;

    const double EPS = 0.1;

    CAD_Object* lens_shape = cad_polyhedron(points, faces);

    CAD_Object* glasses =
        cad_resize( lens_size.x, lens_size.y, lens_size.z + lens_thickness,
            cad_minkowski(
                lens_shape,
                cad_sphere(lens_thickness / 2)
            )
        );

    CAD_Object* low_res_glasses =
        cad_resize( lens_size.x, lens_size.y, lens_size.z + lens_thickness, lens_shape);
    
    // CAD_Object* glasses = cad_union_multi(
    //     cad_extrude(lens_shape, 2),
    //     cad_translate(cad_extrude(lens_shape2, 2), 0, 60, 0),
    // );
    cad_program_save("examples/glasses.scad", glasses, 20);
    // cad_program_save("examples/glasses.scad", low_res_glasses, 5);
    // cad_program_save("examples/test_cube.scad", 
    //                  cad_difference(
    //                     cad_cube(50, 2, 50),
    //                     cad_translate(
    //                         cad_rotate(cad_cube(50, 4, 4), 0, -45, 0),
    //                         7, -1, 5
    //                     )
    //                  )
    //             );
}
