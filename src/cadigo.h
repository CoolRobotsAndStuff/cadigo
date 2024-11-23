#ifndef CADIGO_H_
#define CADIGO_H_

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <unistd.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288419716939937510582097494459230781640628620899862803482534211706798214808651328230664709384460955058223172535940812848111745028410270193852110555964462294895493038196442881097566593344612847564823378678316527120190914564856692346034861045432664821339360726024914127372458700660631558817488152092096282925409171536436789259036001133053054882046652138414695194151160943305727036575959195309218611738193261179310511854807446237996274956735188575272489122793818301194912983367336244065664308602139494639522473719070217986094370277053921717629317675238467481846766940513200056812714526356082778577134275778960917363717872146844090122495343014654958537105079227968925892354201995611212902196086403441815981362977477130996051870721134999999837297804995105973173281609631859502445945534690830264252230825334468503526193118817101000313783875288658753320838142061717766914730359825349042875546873115956286388235378759375195778185778053217122680661300192787661119590921642019893809525720106548586327886593615338182796823030195203530185296899577362259941389124972177528347913151557485724245415069595082953311686172785588907509838175463746493931925506040092770167113900984882401285836160356370766010471018194295559619894676783744944825537977472684710404753464620804668425906949129331367702898915210475216205696602405803815019351125338243003558764024749647326391419927260426992279678235478163600934172164121992458631503028618297455570674983850549458858692699569092721079750930295532116534498720275596023648066549911988183479775356636980742654252786255181841757467289097777279380008164706001614524919217321721477235014144197356854816136115735255213347574184946843852332390739414333454776241686251898356948556209921922218427255025425688767179049460165346680498862723279178608578438382796797668145410095388378636095068006422512520511739298489608412848862694560424196528502221066118630674427862203919494504712371378696095636437191728746776465757396241389086583264599581339047802759009
#endif

#ifndef EPS
#define EPS 0.000000000000000000000000000000000000001
#endif

#define NOT_IMPLEMENTED do { fprintf(stderr, "%s:%d: NOT_IMPLEMENTED\n", __FILE__, __LINE__); abort(); } while(0)
#define UNUSED(variable) (void) variable


typedef long double val_t;
typedef val_t ang_t;

typedef struct {
    union { val_t x; ang_t roll;  val_t first; };
    union { val_t y; ang_t pitch; val_t second;};
    union { val_t z; ang_t yaw;   val_t third;};
    int mark; // some algorithms are easier if you can just mark certain points;
} Vec3;

Vec3 vec3(val_t x, val_t y, val_t z);

Vec3 vec3_add_s (Vec3 v1, val_t scalar);
Vec3 vec3_sub_s (Vec3 v1, val_t scalar);
Vec3 vec3_div_s (Vec3 v1, val_t scalar);
Vec3 vec3_mult_s(Vec3 v1, val_t scalar);

Vec3 vec3_add (Vec3 v1, Vec3 v2);
Vec3 vec3_sub (Vec3 v1, Vec3 v2);
Vec3 vec3_div (Vec3 v1, Vec3 v2);
Vec3 vec3_mult(Vec3 v1, Vec3 v2);
Vec3 vec3_avg (Vec3 v1, Vec3 v2);


typedef struct {
    val_t mag;
    ang_t ang;
} Polar2D;

typedef struct {
    size_t count;
    size_t capacity;
    size_t* items;
} Indexes;

typedef struct {
    size_t count;
    size_t capacity;
    size_t* items;
} Face;

typedef struct {
    size_t count;
    size_t capacity;
    Face* items;
} Faces;

typedef struct {
    size_t count;
    size_t capacity;
    Vec3* items;
} Points;

Face __face(size_t count, size_t* indexes);
Faces __faces(size_t count, Face* faces);
Points __points(size_t count, Vec3* points);

#define face(...)   __face  (sizeof((size_t[]){__VA_ARGS__}) / sizeof(size_t), (size_t[]){__VA_ARGS__})
#define faces(...)  __faces (sizeof((Face[])  {__VA_ARGS__}) / sizeof(Face)  , (Face[])  {__VA_ARGS__})
#define points(...) __points(sizeof((Vec3[])  {__VA_ARGS__}) / sizeof(Vec3)  , (Vec3[])  {__VA_ARGS__})


void free_face(Face f);
void free_faces(Faces fs);
void free_points(Points p);


typedef struct {
    Points points;
    Faces faces;
} CAD;

void cad_free(CAD obj);
CAD cad_clone(CAD obj);

CAD cad_polyhedron(Points points, Faces faces);
CAD cad_polygon(Points points);
CAD cad_line(Points points);

CAD __cad_polygon_from_points(size_t count, Vec3* points);
#define cad_polygon_from_points(...) __cad_polygon_from_points(sizeof((Vec3[])  {__VA_ARGS__}) / sizeof(Vec3)  , (Vec3[])  {__VA_ARGS__})

CAD cad_cube(val_t l);

// Operations - Similar Geometry
CAD* cad_translate(Vec3 v, CAD* obj);
CAD* cad_rotate(Vec3 v, CAD* obj);
CAD* cad_scale(Vec3 v, CAD* obj);

// Operations - Topological Geometry
CAD* cad_catmull_clark3D(CAD* obj);

CAD* cad_extrude(double h, CAD* obj);

// Operations - Booleans
CAD cad_intersection(CAD obj);
CAD cad_difference(CAD obj);
CAD cad_union(CAD obj);


typedef struct {
    Vec3 min;
    Vec3 max;
} Bounds;

Bounds cad_get_bounds(CAD obj);

#endif // CADIGO_H_



// ------ Implementation -------




#ifdef CADIGO_IMPLEMENTATION

Vec3 vec3(val_t x, val_t y, val_t z) {
    Vec3 v;
    v.x = x;
    v.y = y;
    v.z = z;
    v.mark = 0;
    return v;
}

Vec3 vec3_add_s(Vec3 v1, val_t scalar) {
    return vec3(v1.x + scalar, v1.y + scalar, v1.z + scalar);
}

Vec3 vec3_sub_s(Vec3 v1, val_t scalar) {
    return vec3(v1.x - scalar, v1.y - scalar, v1.z - scalar);
}

Vec3 vec3_div_s(Vec3 v1, val_t scalar) {
    return vec3(v1.x / scalar, v1.y / scalar, v1.z / scalar);
}

Vec3 vec3_mult_s(Vec3 v1, val_t scalar) {
    return vec3(v1.x * scalar, v1.y * scalar, v1.z * scalar);
}

Vec3 vec3_add(Vec3 v1, Vec3 v2) {
    return vec3(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
}

Vec3 vec3_sub(Vec3 v1, Vec3 v2) {
    return vec3(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
}

Vec3 vec3_div (Vec3 v1, Vec3 v2) {
    return vec3(v1.x / v2.x, v1.y / v2.y, v1.z / v2.z);
}
Vec3 vec3_mult(Vec3 v1, Vec3 v2) {
    return vec3(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z);
}

Vec3 vec3_avg(Vec3 v1, Vec3 v2) {
    return vec3_div_s(vec3_add(v1, v2), 2);
}

Face __face(size_t count, size_t* indexes) {
    Face ret = {
        .count = count, 
        .capacity = count,
        .items = (size_t*)malloc(sizeof(size_t) * count)
    };

    memcpy(ret.items, indexes, sizeof(size_t) * count);
    return ret;
}

Faces __faces(size_t count, Face* faces) {
    Faces ret = {
        .count = count, 
        .capacity = count,
        .items = (Face*)malloc(sizeof(Face) * count)
    };

    memcpy(ret.items, faces, sizeof(Face) * count);
    return ret;
}

Points __points(size_t count, Vec3* points) {
    Points ret = {
        .count = count, 
        .capacity = count,
        .items = (Vec3*)malloc(sizeof(Vec3) * count)
    };

    memcpy(ret.items, points, sizeof(Vec3) * count);
    return ret;
}

CAD __cad_polygon_from_points(size_t count, Vec3* points) {
    return cad_polygon(__points(count, points));
}

void free_face(Face f) {
    free(f.items);
    f.count = 0;
    f.capacity = 0;
}

void free_faces(Faces fs) {
    for (size_t i = 0; i < fs.count; ++i)
        free_face(fs.items[i]);
    fs.count = 0;
    fs.capacity = 0;
}

void free_points(Points p) {
    free(p.items);
    p.count = 0;
    p.capacity = 0;
}

void cad_free(CAD obj) {
    free_points(obj.points);
    free_faces(obj.faces);
}

#define da_size(array) ((array.capacity) > 0 ? sizeof(array.items[0]) * (array.capacity) : 0)
#define da_last(array) (array.items[(array.count-1)])
#define da_last_index(array) ((array).count-1)

#ifndef DA_INIT_CAP 
#define DA_INIT_CAP 10
#endif

#define da_append(da, item)                                                          \
do {                                                                                 \
    if ((da)->count >= (da)->capacity) {                                             \
        (da)->capacity = (da)->capacity == 0 ? DA_INIT_CAP : (da)->capacity*2;       \
        (da)->items = realloc((da)->items, (da)->capacity*sizeof(*(da)->items));     \
    }                                                                                \
    (da)->items[(da)->count++] = (item);                                             \
} while (0)

#define da_insert(da, index, item)                                                   \
do {                                                                                 \
    size_t __index = (index);                                                        \
    if ((da)->count >= (da)->capacity) {                                             \
        (da)->capacity = (da)->capacity == 0 ? DA_INIT_CAP : (da)->capacity*2;       \
        (da)->items = realloc((da)->items, (da)->capacity*sizeof(*(da)->items));     \
    }                                                                                \
    for (size_t __i = (da)->count; __i > __index; --__i) {                           \
        (da)->items[__i] = (da)->items[__i-1];                                       \
    }                                                                                \
    (da)->count += 1;                                                                \
    (da)->items[__index] = (item);                                                   \
} while (0)

#define da_delete(da, index)                                    \
do {                                                            \
    size_t __index = (index);                                   \
    for (size_t __i = __index; __i < (da)->count-1; ++__i)      \
        (da)->items[__i] = (da)->items[__i+1];                  \
    (da)->count -= 1;                                           \
} while(0);


CAD cad_polyhedron(Points points, Faces faces) {
    return (CAD){.points = points, .faces = faces};
}

CAD cad_polygon(Points points) {
    Face f;
    f.count = points.count;
    f.capacity = points.count;
    f.items = (size_t*)malloc(sizeof(size_t)*points.count);
    for (size_t i = 0; i < points.count; ++i)
         f.items[i] = i;

    Faces fs;
    fs.count = 1;
    fs.capacity = 1;
    fs.items = (Face*)malloc(sizeof(Face));
    fs.items[0] = f;

    return (CAD){.points = points, .faces = fs};
}

CAD cad_line(Points points) {
    return (CAD){.points = points, .faces = (Faces){0}};
}

CAD cad_cube(val_t l) {
    val_t a = l/2;
    CAD c = {
        .points = points(
            vec3(-a, -a, -a),
            vec3(-a, -a,  a),
            vec3(-a,  a,  a),
            vec3(-a,  a, -a),

            vec3( a, -a, -a),
            vec3( a, -a,  a),
            vec3( a,  a,  a),
            vec3( a,  a, -a),
        ),
        .faces = faces(
            // Each pair are opposites
            face(3, 2, 1, 0),
            face(4, 5, 6, 7),

            face(0, 1, 5, 4),
            face(2, 3, 7, 6),

            face(1, 2, 6, 5),
            face(7, 3, 0, 4)
        )
    };
    return c;
}

void cad_print_points(CAD obj) {
    for (size_t i = 0; i < obj.points.count; ++i) {
        printf("(%Lf, %Lf, %Lf), ", 
               obj.points.items[i].x,
               obj.points.items[i].y,
               obj.points.items[i].z
               );
    }
    printf("\n");
}

CAD* cad_translate(Vec3 v, CAD* obj) {
    for (size_t i = 0; i < obj->points.count; ++i) {
        Vec3 old = obj->points.items[i];
        obj->points.items[i] = vec3(old.x + v.x, old.y + v.y, old.z + v.z);
    }
    return obj;
}

Polar2D xy_to_polar(val_t x, val_t y) {
    Polar2D ret;
    ret.mag = sqrt((x*x) + (y*y));
    ret.ang = atan2(y, x);
    return ret;
}

Vec3 polar_to_vec2(Polar2D p) {
    Vec3 ret;
    ret.x = p.mag * cos(p.ang); // Calculate x
    ret.y = p.mag * sin(p.ang);
    ret.z = 0;
    return ret;
}

ang_t degs2rads(ang_t degrees) {
    return degrees * (M_PI / 180.0);
}

// Function to convert radians to degrees
ang_t rads2degs(ang_t radians) {
    return radians * (180.0 / M_PI);
}

Vec3 vec3_rotate_roll(ang_t roll, Vec3 v) {
    Polar2D yz_polar = xy_to_polar(v.y, v.z);
    yz_polar.ang += roll;
    Vec3 v1 = polar_to_vec2(yz_polar);
    return vec3(v.x, v1.first, v1.second);
}

Vec3 vec3_rotate_pitch(ang_t pitch, Vec3 v) {
    Polar2D xz_polar = xy_to_polar(v.x, v.z);
    xz_polar.ang += pitch;
    Vec3 v1 = polar_to_vec2(xz_polar);
    return vec3(v1.first, v.y, v1.second);
}

Vec3 vec3_rotate_yaw(ang_t yaw, Vec3 v) {
    Polar2D xy_polar = xy_to_polar(v.x, v.y);
    xy_polar.ang += yaw;
    Vec3 v1 = polar_to_vec2(xy_polar);
    return vec3(v1.first, v1.second, v.z);
}

CAD* cad_rotate(Vec3 v, CAD* obj) {
    for (size_t i = 0; i < obj->points.count; ++i) {
        obj->points.items[i] = vec3_rotate_roll (degs2rads(v.roll),  obj->points.items[i]);
        obj->points.items[i] = vec3_rotate_pitch(degs2rads(v.pitch), obj->points.items[i]);
        obj->points.items[i] = vec3_rotate_yaw  (degs2rads(v.yaw),   obj->points.items[i]);
    }
    return obj;
}

CAD* cad_scale(Vec3 v, CAD* obj) {
    for (size_t i=0; i < obj->points.count; ++i) {
        obj->points.items[i].x *= v.x;
        obj->points.items[i].y *= v.y;
        obj->points.items[i].z *= v.z;
    }
    return obj;
}

Face cad_copy_face(Face f) {
    Face ret;
    ret.count = f.count;
    ret.capacity = f.capacity;
    ret.items = (size_t*)malloc(da_size(f));
    memcpy(ret.items, f.items, da_size(f));
    return ret;
}

CAD cad_clone(CAD obj) {
    CAD ret;
    ret.points.count    = obj.points.count;
    ret.points.capacity = obj.points.capacity;
    ret.points.items    = (Vec3*)malloc(da_size(obj.points));
    memcpy(ret.points.items, obj.points.items, da_size(obj.points));

    ret.faces.count    = obj.faces.count;
    ret.faces.capacity = obj.faces.capacity;
    ret.faces.items    = (Face*)malloc(da_size(obj.faces));
    for (size_t i = 0; i < obj.faces.count; ++i)
        ret.faces.items[i] = cad_copy_face(obj.faces.items[i]);
    return ret;
}

typedef struct {
    size_t first;
    size_t second;
} IndexPair;

typedef struct {
    size_t count;
    size_t capacity;
    IndexPair* items;
} IndexPairs;

bool is_same_pair(IndexPair ip1, IndexPair ip2) {
    return ((ip1.first == ip2.first  && ip1.second == ip2.second) ||
           (ip1.first == ip2.second && ip1.second == ip2.first));
}

CAD* cad_split_edge(CAD* obj, IndexPair edge) {
    IndexPair current_edge; 
    Vec3 new_point = vec3_avg(obj->points.items[edge.first], obj->points.items[edge.second]);
    da_append(&obj->points, new_point);
    for (size_t i = 0; i < obj->faces.count; ++i) {
        Face* f = &obj->faces.items[i];
        for (size_t j = 0; j < f->count; ++j) { 
            current_edge.first  = f->items[j];
            current_edge.second = f->items[(j + 1) % f->count];
            if (is_same_pair(current_edge, edge)) {
                da_insert(f, (j + 1) % f->count, obj->points.count-1);
                break;
            }
        }
    }
    return obj;
}

IndexPairs get_all_edges(CAD obj) {
    IndexPairs edges = {
        .count = 0,
        .capacity = 1,
        .items = malloc(sizeof(IndexPair))
    };
    for (size_t i = 0; i < obj.faces.count; ++i) {
        Face f = obj.faces.items[i];
        for (size_t j = 0; j < f.count; ++j) {
            IndexPair edge = {
                .first = f.items[j],
                .second = f.items[(j + 1) % f.count]
            };

            bool edge_already_exists = false;
            for (size_t k = 0; k < edges.count; ++k) { 
                if (is_same_pair(edges.items[k], edge)) {
                    edge_already_exists = true;
                    break;
                }
            }
            if (!edge_already_exists) {
                da_append(&edges, edge);
            }
        }
    }
    return edges;
}

Indexes get_adjancent_face_indexes_to_edge(CAD obj, IndexPair edge) {
    Indexes ret = {
        .count = 0,
        .capacity = 1,
        .items = malloc(sizeof(size_t))
    };
    for (size_t i = 0; i < obj.faces.count; ++i) {
        Face f = obj.faces.items[i];
        for (size_t j = 0; j < f.count; ++j) {
            IndexPair current_edge = {
                .first = f.items[j],
                .second = f.items[(j + 1) % f.count]
            };
            if (is_same_pair(current_edge, edge)) {
                da_append(&ret, i);
                if (ret.count >= 2) return ret;
            }
        }
    }

    if (ret.count == 0) {
        fprintf(stderr, "ERROR: Couldn't find any adjacent face to the edge %zu - %zu\n", edge.first, edge.second);
        exit(1);
    }
    return ret;
}

Indexes get_face_indexes_containing_point(CAD obj, size_t point_index) {
    Indexes ret = {
        .count = 0,
        .capacity = 1,
        .items = malloc(sizeof(size_t))
    };

    for (size_t i = 0; i < obj.faces.count; ++i) {
        Face f = obj.faces.items[i];
        for (size_t j = 0; j < f.count; ++j) {
            if (f.items[j] == point_index) {
                da_append(&ret, i);
                break;
            }
        }
    }
    return ret;
}

IndexPairs get_all_edges_containing_point(CAD obj, size_t point_index) {
    IndexPairs ret = {
        .count = 0,
        .capacity = 1,
        .items = malloc(sizeof(IndexPair))
    };

    for (size_t i = 0; i < obj.faces.count; ++i) {
        Face f = obj.faces.items[i];
        for (size_t j = 0; j < f.count; ++j) {
            if (f.items[j] != point_index) continue; // check point in edge
            
            IndexPair current_edge = {
                .first = f.items[j],
                .second = f.items[(j + 1) % f.count]
            };

            // check for duplicate edges
            bool edge_already_exists = false;
            for (size_t k = 0; k < ret.count; ++k) { 
                if (is_same_pair(ret.items[k], current_edge)) {
                    edge_already_exists = true;
                    break;
                }
            }
            if (edge_already_exists) continue;
                

            da_append(&ret, current_edge);
            break;
        }
    }

    return ret;
}

void print_points (Points ps) {
    printf("--------------------------\n");
    for (size_t i = 0; i < ps.count; ++i)
        printf("        [%.10Lf, %.10Lf, %.10Lf],\n", ps.items[i].x, ps.items[i].y, ps.items[i].z);
    printf("--------------------------\n");
}

void print_point(Vec3 p) {
    printf("        [%.10Lf, %.10Lf, %.10Lf],\n", p.x, p.y, p.z);
}

void print_faces(Faces faces) {
    printf("--------------------------\n");
    for (size_t j = 0; j < faces.count; ++j) {
        Face f = faces.items[j];
        printf("[");
        for (size_t k = 0; k < f.count; ++k) {
            printf("%zu,", f.items[k]);
        } 
        printf("]\n");
    }
    printf("--------------------------\n");
}

void print_face(Face f) {
    printf("[");
    for (size_t k = 0; k < f.count; ++k)
        printf("%zu,", f.items[k]);
    printf("]\n");
}


CAD* cad_catmull_clark(CAD* obj) {
    assert(obj->faces.count > 1 && "ERROR: cad_catmull_clark is for polyhedron subdivision. Use another function for polygons.");
    const int original_point = 1;
    const int edge_point = 2;
    const int face_point = 3;
    UNUSED(face_point);

    for (size_t i = 0; i < obj->points.count; ++i)
        obj->points.items[i].mark = original_point;

    CAD old_object = cad_clone(*obj);

    // Face points
    Points face_points = {
        .count = 0,
        .capacity = obj->faces.count,
        .items = malloc(obj->faces.count * sizeof(Vec3)),
    };

    for (size_t i = 0; i < obj->faces.count; ++i) {
        Face f = obj->faces.items[i];
        Vec3 sum = vec3(0, 0, 0);
        for (size_t j = 0; j < f.count; ++j)
            sum = vec3_add(sum, obj->points.items[f.items[j]]);

        face_points.items[i] = vec3_div_s(sum, f.count);
        face_points.count += 1;
    }

    
    // Edge points
    IndexPairs original_edges = get_all_edges(*obj);

    for (size_t i = 0; i < original_edges.count; ++i) {
        Indexes adjancent_faces = get_adjancent_face_indexes_to_edge(old_object, original_edges.items[i]);

        cad_split_edge(obj, original_edges.items[i]); // edge point gets added at the end of the array
        Vec3 avg = vec3(0, 0, 0);
        int count = 0;
        avg = vec3_add(avg, obj->points.items[original_edges.items[i].first]);  count += 1;
        avg = vec3_add(avg, obj->points.items[original_edges.items[i].second]); count += 1;
        for (size_t j = 0; j < adjancent_faces.count; ++j) {
            avg = vec3_add(avg, face_points.items[adjancent_faces.items[j]]);
            count += 1;
        }
        da_last(obj->points) = vec3_div_s(avg, count);
        da_last(obj->points).mark = edge_point;
    }
    
    // Original points get moved
    for (size_t i = 0; i < obj->points.count; ++i) {
        if (obj->points.items[i].mark != original_point) continue;
        
        Indexes touching_face_indexes = get_face_indexes_containing_point((*obj), i);
    
        Vec3 face_points_avg = vec3(0, 0, 0);
        for (size_t j = 0; j < touching_face_indexes.count; ++j) {
            face_points_avg = vec3_add(face_points_avg, face_points.items[touching_face_indexes.items[j]]);
        }
        face_points_avg = vec3_div_s(face_points_avg, touching_face_indexes.count);


        IndexPairs touching_edge_indexes = get_all_edges_containing_point(old_object, i);
        Vec3 edge_points_avg = vec3(0, 0, 0);
        for (size_t j = 0; j < touching_edge_indexes.count; ++j) {
            IndexPair edge = touching_edge_indexes.items[j];
            if (edge.first == i) edge_points_avg = vec3_add(edge_points_avg, vec3_avg(old_object.points.items[edge.first], old_object.points.items[edge.second]));
        }
        edge_points_avg = vec3_div_s(edge_points_avg, touching_edge_indexes.count);

        Vec3 new_point = vec3(0, 0, 0);
        new_point = vec3_add(new_point, vec3_mult_s(face_points_avg, 1));
        new_point = vec3_add(new_point, vec3_mult_s(edge_points_avg, 2));
        new_point = vec3_add(new_point, vec3_mult_s(obj->points.items[i], touching_face_indexes.count - 3));
        new_point = vec3_div_s(new_point, touching_face_indexes.count);

        new_point.mark = original_point;
        obj->points.items[i] = new_point; 
    }

    // New object and faces
    CAD new_obj = {
        .points = obj->points,
        .faces =  (Faces){
            .count = 0,
            .capacity = 1,
            .items = malloc(sizeof(Face))

        }
    };

    for (size_t i = 0; i < obj->faces.count; ++i) {
        Face old_face = obj->faces.items[i];
        da_append(&new_obj.points, face_points.items[i]); // Face point gets appended here
    
        for (size_t j = 0; j < old_face.count; ++j) {
            if (obj->points.items[old_face.items[j]].mark != edge_point) continue;
            Face new_face = {
                .count = 0,
                .capacity = 1,
                .items = (size_t*)malloc(sizeof(size_t))
            };
            da_append(&new_face, old_face.items[j]);
            da_append(&new_face, old_face.items[(j+1)%old_face.count]);
            da_append(&new_face, old_face.items[(j+2)%old_face.count]);
            da_append(&new_face, da_last_index(new_obj.points)); // Face point gets used here
            da_append(&new_obj.faces, new_face);
        }
    }

    cad_free(old_object);
    free_faces(obj->faces);
    obj->faces = new_obj.faces;
    obj->points = new_obj.points;
    return obj;
}


Face reverse_face(Face f) {
    size_t start = 0;
    size_t end = f.count - 1;
    while (start < end) {
        size_t temp = f.items[start];
        f.items[start] = f.items[end];
        f.items[end] = temp;

        start++;
        end--;
    }
    return f;
}

// CAD_Array range(size_t beg, size_t end, size_t step) {
//     size_t count = end - beg;
//     int* array = (int*)malloc(count*sizeof(int));  
//
//     for (size_t i = 0; i < count; i += step) {
//         array[i] = (int)i + (int)beg;
//     }
//     CAD_Array ret = {.count=count, .element_size=sizeof(int), .values=array};
//     return ret;
// }

CAD* make_little_faces_around(CAD* obj) {
    Face face_1 = obj->faces.items[0];
    Face face_2 = obj->faces.items[1];

    for (size_t i=0; i < face_1.count; ++i) {
        Face new_face = {
            .count = 4,
            .capacity = 5,
            .items = malloc(5*sizeof(size_t))
        };

        new_face.items[3] = face_1.items[i];
        new_face.items[2] = face_1.items[(i + 1) % face_1.count];
        new_face.items[1] = face_1.items[(i + 1) % face_2.count] + face_1.count;
        new_face.items[0] = face_1.items[i] + face_1.count;

        da_append(&obj->faces, new_face);
    }
    return obj;
}

CAD* cad_clone_face_with_points(CAD* obj, size_t face_index) {
    Face new_face = cad_copy_face(obj->faces.items[face_index]);
    for (size_t i = 0; i < new_face.count; ++i) {
        da_append(&obj->points, obj->points.items[new_face.items[i]]);
        new_face.items[i] = obj->points.count-1;
    }
    da_append(&obj->faces, new_face);
    return obj;
}

CAD* cad_extrude(double h, CAD* obj) {
    assert(obj->faces.count == 1);
    cad_clone_face_with_points(obj, 0);
    obj->faces.items[1] = reverse_face(obj->faces.items[1]);

    //print_face(obj->faces.items[0]);
    //print_face(obj->faces.items[1]);

    for (size_t i = 0; i < obj->faces.items[1].count; ++i) {
        obj->points.items[obj->faces.items[1].items[i]] = vec3_add(vec3(0, 0, h), obj->points.items[obj->faces.items[1].items[i]]);
    } 
    make_little_faces_around(obj);
    return obj;
}

Vec3 get_face_center(CAD obj, size_t face_index) {
    Vec3 sum = vec3(0, 0, 0);
    for (size_t i = 0; i < obj.faces.items[face_index].count; ++i)
        sum = vec3_add(sum, obj.points.items[obj.faces.items[face_index].items[i]]);
    return vec3_div_s(sum, obj.faces.items[face_index].count);
}

CAD* cad_inset_face(size_t face_index, val_t amount, CAD* obj) {
    assert(amount >= 0 && amount <= 1 && "Amount must be between 0 and 1.");
    assert(face_index < obj->faces.count && "face_index outside of range.");
    cad_clone_face_with_points(obj, face_index);

    Face og_face  = obj->faces.items[face_index];
    Face new_face = obj->faces.items[obj->faces.count-1];

    Vec3 face_center = get_face_center(*obj, face_index);
    
    for (size_t i = 0; i < new_face.count; ++i) {
        obj->points.items[new_face.items[i]] = 
            vec3_add(
                vec3_mult_s(obj->points.items[new_face.items[i]], 1 - amount),
                vec3_mult_s(face_center                         , amount)
            );

        Face new_border_face;
        new_border_face.count = 4;
        new_border_face.capacity = 5;
        new_border_face.items = malloc(sizeof(size_t) * new_border_face.capacity);

        new_border_face.items[0] =  og_face.items[i];
        new_border_face.items[1] =  og_face.items[(i + 1) % og_face.count];
        new_border_face.items[2] = new_face.items[(i + 1) % new_face.count];
        new_border_face.items[3] = new_face.items[i];
        da_append(&obj->faces, new_border_face);
    }
    free_face(obj->faces.items[face_index]);
    da_delete(&obj->faces, face_index);
    return obj;
}

void vec3_print(Vec3 v) {
    printf("<%Lf, %Lf, %Lf>", v.x, v.y, v.z);
}

void vec3_println(Vec3 v) {
    printf("<%Lf, %Lf, %Lf>\n", v.x, v.y, v.z);
}

CAD* cad_hotpoints_subdivision(CAD* obj) {
    assert(obj->faces.count == 1 && "ERROR: object is not simple polygon.");
    const double F = 1.0;
    size_t j1, j2;

    Points points;
    points.count = obj->points.count * 2,
    points.capacity = points.count + 1;
    points.items = malloc(points.capacity * sizeof(Vec3));

    Vec3 p1, p2, q1, q2;
    int k = 0;
    //printf("obj->points.count = %zu\n", obj->points.count);
    for (size_t j=0; j < obj->faces.items[0].count; ++j) {
        j1 = (j+0) % obj->faces.items[0].count;
        j2 = (j+1) % obj->faces.items[0].count;

        p1 = obj->points.items[j1];
        p2 = obj->points.items[j2];

        double dx = p2.x - p1.x;
        double dy = p2.y - p1.y;
        double dz = p2.z - p1.z;

        double radiX = dx / 2.82843; // 2 * 2^0.5 = 2.82843
        double radiY = dy / 2.82843; //
        double radiZ = dz / 2.82843; //

        double ox = (p1.x + p2.x) / 2.0;
        double oy = (p1.y + p2.y) / 2.0;
        double oz = (p1.z + p2.z) / 2.0;


        q1.x = ox - F * radiX / 1.41421; // sqrt(2) = 1.41421
        q1.y = oy - F * radiY / 1.41421;
        q1.z = oz - F * radiZ / 1.41421;
        points.items[k] = q1; k += 1;

        q2.x = ox + F * radiX / 1.41421;
        q2.y = oy + F * radiY / 1.41421;
        q2.z = oz + F * radiZ / 1.41421;
        points.items[k] = q2; k += 1;
        
        //vec3_println(p1);
        //vec3_println(p2);
        //printf("j2 = %zu\n", j2);
    }

    Face f;
    f.count = points.count;
    f.capacity = points.count;
    f.items = (size_t*)malloc(sizeof(size_t)*points.count);
    for (size_t i = 0; i < points.count; ++i)
         f.items[i] = i;

    free_face (obj->faces.items[0]);
    free_points (obj->points);
    obj->faces.items[0] = f;
    obj->points = points;
    return obj;
}

Bounds cad_get_bounds(CAD obj) {
    Bounds ret = {
        .min = vec3(INFINITY, INFINITY, INFINITY),
        .max = vec3(-INFINITY, -INFINITY, -INFINITY)
    };
    for (size_t i = 0; i < obj.points.count; ++i) {
        Vec3 p = obj.points.items[i];
        if (p.x < ret.min.x) ret.min.x = p.x;
        if (p.y < ret.min.y) ret.min.y = p.y;
        if (p.z < ret.min.z) ret.min.z = p.z;
        if (p.x > ret.max.x) ret.max.x = p.x;
        if (p.y > ret.max.y) ret.max.y = p.y;
        if (p.z > ret.max.z) ret.max.z = p.z;
    }
    return ret;
}

Vec3 cad_get_size(CAD obj) {
    Bounds b = cad_get_bounds(obj);
    return vec3_sub(b.max, b.min);
}

CAD cad_curve(Points points) {
    CAD ret;
    ret.faces  = (Faces){0};
    // TODO should clone points maybe?
    ret.points = points;
    return ret;
}

Points cad_alloc_points(size_t count) {
    Points ret = {
        .count = count, 
        .capacity = count+1,
        .items = (Vec3*)malloc(sizeof(Vec3) * count)
    };
    return ret;
}

Faces cad_alloc_faces(size_t count) {
    Faces ret = {
        .count = count, 
        .capacity = count+1,
        .items = (Face*)malloc(sizeof(Face) * count)
    };
    return ret;
}

Face cad_alloc_face(size_t count) {
    Face ret = {
        .count = count, 
        .capacity = count+1,
        .items = (size_t*)malloc(sizeof(size_t) * count)
    };
    return ret;
}

CAD cad_xy_curve_from_function(val_t (*func)(val_t), val_t from, val_t to, size_t nsteps) {
    CAD ret;
    ret.faces  = (Faces){0};
    ret.points = cad_alloc_points(nsteps+1);
    for (size_t step = 0; step <= nsteps; ++step) {
        val_t x = (to - from)/nsteps*step-to;
        ret.points.items[step] = vec3(x, func(x), 0);
    }
    return ret;
}

CAD* cad_curve_to_polygon(CAD* curve) {
    Face f = cad_alloc_face(curve->points.count);
    for (size_t i = 0; i < curve->points.count; ++i)
         f.items[i] = i;
    curve->faces = cad_alloc_faces(1);
    curve->faces.items[0] = f;
    return curve;
}

#define swap_vecs(a, b) do { Vec3 t = (a); (a) = (b); (b) = (t);} while(0)

val_t maxv(val_t x, val_t y) {
    if (x > y) return x; else return y;
}

val_t minv(val_t x, val_t y) {
    if (x < y) return x; else return y;
}

bool ray_from_point_intersects_edge_2D(Vec3 p, Vec3 a, Vec3 b) {
    if (p.y > maxv(a.y, b.y)
    ||  p.y < minv(a.y, b.y)
    ||  p.x > maxv(a.x, b.x)
    ||  p.x < minv(a.x, b.x)) return false;

    if (a.y > b.y) swap_vecs(a, b);
    return (b.y - a.y) / (b.x - a.x) < (p.y - a.y) / (p.x - a.x);
}
 
bool point_inside_face2D(Vec3 p, size_t face_index, CAD obj) {
    bool is_inside = false;
    assert(face_index < obj.faces.count && "Face index out of range.");
    Face f = obj.faces.items[face_index];
    for (size_t i = 0; i < f.count; ++i) {
        Vec3 a = obj.points.items[f.items[(i + 0) % f.count]];
        Vec3 b = obj.points.items[f.items[(i + 1) % f.count]];
        if (ray_from_point_intersects_edge_2D(p, a, b)) {
            is_inside = !is_inside;
        }
    }
    return is_inside;
}

int maxi(int x, int y) {
    if (x > y) return x; else return y;
}

int mini(int x, int y) {
    if (x < y) return x; else return y;
}

typedef struct {
    size_t width;
    size_t height;
    char* mat;
    val_t* zbuffer;
} ASCII_Screen;

ASCII_Screen alloc_ascii_screen() {
    struct winsize w; 
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) {
        perror("ioctl");
        return (ASCII_Screen){0};
    }

    ASCII_Screen ret;
    ret.width  = w.ws_col;
    ret.height = w.ws_row-1;
    ret.mat     = calloc(ret.width*ret.height, sizeof(char));
    ret.zbuffer = calloc(ret.width*ret.height, sizeof(val_t));

    for (size_t i = 0; i < ret.width * ret.height; ++i){
        ret.mat[i] = ' ';
        ret.zbuffer[i] = 0.0;
    } 
    return ret;
}

void free_ascii_screen(ASCII_Screen screen) {
    free(screen.mat);
    free(screen.zbuffer);
}

ang_t angle_between(int a_x, int a_y, int b_x, int b_y) {
    return atan2(b_y - a_y, b_x - a_x);
}

char angle_to_line_char(ang_t angle_radians) {
    ang_t margin = M_PI / 10.0;

    if (fabsl( 0.0  - angle_radians) < margin) return '-';
    if (fabsl( M_PI - angle_radians) < margin) return '-';
    if (fabsl(-M_PI - angle_radians) < margin) return '-';

    if (fabsl( M_PI/2 - angle_radians) < margin) return '|';
    if (fabsl(-M_PI/2 - angle_radians) < margin) return '|';

    if (angle_radians >  M_PI / 2 && angle_radians <  M_PI) return '/';
    if (angle_radians > -M_PI / 2 &&  angle_radians < 0) return '/';

    if (angle_radians >  M_PI / 2 && angle_radians <  M_PI) return '/';
    if (angle_radians > -M_PI / 2 &&  angle_radians < 0) return '/';


    return '\\';
}


void line(ASCII_Screen screen, 
          int a_x, int a_y, 
          int b_x, int b_y) {

    int dx = abs(b_x - a_x), sx = a_x < b_x ? 1 : -1;
    int dy = abs(b_y - a_y), sy = a_y < b_y ? 1 : -1;
    int err = (dx > dy ? dx : -dy) / 2;

    while (a_x != b_x || a_y != b_y) {
        int e2 = err;
        if (e2 > -dx) { err -= dy; a_x += sx; }
        if (e2 <  dy) { err += dx; a_y += sy; }

        if (a_x >= (int)screen.width) continue; 
        if (a_y >= (int)screen.height) continue; 
        if (a_x < 0) continue; 
        if (a_y < 0) continue; 
        screen.mat[a_x + (a_y)*screen.width] = angle_to_line_char(angle_between(
            a_x, a_y, b_x, b_y
        ));
        screen.zbuffer[a_x + (a_y)*screen.width] = -10000.0;
    }
}

void cad_render_to_terminal(val_t zoom, CAD obj) {
    char chars[] = {'.', ':', '-', '=', '+', '*', '#', '%', '@'};
    int chars_count = sizeof(chars) * sizeof(chars[0]);


    ASCII_Screen screen = alloc_ascii_screen();

    CAD temp = cad_clone(obj);

    cad_scale(vec3(2, 1, 1), &temp);

    Bounds b = cad_get_bounds(temp);
    Vec3 size = vec3_sub(b.max, b.min);

    cad_translate(vec3_mult_s(b.min, -1), &temp);

    val_t s = zoom * mini(screen.height, screen.width);
    cad_scale(vec3(s, s, s), &temp);
    cad_translate(vec3(1, 1, 1), &temp);

    IndexPairs edges = get_all_edges(temp);
    for (size_t i = 0; i < edges.count; ++i) {
        line(screen, 
             temp.points.items[edges.items[i].first].x,
             temp.points.items[edges.items[i].first].y,
             temp.points.items[edges.items[i].second].x,
             temp.points.items[edges.items[i].second].y);
    }

    for (size_t i = 0; i < temp.points.count; ++i) {
        Vec3 p = temp.points.items[i];

        if (p.y < 0) continue; 
        if (p.x < 0) continue; 
        if (p.y >= screen.height) continue;
        if (p.x >= screen.width ) continue;

        int x = p.x;
        int y = p.y;

        if (p.z > screen.zbuffer[x+(y*screen.width)]) {
            int c = p.z * (chars_count / size.z);
            c = mini(c, chars_count-1);
            c = maxi(c, 0);

            screen.mat[x+(y*screen.width)] = chars[c];
        }
    }



    printf("\033[H\033[2J");
    for (size_t y = 0; y < screen.height; ++y) {
        for (size_t x = 0; x < screen.width; ++x) {
            printf("%c", screen.mat[screen.width*y + x]);
        }
        printf("\n");
    }

    free_ascii_screen(screen);
}

#endif // CADIGO_IMPLEMENTATION
