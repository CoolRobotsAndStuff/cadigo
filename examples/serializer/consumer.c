#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "common.c"
#include "extensions/visualizer.h"

typedef bool (*reader)(char*, size_t, void*);

bool cad_load(reader read, void* ctx, CAD* obj) {
    bool ok;

    // points
    ok = read((char*)&obj->points.count, sizeof(&obj->points.count), ctx);
    if (!ok) return false;
    da_grow_to_count(&obj->points);
    ok = read((char*)obj->points.items, da_items_size(obj->points), ctx);
    if (!ok) return false;

    // faces
    ok = read((char*)&obj->faces.count, sizeof(obj->faces.count), ctx);
    if (!ok) return false;
    da_grow_to_count(&obj->faces);
    for (size_t i = 0; i < obj->faces.count; ++i) {
        Face* face = &obj->faces.items[i];
        ok = read((char*)&face->count, sizeof(face->count), ctx);
        if (!ok) return false;
        da_grow_to_count(face);
        ok = read((char*)face->items, da_items_size(*face), ctx);
        if (!ok) return false;
    }

    // color
    ok = read((char*)&obj->color, sizeof(Vec3), ctx);
    if (!ok) return false;

    return true;
}

bool cads_load(reader read, void* ctx, CADs* cads) {
    if (!read((char*)&cads->count, sizeof(cads->count), ctx)) return false;
    if (cads->items == NULL) {
        cads->capacity = cads->count;
        cads->items = malloc(da_items_size(*(cads)));
        da_foreach(CAD*, c, cads) *c = cad_alloc(); 
    } else if (cads->count > cads->capacity) {
        cads->capacity = cads->count;
        cads->items = realloc(cads->items, da_items_size(*cads));
    }
    for (size_t i = 0; i < cads->count; ++i) {
        if (!cad_load(read, ctx, cads->items[i])) return false;
    }
    return true;
}


bool read_fifo(char* data, size_t size, void* fifo) {
    again:
    static char buf[1024] = {0};  
    //int nbytes;
    //ioctl((int)fifo, FIONREAD, &nbytes);
    ssize_t r = read((int)fifo, buf, size);
    //printf("tried to read %zu bytes into %p and got %zd. Avaliable bytes were: %d\n", size, buf, r, nbytes);
    // if (r == 8) {
    //     printf("read count=%zu\n", *(size_t*)buf);
    // }
    memcpy(data, buf, r);
    assert(r == (ssize_t)size || r <= 0);
    if (r == (ssize_t)size) {
        return true;
    } else if (r <= 0) {
        return false;
    } else {
        data += r;
        goto again;
    }
}

CADs objs = {0};

int main() {
    sleep(2);
    CAD* new_shape = cad_alloc();
    int fifo;
    for (;;) {
        puts("uwu");
        fifo = open(FIFO_NAME, O_RDONLY);
        if (fifo) break;
        else usleep(100);
    }

    CAD_Viz* viz = cad_viz_init();
    while (cad_viz_keep_rendenring(viz)) {
        cad_viz_begin(viz);
            int nbytes;
            ioctl(fifo, FIONREAD, &nbytes);
            if (nbytes > 0) {
                if (!cads_load(read_fifo, (void*)fifo, &objs)) {
                    if (errno == EAGAIN) {
                        continue;
                    } else {
                        perror("cad_load");
                        return 1;
                    }
                }
            }
            da_foreach(CAD*, obj, &objs) cad_viz_render(viz, **obj);
        cad_viz_end(viz);
        usleep(1000);
    }

    close(fifo);
    unlink(FIFO_NAME);

    return 0;
}
