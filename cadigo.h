#include <float.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <math.h>

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

CAD_Array cad_array_new_empty_like(CAD_Array array) {
    CAD_Array new_array = {.count=array.count, .element_size=array.element_size};
    new_array.values = malloc(new_array.count * new_array.element_size);
    return new_array;
}

Vec3 vec3_array_max(CAD_Array array) {
    Vec3 ret = {.x = -DBL_MAX, .y = -DBL_MAX, .z = -DBL_MAX}; 
    for (int i = 0; i < (int)array.count; ++i) {
        Vec3 val = ((Vec3*)array.values)[i];
        if (val.x > ret.x) ret.x = val.x;
        if (val.y > ret.y) ret.y = val.y;
        if (val.z > ret.z) ret.z = val.z;
    }
    return ret;
}

Vec3 vec3_array_min(CAD_Array array) {
    Vec3 ret = {.x = DBL_MAX, .y = DBL_MAX, .z = DBL_MAX}; 
    for (int i = 0; i < (int)array.count; ++i) {
        Vec3 val = ((Vec3*)array.values)[i];
        if (val.x < ret.x) ret.x = val.x;
        if (val.y < ret.y) ret.y = val.y;
        if (val.z < ret.z) ret.z = val.z;
    }
    return ret;
}

Vec2 line_intersection_2d(Vec2 line1_p1, Vec2 line1_p2, Vec2 line2_p1, Vec2 line2_p2) {
    double a1 = line1_p2.y - line1_p1.y;
    double b1 = line1_p1.x - line1_p2.x;
    double c1 = a1*(line1_p1.x) + b1*(line1_p1.y);

    double a2 = line2_p2.y - line2_p1.y;
    double b2 = line2_p1.x - line2_p2.x;
    double c2 = a2*(line2_p1.x)+ b2*(line2_p1.y);

    double determinant = a1*b2 - a2*b1;
    if (determinant == 0) {
        Vec2 ret = {.x=FLT_MAX, .y=FLT_MAX};
        return ret;
    }
    else {
        double x = (b2*c1 - b1*c2)/determinant;
        double y = (a1*c2 - a2*c1)/determinant;
        Vec2 ret = {.x=x, .y=y};
        return ret;
    }
}

// CAD_Array vec3_array_subdivide(CAD_Array array) {
//     CAD_Array ret;
//     Vec3* vals = (Vec3*)array.values;
//         /*
//         I wanna smooth this
//             |
//             v
//        p2 ·---· p3
//          /     \
//      p1 ·       · p4
//
//         So I take the average between this:
//
//           ·-x-·
//          /     \
//         ·       ·
//
//         And this:
//             x
//            / \
//           ·   ·
//          /     \
//         ·       ·
//
//         and I end up with something like this:
//             x
//           ·   ·
//          /     \
//         ·       ·
//
//         */
//
//     Vec3 p1, p2, p3, p4, avg1, avg2, new_p;
//     for (int i = 0; i < array.count; ++i) {
//         p1 = vals[(i+0)%array.count];
//         p2 = vals[(i+1)%array.count];
//         p3 = vals[(i+2)%array.count];
//         p4 = vals[(i+3)%array.count];
//
//         avg1 = vec3_average(p2, p3);
//
//     }
//     return ret;
// }

Vec2 vec2_average(Vec2 a, Vec2 b) {
    Vec2 ret = {.x=(a.x+b.x) / 2, .y=(a.y+b.y) / 2};
    return ret;
}

CAD_Array vec2_array_naive_subdivide(CAD_Array array) {
    CAD_Array ret = {
        .count = array.count*2,
        .element_size = array.element_size,
        .values = malloc(array.element_size * array.count*2)
    };
    Vec2* vals = (Vec2*)array.values;
    Vec2 p1, p2, p3, p4, avg, line_inter, new_p;
    for (int i = 0; i < (int)array.count; ++i) {
        int i2 = (i+1)%(int)array.count;
        p1 = vals[(i+0)%array.count];
        p2 = vals[(i+1)%array.count];
        p3 = vals[(i+2)%array.count];
        p4 = vals[(i+3)%array.count];

        avg = vec2_average(p2, p3);
        line_inter = line_intersection_2d(p1, p2, p3, p4);
        if (line_inter.x == FLT_MAX || line_inter.y == FLT_MAX) {
            line_inter = avg;
        }
        new_p = vec2_average(avg, line_inter);
        ((Vec2*)ret.values)[(i2*2+0)%ret.count] = p2;
        ((Vec2*)ret.values)[(i2*2+1)%ret.count] = new_p;
    }
    return ret;
}

Vec2 vec2_diff(Vec2 a, Vec2 b) {
    Vec2 ret = {a.x - b.x, a.y - b.y};
    return ret;
}

Vec2 vec2_add(Vec2 a, Vec2 b) {
    Vec2 ret = {a.x + b.x, a.y + b.y};
    return ret;
}

Vec2 vec2_from_angle(double angle) {
    Vec2 result;
    result.x = cos(angle);
    result.y = sin(angle);
    return result;
}

double vec2_to_angle(Vec2 vec) {
    return atan2(vec.y, vec.x);
}

double angle_diff(double x, double y) {
    double x1, y1;
    x1 = x + M_PI; y1 = y + M_PI;
    return fmin(fabs(x1 - y1), fabs(M_PI - fabs(x1 - y1)));
}

double vec2_len(Vec2 vec) {
    return sqrt(vec.x * vec.x + vec.y * vec.y);
}

Vec2 vec2_scale(Vec2 vec, double factor) {
    return vec2(vec.x * factor, vec.y * factor);
}

double vec2_dist(Vec2 a, Vec2 b) {
    return vec2_len(vec2_diff(a, b));
}

// --- Swizzling ---
#define xx(vec) (vec2(vec.x, vec.x))
#define xy(vec) (vec2(vec.x, vec.y))
#define yx(vec) (vec2(vec.y, vec.x))
#define yy(vec) (vec2(vec.y, vec.y))

// vec3 only
#define xz(vec) (vec2(vec.x, vec.z))
#define yz(vec) (vec2(vec.y, vec.z))
#define zx(vec) (vec2(vec.z, vec.x))
#define zy(vec) (vec2(vec.z, vec.y))
#define zz(vec) (vec2(vec.z, vec.z))

#define DEFINE__VEC2__ARRAY__SWIZZLE(name, swizzle)                     \
CAD_Array name(CAD_Array array) {                                       \
    CAD_Array ret = {                                                   \
        .count = array.count,                                           \
        .element_size = sizeof(Vec2),                                   \
        .values = malloc(array.count * sizeof(Vec2))                    \
    };                                                                  \
                                                                        \
    if (array.element_size == sizeof(Vec2)) {                           \
        for (size_t i = 0; i < array.count; ++i) {                      \
            ((Vec2*)ret.values)[i] = swizzle(((Vec2*)array.values)[i]); \
        }                                                               \
                                                                        \
    } else if (array.element_size == sizeof(Vec3)) {                    \
        for (size_t i = 0; i < array.count; ++i) {                      \
            ((Vec2*)ret.values)[i] = swizzle(((Vec3*)array.values)[i]); \
        }                                                               \
                                                                        \
    } else {                                                            \
        perror("Invalid type for array");                               \
    }                                                                   \
    return ret; \
}

#define DEFINE__VEC3__ARRAY__SWIZZLE(name, swizzle)                     \
CAD_Array name(CAD_Array array) {                                       \
    CAD_Array ret = {                                                   \
        .count = array.count,                                           \
        .element_size = sizeof(Vec2),                                   \
        .values = malloc(array.count * sizeof(Vec2))                    \
    };                                                                  \
    if (array.element_size == sizeof(Vec3)) {                          \
        for (size_t i = 0; i < array.count; ++i) {                      \
            ((Vec2*)ret.values)[i] = swizzle(((Vec3*)array.values)[i]); \
        }                                                               \
    } else {                                                            \
        perror("Invalid type for array");                               \
    }                                                                   \
    return ret; \
}

DEFINE__VEC2__ARRAY__SWIZZLE(array_xx, xx);
DEFINE__VEC2__ARRAY__SWIZZLE(array_xy, xy);
DEFINE__VEC2__ARRAY__SWIZZLE(array_yx, yx);
DEFINE__VEC2__ARRAY__SWIZZLE(array_yy, yy);

DEFINE__VEC3__ARRAY__SWIZZLE(array_xz, xz);
DEFINE__VEC3__ARRAY__SWIZZLE(array_yz, yz);
DEFINE__VEC3__ARRAY__SWIZZLE(array_zx, zx);
DEFINE__VEC3__ARRAY__SWIZZLE(array_zy, zy);
DEFINE__VEC3__ARRAY__SWIZZLE(array_zz, zz);

double hypersqrt(double x) {
    return sqrt(sqrt(sqrt(sqrt(x))));
}

CAD_Array vec2_array_blob_subdivide(CAD_Array array) {
    double max_dist_factor = 0.5;
    CAD_Array ret = {
        .count = array.count*2,
        .element_size = array.element_size,
        .values = malloc(array.element_size * array.count*2)
    };
    Vec2* vals = (Vec2*)array.values;
    Vec2 p1, p2, p3, p4, perp, avg, new_p;
    for (int i = 0; i < (int)array.count; ++i) {
        int i2 = (i+1)%(int)array.count;
        p1 = vals[(i+0)%array.count];
        p2 = vals[(i+1)%array.count];
        p3 = vals[(i+2)%array.count];
        p4 = vals[(i+3)%array.count];

        avg = vec2_average(p2, p3);

        Vec2 diff = vec2_diff(p2, p3);
        double angle = vec2_to_angle(diff) + M_PI/2;

        double max_dist = vec2_dist(p2, p3) * max_dist_factor;

        double a1 = vec2_to_angle(vec2_diff(p1, p2));
        double a2 = vec2_to_angle(vec2_diff(p4, p3));
        printf("angle_diff: %lf\n", angle_diff(a1, a2) / (M_PI * 2));
        double a_diff = angle_diff(a1, a2);
        // double dist = (1 - a_diff / M_PI) * max_dist;
        // double dist = (1 - (a_diff * a_diff) / (M_PI * M_PI)) * max_dist;
        double dist = (1 - hypersqrt(a_diff) / hypersqrt(M_PI)) * max_dist;

        printf("dist: %lf\n", dist);

        perp = vec2_add(avg, vec2_scale(vec2_from_angle(angle), dist));
        new_p = vec2_average(avg, perp);
        ((Vec2*)ret.values)[(i2*2+0)%ret.count] = p2;
        ((Vec2*)ret.values)[(i2*2+1)%ret.count] = new_p;
    }
    return ret;
}


CAD_Array vec2_array_subdivide(CAD_Array array) {
    const double F = 1.0;
    int j0, j1;
    int n_points = (int)array.count;
    Vec2* points_input = (Vec2*)array.values;

    CAD_Array ret = {
        .count = array.count * 2,
        .element_size = array.element_size,
        .values = malloc(array.count * 2 * array.element_size)
    };
    Vec2 p1, p2, q1, q2;
    int k = 0;
    for (int j=0; j < n_points; ++j) {
        j0 = (j+0 + n_points) % n_points; // circular form
        j1 = (j+1 + n_points) % n_points;

        p1.x = points_input[j0].x;
        p1.y = points_input[j0].y;

        p2.x = points_input[j1].x;
        p2.y = points_input[j1].y;

        double dx = p2.x - p1.x;
        double dy = p2.y - p1.y;

        double radiX = dx / 2.82843; // 2 * 2^0.5 = 2.82843
        double radiY = dy / 2.82843; //

        double ox = (p1.x + p2.x) / 2.0;
        double oy = (p1.y + p2.y) / 2.0;

        q1.x = ox - F * radiX / 1.41421; // sqrt(2) = 1.41421
        q1.y = oy - F * radiY / 1.41421;
        ((Vec2*)ret.values)[k] = q1; k = k + 1;

        q2.x = ox + F * radiX / 1.41421;
        q2.y = oy + F * radiY / 1.41421;
        ((Vec2*)ret.values)[k] = q2; 

        k = k + 1;
    }
    return ret;
}

CAD_Array vec3_array_scale(Vec3 scale, CAD_Array array) {
    CAD_Array ret = {
        .count = array.count,
        .element_size = array.element_size,
        .values = malloc(array.element_size * array.count)
    };

    Vec3* vals = (Vec3*)(array.values);
    for (size_t i = 0; i < ret.count; ++i) {
        ((Vec3*)ret.values)[i] = vec3(
            (vals[i].x * scale.x),
            (vals[i].y * scale.y),
            (vals[i].z * scale.z)
        );
    }

    return ret;
}

double average(double x, double y) {
    return (x + y) / 2;
}

CAD_Array vec3_array_subdivide(CAD_Array array) {
    CAD_Array ret = {
        .count = array.count * 2,
        .element_size = array.element_size,
        .values = malloc(array.element_size * array.count * 2)
    };

    Vec2* smooth_xy_array = (Vec2*)vec2_array_subdivide(array_xy(array)).values;
    Vec2* smooth_yz_array = (Vec2*)vec2_array_subdivide(array_yz(array)).values;
    Vec2* smooth_xz_array = (Vec2*)vec2_array_subdivide(array_xz(array)).values;
    
    for (size_t i = 0; i < ret.count; ++i) {
        ((Vec3*)ret.values)[i] = vec3(
            average(smooth_xy_array[i].x, smooth_xz_array[i].x),
            average(smooth_xy_array[i].y, smooth_yz_array[i].x),
            average(smooth_yz_array[i].y, smooth_xz_array[i].y)
        );
    }
    return ret;
}


// CAD_Array array_xy(CAD_Array array) {
//     CAD_Array ret = {
//         .count = array.count,
//         .element_size = sizeof(Vec2),
//         .values = malloc(array.count * sizeof(Vec2))
//     };
//
//     if (array.element_size == sizeof(Vec2)) {
//         for (size_t i = 0; i < array.count; ++i) {
//             ((Vec2*)ret.values)[i] = xy(((Vec2*)array.values)[i]);
//         }
//
//     } else if (array.element_size == sizeof(Vec3)) {
//         for (size_t i = 0; i < array.count; ++i) {
//             ((Vec2*)ret.values)[i] = xy(((Vec3*)array.values)[i]);
//         }
//
//     } else {
//         perror("Invalid type for array");
//     }
// }



double cad_array_max(CAD_Array array) {
    double ret = -DBL_MAX; 
    double* vals = (double*)array.values;
    for (int i = 0; i < (int)array.count; ++i) {
        if (vals[i] > ret) {
            ret = vals[i]; 
        }
    }
    return ret;
}

double cad_array_min(CAD_Array array) {
    double ret = -DBL_MAX; 
    double* vals = (double*)array.values;
    for (int i = 0; i < (int)array.count; ++i) {
        if (vals[i] < ret) {
            ret = vals[i]; 
        }
    }
    return ret;
}


#define vectors3(...)  \
    vector3__array(sizeof((Vec3[]){__VA_ARGS__}), (Vec3[]){__VA_ARGS__}) 

CAD_Array vector3__array(size_t size, Vec3* ptr){
    void* array = malloc(size);
    memcpy(array, ptr, size);

    return (CAD_Array) {
        .count = size / sizeof(Vec3),
        .element_size = sizeof(Vec3),
        .values = array
    };
}


#define vectors2(...) ({                       \
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

int cad_program_save(char* name, CAD_Object* program, double fs) {
    FILE *program_file;

    program_file = fopen(name, "w");
    if (program_file == NULL) {
        perror("Could not open file.");
        return 1;
    }
    fprintf(program_file, "$fs=%lf;\n%s\n", fs, program->content);
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
    for (size_t i = 0; i < points.count; ++i){
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

CAD_Object* cad_square(double x, double y) {
    return object_printf("square([%lf, %lf]);", x, y);
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

void cad_add(CAD_Object* o1, CAD_Object* o2) {
    o1->content = cad_union(o1, o2)->content;
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
    return object_printf("difference(){\n%s\n%s\n}\n", o1->content, o2->content);
}

CAD_Object* cad_intersection(CAD_Object* o1, CAD_Object* o2) {
    return object_printf("intersection(){\n%s\n%s\n}\n", o1->content, o2->content);
}

CAD_Object* cad_minkowski(CAD_Object* o1, CAD_Object* o2) {
    return object_printf("minkowski(){\n%s\n%s\n}\n", o1->content, o2->content);
}

CAD_Object* cad_mirror(int x, int y, int z, CAD_Object* o1) {
    return object_printf("mirror([%d, %d, %d])\n%s", x, y, z, o1->content);
}

// Modifying Operations
CAD_Object* cad_translate(double x, double y, double z, CAD_Object* object) {
    object_modify_printf(object, "translate([%f, %f, %f])\n%s", x, y, z, object->content);
    return object;
}

CAD_Object* cad_resize(double x, double y, double z, CAD_Object* object) {
    object_modify_printf(object, "resize([%f, %f, %f])\n%s", x, y, z, object->content);
    return object;
}

CAD_Object* cad_resize_v(Vec3 v, CAD_Object* object) {
    object_modify_printf(object, "resize([%f, %f, %f])\n%s", v.x, v.y, v.z, object->content);
    return object;
}

CAD_Object* cad_scale(double x, double y, double z, CAD_Object* object) {
    object_modify_printf(object, "scale([%f, %f, %f])\n%s", x, y, z, object->content);
    return object;
}

CAD_Object* cad_scale_centered(CAD_Object* object, double x, double y, double z) {
    object_modify_printf(object, "scale([%f, %f, %f])\n%s", x, y, z, object->content);
    return object;
}

CAD_Object* cad_rotate(double x, double y, double z, CAD_Object* object) {
    object_modify_printf(object, "rotate([%f, %f, %f])\n%s", x, y, z, object->content);
    return object;
}

CAD_Object* cad_resize2D(double x, double y, CAD_Object* object) {
    object_modify_printf(object, "resize([%f, %f])\n%s", x, y,object->content);
    return object;
}

CAD_Object* cad_extrude(double h, CAD_Object* object) {
    object_modify_printf(object, "linear_extrude(height=%f)\n%s", h, object->content);
    return object;
}

// Utils

Vec3 vec3_array_bounds_size(CAD_Array array) {
    Vec3 max_point = vec3_array_max(array);
    Vec3 min_point = vec3_array_min(array);
    return vec3(
        max_point.x - min_point.x,
        max_point.y - min_point.y,
        max_point.z - min_point.z
    );
}

Vec3 vec3_div(Vec3 a, Vec3 b) {
    return vec3(a.x / b.x, a.y / b.y, a.z / b.z);
}

Vec3 vec3_sub(Vec3 a, Vec3 b) {
    return vec3(a.x - b.x, a.y - b.y, a.z - b.z);
}

CAD_Array vec3_array_center(CAD_Array array) {
    Vec3 offset = vec3_div(vec3_array_bounds_size(array), vec3(2, 2, 2));
    return cad_array_map(array, Vec3, vec3_sub, offset);
}

CAD_Object* cad_object_copy(CAD_Object* object) {
    CAD_Object* new_object = (CAD_Object*)malloc(sizeof(CAD_Object));
    new_object->content = str_copy(object->content, strlen(object->content));
    return new_object;
}

CAD_Object* cad_reference_point(double x, double y, double z) {
    return cad_translate(x, y, z, cad_sphere(cad_point_size));
}

