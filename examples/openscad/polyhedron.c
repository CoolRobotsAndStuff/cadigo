
#define CADIGO_IMPLEMENTATION
#include "cadigo.h"
#include "extensions/openscad.h"

#include <stdio.h>

int main() {
    CAD* c = cad_cube(30); 
    cad_repeat(4) cad_catmull_clark(c);

    cad_to_openSCAD(*c, "examples/openscad/polyhedron.scad", "polyhedron");

    return 0;
}
