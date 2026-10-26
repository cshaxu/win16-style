#ifndef NTW16S_TEST_VB_QUEUE_H
#define NTW16S_TEST_VB_QUEUE_H
#include <windows.h>
#define VB_QUEUE_ENV L"WIN16STYLE_VB_QUEUE_MAP"
#define VB_QUEUE_MESSAGE L"Win16Style.Test.VBQueue.v1"
#define VB_QUEUE_SURVEY 1
#define VB_QUEUE_POPUP_SURVEY 2
#define VB_QUEUE_PAINT_SURVEY 3
#define VB_QUEUE_PRINT_SAMPLE 4
#define VB_QUEUE_GRAPHICAL_SURVEY 5
typedef struct vb_queue_state {
    DWORD size,pid,tid,generation;
    HWND root,window;
    volatile LONG completed,commands,hscroll,vscroll,acknowledgements,rejected;
    DWORD command_code,scroll_code;
    LONG action,attached,font_records,system_fonts,draw_items,popup_dc,system_text,native_text;
    HWND draw_dc_window;
    DWORD draw_type;
    LONG paint_messages,paint_system,paint_native,paint_probe_error,paint_restored;
    LONG paint_system_memory,paint_native_memory;
    ULONG_PTR paint_system_callsite;
    HFONT paint_system_font;
    LONG print_calls;DWORD print_alignment,print_api;
    LONG graphical_frames,graphical_blits,graphical_text;
    DWORD graphical_type,graphical_state;HWND graphical_dc_window;RECT graphical_rect;
    WCHAR event_name[128];
} vb_queue_state;
#endif
