#define NOB_IMPLEMENTATION
#include "nob.h"

#define BUILD_FOLDER "build/"

int main(int argc, char **argv) {
    NOB_GO_REBUILD_URSELF(argc, argv);

    if (!nob_mkdir_if_not_exists(BUILD_FOLDER)) return 1;

    Nob_Cmd cmd = {0};
    nob_cmd_append(&cmd,
                   "gcc",
                   "-std=c11",
                   "-Wall", "-Wextra",
                   "-Isrc",
                   "-o", "build/cadigo_example",
                   "examples/openscad/example_polyhedron.c",
                   "-lm"
    );
    if (!nob_cmd_run_sync(cmd)) return 1;
    return 0;
}

