# M0 T21 S12: dynamic controls under global mode

The existing `global-common` integration fixture had only created its five controls before showing each parent. We extended the existing pure-C, job-backed private-desktop fixture to create two ordinary button controls after global mode is active and after the parent is visible: one is created with `WS_VISIBLE`, and the other is created hidden and then shown. Both x86 and x64 targets are tested in a process started before global mode and a process started afterwards.

The first isolated run against the S11 package passed: all eight dynamically created buttons reported window attachment, each generated its parent's `BN_CLICKED` notification when activated by `BM_CLICK`, and all reported detached after global stop. This is an attachment and message-level interaction test; it is not a pixel comparison or physical mouse/keyboard test. Other control classes and creation patterns are not inferred from these buttons. The package suite now requires a separate `GLOBAL LATE PASS` marker, so omitting the new exercise cannot silently pass this case. No production code or ABI changed.

The clean x86/x64 GCC 16.1.0, Ninja and ccache build passed source, resource, governance and ABI gates. The final same-batch package passed 13 focused cases in 30 seconds, with no skips, including the dynamic test, ordinary/global coexistence, both restart widths and multiple GUI threads. This is not the full matrix or physical input acceptance. Rebuilt binary hashes differed from S11 even though production source was unchanged; the exact tested batch was therefore published and verified at both `assets` and `O:\winnt`:

- `run16s.exe` x64: `5d039bb89ac0a1de2e9e7b02bd5481e71c764f8b83720179df97e3688035034e`
- `ntw16s32.dll` x86: `4099bc49348ea31eef7a3346d1c9a94f51eb48aa2a16567ab2356e07668b8848`
- `ntw16s64.dll` x64: `870c206659b3ab383e87c992dcec220af634ad06741a3f191851d5d4b0f809ae`
- `demo.exe` x64: `bb00a607d33206fad0374ba9d636c798a5cec37f7304df4cdab78b8c75da4f31`

The test-source diff before this record adds 16 and removes 2 lines across the driver and target; the package marker changes one line. The existing global window attachment remains the sole owner and renderer; there is no changed production path or new export. The common fixture and the global hook's visible-window path were reviewed with `rg 'global.*(common|hook)|WM_SHOWWINDOW' tests src`; existing top-level and pre-created controls remain covered by the same case.
