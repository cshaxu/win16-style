# M0 T18 S64 P178: VCL button container frame models

TButtonGroup and TCategoryButtons are exact-name candidates for the existing grid nonclient frame and scrollbar renderer only when the application-owned Unicode class, same-owner parent and other grid eligibility checks hold. Application client drawing, icons, button content and interaction remain untouched. These restrictions are not evidence that actual VCL controls use this path.

The previous P177 package failed the positive attachment assertion in the self-authored x86/x64 models for both names. The P178 models pass on both widths: the target edge changes by 2736 pixels and the client by zero; scrollbar state, negative identity cases, reparenting and cleanup pass in the fixture. These are models, not genuine Delphi controls; actual button appearance, real layouts, other DPI and integration remain unverified. The available GoVCL test library does not export factories for either class.

The first clean package run timed out in x86 CMake compiler detection under the sandbox and was not released. The retry completed clean x86/x64 builds and ABI checks. The actual same-snapshot package passed 20 focused cases in 12.75 seconds, no skips; this is not the full integration matrix and physical input was not tested. Four files were published to both assets root and O:\winnt\system32 with matching hashes. Both installed copies passed both model cases on x86 and x64. Original graphics, title and font rendering were not changed.

M0, T18 and S64 remain open.

The locally available DelphiVCL4Python 1.0.7 Win32 and Win64 bindings were inspected with Python `hasattr`: neither exports `ButtonGroup`, `CategoryButtons`, `Outline`, `TabbedNotebook`, `DriveComboBox`, `DirectoryListBox` or `FileListBox`. This only limits these bindings as test factories; it is not evidence that genuine controls cannot be tested or that model-only support works in real applications.
