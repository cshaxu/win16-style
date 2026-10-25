#ifndef NTW16S_CONTROL_PROTOCOL_H
#define NTW16S_CONTROL_PROTOCOL_H
#include <windows.h>
/* A verified role belongs to one newly created HWND on its GUI thread. */
typedef struct ntw16s_control_protocol ntw16s_control_protocol;
ntw16s_control_protocol *ntw16s_control_protocol_bind(HWND);
ntw16s_control_protocol *ntw16s_control_protocol_acquire(HWND);
BOOL ntw16s_control_protocol_is(HWND,const WCHAR *);
void ntw16s_control_protocol_release(ntw16s_control_protocol *);
#endif
