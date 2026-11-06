# M0 T23 S1: native Notepad launch and restoration

P4 implementation, publication and remote content are verified. S1 remains
open for owner acceptance. P5 records the completed remote review.

## Confirmed causes and implementation

The previous P34 package launches the installed Notepad intermediary, which loads
ntw16s64.dll. A bounded native debugger following only its own run16s process tree
observed CreateProcessAsUserW in that intermediary, followed by CreateProcessInternalW
and a /SESSION: Notepad child. The production hooks covered CreateProcessA/W only.
A normal, non-debugger module snapshot also showed no style DLL in the final child.
The fixed standard `run16s notepad` path loads the candidate DLL in that child and
styles its root before the observation probe calls any style API. No Notepad-name
exception, package-name table, new export, late scanning or overlay was introduced.

Public process-hook now offers optional `process_hook_propagation_attach_as_user_w`
after ordinary registration, in the same caller-owned transaction. Product startup
and late installation both opt in. It passes the original token, arguments,
environment and security attributes through and reuses existing suspension,
installation, reentry, error and rollback handling. It does not acquire privileges
or tokens. The existing product late-batch capacity remains88. An initial attempted increase
to89 was rejected by the shared limit and caught by actual-package regression
(error87); that unnecessary change was reverted before publication.
CreateProcessAsUserA, WithToken and WithLogon are not newly supported by this change.

The restoration defect was independent: frame_lifecycle read set-only
DWMWA_NCRENDERING_POLICY. Native queries returned E_INVALIDARG (0x80070057), so
policy_saved stayed false; removal left DWM rendering disabled. The fix reads
DWMWA_NCRENDERING_ENABLED and restores its effective enabled/disabled state. The
original USEWINDOWSTYLE versus explicit ENABLED policy is not publicly queryable;
we do not claim to recover that hidden distinction or arbitrary app DWM mutations.
See [Microsoft's attribute contract](https://learn.microsoft.com/en-us/windows/win32/api/dwmapi/ne-dwmapi-dwmwindowattribute).
No original artwork, font policy or client XAML renderer changed.

## Native verification so far

- GCC16.1 x86/x64 incremental compilation and linkage passed. Initial frames test
  linkage failed for missing dwmapi; the test target now declares that dependency.
- `core-frames` passes enabled and disabled DWM initial states, two apply/remove
  cycles, client geometry and existing native window forms. The same new executable
  against the old deployed DLL fails at the restored-state assertion on both widths.
- `core-process-hook-propagate` passes actual AsUserW children, original suspended
  creation, disabled/rejected policy and installation failure193 with terminated
  uncommitted child. Existing A/W, grandchildren and reentry checks still pass.
  A negative executable omitting only the optional AsUserW registration exits13
  at the first AsUserW payload assertion; it does not falsely pass.
- Normal standard-launch candidate: final PID12040, parent18496, installed
  Notepad11.2607.14.0 x64 /SESSION process; candidate ntw16s64.dll present, WinUI
  modules present. Root already shows classic frame before test attachment;
  after WindowRemove, DWM enabled=1 and native caption buttons visibly return.
- Target-window PrintWindow captures show preserved tabs, menus and content.
  Earlier screen BitBlt attempts were obscured by another application and are NOT
  visual passes. PrintWindow evidence is identified separately from screen capture.
- Two successive standard launches retained the styled window (PID10004); both
  the visible owner and intermediary24780 loaded the candidate DLL. UIA tab
  SelectionItem.Select returned success and selected=true.
- Native UIA File/Edit/View Expand/Collapse all succeeded; bounded native messages
  and window APIs verified move/resize, maximize/minimize/restore at144DPI.
  This is native automation, not physical keyboard/mouse acceptance.
- No pre-existing Notepad was operated on. Test-owned scratch recovery text was
  not edited by these tests. Normal close was attempted; owned unsaved instances
  were terminated only after identity/path/start-time validation and bounded wait.

Scratch: build/t23-package/s1-notepad-restore1. Existing failed P3 restoration
captures remain documented; no historical failure is overwritten as a pass.

## Coverage limits

This S covers this installed x64 Notepad and self-contained x86/x64 regressions,
not every WinUI application. Task Manager, Explorer, Terminal/cmd/conhost remain
later S work. Customer document editing, IME, arbitrary language/DPI combinations,
reuse of an unrelated previously unstyled process and physical DLL unloading are
not claimed. WindowRemove removes styling; it does not unload a live DLL or stop
ordinary process propagation. No registry, security policy, installed app files,
or shared-host settings were modified.

## Development failures retained

The first clean package compiled and passed binary/resource/ABI gates, but its
actual-package late-process case failed (install87) because the product requested
89 slots from the existing maximum88 engine. It was not published. The final
candidate retains the original88 capacity; no engine-limit workaround is added.
The first manual CTest invocation also omitted WIN16STYLE_CAPTURE_DIR and stopped
in the harness before executing tests; with the owned directory configured, all
five focused x86 tests passed. The portable-test CMake declaration additionally
links advapi32 under MSVC; MSVC execution is not claimed by this S. That conditional
has no effect on the MinGW product build.

## Review scope

Production search: `rg "DWMWA_NCRENDERING_POLICY" src/lib-win32/win16-window src/ntw16s-dll`.
The remaining two occurrences are setters in frame_layout.c (disable and restore),
not invalid queries. Startup and late propagation share create_process.c; neither
retains an A/W-only parallel product registration path. The new optional public
function uses the same reentry scope and finish/rollback implementation. No DLL
export or product component was added. Ownership of hook transactions, child
handles, frame state and DWM restoration remains with the existing callers.

Code/test/build diff scope (excluding documentation and binaries): 98 added,
20 removed lines including version metadata and product README text; renderer
change is six added/two removed lines. Existing notices and graphic resources
are preserved. Independent QUEUE/xaml-control-style proposal changes are retained
in the worktree and excluded from this delivery commit.

## P4 final delivery

- `Build-Package.ps1 -Worker -Task 23 -RunName s1-clean3`: clean x86/x64 Ninja
  build with GCC16.1 and ccache4.13.6 passed, ABI/resource/foreign-dependency gates
  passed. x86 build296.26s, x64 build284.63s; x86 cache663 direct hits/zero misses.
  Product version0.23.0.0, ABI3 unchanged, four files only.
- Core CTest `^core/(frames|process-hook-propagate|process-hook-late-batch|uwp-frame|uwp-frame_layout)$`
  passed5/5 on each width. Actual package `s1-package3` passed25/25, zero skips:
  mandatory eight gates, local global hook x86/x64, ordinary propagation entry
  x86/x64, late W four-width combinations, propagation-only cross-width both ways,
  global-window x86/x64, initial-menu x86/x64, system-menu visual x86/x64, nested cmd.
  This is focused regression, not the entire historical integration inventory.
- Additional `s1-package2` global-e2e was NOT a pass: private desktop lacks the
  Explorer tray; Shell_NotifyIcon failed0x80004005 before manager activation.
  Local global hook cases use actual product DLLs and passed child/grandchild
  propagation and window attachment on both widths. Actual desktop tray/UAC was
  not retested by S1 and is not claimed as covered.
- Final real candidate Notepad33084,parent35872 loaded the exact x64 DLL below.
  `attached_before_probe=1 native_restored=1`, apply/remove and three PrintWindow
  captures passed. Tabs, File/Edit/View, move/resize/maximize/minimize/restore
  passed at144DPI. Client screen rectangle remained899,241,836,718 across removal.
  Restored screenshot shows native caption buttons, styled screenshot shows the
  original renderer and preserved app tabs/menus. No manual DWM repair was used.
- An initial final interaction check read Edit/View immediately after asynchronous
  UIA Expand and saw collapsed states. The scratch probe now waits for explicit
  Expanded/Collapsed states for at most1500ms; all three then passed. Product
  binaries unchanged. Earlier failed logs are retained, not rewritten as passes.
- Published all four files to assets root and O:/winnt with Publish-Package;
  both destinations match the tested hashes. Two old mapped DLLs were renamed
  to registered scratch mapped-backup without terminating their occupants.
  Running old processes retain old mappings; this is not physical unloading.
  Retired files stay until their mappings are released.
- Post-publication `O:/winnt/run16s.exe notepad` returned0; final Notepad26120,
  parent32592 loaded O:/winnt/ntw16s64.dll with the tested SHA. All three menus,
  tab selection and window-state checks passed again; only self-created instances
  were cleaned up. Existing unrelated Notepad instances are outside this test.

| Product | PE | SHA-256 |
| --- | --- | --- |
| run16s.exe | x64 | `9f2ce144e9a7d5be8c05c24eb8b5d2c3b76790adf24d8e216b96228f814168f6` |
| ntw16s32.dll | x86 | `ab1bb89dcef5f0ce4a687eee337c990d1bd15f6c77068261859018c59e5bd2dd` |
| ntw16s64.dll | x64 | `6e6101e6343c8cd705fd80e6f7a0f1b6fa7e0fdd75a63aa609fa02de9a530041` |
| demo.exe | x64 | `d7b6b9ef2ebf72403bae081e2ceff300c373f49bdc11c3ca6988af5498bf24a1` |

Source snapshot SHA-256: `6fec096678b50c211d54fd488231618d52342d368885b6cde8fd9a35c64bb4c9`.

Retained task evidence: s1-notepad-restore1 (native sources/logs/screenshots and mapped backup), s1-clean3 (source/build), s1-package3 (25 case results), s1-publish1 (eight deployment digests). Earlier failure scratch remains for this acceptance cycle; S1 is not closed.

## P5 remote review and acceptance handoff

P4 commit `37d534bece92f7b5455a0d8168670f7936bfa608` was pushed to origin/main.
Fetched remote main equals the local delivery commit. Ten files were downloaded
from raw.githubusercontent.com at that immutable commit: four product binaries,
three implementation files, two regression sources and this delivery record.
Every downloaded byte matched its committed object; product SHA values match the
same tested/deployed table above. Governance self-test13 negatives, source checks,
resource identity and diff whitespace checks passed before commit.

P5 only records verification/status; four products are unchanged. Retained
QUEUE and xaml-control-style proposal work is uncommitted and outside P4/P5.
M0 and T23 remain open; S1 is verified, awaiting owner acceptance. No S2 admission
or all-app compatibility claim is made by this delivery.
