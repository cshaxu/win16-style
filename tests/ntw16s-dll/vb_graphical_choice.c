#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include "isolated_process.h"
#define CHECK(x) do{if(!(x)){fprintf(stderr,"VB_GRAPHICAL_CHOICE_FAIL line=%d error=%lu %s\n",__LINE__,GetLastError(),#x);return 1;}}while(0)
static unsigned image_color_pixels;
static LRESULT CALLBACK parent(HWND w,UINT m,WPARAM a,LPARAM b)
{return DefWindowProcW(w,m,a,b);}
static DWORD image(HWND w)
{
    RECT r;BITMAPINFO info={0};DWORD *bits=NULL;CHECK(GetClientRect(w,&r));
    HDC screen=GetDC(w),dc=CreateCompatibleDC(screen);info.bmiHeader=(BITMAPINFOHEADER){.biSize=sizeof(info.bmiHeader),.biWidth=r.right,.biHeight=-r.bottom,.biPlanes=1,.biBitCount=32};
    HBITMAP bitmap=CreateDIBSection(screen,&info,DIB_RGB_COLORS,(void **)&bits,NULL,0);CHECK(screen&&dc&&bitmap);
    HGDIOBJ old=SelectObject(dc,bitmap);SendMessageW(w,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT);GdiFlush();DWORD hash=2166136261u;
    image_color_pixels=0;
    for(int i=0;i<r.right*r.bottom;++i){DWORD pixel=bits[i]&0xffffff;if(pixel==0x00e01020)++image_color_pixels;hash^=pixel;hash*=16777619u;}
    CHECK(SelectObject(dc,old)&&DeleteObject(bitmap)&&DeleteDC(dc)&&ReleaseDC(w,screen));return hash;
}
static HBITMAP red_bitmap(HDC reference)
{
    BITMAPINFO info={.bmiHeader={.biSize=sizeof(info.bmiHeader),.biWidth=16,.biHeight=-16,.biPlanes=1,.biBitCount=32}};
    DWORD *bits=NULL;HBITMAP bitmap=CreateDIBSection(reference,&info,DIB_RGB_COLORS,(void **)&bits,NULL,0);
    if(!bitmap)return NULL;
    for(unsigned i=0;i<16u*16u;++i)bits[i]=0x00e01020;
    return bitmap;
}
static HICON red_icon(HDC reference)
{
    HBITMAP color=red_bitmap(reference),mask=CreateBitmap(16,16,1,1,NULL);ICONINFO info={.fIcon=TRUE,.hbmMask=mask,.hbmColor=color};
    HICON icon=color&&mask?CreateIconIndirect(&info):NULL;
    if(mask)DeleteObject(mask);
    if(color)DeleteObject(color);
    return icon;
}
static int worker(int argc,WCHAR **argv)
{
    CHECK(argc==3);HMODULE runtime=LoadLibraryW(argv[2]);CHECK(runtime);
    BOOL (WINAPI *register_runtime)(void)=(void *)GetProcAddress(runtime,"VbRegister");void (WINAPI *release_runtime)(void)=(void *)GetProcAddress(runtime,"VbRelease");CHECK(register_runtime&&release_runtime&&register_runtime());
    HMODULE style=GetModuleHandleW(sizeof(void *)==8?L"ntw16s64.dll":L"ntw16s32.dll");BOOL (__cdecl *attached)(HWND)=style?(void *)GetProcAddress(style,"Ntw16sWindowAttached"):NULL;BOOL (__cdecl *remove)(HWND)=style?(void *)GetProcAddress(style,"Ntw16sWindowRemove"):NULL;
    WNDCLASSW cls={.hInstance=GetModuleHandleW(NULL),.lpfnWndProc=parent,.lpszClassName=L"VBGraphicalChoiceParent"};CHECK(RegisterClassW(&cls));
    HWND root=CreateWindowW(cls.lpszClassName,L"choice",WS_POPUP,0,0,260,100,NULL,NULL,cls.hInstance,NULL);CHECK(root);
    HWND choice=CreateWindowW(L"ThunderRT6CheckBox",L"Graphical",WS_CHILD|WS_VISIBLE|BS_AUTO3STATE|BS_PUSHLIKE,0,0,150,28,root,NULL,runtime,NULL);CHECK(choice);ShowWindow(root,SW_SHOWNOACTIVATE);
    HWND option_a=CreateWindowW(L"ThunderRT6OptionButton",L"A",WS_CHILD|WS_VISIBLE|WS_GROUP|BS_RADIOBUTTON|BS_PUSHLIKE,0,32,70,28,root,NULL,runtime,NULL);
    HWND option_b=CreateWindowW(L"ThunderRT6OptionButton",L"B",WS_CHILD|WS_VISIBLE|BS_RADIOBUTTON|BS_PUSHLIKE,75,32,70,28,root,NULL,runtime,NULL);CHECK(option_a&&option_b);
    if(style){CHECK(attached&&remove&&attached(choice));DWORD styled=image(choice);SendMessageW(choice,BM_SETCHECK,BST_CHECKED,0);CHECK(SendMessageW(choice,BM_GETCHECK,0,0)==BST_CHECKED);DWORD checked=image(choice);CHECK(styled!=checked);CHECK(remove(choice)&&!attached(choice)&&image(choice)!=checked);puts("VB_GRAPHICAL_CHOICE_OK styled=1 check_state=1 detach_native=1");}
    else {CHECK(!style);(void)image(choice);SendMessageW(choice,BM_SETCHECK,BST_CHECKED,0);CHECK(SendMessageW(choice,BM_GETCHECK,0,0)==BST_CHECKED);(void)image(choice);puts("VB_GRAPHICAL_CHOICE_OK styled=0 check_state=1 detach_native=1");}
    HDC screen=GetDC(choice);CHECK(screen);HBITMAP bitmap=red_bitmap(screen);CHECK(ReleaseDC(choice,screen)&&bitmap);
    HWND picture=CreateWindowW(L"ThunderRT6CheckBox",L"",WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX|BS_PUSHLIKE|BS_BITMAP,150,0,100,28,root,NULL,runtime,NULL);CHECK(picture);
    CHECK((HBITMAP)SendMessageW(picture,BM_SETIMAGE,IMAGE_BITMAP,(LPARAM)bitmap)==NULL);
    if(style){CHECK(attached(picture));DWORD normal=image(picture);CHECK(image_color_pixels==256);CHECK((HBITMAP)SendMessageW(picture,BM_GETIMAGE,IMAGE_BITMAP,0)==bitmap);SendMessageW(picture,BM_CLICK,0,0);CHECK(SendMessageW(picture,BM_GETCHECK,0,0)==BST_CHECKED&&image(picture)!=normal&&image_color_pixels==256);EnableWindow(picture,FALSE);CHECK(image(picture)!=normal);EnableWindow(picture,TRUE);CHECK(remove(picture)&&!attached(picture));}
    else {CHECK(!style);}
    CHECK((HBITMAP)SendMessageW(picture,BM_GETIMAGE,IMAGE_BITMAP,0)==bitmap);CHECK(DestroyWindow(picture));CHECK(DeleteObject(bitmap));puts("VB_GRAPHICAL_PICTURE_OK borrowed=1 state=1 disabled=1 native_retire=1");
    screen=GetDC(choice);CHECK(screen);HICON icon=red_icon(screen);CHECK(ReleaseDC(choice,screen)&&icon);
    HWND icon_choice=CreateWindowW(L"ThunderRT6CheckBox",L"",WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX|BS_PUSHLIKE|BS_ICON,150,32,100,28,root,NULL,runtime,NULL);CHECK(icon_choice);
    CHECK((HICON)SendMessageW(icon_choice,BM_SETIMAGE,IMAGE_ICON,(LPARAM)icon)==NULL);
    if(style){CHECK(attached(icon_choice));DWORD normal=image(icon_choice);CHECK(image_color_pixels==256);CHECK((HICON)SendMessageW(icon_choice,BM_GETIMAGE,IMAGE_ICON,0)==icon);SendMessageW(icon_choice,BM_CLICK,0,0);CHECK(SendMessageW(icon_choice,BM_GETCHECK,0,0)==BST_CHECKED&&image(icon_choice)!=normal&&image_color_pixels==256);EnableWindow(icon_choice,FALSE);CHECK(image(icon_choice)!=normal);EnableWindow(icon_choice,TRUE);CHECK(remove(icon_choice)&&!attached(icon_choice));}
    else {CHECK(!style);}
    CHECK((HICON)SendMessageW(icon_choice,BM_GETIMAGE,IMAGE_ICON,0)==icon);CHECK(DestroyWindow(icon_choice));CHECK(DestroyIcon(icon));puts("VB_GRAPHICAL_ICON_OK borrowed=1 state=1 disabled=1 native_retire=1");
    if(style)puts("VB_GRAPHICAL_IMAGE_PIXELS_OK bitmap=256 icon=256 checked=1");
    if(style){CHECK(attached(option_a)&&attached(option_b));SendMessageW(option_a,BM_CLICK,0,0);LRESULT a1=SendMessageW(option_a,BM_GETCHECK,0,0),b1=SendMessageW(option_b,BM_GETCHECK,0,0);DWORD first=image(option_a);SendMessageW(option_b,BM_CLICK,0,0);LRESULT a2=SendMessageW(option_a,BM_GETCHECK,0,0),b2=SendMessageW(option_b,BM_GETCHECK,0,0);DWORD second=image(option_b);printf("VB_GRAPHICAL_OPTION_TRACE a1=%lld b1=%lld a2=%lld b2=%lld first=%08lx second=%08lx\n",(long long)a1,(long long)b1,(long long)a2,(long long)b2,first,second);CHECK(a1==BST_CHECKED&&b1==BST_UNCHECKED&&b2==BST_CHECKED&&a2==BST_UNCHECKED);CHECK(remove(option_a)&&remove(option_b));}
    else {SendMessageW(option_a,BM_CLICK,0,0);CHECK(SendMessageW(option_a,BM_GETCHECK,0,0)==BST_CHECKED);SendMessageW(option_b,BM_CLICK,0,0);CHECK(SendMessageW(option_b,BM_GETCHECK,0,0)==BST_CHECKED&&SendMessageW(option_a,BM_GETCHECK,0,0)==BST_UNCHECKED);}
    puts("VB_GRAPHICAL_OPTION_OK runtime_grouping=1 native_retire=1");
    HWND array[3]={0};for(unsigned i=0;i<ARRAYSIZE(array);++i)array[i]=CreateWindowW(L"ThunderRT6CheckBox",L"Array",WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX|BS_PUSHLIKE,0,64+(int)i*10,70,20,root,(HMENU)(UINT_PTR)(100+i),runtime,NULL);
    CHECK(array[0]&&array[1]&&array[2]);SendMessageW(array[0],BM_SETCHECK,BST_CHECKED,0);SendMessageW(array[2],BM_SETCHECK,BST_INDETERMINATE,0);
    if(style){CHECK(attached(array[0])&&attached(array[1])&&attached(array[2]));}else CHECK(!style);
    CHECK(DestroyWindow(array[1])&&SendMessageW(array[0],BM_GETCHECK,0,0)==BST_CHECKED&&SendMessageW(array[2],BM_GETCHECK,0,0)==BST_INDETERMINATE);
    array[1]=CreateWindowW(L"ThunderRT6CheckBox",L"Array",WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX|BS_PUSHLIKE,75,74,70,20,root,(HMENU)101,runtime,NULL);CHECK(array[1]);
    if(style){CHECK(attached(array[0])&&attached(array[1])&&attached(array[2]));CHECK(remove(array[0])&&!attached(array[0])&&attached(array[1])&&attached(array[2]));}
    CHECK(DestroyWindow(array[0])&&DestroyWindow(array[1])&&DestroyWindow(array[2]));puts("VB_GRAPHICAL_ARRAY_OK independent_state=1 sibling_retire=1 recreate=1");
    CHECK(DestroyWindow(root)&&UnregisterClassW(cls.lpszClassName,cls.hInstance));release_runtime();FreeLibrary(runtime);return 0;
}
int wmain(int argc,WCHAR **argv){return isolated_process(argc,argv,worker);}
