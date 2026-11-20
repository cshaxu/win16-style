#ifndef STYLE_MSSTYLES_THEME_FILE_H
#define STYLE_MSSTYLES_THEME_FILE_H

#include "v4_tables.h"
#include <stdio.h>

enum {
    MSSTYLES_HOST_EXPLORER=1u, MSSTYLES_HOST_TERMINAL=2u, MSSTYLES_HOST_CONSOLE=4u,
    MSSTYLES_HOST_ALL=7u
};

typedef struct msstyles_host_options {
    unsigned enabled;
    bool declared;
} msstyles_host_options;

/* Writes a companion descriptor to a binary stream. The style file is a
 * sibling of the descriptor; stem is its file name without the extension.
 * This authoring profile uses ASCII labels and identifier tokens.
 * On I/O failure the caller discards the incomplete output. */
bool msstyles_theme_file_write(FILE *output,const char *display_name,const char *stem,
    const msstyles_v4_variant *variant,unsigned host_options);

enum { MSSTYLES_THEME_TOKEN_CAPACITY=64 };

typedef struct msstyles_theme_descriptor {
    char file[MSSTYLES_THEME_TOKEN_CAPACITY+sizeof(".msstyles")-1];
    char size[MSSTYLES_THEME_TOKEN_CAPACITY];
    char color[MSSTYLES_THEME_TOKEN_CAPACITY];
    char display_name[MSSTYLES_THEME_TOKEN_CAPACITY];
    msstyles_host_options hosts;
} msstyles_theme_descriptor;

/* Reads the packaged ASCII profile. The style file must be a simple sibling
 * name. The optional display name is a literal UTF-8 label. It is empty for
 * absent, indirect or ambiguous display metadata. A malformed or incomplete
 * visual style descriptor leaves the output unchanged. */
bool msstyles_theme_file_read(FILE *input,msstyles_theme_descriptor *descriptor);

#endif
