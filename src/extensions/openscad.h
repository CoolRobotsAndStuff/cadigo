#ifndef OPENSCAD_H_
#define OPENSCAD_H_

#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include "../cadigo.h"
#include "../scratch_buffer.c"

#endif // OPENSCAD_H_

bool cad_to_openSCAD_module(const char* file_path, const char* module_name, CAD obj);
bool cad_to_openSCAD(const char* file_path, CAD obj);

bool cad_fprintf_openSCAD_module(FILE* file, const char* module_name, CAD obj);
bool cad_fprintf_openSCAD(FILE* file, CAD obj);

bool cad_to_openSCAD(const char* file_path, CAD obj) {
    FILE* program_file;
    program_file = fopen(file_path, "w");
    if (program_file == NULL) {
        perror("Could not open file.");
        return false;
    }
    cad_fprintf_openSCAD(program_file, obj);
    fclose(program_file);
    return true;
}

bool cad_to_openSCAD_module(const char* file_path, const char* module_name, CAD obj){
    FILE* program_file;
    program_file = fopen(file_path, "w");
    if (program_file == NULL) {
        perror("Could not open file.");
        return false;
    }
    cad_fprintf_openSCAD_module(program_file, module_name, obj);
    fclose(program_file);
    return true;
}

bool cad_fprintf_openSCAD(FILE* file, CAD obj) {
    sb_clear();
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
    fprintf(file, "%s", sb_to_string());
    return true;
}

bool cad_fprintf_openSCAD_module(FILE* file, const char* module_name, CAD obj) {
    sb_clear();
    sb_printf("module %s(){\n", module_name);
    sb_append("    polyhedron(\n");
    sb_append("        points=[\n");

    for (size_t i = 0; i < obj.points.count; ++i) {
        Vec3 p = obj.points.items[i];
        sb_printf("            [%.10Lf, %.10Lf, %.10Lf],\n", p.x, p.y, p.z);
    }

    sb_append("        ],\n");
    sb_append("        faces=[\n");

    for (size_t i = 0; i < obj.faces.count; ++i) {
        sb_append("            [");
        Face f = obj.faces.items[i];
        for (size_t j = 0; j < f.count; ++j) {
            sb_printf("%zu,", f.items[j]);
        } 
        sb_append("],\n");
    }

    sb_append("        ]\n);\n");
    sb_append("}\n");
    fprintf(file, "%s", sb_to_string());
    return true;
}
