#define RGFW_IMPLEMENTATION
#include "../lib/RGFW.h"
#include "cadigo.h"
#include <math.h>
#define DEG2RAD 3.14/180.0



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
            float diffuse = (normal.z+1) / 4; // Lighting comming from the front + ambient light / dimming
            glBegin(GL_POLYGON);
                glColor3f(diffuse, diffuse, diffuse);

                for (size_t j = 0; j < face.count; ++j) {
                    Vec3 v = obj.points.items[face.items[j]];
                    glVertex3f(v.x, v.y, v.z);
                }
            glEnd();
        }

        RGFW_window_swapBuffers(win);
        RGFW_window_checkFPS(win, 60);
    }

close_and_return:

    RGFW_window_close(win);
    return 0;
}

