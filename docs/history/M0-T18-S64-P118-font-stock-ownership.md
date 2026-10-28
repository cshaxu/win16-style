# M0 T18 S64 P118: stock System font lease ownership

While investigating Delphi multi-pane status text, a self-owned x64 model ran at high CPU. A debugger attached only to that owned worker showed it looping in `win16_style_system_release`, called from status font return. The root cause was introduced in P117: in `system_acquire`, the `for` initializer assigned the current variant-list head even when `isolated` made the loop condition false. When the source was already the native System font, the fast return reached cleanup without allocating a new node and freed that still-live head. The variant list could then contain freed memory. P117 was affected; the multi-pane prototype merely made the condition easier to encounter.

The fix executes the list traversal and its initialization only for non-isolated acquisitions, so the local cleanup pointer remains NULL on the isolated stock-font fast path. Existing cached and isolated non-stock paths are unchanged. No public ABI, graphics, resource provenance or user interface style was changed. Search `rg -n 'system_acquire|isolated|free\(v\)' src/ntw16s-dll/font.c` identifies the sole source path and cleanup.

A deterministic project-owned C test instruments deallocation of the actual in-repository font implementation: it creates a shared non-System lease, obtains an isolated stock System font, and checks the shared variant was not freed and remains usable. P117 source failed this assertion in both widths; fixed source passed both. An actual P117 x64 DLL in a self-owned two-status-window model (Arial plus System, then redraw and destruction) timed out at the test's 8-second limit; P117 x86 happened to pass that one run and cannot be assumed safe. The fixed actual package passed the same live model in both widths. The diagnostic multi-pane prototype with the fixed font code also passed the formerly failing x86/x64 border/icon models, but that prototype is not in this release and multi-pane text styling remains under investigation.

The clean C17 GCC 16.1.0/Ninja/ccache 4.13.6 x86/x64 build passed PE, 31-export ABI, source and byte-exact original-resource checks. The same four-file package passed 26 focused cases in 17.04 seconds with zero skips, including deterministic stock ownership, actual two-window DLL lifetime, isolated-font semantics, real handle reuse, Delphi status, combo and VB regressions. The actual package was also run in self-owned Delphi 1.0.7 x86/x64 at 144 DPI; the previously verified System-shaped simple-status text remained visible and targets/launchers exited zero with owned jobs empty. It was published to assets root and O:/winnt/system32; both destinations matched the tested hashes and passed both widths of the live stock-font model. Snapshot SHA-256 `051ab6f70709760d6d058723c5dba3ca7c5902cabf9c508eaa52df5771d5e574`.

| File | SHA-256 |
| --- | --- |
| run16s.exe | `9a4ca558e2ab815842b045aeee2355bd66572acadd8fe8243a722e265540b29e` |
| ntw16s32.dll | `adb46412dc7532f4b078d8e85f90d8200f15d28822e5db350eea94d0418eef08` |
| ntw16s64.dll | `78599307b9f388541ea9f2eb8e6d1e1ffe65155cba534d7bd55cf90535c3789e` |
| demo.exe | `9ffa2b2094470eba7470a7c103bb87dc52c080182c237d2f8a7512938ff51cb6` |

The tracked implementation, CMake and package selector changes are 13 added and 1 removed line, plus two new self-owned C tests totaling 20 lines: 33 added, 1 removed, net 32. Documentation, receipts and binary files are excluded. Other allocator and exception paths, additional DPI/runtime versions, physical input and the full T18 matrix remain unverified. M0/T18/S64 remain open.
