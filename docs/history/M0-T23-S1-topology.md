# M0 T23 S1: initial desktop title topology

P2 is a partial research delivery, not completion of S1 or product support.
The user explicitly requires preserving top tabs and menus. No renderer, process
policy, product binary, registry or system setting changed in this investigation.

## Native evidence

A pure C17 read-only probe enumerates selected process identities, package/model,
architecture, framework module paths and visible HWND trees. It records window,
client and DWM caption rectangles, DPI, styles, parent and GUI thread IDs, without
reading user document text or changing existing windows. Scratch:
`build/t23-package/s1-topology1`. GCC16.1 x64 build passed with warnings as errors;
initial compilation lacked Windows10 SDK defines and was corrected. No x86 native
runtime claim is made for this scratch probe. Module presence alone is not styling.

- Existing Explorer8056 is native x64, non-AppContainer and unpackaged. It owns
  taskbar/desktop as well as CabinetWClass folders. A folder's child tree includes
  InputNonClientPointerSource, TITLE_BAR_SCAFFOLDING_WINDOW_CLASS and WinUI content
  bridges. This observed folder was minimized (coordinates around -32000), so its
  geometry is NOT a normal-display/layout result. An ApplicationFrameWindow also
  belongs to Explorer: class name alone cannot safely select a UWP host.
- Existing Taskmgr26784 is native x64, unpackaged and non-AppContainer. Module
  enumeration still fails with error5; no visible top-level window was enumerated.
  No restart, injection, privilege change or close was attempted on this instance.
- No Notepad existed before the owned test. System32/notepad.exe launched PID24296;
  its child3268 owns the actual Notepad HWND. Both resolve to installed packaged
  Notepad11.2607.14.0, native x64 and non-AppContainer. The final child loads
  Microsoft.UI.Xaml from WindowsAppRuntime2_2.6.5.0 and NotepadXamlUI/WinUIEdit.
  A scratch-only text file was opened, no editing/input performed. Both newly
  created processes exited following normal CloseMainWindow; no user instance
  was touched. This is a native baseline, not a styled launch/propagation test.
- Notepad at144DPI: outer rectangle(426,192)-(1274,951), client origin(435,192).
  Client content reaches the top edge. DWM reports caption button bounds
  (618,0)-(838,44) relative to the window. Visible content bridges occupy
  (495,206)-(1000,254) and (435,254)-(1265,304), with other bridges below.
  These rectangles prove top client content, not individual tab/menu semantics.
  Main window and bridges use TID20148; NotepadTextBox/RichEditD2DPT use39332.
  No HMENU exists on the root. A blanket recursive same-thread title adaptation
  would therefore be an incorrect assumption.
- Terminal was absent in this snapshot; actual owned-window validation remains.

Raw evidence: snapshot.log, notepad.log and notepad-owned.json in registered
scratch. Read-only enumeration returns0, including the explicitly reported
Task Manager access denial; that is not a pass for its module inspection.

## Primary-source research (read-only, no imported code)

Microsoft's [title-bar guide](https://learn.microsoft.com/en-us/windows/apps/develop/title-bar)
separates extended client content and interactive passthrough regions. These are
application APIs; the documentation does not establish arbitrary external access.
The [WinUI custom-title design](https://github.com/microsoft/microsoft-ui-xaml/blob/main/docs/design-notes/customtitlebar.md)
describes distinct input and caption windows, rather than one XAML brush.
Terminal's [NonClientIslandWindow implementation](https://github.com/microsoft/terminal/blob/main/src/cascadia/WindowsTerminal/NonClientIslandWindow.cpp)
uses a separate drag input window and explicit caption hit testing. Main-branch
research is not version-pinned proof for this machine's Terminal; no code copied.

## Design decision and next bounded checks

Separate final-process acquisition from exterior layout adaptation. Preserve tabs,
menus and their original handlers. Treat native caption controls, drag regions,
resize borders and interactive client regions as distinct responsibilities.
Prefer native nonclient integration where supported; do not assume a full-width
UWP-style cover fits these desktop windows. Do not relocate every child HWND:
Notepad already demonstrates independent threads and several content bridges.

Two layout candidates need actual prototypes: reserve a classic caption row while
letting the application relayout its content, or adapt only identified caption/
drag regions without covering interactive elements. Neither is yet proven for
these targets. Geometry/role identification must survive resize, maximization,
DPI and tab changes; accessibility/WinRT metadata is evidence to evaluate, not a
pretext to guess roles from fixed coordinates. The UWP library's current simple
frame/title contract must not be silently broadened to arbitrary client layouts.

Remaining S1: owned Terminal normal-display topology; owned Task Manager with
appropriate access; self-owned Explorer folder normal display; top interactive
region identity and ordinary run16s final-process handoff observation. Then select
the smallest implementation with focused regression and explicit failure cases.
No additional T/S assigned, no product release or full-matrix claim in P2.
