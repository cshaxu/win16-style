# M0 T18 S48 — date exception resource test

S47's x86 date test failed its immediate process-wide GDI equality check at
27 -> 25. A further same-package short run reproduced 20 -> 27; a no-exception
WM_PRINTCLIENT control ran four times per width with equal pre/post counts,
though separate process initial counts varied between 20 and 27. These are
observations, not an identified cause for the process-wide fluctuation.

The test now observes the exact `CreateRectRgn` and `DeleteObject` imports of
the loaded product DLL only in the owned test process. On each exceptional
WM_PRINTCLIENT it checks one region was created, deletion returned success,
GdiFlush completed, the exact handle is no longer a region, and the import
slots were restored. A built-in negative deliberately suppresses deletion,
verifies the region remains live, restores imports and deletes the test region.
Both widths detected the simulated leak. The test warms eight repeated
exception paths and verifies the process-wide GDI count does not grow across
another eight; the first process total is still logged as diagnostic. Exact
region tracking and a net process-count check do not prove that every possible
GDI leak elsewhere is absent. The S47 fluctuation remains in TODO.

Three standalone short runs per width passed in approximately 0.97–1.12s each;
one skipped the negative. One x64 run showed diagnostic 20 -> 27 while exact
region cleanup and warmed repeat count 27 -> 27 passed. The original equality
assertion was therefore not treated as a reliable measure of this specific
region. Product source and original resources were unchanged.

Independent clean GCC/Ninja/ccache dual-width build and ABI checks passed.
The actual four-file package passed 16 focused cases at four jobs in 7.41s,
zero skips; installed system32 x86/x64 tests also passed their positive and
negative checks. Both publication locations match the tested hashes below.
Physical input, full T18 integration and the root cause of process GDI count
fluctuation remain open.

| File | SHA-256 |
| --- | --- |
| demo.exe | `03b8f202197abd59934422896937b1bfd0616eedc5f0e13d5334001340f20f17` |
| ntw16s32.dll | `aa93c39c70b9f1a12dddc0743e149dd644d97e5914468c6da639502e5c56a115` |
| ntw16s64.dll | `7efaca3061e6373cf26028b008a5f0cbdbd7b9335e74bad3ddc5173e249f1691` |
| run16s.exe | `b5b397421294c605f0ebdcf79ad06e75eeec60c414c6fa3c3960173381580866` |

T18 and M0 remain open.
