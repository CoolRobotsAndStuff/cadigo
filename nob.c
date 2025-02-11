#define NOB_IMPLEMENTATION
#include "nob.h"

#define BUILD_FOLDER "build/"

#define streq(s1, s2) (strcmp((s1), (s2)) == 0) 

bool build(const char* src, const char* dst) {
    Nob_Cmd cmd = {0};
    nob_cmd_append(&cmd,
                   "gcc",
                   "-std=c11",
                   "-Wall", "-Wextra",
                   "-Isrc",
                   "-o", dst,
                   src,
                   "-lm"
    );
    return nob_cmd_run_sync_and_reset(&cmd);

} 

int main(int argc, char **argv) {
    NOB_GO_REBUILD_URSELF(argc, argv);

    if (!nob_mkdir_if_not_exists(BUILD_FOLDER)) return 1;

    const char *program_name = nob_shift_args(&argc, &argv);
    (void) program_name;

    bool run = false;
    char* src = "";
    char* dst = "";
    while (argc > 0) {
        const char *cmd_or_target = nob_shift_args(&argc, &argv);
        if (streq(cmd_or_target, "run")) {
            run = true;
            continue;
        }
        if (streq(cmd_or_target, "polyhedron")) {
            src = "examples/openscad/polyhedron.c";
            dst = "./build/polyhedron";
        }
        else if (streq(cmd_or_target, "glasses")) {
            src = "examples/glasses/glasses.c";
            dst = "./build/glasses";
        }
        else if (streq(cmd_or_target, "vertex_is_inside")) {
            src = "examples/vertex_is_inside.c";
            dst = "./build/vertex_is_inside";
        }
        else if (streq(cmd_or_target, "2d_boolean")) {
            src = "examples/2d_boolean.c";
            dst = "./build/2d_boolean";
        }
        else  {
            nob_log(NOB_ERROR, "Unknown cmd or target %s", cmd_or_target);
            return 1;
        }
    }
    if (!build(src, dst)) return 1;

    
    if (run) {
        Nob_Cmd cmd = {0};
        nob_cmd_append(&cmd, dst);
        if (!nob_cmd_run_sync_and_reset(&cmd)) return 1;
    }

    return 0;
}

