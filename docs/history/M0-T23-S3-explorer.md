# M0 T23 S3 P1: Explorer window candidate

The standard `run16s explorer` launch can be handed from its first process to
an existing Explorer shell. The launcher now records existing folder windows
and attaches the renderer to a single new visible Explorer folder window; it
does not select a previously existing window. Multiple new candidates are
rejected. This is a bounded temporal association, not a system-provided launch
identity: if an unrelated folder alone appears during the same interval and the
requested window does not appear, it can still be misidentified. Commands with
Explorer arguments retain their original startup path.

The existing renderer recognizes both desktop composition bridge classes. A
new additive DLL export, `Ntw16sWindowThreadHook` at ordinal 39 in each width,
executes the existing same-thread attachment or removal for an explicitly
selected window. Earlier ordinals and the four product files remain unchanged.
The temporary hook is removed after the request; a successful window attachment
pins the DLL in its host, so this is not physical DLL unloading. When the host
already has a DLL of that name, the launcher checks its path and the callback's
first 64 bytes; that is not a complete in-memory image comparison.

A clean C17 x86/x64 build used MinGW GCC 16.1, Ninja and ccache 4.13.6. Source,
resource, ABI, package and dependency checks passed. The same four-file package
passed 29 focused package cases, without skips, covering injection, propagation,
menus and related window paths. The clean x64 private-desktop selection test
passed for an existing hidden window, unique new window, multiple new windows,
and a target loading a DLL from a different path. A separate owned-process
experiment also rejected a modified callback at the same module path. An x64
menu-dispatch test failed once while both width UI suites ran concurrently;
the x86 test and a separate x64 rerun passed. The cause of that single failure
is not established.

On an isolated desktop with a separately created shell, the exact clean package
ran `run16s explorer`. The final Home window loaded this package's x64 DLL and
reported attached; an older folder window in the same Explorer process remained
unattached. At 144 DPI, system, minimize and maximize hit tests returned 3, 8
and 9. PrintWindow captures showed the Win16 caption, original Explorer tab and
navigation region, and a classic system menu. Maximize and restore reported
normal states and retained the attachment; a second launch also attached its
new window. The owned shell and windows were closed after testing. An empty
private desktop starts a shell for an unqualified `explorer.exe`, rather than a
folder; this is distinct from the existing-shell test.

A further private-desktop test used the published `O:\winnt` package and
Windows UI Automation against the newly created folder window. Invoking its
Add New Tab control changed the observed tab-item count from 3 to 6; invoking
Close Tab brought it back to 3. Selecting This PC changed the window title to
This PC and the frame still reported attached. This checks these specific
provider operations, not physical mouse input or every navigation path. The
ShellWindows automation collection did not return the private-desktop window,
so that separate navigation method was not exercised. Additional PrintWindow
samples after each tab operation and the This PC navigation showed the Win16
caption. Removing the window attachment on the same owned window succeeded,
the attachment query became false, and a sample showed the native Explorer
caption again; the DLL remained pinned until the owned shell exited. Three
synthetic hover messages left the system menu visible but did not produce a
highlighted menu state, so they do not establish working mouse hover or absence
of flicker.

These checks do not prove physical mouse clicks, hover flicker, live dragging,
other Explorer tab/navigation operations, all DPI configurations, or every concurrent
launch. The task remains open for the owner's acceptance. No Task Manager UAC
was requested; its separate previously documented visual issues remain open.

The verified package was published to the assets root and `O:\winnt`:

| File | SHA-256 |
| --- | --- |
| run16s.exe | `69c34b6fad54101974a3ffb04c8a35b3cebce127955fd4a6569641d1021aaf9f` |
| ntw16s32.dll | `8be15e272ecd6a548a7a565474dd337ebab9e4ad2463427f4765687633b39037` |
| ntw16s64.dll | `a4e4a0f38555437cd65884138e165b4957d10d32634febf56494e3d4ee0af627` |
| demo.exe | `cb4697009ed9859b15dfe315a435709ef67bbfdb814322cde0bb53b1e4bd9144` |
