#include "extensions/openscad.h"

#define CADIGO_IMPLEMENTATION
#include "cadigo.h"

#include <stdio.h>

int main() {
    CAD c = {
        .points = points(
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
        )
    };
    
    c.faces = (Faces) {
        .count = 1,
        .capacity = 2,
    };
    c.faces.items = malloc(c.faces.capacity * sizeof(Face));

    Face f = {
        .count = c.points.count,
        .capacity = c.points.count + 1
    };
    f.items = malloc(f.capacity * sizeof(size_t));

    for (size_t i = 0; i < f.count; ++i) f.items[i] = i;

    c.faces.items[0] = f;

    c = cad_extrude(c, 4);

    // c = cad_catmull_clark(
    //     cad_catmull_clark(
    //     cad_catmull_clark(
    //     cad_catmull_clark(
    //         c
    //     ))));

    FILE *program_file;
    program_file = fopen("examples/glasses/glasses_shape.scad", "w");
    if (program_file == NULL) {
        perror("Could not open file.");
        return 1;
    }
    cad_to_openSCAD(program_file, "glasses_shape", c);
    fclose(program_file);
    return 0;
}
