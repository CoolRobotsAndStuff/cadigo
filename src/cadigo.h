#ifndef CADIGO_H_
#define CADIGO_H_

#include <string.h>
#include <stdlib.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288419716939937510582097494459230781640628620899862803482534211706798214808651328230664709384460955058223172535940812848111745028410270193852110555964462294895493038196442881097566593344612847564823378678316527120190914564856692346034861045432664821339360726024914127372458700660631558817488152092096282925409171536436789259036001133053054882046652138414695194151160943305727036575959195309218611738193261179310511854807446237996274956735188575272489122793818301194912983367336244065664308602139494639522473719070217986094370277053921717629317675238467481846766940513200056812714526356082778577134275778960917363717872146844090122495343014654958537105079227968925892354201995611212902196086403441815981362977477130996051870721134999999837297804995105973173281609631859502445945534690830264252230825334468503526193118817101000313783875288658753320838142061717766914730359825349042875546873115956286388235378759375195778185778053217122680661300192787661119590921642019893809525720106548586327886593615338182796823030195203530185296899577362259941389124972177528347913151557485724245415069595082953311686172785588907509838175463746493931925506040092770167113900984882401285836160356370766010471018194295559619894676783744944825537977472684710404753464620804668425906949129331367702898915210475216205696602405803815019351125338243003558764024749647326391419927260426992279678235478163600934172164121992458631503028618297455570674983850549458858692699569092721079750930295532116534498720275596023648066549911988183479775356636980742654252786255181841757467289097777279380008164706001614524919217321721477235014144197356854816136115735255213347574184946843852332390739414333454776241686251898356948556209921922218427255025425688767179049460165346680498862723279178608578438382796797668145410095388378636095068006422512520511739298489608412848862694560424196528502221066118630674427862203919494504712371378696095636437191728746776465757396241389086583264599581339047802759009
#endif

typedef long double val_t;
typedef val_t ang_t;

typedef struct {
    union { val_t x; ang_t roll;  val_t first; };
    union { val_t y; ang_t pitch; val_t second;};
    union { val_t z; ang_t yaw;   val_t third;};
    int mark; // some algorithms are easier if you can just mark certain points;
} Vec3;

Vec3 vec3(val_t x, val_t y, val_t z);

typedef struct {
    val_t mag;
    ang_t ang;
} Polar2D;

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
CAD cad_copy(CAD obj);

CAD cad_polyhedron(Points points, Faces faces);
CAD cad_polygon(Points points);
CAD cad_line(Points points);

CAD cad_cube(val_t l);

// Operations - Similar Geometry
CAD cad_translate(Vec3 v, CAD obj);
CAD cad_rotate(Vec3 v, CAD obj);
CAD cad_scale(Vec3 v, CAD obj);

// Operations - Topological Geometry
CAD cad_catmull_clark(CAD obj);
CAD cad_subdivide(CAD obj);

// Operations - Booleans
CAD cad_intersection(CAD obj);
CAD cad_difference(CAD obj);
CAD cad_union(CAD obj);

#endif // CADIGO_H_

#ifdef CADIGO_IMPLEMENTATION

Vec3 vec3(val_t x, val_t y, val_t z) {
    Vec3 v;
    v.x = x;
    v.y = y;
    v.z = z;
    v.mark = 0;
    return v;
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

Face copy_face(Face f) {
    Face ret;
    ret.count = f.count;
    ret.capacity = f.capacity;
    ret.items = (size_t*)malloc(da_size(f));
    memcpy(ret.items, f.items, da_size(f));
    return ret;
}

CAD cad_copy(CAD obj) {
    CAD ret;
    ret.points.count    = obj.points.count;
    ret.points.capacity = obj.points.capacity;
    ret.points.items    = (Vec3*)malloc(da_size(obj.points));
    memcpy(ret.points.items, obj.points.items, da_size(obj.points));

    ret.faces.count    = obj.faces.count;
    ret.faces.capacity = obj.faces.capacity;
    ret.faces.items    = (Face*)malloc(da_size(obj.faces));
    for (size_t i = 0; i < obj.faces.count; ++i)
        ret.faces.items[i] = copy_face(ret.faces.items[i]);
    return ret;
}

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

CAD cad_translate(Vec3 v, CAD obj) {
    for (size_t i = 0; i < obj.points.count; ++i) {
        Vec3 old = obj.points.items[i];
        obj.points.items[i] = vec3(old.x + v.x, old.y + v.y, old.z + v.z);
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

CAD cad_rotate(Vec3 v, CAD obj) {
    for (size_t i = 0; i < obj.points.count; ++i) {
        obj.points.items[i] = vec3_rotate_roll (degs2rads(v.roll),  obj.points.items[i]);
        obj.points.items[i] = vec3_rotate_pitch(degs2rads(v.pitch), obj.points.items[i]);
        obj.points.items[i] = vec3_rotate_yaw  (degs2rads(v.yaw),   obj.points.items[i]);
    }
    return obj;
}

CAD cad_scale(Vec3 v, CAD obj) {
    for (size_t i=0; i < obj.points.count; ++i) {
        obj.points.items[i].x *= v.x;
        obj.points.items[i].y *= v.y;
        obj.points.items[i].z *= v.z;
    }
    return obj;
}


typedef struct {
    size_t a;
    size_t b;
} IndexPair;

typedef struct {
    size_t count;
    size_t capacity;
    IndexPair* items;
} IndexPairs;

CAD cad_split_edge(CAD obj, IndexPair edge) {
    ...
}

IndexPairs get_all_edges(CAD obj) {
    ...
}

Faces get_adjancent_faces_to_edge(IndexPair edge) {
    ...
}

CAD cad_catmull_clark(CAD obj) {
    const int original_point = 1;
    const int edge_point = 2;
    const int face_point = 3;

    for (size_t i = 0; i < obj.points.count; ++i) {
        obj.points.items[i].mark = original_point;
    }

    Points face_points = {
        count = 0;
        capacity = obj.faces.count;
        items = malloc(obj.faces.count * sizeof(Vec3));
    }

    for (size_t i = 0; i < obj.faces.count; ++i) {
        Face f = obj.faces.items[i];
        Vec3 avg = 0;
        for (size_t j = 0; j < f.count; ++j) {
            avg.x += f.items[j].x; 
            avg.y += f.items[j].y; 
            avg.z += f.items[j].z; 
        }
        avg.x = avg.x / obj.faces.count;
        avg.y = avg.y / obj.faces.count;
        avg.z = avg.z / obj.faces.count;

        face_points[i] = avg; 
    }

    IndexPairs original_edges = get_all_edges(obj);
    for (size_t i = 0; i < original_edges.count; ++i) {
        cad_split_edge(obj, original_edges[i]); // edge point gets added at the end

        obj.points.items[obj.points.count-1].mark = edge_point;

        IndexPair adjancent_faces = get_adjancent_face_indexes_to_edge(original_edges[i]);
        obj.points.items[obj.points.count-1] = (original_edges[i].a + 
                                                original_edges[i].b +
                                                face_points[adjancent_faces.a] +
                                                face_points[adjancent_faces.b]) / 4;
    }


}

#endif // CADIGO_IMPLEMENTATION
