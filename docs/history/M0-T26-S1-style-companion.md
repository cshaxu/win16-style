# M0 T26 S1: shared companion-style module loading

`lib-win32/style/companion` now owns sibling module path resolution, file
identity verification, caller-selected export resolution, error cleanup and
module pinning. Its public contract contains no product name, DLL name or
style-specific exports. `ntw16s-dll/global/window.c` supplies the Win2000
module and three entry names, and continues to own style selection and the
existing shared window transition. The former duplicate loader was removed.
The shared code depends on the existing process-global file identity function,
not on a product or renderer implementation. An owned fixture tests missing
exports, output cleanup, rejected path components, valid exports and pinning.

The clean x86/x64 MinGW GCC 16.1, Ninja and ccache build passed; source snapshot
SHA-256 `59be3ef5333a813688694151547502d2f9ad7a373c049a5481052c1382c975ae`.
Eight artifacts were published to assets and O:\winnt before 12 focused
post-publication checks. Both architectures passed the standalone module test,
shared theme test, Win16 control linkage, published Win16 menu, Win2000
scrollbar geometry, and the two-style window bridge. The bridge tests used
verified copies of the actual published DLLs and tested normal-to-style,
both style transitions, removal and takeover of already styled owned windows.
This tests the local window bridge, not system-wide process enumeration, UWP,
physical input or complete visual acceptance. The common archive has no
unresolved Win16, Win2000 or product-private names. All published hashes match:

| File | SHA-256 |
| --- | --- |
| demo.exe | `63c268991d462861bb02d682d06b2f191414aaef3ae5ea5e9b350e42edc4fa0f` |
| ntw16s32.dll | `1f4fb5913ba905cb6dddf93bd4ede0ce106b384c1d87979584e920251aa21b51` |
| ntw16s64.dll | `1411721f665a9440add4b1500c92cc41d3bb993de990d5ea74047ab927c76eb1` |
| ntw2ks32.dll | `572c90df8c0a8a0be8e2089eb1e10c92dc739321c764746761d3a1fec5f6df73` |
| ntw2ks64.dll | `691b268bdd38c1600b768ff726a6b83699f1d7051290b570e6e463417caff6db` |
| ntwshk32.dll | `a2d15569926d88691427c58a228e94f80583eb54bd621d55d838c0845d006585` |
| ntwshk64.dll | `4279b43c822ea2747e50a08f5208140b6a40aa49059ea120fd96d731c89934b2` |
| run16s.exe | `79c8d6e243f8c6ca6e60879fc4fd6915ed3b7bfd87a16d2c4ba3170226ecaf2d` |

T26 S1 remains open; full mechanism inventory and other cross-style
integration, including theme restoration in Win2000, remain to be completed.
