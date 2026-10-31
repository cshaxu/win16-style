# M0 T21 S10: system folder view in an owned process

The S9 Explorer.exe isolation gap remains. This investigation instead hosts the system `IExplorerBrowser` in a C test process, as documented by [Microsoft](https://learn.microsoft.com/en-us/windows/win32/shell/developing-with-windows-explorer) and its [interface reference](https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/nn-shobjidl_core-iexplorerbrowser). This is a real system folder view in an owned host, not the Explorer.exe executable or a substitute for testing its shared shell process. No user Explorer was injected, stopped or manipulated.

The private-desktop probe created one pre-existing and one new host for each of x86 and x64. The target contained a single scratch file and reported it via `IFolderView::ItemCount`; navigation was asynchronous in observation (initial count 0, later 1). The driver verified each root HWND's PID was its own launched process, and each target enumerated seven descendant HWNDs, all on its own process and GUI thread. Classes included `ExplorerBrowserControl`, `SHELLDLL_DefView`, `DirectUIHWND`, two `CtrlNotifySink` and two `ScrollBar` windows. After global attachment the host root and both ScrollBar controls reported attached; DirectUI and the shell containers did not. After global stop the root and both scrollbar attachment markers were absent, the item count remained 1 and all four host processes exited with code 0 after `IExplorerBrowser::Destroy` returned S_OK.

PrintWindow captures show the host caption changed to the project's Win16 frame, while the DirectUI list contents remain modern. The title-area fingerprint before/styled/restored was `e394d8dae76ef939` / `48ed1274da42daad` / `e394d8dae76ef939` in both widths; this hash covers only part of the title area, not the whole image. Native module observations reported the S9 candidate's absolute `ntw16s32.dll` and `ntw16s64.dll` paths, with respective SHA-256 `27304c6b2ece801cec4414461f9dfd7508975754b0b597907355a9c94503658d` and `86a682c6f14c8cceb2e239dfa28bb0bbb1fde8f4bfa22d5596b305c56e855e75`. Captures are off-screen native output, not physical pointer/keyboard proof, and the unchanged DirectUI surface is not counted as styled.

This establishes a bounded real Shell view path only. It does not remove the Explorer.exe exclusion, explain S9's process exit, cover other Shell configurations or prove real desktop behavior. No product source or graphics changed. The S10 package and remote review are recorded below.

The final `s10-shell-final` package was retested with the same owned Shell view probe on both widths: x86 root PIDs 26432/32476 and x64 root PIDs 36308/16452 matched the respective created processes; both existing/new views retained one item after stop, and all target processes exited 0 after `Destroy` returned S_OK. Their module paths were the matching DLLs in the final candidate. The final caption-area fingerprints matched the preliminary before/styled/restored values above. These are explicit private-desktop exploratory probes, not a claim that the package regression suite contains an Explorer.exe case.

The same clean dual-width GCC/Ninja/ccache build passed source, resource and ABI gates, and 13 focused actual-package cases passed without skips (`s10-shell-check`). The four tested files were published to assets root and O:/winnt root:

- run16s.exe x64: `38a1c064661ad35267f0196e2fa00465f8ad9e516c1c469b7095739cae37f846`
- ntw16s32.dll x86: `3b975ecb3190ad916ddd8411c56fdcfc15fca617a02d64956296b1ca94476c4e`
- ntw16s64.dll x64: `66b457ae303c9ad162d701f184dd8e134b2ca5f6a1eb536b1cb2e89de678b7f9`
- demo.exe x64: `d3ca06ef7514eeab6e1215259b368a20f71ca0772b92b0bf3355f3a2415c5a1c`

No product source changed; differing clean-build hashes required both deployment locations to be updated. Remote verification follows the delivery commit.
Before this accounting line the staged change covered six files, 44 insertions and three deletions, including four rebuilt binaries. Product source and the test suite were not changed; the isolated C observer and its captures are task scratch, with their observations and limitations recorded above.
