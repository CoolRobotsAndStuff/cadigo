#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>

double cad_point_size = 1;

// --Scratch buffer
#define MAX_BUFFER_SIZE 1000000


struct CAD_Scratch_Buf { char str[MAX_BUFFER_SIZE]; size_t len;};
struct CAD_Scratch_Buf cad_scratch_buffer;

void sb_append_len(const char *string, size_t len) {
    if (len + cad_scratch_buffer.len > MAX_BUFFER_SIZE - 1)
        exit(-1);

    memcpy(cad_scratch_buffer.str + cad_scratch_buffer.len, string, len);
    cad_scratch_buffer.len += (size_t)len;
}

void sb_append(const char *string) {
    sb_append_len(string, strlen(string));
}

void sb_printf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    size_t available = MAX_BUFFER_SIZE - cad_scratch_buffer.len;
    size_t len_needed = (size_t)vsnprintf(&cad_scratch_buffer.str[cad_scratch_buffer.len], available, format, args);
    if (len_needed > available - 1)
        exit(-1);
    va_end(args);
    cad_scratch_buffer.len += len_needed;
}

void sb_append_signed_int(int64_t i) {
    sb_printf("%lld", (long long)i);
}

void sb_append_double(double d) {
    sb_printf("%f", d);

    //removing unused zeroes and dot
    while (cad_scratch_buffer.len > 0) {
        if (cad_scratch_buffer.str[cad_scratch_buffer.len - 1] != '0' && cad_scratch_buffer.str[cad_scratch_buffer.len - 1] != '.')
            return;
        cad_scratch_buffer.len--;
    }
}

void sb_append_char(char c) {
    if (cad_scratch_buffer.len + 1 > MAX_BUFFER_SIZE - 1)
        exit(-1);
    cad_scratch_buffer.str[cad_scratch_buffer.len++] = c;
}

char *str_copy(const char *start, size_t str_len) {
    char* dst = (char*)calloc(str_len, sizeof(char));
    memcpy(dst, start, str_len);
    // No need to set the end
    return dst;
}

char *sb_to_string(void) {
    cad_scratch_buffer.str[cad_scratch_buffer.len] = '\0';
    return cad_scratch_buffer.str;
}

char *sb_copy(void) {
    return str_copy(cad_scratch_buffer.str, cad_scratch_buffer.len);
}

char *sb_str_copy(void) {
    cad_scratch_buffer.str[cad_scratch_buffer.len] = '\0';
    return str_copy(cad_scratch_buffer.str, cad_scratch_buffer.len + 1);
}


// --Object

typedef struct {
    char* content;
} CAD_Object;



CAD_Object* object_printf(const char* format, ...) {
    cad_scratch_buffer.len = 0;

    va_list args;
    va_start(args, format);
        size_t available = MAX_BUFFER_SIZE - cad_scratch_buffer.len;
        size_t len_needed = (size_t)vsnprintf(&cad_scratch_buffer.str[cad_scratch_buffer.len], available, format, args);
        if (len_needed > available - 1)
            exit(-1);
    va_end(args);
    cad_scratch_buffer.len += len_needed;

    CAD_Object* newobj = (CAD_Object*)malloc(sizeof(CAD_Object));
    newobj->content = sb_str_copy();
    return newobj;
}


void object_modify_printf(CAD_Object* object, const char* format, ...) {
    cad_scratch_buffer.len = 0;

    va_list args;
    va_start(args, format);
        size_t available = MAX_BUFFER_SIZE - cad_scratch_buffer.len;
        size_t len_needed = (size_t)vsnprintf(&cad_scratch_buffer.str[cad_scratch_buffer.len], available, format, args);
        if (len_needed > available - 1)
            exit(-1);
    va_end(args);
    cad_scratch_buffer.len += len_needed;

    free(object->content);
    object->content = sb_str_copy();
}

typedef struct {
    double x;
    double y;
} Vec2;

Vec2 vec2(double x, double y) {
    Vec2 vec = {.x = x, .y = y};
    return vec;
}

typedef struct {
    double x;
    double y;
    double z;
} Vec3;

Vec3 vec3(double x, double y, double z) {
    Vec3 vec = {.x = x, .y = y, .z = z};
    return vec;
}

Vec3 vec3_translate(Vec3 vec, double x, double y, double z) {
    Vec3 new_vec = {
        .x = vec.x + x,
        .y = vec.y + y,
        .z = vec.z + z
    };
    return new_vec;
}

// --array
typedef struct {
    size_t count;
    size_t element_size;
    void*  values;
} CAD_Array;

void cad_array_append(CAD_Array* array, void* value) {
    array->count += 1;
    array->values = realloc(array->values, array->element_size * array->count);
    memcpy((char*)array->values + (array->count-1) * array->element_size, value, array->element_size);    
}

CAD_Array cad_array_concat(CAD_Array a1, CAD_Array a2) {
    assert(a1.element_size == a2.element_size);
    CAD_Array ret = {
        .count = a1.count +  a2.count,
        .element_size=a1.element_size
    };
    ret.values = malloc(ret.element_size * ret.count);
    memcpy(ret.values, a1.values, a1.element_size * a1.count);
    memcpy( (char*)ret.values + (a1.element_size * a1.count),
            a2.values,
            a2.element_size * a2.count);
    return ret;
};

CAD_Array cad_array_reverse(CAD_Array a) {
    CAD_Array ret = {
        .count=a.count,
        .element_size=a.element_size,
        .values = malloc(a.count * a.element_size)
    };

    for (size_t i = 0; i < a.count; ++i) {
        memcpy((char*)ret.values + (ret.count - i - 1) * ret.element_size, 
               (char*)  a.values + i * a.element_size,
               a.element_size);
    }
    return ret;
}

#define cad_array_map(array, type, function, ...) ({                            \
    CAD_Array new_array = {.count=array.count, .element_size=array.element_size};       \
    new_array.values = (type*)malloc(new_array.count * new_array.element_size);          \
    type* vals = (type*)array.values;                                                         \
    for (size_t i=0; i < array.count; ++i) {                                           \
        type new_value = function(vals[i], __VA_ARGS__); \
        memcpy((char*)new_array.values + new_array.element_size * i, &new_value, new_array.element_size);                                  \
    }                                                                                   \
    new_array;                                                                          \
})

CAD_Array cad_array_copy(CAD_Array array) {
    CAD_Array new_array = {.count=array.count, .element_size=array.element_size};
    new_array.values = malloc(new_array.count * new_array.element_size);
    return new_array;
}

#define vectors3(...) ({                        \
    Vec3 temp[] = {__VA_ARGS__};               \
    Vec3* array = (Vec3*)malloc(sizeof(temp)); \
    memcpy(array, temp, sizeof(temp));         \
    CAD_Array vecs = {                         \
        .count = sizeof(temp)/sizeof(Vec3),    \
        .element_size = sizeof(Vec3),          \
        .values = array                        \
    };                                         \
    vecs;                                      \
})

#define vectors2(...) ({                        \
    Vec2 temp[] = {__VA_ARGS__};               \
    Vec2* array = (Vec2*)malloc(sizeof(temp)); \
    memcpy(array, temp, sizeof(temp));         \
    CAD_Array vecs = {                         \
        .count = sizeof(temp)/sizeof(Vec2),    \
        .element_size = sizeof(Vec2),          \
        .values = array                        \
    };                                         \
    vecs;                                      \
})


#define cad_face(...) ({                     \
    int temp[] = {__VA_ARGS__};              \
    int* array = (int*)malloc(sizeof(temp)); \
    memcpy(array, temp, sizeof(temp));       \
    CAD_Array newface = {                    \
        .count = sizeof(temp)/sizeof(int),   \
        .element_size = sizeof(int),         \
        .values = array                      \
    };                                       \
    newface;                                 \
})

#define cad_faces(...) ({                                 \
    CAD_Array temp[] = {__VA_ARGS__};                     \
    CAD_Array* array = (CAD_Array*)malloc(sizeof(temp));  \
    memcpy(array, temp, sizeof(temp));                    \
    CAD_Array faces = {                                   \
        .count = sizeof(temp)/sizeof(CAD_Array),          \
        .element_size = sizeof(CAD_Array),                \
        .values = array                                   \
    };                                                    \
    faces;                                                \
})

int cad_program_save(char* name, CAD_Object* program) {
    FILE *program_file;

    program_file = fopen(name, "w");
    if (program_file == NULL) {
        perror("Could not open file.");
        return 1;
    }
    fprintf(program_file, "%s\n", program->content);
    fclose(program_file);
    return 0;
}

// Primitives
// 3D
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
    return object_printf("cylinder(r1=%f, r2=%f, center=true);", r1, r2);
}

CAD_Object* cad_polyhedron(CAD_Array points, CAD_Array faces) {
    cad_scratch_buffer.len = 0;
    sb_printf("polyhedron(points=[");

    Vec3* v = (Vec3*)points.values;
    printf("hello2: %f\n", v[2].y);
    for (size_t i = 0; i < points.count; ++i){
        printf("\n        [%f, %f, %f],",
                              v[i].x, 
                              v[i].y, 
                              v[i].z);
        sb_printf("\n        [%f, %f, %f],",
                              v[i].x, 
                              v[i].y, 
                              v[i].z);
    }
    sb_printf("], \n    faces=[");

    CAD_Array* faces_array = (CAD_Array*)faces.values;
    for (size_t i = 0; i < faces.count; ++i) {
        sb_printf("\n        [");
        int* point_indexes = (int*)faces_array[i].values;
        for (size_t j = 0; j < faces_array[i].count; ++j) {
            sb_printf("%d", point_indexes[j]);
            if (j < faces_array[i].count - 1) {
                sb_append_char(',');
            }
        }
        sb_append_char(']');
        if (i < faces.count - 1) {
            sb_append_char(',');
        }
    }
    sb_printf("]);");

    CAD_Object* newobj = (CAD_Object*)malloc(sizeof(CAD_Object));
    newobj->content = sb_str_copy();
    return newobj;
}

// 2D

CAD_Object* cad_circle(double radius) {
    return object_printf("circle(r=%f);", radius);
}

CAD_Object* cad_polygon(CAD_Array points) {
    cad_scratch_buffer.len = 0;
    sb_printf("polygon(points=[");

    Vec2* v = (Vec2*)points.values;
    for (size_t i = 0; i < points.count; ++i){
        sb_printf("\n        [%f, %f],",
                              v[i].x, 
                              v[i].y);
    }
    sb_printf("]);\n");

    CAD_Object* newobj = (CAD_Object*)malloc(sizeof(CAD_Object));
    newobj->content = sb_str_copy();
    return newobj;
}


// Bool Operations
CAD_Object* cad_union(CAD_Object* o1, CAD_Object* o2) {
    return object_printf("union(){\n%s\n%s\n}", o1->content, o2->content);
}

#define cad_union_multi(...) ({                                  \
    cad_scratch_buffer.len = 0;                                  \
    CAD_Object* temp[] = {__VA_ARGS__};                          \
    sb_printf("union(){\n");                                     \
    for (size_t i=0; i < sizeof(temp)/sizeof(CAD_Object*); ++i)  \
        sb_printf(temp[i]->content);                             \
    sb_printf("}\n");                                            \
    CAD_Object* newobj = (CAD_Object*)malloc(sizeof(CAD_Object));\
    newobj->content = sb_str_copy();                             \
    newobj;                                                      \
})                                                               

CAD_Object* cad_difference(CAD_Object* o1, CAD_Object* o2) {
    return object_printf("difference(){\n%s\n%s\n}", o1->content, o2->content);
}

CAD_Object* cad_intersection(CAD_Object* o1, CAD_Object* o2) {
    return object_printf("intersection(){\n%s\n%s\n}", o1->content, o2->content);
}

CAD_Object* cad_minkowski(CAD_Object* o1, CAD_Object* o2) {
    return object_printf("minkowski(){\n%s\n%s\n}", o1->content, o2->content);
}

// Modifying Operations
void cad_translate(CAD_Object* object, double x, double y, double z) {
    object_modify_printf(object, "translate([%f, %f, %f])\n%s", x, y, z, object->content);
}

void cad_resize(CAD_Object* object, double x, double y, double z) {
    object_modify_printf(object, "resize([%f, %f, %f])\n%s", x, y, z, object->content);
}

void cad_scale(CAD_Object* object, double x, double y, double z) {
    object_modify_printf(object, "scale([%f, %f, %f])\n%s", x, y, z, object->content);
}

void cad_rotate(CAD_Object* object, double x, double y, double z) {
    object_modify_printf(object, "rotate([%f, %f, %f])\n%s", x, y, z, object->content);
}

void cad_resize2D(CAD_Object* object, double x, double y) {
    object_modify_printf(object, "resize([%f, %f])\n%s", x, y,object->content);
}

void cad_extrude(CAD_Object* object, double h) {
    object_modify_printf(object, "linear_extrude(height=%f)\n%s", h, object->content);
}

// Utils


CAD_Object* cad_object_copy(CAD_Object* object) {
    CAD_Object* new_object = (CAD_Object*)malloc(sizeof(CAD_Object));
    new_object->content = str_copy(object->content, strlen(object->content));
    return new_object;
}

CAD_Object* cad_reference_point(double x, double y, double z) {
    CAD_Object* s = cad_sphere(cad_point_size);
    cad_translate(s, x, y, z);
    return s;
}

