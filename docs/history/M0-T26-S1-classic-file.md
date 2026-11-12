# M0 T26 S1: shared classic file dialog contracts

`lib-style/common/classic_file_contract` owns the existing old-style
OPENFILENAME and COM option conversion, filter and text copying contracts.
`classic_shell_item` owns the existing Shell item, shortcut and classic
space-separated multi-selection result conversion. These are opt-in classic
contracts, not policies imposed on every style. Allocated filter/text memory
uses `CoTaskMemFree`; returned COM interfaces follow COM reference ownership.
The existing Win16 dialog and injection adapters now call those shared entry
points. The previous copies were removed. The shared library depends on Win32
and Shell interfaces, not on the product or Win16 renderer.

The independent C fixture exercises classic conversion, unsupported multi-select,
filter serialization and actual Shell item creation without linking Win16.
The existing Win16 dialog integration fixture exercises the broader contract.
This extraction does not implement a Win2000 dialog or prove XP/Vista/Win7
compatibility. Complete T26 inventory, remaining sharing, real-application
coverage and the previously identified Win2000 DWM restoration issue remain open.

The clean x86/x64 MinGW GCC 16.1, Ninja and ccache build passed. Source
snapshot SHA-256: `2ed388c4faec0755bdd6bf21ec12f4da9c4d07a7fafd7c0b422d151484bb2669`.
The eight artifacts were published to assets and O:\winnt before 14 focused
post-publication checks. Both architectures passed independent classic file
contracts, Win16 dialog integration, print ranges, global window lifecycle,
UWP target fixture, published Win16 keyboard menu and Win2000 scrollbar
geometry. Undefined-symbol inspection found no Win16, Win2000 or product
private references in the common archive. Both destinations match:

| File | SHA-256 |
| --- | --- |
| demo.exe | `bfa0fad01cbbe4901bfbe606bbb6abc2a5b0bbb4936a5ff3f51a4d0bac66d867` |
| ntw16s32.dll | `824727e778e8bd26cc5e3275e4c8403abbc22b601980ba9714303ed97cc96669` |
| ntw16s64.dll | `44c672ec5224aaa8501c964baf3a19912ac820e5fdffe11513fcfc10ed1788f0` |
| ntw2ks32.dll | `093926da50cbc889a67a1c704317a6c4b773f00eb77f3256335fb15dc792896b` |
| ntw2ks64.dll | `736d8b6cf961fd3313b15647f2847d7e336de99b835e8ea5d50cf71e0ebd50b2` |
| ntwshk32.dll | `79322fd866b36c81f00220c2b68de31ad2adb61b819f36a0bfe7b014546ac2eb` |
| ntwshk64.dll | `6b3b52b53386de71e587c70f907fadb745c11f67dd0add0c5306f4385ad48b73` |
| run16s.exe | `3cde7cbde3bdc01c2d2d1be9a28b9667d0d0f66b5ae14362d997147843530b2a` |
