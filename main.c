#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>

#define MAX_BUFFER_SIZE 1000000

struct ScratchBuf { char str[MAX_BUFFER_SIZE]; size_t len;};

struct ScratchBuf scratch_buffer;

void scratch_buffer_append_len(const char *string, size_t len) {
    if (len + scratch_buffer.len > MAX_BUFFER_SIZE - 1)
        exit(-1);

    memcpy(scratch_buffer.str + scratch_buffer.len, string, len);
    scratch_buffer.len += (size_t)len;
}

void scratch_buffer_append(const char *string) {
    scratch_buffer_append_len(string, strlen(string));
}

void scratch_buffer_printf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    size_t available = MAX_BUFFER_SIZE - scratch_buffer.len;
    size_t len_needed = (size_t)vsnprintf(&scratch_buffer.str[scratch_buffer.len], available, format, args);
    if (len_needed > available - 1)
        exit(-1);
    va_end(args);
    scratch_buffer.len += len_needed;
}

void scratch_buffer_append_signed_int(int64_t i) {
    scratch_buffer_printf("%lld", (long long)i);
}

void scratch_buffer_append_double(double d) {
    scratch_buffer_printf("%f", d);

    //removing unused zeroes and dot
    while (scratch_buffer.len > 0) {
        if (scratch_buffer.str[scratch_buffer.len - 1] != '0' && scratch_buffer.str[scratch_buffer.len - 1] != '.')
            return;
        scratch_buffer.len--;
    }
}

void scratch_buffer_append_char(char c) {
    if (scratch_buffer.len + 1 > MAX_BUFFER_SIZE - 1)
        exit(-1);
    scratch_buffer.str[scratch_buffer.len++] = c;
}

char *str_copy(const char *start, size_t str_len) {
    char *dst = calloc(str_len, sizeof(char));
    memcpy(dst, start, str_len);
    // No need to set the end
    return dst;
}

char *scratch_buffer_to_string(void) {
    scratch_buffer.str[scratch_buffer.len] = '\0';
    return scratch_buffer.str;
}

char *scratch_buffer_copy(void) {
    return str_copy(scratch_buffer.str, scratch_buffer.len);
}

char *scratch_buffer_str_copy(void) {
    scratch_buffer.str[scratch_buffer.len] = '\0';
    return str_copy(scratch_buffer.str, scratch_buffer.len + 1);
}

typedef struct {
    char* content;
} CAD_Object;

CAD_Object* object_printf(const char* format, ...) {
    scratch_buffer.len = 0;

    va_list args;
    va_start(args, format);
        size_t available = MAX_BUFFER_SIZE - scratch_buffer.len;
        size_t len_needed = (size_t)vsnprintf(&scratch_buffer.str[scratch_buffer.len], available, format, args);
        if (len_needed > available - 1)
            exit(-1);
    va_end(args);
    scratch_buffer.len += len_needed;

    CAD_Object* new = malloc(sizeof(CAD_Object));
    new->content = scratch_buffer_str_copy();
    return new;
}

void object_modify_printf(CAD_Object* object, const char* format, ...) {
    scratch_buffer.len = 0;

    va_list args;
    va_start(args, format);
        size_t available = MAX_BUFFER_SIZE - scratch_buffer.len;
        size_t len_needed = (size_t)vsnprintf(&scratch_buffer.str[scratch_buffer.len], available, format, args);
        if (len_needed > available - 1)
            exit(-1);
    va_end(args);
    scratch_buffer.len += len_needed;

    free(object->content);
    object->content = scratch_buffer_str_copy();
}

typedef struct {
    double x;
    double y;
} Vec2;

Vec2 vec2(double x, double y) {
    Vec2 new = {.x = x, .y = y};
    return new;
}

typedef struct {
    double x;
    double y;
    double z;
} Vec3;

Vec3 vec3(double x, double y, double z) {
    Vec3 new = {.x = x, .y = y, .z = z};
    return new;
}

typedef struct {
    size_t count;
    int* values;
} Face;

Face face(size_t count, ...) {
    int* values = malloc(sizeof(int) * count);
    va_list args;
    va_start(args, count);
        for (size_t i = 0; i < count; i++) {
            values[i] = va_arg(args, int);
        }
    va_end(args);

    Face new = {.count = count, .values = values};
    return new;
}

// Primitives
CAD_Object* cad_cube(double w, double l, double h) {
    return object_printf("cube([%f, %f, %f]);", w, l, h);
}

CAD_Object* cad_sphere(double radius) {
    return object_printf("sphere(r=%f);", radius);
}

CAD_Object* cad_cylinder(double h, double r) {
    return object_printf("cylinder(h=%f, r=%f);", h, r);
}

CAD_Object* cad_frustum(double r1, double r2) {
    return object_printf("cylinder(r1=%f, r2=%f);", r1, r2);
}

CAD_Object* polyhedron(Vec3* points, size_t pointc, Face* faces, size_t facec) {
    scratch_buffer.len = 0;
    scratch_buffer_printf("polyhedron(points=[");
    for (size_t i = 0; i < pointc; ++i)
        scratch_buffer_printf("\n        [%f, %f, %f],", points[i].x, points[i].y, points[i].z);
    scratch_buffer_printf("], \n    faces=[");

    for (size_t i = 0; i < facec; ++i) {
        scratch_buffer_printf("\n        [");
        for (size_t j = 0; j < faces[i].count; ++j){
            scratch_buffer_printf("%d", faces[i].values[j]);
            // scratch_buffer_append_double(faces[i].values[j]);
            if (j < faces[i].count - 1)
                scratch_buffer_append_char(',');
        }
        scratch_buffer_append_char(']');
        if (i < facec - 1)
            scratch_buffer_append_char(',');
    }

    scratch_buffer_printf("]);");

    CAD_Object* new = malloc(sizeof(CAD_Object));
    new->content = scratch_buffer_str_copy();
    return new;
}

// Bool Operations
CAD_Object* cad_union(CAD_Object* o1, CAD_Object* o2) {
    return object_printf("union(){\n%s\n%s\n}", o1->content, o2->content);
}

CAD_Object* cad_difference(CAD_Object* o1, CAD_Object* o2) {
    return object_printf("difference(){\n%s\n%s\n}", o1->content, o2->content);
}

CAD_Object* cad_intersection(CAD_Object* o1, CAD_Object* o2) {
    return object_printf("intersection(){\n%s\n%s\n}", o1->content, o2->content);
}

// Modifying Operations
void cad_translate(CAD_Object* object, double x, double y, double z) {
    object_modify_printf(object, "translate([%f, %f, %f])\n%s", x, y, z, object->content);
}


int main() {
    double l = 10;
    double w = 5;
    double h = 3;

    const size_t pointc = 6;
    Vec3 points[] = {
        vec3(0,0,0), vec3(l,0,0), vec3(l,w,0), vec3(0,w,0), vec3(0,w,h), vec3(l,w,h)};

    const size_t facec = 5;
    Face faces[] = {
        face(4,  0,1,2,3),
        face(4,  5,4,3,2),
        face(4,  0,4,5,1),
        face(3,  0,3,4  ),
        face(3,  5,2,1  ),
    };

    CAD_Object* my_shape = polyhedron(points, pointc, faces, facec);

    FILE *program;

    char* program_name = "test.scad";
    program = fopen(program_name, "w");
    if (program == NULL) {
        perror("Could not open file.");
        return 1;
    }
    fprintf(program, "%s\n", my_shape->content);
    fclose(program);
}

