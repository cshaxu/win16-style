# run16s launcher

CLI contract: `run16s.exe [--wait] <application> [arguments]`.
The launcher is x64. The target must be an x86/x64 Windows native GUI/console PE.
The application is resolved with Windows SearchPath; the original target command
line tail is passed to CreateProcessW without reconstructing its arguments.
No wait returns zero once installation is staged and the launch suspension is
released. `--wait` returns the actual target exit code, including loader failure.
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
