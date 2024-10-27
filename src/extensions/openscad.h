#ifndef OPENSCAD_H_
#define OPENSCAD_H_

#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include "../cadigo.h"
#include "../scratch_buffer.c"

#endif // OPENSCAD_H_

int cad_to_openSCAD(FILE* file, char* obj_name, CAD obj) {
    (void) obj_name;
    sb_clear();
    //sb_printf("module %s(){\n", obj_name);
    sb_append("polyhedron(\n");
    sb_append("    points=[\n");
    Vec3 p;
    for (size_t i = 0; i < obj.points.count; ++i) {
        p = obj.points.items[i];
        sb_printf("        [%.10Lf, %.10Lf, %.10Lf],\n", p.x, p.y, p.z);
    }
    sb_append("    ],\n");
    sb_append("    faces=[\n");

    Face f;
    for (size_t i = 0; i < obj.faces.count; ++i) {
        sb_append("        [");
        f = obj.faces.items[i];
        for (size_t j = 0; j < f.count; ++j) {
            sb_printf("%zu,", f.items[j]);
        } 
        sb_append("],\n");
    }

    sb_append("    ]\n);\n");
    //sb_append("}\n");
    fprintf(file, "%s", sb_to_string());

    return 0;
}
