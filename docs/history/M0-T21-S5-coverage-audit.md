# M0 T21 S5: global mode coverage audit

## Contract and evidence

| Requirement | Evidence | Boundary |
| --- | --- | --- |
| Manual `run16s --global`; no login startup | S3 controller and CLI; package integration | Controller runs until Ctrl+C or termination; current desktop only |
| x86 and x64, existing and new native windows | S3/S4 private desktop; actual 64-bit demo and optional 32-bit notepad | Hook delivery and each window depend on Windows integrity, desktop, process and GUI-thread responsiveness; initial sweep is bounded and reported failures |
| Existing and future standard windows and basic controls | S4 captured actual frame/control drawing; S5 dual-width probe checks Link, Tab, Header, Progress and Button before and after global startup and after removal | Global and ordinary modes now use the same attachment classifier; features requiring startup-only process hooks, application-drawn content and unsupported or unverified framework controls are not generally covered. Layered and tool windows remain excluded |
| Menus | S5 x86/x64 native menu-first-paint and system-menu selected-color probes with a real global controller | Menu switching, physical pointer input and all menu variants not covered by this audit |
| Normal run16s stays independent | S4 x86/x64 ordinary targets in both startup orders; four post-stop windows and four child processes | Applies to verified processes and controls, not an unconditional guarantee for unknown third-party hook combinations |
| Stop global mode | S4 four globally owned windows detach; targets stay alive; later independent window remains native | No pixel-perfect restore: screenshot fingerprints differ, failed notifications and residual styling are possible. Pinned target DLL remains until the target exits and can keep it locked |
| Duplicates / failure | S4 duplicate gives 183; owned controller crash ends host; x86 host death now stops controller with 1114 | In-flight reentrancy, forced console close, host failure on other integrity levels and long-running resource stress are not exhaustively tested |
| Security and scope | Current user desktop; shared Windows host denylist; no AppInit or global settings change | Denylist is finite; elevation, PPL, other desktops/sessions, UWP/XAML, shell hosts (including Explorer), and all security configurations are not claimed |
| Four files and original resources | Dual-width package and source/resource gates; S3/S4 hash and remote reviews | x86 system rundll32 hosts the x86 hook and is an OS component, not another distributed product |

The user's requested ideal of styling every program is not a measured universal guarantee: Windows hook delivery, privileges, unsupported frameworks, app-specific drawing and finite enumeration prevent that conclusion. The self-owned probes verify the cited paths only. Original source graphics and notices are byte-exact; ordinary mode and the existing renderer remain in use.

The S5 negative comparison used an isolated v6 common-controls process with five controls. S4 created all five but only the Button was attached (`created=31 attached=16`); the shared-entry candidate attached all five (`created=31 attached=31`). The formal dual-width test additionally verifies already-running and newly started targets and removes all five attachments while preserving control state. The ordinary creation path and late entry share `install_created`; the late entry checks current thread, existing ownership and successful installation, while ordinary creation retains its original creation-time style and initialization. The v5 progress control did not meet the existing classifier's v6 protocol requirement and remains native. Public ABI details are recorded in [S5 ABI](M0-T21-S5-abi.md).

The test evidence, initial failure and S4 fix are recorded in [S3](M0-T21-S3-global-controller.md) and [S4](M0-T21-S4-real-lifecycle.md). The final S5 package and deployment are recorded below; remote review follows the delivery commit.

## Final package

`s5-unified-final` is a clean x86/x64 GCC/Ninja/ccache build of the shared-entry source, with the ABI, source-provenance and original-graphics gates passing. Its actual four-file package passed 19 focused cases, no skips (`s5-unified-check`): dual-width existing/new/common controls, mixed ordinary/global ownership and propagation, global first-paint/system menus, demo and ordinary startup/propagation, plus host/controller failure and duplicate-start cases. A final optional private-desktop O:/windows/notepad.exe run used the actual candidate's x86 DLL for owned old and new processes; three start-sweep notifications timed out or failed, while the targeted windows were styled and stayed alive after stop. No complete application or DPI matrix, physical input, or long-running resource stress was claimed.

The tested batch was published to assets root and O:/winnt root:

- run16s.exe x64: `be8b30b8f73ed0981e18e5b91d3ae9954da3bafc41173afba6e345be973cecfa`
- ntw16s32.dll x86: `5c8571b299c09f27b0494b0d6938f764efd7158651e2286207846546287f33d2`
- ntw16s64.dll x64: `18b928b00b2b7eba21368e0d7e2e778fb25a8ac7f8220cc625bb888a14dc9c68`
- demo.exe x64: `d8c7c25d6fa5a72f94354d864b9764d3a249aaf7d80d3754701f7ed3190f32cf`
The staged delivery before this accounting line covered 26 files, 264 insertions and 50 deletions. Review searched `install_created`, `Ntw16sWindowAttach`, `Ntw16sWindowAttached`, `Ntw16sWindowRemove` and `ntw16s_global_window_apply`: both entry paths use the same installation classifier; the global wrapper records only attachments it owns, retains existing ordinary owners, and removes its own through the existing lifecycle owner. The former direct renderer-specific global control/frame selection was removed.

Subsequent evidence: [S6](M0-T21-S6-threads-resources.md) adds double-width multithread and three-cycle resource checks; [S7](M0-T21-S7-input-dpi.md) adds controlled edit/button messages at 144 DPI. Their stated limitations continue to apply.
