# M0 T23 S1 P3: actual Notepad native exterior prototype

## Answer established by experiment

The existing Ntw16sWindowAttach API can draw the Win3.x caption and borders
on the actual Notepad root HWND while preserving the visible tab strip and
menu/toolbar below it. No UWP frame library, overlay window, reparenting, XAML
property editing or propagation hook is used. This is an actual native exterior
prototype, not proof of complete Notepad compatibility or a production release.

## Method and ownership

Registered scratch s1-notepad-native1 and s1-notepad-native2 under build/t23-package.
Pure C17 x64 driver and thread-hook fixture compiled with GCC16.1, warnings as
errors. A targeted WH_CALLWNDPROC callback runs only for the selected Notepad
thread/window, loads the published assets/ntw16s64.dll passively, and calls the
existing same-thread WindowAttach and WindowRemove exports. No product export,
source or resource was changed. Renderer SHA256:
9c7f824a535ca02068c52794016ad0e51087be88d5f4a4c1ecc621afb249cc5f.

The first attempt refused an existing user window26940. The user subsequently
authorized that window, but it had already closed. Actual runs instead started
owned instances after checking no Notepad existed, using only the registered
owned-note.txt scratch file. Target13384/TID8420 and target27500/TID22008 were
observed with creation-time/path records. The callback never selects other roots.
No private desktop or computer-use was used. This is real desktop capture.

## Observed results

Both runs returned attach1/error0 and remove1/error0. In each:

| Phase | Outer rectangle | Client origin | Client size |
| --- | --- | --- | --- |
| Before | 893,206,1741,965 | 902,206 | 830,750 |
| Styled | 896,171,1738,962 | 902,206 | 830,750 |
| Removed | 893,206,1741,965 | 902,206 | 830,750 |

The existing renderer grows/repositions the outer frame to preserve the original
client rectangle. Viewed actual screenshots show a Win3.x title/system button and
min/max buttons above the original Notepad tab strip, File/Edit/View menu and
application toolbar. No blanket child relocation was needed for this observation.
Styled screenshot: s1-notepad-native2/styled.bmp. Before/restored counterparts and
run.log retain the exact outcome; the windows are normal144DPI, not minimized.

Important failure: removed screenshots retain an incorrect light border/title
button presentation despite geometry restoration. A second trial additionally
sent WM_THEMECHANGED and SWP_FRAMECHANGED only to this target and repainted;
refreshed.bmp still shows the defect. Therefore plain repaint/theme notification
is not an established fix. API success does not mean original pixels recovered.
The driver exit0 means its API/capture contract succeeded, NOT visual acceptance.

The first capture was contaminated by a notification and real keyboard/IME input
arriving while the prototype brought the target forward. Those images are not
content-preservation proof. The second fixture removes explicit foreground calls;
it still shows the app's recovered scratch text from that interaction. The probe
itself never sends editing keystrokes or writes the document. Tests of menus/tabs,
focus/IME, maximize/resize and full lifecycle remain unperformed, not passed.

First owned instance later exited. Second normal-close attempts timed out; exact
PID/start-time/path checks preceded cleanup of owned27500, and the final check
found no Notepad process. No user-owned process was terminated. The renderer
reference is retained until target process exit; no physical DLL-unload claim.

## Design consequence

Prefer this native nonclient route for the next Notepad prototype. A new external
shell is not justified by the current evidence. Investigate the per-window native
theme/DWM/title-state restoration contract, then verify actual tab/menu actions
and size transitions with before/after geometry and screenshots. Keep XAML client
controls unchanged. This result neither proves WindowAttach automatically reaches
the final process from run16s nor fixes that separate launch-handoff problem.

S1/T23 remain open. No binary changes: the published four files remain P34;
this delivery records evidence and failure boundaries, not a repaired product.
