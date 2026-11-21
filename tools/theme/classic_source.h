#ifndef CLASSIC_THEME_SOURCE_H
#define CLASSIC_THEME_SOURCE_H

#include <windows.h>
#include <stdbool.h>
#include <stdint.h>
#include "lib-win32-style/appearance/msstyles/classic_presets.h"
#include "lib-win32-style/appearance/msstyles/classic_style.h"
#include "lib-win32-style/appearance/msstyles/classic_theme.h"

typedef struct classic_caption_source {
    uint32_t start, end, text;
} classic_caption_source;

typedef struct classic_theme_source {
    const char *stem, *label;
    classic_caption_source active, inactive;
    uint32_t face, edge, bevel_light;
    bool gradient, legacy_caption_images, own_sizebox;
    classic_button_group buttons;
    classic_caption_metrics caption_metrics;
    classic_scroll_metrics scroll_metrics;
    const char *fonts[3]; /* caption, small caption, menu */
    classic_menu_colors menu;
    classic_menu_metrics menu_metrics;
    COLORREF choice_face,choice_pressed_face,choice_ink,choice_muted_ink;
    COLORREF group_disabled_text;
    COLORREF selection,selection_text;
    COLORREF selected_tab;
    COLORREF progress_trough;
    bool own_progress_states;
    classic_dialog_metrics dialog_metrics;
    classic_dialog_policy dialog_policy;
    unsigned host_options;
    style_caption_presentation caption_presentation;
    style_border_presentation border_presentation;
    classic_control_presentation controls;
    style_scroll_arrow_source arrow_source;
    COLORREF pressed_toolbar;
    unsigned calendar_font_dpi,calendar_font_points;
} classic_theme_source;

#define WIN16_POPUP_MENU_METRICS {18,8,12,4,STYLE_MENU_TEXT_DEFINED|STYLE_MENU_TEXT_STIPPLE_DISABLED,6, \
    CLASSIC_MDI_DEFINED|CLASSIC_MDI_SYSTEM|CLASSIC_MDI_RESTORE,14,1, \
    CLASSIC_MDI_DEFINED|CLASSIC_MDI_MINIMIZE|CLASSIC_MDI_CLOSE,STYLE_MENU_FONT_SYSTEM_BITMAP}
#define WIN2000_POPUP_MENU_METRICS \
    {22,0,16,16,STYLE_MENU_TEXT_DEFINED|STYLE_MENU_TEXT_SHORTCUT_RIGHT| \
        STYLE_MENU_TEXT_BOLD_DEFAULT|STYLE_MENU_TEXT_CENTER_BAR|STYLE_MENU_COMMAND_GLYPHS,0,0,0,0,0,STYLE_MENU_FONT_LOGICAL}

#define WIN16_TITLE_BUTTON_GROUP \
    {3,0,0,{{CLASSIC_BUTTON_SYSTEM,CLASSIC_BUTTON_LEFT,0}, \
            {CLASSIC_BUTTON_MINIMIZE,CLASSIC_BUTTON_RIGHT,0}, \
            {CLASSIC_BUTTON_MAXIMIZE,CLASSIC_BUTTON_RIGHT,0}}, \
     CLASSIC_BUTTON_LAYOUT_EXPLICIT|CLASSIC_BUTTON_LAYOUT_CLIP|CLASSIC_BUTTON_LAYOUT_FILL}

#define WIN16_FRAME_GEOMETRY {STYLE_FRAME_LOGICAL,4,3,6,1,20,1,1}
#define WIN16_FRAME_BORDER \
    {STYLE_BORDER_CLIPPED,STYLE_BORDER_TITLE,{STYLE_BORDER_OUTER,STYLE_BORDER_EDGE,STYLE_BORDER_EDGE}, \
     {STYLE_BORDER_INNER,STYLE_BORDER_EDGE,STYLE_BORDER_EDGE},STYLE_BORDER_LIGHT,STYLE_BORDER_EDGE,WIN16_FRAME_GEOMETRY}

#define WIN2000_FRAME_GEOMETRY {STYLE_FRAME_MONITOR_PIXELS,4,3,3,1,0,0,0}
#define WIN2000_FRAME_BORDER \
    {STYLE_BORDER_PERIMETER,STYLE_BORDER_FACE,{STYLE_BORDER_OUTER,STYLE_BORDER_FACE,STYLE_BORDER_EDGE}, \
     {STYLE_BORDER_NEXT,STYLE_BORDER_LIGHT,STYLE_BORDER_SHADOW},STYLE_BORDER_KEEP,STYLE_BORDER_KEEP,WIN2000_FRAME_GEOMETRY}

#define WIN2000_DIALOG_METRICS {{427,272,565,345,370,197,7,20},500,450,490,298,184,236,74,62,287,196}

static const classic_theme_source classic_sources[]={
    {
        "Win16","Windows 3.x",
        {RGB(164,200,240),RGB(164,200,240),RGB(0,0,0)},
        {RGB(255,255,255),RGB(255,255,255),RGB(0,0,0)},
        RGB(192,192,192),RGB(0,0,0),RGB(255,255,255),false,true,false,
        WIN16_TITLE_BUTTON_GROUP,
        {18,0,{19,18},{0,0}},
        {CLASSIC_SCROLL_EXTENT_CROSS_AXIS,0,STYLE_SCROLL_THUMB_FIXED,200,50,{STYLE_SCROLL_DRAG_LOGICAL,4,1}},
        {"MS Sans Serif, 10, Bold, Quality:NonAntialiased",NULL,"System, 12, Bold, Quality:NonAntialiased"},
        {RGB(255,255,255),RGB(164,200,240),RGB(0,0,0),RGB(0,0,0),RGB(0,0,0),RGB(0,0,0),TRUE},
        WIN16_POPUP_MENU_METRICS,
        RGB(255,255,255),RGB(255,255,255),RGB(0,0,0),RGB(0,0,0),
        RGB(0,0,0),
        RGB(164,200,240),RGB(0,0,0),
        RGB(255,255,255),
        RGB(255,255,255),true,
        WIN2000_DIALOG_METRICS,
        {CLASSIC_WINDOW_PROVIDER_FULL_CLIENT,CLASSIC_FILE_LEGACY_LAYOUT,CLASSIC_FOLDER_LEGACY_TREE,CLASSIC_PRINT_LEGACY_BRIDGE,
            CLASSIC_STOCK_OWNER_CHROME,CLASSIC_STOCK_OWNER_CHROME,CLASSIC_STOCK_OWNER_CHROME},
        MSSTYLES_HOST_ALL,
        {STYLE_CAPTION_TEXT_CENTER_CLIP,STYLE_CAPTION_SEPARATOR_AFTER,STYLE_CAPTION_BUTTON_INDEXED,
            STYLE_CAPTION_BUTTON_EDGE_OUTLINE,STYLE_CAPTION_ICON_HIDDEN,1},
        WIN16_FRAME_BORDER,{.separator=STYLE_LINE_FLAT,.group=STYLE_LINE_FLAT,.edit=STYLE_LINE_FLAT,.push=STYLE_PUSH_STEPPED,
            .progress=STYLE_PROGRESS_OUTLINED,.trackbar=STYLE_TRACKBAR_OUTLINED,.header=STYLE_LINE_FLAT,.tab=STYLE_LINE_FLAT,
            .tab_text=STYLE_TAB_TEXT_LITERAL,
            .choice_metrics={14,13,4,STYLE_CHOICE_DPI},.updown={1,1,3},.updown_mode=CLASSIC_UPDOWN_ART,.control_font_points=12},
        STYLE_SCROLL_ARROW_INDEXED,RGB(128,128,128),78,12
    },
    {
        "Win2K","Windows 2000",
        {RGB(0,0,128),RGB(166,202,240),RGB(255,255,255)},
        {RGB(128,128,128),RGB(181,181,181),RGB(192,192,192)},
        RGB(212,208,200),RGB(64,64,64),RGB(212,208,200),true,false,true,
        WIN2000_TITLE_BUTTON_GROUP,
        {19,16,{16,14},{15,15}},
        {CLASSIC_SCROLL_EXTENT_SYSTEM,0,STYLE_SCROLL_THUMB_PAGE,0,0,{STYLE_SCROLL_DRAG_BAR,8,2}},
        {"Tahoma, 8, Bold","Tahoma, 6, Bold","Tahoma, 8"},
        {RGB(212,208,200),RGB(0,0,128),RGB(0,0,0),RGB(255,255,255),RGB(128,128,128),RGB(128,128,128),TRUE},
        WIN2000_POPUP_MENU_METRICS,
        RGB(255,255,255),RGB(212,208,200),RGB(0,0,0),RGB(128,128,128),
        RGB(128,128,128),
        RGB(10,36,106),RGB(255,255,255),
        RGB(212,208,200),
        RGB(212,208,200),false,
        WIN2000_DIALOG_METRICS,
        {CLASSIC_WINDOW_PROVIDER_NONCLIENT,CLASSIC_FILE_EXPLORER_LAYOUT,CLASSIC_FOLDER_OBSERVED_NATIVE,CLASSIC_PRINT_GENERAL_PAGE,
            CLASSIC_STOCK_OBSERVED_LAYOUT,CLASSIC_STOCK_OBSERVED_LAYOUT,CLASSIC_STOCK_OBSERVED_LAYOUT},
        0,
        {STYLE_CAPTION_TEXT_LEFT_ELLIPSIS,STYLE_CAPTION_SEPARATOR_INSIDE,STYLE_CAPTION_BUTTON_SYSTEM,
            STYLE_CAPTION_BUTTON_NO_OUTLINE,STYLE_CAPTION_ICON_NATIVE,3},
        WIN2000_FRAME_BORDER,{.separator=STYLE_LINE_RELIEF,.group=STYLE_LINE_RELIEF,.edit=STYLE_LINE_RELIEF,.push=STYLE_PUSH_RELIEF,
            .progress=STYLE_PROGRESS_INSET,.trackbar=STYLE_TRACKBAR_RELIEF,.header=STYLE_LINE_RELIEF,.tab=STYLE_LINE_RELIEF,
            .tab_text=STYLE_TAB_TEXT_MNEMONIC,
            .native_edge=STYLE_LINE_RELIEF,.choice_fallback=CLASSIC_CHOICE_SYMBOLS,
            .choice_metrics={13,13,0,STYLE_CHOICE_FONT_GAP|STYLE_CHOICE_ALIGN},.updown_mode=CLASSIC_UPDOWN_RECOLOR},
        STYLE_SCROLL_ARROW_SYMBOL,RGB(212,208,200),0,0
    }
};

#endif
