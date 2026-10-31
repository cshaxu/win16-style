# M0 T21 S7: DPI and controlled input

The owned x86 and x64 fixtures opt into per-monitor-v2 DPI awareness before creating windows. On the private desktop both `GetDpiForWindow` calls reported 144 DPI (150%). The native-message probe checks two plain windows before global mode, eight existing/new windows while global mode is active, and the same eight after shutdown. It sends `WM_SETTEXT`, selects the end, sends `WM_CHAR`, reads the target edit with `WM_GETTEXT`, invokes `BM_CLICK` and verifies exactly one `BN_CLICKED` command. Both ordinary run16s-owned and global-owned targets are included. The results establish these message semantics at the reported DPI; they do not represent physical pointer/keyboard input, IME, accessibility, other controls, multiple monitors or a DPI matrix.

Two initial probe runs failed for test-driver reasons: cross-process `GetWindowTextW` returned cached edit text instead of current control text; after switching to `WM_GETTEXT`, `EM_SETSEL(-1,-1)` did not move the caret to the end. The probe now explicitly sets the selection and verifies the actual target text and click count. These input failures did not require product changes. All test processes and windows are owned and isolated.

An exploratory test showed that if an ordinary run16s process explicitly removes a window's own style and then hides and shows it, global mode can attach it again (`ordinary_process_restyled state=1`). This was initially misclassified as a defect. The user clarified that this reattachment is desired, and explicitly rejected a process-level exclusion as unnecessary complexity. The experimental exclusion and its dedicated test were withdrawn before delivery. The simpler contract remains: existing ordinary attachments are preserved and not removed by global shutdown; an unowned window may be styled by the active global session.

Final clean build, package, deployment and remote verification are recorded below before closure.

`s7-input-final` is the clean dual-width GCC/Ninja/ccache build with the original simple window-ownership behavior; its production and test inputs were compared with the final main tree and matched. The separately built process-exclusion experiment was discarded and not published. The actual package passed 18 focused cases, no skips (`s7-simple-check`), including `GLOBAL INPUT PASS native=2 active=8 stopped=8 dpi32=144 dpi64=144`, the common-control and multithread probes, ordinary launch/propagation and global lifecycle tests. The build and source/ABI/original-resource gates passed.

The rebuilt four-file batch was published to assets root and O:/winnt root with matching SHA-256:

- run16s.exe x64: `cb8bfc61edecefbcc0876946220203692485df3d8b9b4deb2d5c2e0ef6fb5968`
- ntw16s32.dll x86: `cb030463d05f961c6382f4eada1d268b60f6706b18313603e9acb6ecdbfc1620`
- ntw16s64.dll x64: `49874d194d835716bbac51833fd10d64a3ef42419744e446655762c245e795fc`
- demo.exe x64: `9c6f32fd77a12f11c2fcf07cab89902361979aa544e02954611acf85d3d7371d`

S7 did not change product source. Clean-build binaries differed by hash from the previous batch and were therefore redeployed as required. Remote verification follows the delivery commit.
Before this accounting line, the staged change covered 11 files, 68 insertions and 11 deletions. No production source changed; the new probe drives only owned controls and reuses the existing global integration setup. Search covered `WM_SETTEXT`, `WM_GETTEXT`, `WM_CHAR`, `BM_CLICK`, `BN_CLICKED` and `GetDpiForWindow` in the integration fixture and driver.
