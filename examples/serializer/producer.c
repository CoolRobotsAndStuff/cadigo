#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include "common.c"

typedef bool (*writer)(char*, size_t, void*);

#define da_dump_size(da) ( (sizeof((da).count) + sizeof(*((da).items)) * ((da).count)) )

bool cad_dump(writer write, void* ctx, CAD obj) {
    bool ok;
    ok = write((char*)&obj.points.count, sizeof(obj.points.count), ctx);
    if (!ok) return false;
    ok = write((char*)obj.points.items, da_items_size(obj.points), ctx);
    if (!ok) return false;


    ok = write((char*)&obj.faces.count, sizeof(obj.faces.count), ctx);
    if (!ok) return false;
    for (size_t i = 0; i < obj.faces.count; ++i) {
        Face face = obj.faces.items[i]; 
        ok = write((char*)&(face.count), sizeof(face.count), ctx);
        if (!ok) return false;
        ok = write((char*)face.items, da_items_size(face), ctx);
        if (!ok) return false;
    }

    ok = write((char*)&obj.color, sizeof(Vec3), ctx);
    if (!ok) return false;
    return true;
}

bool cads_dump(writer write, void* ctx, CADs cads) {
    bool ok;
    ok = write((char*)&cads.count, sizeof(cads.count), ctx);
    if (!ok) return false;
    for (size_t i = 0; i < cads.count; ++i) {
        if (!cad_dump(write, ctx, *cads.items[i])) return false;
    }
    return true;
}

bool write_fifo(char* data, size_t size, void* fifo) {
    ssize_t w = write((int)fifo, data, size);
    //printf("tried to write %zu bytes from %p and wrote %zd\n", size, data, w);
    if (w <= 0) {
        perror("fifo");
        return false;
    }
    return true;
}

CADs objs = {0};

int main() {
    unlink(FIFO_NAME);
    mkfifo(FIFO_NAME, 0666);
    int fifo = open(FIFO_NAME, O_WRONLY);
    if (!fifo) return 1;

    CAD* orig_shape = cad_cube(2);
    da_append(&objs, orig_shape);
    cad_repeat(3) cad_catmull_clark(orig_shape);
    orig_shape->color = vec3(1, 0, 0);

    CAD* other_shape = cad_cube(1);
    cad_translate_x(other_shape, 2);
    da_append(&objs, other_shape);
    while (true) {
        int nbytes; 
        ioctl(fifo, FIONREAD, &nbytes);
        if (nbytes == 0) {
            bool ok = cads_dump(write_fifo, (void*)fifo, objs);
            if (!ok) return 1;
        }
        cad_rotate_y(orig_shape, 1);
        cad_rotate_y(other_shape, -2);
        usleep(10*1000);
    }
    close(fifo);
    unlink(FIFO_NAME);

    return 0;
}
