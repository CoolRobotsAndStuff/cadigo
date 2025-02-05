#define NOB_IMPLEMENTATION
#include "nob.h"

#define BUILD_FOLDER "build/"

int main(int argc, char **argv) {
    NOB_GO_REBUILD_URSELF(argc, argv);

    if (!nob_mkdir_if_not_exists(BUILD_FOLDER)) return 1;

    Nob_Cmd cmd = {0};
    // nob_cmd_append(&cmd,
    //                "gcc",
    //                "-std=c11",
    //                "-Wall", "-Wextra",
    //                "-Isrc",
    //                "-o", "build/cadigo_example",
    //                "examples/openscad/example_polyhedron.c",
    //                "-lm"
    // );
    // if (!nob_cmd_run_sync_and_reset(&cmd)) return 1;
    //
    // nob_cmd_append(&cmd,
    //                "gcc",
    //                "-std=c11",
    //                "-Wall", "-Wextra",
    //                "-Isrc",
    //                "-o", "build/glasses_example",
    //                "examples/glasses/glasses_shape.c",
    //                "-lm"
    // );
    // if (!nob_cmd_run_sync_and_reset(&cmd)) return 1;

    nob_cmd_append(&cmd,
                   "gcc",
                   "-std=c11",
                   "-Wall", "-Wextra",
                   "-Isrc",
                   "-o", "build/vertex_is_inside",
                   "examples/vertex_is_inside.c",
                   "-lm"
    );
    if (!nob_cmd_run_sync_and_reset(&cmd)) return 1;

    // nob_cmd_append(&cmd, "./build/glasses_example");
    // if (!nob_cmd_run_sync_and_reset(&cmd)) return 1;
    //
    nob_cmd_append(&cmd, "./build/vertex_is_inside");
    if (!nob_cmd_run_sync_and_reset(&cmd)) return 1;

    return 0;
}

