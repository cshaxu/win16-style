#ifndef NTW16S_COMPAT_MENU_H
#define NTW16S_COMPAT_MENU_H
#include "lib-win32-style/appearance/win3x/common/control_api.h"
#include "lib-win32-style/appearance/win3x/common/palette.h"
#include "lib-win32-style/application/menu/row_paint.h"

typedef style_menu_kind win16_style_menu_kind;
#define WIN16_STYLE_MENU_ROW STYLE_MENU_ROW
#define WIN16_STYLE_MENU_SEPARATOR STYLE_MENU_SEPARATOR
#define WIN16_STYLE_MENU_BAR STYLE_MENU_BAR

NTW16S_API BOOL win16_style_menu_text(HDC,const RECT *,HFONT,UINT,
    const win16_style_palette *,const WCHAR *,UINT,win16_style_menu_kind,BOOL);
NTW16S_API BOOL win16_style_menu_measure(HDC,HFONT,UINT,const WCHAR *,win16_style_menu_kind,SIZE *);
#endif
