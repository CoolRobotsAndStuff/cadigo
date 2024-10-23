#include "../cadigo.h"
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

CAD_Object* cad_object_extrude(CAD_Array points, double h) {
    CAD_Array second_face = cad_array_map(points, Vec3, vec3_translate, 0, 0, h);

    points = cad_array_concat(points, second_face);

    CAD_Array faces = cad_faces(
        range(0, points.count / 2, 1),
        cad_array_reverse(range(points.count/2, points.count, 1))
    );

    make_little_faces_around(&faces);

    return cad_polyhedron(points, faces);
}

CAD_Array lens_shape() {
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
    
    points = vec3_array_center(points);
    points = vec3_array_scale(vec3(1, 1, 0.7), points);
    // points = vec3_array_subdivide(points);
    return points;
}

CAD_Object* lens() {
    CAD_Array points = lens_shape();
    Vec3 lens_size = vec3_array_bounds_size(points);

    CAD_Object* lens_shape = cad_object_extrude(points, 0.2);

    double lens_thickness = 2;

    CAD_Object* glasses =
        cad_resize(lens_size.x, lens_size.y, lens_size.z + lens_thickness,
            cad_minkowski(
                lens_shape,
                cad_sphere(lens_thickness / 2)
            )
        );

    return glasses;
}

CAD_Object* flat_lenses(double height) {
    CAD_Array points = lens_shape();
    for (size_t i = 0; i < points.count; ++i) {
        ((Vec3*)points.values)[i].z = 0;
    }
    return cad_object_extrude(points, height);
};

CAD_Object* outer_arch(double x, double y, double z, double thickness) {
    return
        cad_extrude(z,
            cad_intersection(
                cad_difference(
                    cad_resize2D(x, y*2, cad_circle(10)),
                    cad_resize2D(
                        x - thickness *2, 
                        y*2 - thickness*2,
                        cad_circle(10)
                    )
                ),
                cad_translate(-x/2, 0, 0,
                    cad_square(x, y)
                )
            )
        );
}

CAD_Object* inner_arch(double x, double y, double z, double thickness) {
    return
        cad_extrude(z,
        cad_intersection(
            cad_difference(
                cad_resize2D(x+thickness*2, y*2+thickness*2, cad_circle(10)),
                cad_resize2D(x, y*2, cad_circle(10))
            ),
            cad_translate(-(x + thickness*2)/2, 0, 0,
                cad_square(x + thickness*2, y + thickness*2)
            )
        )
        );
}

int main() {
    CAD_Object* lens1 =
        cad_translate(-35, 0, 0,
        cad_rotate(0, -6, -6,
        cad_union(
            lens(), 
            cad_translate(0, 0, -15,
            cad_scale(0.95, 0.95, 1,
                flat_lenses(30)
            ))
        )));
    
    CAD_Object* frame1 =
        cad_translate(-35, 0, 0,
        cad_rotate(0, -6, -6,
        cad_translate(0, 0, -10,
            flat_lenses(20)
        )));

    CAD_Object* lens2  = cad_mirror(1, 0, 0, lens1);
    CAD_Object* frame2 = cad_mirror(1, 0, 0, frame1);
    
    double bridge_len = 78;
    CAD_Object* bridge = cad_cube(bridge_len, 18, 20);
    cad_translate(-bridge_len / 2, 3.2, -10, bridge);

    double glasses_thickness = 2;
    double curve_x = 200;
    double curve_y = 30;

    CAD_Object* curve = outer_arch(
        curve_x, 
        curve_y, 
        100, 
        glasses_thickness);

    cad_rotate(90, 0, 0, curve);
    cad_translate(0, 30, -curve_y + 3.5, curve);

    cad_rotate(90, 0, 0, curve);
    cad_translate(0, 30, -curve_y + 3.5, curve);


    CAD_Object* glasses =
            cad_difference(
                cad_minkowski(
                    cad_intersection(
                        curve,
                        cad_union(
                            frame1,
                            cad_union_multi(
                                frame2,
                                bridge
                            )   
                        )
                    ),
                    cad_sphere(1)
                ),
                cad_union_multi(
                    lens1,
                    lens2,
                    cad_translate(-40, 13.9, -10,
                        cad_cube(80, 6.1, 20)
                    ),
                    cad_translate(0, 6, -10,
                        outer_arch(28.5, 13, 20, 5)
                    )
                )
            )
    ;
    
    cad_program_save("examples/glasses.scad", glasses, 2.1);
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
