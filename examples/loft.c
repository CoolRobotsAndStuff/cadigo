#include "../cadigo.h"
#include <stdio.h>


int main() {
    double l = 10;
    double w = 5;
    double h = 3;

    CAD_Array points = vectors(
        vec3(0,0, 0), 
        vec3(10,0,0),
        vec3(10,5,0),
        vec3(0,5,0),
        vec3(0,5,3),
        vec3(10,5,3)
    );
    
    CAD_Array faces = cad_faces(
        cad_face(0,1,2,3),
        cad_face(5,4,3,2),
        cad_face(0,4,5,1),
        cad_face(0,3,4  ),
        cad_face(5,2,1  ),
    );

    Vec3* v = (Vec3*)points.values;
    printf("hello: %f\n", v[2].y);
    CAD_Object* my_shape = polyhedron(points, faces);

    FILE *program;

    char* program_name = "loft.scad";
    program = fopen(program_name, "w");
    if (program == NULL) {
        perror("Could not open file.");
        return 1;
    }
    fprintf(program, "%s\n", my_shape->content);
    fclose(program);
}
