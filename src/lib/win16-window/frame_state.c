#include "frame_state.h"
#include "../win16-common/font.h"
#include <stdlib.h>
style_window *win16_control_frame_state_create(void)
{return (style_window *)calloc(1,sizeof(style_window));}
void win16_control_frame_state_enter_menu(style_window *s)
{
    ++s->menu_depth;
    menu_loop *loop=calloc(1,sizeof(*loop));
    if(loop){loop->scope=win16_style_menu_begin(&s->palette);
        if(loop->scope){loop->depth=s->menu_depth;loop->previous=s->menus;s->menus=loop;}
        else free(loop);}
}
void win16_control_frame_state_exit_menu(style_window *s)
{
    win16_style_menu_restore_depth(s->menu_depth,&s->menu_items);
    /* A failed inner entry must not pop the outer successful scope. */
    if(s->menus && s->menus->depth==s->menu_depth)win16_control_frame_state_end_menu(s);
    if(s->menu_depth)--s->menu_depth;
}
void win16_control_frame_state_end_menu(style_window *s)
{
    menu_loop *loop=s->menus;
    if(loop){s->menus=loop->previous;win16_style_menu_end(loop->scope);free(loop);}
}
void win16_control_frame_state_dispose(style_window *s)
{
    while(s->menus)win16_control_frame_state_end_menu(s);
    win16_style_menu_restore(&s->menu_items);
    if(s->title_font)DeleteObject(s->title_font);
    win16_style_system_release(s->menu_font);
    free(s);
}
