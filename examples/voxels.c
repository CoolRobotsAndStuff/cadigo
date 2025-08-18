#include <unistd.h>
#include <stdio.h>

#include "extensions/vox.h"
#define CADIGO_IMPLEMENTATION
#include "cadigo.h"

int main() {
    ASCII_Screen screen = alloc_ascii_screen();

    Vox a = vox_cube(vec3i(20, 20, 2));
    vox_translate(&a, vec3i(3, 6, 0));
    
    Vox b = vox(
        rotation_xyz   (10, 10, 10,
        translation_xyz(10, 10, 10, 
            cube(10)
    )));

    CAD t = cube(10);
    rotate_xyz   (&t, 10, 10, 10)
    translate_xyz(&t, 10, 10, 10)
    Vox b = vox(t)

    vox_set(&b, vec3i(5, 5, 0), false);
    vox_set(&b, vec3i(5, 5, 1), false);

    Vox c = vox_union(a, b);

    Vox d = vox_cube(vec3i(3, 10, 2));
    vox_translate(&d, vec3i(8, 8, 0));

    c = vox_difference(c, d);

    cad_clear_ascii_screen(&screen);

    vox_render_to_ascii_screen(&screen, c);
    cad_print_ascii_screen(screen);

    free_ascii_screen(screen);
    return 0;
}
