# M0 T18 S64 P77: disabled Delphi group frame

The existing bounded Delphi group frame intentionally rejected disabled windows; an ordinary disabled TGroupBox consequently retained its themed frame. In its own verified paint scope the adapter now accepts group part 4 state 2 only when the window is disabled. It records the background state, requires the matching text state and rechecks enabled state, parent and paint ownership before drawing the same black one-pixel frame used by the enabled path. The original themed background and text execute first; framework/application title, text, child content and input remain owners. This does not change the original resources or public ABI.

A genuine DelphiVCL4Python 1.0.7 sample at 144 DPI showed state 2 for both background and title. The actual clean package attached its x86/x64 groups; each comparison to the same-width previous disabled capture changed 633 RGB pixels and none in title rectangle (8,0)-(87,25) or independent interior (15,30)-(235,105). Processes exited normally. Earlier source attribution in the diagnostic was invalid due compiler splitting; the final package uses the ordinary unsplit hook. Details and unsuccessful attempts are in [the investigation](M0-T18-S64-disabled-group-investigation.md). Real disabled TRadioGroup, physical input, other DPI, historic disabled frame color fidelity and full integration are not claimed.

The project-authored group/radio models cover enabled and disabled, disable-to-enable transition, disabled empty caption, partial clip, and reentrant enable change, alongside outside-paint, reparent, empty caption and wrong-owner checks. New x86/x64 models rejected the old package at the disabled positive check; deliberately removing the final reentrant state check caused the x86 test to fail. The same-batch actual package passed 20 focused cases, zero skip, in 56.54 seconds, including both widths of group/radio frame, panel, group module, font layout and menu cancel, plus mandatory package baselines. Font cases took substantial time; omitted full integration and physical input are recorded in the test result. Both publication directories passed the installed group model in both widths.

A clean source snapshot built with C17, MinGW GCC 16.1, Ninja and ccache 4.13.6; x86/x64 build times were 28.64/35.92 seconds. ABI, source receipts, governance and byte-exact original resource checks passed. The snapshot manifest SHA256 is `7698996c7055815c4205a3e45bddd0fde096fb8f0129288f43eb6c22564d32d1`. The tested four files were published to assets root and O:/winnt/system32, with eight matching destination hashes:

| File | SHA256 |
| --- | --- |
| run16s.exe | `3ba7ce5f458b1ab6dacae2a596d0f7cac94b00ddb4be2a85309ec0ec2c9488fb` |
| ntw16s32.dll | `f386279f3c32afc8406b8f2c86368094fee79bb2772b9aa0c6661e56105efce5` |
| ntw16s64.dll | `c14cec5f2fa099c4eedfbc5e9871af91bcfe8a1b8ac82ecf9ccdedfc3c15ebca` |
| demo.exe | `e9b1c12e7b4184dd65094a52a179233a387424c330bd6910108bebdd6c02252f` |

Search `rg -n 'TGroupBox|TRadioGroup|ntw16s_group_frame|DrawThemeTextEx' src/ntw16s-dll/attachment.c src/ntw16s-dll/injection/window_titles.c`: the two group roles share the same frame scope, the existing theme hooks forward first, and no independent drawing path was replaced. Production `attachment.c` is +5/-5, net zero; tests/packaging additionally update fixtures and package markers. Disabled radio appearance is model evidence only. T18/S64 remain open.
