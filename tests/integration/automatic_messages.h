#ifndef AUTOMATIC_MESSAGES_H
#define AUTOMATIC_MESSAGES_H
/* The observer closes only this fixture's own modal dialogs, on its GUI thread. */
typedef struct message_button {HWND window;unsigned count;} message_button;
static BOOL CALLBACK find_message_button(HWND w,LPARAM data)
{
    message_button *button=(message_button *)data;WCHAR name[40];
    CHECK(GetClassNameW(w,name,ARRAYSIZE(name)));
    if(!lstrcmpiW(name,L"BUTTON")){button->window=w;++button->count;}
    return TRUE;
}
static void CALLBACK close_message(HWND w,UINT message,UINT_PTR timer,DWORD time)
{
    (void)message;(void)time;
    CHECK(KillTimer(w,timer));observe(w);
    /* Native MB_OK can use internal child ID2 and still return public IDOK.
     * The public contract is its one button and result, not a private ID. */
    message_button button={0};EnumChildWindows(w,find_message_button,(LPARAM)&button);
    CHECK(button.count==1);observe(button.window);
    record("message-dialog-observed");CHECK(PostMessageW(w,WM_CLOSE,0,0));
}
static LRESULT CALLBACK modal_observer(int code,WPARAM wp,LPARAM lp)
{
    if(code==HCBT_ACTIVATE){WCHAR name[40],title[120];
        CHECK(GetClassNameW((HWND)wp,name,ARRAYSIZE(name)));
        GetWindowTextW((HWND)wp,title,ARRAYSIZE(title));
        if(!wcscmp(name,L"#32770") && !wcscmp(title,L"Native message"))CHECK(SetTimer((HWND)wp,0x7731,50,close_message));}
    return CallNextHookEx(NULL,code,wp,lp);
}
static void messages(void)
{
    for(unsigned i=0;i<6;++i){
        HHOOK hook=SetWindowsHookExW(WH_CBT,modal_observer,NULL,GetCurrentThreadId());int result=0;CHECK(hook);
        switch(i){
        case 0:result=MessageBoxA(NULL,"Native text","Native message",MB_OK);break;
        case 1:result=MessageBoxW(NULL,L"Native text",L"Native message",MB_OK);break;
        case 2:result=MessageBoxExA(NULL,"Native text","Native message",MB_OK,0);break;
        case 3:result=MessageBoxExW(NULL,L"Native text",L"Native message",MB_OK,0);break;
        case 4:{MSGBOXPARAMSA p={.cbSize=sizeof(p)};p.lpszText="Native text";p.lpszCaption="Native message";result=MessageBoxIndirectA(&p);break;}
        case 5:{MSGBOXPARAMSW p={.cbSize=sizeof(p)};p.lpszText=L"Native text";p.lpszCaption=L"Native message";result=MessageBoxIndirectW(&p);break;}
        }
        CHECK(result==IDOK && UnhookWindowsHookEx(hook));
    }
    record("six-message-APIs-complete");
}
#endif
