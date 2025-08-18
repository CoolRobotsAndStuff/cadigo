#define CADIGO_IMPLEMENTATION
#include "cadigo.h"

int main(void) {
    Vec3 v = vec3(2, 2, 2);

    printf("v: ");
    vec3_print(v);
    puts("");

    vec3_mult_by(&v, vec3(3, 5, 4));
    printf("v: ");
    vec3_print(v);
    puts("");

    return 0;
}
