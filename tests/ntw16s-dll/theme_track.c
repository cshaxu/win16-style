#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "ntw16s-dll/injection/theme_track.h"

#define CHECK(x) do { if (!(x)) { fprintf(stderr, "THEME_TRACK_FAIL line=%d: %s\n", __LINE__, #x); return 1; } } while (0)
static void opened(HTHEME h, BOOL button)
{
    ntw16s_theme_ticket t = ntw16s_theme_open_begin();
    ntw16s_theme_open_end(t, h, button);
}
static void closed(HTHEME h, BOOL success)
{
    ntw16s_theme_close_begin(h);
    ntw16s_theme_close_end(h, success);
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
        (void)ntw16s_theme_open_begin();
        ntw16s_theme_open_cancel();
        HTHEME after = (HTHEME)(UINT_PTR)0x7070;
        opened(after, TRUE);
        CHECK(!ntw16s_theme_button_begin(after).eligible);
        puts("THEME_TRACK_CANCEL_OK");
        return 0;
    }
    HTHEME button = (HTHEME)(UINT_PTR)0x1010, alias = (HTHEME)(UINT_PTR)0x2020;
    opened(button, TRUE); opened(button, TRUE);
    ntw16s_theme_ticket draw = ntw16s_theme_button_begin(button);
    CHECK(draw.eligible && ntw16s_theme_button_end(button, draw));
    closed(button, TRUE);
    CHECK(!ntw16s_theme_button_end(button, draw));
    CHECK(ntw16s_theme_button_begin(button).eligible);
    draw = ntw16s_theme_button_begin(button);
    closed(NULL, FALSE);
    CHECK(!ntw16s_theme_button_end(button, draw));
    CHECK(ntw16s_theme_button_begin(button).eligible);
    closed(button, TRUE); CHECK(!ntw16s_theme_button_begin(button).eligible);
    opened(alias, TRUE); opened(alias, FALSE);
    CHECK(!ntw16s_theme_button_begin(alias).eligible);
    closed(alias, TRUE); closed(alias, TRUE);

    HTHEME glow = (HTHEME)(UINT_PTR)0x2121;
    ntw16s_theme_ticket glow_open=ntw16s_theme_open_begin();
    ntw16s_theme_open_end(glow_open,glow,NTW16S_THEME_TEXTGLOW);
    ntw16s_theme_ticket glow_draw=ntw16s_theme_glow_begin(glow);
    CHECK(glow_draw.eligible && ntw16s_theme_glow_end(glow,glow_draw));
    CHECK(!ntw16s_theme_button_begin(glow).eligible);
    closed(glow,TRUE);CHECK(!ntw16s_theme_glow_end(glow,glow_draw));

    HTHEME combo=(HTHEME)(UINT_PTR)0x2323;ntw16s_theme_ticket co=ntw16s_theme_open_begin();
    ntw16s_theme_open_end(co,combo,NTW16S_THEME_COMBOBOX);ntw16s_theme_ticket cd=ntw16s_theme_combo_begin(combo);
    CHECK(cd.eligible && ntw16s_theme_combo_end(combo,cd));CHECK(!ntw16s_theme_button_begin(combo).eligible);
    closed(combo,TRUE);CHECK(!ntw16s_theme_combo_end(combo,cd));
    co=ntw16s_theme_open_begin();ntw16s_theme_open_end(co,combo,NTW16S_THEME_COMBOBOX);opened(combo,TRUE);
    CHECK(!ntw16s_theme_combo_begin(combo).eligible && !ntw16s_theme_button_begin(combo).eligible);closed(combo,TRUE);closed(combo,TRUE);

    HTHEME failed = (HTHEME)(UINT_PTR)0x3030;
    opened(failed, TRUE); CHECK(ntw16s_theme_button_begin(failed).eligible);
    closed(failed, FALSE); opened(failed, TRUE);
    CHECK(!ntw16s_theme_button_begin(failed).eligible);

    HTHEME nested = (HTHEME)(UINT_PTR)0x4040;
    ntw16s_theme_ticket outer = ntw16s_theme_open_begin();
    ntw16s_theme_ticket inner = ntw16s_theme_open_begin();
    ntw16s_theme_open_end(inner, nested, TRUE);
    ntw16s_theme_open_end(outer, nested, TRUE);
    CHECK(!ntw16s_theme_button_begin(nested).eligible);

    overlap o = {CreateEventW(NULL, TRUE, FALSE, NULL), CreateEventW(NULL, TRUE, FALSE, NULL), (HTHEME)(UINT_PTR)0x5050};
    CHECK(o.start && o.done);
    HANDLE thread = CreateThread(NULL, 0, close_thread, &o, 0, NULL); CHECK(thread);
    outer = ntw16s_theme_open_begin(); SetEvent(o.start);
    CHECK(WaitForSingleObject(o.done, 2000) == WAIT_OBJECT_0);
    ntw16s_theme_open_end(outer, o.h, TRUE);
    CHECK(WaitForSingleObject(thread, 2000) == WAIT_OBJECT_0);
    DWORD code = 1; CHECK(GetExitCodeThread(thread, &code) && code == 0);
    CloseHandle(thread); CloseHandle(o.start); CloseHandle(o.done);
    CHECK(!ntw16s_theme_button_begin(o.h).eligible);

    HTHEME safe = (HTHEME)(UINT_PTR)0x6060; opened(safe, TRUE);
    CHECK(ntw16s_theme_button_begin(safe).eligible);
    for (unsigned i = 0; i < 140; ++i) opened((HTHEME)(UINT_PTR)(0x100000 + i), TRUE);
    CHECK(!ntw16s_theme_button_begin(safe).eligible);
    puts("THEME_TRACK_OK shared=1 alias=1 combo=1 failed_close=1 nested=1 overlap=1 capacity=1");
    return 0;
}
