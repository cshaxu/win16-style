# On-demand UAC elevation

The launcher stays asInvoker. It first resolves and validates the native PE and
tries the existing suspended creation transaction. Only native error740 requests
ShellExecuteExW with verb runas for the launcher's own absolute path. The elevated
copy validates its token, cannot recurse, and uses the original injection/resume
transaction. DLL propagation hooks retain the original CreateProcess contract.

The resolved absolute target, untouched argument tail, original CWD and wait
policy are passed on the shell command line. There is no request file, named pipe,
credential storage, global hook, new service, or fifth product. The private
--elevated flag grants no privilege; a non-elevated invocation is rejected.

Windows creates the elevated environment and separate console. Caller custom
variables and stdin/stdout/stderr are not forwarded across this boundary. This is
intentional: elevation is not ordinary CreateProcess inheritance. Console targets
wait and propagate their exit code; --wait does the same for GUI targets. GUI
launches otherwise return after the elevated launcher finishes installation and
resume. Cancellation returns1223; shell failure reports its native error. Other
creation errors are not automatically converted into an elevation request.

## Native verification

Build with the standard GCC/Ninja/ccache entry. Both architectures register
launch/elevation-unit, which uses real production elevation.c with a controlled
ShellExecute function: request fields, empty/spaced/trailing-backslash args, CWD,
wait flags, cancel1223, denied5, missing handle6, failed-without-error31, child exit73,
overlong request rejection and steady handle counts. It does not interact with
secure-desktop UI. Each width executes8 bounded cases.

Standalone test fixtures uac-target/uac-gui-target embed requireAdministrator.
They verify their own high token, DLL and HWND attachment; preserve raw report
identity, parent PID, image/module paths and actual DPI; capture the normally
shown frame via native PrintWindow; create a styled same-width child; and retire
owned windows/processes. They are test executables, never package files.

The native driver supports these explicit, opt-in modes:

```text
uac-driver --baseline <admin-fixture>
uac-driver --once <package-run16s> <admin-fixture> <fresh-report> <nonce>
uac-driver --gui-async <package-run16s> <gui-fixture> <fresh-report> <nonce>
uac-driver --batch <package-run16s> <console32> <console64> <gui32> <gui64> <fresh-directory>
```

The first mode expects740 from a non-elevated process, otherwise77. The second
mode invokes the real launcher and system UAC; success must come from the actual
target, with exit73, exact DLL directory, arguments, CWD and Windows environment.
A canceled request is reported separately, not counted as application success.
The high-token batch is itself authorized through Windows and covers six cases:
both widths of Console and GUI with waiting, plus both GUI asynchronous launches.
It records each failure and waits for target-owned completion markers and captures.
All timeouts/cleanup apply only to owned processes. Parent PID/paths and raw reports
remain private; public evidence uses normalized identities and hashes.

## Current bounded coverage

Actual medium-token CreateProcess on both admin fixtures returned740. The old
launcher exited5 and the new integration driver rejected it. The final package
completed a real64-bit fallback into high integrity (12288), exact package DLL,
visible classic frame, target-created success marker, arguments/CWD/environment,
exit73 and styled child. The already-authorized high batch passed6/6, including
x86 helper installation, GUI wait/asynchronous semantics and styled descendants.
The private flag was actually rejected at medium integrity with exit9/error5.

No claim is made for another administrator account's credential prompt, mapped
network drives in that account, real secure-desktop cancellation clicks, arbitrary
protected targets or every downstream ShellExecute/runas operation. Cancellation
handling is automatically tested, but that is not secure-desktop input proof.
Physical input and arbitrary DPI are not newly certified by this UAC work. The
fixtures' actual logical DPI is96; original normal Win32 UI/DPI regression remains
separately required. No security setting or directory permission is changed.

Acceptance can use a known native administrator-required program, for example:

```text
O:\winnt\system32\run16s.exe C:\Windows\regedit.exe
```

Approve only the expected launcher prompt to inspect its styled target; cancel to
check error1223. Launching this example does not request any registry edit. An
application that starts normally and only later requests elevation is outside the
automatic740 retry; invoke run16s at that explicit elevated entry instead.

Primary references: [UAC creation boundary](https://learn.microsoft.com/en-us/windows-server/security/user-account-control/how-user-account-control-works),
[Shell activation](https://learn.microsoft.com/en-us/windows/win32/shell/launch),
[elevated environment rationale](https://devblogs.microsoft.com/oldnewthing/20130703-00/?p=3903).
