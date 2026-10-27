# M0 T18 S34 — runtime-module VCL TEdit

The S33 real DelphiVCL4Python 1.0.7 x86/x64 TEdit controls were created by a
runtime DLL and were not attached. The previous role-11 binding required the
class owner to be the main EXE. Owned process probes observed actual forwarding
of WM_NCCREATE, WM_PAINT, WM_NCPAINT, text and selection messages to the
system-registered EDIT procedure; the caller belonged to the class-owner
module. A `probe` selection replaced by `X` produced `pXbe` in both widths.
The test-only import observers restored their slots and the instances closed.

The product now records an exact newly created TEdit as a passive candidate.
On its own same-thread WM_PAINT it requires a call to the registered system
EDIT procedure from the class-owner module. After the original paint returns,
it binds the existing public EDIT protocol and uses the existing renderer;
only successful installation reports attached. The role binder accepts the
verified module only for this path; the existing EXE and other control paths
retain their owner checks. The existing EDIT classifier now consults the
verified protocol instead of returning before that check. It does not identify
arbitrary controls by DLL name, scan existing windows, or add an export. A
failed witness leaves the original behavior; failed activation is attempted
once and cleans up owned state. Existing editing font policy is reused.

The new self-owned fixture uses a test DLL, not an extra product file. Its
synthetic delayed native initialization is a boundary model and does not
reproduce the actual Delphi creation order. It checks an impostor that answers
EDIT messages but does not forward WM_PAINT, positive forwarding, single and
multiline selection, readonly, actual line scrolling, removal during a parent
paint callback and destruction. Both widths passed at measured 96 DPI; the
same final test failed on both old release widths because the positive control
remained unattached. The clean candidate passed independent real DelphiVCL
samples on x86 and x64 at 144 DPI; both reported candidate DLL path, attached
TEdit and `pXbe`. The clean-candidate screenshots differ from the old sample
by 452 RGB pixels outside the top 26 rows and zero inside those rows on each
width. They do not prove every VCL version, physical input or all edit variants.

A separate 20-iteration synthetic create/remove study showed both candidate
widths initially rising to 52 GDI objects, then stabilizing and falling to 50;
an x64 unstyled control sequence rose from 9 to 13. These process-wide counts
do not distinguish lazy caches from retained objects and are not a proof of
zero leakage; the remaining investigation is recorded in TODO. Exact
reentrant removal and fixture cleanup passed. The first x86 sample observer
failed from its own decorated export and was repaired before the valid runs;
the first incremental compile also exposed and repaired a local Winsock name
collision. The first clean package passed 22 focused cases before a multiline
scroll assertion was added; the second clean package below is the final batch.

Clean x86/x64 GCC 16.1.0 / Ninja / ccache 4.13.6 build, original resource,
source, governance and ABI gates passed. `s34-clean2/snapshot.json` SHA-256
is `e22d9544e19fb363a8a99e23b76c656f2e0987466c3a7e4928d4ecc6da8d07dc`.
The actual same-batch package passed 22 selected cases, zero skips, in 34.43s;
101 standard cases were not selected. The two installed entry probes passed.
All eight published files match the tested SHA-256 values:

| File | SHA-256 |
| --- | --- |
| demo.exe (x64) | `9f7271b24912399667e02f71e7387f1aafb431a6031504b65cda455316d9a854` |
| ntw16s32.dll (x86) | `9a0741045749a3e11f03ba3cc976af8b096903493ba005c15313fd4fb0fd5f2d` |
| ntw16s64.dll (x64) | `99d957a9f326cad528efdfb3139a3e4ba375abc4a2e4dc68f1c08f9434c39786` |
| run16s.exe (x64) | `77cd1f54849deffa1f3aa94f2354cb7a184a647584f67461ca38499dc470fb6b` |

Production changes before staging: attachment.c +40/-8, control_class.h +1/-1,
control_protocol.c +7/-2, control_protocol.h +2/-0 (50 added, 11 removed,
net 39). The new authored test sources and corresponding import receipts are
included. Search used `rg -n -C 3 'TEdit|TTreeView|TListView|superclass'
src/ntw16s-dll` and `rg -n -C 2 'static_text|CallWindowProcW'
src/ntw16s-dll/attachment.c src/ntw16s-dll/injection/window_titles.c`; only
TEdit was admitted, other runtime-owned roles remain for separate evidence.
The old EXE-owned edit binding remains available, and the new candidate does
not broaden any other role. This S is locally verified; P1 push and remote
review remain pending. T18/M0 remain open.
