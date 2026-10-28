#include "theme_track.h"
#include <limits.h>

#define THEME_LIMIT 128
typedef struct theme_record {
    HTHEME theme;
    unsigned refs;
    BOOL ambiguous, sticky;
    int kind;
} theme_record;

static SRWLOCK lock = SRWLOCK_INIT;
static theme_record table[THEME_LIMIT];
static ULONGLONG epoch;
static unsigned opening, closing;
static BOOL disabled;

static theme_record *find_record(HTHEME h)
{
    if (!h) return NULL;
    for (unsigned i = 0; i < THEME_LIMIT; ++i)
        if (table[i].theme == h) return &table[i];
    return NULL;
}
static theme_record *free_record(void)
{
    for (unsigned i = 0; i < THEME_LIMIT; ++i)
        if (!table[i].theme) return &table[i];
    return NULL;
}
static void disable_tracking(void)
{
    ZeroMemory(table, sizeof(table));
    disabled = TRUE;
}
static void advance(void)
{
    if (epoch == ULLONG_MAX) disable_tracking();
    else ++epoch;
}
static void poison(HTHEME h)
{
    theme_record *r = find_record(h);
    if (!r) r = free_record();
    if (!r) { disable_tracking(); return; }
    *r = (theme_record){h, 0, TRUE, TRUE, NTW16S_THEME_OTHER};
}
ntw16s_theme_ticket ntw16s_theme_open_begin(void)
{
    AcquireSRWLockExclusive(&lock);
    ntw16s_theme_ticket t = {epoch, !closing && !opening && !disabled, NULL};
    if (opening == UINT_MAX) disable_tracking();
    else ++opening;
    advance(); t.epoch = epoch;
    ReleaseSRWLockExclusive(&lock);
    return t;
}
void ntw16s_theme_open_end(ntw16s_theme_ticket t, HTHEME h, int kind)
{
    AcquireSRWLockExclusive(&lock);
    if (!opening) disable_tracking();
    else {
        --opening;
        if (h && !disabled) {
            if (!t.eligible || t.epoch != epoch || closing || opening) poison(h);
            else {
                theme_record *r = find_record(h);
                if (!r) {
                    r = free_record();
                    if (r) *r = (theme_record){h, 1, FALSE, FALSE, kind};
                    else disable_tracking();
                } else if (!r->sticky) {
                    if (r->refs == UINT_MAX) poison(h);
                    else { ++r->refs; if (r->kind != kind) r->ambiguous = TRUE; }
                }
            }
        }
    }
    advance(); ReleaseSRWLockExclusive(&lock);
}
void ntw16s_theme_open_cancel(void)
{
    AcquireSRWLockExclusive(&lock);
    if (opening) --opening;
    disable_tracking(); advance(); ReleaseSRWLockExclusive(&lock);
}
void ntw16s_theme_close_begin(HTHEME h)
{
    AcquireSRWLockExclusive(&lock);
    if (closing == UINT_MAX) disable_tracking();
    else ++closing;
    advance();
    if (!disabled) {
        theme_record *r = find_record(h);
        if (r && !r->sticky && --r->refs == 0) *r = (theme_record){0};
    }
    ReleaseSRWLockExclusive(&lock);
}
void ntw16s_theme_close_end(HTHEME h, BOOL success)
{
    AcquireSRWLockExclusive(&lock);
    if (!closing) disable_tracking();
    else { if (!success && h && !disabled) poison(h); --closing; }
    advance(); ReleaseSRWLockExclusive(&lock);
}
static ntw16s_theme_ticket theme_begin(HTHEME h,int kind)
{
    AcquireSRWLockShared(&lock);
    theme_record *r = find_record(h);
    ntw16s_theme_ticket t = {epoch, !disabled && !closing && !opening && r && r->kind==kind && !r->ambiguous, h};
    ReleaseSRWLockShared(&lock); return t;
}
static BOOL theme_end(HTHEME h,ntw16s_theme_ticket t,int kind)
{
    AcquireSRWLockShared(&lock);
    theme_record *r = find_record(h);
    BOOL ok = t.eligible && t.theme == h && !disabled && !closing && !opening && t.epoch == epoch && r && r->kind==kind && !r->ambiguous;
    ReleaseSRWLockShared(&lock); return ok;
}
ntw16s_theme_ticket ntw16s_theme_button_begin(HTHEME h){return theme_begin(h,NTW16S_THEME_BUTTON);}
BOOL ntw16s_theme_button_end(HTHEME h,ntw16s_theme_ticket t){return theme_end(h,t,NTW16S_THEME_BUTTON);}
ntw16s_theme_ticket ntw16s_theme_glow_begin(HTHEME h){return theme_begin(h,NTW16S_THEME_TEXTGLOW);}
BOOL ntw16s_theme_glow_end(HTHEME h,ntw16s_theme_ticket t){return theme_end(h,t,NTW16S_THEME_TEXTGLOW);}
