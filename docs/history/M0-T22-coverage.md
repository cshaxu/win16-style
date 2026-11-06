# M0 T22 window-only coverage audit

This audit preserves the user's window-only scope. Internal XAML controls and
WinUI/desktop hybrid adaptation are not silently counted as implemented. M0 T22
S1 remains open. The published product is P34; P37/P38/P40 added verification only.

| Requirement | Observed result and authoritative evidence | Status |
| --- | --- | --- |
| Ordinary UWP exterior without app cooperation | Calculator and Scan hosted ApplicationFrameWindow receive the shared Win16 title, system menu, buttons and border. P34 actual-package existing/new screenshots and logs in s1-global-package2/native were reread/viewed. Client UI stays native. | Verified for these actual targets at144 DPI; not all UWP variants |
| Simple Calculator command | Published system calc.exe recognition, real --wait lifecycle and actual launcher/manager coexistence; research P22/P28/P37 and s1-global-package1/native/explicit-command.log. | Verified; other AUMIDs use --app or global discovery |
| Global existing and newly opened UWP | P34 uncloak ordering regression and actual-package four cases; P38 formal production controller Enable styles existing Calculator12284 and new Scan37796. | Verified within current x64 hosted boundary |
| Disable and restore | Manager fixtures destroy exact surface HWNDs; P38 formal external request also destroys both and exits controller0. This differs from Uncheck-and-remain-resident. | Engine shutdown verified; P40 actual Uncheck residency verified for ordinary-token window discovery |
| Repeated use and coexistence | P30 actual same-PID Scan reactivation plus surface destruction; P36 two frames in one host and immediate-stop; P37 actual launcher then manager. | Verified sampled paths; P40 formal tray off/on repeat passed on ordinary-token window discovery |
| Title/menu/buttons and input | Real screenshots, native move/resize/system-menu actions P21, corrected visible cursor P27, minimize/maximize/restore logs, Scan full-screen round trip. P34 actual launcher buttons/removal also pass. | Verified sampled144-DPI paths; not every input/DPI combination |
| Shared host and ownership | Separate frame/CoreWindow identity, no host propagation, per-frame removal; P36 both owned surfaces destroyed without terminating host. | Verified sampled coexistence; DLL stays pinned, no physical unload claim |
| Framework classification | Notepad packaged FullTrustApplication/WinUI, Terminal FullTrustApplication dependency; Explorer desktop mixed Shell; Task Manager desktop with incomplete framework-module evidence. See modern-desktop-window-style proposal. | Classified at recorded confidence; deferred candidate, not UWP pass counts |
| Preserve existing native style and artwork | clean12 same-snapshot four-file build/gates; package8 passed20 focused cases with no skips. No changes between P22 and HEAD in win16-window-uwp, win16-window or win16-common at this audit. | Verified focused scope, not universal Win32 compatibility |
| Architecture/toolchain and delivery | Pure C17, GCC/Ninja/ccache dual-width products, three components/four files. P34 assets and O:/winnt hashes match; P38 fetched remote artifacts verified. | Delivered; subsequent test/documentation changes do not change products |

## Verification status and remaining check

P40 completed the bounded formal tray test against published O:/winnt/run16s.exe:
wait for session.ready, invoke the enabled checked menu item through MSAA,
require both exact surfaces destroyed and the controller resident, then enable
and disable again before Exit. All passed at144 DPI for existing Calculator39528
and new Scan34520, controller39840 (s1-tray-window-events2/actual.log). The probe
and product used the ordinary token's existing window-discovery path; administrator
ETW and cross-integrity behavior are not counted as passed by this run. Both-width
probe builds passed; actual execution was x64. There was no new UAC prompt.

This corrects the fixture's assumption that visible styling means the menu is
ready. toggle1 had directly observed disabled=1; ready-aware runs observed enabled
checked actions returning0. The older isolated0x80020003 is not independently
proven to have the same cause. These are real production menu actions through
MSAA, not physical Explorer clicks. Each desktop sweep reported92 failed/timed-out
notifications; successful restoration of the two owned UWP surfaces does not prove
all other desktop windows restored.

P41 requested elevation for the same P40 fixture with --toggle. Windows
UAC canceled before creating the probe: no PID or actual.log, so administrator
repeat-toggle remains unverified. No repeated prompt was issued. The production
files remain unchanged.

After this, review the completed scoped evidence and delivery before closing S1
or T22. This audit does not authorize closure or claim the goal is already complete.

## Explicit limits, not additional implicit tasks

- Only x64 ApplicationFrameHost/CoreWindow hosting is implemented; not arbitrary
  CoreWindow-class windows in Shell, WinUI desktop or all UWP hosting models.
- Multi-monitor DPI and replacement of an existing native title HWND are not
  certified. Renderer code is unchanged since P22; tests do not cover every variant.
- Native auxiliary app menus/client controls remain native by current scope.
- The host module remains pinned; style removal is not physical DLL unloading.
- UIA per-call timeout does not prove cancellation of every hung provider.
- P38 desktop sweeps reported five failed/timed-out notifications at start/stop;
  successful owned-UWP restoration is not proof every desktop window restored.
- Explorer physical icon clicks and launcher-triggered UAC are not covered by the
  elevated probe's formal-controller test. Existing tray UX history is separate.

Evidence records are in [S1 research](M0-T22-S1-feasibility.md); build scratch is
registered in CURRENT. No neighboring repository is a runtime/test dependency.
