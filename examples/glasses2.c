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

    points = vec3_array_subdivide(points);

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

CAD_Object* lens(double thickness) {
    CAD_Array points = lens_shape();
    Vec3 lens_size = vec3_array_bounds_size(points);
    CAD_Object* lens_shape = cad_object_extrude(points, 0.2);

    CAD_Object* lens =
        cad_resize(lens_size.x, lens_size.y, lens_size.z + thickness,
            cad_minkowski(
                lens_shape,
                cad_sphere(thickness / 2)
            )
        );

    return lens;
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
                cad_resize2D(x              , y*2              , cad_circle(10)),
                cad_resize2D(x - thickness*2, y*2 - thickness*2, cad_circle(10))
            ),
            cad_translate(-x/2, 0, 0, cad_square(x, y))
        ));
}

CAD_Object* inner_arch(double x, double y, double z, double thickness, double v_offset) {
    CAD_Object* diff_square = 
        cad_translate(-(x + thickness*2)/2, 0, 0,
            cad_square(x + thickness*2, y + thickness*2 + v_offset)
        );

    return
        cad_extrude(z,
        cad_difference(
                cad_resize2D(x+thickness*2, y+thickness*2,
                cad_intersection(
                    cad_translate(0, -v_offset, 0,
                    cad_resize2D(x+thickness*2, y*2+thickness*2+v_offset*2,
                        cad_circle(10)
                    )),
                    diff_square
                )),

                cad_resize2D(x, y,
                cad_intersection(
                    cad_translate(0, -v_offset, 0,
                    cad_resize2D(x, y*2+v_offset*2,
                        cad_circle(10)
                    )),
                    diff_square
                ))
            )
        ) ;
}

CAD_Object* bridge(double r, double y, double margin_x, double margin_z) {
    return 
        cad_extrude(y,
        cad_difference(
            cad_square((r+margin_x)*2, r*2+margin_z),
            cad_translate(margin_x+r, 0, 0,
                cad_circle(r)
            )
        ));
}

#define TEMPLE_LENGTH 110
#define TEMPLE_HEIGHT 6
#define TEMPLE_WIDTH 3.5
#define TEMPLE_ROTATION -140
#define TEMPLE_END_LEN 40
#define TEMPLE_SMOOTHING 0

CAD_Object* temple() {
    double h = TEMPLE_HEIGHT - TEMPLE_SMOOTHING*2;
    double l = TEMPLE_LENGTH - TEMPLE_SMOOTHING*2;
    double w = TEMPLE_WIDTH  - TEMPLE_SMOOTHING*2;
    double o = TEMPLE_SMOOTHING;
    CAD_Object* ret = cad_cube(w, l, h);
    cad_add(ret, cad_translate(0, 0, h ,
                 cad_rotate(TEMPLE_ROTATION, 0, 0, cad_cube(w, TEMPLE_END_LEN, h)
                            )));
    ret = cad_minkowski(ret, cad_sphere(TEMPLE_SMOOTHING));
    cad_translate(o, o, o, ret);
    return ret;
}


#define SEPARATION 15.0
#define LENS_THICKNESS 2.0
#define FRAME_THICKNESS 4.0

#define LENS_RECESS_ADJUSTMENT 1.6 

#define LENS_CURVATURE -4
#define MAX_HEIGHT 40.0
#define OFFSET (MAX_HEIGHT / 2)

#define LENS_RECESS_FACTOR  0.05
#define BRIDGE_NOSE_HEIGHT 15

#define bridge_y_level 25

int main() {
    CAD_Array points = lens_shape();
    CAD_Object* lens_shape = cad_object_extrude(points, LENS_THICKNESS);

    Vec3 lens_size = vec3_array_bounds_size(points);
    lens_size.z += LENS_THICKNESS;


    CAD_Object* lens =
        cad_resize_v(lens_size,
            cad_minkowski(
                lens_shape,
                cad_sphere(LENS_THICKNESS / 2)
            )
        );

    CAD_Object* lens1 =
        cad_rotate(0, LENS_CURVATURE, -6,
        cad_translate(-lens_size.x/2 - .5 - SEPARATION/2, 0, 0,
            cad_union(
                lens, 

                cad_translate(0, 0, -OFFSET,
                cad_scale(1-LENS_RECESS_FACTOR, 1-LENS_RECESS_FACTOR, 1,
                    flat_lenses(MAX_HEIGHT)
                ))
            )
        ));

    double curve_x = 200;
    double curve_y = LENS_THICKNESS + 4 * LENS_CURVATURE * -1;

    CAD_Object* curve = outer_arch(
        curve_x, 
        curve_y, 
        100, 
        FRAME_THICKNESS - 1);

    cad_rotate(90, 0, 0, curve);
    cad_translate(0, 30, -curve_y + FRAME_THICKNESS/2 - LENS_RECESS_ADJUSTMENT, curve);

    CAD_Object* frame1 =
        cad_rotate(0, LENS_CURVATURE, -6,
        cad_translate(-lens_size.x/2 - .5 - SEPARATION/2, 0, -OFFSET/2,
            flat_lenses(MAX_HEIGHT/2)
        ));


    CAD_Object* lens2  = cad_mirror(1, 0, 0, lens1);
    CAD_Object* frame2 = cad_mirror(1, 0, 0, frame1);
    
    double bridge_width = SEPARATION + lens_size.x * 1.1;

    CAD_Object* positive = 
        cad_difference(
            cad_union(
                cad_union(
                    frame1,
                    frame2
                ),
                cad_translate(-bridge_width/2, lens_size.y/2-.22+2, -OFFSET/2,
                    cad_cube(bridge_width, FRAME_THICKNESS-2, MAX_HEIGHT/2)
                )
            ),
            cad_translate(-bridge_width/2, lens_size.y/2+3.5-bridge_y_level, -OFFSET/2,
                cad_cube(bridge_width, bridge_y_level, MAX_HEIGHT/2)
            )
        );
    
    positive = cad_union(frame2, frame1);

    CAD_Object* bridge = cad_translate(0, lens_size.y/2-.2+3-bridge_y_level, -OFFSET/2,
                inner_arch(SEPARATION + (bridge_y_level - 10)*0.38, BRIDGE_NOSE_HEIGHT, MAX_HEIGHT/2, 0.5, 15)
            );

    positive = cad_intersection(positive, curve);
    positive = cad_minkowski(positive, cad_sphere(1));

    bridge = cad_intersection(bridge, curve);
    bridge = cad_minkowski(bridge, cad_sphere(1));

    //positive = cad_union(positive, bridge);

    CAD_Object* negative = cad_union_multi(lens1, lens2);



    CAD_Object* lenses = cad_difference(positive, negative);

    cad_rotate(-90, 180, 0, lenses);

    CAD_Object* t = temple();
    cad_translate(SEPARATION/2+lens_size.x-0.2, -TEMPLE_LENGTH-2.8, 10, t);

    CAD_Object* t2 = cad_mirror(1, 0, 0, t);

    cad_program_save("examples/glasses.scad", 
                     lens_shape,
                     2.1);


    return 1;
}




