#ifndef WIN16_CONTROL_MONTH_TEXT_H
#define WIN16_CONTROL_MONTH_TEXT_H
#include <windows.h>
BOOL ntw16s_month_text_length(LPCWSTR text,int count,int *length);
BOOL ntw16s_month_title_fits(const RECT *r,const RECT *prev,const RECT *next,int width);
#endif
