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


## P4: mapped DLL identity

The P1 check used the loader-reported DLL path and 64 callback bytes. An owned
process experiment showed that renaming a mapped DLL and placing a same-byte
replacement at the original path leaves the loader-reported path unchanged.
The P1 selection code returned success for that stale mapping. The P4 launcher
also compares the actual mapped-file names before installing the thread hook;
the same regression returns `ERROR_REVISION_MISMATCH` (1306). Existing path and
callback checks remain. This avoids treating a renamed old mapping as the new
file; it is not physical DLL unloading or a claim about every possible image
mutation.

An isolated real Explorer test loaded the candidate DLL into its own shell,
renamed the mapped scratch copy, replaced the original path with identical
bytes, and launched a second folder window. The first window was attached and
the second was not, as intended. Another isolated test with unchanged files
verified standard launch, an existing same-process window remaining unattached,
title-button hit tests at 144 DPI, system menu, maximize/restore and repeated
launch. The normal clean package also passed its selection test. Separate
private-desktop API checks performed one move/resize and a minimize/restore;
the caption remained visible in PrintWindow samples. These are not physical
mouse or live-drag flicker tests; the input desktop was Screen-saver.

The P4 same-snapshot C17, MinGW GCC 16.1, Ninja and ccache dual-width clean
build passed, as did 29 focused actual-package tests without skips. The tested
four files were published to the assets root and `O:\winnt`:

| File | SHA-256 |
| --- | --- |
| run16s.exe | `e214556d958241a9d06b4c775700484fc201b12641934f9d6c47d8044cd7d640` |
| ntw16s32.dll | `54d51d6f93f6269bd3c66f3b2fdc1b2b467bda9daeefa361cbde7c8488cf163e` |
| ntw16s64.dll | `aec3839c721efcd6030fad93e40d803de8c2741704128f3e51e68b16b9dfc938` |
| demo.exe | `a2a3c7fd29ace79120c1d84c6b8fe1628da1abd7550cd7a432a466fa0c4b2c9b` |

If a shared Explorer process already holds an older mapped DLL, the new
launcher will refuse to apply the new hook to that process rather than use the
wrong code offset. Restarting that host is needed to load the new DLL. The
user's shared shell was not terminated during this delivery. S3 remains open
for owner acceptance and the previously stated concurrent-attribution limit.

## Additional physical menu observation

At the owner's confirmed time, a native probe switched briefly to a private
desktop containing its own Explorer shell and folders. The standard published
launcher attached the new folder; a prior folder in that shell remained
unattached. At 144 DPI, `WindowFromPoint` confirmed the target under the
system-button coordinate, and an actual mouse click opened a visible menu.
The original desktop and pointer were restored, and the owned shell exited.

The probe then rejected the menu because its popup handle differed from
`GetSystemMenu`, so it did not move over an item. A separate no-switch private
desktop inspection found the popup in the target process, with the expected
system command IDs and a Win16-style static capture. The probe was corrected
to validate the owner and command IDs; a no-switch preparation also confirmed
the two menu item rectangles are available. The corrected physical hover test
has not yet run. These observations do not prove hover selection, absence of
flicker, menu command execution, live dragging, or other DPI behavior.
