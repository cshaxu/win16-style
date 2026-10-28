# M0 T18 S64 P54 — VB5 sample observations

Two independently sourced VB5 examples were investigated as research-only
inputs. Neither is shipped or required to build or test the product.

`FORMLOAD.EXE` from the `mastering-visual-basic-5` archive's accompanying
disc has SHA-256 `b9f259fe881d1646afc8e9e07d0d919503ff1d71536854b33c570c8de4afd84c`.
The ISO SHA-1 is `a8ad94afabe514d87a23ed5c066a4882b9966f14`. Its adjacent
forms describe two initially hidden buttons; their load code deliberately
spends five seconds in each of two form initializers. A private x86 instance
with the task-local VB5 runtime was exercised once without and once with the
released launcher: after both buttons became visible, each opened a visible
VB5 form, each form was closed, and the application exited with code 0.
The styled main-window capture showed the project title and button rendering.
An earlier probe acted before initialization finished and timed out; that is
not evidence of a product regression. This is one sample, not a complete
keyboard, physical input, DPI or application compatibility test.

The `visual-basic-5` archive's `VB/VISDATA.EXE` has SHA-1
`00dfade18f0f9f0a288318daf1e7d5e7bc40ffd1` and SHA-256
`2e3dbfb01edf7241c6716c2e7e5363e253238e3bbaf2b31d7154da714bfb29b6`.
It imports `MSVBVM50.DLL`; the adjacent
`VB/SAMPLES/VISDATA/DATAFORM.FRM` declares a VB Data control. Source says the
program reads saved settings and can reopen the last database at startup; it
also saves settings on exit. The specific VisData settings key was absent
before each private observation. A task-local manifest supplied available
private OCXs and a DAO bridge, without component registration. The unstyled
program instead displayed `Unexpected error; quitting` before its main window
or the three OCX modules loaded. The self-owned job was terminated rather than
requesting normal application shutdown; the settings key was still absent.
Adding the declared VB5 Extensibility type library did not change the result.
The message is present in the VB5 runtime; its precise underlying failure is
not established. Direct OCX catalog observation of DBGrid returned
`0x80040112`, but that result alone does not diagnose VisData's earlier error.
No database was opened and no VB5 Data drawing or navigation was observed.

The original graphics and product sources were unchanged. The two published
directories still contain the four P52 files with matching SHA-256 values; no
new product binary was built or published for this research record. VB5 Data,
other intrinsic variants, and full task integration remain open.
