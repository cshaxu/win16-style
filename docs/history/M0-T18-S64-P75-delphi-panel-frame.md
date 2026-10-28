# M0 T18 S64 P75: ordinary Delphi panel outer bevel

The observed DelphiVCL4Python 1.0.7 `TPanel` runtime draws its ordinary outer bevel as two complementary system-color `Polyline` paths; the previous renderer did not handle that custom class. See [investigation and unsuccessful test attempts](M0-T18-S64-delphi-panel-edge-investigation.md).

The new candidate observes only the newly created exact `TPanel` inside its own `WM_PAINT`, with the same parent, GUI thread, runtime/class module, calling module and window DC. A full outer pair with the witnessed geometry and complementary highlight/shadow pens permits replay of the original shadow line in black with saved/restored DC. The original API has already run and its return/error is preserved. A mid-pair parent change, absent or partial pair, wrong module/color/role, disabled window, non-edge clip or later reparent prevents the adaptation. No background, inner bevel, text or child layout is replaced. This only covers a verified outer bevel, not arbitrary Delphi panel drawing. Three components, four products, public exports and original resource bytes remain unchanged.

The same-snapshot clean GCC 16.1.0 / Ninja / ccache 4.13.6 build passed x86/x64 ABI and package gates; build times were 23.01 and 27.88 seconds, snapshot manifest SHA-256 `cb566d8aa893d4ed18c1b240362b5abd2da888ad9b3e1635dd9f5c09daf52b1e`. The actual four-file package passed 20 focused cases without skips in 6.84 seconds. New self-contained x86/x64 tests at 96 and 144 DPI cover outside-paint/no edge, custom or incomplete strokes, wrong caller, interior-only and partial clips, paired rendering, reparent and mid-pair reparent, reentrant paint, disabled/re-enabled state and wrong parent owner. Three repeated paints recorded 5 GDI objects before and after on both widths. The old package failed the positive paired-rendering assertion in both widths; an intentionally missing mid-pair parent check was caught on x86. Whole-process resource stability, exception paths and the full test matrix are not proven.

The actual package was used in ten self-owned DelphiVCL4Python x86/x64 runs at 144 DPI: default and width-two outer bevels each changed 369 outer pixels, lowered changed 368, and no bevel changed zero. No interior pixel changed. A real child button comparison held focus and button state fixed and observed zero differences in its rectangle. The initial uncontrolled comparison differed by 258 focus-cue pixels and is not counted. Observer imports were restored and owned instances closed; this is not physical mouse/keyboard validation.

During testing, a separate caller probe and then existing popup/control/group regressions waited in private desktop teardown; the former still waited with the extra caller case skipped. The UI thread waited and MSCTF/COM threads appeared in a diagnostic stack, but root cause was not established. `ImmDisableIME(0)` on the self-owned ASCII test threads let bounded copies complete; the same per-thread isolation was added to the affected native test fixtures. Earlier timed-out/terminated runs remain failures or inconclusive; they were not counted in the final pass. It does not change system settings or product IME behavior.

All four tested files were backed up and published to `assets/` and `O:/winnt/system32`, with matching hashes. The installed x86/x64 panel model passed from both locations.

| File | SHA-256 |
| --- | --- |
| run16s.exe | `78de1ee41cb99cf08c691d86003f42ed028cf6493398d166cf1529bd845ee1ba` |
| ntw16s32.dll | `a0d75aa887dd084049e47446ca96b335f3c6aaa3e485b5624e3a484dcb52e1d7` |
| ntw16s64.dll | `644c58f1ef4995fcba692d53a0430ade259146c9420fb0ff2cbf71b4a39ae03b` |
| demo.exe | `9fada51234b57ba9324495784a4b794b14825a23a7b5f76899d1efaea82e6f8d` |

Production diff `attachment.c` +38/-9, `window_titles.c` +9/-1. The original `Polyline` and app/runtime painting remain owners; the adaptation is a guarded post-draw overlay. Search `rg -n 'TPanel|Polyline|DrawEdge|FillRect' src/ntw16s-dll`: the new paired `Polyline` path is bounded to this observed role; existing independent toolbar, group, frame and paint paths remain unchanged, and other panel backgrounds and application drawing are not claimed. Other runtime versions, deeper bevels beyond the first ring, app-drawn custom controls, physical input and the T18 final integration remain open. M0/T18/S64 remain open.
