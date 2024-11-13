#include "extensions/openscad.h"

#define CADIGO_IMPLEMENTATION
#include "cadigo.h"

#include <stdio.h>

int main() {
    CAD glasses_shape = cad_polygon_from_points(

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
    
    CAD extruded_glasses_shape = cad_clone(glasses_shape);

    cad_extrude(2, &extruded_glasses_shape);
    cad_inset_face(1, .1, &extruded_glasses_shape);
    cad_inset_face(0, .1, &extruded_glasses_shape);
    cad_catmull_clark(&extruded_glasses_shape);
    cad_catmull_clark(&extruded_glasses_shape);
    cad_translate(vec3(-cad_get_bounds(extruded_glasses_shape).max.x, 0, 0), &extruded_glasses_shape);
    cad_to_openSCAD_module("examples/glasses/extruded_glasses_shape.scad", "extruded_glasses_shape", extruded_glasses_shape);
    //cad_to_openSCAD("examples/glasses/extruded_glasses_shape.scad", extruded_glasses_shape);


    CAD flattened_glasses_shape = cad_clone(glasses_shape);

    cad_scale(vec3(1, 1, 0), &flattened_glasses_shape);
    cad_hotpoints_subdivision(&flattened_glasses_shape);
    cad_hotpoints_subdivision(&flattened_glasses_shape);
    cad_translate(vec3(-cad_get_bounds(flattened_glasses_shape).max.x, 0, 0), &flattened_glasses_shape);
    cad_to_openSCAD_module("examples/glasses/flattened_glasses_shape.scad", "flattened_glasses_shape", flattened_glasses_shape);
    //cad_to_openSCAD("examples/glasses/flattened_glasses_shape.scad", flattened_glasses_shape);

    return 0;
}
