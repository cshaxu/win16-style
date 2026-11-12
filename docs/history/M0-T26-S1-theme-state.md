# M0 T26 S1: shared theme ownership

`lib-style/common/theme_state` now owns the existing same-thread staging,
subclass capture, temporary native-theme suppression, latest intercepted
SetWindowTheme arguments, ownership, restore and reentrant cleanup. The
Win16 adapters call this shared API; the Win16-specific BUTTON/COMBOBOX/
TEXTGLOW classification remains in `win16-control/theme_kind`. No Win2000
caller was added: its existing native control behavior and restoration limits
are unchanged. Only theme requests routed through the shared API can be
recorded; an unobserved preexisting theme cannot be reconstructed. The caller
provides the native theme function. The CMake test uses the repository's
COMCTL32 subclass import; generic consumers must provide suitable subclass
imports or a suitable common-controls activation context.

The first published candidate's new independent test failed at process start
in both widths because its link omitted the COMCTL32 subclass import. All
other focused checks, including the original Win16 theme-stage worker, passed.
The test link was corrected and verified before the clean rebuild. No product
behavior was changed to mask this test error.

The corrected clean x86/x64 MinGW GCC 16.1, Ninja and ccache build passed;
snapshot SHA-256 `2ea2f263c24d22cf2ced7fda9dc6e00fcbff76b32c933a62494b91c2404d3a0f`.
All eight files were published to assets and O:\winnt before 14 focused
checks. Both architectures passed independent theme ownership/restoration,
the original Win16 theme-stage allocation and restore test, control and dialog
linkage, shared presentation, published Win16 menu and Win2000 scrollbar
geometry. The theme-stage worker was invoked directly on the current desktop;
its private-desktop launcher was not entered. The shared archive's undefined
symbols contain no Win16, Win2000 or product-private names. Both published
destinations match:

| File | SHA-256 |
| --- | --- |
| demo.exe | `dd6c2eea447390f3009b40a149aa431174d222537e3588ac8cb7b653b4b1ae61` |
| ntw16s32.dll | `f9ef4abad82e36f4adab37a336805ac190d24377719955efd2341a782519774b` |
| ntw16s64.dll | `6aac1b3c0b3d2190b9bc95865516278f648d1bef29c283ede3a6435927248a5f` |
| ntw2ks32.dll | `ea09fb7200a83a491023b3df97c4a2ddd4b7c2f2797fc5e12bf8111fc472102a` |
| ntw2ks64.dll | `1b06959d4661e6b235eeae467a792ab3ca6fec1af6d403e85d36f0f1de1bf411` |
| ntwshk32.dll | `0b2a207069ac8a543d5716a2ffbd73783194e9a01fd2875bc5b0b0c009b2ea6b` |
| ntwshk64.dll | `7193bc4283702de1bba2bc4f3145230ad003aab4e4b154d571008fd3bc99c355` |
| run16s.exe | `185fe33347fecd6f9e1f3edac84f3290b53c083010004c8368bf17dc126f31e3` |

The broader T26 audit, cross-style integration, visual acceptance and known
DWM recovery issue remain open.
