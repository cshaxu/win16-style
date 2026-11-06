#include "ntw16s-dll/injection/injection.h"
#include "elevation.h"
#include "lib-win32/process-launch/command_line.h"
#include "process_lifecycle.h"
#include "startup_error.h"
#include "global.h"
#include "tray.h"
#include "uwp.h"
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <errno.h>
/* MinGW CRT must preserve native arguments. Globbing otherwise interprets the
 * '?' in explicit extended paths (\\?\) before target resolution. */
int _dowildcard=0;
static DWORD prepare_target(HANDLE process,void *context)
{return ntw16s_install(process,context,CreateProcessW);}
static int diagnostic(int code,DWORD error,PCWSTR phase)
{
    WCHAR message[512]={0};
    FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,NULL,error,0,message,512,NULL);
    fwprintf(stderr,L"run16s: %ls failed (%lu): %ls\n",phase,error,message);return code;
}
int wmain(int argc,WCHAR **argv)
{
    if(argc>=2 && !wcscmp(argv[1],L"--app")){
        if(argc!=3){fwprintf(stderr,L"Usage: run16s.exe --app <AUMID>\n");return 2;}
        DWORD error=run16s_uwp_activate(argv[2],NULL);return error?diagnostic(10,error,L"UWP frame activation"):0;
    }
    if(argc>=2 && !wcscmp(argv[1],L"--global-elevated")){
        WCHAR *end=NULL;DWORD error=0;errno=0;
        unsigned long parent=argc==3?wcstoul(argv[2],&end,10):0;
        if(argc!=3 || argv[2][0]<L'0' || argv[2][0]>L'9' || !parent || errno || !end || *end)
            return diagnostic(9,ERROR_INVALID_PARAMETER,L"global elevation");
        if(!run16s_is_elevated(&error))return diagnostic(9,error?error:ERROR_ACCESS_DENIED,L"global elevation");
        HANDLE owner=OpenProcess(SYNCHRONIZE,FALSE,(DWORD)parent);
        if(!owner)return diagnostic(9,GetLastError(),L"global parent");
        if(!FreeConsole() && GetLastError()!=ERROR_INVALID_HANDLE){error=GetLastError();CloseHandle(owner);return diagnostic(9,error,L"global console detach");}
        if(!AttachConsole((DWORD)parent)){error=GetLastError();CloseHandle(owner);return diagnostic(9,error,L"global console attach");}
        run16s_rebind_console_stderr();
        if(!SetConsoleCtrlHandler(NULL,FALSE)){error=GetLastError();CloseHandle(owner);return diagnostic(9,error,L"global console handler");}
        int result=run16s_tray_global(owner);CloseHandle(owner);return result;
    }
    if(argc>=2 && !wcscmp(argv[1],L"--global")){
        if(argc!=2){fwprintf(stderr,L"Usage: run16s.exe --global\n");return 2;}
        DWORD error=0,code=0;
        BOOL elevated=run16s_is_elevated(&error);
        if(error)return diagnostic(9,error,L"global token");
        /* Only hide a console belonging exclusively to this launcher. An
         * existing cmd/terminal console must remain visible and untouched. */
        HWND private_window=run16s_private_console()?GetConsoleWindow():NULL;
        if(private_window)ShowWindow(private_window,SW_HIDE);
        if(!elevated && run16s_global_interactive_console()){
            error=run16s_elevate_global(GetCurrentProcessId(),ShellExecuteExW,&code);
            if(private_window && (error || code))ShowWindow(private_window,SW_SHOW);
            return error?diagnostic(9,error,L"global elevation"):(int)code;
        }
        int result=run16s_tray_global(NULL);
        if(private_window && result)ShowWindow(private_window,SW_SHOW);
        return result;
    }
    BOOL private_console=run16s_private_console();
    int first=1;BOOL wait=FALSE,reentered=FALSE;DWORD error,exit_code=0;WORD subsystem=0;
    WCHAR image[NTW16S_PATH_CAP];ntw16s_context context;
    STARTUPINFOW startup={.cb=sizeof(startup)};PROCESS_INFORMATION child={0};
    PCWSTR raw=win32_process_next_argument(GetCommandLineW());PWSTR command;
    if(argc>first && !wcscmp(argv[first],L"--elevated")){
        reentered=TRUE;++first;raw=win32_process_next_argument(raw);
        if(!run16s_is_elevated(&error))return diagnostic(9,error?error:ERROR_ACCESS_DENIED,L"elevated launcher token");
    }
    if(argc>first && !wcscmp(argv[first],L"--wait")){wait=TRUE;++first;raw=win32_process_next_argument(raw);}
    if(argc<=first || !wcscmp(argv[first],L"--help")){
        fwprintf(argc<=first?stderr:stdout,L"Usage: run16s.exe [--wait] <application> [arguments]\n       run16s.exe --global\n       run16s.exe --app <AUMID>\nConsole targets wait by default; --wait also waits for GUI targets.\n");
        return argc<=first?2:0;
    }
    DWORD length=SearchPathW(NULL,argv[first],L".exe",NTW16S_PATH_CAP,image,NULL);
    if(!length || length>=NTW16S_PATH_CAP)return diagnostic(3,length?ERROR_FILENAME_EXCED_RANGE:GetLastError(),L"target resolution");
    error=ntw16s_validate_image_ex(image,IMAGE_FILE_MACHINE_AMD64,FALSE,&subsystem);
    if(error==ERROR_BAD_EXE_FORMAT)error=ntw16s_validate_image_ex(image,IMAGE_FILE_MACHINE_I386,FALSE,&subsystem);
    if(error){
        if(error==ERROR_BAD_EXE_FORMAT && private_console && run16s_legacy_image(image)){
            /* No target has been created. Choose one error channel, retaining
             * the initial validation exit code even if Shell returns 1223. */
            if(FreeConsole()){
                run16s_system_error(image,win32_process_next_argument(raw),ShellExecuteExW);
                return 4;
            }
        }
        return diagnostic(4,error,L"native target validation");
    }
    /* Keep the parent shell blocked for interactive console targets. GUI
     * targets stay asynchronous unless --wait explicitly requests otherwise. */
    wait=wait || subsystem==IMAGE_SUBSYSTEM_WINDOWS_CUI;
    error=ntw16s_context_paths(&context,NULL);if(error)return diagnostic(6,error,L"package directory");
    command=_wcsdup(raw);if(!command)return diagnostic(5,ERROR_NOT_ENOUGH_MEMORY,L"command allocation");
    error=win32_process_create_suspended(image,command,NULL,NULL,FALSE,0,NULL,NULL,&startup,&child);
    if(error){
        free(command);
        if(error==ERROR_ELEVATION_REQUIRED && !reentered){
            DWORD token_error=0;BOOL elevated=run16s_is_elevated(&token_error);
            if(token_error)return diagnostic(9,token_error,L"launcher token");
            if(!elevated){
                error=run16s_elevate(image,win32_process_next_argument(raw),wait,ShellExecuteExW,&exit_code);
                if(error)return diagnostic(error==ERROR_CANCELLED?ERROR_CANCELLED:9,error,L"UAC elevation");
                return (int)exit_code;
            }
        }
        return diagnostic(5,error,L"target creation");
    }
    free(command);unsigned phase=0;error=win32_process_activate(&child,prepare_target,&context,&phase);
    if(error)return diagnostic(phase==WIN32_PROCESS_PREPARE_FAILED?6:7,error,L"installation/resume");
    if(wait && (error=win32_process_wait(child.hProcess,&exit_code))){
        CloseHandle(child.hProcess);return diagnostic(8,error,L"target wait");
    }
    CloseHandle(child.hProcess);return (int)exit_code;
}
