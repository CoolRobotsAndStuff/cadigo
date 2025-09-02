#include "common.c"
#include "extensions/visualizer.h"

#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

typedef bool (*reader)(char*, size_t, void*);

bool cad_load(reader read, void* ctx, CAD* obj) {
    bool ok;
    uint16_t tmp_count;

    // points
    ok = read((char*)&tmp_count, sizeof(tmp_count), ctx);
    if (!ok) return false;
    obj->points.count = (size_t)tmp_count;

    da_grow_to_count(&obj->points);
    ok = read((char*)obj->points.items, da_items_size(obj->points), ctx);
    if (!ok) return false;

    // faces
    ok = read((char*)&tmp_count, sizeof(tmp_count), ctx);
    if (!ok) return false;
    obj->faces.count = (size_t)tmp_count;

    da_grow_to_count(&obj->faces);
    for (size_t i = 0; i < obj->faces.count; ++i) {
        Face* face = &obj->faces.items[i];
        ok = read((char*)&tmp_count, sizeof(tmp_count), ctx);
        face->count = (size_t)tmp_count;

        if (!ok) return false;
        da_grow_to_count(face);
        for (size_t j =0; j<face->count; ++j) {
            ok = read((char*)&tmp_count, sizeof(tmp_count), ctx);
            if (!ok) return false;
            face->items[j] = (size_t)tmp_count;
        }
    }

    // color
    ok = read((char*)&obj->color, sizeof(Vec3), ctx);
    if (!ok) return false;

    return true;
}

bool cads_load(reader read, void* ctx, CADs* cads) {
    uint16_t tmp_count;
    if (!read((char*)&tmp_count, sizeof(tmp_count), ctx)) return false;
    //printf("Loading cads with count %"PRIu16"\n", tmp_count);
    cads->count = (size_t)tmp_count;
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


bool read_serial(char* data, size_t size, void* fd) {
    ssize_t rr = 0;
    again:
    static char buf[1024] = {0};  
    //int nbytes;
    //ioctl((int)fifo, FIONREAD, &nbytes);
    ssize_t r = read((int)fd, buf, size-rr);
    //printf("tried to read %zu bytes into %p and got %zd. Avaliable bytes were: %d\n", size, buf, r, nbytes);
    // if (r == 8) {
    //     printf("read count=%zu\n", *(size_t*)buf);
    //
    // }
    rr += r;
    //printf("reading serial\n");
    memcpy(data, buf, r);
    //assert(r == (ssize_t)size || r <= 0);
    if (rr >= (ssize_t)size) {
        //printf("done reading\n");
        return true;
    } else if (r <= 0) {
        return false;
    } else {
        data += r;
        goto again;
    }
}

CADs objs = {0};

int main(void) {
    const char *dev = "/dev/ttyACM0";
    int baud = B576000;

    int fd;
    do {
        fd = open(dev, O_RDWR | O_NOCTTY);
        usleep(1);
    } while (fd < 0);

    struct termios t;
    if (tcgetattr(fd, &t) != 0) goto on_error;
    cfmakeraw(&t);
    cfsetispeed(&t, baud);
    cfsetospeed(&t, baud);
    if (tcsetattr(fd, TCSANOW, &t) != 0) goto on_error;

    CAD_Viz* viz = cad_viz_init();
    while (cad_viz_keep_rendenring(viz)) {
        cad_viz_begin(viz);
            int nbytes;
            ioctl(fd, FIONREAD, &nbytes);
            if (nbytes > 0) {
                bool ok = cads_load(read_serial, (void*)fd, &objs);
                if (!ok) {
                    perror("cads_load");
                    return 1;
                }
            }
            da_foreach(CAD*, obj, &objs) cad_viz_render(viz, **obj);
        cad_viz_end(viz);
        usleep(1000);
    }

    close(fd);
    return 0;

    on_error:
        close(fd);
        return 1;
}
