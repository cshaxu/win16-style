#include <stdio.h>
#include "lib-style/win16-window/frame_state.h"
#include "lib-style/win16-window/menu_bar.h"

int main(void)
{
    style_window *state=win16_control_frame_state_create();
    if(!state)return 10;
    if(win16_control_menu_bar_next(0,1,3,FALSE)!=1){
        win16_control_frame_state_dispose(state);
        return 11;
    }
    win16_control_frame_state_dispose(state);
    puts("WIN16_WINDOW_LINK_OK state=1 menu=1");
    return 0;
}
