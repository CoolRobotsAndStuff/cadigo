#include "extensions/openscad.h"

#define CADIGO_IMPLEMENTATION
#include "cadigo.h"

#include <stdio.h>

int main() {
    CAD c = cad_cube(30);
    cad_rotate(vec3(60, 0, 0), &c);

    while (true) {
        cad_rotate(vec3(0, 5, 0), &c);
        cad_render_to_terminal(0.02, c);
        usleep(200000);
    }
}
