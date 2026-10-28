#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <uxtheme.h>
__declspec(dllexport) HRESULT WINAPI FixtureDraw(HTHEME theme,HDC dc,int part,int state,const RECT *r,const RECT *clip){return DrawThemeBackground(theme,dc,part,state,r,clip);}
