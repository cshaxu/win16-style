# run16s launcher

CLI contract: `run16s.exe [--wait] <application> [arguments]`.
The launcher is x64. The target must be an x86/x64 Windows native GUI/console PE.
The application is resolved with Windows SearchPath; the original target command
line tail is passed to CreateProcessW without reconstructing its arguments.
Console targets wait by default and return their actual exit code. GUI targets
return zero once installation is staged and suspension released unless `--wait`
requests their actual exit code. The launcher remains Console subsystem so an
interactive parent cmd naturally waits for it; no start /wait wrapper is required.
Installation staging cannot prove target initialization; runtime fixtures check
the target entry and actual window attachment separately.

Launcher failures print a phase and original Windows error: usage=2,
resolution=3, PE validation=4, process creation=5, installation=6, resume=7,
wait=8. Target exit codes can overlap these values when --wait is used.
The package DLLs are located beside the launcher, by absolute path. Their import
paths must round-trip through the active Windows ANSI code page, or have a usable
short path; otherwise installation fails explicitly. Target Unicode paths and
argument text are preserved. OS rundll32 is only a bounded cross-width installer;
it is not an additional shipped product. No global hook or existing target is used.
Native descendants created through CreateProcessA/W inherit the same package
and style policy. --wait waits for the initial target; the application controls
its own waits for descendants.


On ERROR_ELEVATION_REQUIRED (740), the launcher requests Windows UAC consent
for a new elevated copy of itself, then creates and styles the target there.
Ordinary targets retain the original path. Cancellation returns 1223; another
failure reports its native error. The internal --elevated flag checks the real
token and prevents recursion; it is not an authorization mechanism.

The absolute target, argument tail, working directory and wait policy cross the
boundary. Windows supplies the elevated environment and a separate console;
custom caller environment and standard handles are not forwarded across UAC.
Console targets and --wait return the target exit code. GUI targets otherwise
return after the elevated launch transaction. DLL descendant CreateProcess hooks
do not request UAC; invoke run16s explicitly for an elevation-required target.

Unsupported MZ/NE targets are rejected before target creation (error 193, exit 4).
At entry the launcher snapshots its console attachment: a shared console (for
example cmd) or an explicitly hidden/windowless console keeps the original stderr diagnostic exactly once.
A console with a window containing only the launcher has no shell to retain that diagnostic;
the launcher releases only its own console and requests one Windows Shell open
of the same absolute legacy image and original argument tail. Windows owns the
native error UI; run16s adds no second diagnostic, dialog or pause. Validation
itself does not start the target. Shell cancellation does not replace exit 4.

This is console-attachment detection, not parent-name guessing or Win+R-specific
logic. A shell that already exited can leave a sole-process console; it follows
the same UI policy. A headless/CREATE_NO_WINDOW job (which can still attach a hidden console) retains stderr, including
redirection. Explicit redirection combined with a new sole-process console still
uses system UI. An explicit STARTF_USESHOWWINDOW/SW_HIDE startup also retains stderr.
Query/detach failure keeps stderr. Console subsystem is retained,
so the initial creation of a console can still briefly appear. Malformed/native
PE and other validation errors do not receive an unstyled Shell fallback. A
system-configured legacy handler is Windows' responsibility, not NE style support.
The existing ERROR_ELEVATION_REQUIRED path remains unchanged.
