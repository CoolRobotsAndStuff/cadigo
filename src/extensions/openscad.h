#ifndef OPENSCAD_H_
#define OPENSCAD_H_

#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include "../cadigo.h"
#include "../scratch_buffer.c"

#endif // OPENSCAD_H_

bool cad_to_openSCAD_module(CAD obj, const char* file_path, const char* module_name);
bool cad_to_openSCAD(CAD obj, const char* file_path, const char* object_name);

bool cad_fprint_openSCAD_module(FILE* file, CAD obj, const char* module_name);
bool cad_fprint_openSCAD(FILE* file, CAD obj, const char* object_name);

bool cad_to_openSCAD(CAD obj, const char* file_path, const char* object_name) {
    FILE* program_file;
    program_file = fopen(file_path, "w");
    if (program_file == NULL) {
        perror("Could not open file.");
        return false;
    }
    cad_fprint_openSCAD(program_file, obj, object_name);
    fclose(program_file);
    return true;
}

bool cad_to_openSCAD_module(CAD obj, const char* file_path, const char* module_name){
    FILE* program_file;
    program_file = fopen(file_path, "w");
    if (program_file == NULL) {
        perror("Could not open file.");
        return false;
    }
    cad_fprint_openSCAD_module(program_file, obj,  module_name);
    fclose(program_file);
    return true;
}

bool cad_fprint_openSCAD(FILE* file, CAD obj, const char* object_name) {
    bool is_polygon = obj.faces.count == 1;
    sb_clear();
    if (is_polygon) sb_append("polygon(\n");
    else            sb_append("polyhedron(\n");
    sb_append("    points=[\n");
    Vec3 p;
    for (size_t i = 0; i < obj.points.count; ++i) {
        p = obj.points.items[i];

        if (is_polygon) sb_printf("        [%.10Lf, %.10Lf],\n", p.x, p.y);
        else            sb_printf("        [%.10Lf, %.10Lf, %.10Lf],\n", p.x, p.y, p.z);
    }
    
    if (!is_polygon) {
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
    }
    sb_append("    ]\n);\n");

    Bounds b = cad_get_bounds(obj);
    sb_printf("%s_min = [%.10Lf, %.10Lf, %.10Lf];\n", object_name, b.min.x, b.min.y, b.min.z);
    sb_printf("%s_max = [%.10Lf, %.10Lf, %.10Lf];\n", object_name, b.max.x, b.max.y, b.max.z);
    fprintf(file, "%s", sb_to_string());
    return true;
}

bool cad_fprint_openSCAD_module(FILE* file, CAD obj, const char* module_name) {
    bool is_polygon = obj.faces.count == 1;
    sb_clear();
    sb_printf("module %s(){\n", module_name);
    if (is_polygon) sb_append("    polygon(\n");
    else            sb_append("    polyhedron(\n");
    sb_append("        points=[\n");

    for (size_t i = 0; i < obj.points.count; ++i) {
        Vec3 p = obj.points.items[i];

        if (is_polygon) sb_printf("            [%.10Lf, %.10Lf],\n", p.x, p.y);
        else            sb_printf("            [%.10Lf, %.10Lf, %.10Lf],\n", p.x, p.y, p.z);
    }


    if (!is_polygon) {
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
    }

    sb_append("        ]\n);\n");
    sb_append("}\n");

    Bounds b = cad_get_bounds(obj);
    sb_printf("%s_min = [%.10Lf, %.10Lf, %.10Lf];\n", module_name, b.min.x, b.min.y, b.min.z);
    sb_printf("%s_max = [%.10Lf, %.10Lf, %.10Lf];\n", module_name, b.max.x, b.max.y, b.max.z);

    fprintf(file, "%s", sb_to_string());
    return true;
}


