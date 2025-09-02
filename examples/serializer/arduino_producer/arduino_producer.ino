#define CAD_NO_IOCTL
#include "libs/common.c"

typedef bool (*writer)(char*, size_t, void*);

#define da_dump_size(da) ( (sizeof((da).count) + sizeof(*((da).items)) * ((da).count)) )

bool cad_dump(writer write, void* ctx, CAD obj) {
    bool ok;
    ok = write((char*)&obj.points.count, sizeof(obj.points.count), ctx);
    if (!ok) return false;
    ok = write((char*)obj.points.items, da_items_size(obj.points), ctx);
    if (!ok) return false;


    ok = write((char*)&obj.faces.count, sizeof(obj.faces.count), ctx);
    if (!ok) return false;
    for (size_t i = 0; i < obj.faces.count; ++i) {
        Face face = obj.faces.items[i]; 
        ok = write((char*)&(face.count), sizeof(face.count), ctx);
        if (!ok) return false;
        ok = write((char*)face.items, da_items_size(face), ctx);
        if (!ok) return false;
    }

    ok = write((char*)&obj.color, sizeof(Vec3), ctx);
    if (!ok) return false;
    return true;
}

bool cads_dump(writer write, void* ctx, CADs cads) {
    bool ok;
    ok = write((char*)&cads.count, sizeof(cads.count), ctx);
    if (!ok) return false;
    for (size_t i = 0; i < cads.count; ++i) {
        if (!cad_dump(write, ctx, *cads.items[i])) return false;
    }
    return true;
}

bool write_serial(char* data, size_t size, void* _) {
    (void)_;
    size_t w = Serial.write(data, size);
    //printf("tried to write %zu bytes from %p and wrote %zd\n", size, data, w);
    return true;
}



CAD* cad_regular_polygon(val_t radius, size_t sides) {
    Points ps = {0};
    for (size_t i =0; i<sides; ++i) {
        val_t angle = i * (M_PI*2/sides);
        Vec3 v = vec3(
            cos(angle)*radius,
            sin(angle)*radius,
            0
        );
        da_append(&ps, v);
    }
    CAD* ret = cad_polygon(ps);
    return ret;
}

Vec3 color_wood;
Vec3 color_yellow;
Vec3 color_blue;
Vec3 color_black;

CAD* wheel_model(float side) {
    CAD* ret = cad_regular_polygon(6.5/2, 20);
    cad_extrude(ret, 2.5);
    cad_rotate_y(ret, 90*side);
    ret->color = color_black;
    cad_translate_x(ret, -3.5*side);
    cad_translate_z(ret, -6);
    cad_translate_y(ret, 0.5+1);
    return ret;
}

CAD* motor_model(float side) {
    CAD* ret = cad_cube(1);
    cad_scale(ret, vec3(2, 2, 6));
    cad_translate_x(ret, side*(6.5/2-1));
    cad_translate_y(ret, 0.5+1);
    cad_translate_z(ret, -4);
    ret->color = color_yellow;
    return ret;
}

CAD* model;

val_t rotation = 0;

void setup() {
    color_wood = vec3(189/255.0, 148/255.0, 89/255.0);
    color_yellow = vec3(0.8, 0.8, 0.1);
    color_blue = vec3(0.1, 0.1, 0.8);
    color_black = vec3(0.1, 0.1, 0.1);
    Serial.begin(576000);
    delay(1000);
}

void loop() {
    bool ok;
    size_t n = 9;

    ok = write_serial((char*)&n, sizeof(n), NULL);
    if (!ok) Serial.println("ERROROOROR");

    model = wheel_model(1);
    cad_rotate_y(model, rotation);
    ok = cad_dump(write_serial, NULL, *model);
    if (!ok) Serial.println("ERROROOROR");
    cad_free(model);

    model = wheel_model(-1);
    cad_rotate_y(model, rotation);
    ok = cad_dump(write_serial, NULL, *model);
    if (!ok) Serial.println("ERROROOROR");
    cad_free(model);

    model = motor_model(1);
    cad_rotate_y(model, rotation);
    ok = cad_dump(write_serial, NULL, *model);
    if (!ok) Serial.println("ERROROOROR");
    cad_free(model);

    model = motor_model(-1);
    cad_rotate_y(model, rotation);
    ok = cad_dump(write_serial, NULL, *model);
    if (!ok) Serial.println("ERROROOROR");
    cad_free(model);

    // chasis_main
    model = cad_cube(1);
    cad_scale(model, vec3(8.8, 1, 5));
    model->color = color_wood;
    cad_rotate_y(model, rotation);
    ok = cad_dump(write_serial, NULL, *model);
    if (!ok) Serial.println("ERROROOROR");
    cad_free(model);
    
    // chasis_tail
    model = cad_cube(1);
    cad_scale(model, vec3(6.5, 1, 7));
    cad_translate_z(model, -6);
    model->color = color_wood;
    cad_rotate_y(model, rotation);
    ok = cad_dump(write_serial, NULL, *model);
    if (!ok) Serial.println("ERROROOROR");
    cad_free(model);
    
    // chasis_nose
    model = cad_cube(1);
    cad_scale(model, vec3(3, 1, 6));
    cad_translate_z(model, 5.5);
    model->color = color_wood;
    cad_rotate_y(model, rotation);
    ok = cad_dump(write_serial, NULL, *model);
    if (!ok) Serial.println("ERROROOROR");
    cad_free(model);

    // arduino
    model = cad_cube(1);
    cad_scale(model, vec3(7, 0.2, 5));
    cad_translate_y(model, 0.6);
    cad_translate_z(model, 3);
    model->color = color_blue;
    cad_rotate_y(model, rotation);
    ok = cad_dump(write_serial, NULL, *model);
    if (!ok) Serial.println("ERROROOROR");
    cad_free(model);
    
    // sensor
    model = cad_cube(1);
    cad_scale(model, vec3(1, 0.2, 3.5));
    cad_translate_y(model, 0.5);
    cad_translate_z(model, 8);
    model->color = color_black;
    cad_rotate_y(model, rotation);
    ok = cad_dump(write_serial, NULL, *model);
    if (!ok) Serial.println("ERROROOROR");
    cad_free(model);


    rotation += 1;
    //delay(10);
}

