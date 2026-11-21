#ifndef STYLE_CLASSIC_PRESETS_H
#define STYLE_CLASSIC_PRESETS_H

#include "lib-win32-style/application/window/caption_slots.h"
#include "lib-win32-style/application/window/frame_border.h"
#include "lib-win32-style/application/menu/row_text.h"

#define WIN2000_TITLE_BUTTON_GROUP \
    {4,0,2,{{CLASSIC_BUTTON_MINIMIZE,CLASSIC_BUTTON_RIGHT,0}, \
            {CLASSIC_BUTTON_MAXIMIZE,CLASSIC_BUTTON_RIGHT,0}, \
            {CLASSIC_BUTTON_HELP,CLASSIC_BUTTON_RIGHT,0}, \
            {CLASSIC_BUTTON_CLOSE,CLASSIC_BUTTON_RIGHT,2}},CLASSIC_BUTTON_LAYOUT_EXPLICIT}

#endif
