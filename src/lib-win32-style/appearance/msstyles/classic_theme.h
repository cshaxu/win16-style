#ifndef STYLE_MSSTYLES_CLASSIC_THEME_H
#define STYLE_MSSTYLES_CLASSIC_THEME_H

#include "classic_style.h"
#include "v4_font.h"
#include <windows.h>

typedef struct classic_caption_colors {
    COLORREF start, end, text;
} classic_caption_colors;

typedef struct classic_menu_colors {
    COLORREF background, selection, text, selected_text, disabled_text, separator;
    BOOL present;
} classic_menu_colors;

typedef struct classic_surface_colors {
    COLORREF face, edge;
} classic_surface_colors;

typedef struct classic_frame_colors {
    COLORREF face, light, shadow, edge;
    BOOL present;
} classic_frame_colors;

typedef struct classic_text_colors {
    COLORREF normal, disabled;
    BOOL present;
} classic_text_colors;

typedef struct classic_list_colors {
    COLORREF background,selection,selection_text,dark;
    BOOL present;
} classic_list_colors;

typedef struct classic_tab_colors {
    COLORREF pane,normal,selected,border;
    BOOL present;
} classic_tab_colors;

typedef struct classic_header_colors {
    COLORREF normal,pressed;
    BOOL present;
} classic_header_colors;

typedef struct classic_progress_colors {
    COLORREF trough,fill,border,error,paused;
    BOOL present,extra_states;
} classic_progress_colors;

typedef struct classic_trackbar_colors {
    COLORREF channel,thumb,edge;
    BOOL present;
} classic_trackbar_colors;

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
} classic_push_colors;

enum { CLASSIC_CHECK_STATES=12, CLASSIC_RADIO_STATES=8 };
typedef struct classic_choice_surface {
    classic_surface_colors colors;
    unsigned fields;
    WORD image;
} classic_choice_surface;

typedef struct classic_choice_colors {
    classic_choice_surface check[CLASSIC_CHECK_STATES],radio[CLASSIC_RADIO_STATES];
} classic_choice_colors;

const classic_choice_surface *classic_choice_entry(const classic_choice_colors *colors,
    BOOL radio,LRESULT check,BOOL disabled,LRESULT button_state);
static inline const classic_surface_colors *classic_choice_color(const classic_choice_surface *entry)
{
    return entry && entry->fields==3?&entry->colors:NULL;
}

static inline classic_surface_colors classic_push_color(const classic_push_colors *colors,
    BOOL disabled,BOOL pressed,BOOL defaulted)
{
    return colors->state[disabled?CLASSIC_PUSH_DISABLED:pressed?CLASSIC_PUSH_PRESSED:
        defaulted?CLASSIC_PUSH_DEFAULTED:CLASSIC_PUSH_NORMAL];
}

typedef struct classic_scroll_surface {
    COLORREF face, light, highlight, shadow, dark;
} classic_scroll_surface;

typedef struct classic_scroll_colors {
    classic_scroll_surface thumb[2], track[2]; /* horizontal, vertical */
    classic_scroll_surface sizebox;
    BOOL tracks_and_thumbs;
    BOOL own_sizebox;
} classic_scroll_colors;

enum classic_caption_image_role {
    CLASSIC_IMAGE_MINIMIZE, CLASSIC_IMAGE_MAXIMIZE, CLASSIC_IMAGE_RESTORE,
    CLASSIC_IMAGE_SYSTEM, CLASSIC_IMAGE_ROLE_COUNT
};

typedef struct classic_caption_images {
    WORD id[CLASSIC_IMAGE_ROLE_COUNT][2]; /* normal, pressed */
} classic_caption_images;

typedef struct classic_theme {
    classic_caption_colors active, inactive;
    classic_frame_colors frame;
    classic_text_colors label;
    classic_group_colors group;
    classic_edit_border edit[CLASSIC_EDIT_STATE_COUNT];
    classic_list_colors list;
    classic_combo_colors combo;
    classic_tab_colors tab;
    classic_header_colors header;
    classic_progress_colors progress[2]; /* horizontal, vertical */
    classic_trackbar_colors trackbar[2]; /* horizontal, vertical */
    msstyles_font caption_font, small_caption_font, menu_font;
    classic_menu_colors menu_bar, menu_popup;
    classic_menu_metrics menu_metrics;
    classic_dialog_metrics dialog_metrics;
    WORD menu_images[2]; /* check, submenu */
    classic_push_colors push;
    classic_choice_colors choice;
    classic_surface_colors scroll_arrow;
    classic_button_group title_buttons;
    classic_caption_metrics caption_metrics;
    classic_caption_images caption_images;
    classic_scroll_colors scroll;
    classic_scroll_metrics scroll_metrics;
    BOOL gradient;
} classic_theme;

int classic_scroll_nominal(classic_scroll_metrics metrics,enum classic_scroll_extent_source fallback,
    const RECT *bar,BOOL vertical,UINT dpi);
/* Reads the selected variant from an already mapped resource module. */
HRESULT classic_theme_read(HMODULE module,PCWSTR size,PCWSTR color,classic_theme *theme);
classic_dialog_metrics classic_theme_dialog_metrics(HMODULE module,classic_dialog_metrics fallback);

/* Reads the selected variant of a v4 classic theme as data only. All output
 * values are owned by the caller and the output is unchanged on failure. */
HRESULT classic_theme_load(PCWSTR path,PCWSTR size,PCWSTR color,classic_theme *theme);

typedef struct classic_theme_mapping {
    HMODULE resources;
    classic_theme model;
} classic_theme_mapping;

/* Opens a packaged .theme descriptor from an absolute path outside loader
 * lock. Pass an empty destination. The source owns the data mapping that
 * resolves image and dialog IDs until classic_theme_close. */
HRESULT classic_theme_open(PCWSTR descriptor,classic_theme_mapping *source);
void classic_theme_close(classic_theme_mapping *source);

typedef enum classic_theme_family { CLASSIC_THEME_WIN16, CLASSIC_THEME_WIN2K } classic_theme_family;
/* A folder contains one .theme descriptor. Its resources determine the
 * renderer. The callback borrows each folder name for that invocation only. */
typedef BOOL (*classic_theme_visit)(PCWSTR folder,classic_theme_family family,void *context);
HRESULT classic_theme_open_folder(HMODULE renderer,PCWSTR folder,classic_theme_mapping *,
    classic_theme_family *family);
void classic_theme_visit_folders(HMODULE renderer,classic_theme_visit visit,void *context);
/* A stable, pointer-free choice used by the startup and global protocols.
 * Zero and one preserve the published canonical renderer IDs. */
unsigned classic_theme_profile(PCWSTR folder,classic_theme_family family);
HRESULT classic_theme_profile_folder(HMODULE renderer,unsigned profile,WCHAR folder[MAX_PATH]);
HRESULT classic_theme_open_profile(HMODULE renderer,unsigned profile,classic_theme_mapping *);
enum { CLASSIC_THEME_PROFILE_CAP = 0x10000000u, CLASSIC_THEME_PROFILE_FIRST_CUSTOM = 8 };
static inline classic_theme_family classic_theme_profile_family(unsigned profile)
{return (classic_theme_family)(profile&1u);}
void classic_theme_set_process_profile(unsigned profile);
void classic_theme_set_global_profile_reader(BOOL (*read)(unsigned *));
unsigned classic_theme_process_profile(void);
unsigned classic_theme_enter_profile(unsigned profile);
void classic_theme_leave_profile(unsigned previous);

/* Resolves a packaged theme beside its renderer. If the package directory
 * is absent, an embedded fallback can use resources in that same renderer. */
HRESULT classic_theme_open_packaged(HMODULE renderer,classic_theme_family family,
    BOOL embedded,classic_theme_mapping *source);
DWORD classic_theme_validate_packaged(HMODULE renderer,classic_theme_family family);

#endif
