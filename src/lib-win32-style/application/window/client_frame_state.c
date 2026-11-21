#include "client_frame_state.h"
#include <stdlib.h>
style_client_frame *style_client_frame_state_create(void)
{style_client_frame *s=calloc(1,sizeof(*s));if(s)style_instance_init(&s->instance,s,style_client_frame_state_dispose);return s;}
void style_client_frame_state_dispose(void *context)
{
    style_client_frame *s=context;
    style_menu_owner_stop(&s->menu);
    style_font_pair_dispose(&s->fonts,style_frame_fonts_release);
    free(s);
}
