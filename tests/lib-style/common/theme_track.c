#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "lib-style/common/theme_track.h"
enum { TEST_THEME_OTHER, TEST_THEME_BUTTON, TEST_THEME_TEXTGLOW, TEST_THEME_COMBOBOX };

#define CHECK(x) do { if (!(x)) { fprintf(stderr, "THEME_TRACK_FAIL line=%d: %s\n", __LINE__, #x); return 1; } } while (0)
static void opened(HTHEME h, BOOL button)
{
    style_theme_track_ticket t = style_theme_track_open_begin();
    style_theme_track_open_end(t, h, button);
}
static void closed(HTHEME h, BOOL success)
{
    style_theme_track_close_begin(h);
    style_theme_track_close_end(h, success);
}
typedef struct overlap { HANDLE start, done; HTHEME h; } overlap;
static DWORD WINAPI close_thread(void *argument)
{
    overlap *o = argument;
    if (WaitForSingleObject(o->start, 2000) != WAIT_OBJECT_0) return 1;
    closed(o->h, TRUE); SetEvent(o->done); return 0;
}
int main(int argc, char **argv)
{
    if (argc == 2 && !strcmp(argv[1], "cancel")) {
        (void)style_theme_track_open_begin();
        style_theme_track_open_cancel();
        HTHEME after = (HTHEME)(UINT_PTR)0x7070;
        opened(after, TRUE);
        CHECK(!style_theme_track_begin(after,TEST_THEME_BUTTON).eligible);
        puts("THEME_TRACK_CANCEL_OK");
        return 0;
    }
    HTHEME button = (HTHEME)(UINT_PTR)0x1010, alias = (HTHEME)(UINT_PTR)0x2020;
    opened(button, TRUE); opened(button, TRUE);
    style_theme_track_ticket draw = style_theme_track_begin(button,TEST_THEME_BUTTON);
    CHECK(draw.eligible && style_theme_track_end(button, draw,TEST_THEME_BUTTON));
    closed(button, TRUE);
    CHECK(!style_theme_track_end(button, draw,TEST_THEME_BUTTON));
    CHECK(style_theme_track_begin(button,TEST_THEME_BUTTON).eligible);
    draw = style_theme_track_begin(button,TEST_THEME_BUTTON);
    closed(NULL, FALSE);
    CHECK(!style_theme_track_end(button, draw,TEST_THEME_BUTTON));
    CHECK(style_theme_track_begin(button,TEST_THEME_BUTTON).eligible);
    closed(button, TRUE); CHECK(!style_theme_track_begin(button,TEST_THEME_BUTTON).eligible);
    opened(alias, TRUE); opened(alias, FALSE);
    CHECK(!style_theme_track_begin(alias,TEST_THEME_BUTTON).eligible);
    closed(alias, TRUE); closed(alias, TRUE);

    HTHEME glow = (HTHEME)(UINT_PTR)0x2121;
    style_theme_track_ticket glow_open=style_theme_track_open_begin();
    style_theme_track_open_end(glow_open,glow,TEST_THEME_TEXTGLOW);
    style_theme_track_ticket glow_draw=style_theme_track_begin(glow,TEST_THEME_TEXTGLOW);
    CHECK(glow_draw.eligible && style_theme_track_end(glow,glow_draw,TEST_THEME_TEXTGLOW));
    CHECK(!style_theme_track_begin(glow,TEST_THEME_BUTTON).eligible);
    closed(glow,TRUE);CHECK(!style_theme_track_end(glow,glow_draw,TEST_THEME_TEXTGLOW));

    HTHEME combo=(HTHEME)(UINT_PTR)0x2323;style_theme_track_ticket co=style_theme_track_open_begin();
    style_theme_track_open_end(co,combo,TEST_THEME_COMBOBOX);style_theme_track_ticket cd=style_theme_track_begin(combo,TEST_THEME_COMBOBOX);
    CHECK(cd.eligible && style_theme_track_end(combo,cd,TEST_THEME_COMBOBOX));CHECK(!style_theme_track_begin(combo,TEST_THEME_BUTTON).eligible);
    closed(combo,TRUE);CHECK(!style_theme_track_end(combo,cd,TEST_THEME_COMBOBOX));
    co=style_theme_track_open_begin();style_theme_track_open_end(co,combo,TEST_THEME_COMBOBOX);opened(combo,TRUE);
    CHECK(!style_theme_track_begin(combo,TEST_THEME_COMBOBOX).eligible && !style_theme_track_begin(combo,TEST_THEME_BUTTON).eligible);closed(combo,TRUE);closed(combo,TRUE);

    HTHEME failed = (HTHEME)(UINT_PTR)0x3030;
    opened(failed, TRUE); CHECK(style_theme_track_begin(failed,TEST_THEME_BUTTON).eligible);
    closed(failed, FALSE); opened(failed, TRUE);
    CHECK(!style_theme_track_begin(failed,TEST_THEME_BUTTON).eligible);

    HTHEME nested = (HTHEME)(UINT_PTR)0x4040;
    style_theme_track_ticket outer = style_theme_track_open_begin();
    style_theme_track_ticket inner = style_theme_track_open_begin();
    style_theme_track_open_end(inner, nested, TRUE);
    style_theme_track_open_end(outer, nested, TRUE);
    CHECK(!style_theme_track_begin(nested,TEST_THEME_BUTTON).eligible);

    overlap o = {CreateEventW(NULL, TRUE, FALSE, NULL), CreateEventW(NULL, TRUE, FALSE, NULL), (HTHEME)(UINT_PTR)0x5050};
    CHECK(o.start && o.done);
    HANDLE thread = CreateThread(NULL, 0, close_thread, &o, 0, NULL); CHECK(thread);
    outer = style_theme_track_open_begin(); SetEvent(o.start);
    CHECK(WaitForSingleObject(o.done, 2000) == WAIT_OBJECT_0);
    style_theme_track_open_end(outer, o.h, TRUE);
    CHECK(WaitForSingleObject(thread, 2000) == WAIT_OBJECT_0);
    DWORD code = 1; CHECK(GetExitCodeThread(thread, &code) && code == 0);
    CloseHandle(thread); CloseHandle(o.start); CloseHandle(o.done);
    CHECK(!style_theme_track_begin(o.h,TEST_THEME_BUTTON).eligible);

    HTHEME safe = (HTHEME)(UINT_PTR)0x6060; opened(safe, TRUE);
    CHECK(style_theme_track_begin(safe,TEST_THEME_BUTTON).eligible);
    for (unsigned i = 0; i < 140; ++i) opened((HTHEME)(UINT_PTR)(0x100000 + i), TRUE);
    CHECK(!style_theme_track_begin(safe,TEST_THEME_BUTTON).eligible);
    puts("THEME_TRACK_OK shared=1 alias=1 combo=1 failed_close=1 nested=1 overlap=1 capacity=1");
    return 0;
}
