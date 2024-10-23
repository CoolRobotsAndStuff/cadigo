#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

typedef long double val;
#define VAL_SIZE_BITS (sizeof(val)*8)
#define VAL_FMT "%LF"


val power(val base, val exponent) {
    val result = 1;
    for (val i = 0; i < exponent; i++) {
        result *= base;
    }
    return result;
}

#define MILIMETER 1.0L

#define km  * (MILIMETER * 1000000)
#define hm  * (MILIMETER * 100000)
#define dam * (MILIMETER * 10000)
#define m   * (MILIMETER * 1000)
#define dm  * (MILIMETER * 100)
#define cm  * (MILIMETER * 10)
#define mm  * (MILIMETER * 1)
#define um  * (MILIMETER / 1000)
#define mic * (MILIMETER / 1000)
#define nm  * (MILIMETER / 1000000)
#define pm  * (MILIMETER / 1000000000)


val static inline as_km (val v) { return v / (MILIMETER * 1000000); }
val static inline as_hm (val v) { return v / (MILIMETER * 100000); }
val static inline as_dam(val v) { return v / (MILIMETER * 10000); }
val static inline as_m  (val v) { return v / (MILIMETER * 1000); }
val static inline as_dm (val v) { return v / (MILIMETER * 100); }
val static inline as_cm (val v) { return v / (MILIMETER * 10); }
val static inline as_mm (val v) { return v / (MILIMETER * 1); }
val static inline as_um (val v) { return v / (MILIMETER / 1000); }
val static inline as_mic(val v) { return v / (MILIMETER / 1000); }
val static inline as_nm (val v) { return v / (MILIMETER / 1000000); }
val static inline as_pm (val v) { return v / (MILIMETER / 1000000000); }


typedef struct {
    val x;
    val y;
} Vec2;

Vec2 v2(val x, val y) { return (Vec2){x, y}; }

typedef struct {
    val x;
    val y;
    val z;
} Vec3;

Vec3 v3(val x, val y, val z) { return (Vec3){x, y, z}; }

Vec3 vec3_add(Vec3 a, Vec3 b) {
    Vec3 ret = {a.x + b.x, a.y + b.y, a.z + b.z};
    return ret;
}
Vec3 vec3_substract(Vec3 a, Vec3 b) {
    return v3(a.x - b.x, a.y - b.y, a.z - b.z);
}

Vec3 vec3_rotate_y(val rotation, Vec3 p) {
    double angle = atan2(p.z, p.x) + rotation;
    double mag = sqrt(p.x*p.x + p.z*p.z);
    return v3(cos(angle)*mag, p.y, sin(angle)*mag);
}

Vec3 vec3_rotate_x(val rotation, Vec3 p) {
    double angle = atan2(p.z, p.y) + rotation;
    double mag = sqrt(p.y*p.y + p.z*p.z);
    return v3(p.x, cos(angle)*mag, sin(angle)*mag);
}

Vec3 vec3_rotate_z(val rotation, Vec3 p) {
    double angle = atan2(p.z, p.x) + rotation;
    double mag = sqrt(p.x*p.x + p.z*p.z);
    return v3(cos(angle)*mag, p.y, sin(angle)*mag);
}

typedef struct {
    size_t count;
    Vec2* items;
} Vec2Array;

#define vecs2(...) \
    new__vec2__array(sizeof((Vec2[]){__VA_ARGS__}), (Vec2[]){__VA_ARGS__}) 

Vec2Array new__vec2__array(size_t size, Vec3* ptr){
    return (Vec2Array) {
        .count = size / sizeof(Vec2),
        .items = (Vec2*)memcpy(malloc(size), ptr, size)
    };
}

typedef struct {
    size_t count;
    Vec3* items;
} Vec3Array;

#define vecs3(...) \
    new__vec3__array(sizeof((Vec3[]){__VA_ARGS__}), (Vec3[]){__VA_ARGS__})

Vec3Array new__vec3__array(size_t size, Vec3* ptr){
    return (Vec3Array) {
        .count = size / sizeof(Vec3),
        .items = (Vec3*)memcpy(malloc(size), ptr, size)
    };
}


typedef struct {
    size_t count;
    val* items;
} ValueArray;

#define vals(...) \
    new__value_array(sizeof((val[]){__VA_ARGS__}), (val[]){__VA_ARGS__}) 

ValueArray new__value_array(size_t size, val* ptr){
    return (ValueArray) {
        .count = size / sizeof(val),
        .items = (val*)memcpy(malloc(size), ptr, size)
    };
}


typedef struct {
    size_t count;
    size_t* items;
} Face;

#define face(...) \
    new__face(sizeof((size_t[]){__VA_ARGS__}), (size_t[]){__VA_ARGS__}) 

Face new__face(size_t size, size_t* ptr){
    return (Face) {
        .count = size / sizeof(size_t),
        .items = (size_t*)memcpy(malloc(size), ptr, size)
    };
}

typedef struct {
    size_t count;
    Face* items;
} Faces;

#define faces(...) \
    new__faces(sizeof((Face[]){__VA_ARGS__}), (Face[]){__VA_ARGS__}) 

Faces new__faces(size_t size, Face* ptr){
    return (Faces) {
        .count = size / sizeof(Face),
        .items = (Face*)memcpy(malloc(size), ptr, size)
    };
}

// --- Swizzling ---
// -- Simple (useful for macros like map)
#define x(vec) (vec.x)
#define y(vec) (vec.y)
#define z(vec) (vec.z)

// -- Double 
#define xx(vec) (vec2(vec.x, vec.x))
#define xy(vec) (vec2(vec.x, vec.y))
#define yx(vec) (vec2(vec.y, vec.x))
#define yy(vec) (vec2(vec.y, vec.y))
#define xz(vec) (vec2(vec.x, vec.z))
#define yz(vec) (vec2(vec.y, vec.z))
#define zx(vec) (vec2(vec.z, vec.x))
#define zy(vec) (vec2(vec.z, vec.y))
#define zz(vec) (vec2(vec.z, vec.z))

// -- Triple
#define xxx(vec) (vec3(vec.x, vec.x, vec.x))
#define xxy(vec) (vec3(vec.x, vec.x, vec.y))
#define xxz(vec) (vec3(vec.x, vec.x, vec.z))
#define xyx(vec) (vec3(vec.x, vec.y, vec.x))
#define xyy(vec) (vec3(vec.x, vec.y, vec.y))
#define xyz(vec) (vec3(vec.x, vec.y, vec.z))
#define xzx(vec) (vec3(vec.x, vec.z, vec.x))
#define xzy(vec) (vec3(vec.x, vec.z, vec.y))
#define xzz(vec) (vec3(vec.x, vec.z, vec.z))
#define yxx(vec) (vec3(vec.y, vec.x, vec.x))
#define yxy(vec) (vec3(vec.y, vec.x, vec.y))
#define yxz(vec) (vec3(vec.y, vec.x, vec.z))
#define yyx(vec) (vec3(vec.y, vec.y, vec.x))
#define yyy(vec) (vec3(vec.y, vec.y, vec.y))
#define yyz(vec) (vec3(vec.y, vec.y, vec.z))
#define yzx(vec) (vec3(vec.y, vec.z, vec.x))
#define yzy(vec) (vec3(vec.y, vec.z, vec.y))
#define yzz(vec) (vec3(vec.y, vec.z, vec.z))
#define zxx(vec) (vec3(vec.z, vec.x, vec.x))
#define zxy(vec) (vec3(vec.z, vec.x, vec.y))
#define zxz(vec) (vec3(vec.z, vec.x, vec.z))
#define zyx(vec) (vec3(vec.z, vec.y, vec.x))
#define zyy(vec) (vec3(vec.z, vec.y, vec.y))
#define zyz(vec) (vec3(vec.z, vec.y, vec.z))
#define zzx(vec) (vec3(vec.z, vec.z, vec.x))
#define zzy(vec) (vec3(vec.z, vec.z, vec.y))
#define zzz(vec) (vec3(vec.z, vec.z, vec.z))


typedef struct {
    Vec3Array points;
    Faces faces;  
} Object3D;


Object3D obj_cube(val size) {
    val a = size/2;
    Object3D c = {
        .points = vecs3(
            v3(-a, -a, -a),
            v3(-a, -a,  a),
            v3(-a,  a,  a),
            v3(-a,  a, -a),

            v3( a, -a, -a),
            v3( a, -a,  a),
            v3( a,  a,  a),
            v3( a,  a, -a),
        ),
        .faces = faces(
            // Each pair are opposites
            face(0, 1, 2, 3),
            face(4, 5, 6, 7),

            face(0, 1, 5, 4),
            face(2, 3, 7, 6),

            face(1, 2, 6, 5),
            face(4, 0, 3, 7)
        )
    };
    return c;
}


size_t add_vertex_to(Object3D* obj, Vec3 point) {
    Vec3* new_points = (Vec3*)malloc(sizeof(Vec3) * (obj->points.count + 1));
    memcpy(new_points, obj->points.items, sizeof(Vec3) * obj->points.count);
    free(obj->points.items);

    obj->points.items = new_points;
    obj->points.count += 1;
    obj->points.items[obj->points.count - 1] = point;
    return obj->points.count - 1; 
}



