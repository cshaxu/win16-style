# Legacy PE import staging

Admitted as M0 T12 S1 by CURRENT. The actual x86 DoubleKiller starts normally
but the previous run16s staging produces an access violation at the unresolved
GetModuleHandleA import-name RVA, 0x9ECAE.

## Confirmed break and correction

A bounded native debugger compared newly created, owned bare/styled processes.
Bare startup loads all application imports and resolves its IAT before the
initial breakpoint. The failing styled startup loads only the core system and
appcompat modules: neither ntw16s nor the application's ordinary dependencies
load, and the original IAT entries remain name RVAs. Thus module existence and
successful staging were not proof of initialization.

The decisive variable is the missing IMAGE_DIRECTORY_ENTRY_IAT, not zero OFT
or repeated descriptors. DoubleKiller has both an empty IAT directory and zero
OriginalFirstThunk. Self-contained x86/x64 cases with an empty IAT directory
fail in the old implementation even when OriginalFirstThunk remains present.
Completing only the IAT directory makes both shapes initialize and attach.

Before moving descriptors, derive the minimum/maximum destination thunk extent
from the original FirstThunk arrays, including each null terminator. Bound
scanning to 1024 entries per descriptor, check every read and image-relative
extent, and fail malformed/unterminated arrays with error 193 before publishing
header changes. Existing nonempty IAT directories are preserved. The prior
empty metadata is saved in the existing private stage header and restored by
the unchanged bootstrap. No lookup array or target file is rewritten.

This keeps the established three components/four files, ABI3, startup transaction,
OS rundll32 cross-width helper and original suspension/rollback semantics.
No debug startup, remote-thread bootstrap, new export or in-image scratch is
included. Earlier snapshot, placement, ordering and remote-call experiments did
not solve the target; they have been removed. Claims about IAT already being
resolved in a freshly CREATE_SUSPENDED process were incorrect; the debugger
observed unresolved thunks until loader startup completed.

## Verification and limits

`launch-staging.exe --parent [absolute-package-directory]` uses actual package
DLLs. For each width it covers: existing external import relocation and native
read/overflow/unterminated failures; missing IAT with OFT; missing IAT without
OFT; malformed destination and unterminated destination arrays. Successful
children create an attached window, verify metadata restoration and exit 42.
Rejected children retain headers and original suspension. Baseline stager
builds with the new test fail at the missing-IAT case with 0xC0000005.
Test-Package includes both new architecture cases against the final four files.

Actual DoubleKiller is an additional local compatibility check, not a required
build/test dependency. Owned private-desktop instances verify bare/styled
windows, matching module path, actual title rendering at 96 DPI, move,
minimize/restore and close/exit. No duplicate-file scan, deletion or move action
is exercised. Arbitrary legacy/packed/managed executables are not certified.
Final package, publication and remote review evidence belongs to the T12 history
and CURRENT, not to the experimental logs.

The [Microsoft PE specification](https://learn.microsoft.com/en-us/windows/win32/debug/pe-format)
defines the import/IAT data structures. The startup omission and corrective
behavior above are local probe observations, not a documented universal loader
implementation guarantee.

## Creation-contract oracle correction

The first full matrix preserved a failure in the existing denied-handle numeric
assertion. The unchanged 0.11 package also reproduced it with a bare, unstyled
child (variant 0), so it was not introduced by missing-IAT completion. Three
bounded baseline/candidate comparisons yielded baseline 2/3 and candidate 3/3;
a further original-assertion focused run also failed. These failures remain in
the delivery evidence. The precise object occupying the failed numeric slot
was not captured, so its type is not claimed.

The fixture now creates uniquely named allowed/denied events, duplicates the
candidate before opening a reference, and uses
[CompareObjectHandles](https://learn.microsoft.com/en-us/windows/win32/api/handleapi/nf-handleapi-compareobjecthandles)
to test actual kernel-object identity. It still checks every original vector,
allowed event signaling and denied event non-signaling in the parent. The
reference and pinned handles are explicitly closed. This removes the invalid
assumption that a non-inherited numeric slot must stay unused, rather than
loosening the inheritance contract. Windows 10 is already the native test
baseline; no product API/export or operating-system setting changes. Both
baseline and candidate passed the corrected 64-vector focused matrix.
