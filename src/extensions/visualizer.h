#ifndef VIZ_H_
#define VIZ_H_

#define RGFW_IMPLEMENTATION
#include "../lib/RGFW.h"
#include <math.h>
#define DEG2RAD 3.14/180.0

typedef struct {
    RGFW_window* win;
    CADs objs;
    float pitch, yaw;
    float camX, camZ, camY;
    unsigned int fps;
} CAD_Viz;

CAD_Viz* cad_viz_init();
bool cad_viz_keep_rendenring(CAD_Viz* ctx);
void cad_viz_begin(CAD_Viz* ctx);
void cad_viz_end(CAD_Viz* ctx);
void cad_viz_render(CAD_Viz* ctx, CAD obj);

#endif // VIZ_H_


#ifdef CADIGO_IMPLEMENTATION

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
    CAD_Viz* ret = malloc(sizeof(*ret));

    ret->win = win;
    ret->objs = (CADs){0};
    ret->pitch=0.0;
    ret->yaw=0.0;
    ret->camX=0;
    ret->camZ=0;
    ret->camY=0;
    ret->fps=60;
    return ret;
}

bool cad_viz_keep_rendenring(CAD_Viz* ctx) {
    return !RGFW_window_shouldClose(ctx->win);
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

    if (RGFW_isPressed(ctx->win, RGFW_w)) {
        ctx->camX += cos((ctx->yaw + 90) * DEG2RAD)/5.0;
        ctx->camZ -= sin((ctx->yaw + 90) * DEG2RAD)/5.0;
    }
    if (RGFW_isPressed(ctx->win, RGFW_s)) {
        ctx->camX += cos((ctx->yaw + 270) * DEG2RAD)/5.0;
        ctx->camZ -= sin((ctx->yaw + 270) * DEG2RAD)/5.0;
    }

    if (RGFW_isPressed(ctx->win, RGFW_a)) {
        ctx->camX += cos(ctx->yaw * DEG2RAD)/5.0;
        ctx->camZ -= sin(ctx->yaw * DEG2RAD)/5.0;
    }

    if (RGFW_isPressed(ctx->win, RGFW_d)) {
        ctx->camX += cos((ctx->yaw + 180) * DEG2RAD)/5.0;
        ctx->camZ -= sin((ctx->yaw + 180) * DEG2RAD)/5.0;
    }

    if (RGFW_isPressed(ctx->win, RGFW_space))  ctx->camY -= 0.2;
    if (RGFW_isPressed(ctx->win, RGFW_shiftL)) ctx->camY += 0.2;
    
    float rot_sensitivity = 1;
    if (RGFW_isPressed(ctx->win, RGFW_h)) ctx->yaw   -= rot_sensitivity;
    if (RGFW_isPressed(ctx->win, RGFW_l)) ctx->yaw   += rot_sensitivity;
    if (RGFW_isPressed(ctx->win, RGFW_j)) ctx->pitch += rot_sensitivity;
    if (RGFW_isPressed(ctx->win, RGFW_k)) ctx->pitch -= rot_sensitivity;

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

void cad_viz_render(CAD_Viz* ctx, CAD obj) {
    (void) ctx;
    for (size_t i = 0; i < obj.faces.count; ++i) {
        Face face = obj.faces.items[i];

        if (face.count < 3) continue;  // Skip invalid faces
        Vec3 normal = cad_calculate_face_normal(obj, i);

        // light = normal.x * light_dir.x + normal.y * light_dir.y + normal.z * light_dir.z;
        float ambient_light = 1.5;
        float brightness = 0.20;
        float lighting = (normal.z+ambient_light) * brightness;

        
        Vec3 center = vec3(0, 0, 0);
        for (size_t j = 0; j < face.count; ++j) {
            vec3_add_to(&center, obj.points.items[face.items[j]]);
        }
        vec3_div_by_s(&center, face.count);

        glBegin(GL_TRIANGLE_FAN);
            glColor3f(lighting, lighting, lighting);

            glVertex3f(center.x, center.y, center.z);
            for (size_t j = 0; j < face.count; ++j) {
                Vec3 v = obj.points.items[face.items[j]];
                glVertex3f(v.x, v.y, v.z);
            }

            if (face.count) {
                Vec3 v = obj.points.items[face.items[0]];
                glVertex3f(v.x, v.y, v.z);
            }
        glEnd();

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

void cad_viz_end(CAD_Viz* ctx) {
    RGFW_window_swapBuffers(ctx->win);
    RGFW_window_checkFPS(ctx->win, ctx->fps);
}



int cad_visualize(CAD obj) {
    float pitch=0.0, yaw=0.0;
    float camX=0, camZ=0, camY=0;

    RGFW_window* win = RGFW_createWindow("Cadigo Visualizer", RGFW_RECT(0, 0, 800, 450), RGFW_windowCenter | RGFW_windowNoResize );

    RGFW_window_showMouse(win, 0);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    cad_viz_glPerspective(60, 16.0 / 9.0, 1, 1000);
    glMatrixMode(GL_MODELVIEW);

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
