#include "extensions/openscad.h"

#define CADIGO_IMPLEMENTATION
#include "cadigo.h"

#include <stdio.h>

val_t normal_distribution(val_t x) {
    val_t sigma = .4;
    val_t mu = 0;
    val_t coefficient = 1.0 / (sigma * sqrt(2.0 * M_PI));
    val_t exponent = -((x - mu) * (x - mu)) / (2.0 * sigma * sigma);
    return coefficient * exp(exponent);
}

val_t cad_cos(val_t x) { return (val_t)cos((val_t)x); }
val_t cad_sin(val_t x) { return (val_t)sin((val_t)x); }

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
    //cad_catmull_clark(&extruded_glasses_shape);
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
    

    //CAD bridge_curve = cad_xy_curve_from_function(normal_distribution, -0.8, 0.8, 30);

    CAD bridge_curve = cad_xy_curve_from_function(cad_sin, -M_PI*1.5, M_PI*0.5, 30);
    Bounds b = cad_get_bounds(bridge_curve);
    Vec3 size = cad_get_size(bridge_curve);
    cad_translate(vec3(-b.max.x + size.x /2, -b.min.y, 0), &bridge_curve);
    cad_curve_to_polygon(&bridge_curve);
    cad_to_openSCAD_module("examples/glasses/bridge_curve.scad", "bridge_curve", bridge_curve);

    cad_rotate(vec3(60, 0, 0), &extruded_glasses_shape);

    while (true) {
        cad_rotate(vec3(0, 5, 0), &extruded_glasses_shape);
        cad_render_to_terminal(0.04, extruded_glasses_shape);
        usleep(100000);
    }

    return 0;
}
