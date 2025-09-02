#define NOB_STRIP_PREFIX
#define NOB_IMPLEMENTATION
#include "nob.h"

#define BUILD_FOLDER "build/"

#define streq(s1, s2) (strcmp((s1), (s2)) == 0) 

bool build(const char* src, const char* dst, bool rgfw, bool gcc_extensions) {
    Nob_Cmd cmd = {0};
    nob_cmd_append(&cmd,
                   "gcc",
                   "-Wall", "-Wextra",
                   "-Isrc",
                   "-ggdb",
                   "-o", dst,
                   src,
                   "-lm"
    );
    if (!gcc_extensions) nob_cmd_append(&cmd, "-std=c11");
    if (rgfw) nob_cmd_append(&cmd,
        "-lX11", "-lGL", "-lXrandr",
        "-Wno-unused-label",
        "-Wno-pointer-sign",
        "-Wno-int-to-pointer-cast",
        "-Wno-pointer-to-int-cast",
        "-Wno-unused-value"
    );

    return nob_cmd_run_sync_and_reset(&cmd);

} 

int main(int argc, char **argv) {
    NOB_GO_REBUILD_URSELF(argc, argv);

    if (!nob_mkdir_if_not_exists(BUILD_FOLDER)) return 1;

    const char *program_name = nob_shift_args(&argc, &argv);
    (void) program_name;

    bool run = false;
    Nob_File_Paths srcs = {0};
    Nob_File_Paths dsts = {0};
    bool rgfw = false;
    bool gcc_extensions = false;
    while (argc > 0) {
        const char *cmd_or_target = nob_shift_args(&argc, &argv);
        if (streq(cmd_or_target, "run")) {
            run = true;
            continue;
        }
        if (streq(cmd_or_target, "polyhedron")) {
            da_append(&srcs, "examples/openscad/polyhedron.c");
            da_append(&dsts, "./build/polyhedron");
        }
        else if (streq(cmd_or_target, "glasses")) {
            da_append(&srcs, "examples/glasses/glasses.c");
            da_append(&dsts, "./build/glasses");
            rgfw = true;
        }
        else if (streq(cmd_or_target, "serializer")) {
            mkdir_if_not_exists("./build/serializer/");
            da_append(&srcs, "examples/serializer/producer.c");
            da_append(&dsts, "./build/serializer/producer");

            da_append(&srcs, "examples/serializer/consumer.c");
            da_append(&dsts, "./build/serializer/consumer");

            gcc_extensions = true;
            rgfw = true;
        }
        else if (streq(cmd_or_target, "arduino_serializer")) {
            mkdir_if_not_exists("./build/serializer/");
            gcc_extensions = true;
            rgfw = true;
            bool ok = build(
                "examples/serializer/arduino_consumer.c",
                "./build/serializer/arduino_consumer",
                rgfw, gcc_extensions
            );
            if (!ok) return 1;
            Cmd cmd = {0};

            const char* sketch="examples/serializer/arduino_producer/arduino_producer.ino";
            cmd_append(&cmd, "arduino-cli", "compile", "-b", "arduino:avr:uno", sketch);
            bool arduino_compiled = cmd_run_sync_and_reset(&cmd);
            if (arduino_compiled) {
                cmd_append(&cmd, "arduino-cli", "upload", "-p", "/dev/ttyACM0", "-b", "arduino:avr:uno", sketch, "-t", "-v");
                cmd_run_sync_and_reset(&cmd);
            }

            cmd.count = 0;
            if (run) {
                cmd_append(&cmd, "./build/serializer/arduino_consumer");
                cmd_run_sync_and_reset(&cmd);
            }
            return 0;
        }
        else if (streq(cmd_or_target, "arduino_consumer")) {
            mkdir_if_not_exists("./build/serializer/");
            gcc_extensions = true;
            rgfw = true;
            bool ok = build(
                "examples/serializer/arduino_consumer.c",
                "./build/serializer/arduino_consumer",
                rgfw, gcc_extensions
            );
            if (!ok) return 1;
            Cmd cmd = {0};
            if (run) {
                cmd_append(&cmd, "./build/serializer/arduino_consumer");
                cmd_run_sync_and_reset(&cmd);
            }
            return 0;
        }
        else if (streq(cmd_or_target, "vertex_is_inside")) {
            da_append(&srcs, "examples/vertex_is_inside.c");
            da_append(&dsts, "./build/vertex_is_inside");
        }
        else if (streq(cmd_or_target, "2d_boolean")) {
            da_append(&srcs, "examples/2d_boolean.c");
            da_append(&dsts, "./build/2d_boolean");
        }
        else if (streq(cmd_or_target, "voxels")) {
            da_append(&srcs, "examples/voxels.c");
            da_append(&dsts, "./build/voxels");
        }
        else if (streq(cmd_or_target, "visualizer")) {
            da_append(&srcs, "examples/visualizer.c");
            da_append(&dsts, "./build/visualizer");
            rgfw = true;
        }
        else if (streq(cmd_or_target, "pipi")) {
            da_append(&srcs, "examples/pipi.c");
            da_append(&dsts, "./build/pipi");
            rgfw = true;
        }
        else if (streq(cmd_or_target, "vecops")) {
            da_append(&srcs, "examples/vecops.c");
            da_append(&dsts, "./build/vecops");
            rgfw = true;
        }
        else if (streq(cmd_or_target, "cutface")) {
            da_append(&srcs, "examples/cutface.c");
            da_append(&dsts, "./build/cutface");
            rgfw = true;
        }
        else if (streq(cmd_or_target, "cutface_border")) {
            da_append(&srcs, "examples/cutface_border.c");
            da_append(&dsts, "./build/cutface_border");
            rgfw = true;
        }
        else if (streq(cmd_or_target, "image_texture")) {
            da_append(&srcs, "examples/image_texture.c");
            da_append(&dsts, "./build/image_texture");
            rgfw = true;
        }
        else  {
            nob_log(NOB_ERROR, "Unknown cmd or target %s", cmd_or_target);
            return 1;
        }
    }
    assert(srcs.count = dsts.count);
    for (size_t i = 0; i < srcs.count; ++i) {
        if (!build(srcs.items[i], dsts.items[i], rgfw, gcc_extensions)) return 1;
    }

    
    if (run) {
        Nob_Procs procs = {0};
        for (size_t i = 0; i < dsts.count; ++i) {
            Nob_Cmd cmd = {0};
            nob_cmd_append(&cmd, dsts.items[i]);
            da_append(&procs, nob_cmd_run_async(cmd));
            cmd.count = 0;
        }
        nob_procs_wait(procs);
    }

    return 0;
}

