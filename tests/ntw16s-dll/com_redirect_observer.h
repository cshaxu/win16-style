#ifndef TEST_COM_REDIRECT_OBSERVER_H
#define TEST_COM_REDIRECT_OBSERVER_H

static BOOL CALLBACK find(HWND w,LPARAM x){(void)x;WCHAR title[80];GetWindowTextW(w,title,80);if(IsWindowVisible(w)&&!lstrcmpW(title,L"Win16S redirect probe")){old=GetDlgItem(w,1121)!=NULL;seen=1;SetTimer(w,985,200,tick);return FALSE;}return TRUE;}
static DWORD WINAPI observe(void *x){(void)x;for(int i=0;i<100;++i){if(!seen)EnumThreadWindows(ui,find,0);if(done)return 0;Sleep(50);}PostThreadMessageW(ui,WM_QUIT,0,0);return 1;}

#endif
