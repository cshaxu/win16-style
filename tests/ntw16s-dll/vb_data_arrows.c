/* Isolated runtime model: verifies the Data navigation render, not DAO semantics. */
#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include "isolated_process.h"
#define CHECK(x) do { if (!(x)) { fprintf(stderr,"VB_DATA_ARROW_FAIL line=%d error=%lu %s\n",__LINE__,GetLastError(),#x); return 1; } } while (0)
#define W 140
#define H 22
typedef struct capture { DWORD pixels[2][4][W*H]; RECT client[2]; BOOL ready[2]; } capture;
static unsigned notices;
static LRESULT CALLBACK parent_proc(HWND w,UINT m,WPARAM a,LPARAM b) { if(m==WM_COMMAND && b) ++notices; return DefWindowProcW(w,m,a,b); }
static int child(const WCHAR *path,BOOL styled) {
 HMODULE runtime=LoadLibraryW(path); CHECK(runtime && runtime==GetModuleHandleW(L"MSVBVM60.DLL"));
 BOOL (WINAPI *reg)(void)=(void*)GetProcAddress(runtime,"VbDataRegister"), (WINAPI *release)(void)=(void*)GetProcAddress(runtime,"VbDataRelease");
 BOOL (WINAPI *inspect)(HWND,BOOL*,unsigned*,unsigned*)=(void*)GetProcAddress(runtime,"VbDataInspect"); CHECK(reg && release && inspect && reg());
 HMODULE dll=GetModuleHandleW(sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll"); BOOL (__cdecl *attached)(HWND)=dll?(void*)GetProcAddress(dll,"Ntw16sWindowAttached"):NULL; CHECK(styled?attached!=NULL:dll==NULL);
 HINSTANCE self=GetModuleHandleW(NULL); WNDCLASSW c={.hInstance=self,.lpfnWndProc=parent_proc,.lpszClassName=L"VBDataTestParent"}; CHECK(RegisterClassW(&c));
 HWND root=CreateWindowW(c.lpszClassName,L"",WS_POPUP|WS_VISIBLE,0,0,280,150,NULL,NULL,self,NULL); CHECK(root);
 HWND w=CreateWindowExW(WS_EX_CLIENTEDGE|WS_EX_NOPARENTNOTIFY,L"ThunderRT6Data",L"",WS_CHILD|WS_VISIBLE|WS_BORDER,0,0,W+6,H+6,root,(HMENU)1,runtime,NULL); CHECK(w);
 CHECK(RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW)); CHECK(!styled || attached(w));
 RECT rc,wr; POINT pt={0,0}; CHECK(GetClientRect(w,&rc) && GetWindowRect(w,&wr) && ClientToScreen(w,&pt)); CHECK(rc.right==W && rc.bottom==H);
 HFONT font=(HFONT)SendMessageW(w,WM_GETFONT,0,0); CHECK(font);
 WCHAR name[128]; CHECK(GetEnvironmentVariableW(L"WIN16STYLE_VB_DATA_ARROW_CAPTURE",name,128)); HANDLE map=OpenFileMappingW(FILE_MAP_WRITE,FALSE,name); CHECK(map); capture *cap=MapViewOfFile(map,FILE_MAP_WRITE,0,0,sizeof(*cap)); CHECK(cap); cap->client[styled]=rc;
 HDC screen=GetDC(w),dc=CreateCompatibleDC(screen); BITMAPINFO info={0}; info.bmiHeader=(BITMAPINFOHEADER){.biSize=sizeof(BITMAPINFOHEADER),.biWidth=W,.biHeight=-H,.biPlanes=1,.biBitCount=32}; DWORD *pixels=NULL; HBITMAP bitmap=CreateDIBSection(screen,&info,DIB_RGB_COLORS,(void**)&pixels,NULL,0); CHECK(screen && dc && bitmap); HGDIOBJ old=SelectObject(dc,bitmap); CHECK(old);
 BOOL pressed; unsigned clicks,cancels;
 for(unsigned state=0;state<4;state++) {
  if(state==1){SendMessageW(w,WM_LBUTTONDOWN,MK_LBUTTON,MAKELPARAM(8,8)); CHECK(GetCapture()==w && inspect(w,&pressed,&clicks,&cancels) && pressed);}
  if(state==2){SendMessageW(w,WM_CANCELMODE,0,0); CHECK(GetCapture()!=w && inspect(w,&pressed,&clicks,&cancels) && !pressed && cancels==1);}
  if(state==3){EnableWindow(w,FALSE); CHECK(!IsWindowEnabled(w));}
  CHECK(RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW)); for(int i=0;i<W*H;i++)pixels[i]=0xffffffff;
  CHECK(BitBlt(dc,0,0,W,H,screen,0,0,SRCCOPY)); GdiFlush(); CopyMemory(cap->pixels[styled][state],pixels,sizeof(cap->pixels[0][0]));
 }
 EnableWindow(w,TRUE); CHECK(IsWindowEnabled(w)); int positions[4]={8,26,W-27,W-9};
 for(int i=0;i<4;++i){SendMessageW(w,WM_LBUTTONDOWN,MK_LBUTTON,MAKELPARAM(positions[i],8));CHECK(GetCapture()==w);SendMessageW(w,WM_LBUTTONUP,0,MAKELPARAM(positions[i],8));CHECK(GetCapture()!=w);}
 CHECK(inspect(w,&pressed,&clicks,&cancels) && !pressed && clicks==4 && notices==4);
 if(styled){
  /* A caller outside the runtime must not replace surrounding pixels. */
  HDC native=CreateCompatibleDC(screen);HBITMAP source=CreateCompatibleBitmap(screen,11,13);CHECK(native && source);HGDIOBJ previous=SelectObject(native,source);CHECK(previous);RECT all={0,0,11,13},bar={1,1,3,12};FillRect(native,&all,GetStockObject(WHITE_BRUSH));FillRect(native,&bar,GetStockObject(BLACK_BRUSH));CHECK(SetPixelV(screen,0,0,RGB(255,0,0)));CHECK(StretchBlt(screen,4,4,11,13,native,0,0,11,13,SRCCOPY));CHECK(GetPixel(screen,0,0)==RGB(255,0,0));CHECK(SelectObject(native,previous) && DeleteObject(source) && DeleteDC(native));
  /* The overlay must respect the runtime's smaller destination clip. */
  CHECK(ValidateRect(w,NULL));CHECK(PatBlt(screen,0,0,W,H,WHITENESS));CHECK(SetPixelV(screen,10,10,RGB(255,0,0)));SendMessageW(w,WM_APP+91,TRUE,0);CHECK(InvalidateRect(w,NULL,FALSE));SendMessageW(w,WM_PAINT,0,0);CHECK(GetPixel(screen,10,10)==RGB(255,0,0));SendMessageW(w,WM_APP+91,FALSE,0);
 }
 CHECK((HFONT)SendMessageW(w,WM_GETFONT,0,0)==font); cap->ready[styled]=TRUE;
 CHECK(UnmapViewOfFile(cap) && CloseHandle(map) && SelectObject(dc,old) && DeleteObject(bitmap) && DeleteDC(dc) && ReleaseDC(w,screen));
 HWND rtl=CreateWindowExW(WS_EX_CLIENTEDGE|WS_EX_LAYOUTRTL,L"ThunderRT6Data",L"",WS_CHILD|WS_VISIBLE|WS_BORDER,0,40,W,H,root,(HMENU)2,runtime,NULL); CHECK(rtl); SendMessageW(rtl,WM_PAINT,0,0); CHECK(!styled || !attached(rtl)); CHECK(DestroyWindow(rtl));
 HWND scroll=CreateWindowExW(WS_EX_CLIENTEDGE,L"ThunderRT6Data",L"",WS_CHILD|WS_VISIBLE|WS_BORDER|WS_HSCROLL,0,40,W,H,root,(HMENU)3,runtime,NULL); CHECK(scroll); SendMessageW(scroll,WM_PAINT,0,0); CHECK(!styled || !attached(scroll)); CHECK(DestroyWindow(scroll));
 WNDCLASSW fake; CHECK(GetClassInfoW(runtime,L"ThunderRT6Data",&fake)); fake.hInstance=self; CHECK(RegisterClassW(&fake)); HWND wrong=CreateWindowExW(WS_EX_CLIENTEDGE,L"ThunderRT6Data",L"",WS_CHILD|WS_VISIBLE|WS_BORDER,0,40,W,H,root,(HMENU)4,self,NULL); CHECK(wrong); SendMessageW(wrong,WM_PAINT,0,0); CHECK(!styled || !attached(wrong)); CHECK(DestroyWindow(wrong) && UnregisterClassW(L"ThunderRT6Data",self));
 if(styled){ HWND other=CreateWindowW(c.lpszClassName,L"",WS_POPUP|WS_VISIBLE,0,0,150,100,NULL,NULL,self,NULL); CHECK(other && SetParent(w,other)==root); SendMessageW(w,WM_NULL,0,0); CHECK(!attached(w)); CHECK(DestroyWindow(w) && DestroyWindow(other)); } else CHECK(DestroyWindow(w));
 CHECK(DestroyWindow(root) && UnregisterClassW(c.lpszClassName,self) && release() && FreeLibrary(runtime)); return 0;
}
static int worker(int argc,WCHAR **argv) {
 CHECK(argc==3); WCHAR self[MAX_PATH],cmd[2048],name[128]; CHECK(GetModuleFileNameW(NULL,self,MAX_PATH)); CHECK(swprintf_s(name,128,L"Local\\VBDataArrowCapture-%lu-%llu",GetCurrentProcessId(),(unsigned long long)GetTickCount64())>0);
 HANDLE map=CreateFileMappingW(INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE,0,sizeof(capture),name); CHECK(map && GetLastError()!=ERROR_ALREADY_EXISTS); capture *cap=MapViewOfFile(map,FILE_MAP_READ|FILE_MAP_WRITE,0,0,sizeof(*cap)); CHECK(cap && SetEnvironmentVariableW(L"WIN16STYLE_VB_DATA_ARROW_CAPTURE",name)); ZeroMemory(cap,sizeof(*cap));
 for(unsigned mode=0;mode<2;mode++) {
  if(mode)swprintf_s(cmd,2048,L"\"%ls\" --wait \"%ls\" --child \"%ls\" 1",argv[1],self,argv[2]); else swprintf_s(cmd,2048,L"\"%ls\" --child \"%ls\" 0",self,argv[2]);
  HANDLE job=CreateJobObjectW(NULL,NULL); JOBOBJECT_EXTENDED_LIMIT_INFORMATION limit={0}; limit.BasicLimitInformation.LimitFlags=JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE; CHECK(job && SetInformationJobObject(job,JobObjectExtendedLimitInformation,&limit,sizeof(limit)));
  STARTUPINFOW si={.cb=sizeof(si),.dwFlags=STARTF_USESTDHANDLES,.hStdInput=GetStdHandle(STD_INPUT_HANDLE),.hStdOutput=GetStdHandle(STD_OUTPUT_HANDLE),.hStdError=GetStdHandle(STD_ERROR_HANDLE)}; PROCESS_INFORMATION p={0}; CHECK(CreateProcessW(NULL,cmd,NULL,NULL,TRUE,CREATE_SUSPENDED|CREATE_NO_WINDOW,NULL,NULL,&si,&p)); CHECK(AssignProcessToJobObject(job,p.hProcess) && ResumeThread(p.hThread)==1); DWORD wait=WaitForSingleObject(p.hProcess,10000),code=1; if(wait==WAIT_OBJECT_0)GetExitCodeProcess(p.hProcess,&code); CloseHandle(p.hThread); CloseHandle(p.hProcess); CloseHandle(job); CHECK(wait==WAIT_OBJECT_0 && !code);
 }
 CHECK(cap->ready[0] && cap->ready[1] && EqualRect(&cap->client[0],&cap->client[1])); RECT r=cap->client[0]; CHECK(r.left>=0 && r.top>=0 && r.right<=W && r.bottom<=H && r.right>r.left && r.bottom>r.top);
 for(int state=0;state<4;state++){unsigned middle=0,buttons[4]={0}; for(int y=0;y<H;y++)for(int x=0;x<W;x++){BOOL different=(cap->pixels[0][state][y*W+x]&0xffffff)!=(cap->pixels[1][state][y*W+x]&0xffffff);if(x>=35 && x<W-35)middle+=different;else if(x<18)buttons[0]+=different;else if(x<35)buttons[1]+=different;else if(x<W-18)buttons[2]+=different;else buttons[3]+=different;} CHECK(!middle && buttons[0] && buttons[1] && buttons[2] && buttons[3]);}
 for(int mode=0;mode<2;mode++){unsigned changed=0,restored=0;for(int i=0;i<W*H;i++){changed+=(cap->pixels[mode][0][i]&0xffffff)!=(cap->pixels[mode][1][i]&0xffffff);restored+=(cap->pixels[mode][0][i]&0xffffff)!=(cap->pixels[mode][2][i]&0xffffff);}CHECK(changed && !restored);}
 printf("VB_DATA_ARROW_OK bits=%u states=4 middle_preserved=1 input=4 wrong_caller=1 clip=1 wrong_owner=1 unsupported=1 reparent=1\n",(unsigned)(8*sizeof(void*))); CHECK(UnmapViewOfFile(cap) && CloseHandle(map) && SetEnvironmentVariableW(L"WIN16STYLE_VB_DATA_ARROW_CAPTURE",NULL)); return 0;
}
int wmain(int argc,WCHAR **argv){if(argc==4 && !wcscmp(argv[1],L"--child"))return child(argv[2],!wcscmp(argv[3],L"1"));return isolated_process(argc,argv,worker);}
