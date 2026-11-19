#ifndef STYLE_MSSTYLES_V4_PROPERTY_H
#define STYLE_MSSTYLES_V4_PROPERTY_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct msstyles_v4_property {
    uint32_t name_id, type_id, class_id, part_id, state_id;
    uint32_t short_value, declared_size;
    const unsigned char *data;
} msstyles_v4_property;

/* Read borrows data from bytes. Short records carry no trailing data, even
 * when their declared size is nonzero. Failure leaves the outputs untouched. */
bool msstyles_v4_property_read(const unsigned char *bytes,size_t available,
    size_t class_count,msstyles_v4_property *property,size_t *used);

/* The source data and destination must not overlap. */
bool msstyles_v4_property_write(unsigned char *bytes,size_t available,
    size_t class_count,const msstyles_v4_property *property,size_t *used);

#endif
