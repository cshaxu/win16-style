# M0 T26 S1: style component layout

The six existing Win16/Win2000 component directories were moved from
`src/lib-win32` to `src/lib-style`; their corresponding component tests moved
to `tests/lib-style`. CMake, includes, imported-source destinations and
receipts, and the five original-resource verification paths follow the move.
Existing source/resource attribution and byte-exact retained artwork remain.

This is a directory and dependency-path migration, not completion of the
cross-style mechanism extraction. In particular, window/menu tracking,
control lifecycles and modern-frame adaptation still require the S1 audit and
subsequent work described in the proposal.

Before the clean build, x86 and x64 incremental product builds and 16 direct
component tests passed: Win16 common/window/control/dialog links, UWP frame
layout, Win2000 caption, shared global window, and print page ranges. Source
manifest, original-resource identity, governance and their self-tests passed.
These tests did not use a private desktop or computer-use. Existing Win2000
DWM restoration and screen-pixel observations remain unresolved as documented
in the prior S1 delivery; they are not included in these passing counts.

The clean x86/x64 MinGW GCC 16.1.0, Ninja and ccache 4.13.6 build passed;
snapshot SHA-256 is `19e9c57d04fe34515e70642ca2bcd690756ef4ece41590c77ab3427da4172688`.
All eight artifacts were published first to `assets` and `O:\winnt`; the
subsequent 22 direct focused checks passed, including the published launcher
with Win2000 scrollbar geometry and matching build binaries with both styles.
The eight hashes matched the candidate at both destinations after the checks.
Win2000 DWM restore was not rerun as a passing check; its prior failure remains
open. These results do not prove physical input, visual fidelity, actual UWP
applications, or the complete regression matrix. S1 and T26 remain open.

| Published file | SHA-256 |
| --- | --- |
| demo.exe | `48d5b6976d06422ca1f4eac23ca1297a005abca87fca499902b5ebc6965bfe47` |
| ntw16s32.dll | `d51bad59658b149585fc26717fb3e6c06c28c8aed0735fd8a9e8532cd1defec0` |
| ntw16s64.dll | `6f015ce0b3737af5750298e3164c307fbb64a5394794e1ae760661c3ca3bf2bc` |
| ntw2ks32.dll | `c0a66f09e871479d2078738da40ddf0b885d2658c8e4b0b98b704e7fb2b5e070` |
| ntw2ks64.dll | `85396f2e9c171b7ddd82f8c7083d65f7aa2266d5540686f047dcbe01cf3d67d6` |
| ntwshk32.dll | `0ab41a8c2843a73d71d3c2d87732ad19764934a1ab4c42997714b3694c075949` |
| ntwshk64.dll | `8a295d1c42180d01190071f49e768c207ee0cab6e5c9a4041e9f706e6a09d097` |
| run16s.exe | `a574bd95a52f3075fa45966771cfbb751b2d502bd4611aea328d75c44aedc2ad` |
