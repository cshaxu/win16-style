#include "lib-win32/process-image/image.h"
#include "lib-win32-style/access/startup/install.h"
#include "lib-win32-style/appearance/msstyles/classic_theme.h"
#include "elevation.h"
#include "lib-win32/security/elevation.h"
#include "lib-win32/process-launch/command_line.h"
#include "lib-win32/process-launch/process.h"
#include "lib-win32/process-launch/lifecycle.h"
#include "startup_error.h"
#include "global.h"
#include "tray.h"
#include "uwp.h"
#include "explorer.h"
#include "console_host.h"
#include "window_target.h"
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <errno.h>
/* MinGW CRT must preserve native arguments. Globbing otherwise interprets the
 * '?' in explicit extended paths (\\?\) before target resolution. */
int _dowildcard=0;
static DWORD prepare_target(HANDLE process,void *context)
{return style_startup_install(process,context,L"ntwshk",CreateProcessW);}
static int diagnostic(int code,DWORD error,PCWSTR phase)
{
    WCHAR message[512]={0};
    FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,NULL,error,0,message,512,NULL);
    fwprintf(stderr,L"run16s: %ls failed (%lu): %ls\n",phase,error,message);return code;
}
static int usage(void)
{
    fwprintf(stderr,L"Usage: run16s.exe --global\n       run16s.exe <theme-folder> <application> [arguments]\n");
    return 2;
}
int wmain(int argc,WCHAR **argv)
{
    if(argc>=2 && !wcscmp(argv[1],L"--global-elevated")){
        WCHAR *end=NULL;DWORD error=0;errno=0;
        unsigned long parent=argc==3?wcstoul(argv[2],&end,10):0;
        if(argc!=3 || argv[2][0]<L'0' || argv[2][0]>L'9' || !parent || errno || !end || *end)
            return diagnostic(9,ERROR_INVALID_PARAMETER,L"global elevation");
        if(!win32_security_is_elevated(&error))return diagnostic(9,error?error:ERROR_ACCESS_DENIED,L"global elevation");
        HANDLE owner=OpenProcess(SYNCHRONIZE,FALSE,(DWORD)parent);
        if(!owner)return diagnostic(9,GetLastError(),L"global parent");
        if(!FreeConsole() && GetLastError()!=ERROR_INVALID_HANDLE){error=GetLastError();CloseHandle(owner);return diagnostic(9,error,L"global console detach");}
        if(!AttachConsole((DWORD)parent)){error=GetLastError();CloseHandle(owner);return diagnostic(9,error,L"global console attach");}
        run16s_rebind_console_stderr();
        if(!SetConsoleCtrlHandler(NULL,FALSE)){error=GetLastError();CloseHandle(owner);return diagnostic(9,error,L"global console handler");}
        int result=run16s_tray_global(owner);CloseHandle(owner);return result;
    }
    if(argc>=2 && !wcscmp(argv[1],L"--global")){
        if(argc!=2)return usage();
        DWORD error=0,code=0;
        BOOL elevated=win32_security_is_elevated(&error);
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
    BOOL reentered=argc>=2 && !wcscmp(argv[1],L"--elevated");
    int theme_index=reentered?2:1, target_index=theme_index+1;
    if(argc<=target_index || !argv[theme_index][0] || !wcsncmp(argv[theme_index],L"--",2) ||
       !argv[target_index][0] || !wcsncmp(argv[target_index],L"--",2))return usage();
    PCWSTR theme=argv[theme_index];
    BOOL private_console=run16s_private_console();
    DWORD error,exit_code=0;WORD subsystem=0;
    WCHAR image[STYLE_STARTUP_PATH_CAP];style_startup_context context;
    STARTUPINFOW startup={.cb=sizeof(startup)};PROCESS_INFORMATION child={0};
    PCWSTR raw=win32_process_next_argument(GetCommandLineW());PWSTR command;
    if(reentered){
        raw=win32_process_next_argument(raw);
        if(!win32_security_is_elevated(&error))return diagnostic(9,error?error:ERROR_ACCESS_DENIED,L"elevated launcher token");
    }
    classic_theme_family family;
    classic_theme_mapping source={0};
    HMODULE launcher=GetModuleHandleW(NULL);
    HRESULT loaded=classic_theme_open_folder(launcher,theme,&source,&family);
    if(FAILED(loaded))return diagnostic(6,HRESULT_CODE(loaded),L"theme load");
    unsigned hosts=source.hosts.enabled;
    classic_theme_close(&source);
    BOOL win2k=family==CLASSIC_THEME_WIN2K;
    unsigned theme_id=classic_theme_folder_id(theme);
    WCHAR resolved_folder[MAX_PATH];
    loaded=classic_theme_profile_folder(launcher,theme_id,resolved_folder);
    if(FAILED(loaded))return diagnostic(6,HRESULT_CODE(loaded),L"theme identity");
    if(_wcsicmp(theme,resolved_folder))return diagnostic(6,ERROR_INVALID_DATA,L"theme identity");
    if(hosts)run16s_window_set_profile(theme_id);
    raw=win32_process_next_argument(raw);
    DWORD length=SearchPathW(NULL,argv[target_index],L".exe",STYLE_STARTUP_PATH_CAP,image,NULL);
    if(!length || length>=STYLE_STARTUP_PATH_CAP)return diagnostic(3,length?ERROR_FILENAME_EXCED_RANGE:GetLastError(),L"target resolution");
    BOOL explorer=run16s_explorer_default(image,win32_process_next_argument(raw));
    BOOL terminal=!explorer && run16s_terminal_image(image);
    BOOL console=run16s_console_host_image(image);
    if((explorer && !(hosts&MSSTYLES_HOST_EXPLORER)) ||
       (terminal && !(hosts&MSSTYLES_HOST_TERMINAL)) ||
       (console && !(hosts&MSSTYLES_HOST_CONSOLE)))
        return diagnostic(10,ERROR_NOT_SUPPORTED,L"theme window attachment for this target");
    BOOL uwp_handled=FALSE;
    error=run16s_uwp_command(image,win32_process_next_argument(raw),FALSE,
        (classic_theme_selection){theme_id,family},&uwp_handled);
    if(error||uwp_handled)return error?diagnostic(10,error,L"UWP frame activation"):0;
    error=win32_process_image_validate_ex(image,IMAGE_FILE_MACHINE_AMD64,FALSE,&subsystem);
    if(error==ERROR_BAD_EXE_FORMAT)error=win32_process_image_validate_ex(image,IMAGE_FILE_MACHINE_I386,FALSE,&subsystem);
    if(error){
        if(error==ERROR_BAD_EXE_FORMAT && private_console && win32_process_image_is_legacy(image)){
            /* No target has been created. Choose one error channel, retaining
             * the initial validation exit code even if Shell returns 1223. */
            if(FreeConsole()){
                run16s_system_error(image,win32_process_next_argument(raw),ShellExecuteExW);
                return 4;
            }
        }
        return diagnostic(4,error,L"native target validation");
    }
    BOOL wait=subsystem==IMAGE_SUBSYSTEM_WINDOWS_CUI;
    error=style_startup_paths(&context,NULL);if(error)return diagnostic(6,error,L"package directory");
    wcscpy(context.module,L"ntwshk");
    if(win2k)context.flags|=STYLE_STARTUP_POLICY_ALTERNATE_APPEARANCE;
    context.flags|=theme_id<<STYLE_STARTUP_PROFILE_SHIFT;
    command=_wcsdup(raw);if(!command)return diagnostic(5,ERROR_NOT_ENOUGH_MEMORY,L"command allocation");
    if(win32_process_create(image,command,NULL,NULL,FALSE,CREATE_SUSPENDED,NULL,NULL,&startup,&child))error=ERROR_SUCCESS;
    else{error=GetLastError();if(!error)error=ERROR_GEN_FAILURE;}
    if(error){
        free(command);
        if(error==ERROR_ELEVATION_REQUIRED && !reentered){
            DWORD token_error=0;BOOL elevated=win32_security_is_elevated(&token_error);
            if(token_error)return diagnostic(9,token_error,L"launcher token");
            if(!elevated){
                error=run16s_elevate(image,win32_process_next_argument(raw),theme,ShellExecuteExW,&exit_code);
                if(error)return diagnostic(error==ERROR_CANCELLED?ERROR_CANCELLED:9,error,L"UAC elevation");
                return (int)exit_code;
            }
        }
        return diagnostic(5,error,L"target creation");
    }
    run16s_explorer_snapshot *folders=explorer?run16s_explorer_snapshot_create():
        terminal?run16s_terminal_snapshot_create():NULL;
    if((explorer || terminal) && !folders)diagnostic(0,GetLastError(),L"window snapshot");
    free(command);unsigned phase=0;error=win32_process_activate(&child,prepare_target,&context,&phase);
    if(error){run16s_explorer_snapshot_free(folders);return diagnostic(phase==WIN32_PROCESS_PREPARE_FAILED?6:7,error,L"installation/resume");}
    if(folders){
        /* The Shell may forward this launch into an existing Explorer process.
         * Only a new, unambiguous folder window can be attached. */
        DWORD attached=run16s_explorer_attach_new(folders,image,context.directory);
        if(attached && attached!=ERROR_NOT_FOUND)diagnostic(0,attached,L"Explorer window attachment");
        run16s_explorer_snapshot_free(folders);
    }
    if(console){
        DWORD attached=run16s_console_host_attach(child.hProcess,context.directory);
        if(attached && attached!=ERROR_NOT_FOUND && attached!=ERROR_PROCESS_ABORTED)
            diagnostic(0,attached,L"console window attachment");
    }
    else if((hosts&MSSTYLES_HOST_CONSOLE) && subsystem==IMAGE_SUBSYSTEM_WINDOWS_CUI){
        DWORD attached=run16s_console_host_attach_current(context.directory);
        if(attached && attached!=ERROR_NOT_FOUND)
            diagnostic(0,attached,L"terminal window attachment");
    }
    if(wait && (error=win32_process_wait(child.hProcess,&exit_code))){
        CloseHandle(child.hProcess);return diagnostic(8,error,L"target wait");
    }
    CloseHandle(child.hProcess);return (int)exit_code;
}
