#ifndef STYLE_UWP_INPUT_H
#define STYLE_UWP_INPUT_H
#include "frame.h"
/* Caller pins the attached frame across native and provider reentry. */
BOOL style_uwp_input_message(style_uwp_frame *,UINT,WPARAM,LPARAM,LRESULT *);
#endif
