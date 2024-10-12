#include <stdbool.h>
#include <SDL2/SDL_render.h>
#include <time.h>
#include "cadigo1.h"
#include <SDL2/SDL.h>

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
    cam.position = v3(0, -10, -100);
    cam.rotation = v3(0, 0, 0);
    cam.zoom = 1;

    bool shifting        = false;
    bool moving_forward  = false;
    bool moving_backward = false;
    bool moving_left     = false;
    bool moving_right    = false;
    bool moving_up       = false;
    bool moving_down     = false;

    bool loop = true; 

    while (loop) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT){
                loop = false;
                break;
            }

            if (event.type == SDL_KEYUP) {
                switch (event.key.keysym.sym) {
                    case SDLK_LSHIFT:
                    case SDLK_RSHIFT: shifting   = false; break;
                    case SDLK_k: moving_up       = false; break;
                    case SDLK_j: moving_down     = false; break;
                    case SDLK_l: moving_left     = false; break;
                    case SDLK_h: moving_right    = false; break;
                    case SDLK_i: moving_forward  = false; break;
                    case SDLK_m: moving_backward = false; break;
                    default:
                        printf("Key pressed: %s\n", SDL_GetKeyName(event.key.keysym.sym));
                        break;
                }
            }

            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_LSHIFT:
                    case SDLK_RSHIFT: shifting   = true; break;
                    case SDLK_k: moving_up       = true; break;
                    case SDLK_j: moving_down     = true; break;
                    case SDLK_l: moving_left     = true; break;
                    case SDLK_h: moving_right    = true; break;
                    case SDLK_i: moving_forward  = true; break;
                    case SDLK_m: moving_backward = true; break;
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

        Object3D c = obj_cube(10);
        for (size_t i = 0; i < c.points.count; ++i) {
            Vec3 p = c.points.items[i];
            //p = vec3_add(v3(x, 10, 100), p);
            //printf("point %zu:"VAL_FMT", "VAL_FMT", "VAL_FMT"\n", i, as_mm(p.x), as_mm(p.y), as_mm(p.z));

            Vec2 p2 = project_3d_2d(p, &cam);
            p2.x = p2.x * WINDOW_WIDTH + WINDOW_WIDTH / 2.;
            p2.y = p2.y * WINDOW_WIDTH + WINDOW_WIDTH / 2.;


            //printf("point2d %zu: "VAL_FMT", "VAL_FMT"\n", i, p2.x, p2.y);
            SDL_RenderDrawPoint(renderer, p2.x, p2.y);
        }

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
