#ifndef WIN16_CONTROL_TEXT_POLICY_H
#define WIN16_CONTROL_TEXT_POLICY_H
#include <windows.h>
BOOL win16_control_report_wide_text(HDC,const WCHAR *,int,BOOL);
BOOL win16_control_report_ansi_text(HDC,const char *,int,BOOL);
BOOL win16_control_vb_glyph_prepare(HDC,LPCWSTR *,UINT *,UINT,WCHAR **,char **,BOOL (*)(HDC,const WCHAR *,UINT,WCHAR *));
HFONT win16_control_static_text_font(HDC,LPCWSTR,int,UINT,const void *,HWND,HMODULE);
#endif
