#ifndef STYLE_MSSTYLES_V3_FRAME_H
#define STYLE_MSSTYLES_V3_FRAME_H

#include "classic_theme.h"

/* Projects the standard v3 caption contract into the shared frame model. */
BOOL msstyles_v3_frame_model(const msstyles_v3_theme *theme,classic_theme *model);
/* Borrows bitmap data from the same mapping as the v3 property view. */
BOOL msstyles_v3_push_image(const msstyles_v3_theme *theme,style_push_sprite *sprite);
BOOL msstyles_v3_frame_images(const msstyles_v3_theme *theme,style_frame_sprites *sprites);

#endif