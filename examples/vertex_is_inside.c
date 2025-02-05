#include "extensions/openscad.h"

#define CADIGO_IMPLEMENTATION
#include "cadigo.h"

#include <stdio.h>

int main() {
    ASCII_Screen screen = alloc_ascii_screen();

    CAD big_s = cad_square(20);
    cad_rotate(vec3(0, 0, 45), &big_s);

    CAD smol_s = cad_square(10);
    cad_translate(vec3(-13, 0, 0), &smol_s);

    while (true) {
        cad_translate(vec3(1, 1, 0), &smol_s);
        cad_rotate(vec3(0, 0, 10), &smol_s);
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
