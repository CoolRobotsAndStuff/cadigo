#include <unistd.h>
#define CADIGO_IMPLEMENTATION
#include "cadigo.h"
#include <stdio.h>

int main() {
    ASCII_Screen screen = alloc_ascii_screen();

    CAD* big_s = cad_square(20);
    cad_rotate(big_s, vec3(0, 0, 45));

    CAD* smol_s = cad_square(10);
    cad_rotate(smol_s, vec3(0, 0, 45));
    cad_translate(smol_s, vec3(13, 0, 0));

    cad_substract(smol_s, big_s);

    cad_clear_ascii_screen(&screen);

    //cad_render_to_ascii_screen(&screen, 0.02, big_s);
    cad_render_to_ascii_screen(&screen, 0.02, *smol_s);
    cad_print_ascii_screen(screen);

    free_ascii_screen(screen);


}
