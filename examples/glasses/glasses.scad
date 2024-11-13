include <extruded_glasses_shape.scad>
include <flattened_glasses_shape.scad>

sep = 10;
frame_scaleup = 1.2;
lens_width = flattened_glasses_shape_max[0] - flattened_glasses_shape_min[0];

brow_thick = 15;

extruded_glasses_shape();

union() {
    mirror([1, 0]) 
    translate([-sep*.5, 0, 0])
    flattened_glasses_shape();

    translate([-sep*.5, 0, 0])
    flattened_glasses_shape();
    
    translate([0, -(brow_thick/2)+flattened_glasses_shape_max[1]]) 
    square([sep+(lens_width*2), brow_thick], center=true);
}


