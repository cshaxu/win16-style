# M0 T18 S31 — bounded VCL static text

The old package did not attach the actual `TStaticText` HWNDs created by the
official DelphiVCL4Python 1.0.7 x86 and x64 runtime. The two owned instances
used normal Segoe UI and disabled Tahoma text at 144 DPI. The class belongs to
the VCL module and forwards its actual `WM_PAINT` to the registered system
`STATIC` procedure; it is windowed, correcting the earlier S26 grouping.
The runtime wheels were downloaded to task scratch only, are not signed, and
are not a project dependency. The x86 Python 3.13.1 embeddable executable has
a valid publisher signature. No downloaded source or image is shipped.

The adaptation admits a creation-time exact-name candidate but reports it
attached only after a same-thread `WM_PAINT` calls the actual registered
`STATIC` procedure from the class owner's module. A private, nested, bounded
scope permits the native procedure's own `DrawTextW` call to borrow the
existing metric-matched System font on its DC. The screen and buffered disabled
DC paths are covered. The application font handle and native painting remain
owned by the application; `WM_PRINTCLIENT`, measurement, RTL and non-ASCII
text remain native. Nested unrelated calls form barriers. The two new
process-local hooks use the existing transactional bootstrap; there is no new
product file, public export, graphic resource or external runtime dependency.
Before staging, `git diff --numstat` for product source recorded attachment.c
73 added/8 removed and window_titles.c 33 added/0 removed; the new private
header contains 17 lines. The new native test contains 169 lines. Existing
application state, source font and native callback ownership remain unchanged;
the older paths retain their existing behavior outside the verified scope.

The test-only v6 C model uses same-class unstyled reference windows: x86 and
x64 clean candidate display each differ by 599 RGB pixels from native, while
print differs by zero. It checks the original font in parent callbacks,
reentrant font changes, a disabled native reference repainted in a nested
callback, and an unrelated fake class. Both bare models produce zero screen
and print differences. The old published package fails the verified-attachment
check on both widths. The clean candidate additionally ran against the actual
VCL runtime on both widths: normal and disabled controls were attached, source
fonts stayed Segoe UI/Tahoma, and native captures showed 1,107 changed pixels
outside the top 26 rows and zero changed pixels within those rows. Two owned
instances exited successfully. A local clean dual GCC/Ninja/ccache build,
source/resource/governance/ABI gates and the actual package passed 18 focused
cases with no skips in 39.59 seconds; 103 other cases were not selected.
Installed entry tests passed x86 from assets and x64 from system32. The tested
package and all eight deployed files matched SHA-256.

| Tested and deployed file | SHA-256 |
| --- | --- |
| demo.exe | `1d335de5e1f8c17bd7ca04e4cf07233f8f771ca0081553e3b615c3ee345ed13d` |
| ntw16s32.dll | `6eef1ce6d7f1c84abbbd1640c9bf983ab0fc5cde92f4f7b25ccf7eab0cf82e1e` |
| ntw16s64.dll | `939e340fbea69a6ea518ac47607a8e7b710feab4e76aeaca968f2a2e0d1bfb47` |
| run16s.exe | `071116a9f34907ac6abbefa8c899772c6005a226f16d54d69421ad4b9bb6ca74` |

The first focused batch passed both font-layout and menu cases but stopped on
an x86 VB child timeout. A serial retry on this candidate and a contemporaneous
old-package control initially reproduced the same timeout. A scratch-only
instrumented old-package run later passed, as did the unchanged candidate VB
tests on both widths in the final 18-case batch. The intermittent timeout is
not attributed to a proven product cause and must not be erased from the
record. Menu cancellation passed at 73.92/72.82 seconds in that first batch;
its speed needs separate work under the user's test-efficiency requirement.

In a separate exception experiment, raising through a Windows window callback
terminated each owned test instance with `0xC000041D` before the test handler
could recover. The code uses the project's existing native unwind cleanup for
the new scopes and DC lease, but recovery across that system callback was not
proved. Physical input, real printers, other VCL releases, static-linked real
applications, non-ASCII text and full T18 integration remain unverified. The
32-bit raw Python wheel SHA-256 is
`e6db59c0e1fd6900050531e7f02f5c3e04c3fa7e2749b08d52f390f3658e18b9`;
the x64 wheel is
`19729cbaf892e6de8e4d57e0bb7faf9c5857c58708159cffd610c1117dd5059e`.
Scratch remains registered until evidence and final T18 handoff are settled.
