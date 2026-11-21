#ifndef STYLE_CONTROL_ROLES_H
#define STYLE_CONTROL_ROLES_H
#include <windows.h>

enum style_control_role {
    STYLE_ROLE_SEPARATOR=1u<<0, STYLE_ROLE_GROUP=1u<<1, STYLE_ROLE_PUSH=1u<<2,
    STYLE_ROLE_EDIT=1u<<3, STYLE_ROLE_PROGRESS_BORDER=1u<<4, STYLE_ROLE_TRACKBAR=1u<<5,
    STYLE_ROLE_HEADER=1u<<6, STYLE_ROLE_TAB=1u<<7, STYLE_ROLE_NATIVE_EDGE=1u<<8,
    STYLE_ROLE_UPDOWN=1u<<9
};

static inline unsigned style_control_button_paint_role(UINT type,DWORD style)
{
    if(type==BS_GROUPBOX)return STYLE_ROLE_GROUP;
    return type==BS_PUSHBUTTON || type==BS_DEFPUSHBUTTON || (style&BS_PUSHLIKE)?STYLE_ROLE_PUSH:0;
}
BOOL style_control_role_native(HWND,unsigned,BOOL (*)(HWND,const WCHAR *));
#endif
