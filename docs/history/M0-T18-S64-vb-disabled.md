# M0 T18 S64 P5 — real VB disabled states

The optional native intrinsic observer now accepts `disabled`: after its usual
state checks it disables and reenables four option buttons, four checkboxes and
two combo boxes, capturing each state. It checks the original check/selection,
window style and font return unchanged. In styled runs it verifies the source
checkbox/radio glyph (182 pixels each), disabled combo arrow (32 pixels each)
and a change in each complete control image. It does not send BM_CLICK to a
disabled control: an explicitly sent message changed the state in both bare
and styled samples, and is not equivalent to a user click on a disabled HWND.

The original VB5/runtime50 and VB6/runtime60 x86 samples at 96 DPI each passed
bare and styled, with ten controls and no owned processes left. Fresh candidate
runs passed both, followed by installed assets VB5 and system32 VB6 runs. Bare
PrintWindow captures did not always differ when disabled, so the baseline does
not assert a pixel change. The initial probe also misread EnableWindow's return
value; these false starts and the fixed procedure are documented in
[VB evidence](../etc/evidence/vb-current-s64.json). No physical input, focus
traversal, accessibility, IME or handler notification is claimed. Other DPI,
runtime versions, images and the deferred OCX/DriveListBox scope remain open.

The same-snapshot GCC 16.1.0/Ninja/ccache 4.13.6 x86/x64 build passed source,
resource, governance and PE/ABI gates; snapshot manifest SHA-256 is
`7099dee30e76e6ca94a59d87d310407819dd61e885dd1f77caacd30eb370be75`.
Fourteen focused actual-package tests passed in 11.24 seconds at four jobs,
zero skips; other package cases were not run. The optional real samples are
not required for project builds. No renderer or graphics source changed; the
rebuild changed binary hashes, so the four tested files were published to
assets root and O:/winnt/system32 with matching hashes:

| File | SHA-256 |
| --- | --- |
| demo.exe | `a047ef6eb1043f95ea7f7570a03ab0d79946511b0dbb1d1dce111c3ec4544343` |
| ntw16s32.dll | `ed2f091ce0e35699cdb732811f001ae2e06226570a12a2b471f16785e3ccaf03` |
| ntw16s64.dll | `b496e1fb277ed29bbd2bb312a9e30e9441c513478e85efdb452fb2580249bd00` |
| run16s.exe | `1efbef53075aa2fd8f3028dfeb286486525f3ab51dfc4febec5e2da9e3ef4087` |

Test source scope: 26 added lines, 0 removed. The runtime and application
retain their values, events and fonts; the source change adds only an optional
observer scenario. S64's separate VCL scope, cross-process hook diagnosis and
VB task-level gaps remain open.
