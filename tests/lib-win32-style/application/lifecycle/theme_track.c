#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "../../../ntw16s-dll/exception_probe.h"
#include "lib-win32-style/application/lifecycle/theme_track.h"


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
static LRESULT PROCESS_HOOK_UNWIND_FUNCTION CALLBACK scope_proc(HWND w,UINT m,WPARAM wp,LPARAM lp)
{
    if(m!=WM_APP+1)return DefWindowProcW(w,m,wp,lp);
    style_theme_track_scope scope={TRUE,(BOOL)wp,(HTHEME)lp};
    if(scope.closing)style_theme_track_close_begin(scope.handle);
    else (void)style_theme_track_open_begin();
    process_hook_unwind_scope unwind;
    PROCESS_HOOK_UNWIND_BEGIN(unwind,style_theme_track_scope_end,&scope);
    RaiseException(NTW16S_TEST_EXCEPTION_CODE,0,0,NULL);
    PROCESS_HOOK_UNWIND_END(unwind);return 0;
}
static int check_scope(void)
{
    HINSTANCE module=GetModuleHandleW(NULL);WNDCLASSW cls={.hInstance=module,.lpfnWndProc=scope_proc,.lpszClassName=L"ThemeScopeFixture"};CHECK(RegisterClassW(&cls));
    HWND w=CreateWindowW(cls.lpszClassName,L"",WS_POPUP,0,0,80,40,NULL,NULL,module,NULL);CHECK(w);
    style_theme_track_scope inactive={0};style_theme_track_scope_end(&inactive);
    HTHEME h=(HTHEME)(UINT_PTR)0x7071;opened(h,TRUE);CHECK(style_theme_track_begin(h,STYLE_THEME_BUTTON).eligible);
    CHECK(test_unwind(w,WM_APP+1,TRUE,(LPARAM)h));CHECK(!style_theme_track_begin(h,STYLE_THEME_BUTTON).eligible);
    h=(HTHEME)(UINT_PTR)0x7072;opened(h,TRUE);CHECK(style_theme_track_begin(h,STYLE_THEME_BUTTON).eligible);
    CHECK(test_unwind(w,WM_APP+1,FALSE,0));
    h=(HTHEME)(UINT_PTR)0x7073;opened(h,TRUE);CHECK(!style_theme_track_begin(h,STYLE_THEME_BUTTON).eligible);
    CHECK(DestroyWindow(w) && UnregisterClassW(cls.lpszClassName,module));
    puts("THEME SCOPE PASS inactive=1 native_close_unwind=1 native_open_unwind=1 existing_cancel_policy=1");return 0;
}
int main(int argc, char **argv)
{
    if(argc==2 && !strcmp(argv[1],"scope"))return check_scope();
    CHECK(style_theme_classify(L"BUTTON")==STYLE_THEME_BUTTON);
    CHECK(style_theme_classify(L"combobox")==STYLE_THEME_COMBOBOX);
    CHECK(style_theme_classify(L"TEXTGLOW")==STYLE_THEME_TEXTGLOW);
    CHECK(style_theme_classify(NULL)==STYLE_THEME_OTHER);
    CHECK(style_theme_classify((LPCWSTR)(UINT_PTR)1)==STYLE_THEME_OTHER);
    CHECK(style_theme_classify(L"SCROLLBAR")==STYLE_THEME_OTHER);
    if (argc == 2 && !strcmp(argv[1], "cancel")) {
        (void)style_theme_track_open_begin();
        style_theme_track_open_cancel();
        HTHEME after = (HTHEME)(UINT_PTR)0x7070;
        opened(after, TRUE);
        CHECK(!style_theme_track_begin(after,STYLE_THEME_BUTTON).eligible);
        puts("THEME_TRACK_CANCEL_OK");
        return 0;
    }
    HTHEME button = (HTHEME)(UINT_PTR)0x1010, alias = (HTHEME)(UINT_PTR)0x2020;
    opened(button, TRUE); opened(button, TRUE);
    style_theme_track_ticket draw = style_theme_track_begin(button,STYLE_THEME_BUTTON);
    CHECK(draw.eligible && style_theme_track_end(button, draw,STYLE_THEME_BUTTON));
    closed(button, TRUE);
    CHECK(!style_theme_track_end(button, draw,STYLE_THEME_BUTTON));
    CHECK(style_theme_track_begin(button,STYLE_THEME_BUTTON).eligible);
    draw = style_theme_track_begin(button,STYLE_THEME_BUTTON);
    closed(NULL, FALSE);
    CHECK(!style_theme_track_end(button, draw,STYLE_THEME_BUTTON));
    CHECK(style_theme_track_begin(button,STYLE_THEME_BUTTON).eligible);
    closed(button, TRUE); CHECK(!style_theme_track_begin(button,STYLE_THEME_BUTTON).eligible);
    opened(alias, TRUE); opened(alias, FALSE);
    CHECK(!style_theme_track_begin(alias,STYLE_THEME_BUTTON).eligible);
    closed(alias, TRUE); closed(alias, TRUE);

    HTHEME glow = (HTHEME)(UINT_PTR)0x2121;
    style_theme_track_ticket glow_open=style_theme_track_open_begin();
    style_theme_track_open_end(glow_open,glow,STYLE_THEME_TEXTGLOW);
    style_theme_track_ticket glow_draw=style_theme_track_begin(glow,STYLE_THEME_TEXTGLOW);
    CHECK(glow_draw.eligible && style_theme_track_end(glow,glow_draw,STYLE_THEME_TEXTGLOW));
    CHECK(!style_theme_track_begin(glow,STYLE_THEME_BUTTON).eligible);
    closed(glow,TRUE);CHECK(!style_theme_track_end(glow,glow_draw,STYLE_THEME_TEXTGLOW));

    HTHEME combo=(HTHEME)(UINT_PTR)0x2323;style_theme_track_ticket co=style_theme_track_open_begin();
    style_theme_track_open_end(co,combo,STYLE_THEME_COMBOBOX);style_theme_track_ticket cd=style_theme_track_begin(combo,STYLE_THEME_COMBOBOX);
    CHECK(cd.eligible && style_theme_track_end(combo,cd,STYLE_THEME_COMBOBOX));CHECK(!style_theme_track_begin(combo,STYLE_THEME_BUTTON).eligible);
    closed(combo,TRUE);CHECK(!style_theme_track_end(combo,cd,STYLE_THEME_COMBOBOX));
    co=style_theme_track_open_begin();style_theme_track_open_end(co,combo,STYLE_THEME_COMBOBOX);opened(combo,TRUE);
    CHECK(!style_theme_track_begin(combo,STYLE_THEME_COMBOBOX).eligible && !style_theme_track_begin(combo,STYLE_THEME_BUTTON).eligible);closed(combo,TRUE);closed(combo,TRUE);

    HTHEME failed = (HTHEME)(UINT_PTR)0x3030;
    opened(failed, TRUE); CHECK(style_theme_track_begin(failed,STYLE_THEME_BUTTON).eligible);
    closed(failed, FALSE); opened(failed, TRUE);
    CHECK(!style_theme_track_begin(failed,STYLE_THEME_BUTTON).eligible);

    HTHEME nested = (HTHEME)(UINT_PTR)0x4040;
    style_theme_track_ticket outer = style_theme_track_open_begin();
    style_theme_track_ticket inner = style_theme_track_open_begin();
    style_theme_track_open_end(inner, nested, TRUE);
    style_theme_track_open_end(outer, nested, TRUE);
    CHECK(!style_theme_track_begin(nested,STYLE_THEME_BUTTON).eligible);

    overlap o = {CreateEventW(NULL, TRUE, FALSE, NULL), CreateEventW(NULL, TRUE, FALSE, NULL), (HTHEME)(UINT_PTR)0x5050};
    CHECK(o.start && o.done);
    HANDLE thread = CreateThread(NULL, 0, close_thread, &o, 0, NULL); CHECK(thread);
    outer = style_theme_track_open_begin(); SetEvent(o.start);
    CHECK(WaitForSingleObject(o.done, 2000) == WAIT_OBJECT_0);
    style_theme_track_open_end(outer, o.h, TRUE);
    CHECK(WaitForSingleObject(thread, 2000) == WAIT_OBJECT_0);
    DWORD code = 1; CHECK(GetExitCodeThread(thread, &code) && code == 0);
    CloseHandle(thread); CloseHandle(o.start); CloseHandle(o.done);
    CHECK(!style_theme_track_begin(o.h,STYLE_THEME_BUTTON).eligible);

    HTHEME safe = (HTHEME)(UINT_PTR)0x6060; opened(safe, TRUE);
    CHECK(style_theme_track_begin(safe,STYLE_THEME_BUTTON).eligible);
    for (unsigned i = 0; i < 140; ++i) opened((HTHEME)(UINT_PTR)(0x100000 + i), TRUE);
    CHECK(!style_theme_track_begin(safe,STYLE_THEME_BUTTON).eligible);
    puts("THEME_TRACK_OK shared=1 alias=1 combo=1 failed_close=1 nested=1 overlap=1 capacity=1");
    return 0;
}
