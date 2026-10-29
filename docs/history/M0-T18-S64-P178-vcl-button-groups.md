# M0 T18 S64 P178: VCL button container frame models

TButtonGroup and TCategoryButtons are exact-name candidates for the existing grid nonclient frame and scrollbar renderer only when the application-owned Unicode class, same-owner parent and other grid eligibility checks hold. Application client drawing, icons, button content and interaction remain untouched. These restrictions are not evidence that actual VCL controls use this path.

The previous P177 package failed the positive attachment assertion in the self-authored x86/x64 models for both names. The P178 models pass on both widths: the target edge changes by 2736 pixels and the client by zero; scrollbar state, negative identity cases, reparenting and cleanup pass in the fixture. These are models, not genuine Delphi controls; actual button appearance, real layouts, other DPI and integration remain unverified. The available GoVCL test library does not export factories for either class.

The first clean package run timed out in x86 CMake compiler detection under the sandbox and was not released. The retry completed clean x86/x64 builds and ABI checks. The actual same-snapshot package passed 20 focused cases in 12.75 seconds, no skips; this is not the full integration matrix and physical input was not tested. Four files were published to both assets root and O:\winnt\system32 with matching hashes. Both installed copies passed both model cases on x86 and x64. Original graphics, title and font rendering were not changed.

M0, T18 and S64 remain open.

The locally available DelphiVCL4Python 1.0.7 Win32 and Win64 bindings were inspected with Python `hasattr`: neither exports `ButtonGroup`, `CategoryButtons`, `Outline`, `TabbedNotebook`, `DriveComboBox`, `DirectoryListBox` or `FileListBox`. This only limits these bindings as test factories; it is not evidence that genuine controls cannot be tested or that model-only support works in real applications.

Embarcadero's [VCLButtons example](https://docwiki.embarcadero.com/CodeExamples/Sydney/en/VCLButtons(Delphi)) explicitly assigns `BorderStyle := bsNone` to TButtonGroup. This documented configuration cannot satisfy P178's CLIENTEDGE gate and its button items are outside P178's nonclient renderer. This is source-level evidence of a coverage gap, not a run of that example; actual button drawing and interactions need a genuine Delphi sample.

## Real TCategoryButtons sample candidate

The author's [WhiteStarUML portable release](https://sourceforge.net/projects/whitestaruml/files/WhiteStarUML-NoInstall-5.5.8.zip/download) was obtained using curl after the earlier downloader returned HTML. The 30,723,693-byte ZIP has SHA256 c8a4fd77420e2c586e33e6c47e5c428c61ac3df6009a0e0f5b5fdcc809d3bdf6; its 7,289,096-byte executable has SHA256 f4ae9a7b968f2fdf15926dd4ba64af4c4ba7e1c80ad986da5db1e5d59f961235. Static inspection found a TPF0 form resource with TCategoryButtons named PaletteNavBar and category data. This establishes a promising real sample, not runtime creation, behavior, compatibility or a fix. The archive was extracted only into registered task scratch; no installer, registration or application execution occurred. Startup side effects and safe environment isolation remain to be investigated before launch.
