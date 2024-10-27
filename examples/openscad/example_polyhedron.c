#include "extensions/openscad.h"

#define CADIGO_IMPLEMENTATION
#include "cadigo.h"

#include <stdio.h>

int main() {
    CAD c = cad_catmull_clark(cad_cube(3));


    FILE *program_file;
    program_file = fopen("examples/openscad/example_polyhedron.scad", "w");
    if (program_file == NULL) {
        perror("Could not open file.");
        return 1;
    }
    cad_to_openSCAD(program_file, "my_cube", c);
    fclose(program_file);
    return 0;
}
