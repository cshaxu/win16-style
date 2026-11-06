# M0 T22 closure: UWP window exterior

M0 T22 S1 is closed by P43 after the requested implementation and bounded native
verification. No next T is admitted. This completes the window-first task, not
internal XAML styling or universal modern-application compatibility.

## Delivered behavior

- `run16s calc.exe` identifies the system Calculator activation entry and styles
  its hosted exterior; explicit `--app <AUMID>` supports the general activation path.
- `run16s --global` retains its tray-first UX. Enable discovers existing/new
  applicable hosted UWP windows; Uncheck removes owned style surfaces and leaves
  the controller resident. Re-enable and final Exit follow the existing semantics.
- Win3.x title, frame, system menu and caption buttons reuse the existing renderer.
  The application retains its client content and native layout. No registry,
  target executable or security-setting changes are required.
- Generic exterior rendering lives in `lib-win32/win16-window-uwp`; product target
  identity/activation/global coordination stays in `ntw16s-dll/uwp` and run16s.
  Targeted host-thread attachment does not enable propagation in the shared host.

## Completion evidence inspected for P43

The [requirement-by-requirement audit](M0-T22-coverage.md) is the detailed matrix.
P43 reread raw build/package results and actual logs, viewed Calculator/Scan BMPs,
and compared tested candidate, assets and O:/winnt bytes rather than relying only
on summaries. Principal evidence:

| Contract | Evidence |
| --- | --- |
| Clean GCC/Ninja/ccache x86/x64, four products, ABI/resources/boundaries | s1-clean12/result.json and package.json; passed. Pure C17, no fifth product. |
| Actual package native regression | s1-package8/result.json:20 focused passes, no skips; full repository matrix was not run and is not claimed. |
| Visible existing/new Calculator and Scan | s1-global-package2/native four logs/screenshots: visible surface, actual144DPI capture, removal and stop-race pass. |
| Launcher and caption controls | s1-global-package1/native/explicit-command.log: calc command0, maximize/minimize/restore and removal geometry pass. |
| Real pointer and reuse | s1-pointer-regression1/native/scan.log; s1-retire1/native/{scan,calc}.log: same-PID Scan reuse and new-PID Calculator, exact surface destruction. Renderer unchanged since these tests. |
| Shared host and ordinary-launch coexistence | P36/P37 actual owned pairs and real launcher path; source preprocessing without test macro unchanged from P34. |
| Formal tray ordinary-token repeat | P40 s1-tray-window-events2: two on/off cycles, surface destruction, residency, Exit0. |
| Formal tray administrator repeat | P42 s1-tray-admin-repeat2: elevated probe11248/controller10284, Calculator21548/Scan34716; both cycles and Exit0. |
| Deployment | Four candidate SHA values equal package8, assets and O:/winnt; P42 remote bytes verified. P43 repeats remote verification after push. |

No products changed after P34. The two manager-source differences since P34 are
inside NTW16S_GLOBAL_MANAGER_TEST only; P36 verified production preprocessed output
unchanged on both compilers. Later fixture work does not require another binary
release. This closure is documentation only. Original artwork/attribution remain.

## Explicit limitations

The proven targets are x64 ApplicationFrameHost/CoreWindow-hosted Calculator and
Scan at144 DPI. Other hosting models, multi-monitor DPI transitions and native
title-HWND replacement are not certified. Notepad/Terminal/Task Manager/Explorer
remain classified desktop/hybrid candidates in the queue, not UWP pass counts.
Internal XAML controls, fonts and templates remain excluded by the user's scope.

The host DLL remains pinned until host exit. Removal deletes the owned adaptation;
it is not physical DLL unloading. Global desktop sweeps had5 failed notifications
under admin (92 under ordinary token); owned UWP restoration does not certify all
other windows. Existing global restoration debt is retained. UIA per-call timeouts
do not prove cancellation of all hung providers. Native auxiliary client menus
remain native. Formal menu tests invoke actual MSAA menu actions, not Explorer
physical clicks; launcher-triggered UAC is a pre-existing tray verification debt.
Default opt-in interactive CTest entries skip; they are not runtime pass counts.

The original old0x80020003 menu automation failure has no separately established
cause. The observed disabled-menu readiness issue was corrected in the test;
subsequent ordinary and admin real-menu runs pass without a speculative product fix.

These limitations are explicit supported-boundary/deferred coverage, not claims of
universal success or hidden incomplete client-control work. Follow-up admission
remains user-controlled; no WinUI or XAML task is automatically started.
