#ifndef STYLE_CONTROL_COLORS_H
#define STYLE_CONTROL_COLORS_H
#include <windows.h>
#include <commctrl.h>

typedef struct classic_surface_colors {
    COLORREF face, edge;
} classic_surface_colors;

typedef struct classic_text_colors {
    COLORREF normal, disabled;
    BOOL present;
} classic_text_colors;

typedef struct classic_list_colors {
    COLORREF background,selection,selection_text,dark;
    BOOL present;
} classic_list_colors;

enum classic_report_role { CLASSIC_REPORT_LIST,CLASSIC_REPORT_TREE,CLASSIC_REPORT_ROLE_COUNT };
enum classic_report_state {
    CLASSIC_REPORT_SELECTED,CLASSIC_REPORT_UNFOCUSED,CLASSIC_REPORT_DISABLED,CLASSIC_REPORT_STATE_COUNT
};
typedef struct classic_report_item_colors {
    COLORREF text,background;
    BOOL has_text,has_background;
} classic_report_item_colors;

enum classic_bar_role { CLASSIC_BAR_TOOLBAR,CLASSIC_BAR_REBAR,CLASSIC_BAR_STATUS,CLASSIC_BAR_ROLE_COUNT };
enum classic_bar_color_role {
    CLASSIC_BAR_FACE,CLASSIC_BAR_PRESSED,CLASSIC_BAR_HIGHLIGHT,CLASSIC_BAR_SHADOW,
    CLASSIC_BAR_EDGE,CLASSIC_BAR_TEXT,CLASSIC_BAR_COLOR_COUNT
};
typedef struct classic_bar_colors {
    COLORREF values[CLASSIC_BAR_COLOR_COUNT];
    unsigned present;
} classic_bar_colors;

enum classic_calendar_role {
    CLASSIC_CALENDAR_OUTSIDE,CLASSIC_CALENDAR_TEXT,CLASSIC_CALENDAR_SELECTED,CLASSIC_CALENDAR_SELECTED_TEXT,
    CLASSIC_CALENDAR_GRID,CLASSIC_CALENDAR_TRAILING_TEXT,CLASSIC_CALENDAR_ROLE_COUNT
};
typedef struct classic_calendar_colors {
    COLORREF values[CLASSIC_CALENDAR_ROLE_COUNT];
    unsigned present;
} classic_calendar_colors;
static inline COLORREF classic_bar_color(const classic_bar_colors *colors,enum classic_bar_color_role role,
    COLORREF fallback)
{
    return colors->present&(1u<<role)?colors->values[role]:fallback;
}


enum classic_combo_state {
    CLASSIC_COMBO_NORMAL, CLASSIC_COMBO_PRESSED, CLASSIC_COMBO_DISABLED, CLASSIC_COMBO_STATE_COUNT
};

typedef struct classic_combo_colors {
    COLORREF button[CLASSIC_COMBO_STATE_COUNT],border,disabled_border;
    BOOL present;
} classic_combo_colors;

static inline enum classic_combo_state classic_combo_state_for(BOOL enabled,BOOL pressed)
{
    return !enabled?CLASSIC_COMBO_DISABLED:pressed?CLASSIC_COMBO_PRESSED:CLASSIC_COMBO_NORMAL;
}

typedef struct classic_group_colors {
    classic_text_colors text;
    COLORREF shadow, highlight;
} classic_group_colors;

enum classic_edit_state {
    CLASSIC_EDIT_NORMAL, CLASSIC_EDIT_READONLY, CLASSIC_EDIT_DISABLED, CLASSIC_EDIT_STATE_COUNT
};

typedef struct classic_edit_border {
    COLORREF line, dark;
    BOOL present;
} classic_edit_border;

static inline enum classic_edit_state classic_edit_state_for(HWND window)
{
    if(!IsWindowEnabled(window))return CLASSIC_EDIT_DISABLED;
    return GetWindowLongPtrW(window,GWL_STYLE)&ES_READONLY?CLASSIC_EDIT_READONLY:CLASSIC_EDIT_NORMAL;
}

enum classic_push_state {
    CLASSIC_PUSH_NORMAL, CLASSIC_PUSH_PRESSED, CLASSIC_PUSH_DISABLED,
    CLASSIC_PUSH_DEFAULTED, CLASSIC_PUSH_STATE_COUNT
};

typedef struct classic_push_colors {
    classic_surface_colors state[CLASSIC_PUSH_STATE_COUNT];
    COLORREF text[CLASSIC_PUSH_STATE_COUNT];
    unsigned text_states;
} classic_push_colors;

enum { CLASSIC_CHECK_STATES=12, CLASSIC_RADIO_STATES=8 };
typedef struct classic_choice_surface {
    classic_surface_colors colors;
    unsigned fields;
    WORD image,fallback_image;
} classic_choice_surface;

typedef struct classic_choice_colors {
    classic_choice_surface check[CLASSIC_CHECK_STATES],radio[CLASSIC_RADIO_STATES];
} classic_choice_colors;

enum { CLASSIC_CHOICE_WIDTH=14, CLASSIC_CHOICE_HEIGHT=13 };
typedef struct classic_choice_art {
    WORD image,fallback_image;
    BOOL available;
    char cell[CLASSIC_CHOICE_WIDTH*CLASSIC_CHOICE_HEIGHT];
} classic_choice_art;

/* The decoder borrows a caller-owned mapping and writes into the caller's cache. */
typedef const char *(*style_choice_art_read_fn)(HMODULE,WORD,WORD,classic_choice_art *);

static inline BOOL classic_choice_has_art(const classic_choice_colors *colors)
{
    for(unsigned i=0;i<ARRAYSIZE(colors->check);++i)if(colors->check[i].image || colors->check[i].fallback_image)return TRUE;
    for(unsigned i=0;i<ARRAYSIZE(colors->radio);++i)if(colors->radio[i].image || colors->radio[i].fallback_image)return TRUE;
    return FALSE;
}

static inline const classic_choice_surface *classic_choice_entry(const classic_choice_colors *colors,
    BOOL radio,LRESULT check,BOOL disabled,LRESULT button_state)
{
    if(!colors)return NULL;
    unsigned value=check==BST_UNCHECKED?0:(!radio && check==BST_INDETERMINATE?2:1);
    unsigned state=4*value+(disabled?3:(button_state&BST_PUSHED)?2:(button_state&BST_HOT)?1:0);
    return radio?&colors->radio[state]:&colors->check[state];
}

static inline const classic_surface_colors *classic_choice_color(const classic_choice_surface *entry)
{
    return entry && entry->fields==3?&entry->colors:NULL;
}

static inline enum classic_push_state classic_push_state_from_button(BOOL disabled,BOOL pressed,BOOL defaulted)
{
    return disabled?CLASSIC_PUSH_DISABLED:pressed?CLASSIC_PUSH_PRESSED:
        defaulted?CLASSIC_PUSH_DEFAULTED:CLASSIC_PUSH_NORMAL;
}
static inline classic_surface_colors classic_push_color(const classic_push_colors *colors,
    BOOL disabled,BOOL pressed,BOOL defaulted)
{
    return colors->state[classic_push_state_from_button(disabled,pressed,defaulted)];
}
static inline COLORREF classic_push_text_color(const classic_push_colors *colors,enum classic_push_state state)
{
    return colors->text_states&(1u<<state)?colors->text[state]:CLR_INVALID;
}

enum { CLASSIC_COMBO_ARROW_WIDTH=7, CLASSIC_COMBO_ARROW_HEIGHT=9 };
typedef struct classic_combo_arrow_art {
    BYTE rows[CLASSIC_COMBO_ARROW_HEIGHT];
    BOOL present;
} classic_combo_arrow_art;

#endif
