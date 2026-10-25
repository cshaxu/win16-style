# M0 T15 S3: dialog frames, owner-drawn bars and hidden controls

The owner approved automatic sequential S admission through T15 closure, without a new T.
M0 remains fixed. Baseline da2631f6b1a84f5b4e06eddd0c72c63dc8fd6bcb was clean before S3.
This P1 is locally verified and published; commit/push and actual remote review are pending.
T15 remains open for the standard/native framework controls listed in its proposal.

## Diagnosis and bounded changes

Actual elevated HD Tune Pro4.60 x86 loads the correct DLL and styles supported children,
but its main dialog was unattached. A ready posted-message probe plus creation-return trace
locates the first divergence after WM_INITDIALOG: frame layout rejects its two top-level
MFT_OWNERDRAW0x100 menu entries, returning ERROR_NOT_SUPPORTED50. Ordinary A/W property
sheets under COMCTL5/6 attach with the old package; no PropertySheet detour is added.
The private window-aware bar snapshot now keeps application measurement/drawing callbacks,
itemID/itemData, menu ownership and commands. It validates identity after callbacks and
before drawing; application destruction or data replacement retires that snapshot.
Application-owned graphics/fonts remain application-owned. Standard string bars and the
confirmed title buttons/text/role-font routines are retained, with exact source comparisons.

An independent dialog fixture proves that a dialog may become visible before WM_INITDIALOG
returns. The coordinator now completes only a pending, new, captioned top-level dialog at
its real first-show boundary. A settled/installed record prevents duplicate attachment after
explicit removal, but permits retry when an initially unsupported frame becomes supported.
Nested creation scopes retain the correct owner; failed creation and OOM release their nodes.
The initial duplicate-install and failed-retry prototypes were rejected by production-path tests.
No new detour, public export, ABI, component or product file is introduced.

A root-fixed actual screenshot then revealed three application-hidden property-sheet buttons
being displayed. This was present in the earlier root-unattached diagnostic too. Existing
native drawing suppression sent WM_SETREDRAW(TRUE), which sets WS_VISIBLE on hidden BUTTON
and multiline EDIT. A common visibility predicate now guards the four short suppression paths;
nested leaves share the active bracket. No font or drawing algorithm changes. An exact reverse
reconstruction of that predicate/four guards reproduces the original controls.c.

The old close probe incorrectly required exit0. Actual bare/styled HD Tune both close normally
with native exit2, destroyed windows and empty owned Jobs. Success now distinguishes the driver
exit0 from the application's legitimate exit2. No user service/process was used for experiments.
The initially proposed long-lived admin controller was rejected by automatic review and never
started. Only explicitly approved fixed one-shot probes, at most180seconds, were used.
System UAC cancellations1223 are recorded as unexecuted, never passes.

## Verification and publication

- Three new pure-C fixtures: dialog_show, dialog_scope, control_visibility. Both widths cover
  native A/W init/first-show, unaware/PMv2, actual caption pixels, explicit removal, nested scope,
  unsupported retry, callback destruction/data mutation, allocation failure and exact coordinator
  ownership0. Visibility covers16 parent/child combinations per width and four native protocols,
  application redraw control, text, detach and borrowed HFONT preservation.
- Old packages fail the early-show and owner-only menu fixtures in both widths. In the hidden
  regression, the prior candidate fails BUTTON and multiline EDIT; its COMBOBOX/LISTBOX controls
  already pass. Both fixed widths pass all16 combinations; no artificial failures are claimed.
- Frozen170 product inputs match the clean source. C17/GCC16.1, Ninja/ccache;41 actual-package
  cases pass in352.435seconds at4slots, no skips. Twelve related CTests pass;84 registered CTests
  were not all executed. Original five headers/provenance and18 embedded tables per DLL pass.
- Final approved HD Tune controller13488 owns bare39056 and styled27284 (launcher41620), x86,
  actual DPI96. Root attached1; DLL absolute path/SHA match the final batch. Three standard buttons
  stay hidden before and after minimize/restore/move. Native menu opens, is captured and cancels
  without a command. Both application exits2, owned Jobs empty; controller exits. No disk test runs.
- Final actual DoubleKiller four-page/Run-pixel and MadShow VB6 probes pass. Both deployed locations
  pass actual x86/x64 automatic launch and ABI3/public31 checks. All eight files match tested SHA.
  Fresh Restart Manager query finds0 owners before publication; a previously observed md5summer
  owner had released already. No pre-existing process was closed. Eight prior files were backed up.

Native acceptance PNGs are outside Git in the user's visualization folder and indexed by path/SHA
in the evidence. These are PrintWindow captures, not physical keyboard/mouse or screen-interaction
claims. Only native target96DPI is demonstrated for HD Tune; synthetic fixtures cover both widths
and awareness contexts. No x64 HD Tune, multi-monitor or arbitrary third-party owner-draw promise.
The original non-CP1252 font policy remains; existing application encoding/self-drawing is not
silently repaired. HD Tune still has7 unsupported icon/PushLike buttons,6 client-edge edits and
other common controls for subsequent S work. The inherited observer's failures13 names those
excluded controls; it is not the root/visibility acceptance predicate.

The actual root/menu screenshots and all case identities, failed experiments, hashes, batch,
publication records and installed checks are in [evidence](../etc/evidence/framework-completion.json).
P1 remote/coordinator review and S3 closure are still pending; T15/M0 must remain open.

Code scope: 11 product/build/test files, +456/-35, net+421; no moved/deleted paths. Per-file numstat, state owners and production-hit dispositions are in the evidence. Registered CTests: {'x86': 32, 'x64': 52}; not all executed.
