#ifndef STYLE_PRINT_GENERAL_H
#define STYLE_PRINT_GENERAL_H
#define STYLE_PRINT_GENERAL 201
#define STYLE_PRINT_GENERAL_FULL 202
#define STYLE_PRINT_GENERAL_LARGE 203
#define STYLE_PRINT_DEVICE 1200
#define STYLE_PRINT_ALL 1201
#define STYLE_PRINT_SELECTION 1202
#define STYLE_PRINT_CURRENT 1203
#define STYLE_PRINT_PAGES 1204
#define STYLE_PRINT_RANGE 1205
#define STYLE_PRINT_COPIES 1206
#define STYLE_PRINT_COLLATE 1207
#define STYLE_PRINT_FILE 1208
#define STYLE_PRINT_PREFERENCES 1209
#define STYLE_PRINT_STATUS 1210
#define STYLE_PRINT_DEVICE_STATUS 1211
#define STYLE_PRINT_DEVICE_LOCATION 1212
#define STYLE_PRINT_DEVICE_COMMENT 1213
#define STYLE_PRINT_FIND_DEVICE 1214
#ifndef RC_INVOKED
#include <windows.h>
#include <commdlg.h>
#include "metrics.h"
/* On S_OK, transfer one module reference. The page releases it after modal use. */
typedef HRESULT (*style_print_general_theme)(HMODULE,HMODULE *,classic_dialog_metrics *);
BOOL style_print_general_a(HINSTANCE resources,PRINTDLGEXA *,HRESULT *,style_print_general_theme);
BOOL style_print_general_w(HINSTANCE resources,PRINTDLGEXW *,HRESULT *,style_print_general_theme);
#endif
#endif
