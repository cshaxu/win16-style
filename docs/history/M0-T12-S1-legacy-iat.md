# M0 T12 S1 — legacy missing-IAT launch compatibility

User request: fix `run16s "D:\programs\misc\DoubleKiller.exe"` failing to start.
Baseline: 0203e097ba5d105d175d701d5c6cc779f99c67b9 / 0.11.0.0. M0 remains open.
This S retains three components, four products, C17/GCC/Ninja/ccache and ABI3.

## Diagnosis and minimal implementation

DoubleKiller is a native x86 PE32 GUI application with 18 zero-OFT descriptors
and an empty IAT data directory. A bounded native debugger established that
the failing staged target reaches its initial breakpoint without its ordinary
dependencies or ntw16s loaded; its IAT still contains hint/name RVAs. Execution
then faults at 0x9ECAE, the GetModuleHandleA import-name RVA. Bare startup loads
the dependencies and resolves these slots before the initial breakpoint.

The decisive missing metadata is DIRECTORY_ENTRY_IAT. Independent x86/x64
fixtures fail with the old stager even when OFT remains nonzero. The final
product change adds a bounded FirstThunk destination-range calculation when
the IAT directory is absent, before relocating the descriptors. It includes
each terminator, preserves existing directories and restores original metadata
through the existing bootstrap. No target file or lookup array is rewritten.
Unterminated/out-of-image ranges return native error 193 before header changes;
the caller retains its original suspension and existing rollback contract.

Earlier lookup snapshots, image-padding placement, descriptor order, delayed
restore and remote-thread experiments did not fix the target and were removed.
The statements that the initial suspended IAT was already resolved, that the
failure required repeated DLL descriptors, or that a new injection mechanism
was necessary were not supported. No debug startup, new export, remote-thread
bootstrap, extra component or product is included. Production behavior changes
only in staging.c; the other production changes are task/version metadata.

## Native regression and creation-oracle correction

`launch-staging.exe --parent [absolute-package-directory]` now tests each width
with missing IAT and either retained or zero OFT. Successful children initialize
the actual DLL, create attached windows, verify restored directory metadata and
exit 42. Malformed extent and missing terminator cases return 193 with unchanged
headers and suspension. Existing external-import tests remain. The new tests
linked against HEAD's old stager detect the fault in both widths: test exit 1,
child 0xC0000005. Test-Package includes both final-package variants.

The first native full matrix recorded 65 passes, 3 skips and 1 failure in the
old numeric denied-handle assertion. Its original focused rerun also failed.
Three unchanged-baseline/candidate comparisons recorded baseline 2/3 and
candidate 3/3; the baseline failure occurred in a bare native child (variant 0).
Thus the failure is not introduced by this product change. The actual object
type occupying the failed slot was not captured and is not claimed.

The fixture now pins a candidate before opening a uniquely named Event
reference and uses CompareObjectHandles to check underlying object identity.
The original vectors, inheritance flags and parent event-signal assertions
remain; all reference/pinned handles are closed. Both old and new packages pass
the corrected 64-vector focused matrix. No product hook change masks this issue.
The original failed runs are retained in the summarized evidence, not relabeled
as passes. The unrelated earlier T11 rollback-count debt remains unchanged.

After this test-only correction, another clean dual-width build was made.
Read-only comparison of PE headers and all raw sections found 71 unchanged
loaded images and only the 2 intended contract-fixture changes. Only COFF and
export timestamps plus checksum are normalized; unmapped trailing COFF symbols
are excluded. No binary is edited. The first full-matrix evidence and corrected
final-package contract check are separate; this is not a claim of a second
all-green 69-test run with identical raw hashes.

## Actual target and delivery boundary

The rebuilt four-file package starts DoubleKiller through x64 run16s and the
existing x86 OS helper. The owned private-desktop target loads that package's
ntw16s32.dll, displays a 704×483 styled window at 96 DPI and yields 12,440 classic
caption pixels (bare window: 704×493, zero such pixels). Native PrintWindow
screenshots were inspected. Move, minimize, restore and close succeed; the wait
launch exits 0. The default asynchronous command also displays the styled
target; its launcher exit 0 is distinguished from a target exit observation.
The target file SHA remains unchanged. No duplicate scanning, deletion, moving,
physical mouse/keyboard or arbitrary-DPI compatibility is claimed.

Original resources and provenance remain byte-exact; renderer/title/font/menu/
scrollbar source is unchanged. No sibling repository is modified or required.
Raw logs/screenshots remain temporary; selected facts and SHA records are in
[evidence](../etc/evidence/legacy-iat-staging.json). Final four-file identity,
actual-package results and publication belong to [release](../etc/release.json).

Two abandoned earlier T12 debugger/log-reading workers were found still active;
their exact command lines, creation identities and task ownership were checked
before stopping only their process trees. Their unbounded multi-gigabyte logs
are not used as primary evidence. The corrected debugger has finite event/time
budgets and terminates only its freshly created target handle.

P1 delivery and subsequent actual remote review/cleanup are recorded below
after their checks complete; this history does not treat an experimental build
or a merely successful installation return as task completion.

## P1 verified local delivery

Final clean four-file package: 0.12.0.0 / M0 T12 S1, GCC 16.1.0, Ninja and
shared ccache. Actual package 23/23 passed in 260.06 seconds (4 slots), including
the two missing-IAT cases, original 64 creation vectors, all-width propagation,
nested native cmd, rollback, demo, fonts, menus, dialogs and messages. The 3
existing full-matrix clipboard/optional-backend skips remain explicit. The
initial full-matrix oracle failure is recorded above rather than erased.

Both destinations were backed up before replacing only the four product names.
All 8 published SHA-256 values equal the tested package. The exact default
command then passed using O:/winnt/system32/run16s.exe and actually loaded that
directory's ntw16s32.dll; native screenshot and lifecycle checks passed again.
Source/artifact identity and per-case results are in release.json. P1 is locally
verified and published; remote review and task closure are subsequent steps.

The published default command and bare control also passed when launched from
the repository with lpCurrentDirectory=NULL. The fix therefore does not rely
on forcing the application directory as its working directory.

## P2 remote review and closure

P1 507847c7ac264a1a37ea2e2d1155ad2c3c97e4e2 was pushed to main. A separate
review read the actual commit/diff and downloaded all 139 source inputs plus
4 products from GitHub raw at that commit. All SHA values match the tested
release, with existing Git LF checkout transforms explicitly recorded. All
8 publication hashes were rechecked after commit. P2 changes documentation
only; there is no replacement binary or untested product amendment.

The task audit covers the original launch failure, decisive missing-IAT cause,
minimal in-scope fix, both-width positive/old-negative/malformed tests, actual
DoubleKiller rendering/closure under final and published packages, ordinary
Win32 regression, corrected object-identity creation checks, original resources,
same-batch package, two publication destinations and actual remote contents.
The initial full-matrix failure and three skips remain explicit. Neither this
S nor the object-identity test correction claims to solve the T11 rollback debt
or certify arbitrary old programs.

Twelve registered t12-* scratch roots/files were removed after absolute-path,
reparse-point and active-process checks; the inventory is preserved in evidence.
That includes the earlier multi-gigabyte defective debugger logs. All 8 prior
publication copies were proven recoverable from the 0203e09 Git blobs before
removing task backups. The external acceptance PNG remains. Persistent Ninja
outputs and shared ccache remain; both architecture dry-runs report no work.
S1 and T12 are closed; M0 remains open. T8/T9/T11 and cancelled T10 are unchanged.
