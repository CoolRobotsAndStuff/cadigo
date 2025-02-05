#include <unistd.h>

#define CADIGO_IMPLEMENTATION
#include "cadigo.h"

#include <stdio.h>

int main() {
    ASCII_Screen screen = alloc_ascii_screen();

    CAD big_s = cad_square(20);
    cad_rotate(&big_s, vec3(0, 0, 45));

    CAD smol_s = cad_square(10);
    cad_translate(&smol_s, vec3(-13, 0, 0));

    while (true) {
        cad_translate(&smol_s, vec3(1, 1, 0));
        cad_rotate(&smol_s, vec3(0, 0, 10));
        for (size_t i = 0; i < smol_s.points.count; ++i) {
            if (point_inside_face2D(smol_s.points.items[i], 0, big_s)){
                smol_s.points.items[i].color = CAD_RED;
            } else {
                smol_s.points.items[i].color = CAD_CYAN;
            }
            usleep(50000);
        }

        cad_clear_ascii_screen(&screen);

        cad_render_to_ascii_screen(&screen, 0.02, big_s);
        cad_render_to_ascii_screen(&screen, 0.02, smol_s);
        cad_print_ascii_screen(screen);
    }

    free_ascii_screen(screen);


}
