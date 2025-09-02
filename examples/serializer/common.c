#define CADIGO_IMPLEMENTATION
#define CADIGO_CUSTOM_TYPE
typedef float val_t;
#define VAL_FMT "f"
#include "cadigo.h"

#define da_items_size(da) ( sizeof(*((da).items))*(da).count )
#define da_foreach(Type, it, da) for (Type *it = (da)->items; it < (da)->items + (da)->count; ++it)

#define da_grow_to_count(da) (                               \
    ((da)->items == NULL) ? (                                 \
        (da)->capacity = (da)->count,                          \
        (da)->items = malloc(da_items_size(*(da))),            \
        memset((da)->items, 0, da_items_size(*(da)))            \
    ) : ((da)->count > (da)->capacity) ? (                       \
        (da)->capacity = (da)->count,                             \
        (da)->items = realloc((da)->items, da_items_size(*(da)))   \
    ) : (                                                           \
        (da)->items                                                  \
    )                                                                 \
)

#define FIFO_NAME "./build/serializer/fifo"
