#ifndef STYLE_MSSTYLES_THEME_FILE_H
#define STYLE_MSSTYLES_THEME_FILE_H

#include "v4_tables.h"
#include <stdio.h>

/* Writes a companion descriptor to a binary stream. The style file is a
 * sibling of the descriptor; stem is its file name without the extension.
 * This authoring profile uses ASCII labels and identifier tokens.
 * On I/O failure the caller discards the incomplete output. */
bool msstyles_theme_file_write(FILE *output,const char *display_name,const char *stem,
    const msstyles_v4_variant *variant);

#endif
