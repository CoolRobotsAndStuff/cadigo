#ifndef VIZ_H_
#define VIZ_H_

#define STB_IMAGE_IMPLEMENTATION
#include "../lib/stb_image.h"

#define RGFW_IMPLEMENTATION
#include "../lib/RGFW.h"
#include <math.h>
#define DEG2RAD 3.14/180.0

typedef struct {
    RGFW_window* win;
    float pitch, yaw;
    float camX, camZ, camY;
    unsigned int fps;
    bool show_lines;
    float dt;
} CAD_Viz;

CAD_Viz* cad_viz_init();
bool cad_viz_keep_rendenring(CAD_Viz* ctx);
void cad_viz_begin(CAD_Viz* ctx);
void cad_viz_end(CAD_Viz* ctx);
void cad_viz_render(CAD_Viz* ctx, CAD obj);

#endif // VIZ_H_


#ifdef CADIGO_IMPLEMENTATION

val_t vec3_magnitude(Vec3 v) {
    return sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}

Vec3 vec3_to_unit(Vec3 v) {
    return vec3_div_s(v, vec3_magnitude(v));
}

val_t vec3_sum(Vec3 v) {
    return v.x+v.y+v.z;
}

val_t vec3_dot(Vec3 a, Vec3 b) {
    vec3_mult_by(&a, b);
    return vec3_sum(a);
}

Vec3 vec3_parallel_to_axis(Vec3 v, Vec3 axis) {
    // Project v onto axis. Result will be parallel to axis
    // (v * axis / norm(axis)) * axis / norm(axis)
    // = (v * axis) * axis / norm(axis)^2
    // = (v * axis) * axis / (axis' * axis)
    return vec3_mult_s(axis, vec3_dot(v, axis) / vec3_dot(axis, axis));
}

Vec3 vec3_perpendicular_to_axis(Vec3 v, Vec3 axis) {
    //Component of x orthogonal to v. Result is perpendicular to v."""
    return vec3_sub(v, vec3_parallel_to_axis(v, axis));
}

Vec3 vec3_cross(Vec3 a, Vec3 b) {
    Vec3 r;
    r.x = a.y * b.z - a.z * b.y;
    r.y = a.z * b.x - a.x * b.z;
    r.z = a.x * b.y - a.y * b.x;
    return r;
}

Vec3 vec3_zero() {
    return vec3(0, 0, 0);
}

void vec3_rotate_around_axis(Vec3* v, val_t angle, Vec3 axis) {
    // http://math.stackexchange.com/a/1432182/81266
    Vec3 perpend = vec3_perpendicular_to_axis(*v, axis);
    Vec3 parallel = vec3_parallel_to_axis(*v, axis);

    Vec3 w = vec3_cross(axis, perpend);

    *v = vec3_zero();
    vec3_add_to(v, parallel);
    vec3_add_to(v, vec3_mult_s(perpend, cos(angle)));
    vec3_add_to(v, 
        vec3_mult_s(
            vec3_mult_s(
                vec3_to_unit(w),
                vec3_magnitude(perpend)
            ), sin(angle)
        )
    );
}

static inline void cad_viz_glPerspective(double fovY, double aspect, double zNear, double zFar) {
    const double f = 1 / (cos(fovY) * sin(fovY));
    float projectionMatrix[16] = {0};
    
    projectionMatrix[0] = f / aspect;
    projectionMatrix[5] = f;
    projectionMatrix[10] = (zFar + zNear) / (zNear - zFar);
    projectionMatrix[11] = -1.0;
    projectionMatrix[14] = (2.0 * zFar * zNear) / (zNear - zFar);
    
    glMultMatrixf(projectionMatrix);
}


CAD_Viz* cad_viz_init() {
    RGFW_window* win = RGFW_createWindow("Cadigo Visualizer", RGFW_RECT(0, 0, 800, 450), RGFW_windowCenter | RGFW_windowNoResize );

    RGFW_window_showMouse(win, 0);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    cad_viz_glPerspective(60, 16.0 / 9.0, 1, 1000);
    glMatrixMode(GL_MODELVIEW);

    RGFW_window_mouseHold(win, RGFW_AREA(win->r.w / 2, win->r.h / 2));    
    CAD_Viz* ret = malloc(sizeof(CAD_Viz));

    ret->win = win;
    ret->pitch=0.0;
    ret->yaw=0.0;
    ret->camX=0;
    ret->camZ=0;
    ret->camY=0;
    ret->fps=60;
    ret->dt=1/60;
    ret->show_lines=true;
    return ret;
}

bool cad_viz_keep_rendenring(CAD_Viz* ctx) {
    return !RGFW_window_shouldClose(ctx->win);
}

void cad_viz_begin_no_events(CAD_Viz* ctx) {
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // Update camera
    if      (ctx->pitch >=  70) ctx->pitch = 70;
    else if (ctx->pitch <= -60) ctx->pitch = -60;
    glRotatef(ctx->pitch, 1.0, 0.0, 0.0);
    glRotatef(ctx->yaw  , 0.0, 1.0, 0.0); 
    glTranslatef(ctx->camX, ctx->camY, -ctx->camZ);

    glViewport(0, 0, ctx->win->r.w, ctx->win->r.h);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}


void cad_viz_begin(CAD_Viz* ctx) {
    while (RGFW_window_checkEvent(ctx->win)) {
        if (ctx->win->event.type == RGFW_quit) {
            RGFW_window_close(ctx->win);
            return;
        }

        switch (ctx->win->event.type) {
            case RGFW_mousePosChanged: {
                int dev_x = ctx->win->event.vector.x;
                int dev_y = ctx->win->event.vector.y;
                ctx->yaw   += (float)dev_x / 15.0;
                ctx->pitch += (float)dev_y / 15.0;
                break;
            }
            case RGFW_keyPressed:
                switch (ctx->win->event.key) {
                    case RGFW_return:
                        RGFW_window_showMouse(ctx->win, 0);
                        RGFW_window_mouseHold(ctx->win, RGFW_AREA(ctx->win->r.w / 2, ctx->win->r.h / 2));    
                        break;

                    case RGFW_backSpace:
                        RGFW_window_showMouse(ctx->win, 1);
                        RGFW_window_mouseUnhold(ctx->win);
                        break;

                    case RGFW_q:
                        RGFW_window_close(ctx->win);
                        return;
                        break;

                    case RGFW_left:   ctx->yaw -= 5; break;
                    case RGFW_right:  ctx->yaw += 5; break;
                    case RGFW_up:   ctx->pitch -= 5; break;
                    case RGFW_down: ctx->pitch += 5; break;
                    default: break;
                }
                break;
            default:
                break;
        }
    }

    float speed = 60;

    if (RGFW_isPressed(ctx->win, RGFW_w)) {
        ctx->camX += (cos((ctx->yaw + 90) * DEG2RAD)/5.0)*ctx->dt*speed;
        ctx->camZ -= (sin((ctx->yaw + 90) * DEG2RAD)/5.0)*ctx->dt*speed;
    }
    if (RGFW_isPressed(ctx->win, RGFW_s)) {
        ctx->camX += cos((ctx->yaw + 270) * DEG2RAD)/5.0*ctx->dt*speed;
        ctx->camZ -= sin((ctx->yaw + 270) * DEG2RAD)/5.0*ctx->dt*speed;
    }

    if (RGFW_isPressed(ctx->win, RGFW_a)) {
        ctx->camX += cos(ctx->yaw * DEG2RAD)/5.0*ctx->dt*speed;
        ctx->camZ -= sin(ctx->yaw * DEG2RAD)/5.0*ctx->dt*speed;
    }

    if (RGFW_isPressed(ctx->win, RGFW_d)) {
        ctx->camX += cos((ctx->yaw + 180) * DEG2RAD)/5.0*ctx->dt*speed;
        ctx->camZ -= sin((ctx->yaw + 180) * DEG2RAD)/5.0*ctx->dt*speed;
    }

    if (RGFW_isPressed(ctx->win, RGFW_space))  ctx->camY -= 0.2*ctx->dt*speed;
    if (RGFW_isPressed(ctx->win, RGFW_shiftL)) ctx->camY += 0.2*ctx->dt*speed;
    
    float rot_sensitivity = 1;
    if (RGFW_isPressed(ctx->win, RGFW_h)) ctx->yaw   -= rot_sensitivity*ctx->dt*speed;
    if (RGFW_isPressed(ctx->win, RGFW_l)) ctx->yaw   += rot_sensitivity*ctx->dt*speed;
    if (RGFW_isPressed(ctx->win, RGFW_j)) ctx->pitch += rot_sensitivity*ctx->dt*speed;
    if (RGFW_isPressed(ctx->win, RGFW_k)) ctx->pitch -= rot_sensitivity*ctx->dt*speed;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // Update camera
    if      (ctx->pitch >=  80) ctx->pitch = 80;
    else if (ctx->pitch <= -80) ctx->pitch = -80;
    glRotatef(ctx->pitch, 1.0, 0.0, 0.0);
    glRotatef(ctx->yaw  , 0.0, 1.0, 0.0); 
    glTranslatef(ctx->camX, ctx->camY, -ctx->camZ);

    glViewport(0, 0, ctx->win->r.w, ctx->win->r.h);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

// Vec3 vec3_new_perpendicular_to(Vec3 v) {
//     //https://stackoverflow.com/a/43454629
//     bool b0 = (abs(v.x) <  abs(v.y)) && (abs(v.x) <  abs(v.z));
//     bool b1 = (abs(v.y) <= abs(v.x)) && (abs(v.y) <  abs(v.z));
//     bool b2 = (abs(v.z) <= abs(v.x)) && (abs(v.z) <= abs(v.y));
//
//     return vec3_cross(v, vec3((val_t)b0, (val_t)b1, (val_t)b2));
// }

void cad_viz_render_vec3_at(CAD_Viz* ctx, Vec3 position, Vec3 v) {
    Vec3 color = vec3(0.8, 0.1, 0.1);
    (void) ctx;
    Vec3 end_point = vec3_add(v, position);
    glLineWidth(5);
    glBegin(GL_LINES);
        glColor3f(color.x, color.y, color.z);
        glVertex3f(position.x, position.y, position.z);
        glVertex3f(end_point.x, end_point.y, end_point.z);
    glEnd();

    val_t ang = 0.05;

    Vec3 v_prime = vec3_mult_s(v, 0.9);
    Vec3 p1 = vec3_add(
        vec3_rotate_yaw  (ang+(2*M_PI),
        vec3_rotate_roll (ang+(2*M_PI),
        vec3_rotate_pitch(ang+(2*M_PI),
            v_prime
        ))),
        position
    );

    Vec3 p2 = vec3_add(
        vec3_rotate_yaw  (-ang+(2*M_PI),
        vec3_rotate_roll (-ang+(2*M_PI),
        vec3_rotate_pitch(-ang+(2*M_PI),
            v_prime
        ))),
        position
    );

    glBegin(GL_TRIANGLES);
        glColor3f(0.8, 0.1, 0.8);
        glVertex3f(end_point.x, end_point.y, end_point.z);
        glVertex3f(p1.x, p1.y, p1.z);
        glVertex3f(p2.x, p2.y, p2.z);
    glEnd();
    glLineWidth(1);
}


void cad_viz_render(CAD_Viz* ctx, CAD obj) {
    (void) ctx;

    for (size_t i = 0; i < obj.faces.count; ++i) {
        Face face = obj.faces.items[i];

        if (face.count < 3) continue;  // Skip invalid faces
        Vec3 normal = cad_calculate_face_normal(obj, i);

        // light = normal.x * light_dir.x + normal.y * light_dir.y + normal.z * light_dir.z;
        float ambient_light = 1.5;
        float brightness = 0.40;
        float lighting = (normal.z+ambient_light) * brightness;

        Vec3 center = vec3(0, 0, 0);
        for (size_t j = 0; j < face.count; ++j) {
            vec3_add_to(&center, obj.points.items[face.items[j]]);
        }
        vec3_div_by_s(&center, face.count);

        //printf("texture: %d\n", texture);
        // if (texture >= 0) {
        //     glBindTexture(GL_TEXTURE_2D, texture);
        //     glEnable(GL_TEXTURE_2D);
        // }

        // glBegin(GL_POLYGON);
        //     glColor4f(1, 1, 1, 1);
        //     glTexCoord2f(0, 0);
        //     glVertex2f(-0.8f,  0.8f);
        //     glTexCoord2f(0, 1);
        //     glVertex2f(-0.8f, -0.8f);
        //     glTexCoord2f(1, 1);
        //     glVertex2f( 0.8f, -0.8f);
        //     glTexCoord2f(1, 0);
        //     glVertex2f( 0.8f,  0.8f);
        // glEnd();


        glBegin(GL_TRIANGLE_FAN);
            glColor3f(lighting*obj.color.x, lighting*obj.color.y, lighting*obj.color.z);

            glTexCoord2f(0.5, 0.5);
            glVertex3f(center.x, center.y, center.z);
            for (size_t j = 0; j < face.count; ++j) {
                Vec3 v = obj.points.items[face.items[j]];
                glTexCoord2f(1, i%2);
                glVertex3f(v.x, v.y, v.z);
            }

            if (face.count) {
                Vec3 v = obj.points.items[face.items[0]];
                glTexCoord2f(1, 1);
                glVertex3f(v.x, v.y, v.z);
            }
        glEnd();

        if (ctx->show_lines) {
            glBegin(GL_LINES);
                for (size_t j = 0; j < face.count; ++j) {
                    Vec3 v1 = obj.points.items[face.items[j]];
                    Vec3 v2 = obj.points.items[face.items[(j+1)%(face.count)]];
                        glColor3f(0, 0, 0);
                        glVertex3f(v1.x, v1.y, v1.z);
                        glVertex3f(v2.x, v2.y, v2.z);
                }
            glEnd();
        }
    }
}


void cad_viz_end(CAD_Viz* ctx) {
    RGFW_window_swapBuffers(ctx->win);
    u32 current_fps = RGFW_window_checkFPS(ctx->win, ctx->fps);
    ctx->dt = 1.0 / current_fps;
    //printf("Current fps: %du\n", current_fps);
}

int load_texture(const char *path) {
    int w, h, channels;
    stbi_set_flip_vertically_on_load(1);
    unsigned char *data = stbi_load(path, &w, &h, &channels, 0);
    if (!data) return 0;

    int format = GL_RGB; //(channels == 4) ? GL_RGBA : (channels == 3) ? GL_RGB : GL_LUMINANCE;

    unsigned int tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, data);

    stbi_image_free(data);
    return tex;
}


int cad_visualize(CAD obj) {
    float pitch=0.0, yaw=0.0;
    float camX=0, camZ=0, camY=0;

    const char *img = "texture.png";

    RGFW_window* win = RGFW_createWindow("Cadigo Visualizer", RGFW_RECT(0, 0, 800, 450), RGFW_windowCenter | RGFW_windowNoResize );
    int tex = load_texture(img);

    RGFW_window_showMouse(win, 0);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    cad_viz_glPerspective(60, 16.0 / 9.0, 1, 1000);
    glMatrixMode(GL_MODELVIEW);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex);

    RGFW_window_mouseHold(win, RGFW_AREA(win->r.w / 2, win->r.h / 2));    
    while (RGFW_window_shouldClose(win) == 0) {
        while (RGFW_window_checkEvent(win)) {
            if (win->event.type == RGFW_quit) goto close_and_return;

            switch (win->event.type) {
                case RGFW_mousePosChanged: {
                    int dev_x = win->event.vector.x;
                    int dev_y = win->event.vector.y;
                    yaw   += (float)dev_x / 15.0;
                    pitch += (float)dev_y / 15.0;
                    break;
                }
                case RGFW_keyPressed:
                    switch (win->event.key) {
                        case RGFW_return:
                            RGFW_window_showMouse(win, 0);
                            RGFW_window_mouseHold(win, RGFW_AREA(win->r.w / 2, win->r.h / 2));    
                            break;

                        case RGFW_backSpace:
                            RGFW_window_showMouse(win, 1);
                            RGFW_window_mouseUnhold(win);
                            break;

                        case RGFW_q:
                            goto close_and_return;
                            break;

                        case RGFW_left:   yaw -= 5; break;
                        case RGFW_right:  yaw += 5; break;
                        case RGFW_up:   pitch -= 5; break;
                        case RGFW_down: pitch += 5; break;
                        default: break;
                    }
                    break;
                default:
                    break;
            }
        }

        if (RGFW_isPressed(win, RGFW_w)) {
            camX += cos((yaw + 90) * DEG2RAD)/5.0;
            camZ -= sin((yaw + 90) * DEG2RAD)/5.0;
        }
        if (RGFW_isPressed(win, RGFW_s)) {
            camX += cos((yaw + 270) * DEG2RAD)/5.0;
            camZ -= sin((yaw + 270) * DEG2RAD)/5.0;
        }

        if (RGFW_isPressed(win, RGFW_a)) {
            camX += cos(yaw * DEG2RAD)/5.0;
            camZ -= sin(yaw * DEG2RAD)/5.0;
        }

        if (RGFW_isPressed(win, RGFW_d)) {
            camX += cos((yaw + 180) * DEG2RAD)/5.0;
            camZ -= sin((yaw + 180) * DEG2RAD)/5.0;
        }

        if (RGFW_isPressed(win, RGFW_space))  camY -= 0.2;
        if (RGFW_isPressed(win, RGFW_shiftL)) camY += 0.2;
        
        float rot_sensitivity = 1;
        if (RGFW_isPressed(win, RGFW_h)) yaw   -= rot_sensitivity;
        if (RGFW_isPressed(win, RGFW_l)) yaw   += rot_sensitivity;
        if (RGFW_isPressed(win, RGFW_j)) pitch += rot_sensitivity;
        if (RGFW_isPressed(win, RGFW_k)) pitch -= rot_sensitivity;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();

        // Update camera
        if      (pitch >=  70) pitch = 70;
        else if (pitch <= -60) pitch = -60;
        glRotatef(pitch, 1.0, 0.0, 0.0);
        glRotatef(yaw  , 0.0, 1.0, 0.0); 
        glTranslatef(camX, camY, -camZ);


        glViewport(0, 0, win->r.w, win->r.h);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        for (size_t i = 0; i < obj.faces.count; ++i) {
            Face face = obj.faces.items[i];
            if (face.count < 3) continue;  // Skip invalid faces
            Vec3 normal = cad_calculate_face_normal(obj, i);

            // light = normal.x * light_dir.x + normal.y * light_dir.y + normal.z * light_dir.z;
            float ambient_light = 1.5;
            float brightness = 0.20;
            float lighting = (normal.z+ambient_light) * brightness;
            glBegin(GL_POLYGON);
                glColor3f(lighting, lighting, lighting);

                for (size_t j = 0; j < face.count; ++j) {
                    Vec3 v = obj.points.items[face.items[j]];
                    if (j%4 == 0) glTexCoord2f(0, 0);
                    if (j%4 == 1) glTexCoord2f(0, 1);
                    if (j%4 == 2) glTexCoord2f(1, 1);
                    if (j%4 == 3) glTexCoord2f(1, 0);
                    glVertex3f(v.x, v.y, v.z);
                }
            glEnd();


                for (size_t j = 0; j < face.count; ++j) {
                    Vec3 v1 = obj.points.items[face.items[j]];
                    Vec3 v2 = obj.points.items[face.items[(j+1)%(face.count)]];
                    glBegin(GL_LINES);
                        glColor3f(0, 0, 0);
                        glVertex3f(v1.x, v1.y, v1.z);
                        glVertex3f(v2.x, v2.y, v2.z);
                    glEnd();
                }
        }

        RGFW_window_swapBuffers(win);
        RGFW_window_checkFPS(win, 60);
    }

close_and_return:

    RGFW_window_close(win);
    return 0;
}

#endif // CADIGO_IMPLEMENTATION
