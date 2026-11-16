#include "frame_state.h"
#include "frame_font.h"
#include "../common/font.h"
#include <stdlib.h>
style_window *win16_control_frame_state_create(void)
{style_window *s=calloc(1,sizeof(*s));if(s)style_instance_init(&s->instance,s,win16_control_frame_state_dispose);return s;}
void win16_control_frame_state_dispose(void *context)
{
    style_window *s=context;
    style_menu_owner_stop(&s->menu);
    style_font_pair_dispose(&s->fonts,win16_control_release_frame_fonts);
    free(s);
}
