# M0 T21 S8: DPI-unaware virtualized path

The existing native integration fixtures gained a test-only process environment option. With it, the owned x86 and x64 targets select `DPI_AWARENESS_CONTEXT_UNAWARE` before creating windows. Windows reported 96 through `GetDpiForWindow` for both, while the per-monitor-aware comparison in this same environment reported 144. No system DPI, theme or monitor setting is changed. This is an effective 96 DPI virtualization path on the available desktop, not evidence from a physical 100% display or multiple monitors.

Both isolated runs passed existing/new windows, ordinary/global ownership, post-stop ordinary children, and controlled edit/button messages before, during and after global mode. The unaware probe explicitly fails unless both targets report 96. The initial unaware run observed one failed or timed-out initial notification, while all targeted windows passed; individual failed notifications were not identified. No physical mouse, keyboard, IME or complete DPI matrix is claimed. Product source and the simple window ownership behavior were not changed.

The final clean package, actual-package results, hashes and remote review are recorded below.

`s8-dpi-final` passed the x86/x64 clean GCC/Ninja/ccache build and the source, original graphics and ABI gates. Its actual four-file package passed 18 focused cases with no skips (`s8-dpi-check`). Final output reported `dpi32=96 dpi64=96` for the DPI-unaware path and `dpi32=144 dpi64=144` for the per-monitor-aware path; both ran native/active/stopped input checks and the mixed-ownership chain. No product source changed.

The rebuilt tested batch was published to assets root and O:/winnt root:

- run16s.exe x64: `c35827b5ec076f816f1cbd23557fd0bdfc648cc99686b0a8abfc59afefaaf6d6`
- ntw16s32.dll x86: `7244edd7efda3920725c681b534a955edc237e5fc3cc967fb42d0f4c5e3f09cf`
- ntw16s64.dll x64: `9f20c454ff9d612b97aeed2d9c4ecebd426e5b468a49b1bef78aa3445a186fe0`
- demo.exe x64: `31884fc1273e54edf92d5e220f01c176165fc88caa17ad17c86d94ec6d8caf2e`

Clean-build binary hashes differed from the previous batch, so both deployment locations were updated. Remote verification follows the delivery commit.
The staged diff before this accounting line covered 10 files, 55 insertions and 9 deletions, including four rebuilt binaries. Production source is unchanged; only the test fixture awareness context, driver option, package test registration and evidence were added.
