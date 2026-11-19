#ifndef STYLE_MSSTYLES_V4_TABLES_H
#define STYLE_MSSTYLES_V4_TABLES_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct msstyles_v4_name {
    const unsigned char *bytes;
    uint32_t units; /* UTF-16LE code units including the terminator. */
} msstyles_v4_name;

/* A CMAP entry is a UTF-16LE name including its terminator, aligned to eight bytes. */
bool msstyles_v4_class_read(const unsigned char *bytes,size_t available,
    msstyles_v4_name *name,size_t *used);
bool msstyles_v4_class_write(unsigned char *bytes,size_t available,
    msstyles_v4_name name,size_t *used);

typedef struct msstyles_v4_variant {
    msstyles_v4_name record, size, color;
} msstyles_v4_variant;

/* Each read consumes one VMAP entry and borrows the three names from bytes. */
bool msstyles_v4_variant_read(const unsigned char *bytes,size_t available,
    msstyles_v4_variant *variant,size_t *used);

/* Destination must not overlap any of the source names. */
bool msstyles_v4_variant_write(unsigned char *bytes,size_t available,
    const msstyles_v4_variant *variant,size_t *used);

#endif
