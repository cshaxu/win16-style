# M0 T15 S2 — DoubleKiller native superclass styling

User request: make DoubleKiller use this project's style. The user explicitly requires
continuing with S tasks, never allocating a new T autonomously. An uncommitted T16
allocation was withdrawn immediately; this delivery is T15 S2, version 0.15.0.0.
M0 and T15 remain open for appearance acceptance; T15 S1 and unrelated tasks stay unchanged.
Baseline is 8fc6ad0010d8dcfcd0ba335933c24e0b8bbebe97, initially clean.

## Diagnosis and implementation

The existing launch/IAT fix works: the actual x86 DoubleKiller loads the matching DLL
and its main frame is attached. Its VCL classes have registered names TButton,
TCheckBox, TEdit/TSpinEdit and TComboBox. USER identifies their native instance types
as Button, Edit and ComboBox. The registered-name classifier omitted them.
The shared private classifier now accepts only the existing renderer's six standard
USER protocols when RealGetWindowClass confirms the native type. It does not guess
from a T prefix, executable name, private VCL object layout or procedure bytes.
All three consumers use that classifier; list drawing also uses the same classification.
Existing owner-draw/nodata/unsupported subtype rejection remains. Superclass client-edge
edits retain native client geometry and edit semantics; only existing border rendering
is added. There are no new exports/hooks, DllMain changes, components or product files.

The first prototype missed early list theme capture (native create error 5023; the owned
Options probe could not finish). An early-child-subclass prototype then passed a 33-case
package matrix but still did not actually render the target controls: VCL replaces its
InitWndProc during WM_NCCREATE, bypassing those early subclasses. Thirty-three attachment
records were therefore insufficient evidence. The actual Run button retained f0f0f0
rather than the renderer's c0c0c0 fill, and temporary tracing saw only frame paints.
That prototype was never published. Its failed rendering and earlier successful matrix
are recorded separately instead of treating them as final acceptance.

The final implementation stages early theme requests in a same-thread private list,
without changing InitWndProc. After native WM_CREATE initialization it commits the node
to the normal subclass lifetime and installs the existing renderer. The original known
native LISTBOX/ComboLBox capture path stays intact. Cancellation, failed creation,
terminal teardown and unfinished scope cleanup retire staged records. No repair of a
foreign subclass chain, forced WNDPROC replacement, global hook or shared class change
is used. Original resources/provenance and title/menu/font/scrollbar/VB renderer sources
remain unchanged, with explicit source comparisons in evidence.

## Native verification

The self-contained C superclass fixture reproduces a framework resetting its initial
WNDPROC before invoking the native procedure. It covers seven superclass types, fake
TButton rejection, owner-draw button/list rejection, click/space notifications, 3-state
checks, edit replacement/undo, list/combo selection, dropdown opening/cancel, actual fill
and detach pixel changes, client geometry, borrowed HFONT retention and empty owned Jobs.
Both widths run bare/styled at actual 96 and 144 DPI. It has no DoubleKiller dependency.
The enhanced c0c0c0 assertion rejects the early-subclass prototype despite its successful
attachment checks; the original package is rejected in both widths for omitted attachment.

A second C fixture compiles the production theme implementation with test-only allocation
tracking. It checks no early WNDPROC change, actual theme-call routing, commit, latest
application theme restoration, cancellation, terminal destruction, allocation failure
and all owned allocations returning to zero. On x86, opaque WNDPROC tokens need not keep
pointer identity; restoration is verified by actual SendMessage/CallWindowProc behavior
using an application-owned callback, rather than an invalid token-equality assumption.

The optional real-app driver runs only new owned private-desktop instances. It visits
four pages, reads native control/registration/font/attachment state, captures normal-sized
native PrintWindow images, moves/minimizes/restores, closes and requires exit 0/Job empty.
After page initialization it observes 28 buttons (including checks), 3 edits and 2 combos,
all 33 attached. A separate Run-button render/removal check proves c0c0c0 before detach,
f0f0f0 afterward and 1,843 changed pixels. It removes only temporary test-instance style,
never clicks Run, scan, delete, move or registry-setting commands. Original target bytes
remain unchanged. Screenshots are actual native output, not generated artwork.

Only the actual x86 ANSI application is certified by this compatibility check. The core
fixture uses public C/native protocols on both widths; it is not a real x64 Delphi runtime.
VCL page/tab presentation, owner-draw checklist/listview contents, windowless labels/group
artwork, progress/animation and custom spin buttons retain application rendering. Physical
input, other Delphi versions/OCX, arbitrary complex text and multi-monitor DPI changes
are not claimed. The exact tested package/publication and remote checks are recorded below
once completed, and in [evidence](../etc/evidence/doublekiller-controls.json).

The final similar-issue sweep also unified the group-box presentation guard, the remaining
registered-name BUTTON comparison in controls.c, and added an aliased group detach-pixel case.
The #32770 dialog-class distinction stays unchanged; ComboLBox remains an explicit native special case.
This required a fresh final snapshot and package run; preceding matrices remain historical evidence.

## P1 local delivery

The final clean batch contains exactly four 0.15.0.0 products, ABI3/31. GCC16.1,
Ninja1.13.2 and ccache4.13.6 build 167 frozen inputs. Final actual-package 35/35
passes in 260.64 seconds (4 slots), no skips; 10 final focused cases also pass.
All 79 registered CTests were not run. The final batch is distinct from both
historical prototype packages. Original resource/provenance and embedded 18-table
gates pass; no runtime hook or public export is added.

After native Restart Manager reports zero owners, both destinations are backed up
and only the four product names replaced. All eight SHA values match the tested
batch. Actual default launches (without --wait) from assets and system32 each load
that directory's DLL, pass four-page/render/detach checks, close exit0 and leave an
empty Job. Installed x86/x64 ABI tests pass. Final real MadShow also passes.
Title top24 rows match the old screenshot exactly; client pixels change by29,509.
Current hashes, test cases, source manifest and installation evidence are in release.json
and this S's evidence. P1 is locally verified/published; remote review remains pending.

Code review scope: 11 source/test/build files, +504/-12 (net492); five private renderer
files account for +62/-12 (net50). No product paths retired, no unrelated work overwritten.
Both persistent Ninja builds now report no work; shared ccache remains in place.

## P2 actual remote review and closure

P1 330fed79c6b9e09627a7c516bd670f53d4de9f59 is pushed to main. Actual GitHub raw
retrieval checks 167 frozen inputs, four tested binaries and twelve changed documents
(183 files), matching both Git objects and tested SHA values. A coordinator review
reads the actual commit/diff, owner request and evidence, including the initial false
positive and strengthened oracle. All 167 local inputs and eight deployed hashes
remain unchanged. P2 changes only these closure records; product input/artifact identity
remains P1, version 0.15.0.0.

After evidence extraction, elevated process inspection finds no live scratch references.
Both registered roots and descendants pass exact containment/no-reparse checks before
removal: t15-doublekiller 433 files/143,179,993 bytes; t15-package 3,324 files/177,275,501
bytes. All eight pre-publication backups are proven recoverable from 8fc6ad0 Git assets.
No user process is terminated. Persistent Ninja/ccache and external native acceptance
PNGs remain. S2 is closed; T15 and M0 remain open awaiting user appearance review.
Unsupported custom presentation and physical/multimonitor boundaries remain explicit.
