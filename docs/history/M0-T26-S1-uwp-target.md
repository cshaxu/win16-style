# M0 T26 S1: shared UWP target discovery

The existing CoreWindow/ApplicationFrameWindow search and association checks
were extracted from the product coordinator to `lib-style/uwp/target`. The
shared `style_uwp_target` contains borrowed observations only; callers must
revalidate before acting. The product `target_change` still owns the existing
Win16-specific hook export, message protocol, module loading and removal
semantics. The launcher, global coordinator and tests now call shared discovery
directly; the old product discovery functions and type were removed.

Incremental x86/x64 builds passed for the product DLL, common library and
UWP manager fixture; the x64 launcher and resolution fixture also linked.
Direct tests using self-owned windows passed both architectures for hidden
window exclusion, lookup by HWND/PID, matching identities, PID/frame mismatch
and destroyed target; the native-title fixture also passed. Calculator image
resolution passed on x64. The common archive's undefined symbols contained no
Win16, Win2000 or product-private names in either architecture. No private
desktop or computer-use was used.

These fixtures simulate window classes; they do not prove real UWP activation,
cross-process hook installation, actual renderer appearance, WinUI support,
physical input, or the complete test matrix. The user-requested broader
cross-style extraction remains open.

The clean x86/x64 MinGW GCC 16.1.0, Ninja and ccache 4.13.6 build passed;
snapshot SHA-256 is `cac9004bfa4203f75e6392e82d5af4ebeac883ca5d5016c3d74690e724482a75`.
All eight artifacts were published to `assets` and `O:\winnt` before the
13 direct post-publication focused checks. Both architectures passed target
discovery, native title, UWP frame layout, shared window lifecycle, the
published Win16 menu and Win2000 scrollbar geometry; x64 Calculator image
resolution passed. Both destinations matched the candidate hashes below. The
actual UWP and WinUI application limitations above remain open.

| Published file | SHA-256 |
| --- | --- |
| demo.exe | `52cfd90e0e37af6fd502429d406f8bdb4ac8dd08240d02d91da738af29e6cf21` |
| ntw16s32.dll | `4faf137a20d41e8302d18406f42a579fe5bcd7396472afc49a1a8428a6b39216` |
| ntw16s64.dll | `946ca71b1d28b8e71745497f36f34eb28359f25ffd1f90d01c326ec7338bc873` |
| ntw2ks32.dll | `898df166ed86b56569385ca32b6802a27d8a4a58036c8832e8460c38c2496ec4` |
| ntw2ks64.dll | `3cf989d989cb9a647fa63074bb575c75797d9f185498fc3a09ad883c9cbe0f1c` |
| ntwshk32.dll | `8d1269d6bbbbd184072822c8ee1d4da4b9ba49acbbde04d9b2f216eff4f24d6d` |
| ntwshk64.dll | `9d8029604fd5e2c1119838e34e892b266d6a0155ea29681b53dc712e49c708cc` |
| run16s.exe | `ff2bab9299b1706f882223b77f73a1af8e5c05803635eab706773dc899dc69db` |
