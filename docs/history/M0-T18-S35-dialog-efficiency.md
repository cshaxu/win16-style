# M0 T18 S35 — dialog regression efficiency

The S33 full package passed x86/x64 dialog-scope in 24.86/24.46 seconds,
dialog-show in 115.27/115.04 seconds, and the four-mode two-thread launch
integration suites dialogs/messages in 85.28/92.46 seconds. A scratch-only
x86 styled, DPI-unaware dialog-show run retained all 15 cases and took 30.18
seconds; after its first two cases, many individual cases took approximately
two seconds. This is timing evidence, not a proven Windows internal cause.

Keeping a same-thread owned anchor window for the duration of each test batch
reduced that scratch run to 1.09 seconds without changing its modal owner,
close messages, EndDialog call or result. A separate baseline/anchor probe
showed the tested dialog active immediately before EndDialog and inactive
immediately after it for both sampled timer and initialization paths. The
final source does not call SetActiveWindow or otherwise force a focus switch.
The scope and show tests now additionally assert that the dialog is active at
EndDialog, preserve every existing assertion and destroy the anchor. The launch
target retains an anchor only during the dialogs/messages suites, on each
owned GUI thread; it retains the original API calls, NULL modal owners, posted
close message, return checks, process and thread cleanup. Product source,
renderer, graphics, ABI and system settings are unchanged.

The final clean x86/x64 GCC 16.1.0 / Ninja / ccache 4.13.6 build passed the
source, byte-exact resources, governance and ABI gates. Snapshot file SHA-256:
`a56d0b414450a129aa42a9c6ec5133fc782acb68c7305f6b97d0d6a7320a7b4d`.
The actual same-batch package passed 18 focused cases, zero skips, in 35.66s;
105 standard cases were not selected. Scope x86/x64 took 1.19/0.64s and show
4.19/3.36s during that batch. Both show tests retained four bare/styled and
DPI contexts, 15 cases each, original owner-draw/geometry/font checks and four
empty owned jobs. Separate full actual-candidate launch integration runs
passed dialogs in 3.31s and messages in 6.25s, each with both architectures,
bare/styled and two GUI threads; the candidate hashes matched before and after.
Installed x86 assets and x64 system32 show entry tests passed. A full package
matrix beyond these selected and separately run suites was not repeated; the
S33 full run remains separate evidence. Physical input was not tested.

All four newly rebuilt files were tested together then published to assets and
O:/winnt/system32, with all eight installed SHA-256 values matching:

| File | SHA-256 |
| --- | --- |
| demo.exe (x64) | `bba9eb2f8bb2b8a61b830e07dad80f3744c5fed15dadb1f7a537138cb22fdeec` |
| ntw16s32.dll (x86) | `6f5a4a4426c5f0a480f29836f3f79df1225a5ee38798159a5debafec423d3f53` |
| ntw16s64.dll (x64) | `c1d31244865bac55a571de79a33fd1d64aeae90828a8b78193294b7eeb64e640` |
| run16s.exe (x64) | `b3a28cfda4a68b427456d7f3379435ec37f0008b793a1e7e9a2f6ef051469492` |

Search used `rg -n -C 2 'ShowWindow|SetActiveWindow|DialogBox|for\('
tests/ntw16s-dll/dialog_scope.c tests/ntw16s-dll/dialog_show.c` and reviewed
`tests/integration/automatic_target.c` and `automatic_messages.h`. The latter
was not modified; its existing posted WM_CLOSE and API result checks remain.
The test-only diff before staging is +8/-0 in automatic_target.c, +5/-2 in
dialog_scope.c and +5/-0 in dialog_show.c: 18 added, 2 removed, net 16.
One early scratch experiment started its styled child with different hidden
startup semantics and failed visibility; it was discarded and is not counted
as a test pass. P1 `c3b1d09` was pushed; all 11 changed files including
the four assets were retrieved from GitHub and matched. P2 `ca0eea5` closed
S35 and admitted S36; its actual remote CURRENT was verified. T18/M0 and
the deferred task-level audit remain open.
