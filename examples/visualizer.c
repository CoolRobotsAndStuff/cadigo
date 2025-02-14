#include "extensions/visualizer.h"

#define CADIGO_IMPLEMENTATION
#include "cadigo.h"

int main() {
    CAD c = cad_cube(10);
    cad_catmull_clark(
    cad_catmull_clark(
    cad_catmull_clark(
    cad_catmull_clark(
            &c
    ))));
    return cad_visualize(c);
}
