# M0 T18 S42 — VCL date-picker button

The DLL recognizes a newly created runtime-owned `TDateTimePicker` only after
observing the registered native date-picker procedure during WM_PAINT and
checking the public date-picker information and same-thread parent. It paints
only the native dropdown button, using the existing Win3.1 combo glyph at the
window DPI. The date field and native month calendar remain native. Checked,
up/down, app-parse, RTL and other unsupported forms are retained natively; no
claim is made for other VCL versions, custom layouts or all DPI values.

A self-owned C native superclass fixture verified x86 and x64: fake and
checked forms rejected, date and keyboard notification, popup open/close,
clip/DC state, reentrant removal/destruction, and exception cleanup. At 96 DPI
765 changed pixels lay inside the button, zero outside. The old released x64
DLL failed the new test at the attachment assertion. Actual DelphiVCL4Python
1.0.7 instances at 144 DPI passed on both architectures using the final package:
public button (104,0)-(130,24), keyboard change and restoration, one dropdown
and close notification, unchanged date during popup. Earlier candidate
comparisons found 552 changed pixels confined to the same button on both
widths. The first S41 x64 probe's exit 1 remains unexplained. Physical input,
other DPI, versions and variants remain untested.

The new exception test revealed an x64 crash after a WM_PRINTCLIENT exception:
the VB paint scope remained on a thread-local stack after unwind. Native-only
control comparison did not crash. The bounded print cleanup now restores that
scope and prior status/spin state as well as its region and attachment lease;
both widths subsequently pass the exception/removal and color-box regression.
The native-control comparison is evidence for this case, not a claim about all
exception paths. Original resource identity is byte-exact.

Independent GCC/Ninja/ccache clean x86/x64 builds and ABI tests passed. The
actual four-file package passed 16 focused cases, zero skips, in 9.23 seconds;
the full integration and physical input are pending T18 closure. Both published
locations match the tested hashes below, and installed system32 x86/x64 date
probes passed. Source manifest and resource identity checks passed.

| File | SHA-256 |
| --- | --- |
| demo.exe | `4da7c0bd4a3b6b211af799347231482db238b5d6aff3724b34a682a5b0fd2899` |
| ntw16s32.dll | `df5cba8ba257e3142964ccb73e87b7cb22f491a496c51c9a933b191188941b56` |
| ntw16s64.dll | `a8b2dd29674e339fecf241bb7fc7f36fc9fdf77cdc06e7f21ccd4368ef48af55` |
| run16s.exe | `a3fc91cba867e64749db6ea1f8ae17e9d6061df962ba37808bd3ce365e83b639` |

T18 and M0 remain open; S40 screen presentation differences remain in TODO.
