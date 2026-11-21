#ifndef STYLE_MSSTYLES_CLASSIC_THEME_H
#define STYLE_MSSTYLES_CLASSIC_THEME_H

#include "classic_style.h"
#include "lib-win32-style/application/window/palette.h"
#include "v4_font.h"
#include "theme_file.h"
#include "lib-win32-style/application/menu/menu_art.h"
#include <windows.h>

typedef struct classic_caption_colors {
    COLORREF start, end, text;
} classic_caption_colors;

typedef struct classic_menu_colors {
    COLORREF background, selection, text, selected_text, disabled_text, separator;
    BOOL present;
} classic_menu_colors;

HRESULT style_menu_art_read(HMODULE module,const WORD images[2],style_menu_art *art);

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
    WORD image;
} classic_choice_surface;

typedef struct classic_choice_colors {
    classic_choice_surface check[CLASSIC_CHECK_STATES],radio[CLASSIC_RADIO_STATES];
} classic_choice_colors;

enum { CLASSIC_CHOICE_WIDTH=14, CLASSIC_CHOICE_HEIGHT=13 };
typedef struct classic_choice_art {
    WORD image;
    BOOL available;
    char cell[CLASSIC_CHOICE_WIDTH*CLASSIC_CHOICE_HEIGHT];
} classic_choice_art;

/* A one-entry cache for a caller-owned mapping. Decoded pixels are copied. */
const char *classic_choice_art_get(HMODULE module,WORD image,classic_choice_art *art);
BOOL classic_choice_has_art(const classic_choice_colors *colors);

const classic_choice_surface *classic_choice_entry(const classic_choice_colors *colors,
    BOOL radio,LRESULT check,BOOL disabled,LRESULT button_state);
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

typedef struct classic_scroll_surface {
    COLORREF face, light, highlight, shadow, dark;
} classic_scroll_surface;

typedef struct classic_scroll_colors {
    classic_scroll_surface thumb[2], track[2]; /* horizontal, vertical */
    classic_scroll_surface sizebox;
    BOOL tracks_and_thumbs;
    BOOL own_sizebox;
} classic_scroll_colors;

typedef struct classic_caption_images {
    WORD id[CLASSIC_IMAGE_ROLE_COUNT][2]; /* normal, pressed */
} classic_caption_images;

/* Copies palette roles out of the mapped resource. Call outside loader lock. */
HRESULT classic_caption_art_read(HMODULE module,const classic_caption_images *images,classic_caption_art *art);

enum { CLASSIC_COMBO_ARROW_WIDTH=7, CLASSIC_COMBO_ARROW_HEIGHT=9 };
typedef struct classic_combo_arrow_art {
    BYTE rows[CLASSIC_COMBO_ARROW_HEIGHT];
    BOOL present;
} classic_combo_arrow_art;

typedef struct classic_theme {
    classic_caption_colors active, inactive;
    classic_frame_colors frame;
    classic_text_colors label;
    classic_group_colors group;
    classic_edit_border edit[CLASSIC_EDIT_STATE_COUNT];
    classic_list_colors list;
    classic_report_item_colors report[CLASSIC_REPORT_ROLE_COUNT][CLASSIC_REPORT_STATE_COUNT];
    classic_bar_colors bars[CLASSIC_BAR_ROLE_COUNT];
    classic_calendar_colors calendar;
    unsigned calendar_font_dpi,calendar_font_points;
    COLORREF date_button_face;
    BOOL has_date_button_face;
    classic_combo_colors combo;
    style_tab_colors tab;
    style_header_colors header;
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
    style_scroll_arrow_art scroll_art;
    classic_combo_arrow_art combo_arrow;
    classic_button_group title_buttons;
    classic_dialog_policy dialog_policy;
    classic_caption_metrics caption_metrics;
    style_caption_presentation caption_presentation;
    style_border_presentation border_presentation;
    classic_control_presentation controls;
    classic_caption_images caption_images;
    classic_scroll_colors scroll;
    classic_scroll_metrics scroll_metrics;
    BOOL gradient;
} classic_theme;

static inline void classic_theme_window_palette(style_window_palette *p,const classic_theme *theme)
{
    p->active=theme->active.start;
    p->inactive=theme->inactive.start;
    p->active_text=theme->active.text;
    p->inactive_text=theme->inactive.text;
    if(theme->frame.present){
        p->face=theme->frame.face;
        p->light=theme->frame.light;
        p->shadow=theme->frame.shadow;
        p->edge=theme->frame.edge;
    }
}

style_relief_palette classic_relief_palette(const classic_frame_colors *,const classic_list_colors *,const classic_text_colors *);
static inline style_relief_palette classic_theme_relief_palette(const classic_theme *theme)
{
    return theme?classic_relief_palette(&theme->frame,&theme->list,&theme->label):classic_relief_palette(NULL,NULL,NULL);
}

int classic_scroll_nominal(classic_scroll_metrics metrics,enum classic_scroll_extent_source fallback,
    const RECT *bar,BOOL vertical,UINT dpi);
/* Reads the selected variant from an already mapped resource module. */
HRESULT classic_theme_read(HMODULE module,PCWSTR size,PCWSTR color,classic_theme *theme);
classic_dialog_metrics classic_theme_dialog_metrics(HMODULE module);
HRESULT classic_theme_dialog_resources(HMODULE,HMODULE *,classic_dialog_metrics *);

/* Reads the selected variant of a v4 classic theme as data only. All output
 * values are owned by the caller and the output is unchanged on failure. */
HRESULT classic_theme_load(PCWSTR path,PCWSTR size,PCWSTR color,classic_theme *theme);

typedef enum classic_theme_family { CLASSIC_THEME_WIN16, CLASSIC_THEME_WIN2K } classic_theme_family;
typedef struct classic_theme_selection {
    unsigned id;
    classic_theme_family family;
} classic_theme_selection;

/* The validated folder selection carries the layout's stable wire tag. */
static inline enum classic_window_provider classic_theme_selection_layout(classic_theme_selection selection)
{
    const unsigned layout=(unsigned)selection.family+CLASSIC_WINDOW_PROVIDER_FULL_CLIENT;
    return layout>=CLASSIC_WINDOW_PROVIDER_FULL_CLIENT && layout<=CLASSIC_WINDOW_PROVIDER_NONCLIENT?
        (enum classic_window_provider)layout:CLASSIC_WINDOW_PROVIDER_UNSPECIFIED;
}
typedef struct classic_theme_mapping {
    HMODULE resources;
    classic_theme model;
    WCHAR display_name[MSSTYLES_THEME_TOKEN_CAPACITY];
    msstyles_host_options hosts;
    classic_theme_family family;
    WCHAR compatibility_folder[MSSTYLES_THEME_TOKEN_CAPACITY];
} classic_theme_mapping;

/* Opens a packaged .theme descriptor from an absolute path outside loader
 * lock. Pass an empty destination. The source owns the data mapping that
 * resolves image and dialog IDs until classic_theme_close. */
HRESULT classic_theme_open(PCWSTR descriptor,classic_theme_mapping *source);
void classic_theme_close(classic_theme_mapping *source);

/* A folder contains one .theme descriptor. Its resources determine the
 * renderer. The callback borrows the folder and optional display name for
 * that invocation only. */
typedef BOOL (*classic_theme_visit)(PCWSTR folder,PCWSTR display_name,classic_theme_family family,void *context);
HRESULT classic_theme_open_folder(HMODULE renderer,PCWSTR folder,classic_theme_mapping *,
    classic_theme_family *family);
void classic_theme_visit_folders(HMODULE renderer,classic_theme_visit visit,void *context);
/* A stable, pointer-free directory ID used by all three entry paths. */
unsigned classic_theme_folder_id(PCWSTR folder);
HRESULT classic_theme_profile_folder(HMODULE renderer,unsigned profile,WCHAR folder[MAX_PATH]);
HRESULT classic_theme_open_profile(HMODULE renderer,unsigned profile,classic_theme_mapping *);
HRESULT classic_theme_open_selected(HMODULE renderer,classic_theme_selection selection,classic_theme_mapping *);
/* S_FALSE means no active theme. On S_OK, the selection includes the adapter. */
HRESULT classic_theme_active_selection(classic_theme_selection *);
static inline BOOL classic_theme_require_layout(enum classic_window_provider layout)
{
    classic_theme_selection selection;
    HRESULT active=classic_theme_active_selection(&selection);
    if(active!=S_OK){
        SetLastError(active==S_FALSE?ERROR_RESOURCE_DATA_NOT_FOUND:HRESULT_CODE(active));
        return FALSE;
    }
    if(classic_theme_selection_layout(selection)!=layout){
        SetLastError(ERROR_NOT_SUPPORTED);
        return FALSE;
    }
    return TRUE;
}
HRESULT classic_theme_open_dialog(HMODULE renderer,classic_theme_mapping *source);
enum { CLASSIC_THEME_PROFILE_CAP = 0x10000000u, CLASSIC_THEME_PROFILE_FIRST_CUSTOM = 8,
    CLASSIC_THEME_FOLDER_ID_MARKER = 6 };
static inline classic_theme_family classic_theme_profile_family(unsigned profile)
{return (classic_theme_family)(profile&1u);}
static inline BOOL classic_theme_is_folder_id(unsigned id)
{
    return id>=CLASSIC_THEME_PROFILE_FIRST_CUSTOM && id<CLASSIC_THEME_PROFILE_CAP &&
        (id&CLASSIC_THEME_FOLDER_ID_MARKER)==CLASSIC_THEME_FOLDER_ID_MARKER;
}
/* The global protocol has one DWORD for the selection. The high field
 * carries the adapter independently from the bounded directory ID. */
static inline unsigned classic_theme_encode_selection(classic_theme_selection selection)
{return selection.id|(CLASSIC_THEME_PROFILE_CAP*(unsigned)selection.family);}
static inline BOOL classic_theme_decode_selection(unsigned value,classic_theme_selection *selection)
{
    unsigned id=value&(CLASSIC_THEME_PROFILE_CAP-1u),adapter=value/CLASSIC_THEME_PROFILE_CAP;
#ifdef STYLE_THEME_EXTERNAL_ONLY
    if(!classic_theme_is_folder_id(id) || adapter>1u)return FALSE;
#else
    if(classic_theme_is_folder_id(id)){
        if(adapter>1u)return FALSE;
    }else{
        if(adapter || (id>1 && id<CLASSIC_THEME_PROFILE_FIRST_CUSTOM))return FALSE;
        adapter=(unsigned)classic_theme_profile_family(id);
    }
#endif
    *selection=(classic_theme_selection){id,(classic_theme_family)adapter};
    return TRUE;
}
void classic_theme_set_process_profile(unsigned profile,classic_theme_family family);
void classic_theme_set_global_profile_reader(BOOL (*read)(unsigned *));
unsigned classic_theme_process_profile(void);
unsigned classic_theme_enter_profile(classic_theme_selection selection);
void classic_theme_leave_profile(unsigned previous);

HRESULT classic_theme_open_active(HMODULE renderer,classic_theme_mapping *source);
#ifndef STYLE_THEME_EXTERNAL_ONLY
/* Unselected or preset callers of the standalone compatibility builds. */
HRESULT classic_theme_open_packaged(HMODULE renderer,classic_theme_family family,
    BOOL embedded,classic_theme_mapping *source);
#endif
/* The active theme owns the lookup. Standalone compatibility builds use the
 * layout to select a fallback only when an active directory is unavailable. */
DWORD classic_theme_attachment_layout(HMODULE renderer,enum classic_window_provider layout,classic_theme_mapping *source);

#endif
