#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_stdinc.h>
#include <stdbool.h>
#include <SDL2/SDL_render.h>
#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#include "cadigo1.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>

#define WINDOW_WIDTH 600
#define EPS 1

typedef struct {
    Vec3 position;
    Vec3 rotation;
    val zoom;
} Camera;


static Vec2 project_3d_2d(Vec3 v, Camera* camera) {
    Vec3 v1 = v;
    
    v1 = vec3_rotate_y(camera->rotation.y, v1);
    v1 = vec3_rotate_x(camera->rotation.x, v1);

    v1 = vec3_substract(v1, camera->position);

    return v2(v1.x/v1.z, v1.y/v1.z);
}
static Vec2 project_2d_scr(Vec2 v) {
    return v2((v.x + 1)/2*WINDOW_WIDTH/8 + WINDOW_WIDTH/2., (1 - (v.y + 1)/2)*WINDOW_WIDTH/8 + WINDOW_WIDTH/2.);
}

void render_object3D_points(SDL_Renderer* renderer, Object3D obj, Camera* cam, Uint32 color) {
    for (size_t i = 0; i < obj.points.count; ++i) {
        Vec3 p = obj.points.items[i];
        Vec2 p2 = project_3d_2d(p, cam);
        p2.x = p2.x * WINDOW_WIDTH + WINDOW_WIDTH / 2.;
        p2.y = p2.y * WINDOW_WIDTH + WINDOW_WIDTH / 2.;
        filledCircleColor(renderer, p2.x, p2.y, 3, color);
    }
}

void render_object3D_lines(SDL_Renderer* renderer, Object3D obj, Camera* cam) {
    Face face;
    size_t p1_i, p2_i;
    Vec2 p1, p2;
    size_t faces_i, face_i;
    for (faces_i = 0; faces_i < obj.faces.count; ++faces_i) {
        face = obj.faces.items[faces_i];
        for (face_i = 0; face_i < face.count; ++face_i) {
            p1_i = face.items[face_i];
            p2_i = face.items[(face_i + 1) % face.count];

            p1 = project_3d_2d(obj.points.items[p1_i], cam);
            p2 = project_3d_2d(obj.points.items[p2_i], cam);

            p2.x = p2.x * WINDOW_WIDTH + WINDOW_WIDTH / 2.;
            p2.y = p2.y * WINDOW_WIDTH + WINDOW_WIDTH / 2.;

            p1.x = p1.x * WINDOW_WIDTH + WINDOW_WIDTH / 2.;
            p1.y = p1.y * WINDOW_WIDTH + WINDOW_WIDTH / 2.;

            if (SDL_RenderDrawLine(renderer, p1.x, p1.y, p2.x, p2.y) < 0) {
                fprintf(stderr, "SDL ERROR, could not draw line.");
                exit(1);
            }
        }
    }
}

void render_object3D_faces(SDL_Renderer* renderer, Object3D obj, Camera* cam, Uint32 color) {
    Face face;
    size_t p1_i, p2_i;
    Vec2 p1, p2;
    size_t faces_i, face_i;
    for (faces_i = 0; faces_i < obj.faces.count; ++faces_i) {
        face = obj.faces.items[faces_i];
        Sint16* xs = malloc(sizeof(Sint16) * face.count);
        Sint16* ys = malloc(sizeof(Sint16) * face.count);

        for (face_i = 0; face_i < face.count; ++face_i) {
            p1_i = face.items[face_i];
            p1 = project_3d_2d(obj.points.items[p1_i], cam);

            p1.x = p1.x * WINDOW_WIDTH + WINDOW_WIDTH / 2.;
            p1.y = p1.y * WINDOW_WIDTH + WINDOW_WIDTH / 2.;

            xs[face_i] = (Sint16)p1.x;
            ys[face_i] = (Sint16)p1.y;
        }

        filledPolygonColor(renderer, xs, ys, (int)face.count, color);

        free(xs);
        free(ys);
    }
}

void print_edges(Object3D obj) {
    for (size_t i = 0; i < obj.faces.count; ++i) {
        Face face = obj.faces.items[i];
        for (size_t j = 0; j < face.count; ++j) {
            printf("%zu --- ", face.items[j]);
            printf("%zu\n", face.items[(j + 1) % face.count]);
        }
    }
}

#define NSPF 100000000 / 2

#define SENSITIVITY 500000000. / NSPF
#define SENSITIVITY_ROT 0.1

int main(void) {
    SDL_Event event;
    SDL_Renderer *renderer;
    SDL_Window *window;
    int i;

    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_WIDTH, 0, &window, &renderer);

    Camera cam;
    Camera other_cam;
    cam.position = v3(0, -10, -100);
    cam.rotation = v3(0, 0, 0);
    cam.zoom = 1;

    other_cam.position = v3(0, -10, -80); 
    other_cam.rotation = v3(0, 0, 0);
    other_cam.zoom = 1;

    bool shifting        = false;
    bool moving_forward  = false;
    bool moving_backward = false;
    bool moving_left     = false;
    bool moving_right    = false;
    bool moving_up       = false;
    bool moving_down     = false;

    bool other_shifting        = false;
    bool other_moving_forward  = false;
    bool other_moving_backward = false;
    bool other_moving_left     = false;
    bool other_moving_right    = false;
    bool other_moving_up       = false;
    bool other_moving_down     = false;

    bool loop = true; 

    Object3D c = obj_cube(10);

    //print_edges(c);
    //size_t new_index = add_vertex_to(&c, v3(3, 3, 3));

    while (loop) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT){
                loop = false;
                break;
            }

            if (event.type == SDL_KEYUP) {
                switch (event.key.keysym.sym) {
                    case SDLK_k: moving_up       = false; break;
                    case SDLK_j: moving_down     = false; break;
                    case SDLK_l: moving_left     = false; break;
                    case SDLK_h: moving_right    = false; break;
                    case SDLK_i: moving_forward  = false; break;
                    case SDLK_m: moving_backward = false; break;

                    case SDLK_a: other_moving_left     = false; break;
                    case SDLK_d: other_moving_right    = false; break;
                    case SDLK_w: other_moving_forward  = false; break;
                    case SDLK_s: other_moving_backward = false; break;

                    default:
                        printf("Key pressed: %s\n", SDL_GetKeyName(event.key.keysym.sym));
                        break;
                }
            }

            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_k: moving_up       = true; break;
                    case SDLK_j: moving_down     = true; break;
                    case SDLK_l: moving_left     = true; break;
                    case SDLK_h: moving_right    = true; break;
                    case SDLK_i: moving_forward  = true; break;
                    case SDLK_m: moving_backward = true; break;

                    case SDLK_a: other_moving_left     = true; break;
                    case SDLK_d: other_moving_right    = true; break;
                    case SDLK_w: other_moving_forward  = true; break;
                    case SDLK_s: other_moving_backward = true; break;
                    default:
                        printf("Key pressed: %s\n", SDL_GetKeyName(event.key.keysym.sym));
                        break;
                }
            }
        }
        
        if (!shifting){
            if (moving_up)       cam.position.y += SENSITIVITY;
            if (moving_down)     cam.position.y -= SENSITIVITY;
            if (moving_left)     cam.position.x -= SENSITIVITY;
            if (moving_right)    cam.position.x += SENSITIVITY;
            if (moving_forward)  cam.position.z -= SENSITIVITY;
            if (moving_backward) cam.position.z += SENSITIVITY;
        } else {
            if (moving_left)     cam.rotation.y += SENSITIVITY_ROT;
            if (moving_right)    cam.rotation.y -= SENSITIVITY_ROT;
            if (moving_up)       cam.rotation.x -= SENSITIVITY_ROT;
            if (moving_down)     cam.rotation.x += SENSITIVITY_ROT;
        }

        if (other_moving_up)       other_cam.position.y += SENSITIVITY;
        if (other_moving_down)     other_cam.position.y -= SENSITIVITY;
        if (other_moving_left)     other_cam.position.x += SENSITIVITY;
        if (other_moving_right)    other_cam.position.x -= SENSITIVITY;
        if (other_moving_forward)  other_cam.position.z -= SENSITIVITY;
        if (other_moving_backward) other_cam.position.z += SENSITIVITY;

        
        
        render_object3D_faces(renderer, c, &cam, 0xAA0000AA);
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        render_object3D_lines(renderer, c, &cam);
        render_object3D_points(renderer, c, &cam, 0xFF0000FF);

        render_object3D_faces(renderer, c, &other_cam, 0xAAAA0000);
        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
        render_object3D_lines(renderer, c, &other_cam);
        render_object3D_points(renderer, c, &other_cam, 0xFFFF0000);

        SDL_RenderPresent(renderer);

        struct timespec req;
        req.tv_sec = 0;
        req.tv_nsec = NSPF;
        nanosleep(&req, NULL);
    }


    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}
// int main() {
//     Object3D c = obj_cube(1 cm);
//     for (size_t i = 0; i < c.points.count; ++i) {
//         Vec3 p = c.points.items[i];
//         printf("point %zu %ld, %ld, %ld\n", i, as_mm(p.x), as_mm(p.y), as_mm(p.z));
//     }
// }
