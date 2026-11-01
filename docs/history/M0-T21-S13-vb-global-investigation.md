# M0 T21 S13: VB6 global-mode investigation (in progress)

Using the S12 candidate and an owned private-desktop run, the x86 `O:\winnt\madshow.exe` main window was observed before global activation, after the initial sweep and after stop. The global controller reported no failed notifications; matching `ntw16s32.dll` was observed in both a pre-existing instance (PID 12188) and a new instance (PID 7892). The ordinary-mode comparison used a separately owned instance. These are local optional observations, not a build dependency.

The first global screenshot was captured as soon as the title changed, before the sweep finished, and showed native-looking result-field borders. A second capture 500 ms after that point showed classic borders in both old and new instances. Thus that early screenshot is not evidence of a persistent border defect. Stable global captures still show different windowless label typography from ordinary mode. The existing ordinary path wraps text APIs in `injection/window_titles.c` under a verified VB paint scope; global mode does not install the ordinary startup hook transaction. This is a plausible mechanism, not yet a demonstrated complete causal chain or an approved unsafe retrofit of startup hooks into a running process.

The existing x86 VB observer could not install its thread hook: `SetWindowsHookExW` returned 126 in both ordinary and unstyled private instances. Consequently no claim about attachment counts or input behavior is made from that observer. A separate ordinary capture was obtained without the observer, but its driver later reported error 5 during cleanup, so its cleanup result is not counted as a pass. The job-backed isolation closed only the test's processes. Raw captures and scratch probes remain in `build/t21-s13`; full comparison, a safe approach and final actual-package verification remain outstanding.

Further read-only in-process-image byte observations compared three owned x86 MadShow instances. At `DrawTextA`, `DrawTextExA`, `TextOutA`, `ExtTextOutA`, and `TextOutW`, ordinary startup showed a leading `e9` detour at all five entries. In the bare and globally attached instances the eight bytes at each corresponding entry were identical to one another; for example `DrawTextA` began `8bff558bec83ec18` in both. The global controller reported zero notification failures and the global instance had the candidate 32-bit DLL loaded. This distinguishes a loaded and visibly attached renderer from startup text-hook initialization.

For a bounded negative control, a **scratch-only, mixed diagnostic package**, never released, was linked with the S12 ordinary renderer except that six VB text hook registrations were omitted. Its actual ordinary-launched MadShow screenshot shows the same native-looking labels and edit text as global mode, whereas the complete ordinary package shows the bitmap font. This strongly supports the text-hook difference as the cause of this sample's typography; it does not identify which single API is necessary, prove all controls are attached, or cover other frameworks. Both screenshot-only ordinary drivers returned cleanup error 5 after writing their captures, so neither driver run is counted as a complete lifecycle pass. `MSVBVM60.DLL` imports `DrawTextA`, `DrawTextExA`, `TextOutA` and `ExtTextOutA`; these import entries are a possible narrower research direction, not a validated product solution.

The startup transaction is explicitly limited to a fresh process: it modifies executable prologues without suspending concurrent threads and can terminate an uncommitted process after failed rollback. It cannot safely be reused unchanged in arbitrary already-running applications. No binary fix has been adopted or published; safe coexistence, loading, thread safety, teardown and interaction with ordinary-mode ownership still require proof.

An additional delivery-path failure was found during this investigation. On a private desktop, the exact S12 candidate from its original build directory passes `global-common`, but the same four published files at `O:\winnt` fail to start global mode with error 126; the same error occurs for copies under the task scratch directory and the user's temporary directory. A diagnostic launcher on the scratch copy identifies the failure specifically at the 64-bit `SetWindowsHookExW`: its callback address belongs to the successfully loaded DLL and the resolved path is the requested DLL, yet the call returns 126. The matching candidate directory still passes. Byte hashes match for the compared x64 DLLs. The cause is not established, and this is not evidence that all published-path executions fail on every desktop. The previous actual-package tests did not adequately cover the published path. No user process was altered; S13 remains open and this startup failure takes priority over the experimental VB import-table approach.

Subsequent private-desktop observations narrow the 126 failure. A single process alternated the byte-identical S12 candidate and deployed x64 DLL: candidate succeeded, deployed failed 126, candidate succeeded again; changing the process current directory did not change the result. The exact `GetModuleFileNameW` function imported by this machine's USER32 returned both complete paths successfully. A private diagnostic call to the underlying native hook function with the same already-loaded module and callback accepted the previously tested candidate path but returned 126 for the deployed path, a different existing path, a system DLL name and a nonexistent name. This diagnostic does not constitute a supported product API or fix.

Ten previously exercised final candidate paths from S3 through S12 could each register their own x64 hook, whereas multiple earlier unused/intermediate and new paths failed with 126. This strongly resembles exhaustion of the shared hook-DLL registration table described in an [archived Microsoft engineering explanation](https://www.sysadmins.lv/retired-msft-blogs/alejacma/setwindowshookex-fails-with-error-errormodnotfound.aspx); its stated 32-entry limit applies to the Windows versions discussed there and is not assumed to describe the present Windows version. Thus a deterministic defect in our published path is **not established**: accumulated distinct test package paths and Windows session state are a serious competing explanation. Project tests must cover deployed paths and avoid needless distinct hook paths. We did not close, restart or inject unrelated processes. A read-only module inventory found no project DLL in processes it could inspect; this does not prove that the system's registration table is empty. Creating a separate noninteractive window station for an independent control failed with access denied (5). No session restart or security change was attempted.

An exploratory x86-only build replaced four already-resolved imports in the **owned** `MSVBVM60.DLL` instance with prototype wrappers that borrow the existing per-window paint-scope font. This prototype changes the runtime's import table, not the system API prologues. It is not production code: the preliminary implementation does not yet prove safe PE bounds, concurrent installation and removal, runtime unload, nested calls, last-error preservation, conflicting hooks, normal-mode coexistence or x64 behavior. In a private desktop the existing and new MadShow instances visibly used the bitmap font for their windowless labels. The API prologue observer still read the original system bytes, the manager reported zero sweep failures, and both owned instances remained alive through global stop. After an explicit repaint, the frame and labels looked native again; some result-field text remained visually different, also observed in the S12 global baseline, so complete pixel restoration is not claimed.

Because this session could not register any new DLL hook path, the prototype was exercised temporarily at the already-registered S12 **scratch candidate** path. The exact prior 32-bit DLL was backed up, restored in a `finally` block, and its SHA-256 rechecked as `4099bc49348ea31eef7a3346d1c9a94f51eb48aa2a16567ab2356e07668b8848`. The `assets` and `O:\winnt` files were never changed. Raw prototype captures and code remain in `build/t21-s13`; no prototype has been adopted into product or represented as a completed fix.

The exploratory driver subsequently sent two `BM_CLICK` messages to the actual `&Grid` button in each owned MadShow instance, receiving four bounded message completions; global stop completed and both processes remained alive through the stop. This is message delivery evidence, not physical-input or a full semantic verification of the simulation. The initial driver attempt incorrectly searched for `Grid` without the mnemonic ampersand and failed; enumerating the real button caption identified the test bug, and the corrected run passed. An explicit full-window redraw after stop still showed some residual value text, so the accepted visual-residue limitation remains. The S12 scratch DLL was again restored and hash-checked after each run.

One additional owned mixed-order prototype run started the global session, then launched a second MadShow via ordinary `run16s --wait`. Its launcher PID was 7600 and its actual GUI target PID was 19776; the target loaded the experimental x86 DLL. Both `&Grid` controls accepted the bounded paired click messages. After global stop, the ordinary target's screenshot still shows the classic frame and bitmap label font. This is one x86 visual/message sample for this order only; it does not prove the opposite order, callback reentrancy, propagation, or safety of the experimental import patch. Again the scratch candidate's original 32-bit DLL was restored and its hash checked.

The reverse sequence was then sampled once: an owned ordinary `run16s --wait` launcher (PID 6828, GUI target PID 19748) was running before the global controller started. After the global stop its capture retained the classic title and bitmap label text, and both instances received paired `&Grid` click messages. This adds a visual/message sample in each order; it does not by itself prove reentrancy, correct hook chaining, propagation or safe unloading. The previous paragraph's stated opposite-order limitation applies to that earlier single run. Both runs used the exploratory DLL, not a release candidate.

Read-only enumeration subsequently found three lingering `Win16S-Dialog-*` desktops, each containing a `CabinetWClass` window owned by a different `explorer.exe` process (PIDs 24036, 34008 and 13612; parent PID 1664; COM `/factory,... -Embedding` commands). Their creation times and titles are consistent with the S9 exploration, but their direct ownership by that test is not established. They did not have the project DLL loaded in the readable module inventory. The S9 observer stopped waiting when the directly spawned Explorer process exited; an indirectly activated Explorer window can appear afterwards. Its earlier observation was therefore limited to the period before that exit and is not evidence that no later Explorer window was created. These existing system processes and windows were not messaged, injected, closed or terminated.

The shared private-desktop driver previously closed its own desktop handle without checking whether another process retained the desktop. It now checks for release for at most one second after terminating its owned job; an observable retained desktop is reported as a failure, without closing or terminating the holder. Incremental dual-width `global-common` and ordinary/global `global-e2e` runs passed with the check. Its `--self-test` keeps a second handle to an owned desktop: the check detected the retained desktop, and after closing the second handle the object was gone. The `desktop-cleanup` package case registers this negative control. A focused incremental run against the unchanged S12 package and the newly built driver passed all 10 selected cases in 26.3 seconds with no skips; the historical driver was restored and hash-checked afterwards. This is not an S13 clean build or delivery. The check exercises a real kernel desktop lifetime, not whether a process belongs to the test job, and does not exclude every delayed COM activation race; it does not resolve the pre-existing Explorer instances or prove the hook table is full.

Follow-up read-only inventory found one active interactive user session and no available separate Windows VM tool in the usual locations. Creating a new noninteractive window station also returned access denied (5) when requesting reduced permissions. A module inventory reported no visible project test DLLs among 311 process module listings; protected or otherwise incomplete listings remain a limitation, so this does not identify the holder of any hook-table entries. No logoff, restart, security change or termination of the Explorer COM servers was attempted.

The scratch VB prototype was tightened to install only after its global callback has successfully pinned its own DLL and while the session is active; its text wrappers now use a nesting count and preserve the original API's returned last-error value. Both owned MadShow mixed-order screenshot/message checks passed again and the S12 scratch candidate was restored and hash-checked. These changes only remove identified prototype flaws: PE validation, runtime lifetime, concurrent IAT ownership, actual nested-call and last-error fault tests, and the delivery-path 126 limitation remain unresolved. No production source was changed for the VB approach.

The next step is a separately testable, **read-only** in-process import-slot scanner in `src/ntw16s-dll/import_scan.c`; it does not patch an IAT and is not yet called by global mode. It checks PE architecture and import-directory bounds, verifies each read belongs to the same committed allocation with `VirtualQuery` and `ReadProcessMemory`, bounds names and iteration, refuses duplicate matches, and publishes no slots on a malformed scan. A self-contained native fixture tests malformed names, slots, descriptors, duplicate entries, ordinal imports and decommitted pages alongside the current executable's real imports. Manually compiled x86 and x64 GCC C17 runs each passed ten synthetic checks and a real executable import; an optional x86 observation resolved all four expected imports in this machine's actual `MSVBVM60.DLL` in 15 ms. The runtime is not a required test dependency. The scanner and test are registered with CMake and the package case list, but have not had a clean package build or been integrated into the styling path. Concurrency, actual patch semantics, chaining, teardown and published-path verification remain outstanding.

Follow-up fixtures added a failed multi-query check (no partial slots escape), invalid DOS headers, and two adjacent committed allocations whose second allocation falsely falls within the claimed image size. The scanner rejects that cross-allocation slot. Current x86 and x64 CMake/Ninja/CTest runs each pass 13 synthetic checks and one current-image import, with an additional optional x86 runtime check; the self-contained CTest case took about 0.3 seconds per architecture. The first attempt omitted the repository's required `WIN16STYLE_CAPTURE_DIR` and failed in the test wrapper; it was rerun with owned scratch directories and passed. No actual package or deployment is implied.

Direct `VirtualQuery` measurements show all four observed VB6 import slots on this machine have `PAGE_EXECUTE_READ` protection. The earlier scratch prototype's temporary `PAGE_READWRITE` therefore removed execute permission from a page that may also contain running code; it must not be shipped. A separate guarded updater in `src/ntw16s-dll/import_patch.c` now preserves executable access during each short write, restores the prior protection, uses compare-and-exchange, refuses pre-existing foreign pointers, serializes its own competing updates and attempts to roll back its own changes on later failure. It is compiled into the DLL but is not called by global mode. Both CMake/CTest architectures passed self-owned guarded-page, foreign-hook, two-thread and two injected mid-transaction failure cases. A test-only probe executes machine code from the same page during the write window; a scratch negative variant that switched the page to non-executable write access failed this check as expected. These tests do not prove behavior if another independently written patcher concurrently changes page protection, if protection restoration itself fails, or if a runtime unloads during application; lifetime and integration remain outstanding.

Subsequently, an injected one-time protection-restoration failure was added to the x86/x64 updater tests: the changed pointer and original executable protection were restored. Repeated or externally induced protection failures remain a limitation. The updater was then integrated into the existing global attachment path for verified VB6 `ThunderRT6FormDC` windows only, after the style DLL is pinned and the window has attached. It retains the runtime while resolving and updating its four imports, skips a conflicting foreign pointer, and uses existing per-window paint scopes; it does not alter system API prologues. Global stop removes the window scopes, leaving the runtime import wrappers passive until that runtime unloads. This does not extend the contract to VB5, other runtimes or application-owned drawing.

The self-contained fake-runtime test passed on x86/x64: four imports, actual scoped pixel changes, unchanged unscoped pixels and returned errors, pre-existing foreign-pointer rejection, idempotence and runtime unload/reload. A private-desktop global end-to-end fixture additionally verified real changed pixels in two pre-existing and two new targets, both widths, and original pixels after stop. The original S12 DLL failed that new test as expected: its existing window was attached but its pixel hash stayed equal to the native hash. Preliminary main-worktree builds also passed the ordinary/global x86/x64 coexistence probe and both startup orders with owned MadShow windows; one reverse-order run reported one timed-out or failed initial window notification, so it is not counted as a zero-failure sweep. The current work is not yet a clean-package delivery or deployed fix.


## Clean package and staged delivery

The C17 GCC/Ninja/ccache clean dual-width build completed successfully. The clean candidate passed 15 focused cases at its own path and 23 focused cases, including mixed ordinary/global ownership, actual scoped VB fixture pixels, stop/restoration, import safety and both system menus, at the previously registered S12 scratch path. All four candidate files were substituted together, hash checked, and the original scratch files restored and verified. The old S12 DLL was detected by the new VB pixel test as a negative control. This is focused coverage, not a complete compatibility matrix.

The tested four files were published to `assets` and `O:\\winnt`, with matching SHA-256 values: demo.exe `c5429c5e8e4a9df768fb4179b7c656186fd9e9dcea96ad94dce842b920ea14bf`; ntw16s32.dll `ba0a77f01dc7f6e4d28c7c81d074e042fa5b5b1e4b25f7a993cf4842c7a3711c`; ntw16s64.dll `3a4c9c8aa3f4cdb22037b7421b2adf40e78d234cb3c8ed632fb2ddf9abdecca2`; run16s.exe `47562fb91e78e1f2b590f6d1cf2f65e52c4c50d8a25be5341c5ac5c933851a7e`. The deployed-path private-desktop probe still reports `global startup or shutdown failed (126)` and exits unsuccessfully. This is a real current-session delivery limitation; the table-exhaustion explanation remains unproven, and no claim of successful deployed global startup is made. S13 and T21 stay open pending resolution and deployed-path verification.


## Token and equivalent-path check

The current probe process was checked with `whoami /groups`: it has Medium Mandatory Level, and the Administrators SID is deny-only. Tool sandbox escalation did not create a Windows elevated token. Therefore the existing 126 results are non-elevated observations; an elevated comparison has not been performed and cannot be claimed. On an owned private desktop, one process loaded the published x64 DLL successfully from `O:\winnt\ntw16s64.dll`; the ordinary path, a `.` component and a case variation each returned 126 from hook registration. A previously registered S12 scratch path succeeded in the same process. The attempted extended-path argument was mangled by the test driver's command-line quoting and gives no evidence; it is excluded. This is not a same-binary comparison of the two paths in this particular run (the separately documented earlier comparison was). It excludes simple dot-component and case spelling as remedies in this session, but does not establish the cause or elevated behavior. No user process or system setting was modified.


## Elevated comparison

A user-approved elevated probe ran on a private desktop and reported High Mandatory Level with the Administrators group enabled. In the same process, loading the deployed `O:\winnt\ntw16s64.dll` succeeded but `SetWindowsHookExW` still returned 126; the previously registered S12 scratch candidate path successfully installed and uninstalled a hook. The private desktop driver exited zero. This establishes that simply running the probe as administrator does not resolve the current session failure. The two DLLs differ in this particular run; the previously recorded same-byte comparison remains the evidence separating path/session effects from binary contents. No existing user process or global setting was changed. The cause and a working deployed-path fix remain unproven.


## Elevated separate-window-station comparison

A second short, user-approved High Mandatory Level probe successfully created its own noninteractive window station and desktop, ran one owned child there, waited for its exit, and closed both objects. In this separate station the deployed `O:\winnt\ntw16s64.dll` again loaded successfully but hook registration returned 126; the previously registered S12 candidate path succeeded in the same child, which exited zero. The earlier medium-token station creation failed with error 5; this elevated run shows that failure did not rule out the test. The result excludes an explanation limited to the original desktop and is consistent with a broader session-level registration constraint, but does not establish its precise cause or capacity. Read-only `tasklist /m` found no matching project DLL among visible modules, which is not proof that every protected process or kernel registration is clear. No existing process was closed or injected.


A separate self-owned private-desktop native probe loaded the deployed x64 DLL and attempted `WH_CALLWNDPROC` first for its own thread, then globally: both returned 126. The same probe using the previously registered S12 candidate path succeeded in both modes and unhooked immediately. These are separate owned process runs with different DLL builds, not a substitute for the earlier same-byte path comparison. The observation excludes simply switching to per-thread hooks as a remedy in this session; it does not identify the underlying resource or establish results for other hook types.


An elevated, read-only PowerShell `Get-Process` module enumeration visited 362 process objects and reported no `ntw16s32.dll` or `ntw16s64.dll` matches and no thrown access exceptions. Empty or incomplete module lists, protected processes, transient exits and alternate module names are not ruled out; this is not proof that all processes or kernel hook-registration slots are clear. It did not modify, inject, or terminate any process.


The same isolated own-thread/global registration probe was compiled for x86: deployed `O:\winnt\ntw16s32.dll` returned 126 for both scopes, while the previously registered x86 S12 scratch DLL succeeded for both. Each owned test exited and unhooked normally. This confirms the current symptom affects both product bitnesses, without establishing whether their underlying capacity is shared.


## Owned-process module lifetime observation

An isolated probe extended the existing S12 `global-common` targets with a `GetModuleHandleW` query. The manager started and stopped successfully with zero sweep failures; all four existing/new x86/x64 windows and controls passed the pre-existing restore checks, while each target reported its matching style DLL still loaded after global stop. All four owned targets were then closed by the existing driver and the private desktop was released. This matches the explicit `GET_MODULE_HANDLE_EX_FLAG_PIN` in `global_hook.c`, which pins the module for the life of each process. The first scratch fixture omitted the common-controls v6 manifest and failed before global activation; it was rebuilt with the existing manifest resources before the successful run. This demonstrates retained modules in affected processes, not that they caused the present 126 or that any unrelated live process contains them. Removing the pin without a proven safe replacement could leave callbacks or VB import wrappers pointing into an unloaded DLL; it is not treated as an approved fix.


## Native module-enumeration coverage correction

The earlier PowerShell module count did not distinguish empty/inaccessible module lists, so its `inaccessible=0` is not reliable. A native x64 `CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32)` probe with a medium-integrity token examined 360 process IDs: 201 module snapshots succeeded, 159 failed with access denied, and no `ntw16s32.dll` or `ntw16s64.dll` match was found in the successful snapshots. Process exits and protected or differently named modules remain outside this evidence. An elevated read-only version was requested separately; its outcome must not be inferred while UAC is pending.


The administrator-approved native snapshot then completed with High Mandatory Level: 363 process IDs, 333 successful module snapshots, 20 access-denied and 10 other failures, zero matching named project modules in successful snapshots. This improves coverage but does not inspect the 30 failures, other sessions, differently named modules or kernel registration state, and cannot establish the cause of 126. No process was modified.


## Registered path-name probe

A private-desktop diagnostic repeated the previously documented unsupported native hook call with the **same loaded deployed x64 DLL and callback** for every attempt. The deployed x64 path, `user32.dll`, a nonexistent path and the system USER32 path returned 126; substituting the already exercised S12 **x86 DLL path as a name** produced a hook handle which was immediately unhooked. No target window was messaged while this diagnostic hook existed. This is evidence that, at this particular registration stage, the supplied registered name changes the outcome independently of the already loaded callback image and even when that name refers to the other bitness. It does not establish why the names are retained, a capacity, or safe module loading in a target process. The unsupported call and mismatched name are diagnostic only and are not a product workaround.


## In-context WinEvent alternative

An owned private-desktop x64 prototype used a new C DLL with `SetWinEventHook(EVENT_OBJECT_SHOW, ..., WINEVENT_INCONTEXT | WINEVENT_SKIPOWNPROCESS)` and a separately created target. With the DLL at its new scratch path registration failed 126 before a target was created. As a positive control, the exact same DLL bytes were temporarily substituted for the x64 DLL at the already registered S12 scratch candidate path; registration succeeded and an event signaled from the owned target process proved the in-context callback actually ran. The target exited, the hook was removed, and the original S12 DLL was restored in `finally` and checked against SHA-256 `870c206659b3ab383e87c992dcec220af634ad06741a3f191851d5d4b0f809ae`. This rules out simply replacing the existing window hook with an in-context WinEvent hook as a remedy for this session; it does not characterize out-of-context events or other Windows versions. No product or deployed file was changed.


An additional x64 owned-private-desktop probe registered `SetWinEventHook` with `WINEVENT_OUTOFCONTEXT` and no DLL; registration succeeded and the window-show event from an owned new target arrived. The callback PID was the observing controller (4992), while the event HWND belonged to the target (5100). The owned target and hook were cleaned up. This demonstrates a notification path unaffected by the observed registration failure, not styling or target-process attachment. The existing `ntw16s_install` contract explicitly permits only a newly created uncommitted suspended target, and the existing global attacher requires execution on the target GUI thread. Thus using this notification to style arbitrary already-running windows requires a separate, safely validated target-loading and UI-thread-dispatch mechanism; it is not a drop-in replacement or a completed fix.


## Bounded historical-name inventory

A private-desktop diagnostic enumerated the 40 x86/x64 DLL paths that currently exist directly under the `candidate` directories of `build/t21-package`. Using the same loaded deployed x64 callback with the unsupported native call solely as a name-registration probe, it immediately unhooked every successful call. Two runs each returned 20 handles and 20 errors 126; the successful names comprised 10 `ntw16s32.dll` and 10 `ntw16s64.dll` paths. The process and private desktop exited normally. This bounds an observable set of project test paths that the current session accepts, including both bitness names, despite the native process module inventory finding no matching loaded named modules in its accessible set. It does not enumerate all system names, demonstrate a particular total capacity, establish whether the remaining entries are referenced by inaccessible processes, or authorize use of the unsupported call in the product.


A further read-only diagnostic enumerated the current window-system atom namespace through the undocumented `NtUserGetAtomName`, with the existing `Win16Style.Global.Control.1` registered-message atom as a successful positive control. It returned 705 names including 16 containing `.dll`, but no name containing `ntw16s`. This does not expose or disprove the separate hook-module registration array described in historical kernel diagnostics; it confirms that simply searching this accessible atom namespace is insufficient to identify the 20 accepted project path names. The undocumented call is a diagnostic only and is not used by product code.


## Same-process thread-hook feasibility

With the deployed path still subject to 126, native x86 and x64 prototypes each loaded a newly named own test DLL into an owned process, created their own GUI thread, and called `SetWindowsHookExW(WH_CALLWNDPROC, callback_in_loaded_DLL, NULL, own_gui_thread_id)` from another thread of that same process. In both cases registration succeeded and a bounded message confirmed the callback actually executed on the GUI thread; the hook was removed and the owned thread, process and private desktop exited. This differs from the earlier failed per-thread test, which supplied a non-NULL module handle and therefore encountered path registration. The result shows a potential way to enter the existing target-thread attacher without a registered hook DLL name **after the DLL and installer are already executing in the target**. It does not prove loading or safely scheduling into any existing process, installer-thread lifetime, cross-bitness coordination, full window coverage, detachment, security boundaries, or compatibility with ordinary mode. No user process was inspected, modified or injected by this prototype, and no product change is claimed.


## Owned already-running target prototype

Two C prototypes, x86-to-x86 and x64-to-x64, started an owned target on a private desktop and waited for its visible GUI window before modifying that target. Each resolved the remote loader address using the matching target module base and local export offset, loaded an own test DLL into that process, then started an own worker inside it. The worker installed a `WH_CALLWNDPROC` hook for the target GUI thread with a NULL module handle, signaled ready, waited for stop and unhooked. A bounded message caused the target-thread callback to signal; both widths returned `loaded=1 ready=1 sent=1 callback=1 stopped=1 code=0`, and the targets, job and private desktops were cleaned up. These are same-bitness feasibility tests of a *test DLL*, not production attachment: they do not establish cross-bitness bridging, runtime style rendering, ownership, race-free shutdown, access to arbitrary applications, integrity/security limits, multi-thread enumeration, process lifecycle handling or safe release. No existing user process was modified and this does not resolve or explain the kernel 126 condition.


## Real attacher in owned running targets

A further private-desktop prototype used the unmodified S12 product DLLs, the existing session mapping/owner checks, and `Ntw16sGlobalHook` itself as the target-local thread-hook callback. An owned target loaded its matching real DLL without a startup payload, then an owned remote worker installed the callback for that target's GUI thread with a NULL module handle. An apply control message caused `Ntw16sWindowAttached` to report 1; a stop/remove control message caused it to report 0; the worker then unhooked and exited. Both same-bitness x86 and x64 runs passed, with all targets and desktops cleaned up. To satisfy the existing owner-image directory check, only the `run16s.exe` in the previously registered S12 **scratch candidate** was temporarily replaced with the own controller; the original was restored in `finally` and checked against SHA-256 `5d039bb89ac0a1de2e9e7b02bd5481e71c764f8b83720179df97e3688035034e`. This proves real attachment-state transitions in owned cases, not the visual result, full control styling, cross-bitness coordination, existing production controller integration, safe use on arbitrary processes or complete shutdown under races. No deployed files or existing user processes were modified.


## Owned-window print comparison

The real-attacher prototype was extended to hash the top 36 pixels of `PrintWindow` output for the owned target before apply, during attachment and after remove. Both x64 and x86 successful runs produced respective before/styled/after values `1499439674 / 769920024 / 1560912886`, while attachment transitioned 0/1/0. This is evidence of changed rendered pixels, not a visual fidelity or complete recovery proof: after-remove differs from the initial capture and the comparison does not control activation or other system state. One initial x86 run failed to find the remotely loaded test DLL (exit 13); an instrumented retry passed and the cause of the first failure remains unproven. The private scratch controller was restored and hash-checked after each run; no deployed file changed.


The x86 scratch prototype was instrumented to print the remote loader exit value and given a bounded retry for `ERROR_BAD_LENGTH` from module snapshot creation. Four subsequent isolated runs all loaded the module, attached, observed changed print hashes, removed and shut down successfully; the scratch controller was restored and hash-checked. The instrument did not record whether any snapshot retry was needed, so these passes do not prove the cause of the prior one-off exit 13 or establish production reliability.


The x64 owned target was then captured with native `PrintWindow` to three BMPs in task scratch. Visual inspection shows a native title, the Win16 title after attachment, and the native title after remove. The before/after images are both 150 x 100; an RGB pixel comparison found 812 changed pixels bounded by x=8..141, y=31..91, so the top 31 rows are identical and residual differences remain in the lower client area. The attached image is 142 x 90 and therefore cannot be directly compared pixel-for-pixel at the same coordinates. This is one x64 owned-window sample using the S12 attacher, not a complete visual or production-package regression. All owned processes exited and the scratch controller was restored and hash-checked.


## Cross-bitness host feasibility

An owned private-desktop prototype used a 64-bit controller and its real-format session mapping, started Windows `SysWOW64\rundll32.exe` to host an x86 test helper DLL, and used that helper to start an owned x86 target and install the real S12 x86 `Ntw16sGlobalHook` as a NULL-module same-process thread hook. The target reported attachment 1 and, after the stop control, 0; the worker and system host exited and the private desktop was released. The controller's scratch candidate `run16s.exe` was restored and checked against SHA-256 `5d039bb89ac0a1de2e9e7b02bd5481e71c764f8b83720179df97e3688035034e`. This proves one 64-controller/32-helper/32-target coordination pattern, not a production implementation: the helper is a separate scratch DLL, the target was created by the helper, and integrating its role into the existing ntw16s32 product DLL, discovering independently existing targets, security checks, visual effects, ordinary-mode coexistence, fault cleanup and all races remain unverified. No fifth product or deployed file was added.


## Internal worker implementation in progress

`global_thread.c` adds an internal, currently unexported and uncalled target-local worker to the product build. It validates the same-process GUI thread and session cookie, verifies owner identity/liveness and phase, switches the fresh worker to the GUI thread desktop, installs the existing callback with a NULL module handle, and removes the hook on stop, owner exit or target-thread exit. The caller must retain the immutable same-bitness packet until return and create the per-thread ready event and session stop event. A self-contained private-desktop test creates only an owned target, checks bad-cookie rejection, receives a real hook callback, signals stop and verifies no callback after worker exit. CMake/Ninja incremental x86 and x64 builds and the corresponding CTest cases passed in under one second each. An initial x86 CTest run used an out-of-date test driver and failed before launching the target; adding the explicit build dependency rebuilt it and the test passed. This is an internal building block, not an exported API, controller integration, complete security review, clean package, binary publication or fix for deployment error 126. Existing user processes were not modified.


The internal worker test now also uses an independently created owned owner process. After a target-thread callback, the test lets that owner exit without signaling the worker stop event; both x86 and x64 workers notice the owner process handle, unhook, return, and a subsequent message produces no callback. Normal-stop and owner-loss CTest cases both passed in each architecture, two cases per architecture in about one second. The test covers process-handle signaling after a normal owner exit, not every crash, PID reuse or remote injection race. No product controller integration or binary delivery is implied.


The worker is now exposed for future target-local control as additive named export `Ntw16sGlobalThread`, ordinal 37, stdcall on x86 and native x64 calling convention. The existing ABI version remains 3 and all earlier exports and ordinals remain unchanged; the exact export/ordinal package gates and ABI test were updated. Both incremental DLL builds and actual `core-abi` runs passed with 36 named exports; calling the new entry with NULL returned `ERROR_INVALID_PARAMETER`. This is not yet called by the controller, not a clean-package validation, and not a binary release or 126 fix.


## Read-only same-bitness loader resolution

The in-progress fallback now includes an internal resolver compiled into the launcher and DLL. It finds the actual module containing the local `LoadLibraryW` export, restricts it to the system kernel32/kernelbase directory, snapshots same-bitness target modules with bounded `ERROR_BAD_LENGTH` retries, and checks path, PE machine, image size/timestamp, executable image mapping and the first 16 entry bytes before returning the target-relative address. It only reads target memory and has no export or controller call yet. These checks are not a cryptographic image identity proof and do not remove target exit, module replacement, loader or synchronization races; callers must retain a validated target handle and account for failure. Owned private-desktop x86 and x64 tests verify resolution against a running own child, reject a mismatched PID and clean up; the three worker/owner-loss/resolver CTest cases pass per width in about 1.4 seconds. No injection or user process modification was performed, and this is not a deployed fix.


The read-only resolver now also finds the actual target DLL worker export by parsing the target's bounded PE export directory, rejecting unexpected filenames, paths, invalid ordinals, forwarded exports and non-executable image addresses. The x86/x64 test loads each actual freshly built project DLL in an owned running child and compares the resolved entry bytes; it also verifies wrong PID, wrong name, missing path and an intentionally corrupted export count in a second owned child are rejected. Each architecture passed in about 0.4 seconds. An initial CMake run failed because its forward-slash path was not normalized; the resolver now canonicalizes the full path and both tests pass. These checks do not make target memory immutable, solve module unload/races, or implement remote injection; controller integration and deployment remain pending.


## Bounded owned-target loader building block

The in-progress launcher/DLL now contains an uncalled same-bitness remote-load ticket. It validates the candidate DLL file and holds a read handle without write/delete sharing, resolves the target system loader, writes only the path into the target, and starts the loader thread. A timeout or attempted discard while that thread remains active retains the process/thread/file handles and remote path; cleanup is retriable and never terminates the target. Completion resolves the actual target export and releases the ticket; a target already exited returns `ERROR_PROCESS_ABORTED`. Test-only x86/x64 DLL fixtures delay their loader entry in owned targets to check timeout, retained path, duplicate ticket rejection, completion, and separately terminate only an owned fixture target mid-load to check cleanup. Both CTest cases pass in under one second. The fixture's deliberate bounded DllMain wait is test-only and is not product behavior. This does not cover arbitrary applications, concurrent target module unload/replace, safe rollback of a successful load after subsequent resolution failure, controller integration, or production package/deployment; a successfully loaded DLL reference may remain until target exit.


## Target worker startup building block

The in-progress launcher/DLL now includes an uncalled same-bitness start ticket. It verifies the supplied process and GUI-thread ownership, resolves the actual worker export in the target image, rejects pre-existing ready-event collisions, starts the worker with an immutable target packet and retains process/thread/event/packet ownership across timeouts. After the worker confirms it copied the packet and installed its hook, the remote packet is freed; closing an active ticket returns `ERROR_BUSY` rather than terminating the target. On both widths, private-desktop tests exercise both a preloaded actual product DLL and a DLL remotely loaded into a running owned target, including actual worker ready/stop, wrong PID/thread/cookie and event collision. Two tests per width pass in about 1.3 seconds. This does not validate visual attachment (the test owner is intentionally not named `run16s.exe`), monitor/controller discovery, x64-to-x86 coordination, timeouts under every race, arbitrary application injection or deployment. The caller must hold a verified target DLL reference for the worker lifetime.


## Read-only window discovery building block

The fallback has an uncalled same-thread window discovery unit: it installs an out-of-context show-event observer before enumerating existing visible top-level and child windows, reports HWND/PID/TID, and requires its caller to revalidate identity before any later action. It does not itself open, inject into or modify targets. The callback rejects reentrant stop and stop from the wrong thread; caller must pump messages and own the watcher through callback completion. Private-desktop x86/x64 tests use only owned children and verify existing windows, a later new process, a newly shown child in the existing process, no notifications after stop, nested-stop rejection and wrong-thread rejection. Each case passes in about 1.2 seconds. Duplicate show/scan notifications, races and inaccessible/hidden targets remain for the future controller to handle; discovery alone does not apply style or establish complete desktop coverage.


## Target identity boundary building block

A read-only fallback policy now validates a visible HWND against its PID and GUI thread twice, checks same session and user SID, rejects AppContainer, checks the requested native x86/x64 machine, records process and thread creation times, and applies the existing shared Windows host exclusion before a future injector is called. Ordinary `run16s` origin is not an exclusion. The existing global hook now uses the same image exclusion predicate. Both x86/x64 private-desktop owned-window tests pass for identity, birth times, wrong PID/thread/machine/SID data and the image exclusion predicate. Different-user, AppContainer, elevated/protected and PID reuse race behavior have not been exercised. Incremental DLL PE gates pass after adding the Windows `ADVAPI32.dll` system dependency for token checks. With new DLLs temporarily at the previously registered S12 scratch path, the existing x86/x64 control and ordinary/global coexistence tests passed (including 144 DPI input), then both scratch DLLs were restored and hash-checked. This is not yet wired into a fallback manager, a full clean-package regression or a deployment.


## Same-bitness actual-DLL single-window coordinator

The fallback now includes an uncalled nonblocking same-bitness coordinator. It inspects the discovered target, opens and retains the exact process object, revalidates process and GUI-thread birth plus HWND ownership before mutation, starts the bounded loader, starts the target-local worker after loading, and sends the existing apply/remove protocol with a bounded message. Tickets remain owned across pending work; close refuses to tear down active remote workers. The new private-desktop x86/x64 tests use an owned `run16s.exe`-named test controller and a byte-identical copy of the corresponding freshly built actual product DLL, so the existing owner-directory check remains active. Each starts an owned target without the DLL, loads it, verifies actual parent and button attachment, button click, stop/removal and target cleanup; both passed in under one second. The package test explicitly compares fixture DLL SHA-256 with the package DLL before using this case. Process/thread birth mismatch tests also pass at both widths. This is one owned window per architecture, not the production controller, cross-bitness scheduling, multiple threads/processes, complete visual verification, or a deployed 126 fix. Control-message success itself is not treated as proof of styling; the owned test additionally queried actual attachment and clicked the control. HWND reuse on the same surviving thread, target races, failed-message cleanup, loader hang and arbitrary application safety remain to be handled by the manager.

## Unintegrated manager probe

A candidate manager combines the out-of-context observer and coordinator with bounded worker concurrency. On a private desktop, its same-bitness x86 and x64 tests each style one owned window that existed before manager startup and one owned window created afterwards, verify parent and button attachment plus button clicks, send removal, signal stop, and clean up both targets. The individual tests took 0.84 and 0.80 seconds. A sandboxed attempt could not create the private desktop (error 5); the same isolated test passed outside that sandbox. This manager is not yet connected to the real controller or x86 host, and these tests do not prove cross-bitness coordination, arbitrary application compatibility, thread/PID reuse, all shutdown races, or deployed behavior. The user cannot currently log off, and a logoff is not a prerequisite for continuing this work. No new package was released.

## P30 integrated fallback and stage package

The x64 controller and x86 system host first try the existing system hook. When registration returns 126, the corresponding width uses the existing target-local thread hook and an out-of-context discovery manager; other registration errors are not silently converted to success. Four bounded manager workers handle each width, with same-user/session, architecture, process/thread birth and HWND revalidation; ordinary-launch processes are not excluded. No fifth product, global system setting or resource was added. The DLL remains pinned in processes where the renderer attaches, as before.

In four-way package regression, the first two attempts failed because an existing x86 VB test window was not attached. A temporary diagnostic build observed control-message timeouts 1460 after target worker initialization under concurrent load; other errors 193 represented cross-architecture targets rejected by policy. This is evidence of a plausible failure path, not proof that every failed window had that cause. The manager now retries a timed-out apply message up to three times without reloading or installing another worker. A new x86/x64 owned-target test deliberately occupies the GUI thread for 250 ms, checks both windows reach the active manager state, interact and detach; both widths passed. A scratch-only negative with retry removed failed (`second=0`, exit 14). The original four-way combination subsequently passed twice.

The final clean GCC/Ninja/ccache snapshot built both widths, passed ABI, source and byte-exact resource gates, and produced the four-file package. Actual package focused regression passed 25/25, zero skips, including 96/144 DPI, x86/x64 existing/new windows, ordinary/global coexistence, dynamic controls, VB fixture, repeated multithread cycles, owner/host failure, and ordinary-mode propagation. Two additional immediate-menu tests failed: `global-menu-initial-x86` and x64 each observed zero owner-draw messages; in a separate run `global-system-menu-x64` showed the native menu. The tests create a window and immediately open its menu, potentially before asynchronous discovery and attachment. This is a real uncovered timing boundary, not a passed test; it remains open. No claim is made that a window can never briefly appear native, that every process can be injected, or that session-table exhaustion has been diagnosed.

The actual final package also passed the optional owned private-desktop MadShow comparison: matching x86 DLL loaded in pre-existing and new instances; the title print hash changed, Grid received four clicks, both targets remained alive after global stop, and no sweep notification timed out in that run. A prior diagnostic run had one notification timeout. The observed restored print hash differed from the pre-start hash, so perfect pixel restoration is not claimed. The package was published to `assets/` and `O:\\winnt` with matching SHA-256: demo `57025f0bda5dbc4bf07d81b7cc9ff65b248946be50c26b0c0a0ea2456184e502`, 32-bit DLL `33fa365e1139ef9adb43b37b4a0ee476c11cde5375b3311e79819d3e3b7658bc`, 64-bit DLL `1e0f54be3cdd59d34027824abc58767b54e944ce541d9c456b5cf918e2e24f7d`, launcher `ed2eea4a3e48b1483318e46edd759a6f710c0cecb18222729d89305ab042b2f4`. An owned private-desktop run using `O:\\winnt\\run16s.exe` itself reported x64 error 126 and passed existing/new dual-width attachment, normal-mode retention, input and stop. Protected/elevated targets, Explorer, UWP/XAML, complete restoration and all races remain outside this evidence. S13 and T21 stay open.

## P31 menu witness and readiness correction

The P30 conclusion that both ordinary first-menu failures demonstrated native drawing was too strong. A private diagnostic showed both the project DLL and attached HWND, and `MFT_OWNERDRAW` on the popup items at `WM_ENTERIDLE`, even when the old observer counted zero draw messages. The global renderer's later-installed subclass consumes `WM_DRAWITEM` before the earlier test subclass sees it. The regression witness now uses a same-thread `WH_CALLWNDPROC` hook with a NULL module handle to observe actual menu draw messages before subclass dispatch; unstyled x86/x64 negative controls each reported zero, while styled tests observed three or six messages per architecture. This observes messages, not a pixel-perfect screenshot.

The system-menu witness did exhibit a separate readiness race: P30 marked its internal phase active before both bitness observers were installed. In four immediate-launch private-desktop runs, three showed native system menus, while all four passed after a 300 ms diagnostic delay. A new internal session `ready` field becomes set only after the x86 host and x64 hook or fallback manager are ready; it is cleared during shutdown. The relay waits for this field without inserting a fixed delay. Two four-way incremental runs and a four-way run using the actual `O:\\winnt` package passed both bitnesses for ordinary and system menus. Asynchronous fallback attachment still cannot guarantee first-frame coverage for all target programs.

The clean same-snapshot package passed 29/29 focused cases serially, including those four menu cases, without skips. An earlier four-job batch failed `global-real-demo` after the new target's DLL could not be confirmed (last error 18); its isolated rerun passed. A second four-job batch failed the x64 multi-thread resource threshold with GDI 22 to 25; its isolated rerun passed with 22 to 23. Their underlying causes are not established and the passing serial run does not erase them. Actual deployed-path dual-width existing/new windows, input, ordinary-mode retention and stop passed again under error 126 fallback.

The four P31 package SHA-256 values are demo `799ea91817277bfd9af5786e53c61608c6da7dc85ae36339b1a92746013a3f0f`, x86 DLL `9aa3f9bec3dd6cf7d5fdbbe9ab7f43154cfd012a71ab1b2f97bec0ec002e64f3`, x64 DLL `088fc55c0603803dc420342b8379e2caa11bc94c823b345223f09092c1c41b74`, and launcher `7c199231fd4520d7fd2879583f229a9de10471d7ccac70f5bf7be2408c74c77c`. They were published to `assets/` and `O:\\winnt`. S13/T21 remain open for the task-level audit and the unresolved concurrent-run observations.

## Further concurrency observations

A later four-job run of the same P31 package failed the x86 manager test: both target buttons remained clickable and both windows were detached at shutdown, but the combined second-target attachment/manager-active predicate did not complete within its test budget. Those observations do not distinguish which part of that predicate failed. A temporary incremental build increased the existing bounded control-message retries from three to ten and split that diagnostic, but a four-job suite using it instead failed the x86 immediate-menu witness (`draws=0`, `idle=0`). The manager case passed in that run. This does not establish that increasing retries fixes the manager failure or causes the menu failure; the retry change and test-only diagnostic changes were withdrawn and were not published.

Eight simultaneous owned private-desktop x86/x64 samples using the published P31 package and a scratch diagnostic witness all passed: the queued menu click found an attached window, and the popup initialization and owner-draw messages were observed. The logging itself changes timing, so those passes do not explain the earlier no-idle failure or rule out an asynchronous race. No additional product binary was published in this investigation. The goal remains open; the first-frame and concurrent-run limitations remain explicitly unproven rather than characterized as fixed.

## P33 busy-target confirmation

An owned private-desktop fixture delayed its first target control handler by 650 ms and separately observed actual HWND attachment and the manager's active count. With the P31 retry policy, both x86 and x64 reproduced `attached=1 active=0` and test exit 14; both windows were still clickable and removed at shutdown. This proves a bounded target delay can make the manager report failure after attachment, but the earlier concurrency failure lacked the separate diagnostics and is not proven to have the same cause. The manager now retries only the existing apply message until its existing five-second operation deadline, checking stop between attempts; it neither reloads the DLL nor starts another worker. The updated fixture is a negative control for P31 and passes both widths with the fix. Four concurrent owned fixtures also passed.

The clean same-snapshot package passed 29/29 focused cases serially, zero skips. In a four-job package run, both busy-target manager cases passed but the pre-existing x64 resource threshold failed again: GDI count 22 to 25 after three cycles, with unchanged handle count. Its cause remains unknown; this run is not counted as a full pass. The actual deployed `O:\\winnt\\run16s.exe` path still reported system hook error 126, used the fallback and passed dual-width existing/new, input, ordinary-mode retention and stop checks. Package SHA-256: demo `4a18992637e341abec363be5dda0e89e50215925a77ecaa3f36a43c9fe41e15f`, x86 DLL `0a5990e5dc553522b02fae65126f793c14e2f2fbd5f0d7ab28f93494b3270145`, x64 DLL `75ec49d0f035e5adabe1427149e782c64ceead5148cc2ccb2eb8bcc7fb55683d`, launcher `14462e23961266be10b542a314238f3ae32cacc93a469d2ae648d08c9ba7b51f`. All four were published to `assets/` and `O:\\winnt`. First-frame and concurrent resource observations remain open; S13/T21 stay open.

## Bounded resource follow-up

Four simultaneous private-desktop runs of the original three-cycle dual-width probe passed, with x64 GDI count 22 to 24 in those samples; the earlier 22 to 25 failures are retained. Two additional own-target diagnostic runs each executed eight start/stop cycles. Handle counts remained 471/214 and 469/214 respectively in those samples; x86 GDI varied 53–55 and x64 moved from 22 to 24, then remained 23–25. One x64 sample was 24 at cycle 6 and 25 at cycle 7. These finite observations do not show a per-cycle linear rise, but do not prove absence of a leak, identify object types, or explain the previously exceeded threshold. They used the published path and did not modify product binaries or other processes.

## Tracked renderer objects

An x64 scratch-only DLL instrumented object creation/deletion in `win16_style.c`, `font.c`, `controls.c`, `attachment.c`, `theme.c`, `menu_items.c`, and `menu_paint.c`. It was combined with the other three P33 files solely in an isolated diagnostic package; that mixed diagnostic package was not published. One owned target completed eight start/stop cycles. Of the successful operations observed in those seven files, 64 created fonts, 192 bitmaps, 32 brushes, and 240 compatible DCs each had matching successful deletion; 92 regions were passed to successful `GetDCEx` calls with ownership-transfer flags, and no observed deletion call failed. Microsoft documents that the system owns and eventually deletes such regions after a successful call ([GetDCEx](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getdcex)). The process-wide x64 GDI total nonetheless moved from 22 to 26 in this instrumented run. The tracing does not cover every source file, APIs used internally by Windows, system-owned regions after transfer, or all timing and process lifetimes; instrumentation itself may alter timing. It therefore neither explains the total difference nor proves the absence of a leak. The scratch clean-source CMake input and candidate DLL were restored and hash-checked afterwards; the published package was not modified.

## GDI flush and region comparison

In a private-desktop eight-cycle run of the published package, both owned targets sent `GdiFlush` on each of their two GUI threads after every stop. In that sample, the process GDI count was identical immediately before and after each flush: x86 varied between 55 and 57, and x64 between 22 and 24. This does not identify object types or prove all system work was complete.

An additional scratch-only mixed diagnostic package changed the traced x64 region path to select the region into a DC and explicitly delete it instead of transferring it to `GetDCEx`. This is an experiment, not an approved painting change or an equivalent-visual proof. Eight owned start/stop cycles yielded x64 total GDI 22, 23, 24, 24, 24, 24, 24, 24. The seven instrumented files logged 64 font, 192 bitmap, 32 brush, 240 DC and 92 region creations with matching successful deletions; no tracked deletion failed. Since the total still changed in this one altered run, the observation does not support attributing the entire increase solely to the original region transfer; it also cannot exclude Windows or uninstrumented objects. The original CMake input and x64 DLL in scratch were restored and hash-checked; no product change or binary publication resulted.

## P37 exited-target retirement

The fallback manager originally never removed entries for exited targets. Its per-session entry count therefore only increased and could eventually exhaust the 16,384-entry bound even with few live targets. A self-owned x86 negative regression closed one of two test processes, generated later window-show events and still found its exact PID/TID tracked (`exited=1 retired=0`, exit 14). The manager now checks eight hash buckets per discovered window event and, at capacity, checks all buckets before refusing a new entry. A completed target worker or exited retained process is retired only after its attachment ticket closes successfully; failed entries without a retained process are retired when the process no longer exists. The normal path performs no timer polling; an idle session may retain stale records until another eligible window event. Access-denied PID checks and PID reuse of failed entries without retained identity remain limitations, as does a completely full table of live targets.

In x86/x64 owned private-desktop regression, the exited PID/TID was no longer tracked, the other attached window remained styled and clickable, and a later new target attached and responded. Four concurrent runs passed. Separate scratch-only dual-width builds set the limit to three and logged `CAPACITY_SWEEP before=3`; both retained the live target and admitted the replacement. These are reduced-limit probes, not a 16,384-process test. The same clean batch passed 29/29 serial actual-package cases without skips; previous concurrent GDI and first-menu observations are not erased. The deployed `O:\\winnt\\run16s.exe` passed dual-width existing/new, input, normal-owner retention and stop under the observed 126 fallback.

The P37 four-file SHA-256 values are demo `040bedef4f9a5e53dca219964d45720bfaf7d9656b323b62fdf2fcdcaf69996a`, x86 DLL `d77314c7443568a5bb5ee8e6cd95e9fc3f85a8616abf5e5cab1bf0318389482d`, x64 DLL `b8ce95e59e3704aeec58964fabcd3d3ea66eaa3cb6f0a9024b7d256b384a7a39`, and launcher `277c83179aa15e3cfa9c02c3d0ba9eb55f8b0f303942a406f754ae115dd1dd0b`. They were published to `assets/` and `O:\\winnt`. The first publication attempt was interrupted by mapped old DLLs, including the running Codex, terminal, browser and system input processes. Rather than close these active programs, each old image was moved on the same volume into `build/t21-package/manager-retire-publish/held-ntw16s32.dll` or `held-ntw16s64.dll`; the same tested batch was then installed and both destinations hash-checked. Existing processes may retain their old mapped image until exit. The held files are retained task scratch and must not be deleted while mapped. S13 and T21 remain open.

## P38 bounded GDI object classification

A scratch-only x64 owned-target diagnostic inspected the current process's Windows shared GDI handle table after each of eight global start/stop cycles. Its internal table layout is undocumented and the classification is diagnostic evidence only, not a supported product dependency. `GetGuiResources` independently reported x64 totals 22, 23, 24, 24, 24, 24, 24, 24; the diagnostic classified the corresponding region counts as 12, 13, 14, 14, 14, 14, 14, 14, while the listed DC, bitmap, font, brush and pen counts were unchanged. A separate run with no global manager and repeated own-window redraws reported x64 total 9 and zero classified regions in every sample. This control differs in work performed and does not establish a leak, the lifetime or ownership of the additional regions, or the cause of the earlier threshold failure. The same styled run passed both widths and released its owned processes; no product or deployment file changed. The x86 control's resource count changed and was not diagnosed; it is not used to infer the x64 cause.

## P39 initial-menu witness boundary

Eight simultaneous private-desktop runs of the deployed package and original first-menu witness reproduced one x86 `draws=0 idle=0` failure; seven passed. A scratch witness recorded menu initialization, entry and final attachment without changing the product. Twelve additional simultaneous x86/x64 styled runs all reached the menu loop, reported final attachment and passed; the added diagnostics can change timing and do not explain the failed original run. The same diagnostic witness without a style DLL reported no menu initialization or entry in eight x86/x64 private-desktop controls. The witness posts `WM_NCLBUTTONDOWN` instead of generating physical input, so `idle=0` alone does not prove a real mouse click, menu rendering failure or the absence of attachment. It remains an unresolved timing/input boundary; the passing instrumented runs are not a fix. All runs used self-owned processes. No product or binary deployment changed.

As a distinct input control, a scratch witness posted `WM_SYSCOMMAND/SC_KEYMENU` for the File accelerator instead of the synthetic non-client mouse message. Four unstyled x86/x64 controls all entered the menu loop with zero project owner-draw messages (their existing style-required exit code was therefore 1); four styled x86/x64 cases all entered the loop and observed three owner-draw messages. This establishes a useful keyboard-command comparison, not physical mouse coverage or an explanation of the earlier one-off failure. The existing mouse-oriented regression was not replaced.

## P40 keyboard-menu regression

The existing menu witness now accepts a separate keyboard-command mode while retaining its original synthetic mouse-message mode. Four focused private-desktop runs against the deployed P37 package passed: both widths reached the menu loop and reported three owner-draw messages under global mode; both unstyled controls reached it with zero owner-draw messages. As a negative check, requiring styled output without global mode failed on both widths. The four cases are registered separately in the package test list; the PowerShell script parses successfully. These tests cover a posted system menu command, not physical keyboard or mouse input, and they do not explain the earlier mouse-witness failure. This phase changes test code only; no product binary was rebuilt or published.

## P41 formal package runner verification

The P40 four keyboard cases were run through `tools/Test-Package.ps1` against the unchanged P37 four-file package. The isolated test build root paired byte-identical P37 product outputs and existing P37 test tools with the newly rebuilt x86/x64 menu witnesses; it is not a clean rebuild of the product. Package identity/resource/ABI checks and all 12 selected cases (four affected plus eight mandatory) passed with zero skips. The four affected cases took about 1.5 seconds each; the mandatory `demo-pair` took 44.3 seconds, accounting for most of the 46.8-second parallel run. This is a focused test, not the complete integration or core matrix, and does not change previous physical-input or occasional mouse-witness limitations. No product file or deployment changed.

## P42 windows created visible

The dual-width four-thread owned-target fixture now has a distinct `visible` mode: each additional top-level window is created with `WS_VISIBLE` and is not subsequently shown with `ShowWindow`. Against the deployed P37 package, that mode and the existing explicit-show mode both passed on a private desktop. Each covered four already-existing roots, four new top-level windows and their buttons, three global start/stop cycles and attachment removal; the visible mode is separately registered in package tests. This verifies the exercised same-thread creation case, not arbitrary window classes, first-frame appearance or every visibility transition. Product binaries and deployments were unchanged.

## P43 actual-package visible-window attempt

The formal focused package runner did not pass. The first four-job run stopped on `global-threads-visible`: its three attach/remove cycles completed, but x64 GDI count increased from 22 to 25, exceeding the existing +2 limit; one global start also reported one timed-out or failed window notification. A subsequent serial run stopped earlier on the pre-existing `global-threads` case with the same x64 22 to 25 resource assertion, before reaching the new visible case. These failures supersede neither the previous isolated visible/explicit-show passes nor the earlier serial 29-case package pass; they show the threshold issue is not restricted to parallel test execution. The counting cause and whether the extra regions persist or leak are not established. No retries are reported as a passing package run, and no product or deployment file changed.

## P44 user-reported late-dialog and revised ownership contract

The user reports that windows/dialogs created after global activation remain native and requests one unified propagating process-hook model: global activation should install it in running processes where possible, and global stop need not preserve ordinary-launch hooks or styling as a special case. The existing fallback instead installs target GUI-thread message hooks and attaches windows; it does not install the ordinary CreateProcess interception into already-running processes. Ordinary `startup_hooks.c` explicitly assumes an initial loader thread and can terminate a process on transaction rollback failure, so reusing that bootstrap in a live multithreaded process is unsafe. A safe late-attach and stop design is not yet implemented.

A self-owned 32/64-bit `MessageBoxW` probe found the dialog attached but its OK button not reported attached after 2.5 seconds, in both global and ordinary launch modes. This is not evidence of a global-specific regression: the same witness result in ordinary mode may reflect shared behavior or the witness itself. The user's `D:\programs\misc\winmine.exe` was confirmed x86 PE; an isolated owned instance opened Help → About (command 593, `#32770`), and native `PrintWindow` screenshots showed a classic title both when the target started before global and when it started afterwards. That limited observation does not reproduce the user's native About appearance and cannot disprove their real-session report; screenshots are task scratch. One stop reported two timed-out or failed notifications while the probe was closing its target, so cleanup timing remains uncertain. No user process was modified, no product fix was published, and the revised contract remains open.

## P45 propagation entry negative witness

A new owned x86/x64 native fixture starts with a visible root, waits until the root is attached, and then creates a child with no window; the child tests for the matching DLL at its entry and, if loaded, creates an equally windowless grandchild. Ordinary `run16s` passed both widths: child and grandchild reported the DLL at entry, the child returned zero, and both processes exited. Under the deployed global mode, both widths had an attached root but the child reported no DLL at entry and exited 41; its grandchild was therefore not created. This confirms that current global window discovery does not implement the user-requested process propagation for this case. It does not by itself establish initialization state from module presence; the ordinary grandchild observation separately shows actual second-generation propagation for this fixture.

The fixture builds in both architectures, and the ordinary positive controls are registered in package tests. The failing global comparisons remain explicit diagnostic commands until the product implements the feature; they have not been hidden or counted as passing. They use only owned processes and private desktops. No live-process patching, product binary change, or publication was performed.

## P46 running-process hook primitive prototype

For isolated research, upstream MinHook v1.3.4 source was checked out in task scratch at commit `c3fcafdc10146beb5919319d0683e44e3c30d537`; it was not added as a product dependency. A standalone C executable per architecture started four self-owned worker threads repeatedly calling an owned test function, installed a MinHook detour after the threads were running, exercised it, disabled it, stopped/joined the threads and uninitialized the library. Both returned success with no unexpected function results; the x86 run recorded 2,794,063 calls / 913,754 detoured calls and x64 recorded 4,916,694 / 1,228,420. Counts are timing-dependent, and this checks neither Win32 API interception nor injection/propagation, third-party hooks, busy callback removal, abrupt process/controller termination, or production late attach.

Inspection of upstream `src/hook.c` at that pinned revision shows its `Freeze` implementation marks threads it cannot suspend as unsuspended and continues. Consequently a successful upstream return is not sufficient evidence that all concurrently executing target threads were protected from patching. The project startup-only transaction also assumes initial-loader execution and shares MinHook buffer code, so directly invoking or combining these backends inside arbitrary running processes is not a validated solution. No existing process, product source, binary or deployment was modified by the prototype. A fail-closed thread quiescence approach, coexistence with ordinary-mode hooks, in-flight callback lifetime and cross-bitness actual propagation remain required before integration.

## P47 fail-on-suspend-failure experiment

A scratch-only variant of upstream MinHook first suspends every enumerated thread before rewriting any saved instruction pointer; if a suspension fails, it resumes previously suspended threads and returns an error without patching. In a deterministic test that forces the second suspension to fail, self-owned x86 and x64 four-worker processes both reported an error, an unchanged target function, continued worker progress and no unexpected return values. With fault injection disabled, the same dual-width exercise enabled and disabled the detour and observed no erroneous returns. These are short, bounded tests of an experimental fork, not a production hook backend.

The prototype still reopens thread handles and ignores failures in instruction-pointer/context handling; thread creation and exit during enumeration, rollback after a context update, in-flight detours, loader-lock interactions, collisions with ordinary-mode hooks and actual CreateProcess propagation are unproven. Its fault-injection error value is a placeholder, not a product API. Thus it does not establish safe late installation and was not imported or deployed.

## P48 late CreateProcess propagation prototype

A scratch-only C program starts an owned worker thread, then uses the experimental P47 MinHook fork to install a `CreateProcessW` detour in its own already-running process. The detour creates only the requested owned child suspended, calls the project's existing `ntw16s_install` from its locally built installer library with the published `O:\winnt` package and style/propagation policy, then resumes it. On a private desktop, 32→32, 64→64, 32→64 and 64→32 all succeeded: the launched styled test root attached, its child and grandchild each reported the matching DLL at entry, and the target plus prototype returned zero. The cross-width cases exercised the existing helper path. Each prototype disabled the hook after its owned descendant exited and then joined its worker.

This demonstrates actual late `CreateProcessW` interception and existing-installer propagation in this isolated prototype, not initialization of the product's global manager or arbitrary existing processes. It intercepts only W, does not handle existing ordinary hooks, concurrent creation/in-flight detours during disable, process failures/rollback beyond the owned fixture, diagnostics, security boundaries, or the P47 thread-enumeration/context races. The prototype is not suitable for production deployment and no product source, released binary or active user process was modified. Temporary upstream code remains scratch only.

## P49 busy instruction-pointer guard experiment

A further scratch-only MinHook variant avoids rewriting suspended threads' instruction pointers: it rejects the operation if a captured instruction pointer is within a conservative 64-byte target region or the trampoline slot, then resumes the suspended threads. A self-owned dual-width test held a worker inside the target function using an in-function volatile loop. Both versions rejected hook installation and the worker resumed and returned 12. A positive dual-width private-desktop run also installed the guarded `CreateProcessW` prototype and propagated through the existing installer to the child and grandchild. These are finite tests of an experimental fork, not a product backend.

An earlier version of the busy test used an interlocked Windows API call in its loop: x86 rejected, but x64 unexpectedly allowed the operation because the sampled instruction pointer could be inside the called API rather than the test function. That failed observation demonstrates that the proposed instruction-pointer check alone does not establish that no thread is executing or will return through affected code. In-flight detours, context/handle failures, thread-creation races and interaction with other hooks remain open. No product or deployment file changed.

## P50 correction: MessageBox button witness

The P44 statement that the MessageBox OK button was not observed attached was a probe error. The fixture assumed `GetDlgItem(dialog, IDOK)`, but enumerating the actual dialog showed a `Button` labeled OK with control ID 2 on this host; the assumed ID returned NULL. Finding the actual Button by class instead, the deployed package reported dialog and button attached in both x86 and x64 global cases, and the same corrected check passed both ordinary-mode controls. This is attachment-state evidence only, not a visual or interaction proof; it does not contradict the user's observation about other windows or explain it.

A speculative manager change to notify an already-active target thread when another window is discovered was built and tested before the witness error was understood. It did not change the erroneous result and was withdrawn completely. The corrected owned MessageBox scenario is now a dual-width package regression. The separately established windowless-child propagation failure remains real and unfixed; no product binary was published.

## P51 retained-handle thread guard building block

An authored, experimental internal guard now snapshots the current process's other threads, opens and retains their thread handles before suspension, suspends them, and rejects a caller-specified address region if a suspended instruction pointer is inside it. It does not rewrite contexts. Error cleanup resumes successfully suspended handles; if a resume fails the handle is retained for caller retry. The guard is not yet linked into any product or manager path.

Native x86 and x64 tests passed: injected failure before the second suspension resumed earlier threads; a worker executing inside the specified range caused `ERROR_BUSY` and subsequently completed; a normal guard paused and resumed a separate counter. Each CTest case completed in under one second, and both architectures also compiled the non-test version with C17 warnings as errors. This is not proof of safe late patching: a single thread snapshot cannot prevent later thread creation; an instruction pointer is not a call-stack or in-flight-callback proof; lock interactions, handle/resume failure, other hook collisions and actual integration remain open. No product binary changed. The source-receipt verifier initially exposed three stale hashes from P40/P42; these receipts were updated alongside receipts for new authored fixtures and the verifier passed. This corrects earlier incomplete source-gate bookkeeping, without implying those earlier stages passed that gate.

## P52 thread-set recheck

The experimental late guard now takes a second current-process thread snapshot after suspending the handles from the first and rejects a changed set before any patching. A test-only callback created an additional self-owned worker between suspension and that check; both x86 and x64 returned `ERROR_RETRY`, resumed the original workers and later joined the added worker. Existing injected suspension failure, busy-address rejection and normal pause/resume controls also passed in both widths, each CTest under one second. Source-receipt verification passed.

This narrows one observed race only: a thread can still appear after the second snapshot, and taking a snapshot while other threads are suspended may itself interact with process locks. Instruction-pointer checks do not prove call-stack or callback quiescence. The guard is not linked into product DLLs, no existing process was modified and no binary was deployed.

## P53 single-hook late patch building block

An experimental single-owner late-patch primitive uses the existing MinHook trampoline allocator/decoder and the P52 thread guard. It prepares a trampoline before stopping other threads, checks the target bytes have not changed, publishes the original-call slot and applies one jump while the guard is held. It retains an installed trampoline and does not implement removal; it is linked only to its own tests, not to product DLLs or the global manager. A guard error or target-byte conflict returns without publishing or patching. The design still has the P52 snapshot race and does not make arbitrary existing-process patching safe.

The x86/x64 test keeps an owned worker calling the target; injected guard failure and deliberately changed expected bytes both leave the target and original slot unchanged, while normal installation redirects calls and the trampoline retains original behavior. Four additional parallel owned-process runs passed without incorrect results. The first test variant incorrectly observed constant-folded calls despite the patch and failed on both widths; using a volatile function pointer made it exercise the actual patched entry, after which both CTest cases passed. The implementation does not yet support an atomic multi-hook transaction, safe in-flight shutdown, ordinary-hook coexistence or actual global integration. No product binary was released.

## P54 multi-hook preflight experiment

The test-only late-patch backend can now prepare a bounded batch, reject duplicate/overlapping targets and page-crossing patches, suspend/check once, compare all target bytes, and make every target page writable before publishing any trampoline slot or writing any jump. A conflict in the second target and an injected protection failure on the second target both left both own test functions and slots unchanged; the test also confirmed the first target's page protection was restored. With no injected failure both detours and both original-call trampolines worked while a self-owned worker ran, on x86 and x64. The prior guard and single-patch tests also passed on both widths. This is not a claim that all error paths are atomic: an error restoring page protection, flushing the instruction cache or resuming a thread after writes can leave installed patches or retained handles, and callers must inspect/retain those states. No removal, arbitrary live-process safety, new-thread exclusion after the second snapshot, production integration or binary delivery is established.

## P55 production process-detour prototype

A repository test-only adapter now connects the P54 experimental late batch to the existing production `ntw16s_attach_processes` implementation, including its real `CreateProcessA/W` detours and `ntw16s_install`; it does not rerun startup bootstrap or add another implementation of propagation. In eight isolated combinations (x86/x64 late-hook parent × x86/x64 child × A/W API), the test process started a worker before installing two detours, then created an owned styled target whose child and grandchild reported their matching DLL at entry. All eight passed. The formal actual-package runner passed these eight and eight mandatory cases, 16 total, zero skips. It used the unchanged P37 product package; the test-only adapter retains hooks until its own process exits and is not the global manager.

This demonstrates the production process-detour code can use the experimental batch in owned processes. The batch's remaining thread-creation race, partial late errors, stop/in-flight behavior, interaction with previously ordinary-hooked processes, concurrent initialization, and attaching the rest of the ordinary styling hooks remain open. No product DLL or launcher was rebuilt for delivery, no existing user process was modified and no binary was published.

## P56 reusable late process initializer

The production `CreateProcessA/W` registration was factored to accept an internal attachment callback while retaining the original startup entry point. A separate experimental initializer now serializes setup, prepares the two existing detours through the P54 batch, retains installed state, and treats repeated initialization or an already-populated process policy as a no-op. It is built only into an owned test fixture, not yet linked to the product DLL or invoked by the global worker. Eight A/W × parent/child bitness private-desktop cases passed after this refactor, each calling the initializer twice and observing actual child and grandchild propagation; after a minor path-validation change, one 32→64 A and one 64→32 W case were rerun and passed. A dual-width synthetic pre-existing-policy test verified that calling the initializer did not alter the CreateProcess entry bytes; it does not demonstrate coexistence with actual ordinary-mode hooks. Separately, an incremental candidate containing the production registration refactor passed ordinary dual-width child/grandchild tests. The candidate was not a clean package or released.

The initializer currently does not bind its behavior or descendants to the global session, does not intercept the rest of the styling APIs, and is not safe to deploy to arbitrary running processes: the thread-creation race, error/handle lifetime, concurrent initializer behavior and in-flight stop remain unverified. The revised user contract therefore remains incomplete. Source-receipt verification passed; no binaries were published.

## P57 session-bound propagation candidate

The internal version-1 staging packet now permits an explicit global-session policy and cookie while retaining the 32-byte header; non-global packets still reject a nonzero reserved field, and global packets require a cookie. The existing A/W detours check the matching current-desktop session, active phase and live owner before propagating a session-bound child; ordinary policy retains its previous behavior. In the experimental late initializer, this policy is selected only when a nonzero session cookie is supplied. The test uses an owned private-desktop session and verifies correct and incorrect cookies, stopped phase and preservation of `GetLastError`.

With an incremental same-source candidate and self-owned processes, eight A/W × parent/child bitness combinations all propagated while active. After the session was switched to stopping, both the late-hooked parent and an already-injected waiting child created new descendants that entered without the DLL (the test's expected exit 41). The formal focused candidate-package runner passed those eight plus eight mandatory cases, 16/16, zero skips; dual-width packet parser tests and ordinary propagation controls passed separately. This is not a clean release package or a test of the actual global manager: the late initializer remains test-only and no binary was deployed. It tests suppression of subsequent process injection, not physical hook removal, restoration of existing windows, session races during a create, controller crash, or post-stop styling by an already-injected child. Those items remain open.

## P58 global process-hook integration candidate

The fallback's target-local worker now verifies the session owner against the actual DLL directory, pins the DLL and runs the P57 late process initializer before signaling ready or installing its GUI thread hook. A late-init error is returned by the worker instead of claiming readiness. The existing process detours remain in place after stop, but session-bound calls pass through without injecting when their session is inactive; a subsequent session rebinds the existing process context to its new cookie without reinstalling the same hooks. The user no longer requires preserving ordinary-launch ownership separately; existing already-initialized ordinary hooks are not reinstalled.

For the system-hook callback path, an authenticated target schedules the same initializer on a dedicated target-local thread outside DllMain. The callback defers window attachment until initialization completes, then notifies this process's visible windows. A self-owned same-thread callback simulation initially demonstrated a timing failure in which the first child entered without the DLL; the deferral made the same dual-width simulation pass. The actual system-wide hook registration still returns 126 on this host, so this simulation is not proof that system-wide registration works.

The incremental candidate passed the formerly failing actual global propagation probe on both widths. A self-owned lifecycle test began with an existing target and exercised two global sessions on the same x86 and x64 processes: attachment and child propagation while active, removal and no child injection after stop, and rebind on restart. A focused candidate package run passed 25/25 cases, zero skips, including both lifecycle widths, new targets, A/W cross-width late propagation, local callback simulation, existing common controls and ordinary/global coexistence. An earlier formal run accidentally used the pre-deferral P58c DLL with the newer test and failed local-hook x86; this was a genuine test of that older candidate, not evidence that the current P58d candidate passed, and is retained as a negative result. The corrected P58d candidate and all four tested files were hash-checked by the package runner but were incremental, with demo copied from the earlier build; they are not a clean same-snapshot release and were not published.

This integrates process creation propagation, not the complete ordinary styling/API interception suite for already-running processes. A stop disables session-bound future propagation but does not physically remove detours or prove all existing/new descendant windows restore; in-flight session transitions, thread creation after the second snapshot, API-hook conflicts, actual system-hook success, privilege boundaries, errors in page-protection/handle recovery and long-term stability remain unverified. Target-local code freezes other target threads during late patching; the remaining lock and thread-creation races are a material risk and preclude claiming universal safety. No existing user process was modified by these isolated tests. T21/S13 remain open.

## P59 unified style-hook candidate

The late initializer now registers the same process, window/style, file, COM and folder detours through one batch as ordinary startup. Global window attachment adopts an already attached window without distinguishing how it was started, allowing global stop to attempt removal uniformly, per the revised user request. Session-bound creation scopes and dialog bridges bypass styling/bridging after the session stops; hooks and trampoline storage remain present, and an existing ordinary-started process can still restyle its own subsequent windows because its startup policy is not session-bound. No claim of physical detour removal is made.

The first full-hook candidate showed a genuine lifecycle failure on both widths: a window created after global stop still attached (`stopped_window=1`). Session-gating the creation scope made two cycles per width pass (`stopped_window=0`), with active windows and propagation intact. A second regression left dynamically created child controls attached at stop; adopting already attached windows into the same global removal wrapper made dual-width `global-common` pass and changed the ordinary/global existing-window expectation to uniform removal, matching the current user instruction. The x86/x64 global-window unit tests passed after rerunning on a permitted private desktop; earlier runs without a capture directory or desktop access failed before exercising behavior.

The incremental P59d candidate passed 25/25 focused package cases without skips, including the two-session lifecycle, existing/new target propagation, local-hook simulation, late dialog, dynamic controls and ordinary/global coexistence. The old candidates were retained and their failures are not counted as passing. Original resource and source-receipt gates passed. This is not yet a clean same-snapshot build or published package; thread-suspension/new-thread races, safe teardown, real system-wide hook registration (126 on this host), concurrent stop during creation, and complete external application coverage remain open. T21/S13 stay open.

## P59 clean package regression

The p59c same-snapshot clean x86/x64 build, source and original-resource gates passed. The old global-start test fixture initially failed the owner-path identity check; placing its own run16s-named fixture and candidate DLL together preserved that check, and local and remote CTest cases passed on both widths. The first actual clean-package run then failed global-system-menu-x86; a separate retry showed the same failure on x86 and x64. Each owned target showed a window and immediately opened its system menu: the sampled selection was native (`blue=0`), while the manager reported system-wide hook error 126 and used asynchronous target-thread fallback. This establishes an observed fast-start gap, not that every later window or every host necessarily fails. The run stopped on failure and does not establish results for remaining selected cases. Earlier focused incremental lifecycle and propagation results remain valid only for their stated scope. The failed clean candidate was not deployed; T21/S13 remain open. Follow-up must address and retest early-window/menu timing without weakening owner authentication or hiding the fast-start case.

## P59 timing controls

A same-driver private-desktop comparison against the actual deployed O:\winnt package passed the immediate system-menu witness on both widths; P58d, the previous propagation candidate, failed it on both widths just like P59c. Thus the observed change predates the additional P59 style hooks, and its exact internal delay has not been isolated. With P59c, an owned variant opening the menu 500 ms after showing the window still produced the native menu on both widths, while a 2.5 second variant produced the classic menu on both. A separate polling variant observed actual `Ntw16sWindowAttached` at 1172 ms (x86) and 781 ms (x64) in one concurrent pair; polling and scheduling may affect timing, and these are samples, not limits. The original immediate test remains unchanged and failing. Earlier language identifying the full P59 initialization alone as the cause is superseded by this P58 comparison. Do not restore early visual attachment in a way that silently reintroduces the separately witnessed child-propagation gap; measure stages and preserve both contracts. No product or published binary was changed by these diagnostic variants.

## P59 late-initialization timing and queue experiment

Temporary diagnostic-only builds reported 81 prepared hooks without measurable delay at millisecond resolution. In one x86 and one x64 owned-target sample, each late patch batch spent approximately 266–328 ms, and the first two batches returned `ERROR_RETRY` (1237) before a third succeeded; the measured initialization therefore took roughly 0.9 seconds. Two target-local workers entered the same initializer and one waited on its lock. `ERROR_BAD_EXE_FORMAT` (193) from the opposite-bitness manager in these logs is the expected policy rejection, not an injection success or a claim that the target is a 16-bit program. These timings use instrumented temporary binaries and may be affected by tracing.

A temporary manager change serialized admissions for a given process while retaining parallelism across different processes. In one four-way comparison it did not make either immediate menu pass; the separately measured attach times were 1282 ms x86 and 1078 ms x64. These small concurrent samples do not prove the change has no performance benefit in general, but they do not justify its added scheduling complexity. The experiment was removed from production source; existing uncommitted P59 changes remain. The exact cause of thread-set changes, the 126 system-hook registration and the early-menu gap remain open. No candidate was published.

## P59 thread-origin diagnostic

A temporary private-desktop diagnostic recorded the specific thread IDs absent from the first guard snapshot, then queried their start addresses after the suspended threads had been resumed. In one x86 sample the first two new threads started in `combase.dll` and `ntdll.dll`; in one x64 sample the same module pattern appeared. These module identities alone do not establish which component requested the threads or whether they are application, operating-system, or project initiated. The successful later snapshots included 20 and 18 non-worker threads respectively. The diagnostic used the native thread-information entry point only in a disposable probe and did not change the product or relax the retry check.

Microsoft documents `TH32CS_SNAPTHREAD` as system-wide even with a process ID: https://learn.microsoft.com/en-us/windows/win32/api/tlhelp32/nf-tlhelp32-createtoolhelp32snapshot . Passing a PID alone is therefore not a valid optimization. A faster target-specific mechanism, if considered, still needs a documented compatibility boundary, comparison against the existing guard, churn and failure tests; neither faster enumeration nor two snapshots proves absence of threads created after verification. No new binary was published.

## P59 process-snapshot comparison prototype

A scratch-only C probe compared the current Toolhelp system thread snapshot with documented process snapshotting (`PssCaptureSnapshot` with `PSS_CAPTURE_THREADS`, `PssWalkSnapshot` and `PSS_WALK_THREADS`; https://learn.microsoft.com/en-us/windows/win32/api/processsnapshot/nf-processsnapshot-psscapturesnapshot and https://learn.microsoft.com/en-us/windows/win32/api/processsnapshot/ne-processsnapshot-pss_walk_information_class). On this host with the same outside-sandbox process view as the global tests, five x86 and five x64 stable-own-process comparisons matched all five thread IDs; Toolhelp samples took 78–110 ms and PSS 0–16 ms at `GetTickCount64` resolution. An additional PSS snapshot while three owned idle threads were explicitly suspended succeeded at both widths and the threads were resumed and joined. Five-thread counts include runtime-created threads; timings are a small sample, not a bound. Sandbox-only comparisons returned zero milliseconds for both, so that environment is not representative of actual global enumeration.

This demonstrates a possible faster enumeration, not a safe replacement. The tests did not exercise PSS with another thread suspended while holding allocator, loader or system locks, churn, failure injection, module unload, Windows versions, the actual patch guard or first-menu/propagation behavior. The post-verification new-thread race remains. No PSS calls or additional dependency have been added to production, and no binary was published. The scratch probe remains under build/t21-s13 for follow-up.

## PSS late-guard integration and heap-lock negative

A temporary PSS-backed guard adapter retained the two snapshots and existing suspension/context checks. Its x86 and x64 `core-late-guard` probes passed injected suspend failure, new-thread retry and busy-address rejection. In private-desktop immediate-menu tests the first x86 run failed while the first x64 run passed; three subsequent x86 and three x64 runs passed. A separate pair measured attachment at 906 ms x86 and 203 ms x64. These small mixed results do not prove deterministic first-window handling; the target-thread fallback is still asynchronous. No original fast test was relaxed.

A separate bounded own-process negative suspended a thread while it held the process heap lock. On x64 the original PSS prototype timed out; stage markers showed capture had returned but `PssWalkMarkerCreate` had not. The test driver terminated only its own timed-out process. Pre-creating the PSS walk marker before suspending the lock owner allowed the limited test to complete on both widths, including capture, walk and free. This does not prove safety for loader locks, other heaps, allocation during snapshot capture/walking, or a thread created after verification. The original PSS-backed adapter creates its marker after suspension and is unsafe; it must not be merged or published. The temporary source and product binaries remain scratch only, and production source was restored. A redesigned preallocated guard and adversarial tests are necessary before product consideration.

## P59 real system-hook path control

Using an already registered S12 scratch candidate path solely as a controlled test location, all four P59c clean-candidate files were temporarily substituted together, their hashes checked, and four native tests run on owned private desktops. The controller emitted no `global x64 hook unavailable` message, confirming the tested path did not take that fallback. Immediate system-menu witnesses failed on both x86 and x64 (`blue=0`); separate global propagation probes passed on both widths, observing the matching DLL in child and grandchild and an attached root. Consequently the first-menu delay is not solely a 126/fallback effect: the current system-hook callback also defers attachment while its asynchronous initializer runs. These are four bounded tests, not proof of all system-hook semantics.

The same controlled path and tests were repeated with the scratch-only PSS guard candidate: both immediate-menu and propagation probes passed on each width in this single run. That candidate remains unsafe because its walk-marker creation can deadlock under the heap-lock negative recorded above; the results do not authorize deployment or establish deterministic coverage. Both comparisons restored all four original S12 files in `finally` and verified their original SHA-256 values. The deployed O:\winnt path and assets were not changed. All raw case and restore logs remain in build/t21-s13/real-hook-p59 and real-hook-pss.

## PSS preallocated-guard prototype

A scratch-only guard implementation now creates its verification walk marker before suspending any target thread, disposes the first snapshot before suspension, and defers verification snapshot cleanup until after resuming the threads. Existing x86/x64 guard tests passed suspend-failure, changed-thread-set (1237), busy-instruction (170) and cleanup witnesses. A new separate owned-process test arranges for a thread to acquire the process heap immediately before suspension; the full guard begin/end and thread join succeeded on both widths. This test creates a controlled heap-lock window and does not prove safety with arbitrary loader, CRT, third-party or kernel locks, nor that PSS itself never allocates or blocks internally.

At the already registered private test path the scratch candidate passed one run each of x86/x64 immediate system menu and propagation; all four original test-package files were restored and hash-verified. On the actual error-126 fallback, one immediate system-menu run per width passed, and the existing lifecycle probe passed two start/stop cycles for both widths, including active and stopped child propagation and subsequent windows. These bounded successes do not erase the earlier immediate-menu failure, prove complete first-window coverage, or resolve the post-second-snapshot thread-creation race. Error handling, failed PSS cleanup, retention after resume errors, thread-ID reuse, lock interactions, and true cross-version compatibility are not yet covered. The implementation and all binaries remain under build/t21-s13; none have been adopted into product or deployed.

## PSS snapshot cleanup ownership prototype

A scratch-only injected-error negative demonstrated that the previous PSS guard discarded a snapshot handle after a failed `PssFreeSnapshot`: both widths returned error 5 and the test independently recovered the orphan (`orphan=1`), proving the error path was not safe. A revised scratch guard stores both snapshot objects in an explicit owner associated with the guard, retains failed handles, and permits cleanup to be retried. On x86/x64, two injected successive snapshot-free failures returned error 5 with the owner retained; a later cleanup succeeded and cleared the owner. A second case injected failure during cleanup after a successful suspension and verified threads were resumed, the owner retained, and a later retry succeeded. The existing busy/suspension/thread-churn checks and owned heap-lock test also passed in this scratch build. The injected negative and positive tests are in build/t21-s13 and use only owned processes.

This is not integrated: existing `late_patch.c` and `late_process.c` currently detect incomplete guard cleanup through `guard.threads` only, so a production owner field would also require modifying and testing those callers, failure retention, persistent failures and teardown. The PSS first-window nondeterminism, other lock types and post-verification new-thread race remain. No production code or published binaries were changed by this prototype.

## PSS ownership integration controls

Scratch copies of the real late-patch batch and late-process initialization paths were adapted to recognize the prototype guard's pending snapshot owner in addition to thread handles. On both widths a batch fault test injected two snapshot-free failures: the first install returned error 5 and retained ownership, a second install was rejected with 87, cleanup retried successfully, and discarding a patch with a pending owner returned 5023. The same binaries then passed the existing two-function conflict, protection restoration and successful patch checks with their owned worker joined.

A separate self-contained test compiled the actual late-process initializer with a synthetic two-entry registration adapter. Both widths returned 5 on first initialization, retained the snapshot owner, returned the same sticky error on repeat, and cleaned up via an explicit test-only hook. An x64 negative control compiled the same initializer without its pending-owner check: the test failed as intended, reporting repeat 87 instead of sticky 5. The explicit cleanup hook is diagnostic only; this is not a production cleanup policy. These scratch-only probes show the required caller contracts, but do not yet demonstrate persistent failure behavior, automatic recovery, safe snapshot calls under all locks, or the post-verification thread-creation race. No source or binary in the delivered product was changed or published in this step.

## Main-worktree PSS candidate and clean-package results

The preallocated PSS snapshot guard and explicit pending-snapshot owner were integrated as uncommitted candidate source, with matching caller checks in the late-patch and late-process paths. The formal x86/x64 guard tests now include a suspended heap-lock owner and failed snapshot-free/retry cases; both widths' guard and late-batch CTests passed. Source receipts and original-resource identity passed. A same-source clean dual-width package built and passed ABI/PE checks. A focused actual-package run of 39 cases passed with zero skips, including both global menus, lifecycle, late dialogs, all eight A/W cross-width propagation combinations and normal launch propagation. At the already registered private test path, one actual system-global-hook menu and child/grandchild propagation run per width passed; all four historic files were restored and their original hashes verified.

The fuller actual-package run failed the existing global-threads resource threshold twice: once at jobs=4 (x64 GDI 22 to 25) and once serially (22 to 26), with unchanged handle counts. Subsequent alternating two-run comparisons of the P59 baseline and this candidate all passed with x64 GDI 22 to 24; this does not explain the failures or exclude a leak. An additional repeated clean-package fast-menu test under the actual 126 fallback failed 2 of 5 x86 runs and passed all 5 x64 runs. Thus faster enumeration improves some observed samples but does not close first-window timing. The candidate is not released or committed, and T21/S13 remain open. PSS cleanup after persistent failure is retained by the guard, but the initializer currently fails closed with a sticky error; automatic recovery and other lock conditions are unproven. Original resource identity remains byte-exact.

## Creation-event timing and owned Winmine follow-up

A scratch x86/x64 private-desktop C probe registered an out-of-context `EVENT_OBJECT_CREATE` through `EVENT_OBJECT_SHOW` observer for its own newly created child, then the child immediately showed a normal top-level window. In one sample per width, create and show callbacks arrived within the same `GetTickCount64` tick; in x86 the creation callback saw the window hidden, whereas in x64 it already saw it visible. The callback is asynchronous and this small sample cannot prove a general timing bound. It does show that simply observing creation earlier does not guarantee the manager can initialize before the first interaction. The product's visible-window filtering and discovery policy were not weakened.

The clean PSS candidate was also exercised against an owned private-desktop instance of `D:\programs\misc\winmine.exe`: the app was launched first, global mode started using the candidate path and the fallback after 126, and Help > About was opened via its actual command 593. `PrintWindow` captured a classic title on the `#32770` About dialog at build/t21-s13/winmine-about-pss-candidate.bmp; the app remained alive when global mode stopped and that later run reported 11 start and 11 stop notifications with zero failures. An earlier run closed the app before stopping global and reported two failed or timed-out notifications; it is retained as a cleanup-order observation, not counted as the zero-failure run. Neither sample proves complete visual restoration, real mouse input, all About controls, or that the user's interactive-session report is fixed. No deployment or unrelated process intervention occurred.

## PSS thread identity hardening

The PSS candidate now checks that an opened thread handle has the snapshot's process ID, thread ID and creation time and is still alive, before it can be suspended; the verification snapshot repeats the identity check against the retained handle. This closes the identified case in which a thread ID could be reused by a different process between the first snapshot and `OpenThread`. A scratch probe confirmed that `PSS_CAPTURE_THREADS` creation times matched `GetThreadTimes` for two own threads in one sample at each width on this host. The formal guard test now rejects synthetic wrong PID, wrong TID and stale creation time while accepting the matching thread; both widths passed the existing guard and batch tests. Failure is conservative (`ERROR_RETRY`); Windows versions that omit or change the timestamp have not been tested. This does not prevent a new thread from starting after verification, and it does not solve the first-menu timing or GDI observations. The previous clean package predates this source change and cannot be published as the current source. No new clean build or deployment has been performed for this revision.

## PSS fallback stage timings

Temporary owned-target diagnostic builds measured the present candidate under the 126 fallback. In three x86 delayed-menu samples the actual PSS late initialization completed in one batch and took 0–16 ms at millisecond resolution; attachment was observed after 172–234 ms. Two additional samples recorded window show, PSS initializer start and attachment: initializer start was approximately 94 ms after show, and attachment was observed 171–188 ms after show. A phase trace from another owned run showed discovery shortly after show, target initialization and hook ready at approximately 94 ms, and the manager's first synchronous window-control attempt returning `ERROR_TIMEOUT` (1460) about 109 ms after ready; the window later attached. This is a single traced observation, not proof of the timeout cause or a universal latency bound. The x64 manager's `ERROR_BAD_EXE_FORMAT` (193) for that x86 target is the expected opposite-bitness rejection.

A separate scratch timing adaptation of the existing remote resolver test measured both LoadLibrary and worker-export resolution at 0 ms resolution in three own-process x86 and three x64 samples; its bad PID/name, missing image and corrupt-export negative checks passed. Removing the duplicate resolver or identity verification is not justified by these observations. Temporary instrumentation was removed and the ordinary incremental product outputs rebuilt. The first-menu race and occasional resource-count failures remain unresolved; no candidate was published.

## Target-side window notification timing

A temporary x86 target-side trace measured the control hook and `ntw16s_global_window_apply` on a private desktop while using the PSS candidate and error-126 fallback. In three owned delayed-menu samples, the initial control callback was observed roughly 109–141 ms after the target began showing its window; the first window attachment took approximately 31–62 ms, and a subsequent duplicate attachment check returned within the same timer tick. Each sample eventually showed the classic menu. These are instrumented `GetTickCount64` observations on an intentionally delayed menu and do not reproduce or explain the prior 100 ms `SendMessageTimeoutW` failure. They show that a repeated `ntw16s_remote_global_thread` resolver is not the only relevant work, and do not justify changing timeouts, skipping attachment or suppressing a necessary notification.

`SendMessageTimeoutW` success currently confirms message delivery but `global_attach.c` ignores the target window's return value, so it should not be described as proof of a successful attachment; independent HWND and visual witnesses remain necessary. The diagnostic-only changes were restored and the normal incremental x86 output rebuilt. No source behavior or published binaries changed in this step.

## Attached-window acknowledgement feasibility

A scratch-only native private-desktop prototype tested a separate query delivered to a target window's installed subclass after a thread-hook control message. In both widths, the control message was delivered before attachment; the query returned zero before attachment and a fixed test marker after `SetWindowSubclass`, then returned zero for a wrong test cookie, inactive test session and removed subclass. The first attempt used an unsupported common-controls initialization flag and exited before exercising the mechanism; the corrected run passed on both widths. It does not use or modify the project attacher or real session policy, does not prove rendering, and does not prevent an application from coincidentally or intentionally returning the same marker. It also does not settle reentrancy, in-flight removal, unsupported windows, control/query timeouts, session authentication or retry scheduling. In particular, an unsupported first discovered window must not monopolize all manager workers for the full retry budget. No acknowledgement protocol has been added to the product, and control-message delivery must still not be reported as proof of attachment.

## Actual attacher acknowledgement negative

A temporary prototype added a session-cookie-checked query to the project's real global window subclass and made the existing attacher require the query marker after an APPLY message. In private self-owned x86/x64 `core-global-attach` runs, supported parent and child attachment, click, removal and cleanup passed. A second owned test used a layered top-level window that the existing global hook intentionally declines. The unmodified attacher reported `advance=0` while the target's own `Ntw16sWindowAttached` query reported `attached=0` on both widths; the acknowledgement prototype reported `advance=21` (`ERROR_NOT_READY`) and `attached=0`. Thus the previous concern is a reproduced false-success defect, not just an inference. The negative baseline exited 14 as intended and the acknowledgement variant passed.

The query marker is not a security proof against a target that deliberately returns it, and the prototype does not yet handle reentrancy or all session races. More importantly, directly returning `ERROR_NOT_READY` is insufficient for the current manager: the target-local hook thread may already be active, `ntw16s_global_attach_close` can return busy, and the failed-entry path may not retry a later eligible window on the same GUI thread. A correction must keep worker lifetime and thread-hook status separate from per-HWND attachment outcome, avoid five-second worker starvation on unsupported windows, and verify a later eligible window. The temporary changes to `global_window.c` and `global_attach.c` were restored byte-for-byte and normal incremental DLL/fixture outputs rebuilt. The ACK protocol has not been merged or published.

## Unsupported first window followed by an eligible window

The scratch acknowledgement variant was tested with a self-owned layered first window that the hook intentionally does not style, followed by a regular top-level window created on the same GUI thread. The actual attacher returned `ERROR_NOT_READY` (21) for the first HWND, while the installed thread hook automatically attached the later HWND on both widths; a stopping-session control removed the later attachment and both target and worker exited. This verifies the intended no-special-casing behavior in that bounded sequence.

A separate x86/x64 fixture exercised the actual manager with the same acknowledgement variant and sequence. It recorded `failed=1 active=0` for the initial unsupported HWND, yet the later same-thread window attached, was removed, and the manager and owned target cleaned up. Thus an already installed thread worker can continue doing useful work even while the existing manager entry is classified FAILED. The fixture used an explicit stopping-session remove message; it is not a full run16s desktop sweep or proof for all window types. The remaining issue is accurate, bounded manager bookkeeping and retry behavior, not proof that the worker was immediately removed. The ACK protocol and these fixtures are scratch-only; no production change or deployment occurred in this step.

## Main-worktree acknowledgement and manager state candidate

The formerly scratch-only acknowledgement was added to the uncommitted main-worktree candidate: after an APPLY control is delivered, the attacher asks the global window subclass for a session-cookie-checked attachment marker and returns `ERROR_NOT_READY` if absent. The target subclass returns the marker only while its global wrapper and underlying window attachment are live and the session is active. The marker is an internal diagnostic acknowledgement, not a security proof against a target deliberately impersonating it, and does not prove correct pixels. The manager now preserves an already running thread worker when the first HWND is not attached, reports its active-thread and failed-window facts separately, and scans existing windows on that thread. No user-specific or ordinary-launch exclusion was added.

A new x86/x64 manager fixture covers an unsupported layered first window with a regular same-thread window either already present or created later; it verifies the first parent stays unstyled, the normal child can still attach, the later/previously existing eligible top-level attaches and is removed on stop, and valid/wrong/inactive cookie queries. Incremental CTests passed all four functional scenarios. One concurrent x86 preexisting run passed its function markers but the private-desktop driver reported a retained desktop; its separate rerun passed, and no own run16s process was visible afterwards. This failure is not erased and its holder is unknown. A concurrent x64 existing busy-manager test also failed once (parent attached, child absent) and its isolated rerun passed; causality remains unknown. The new tests are registered for actual-package execution with fixture-DLL hash verification. Source and original-resource gates pass. No clean package, deployment or commit has been made for this new revision; earlier GDI and first-menu observations remain open.

## P59 stage package, deployment and verification

The current main-worktree candidate was built cleanly from one source snapshot with both MinGW widths. Source receipts, byte-exact original-resource checks, governance self-test and PE/ABI checks passed. The actual four-file package passed 43 focused cases with no skips, including acknowledgement, unsupported first window and earlier/later eligible same-thread windows, existing/new global windows, lifecycle, late dialogs, ordinary launch and eight A/W cross-width propagation combinations. The established global-threads resource test passed in one subsequent standalone run (x64 GDI 22 to 24; handles unchanged). The old GDI 22 to 25/26 failures and x64 busy-window/private-desktop cleanup failures above remain unexplained; one pass does not erase them.

At an already registered private test path, one x86 and one x64 genuine system-global-hook menu test and one propagation test per width passed; all four historical files were restored and hash-verified. The actual 126 fallback path passed five subsequent immediate x86 menu samples; the prior 2/5 failures remain evidence of a race. The candidate was published from the tested package to assets root and O:\winnt, and all eight destination hashes matched: demo `1584130ddfa754a0e99d10b4f66d04167f4137fab9ae4e794de3037040636a9c`, x86 DLL `87e9d009bb9e67aa59232f60374ee87172a967b1dd22f5aa51af644a8014696c`, x64 DLL `4767e9a1ce5b4feb91b2c8b6f696d38cbc1370b57fa69ca12277be5f35bd5099`, launcher `4521dd00cc07b66f9f04aac95598faeeaa9ef7d008d6a9f36f2255f896964a5f`. An own private-desktop test of the deployed launcher passed existing/new dual-width windows, input and stop (`restore=8`); the 126 fallback was observed. The deployed launcher also styled the About title of an owned previously launched `D:\programs\misc\winmine.exe` instance; stop reported no failed notifications and the game remained alive. Screenshot build/t21-s13/winmine-about-ack-deployed.png is task scratch. These results do not establish all window classes, user-session appearance, physical input or perfect restoration.

The PSS guard speeds enumeration but cannot prevent threads starting after its second snapshot. Process detours and pinned DLLs remain after global stop, session-bound hooks pass through, and ordinary-started processes may still style their future windows using their own policy. Unsupported/elevated/protected/UWP and user-session boundaries remain as documented. The attached marker distinguishes this project's live wrapper from ordinary message delivery in tested cases, but is not an anti-spoof security proof and does not prove pixels. T21/S13 remain open. This is a stage delivery, not a T completion claim.

Scope accounting for this stage: `git diff --numstat -- CMakeLists.txt src tests tools` plus the new 94-line unsupported-window test gives 418 added, 130 removed, net +288 text lines before documentation and binary assets. Search used `rg -n "ntw16s_attach_.*_using|guard\\.threads|guard->threads|NTW16S_GLOBAL_CONTROL|ntw16s_global_attach_advance" src tests`; ordinary and late registration share the adapters, PSS snapshot ownership is checked alongside retained guard threads in the late patch and late-process paths, and the global attach control and manager now separately report a delivered-but-unattached window. Existing launch paths and renderers remain owned by the existing components. The source-receipt and original-resource checks passed; no original glyph asset was replaced.

P59 was pushed as `72ff492e116dca466c83c7163bc873bb407ee05e`. A subsequent `git fetch origin main` returned the same commit; `git show FETCH_HEAD:assets/...` was hashed for all four files and matched the tested package and both deployed locations. The fetched `global_attach.c`, `late_guard.c` and new unsupported-window test matched the main worktree bytes. This is a delivery verification, not closure of the remaining S13/T21 findings.
# 无窗口传播复核（P59 之后，S13 继续）

- 新增双位宽原生回归：全局启动后创建自有无窗口父进程，再由其创建无窗口子进程；父子分别检查本进程 DLL，子进程没有加载时返回诊断码 41。测试要求两层均加载且子进程返回 0。
- 实际 `O:/winnt` 部署路径采用 126 后备路径时，x86/x64 均为父 `dll=0`、子 `dll=0`、`child_exit=41`；测试按预期失败。
- 在已登记的隔离候选路径使用同一批 P59 四文件，系统全局钩子注册成功、未出现 126，双位宽仍有相同失败。探针仅操作自有实例；临时候选文件已恢复，摘要一致。日志在 `build/t21-s13/real-hook-headless/`。
- 结论仅限上述受控场景：窗口消息钩子不足以发现无窗口进程，不能证明进程传播已全覆盖。无窗口进程先于全局启动的场景尚未独立验证；进程创建事件方案的权限、时序和正确清理亦尚未验证。
- 当前改动仅增加失败回归及其导入摘要，并无产品修复；S13 和 T21 继续开放。
- 事件通知权限探针：当前普通权限下，针对自有唯一映像名订阅 WMI `Win32_ProcessStartTrace`，`Register-WmiEvent` 立即返回 Access denied，未启动目标；独立 C 探针对以本进程 PID 命名的普通实时 ETW 会话调用 `StartTraceW`，返回 5（拒绝访问），没有创建会话或订阅其他进程。这是本机当前令牌的结果，不代表管理员或其他权限配置。微软文档说明普通 ETW 会话控制的权限条件；未更改权限、组或安全设置。两个探针均未验证事件时延，也未成为产品依赖。
- 启动前无窗口目标的可行性对照：自建隔离桌面的自有 x86/x64 进程先启动并等待，随后启动实际 `O:/winnt/run16s.exe --global`；自有驱动显式调用现有 `ntw16s_global_load_begin/finish` 和 `ntw16s_global_start_begin/ready`，以目标主线程 ID 接入。双宽均 `load=0 start=0`，父进程 `dll=1`，实际创建的子进程 `dll=1 child_exit=0`。驱动只处理自有目标，管理器正常停止。初次受限执行无法创建私有桌面（5），提升沙箱权限后双宽通过；未提升目标进程。此实验没有验证启动时枚举及策略过滤、线程/进程竞态或退出后传播关闭，更不代表产品现已自动处理无窗口进程。原型位于 `build/t21-s13/headless-existing-worker.c`。
- 在主工作树增量实现无 HWND 线程的策略入口，复用原用户 SID、会话、位宽、映像排除及进程/线程出生时间检查，并检查线程桌面名称；窗口入口仍要求 HWND。自有私有桌面双宽 `core-global-policy` 包含正常目标、错误位宽、伪造线程出生时间及另一个私有桌面被拒绝，均通过。测试目标线程仍有窗口，尚需独立无窗口测试；仅比较桌面名称对不同 window station 同名桌面的区分尚未证明，不能视为完整安全边界。新入口尚无生产调用、无产品包或发布。来源摘要校验通过。
- 后续将桌面名称比较改为 `CompareObjectHandles` 比较实际桌面对象，获取导出必须来自 kernelbase：错误尝试 kernel32 导致自有目标被拒绝，已修正；双宽同桌面及不同桌面回归通过。跨 window station 同名桌面负例尝试创建自有 window station，子进程返回 12，双宽明确 `POLICY STATION SKIP`，未覆盖该场景。增加 `ntw16s_global_attach_begin_thread` 复用现有异步加载和线程启动，绕过窗口控制消息；当前仅双宽编译通过，尚无真实接口测试、自动枚举或产品调用。来源摘要校验通过。
- 续测 `build/t21-s13/headless-existing-worker.c`：双宽私有桌面先启动自有无窗口目标，再启动现有 `O:/winnt/run16s.exe --global`，测试驱动链接主工作树的新 `ntw16s_global_attach_begin_thread/advance`，实际 DLL 为已发布路径；以事件同步目标创建子进程、管理器退出、目标再创建子进程。两种位宽均 `attach=0`，活动时子进程 `dll=1 exit=0`，停止后子进程 `dll=0 exit=41`，目标 `exit=0`。当前仍为探针主动接入，非产品自动枚举；仅验证当前双宽自有目标与正常停止。
- 主工作树增量候选增加启动时线程快照，同桌面、用户、位宽及映像策略校验后为尚未有窗口入口的进程排队，每进程选一个线程；双宽主机无论系统消息钩子是否成功均运行管理器。`build/t21-s13/startup-candidate` 四文件增量候选、自有私有桌面目标先于全局启动的 x86/x64 对照均 `auto_load=1`、活动子进程 DLL=1/退出 0、停止后子进程 DLL=0/退出 41。探针按 DLL 加载后等待 250ms，不能单独证明所有启动时序；真实系统钩子成功分支、完整回归、资源和并发安全、普通启动互动、跨 window station 与新无窗口进程未覆盖。测试版未发布。来源清单校验通过。
- 上述启动前目标验证纳入双宽 `global-headless-existing` 及实际包测试列表；增量候选双宽通过，已发布旧版 x64 负例 `auto_load=0`、驱动退出 1。普通启动双宽目标、子进程及孙进程传播通过；原有窗口双宽各两轮全局启停、活动期传播及停止后不传播通过。正式测试首轮编译被 `-Werror=misleading-indentation` 拒绝，调整清理语句排版后双宽构建通过。未改变增量候选产物或发布，新增用例仍保留 250ms 等待边界。
- 将增量候选四文件临时复制到已登记、可成功注册系统钩子的独立路径，x86/x64 已有无窗口目标自动接入及停止后不传播均通过；用完逐项恢复并校验原有摘要。首次脚本将原生 stderr 误当 PowerShell 终止错误，完成恢复后调整脚本并重试通过。后备路径私有桌面 `global-e2e` 的旧/新窗口各四、恢复八及双宽输入通过（144 DPI）。正式无窗口用例新增目标就绪事件，确保目标先于全局启动；双宽重新构建及复测通过。系统钩子成功分支复测发生在新增就绪事件之前。
- 对上述已登记路径以新增就绪事件的正式探针复测，系统钩子成功分支 x86/x64 均 `before_global_ready=1 auto_load=1`、活动传播和停止后不传播通过，四文件恢复摘要一致。增量候选实际包聚焦回归 `build/t21-package/startup-focused/result.json` 为 `passed-focused`、26 项、零跳过，覆盖双宽策略、管理器、窗口、菜单、普通与全局传播和启动前无窗口目标；新无窗口进程的两项失败回归未列入本次聚焦集合，不能视为通过。双宽干净构建、完整矩阵、真实环境资源风险及发布尚未完成。
- 同一源码快照 `build/t21-package/startup-clean` 的 x86/x64 干净构建、ABI、源与资源自检、四文件包门禁通过；对该包的独立聚焦测试 `clean-source/build/t21-package/startup-focused-clean/result.json` 为 `passed-focused`，26 项、零跳过。活动期间新无窗口进程的已知失败用例未纳入该聚焦集合，仍不得计作通过。尚需审查启动时线程快照的筛选、重复和退出竞态及真实资源影响，未发布。
- 事后审查发现启动快照按 PID 查重时，失败的窗口线程记录会阻止同进程其他线程进入候选，已仅把排队中、工作中或活动记录视作可阻止；该状态迁移边界尚无独立负例。另将 Ctrl+C 处理器注册移至请求事件创建后，以便延长的启动过程也能转入既有退出路径；启动期间实际 Ctrl+C 仍待专门验证。两处修改使前述干净包不再对应主工作树。`startup-audit-candidate` 双宽已有无窗口目标活动/停止传播通过，双宽 manager 原有窗口、新窗口、清理及复用测试通过；未完成新的干净包及发布。PID/线程重用与资源影响仍需审查。
- 私有桌面自有新进程组中，在会话进入 ACTIVE、ready 尚为 0 时发送 `CTRL_BREAK_EVENT`：前一干净候选退出码 3221225786、阶段仍为 ACTIVE，新增处理器的增量候选退出码 0、阶段 STOPPING 且事后宿主不存活。正式 `global-startup-break` 增加结束时读取宿主 PID 的复核，增量候选通过；这是 Ctrl+Break，不代表独立验证了所有 Ctrl+C/控制台关闭时序。已加入包测试和来源清单；新干净包构建进行中。
- 修正后的快照 `build/t21-package/startup-audit-clean` 双宽干净构建、ABI、来源、资源及四文件包检查通过；该包 `clean-source/build/t21-package/startup-audit-test/result.json` 为 `passed-focused`、27 项、零跳过，包含正式启动 Ctrl+Break、双宽已有无窗口传播及停止后不传播。仍未包含运行中新无窗口进程的失败用例，未解决此前列出的线程快照和资源边界，也未发布。
- 扩展聚焦包测试发现 `global-common` 失败：启动新目标时会话 `phase=ACTIVE ready=0`，x64 新进程始终 `mask=0` 且未加载样式 DLL；私有桌面串行复现，已发布版本同场景通过。诊断中等到 `ready=1` 才启动则通过，因此不能把问题归咎于现有控件样式。主工作树在启动线程快照后再次枚举已有窗口并交给同一发现器，属于启动阶段处理；增量 `startup-refresh-candidate` 原测试两次通过，另一次诊断明确 `ready=0` 启动后双宽窗口及控件通过。该对照支持启动发现竞态，尚不足以证明所有时序已消除；此前干净包因源码更新已过期，扩展回归、双宽干净构建及发布待完成。
- 新增量候选的扩展测试 `startup-refresh-extra` 首次在 `global-threads` 报资源超阈：x64 三轮前后句柄 213→213、GDI 22→25。随后串行隔离对照，已发布版和候选版均出现 x64 第一个旧窗口 `state=0`；另一次候选诊断在 `ready=1` 确认目标 DLL 已加载但窗口未附着。临时加入管理器记录后，相同测试三轮通过，x64 句柄 210→210、GDI 22→24；输出 193 的其他位宽目标是管理器检查不匹配，不能据此解释 x64 窗口失败。日志改变时序，尚不能判定根因或修复；临时源码日志已移除，诊断二进制仅在 scratch。不得将此结果当作通过或发布依据。
- 续用自有共享内存记录、无同步日志的隔离诊断：x64 目标 DLL 已加载但两个窗口线程的管理器任务均返回 `ERROR_RETRY`（1237），PSS 双线程快照校验中有此返回路径；这提示同进程并发远程工作线程可能相互干扰，但不单独证明全部 1237 的具体内部触发点。临时诊断源码已移除。主工作树增量改用 64 个分片 SRW 锁，对同 PID 的管理器接入任务串行；`startup-serialized-candidate` 的原多线程三轮启停测试连续两次通过，x64 句柄 209→209、GDI 22→22，原启动竞态控件测试通过。分片碰撞会使不同 PID 顺序执行；饥饿、超时退出及旧 GDI 波动是否同因尚需验证。该候选未干净构建或发布。
- 串行包扩展回归仍有 `global-vb-e2e` 新 x64 目标未附着；将 x64 管理器就绪前移至启动 sweep 及同一发现器的末尾补扫之后，私有 VB 原失败用例通过，但多线程可见测试依然失败。临时加入最多三次可取消重试后，多线程可见用例一次通过，串行包回归随后仍失败；自有共享内存探针记录同一线程首次及三次重试均为 1237，故简单重试并未解决问题，已撤销重试及临时诊断桩。该轮所有候选均未发布；PSS 返回 1237 的精确条件、同位宽工作互扰以外因素以及最终窗口发现覆盖仍需证据。
- 自有内存诊断将 1237 定位到快照线程身份的存活/时间查询阶段。独立 PSS 对照创建并等待一个线程退出但保持其句柄：快照仍包含它，`Flags=1`（`PSS_THREAD_FLAGS_TERMINATED`）、线程句柄已 signaled。新 `core-late-guard` 负例在旧 guard 稳定输出 `begin=1237 end=1237`；主工作树改为首/第二快照均跳过标记 terminated 的项，并保留所有活线程 PID、TID、出生时间和并发新线程检查，双宽输出 `PSS EXITED HANDLE ... begin=0 end=0`，既有变化检测 1237、忙区域 170、部分失败释放和 heap 对照均通过。无诊断桩 `guard-terminated-candidate` 双宽多线程三轮启停一次通过，x64 句柄 218→218、GDI 22→22；尚未完成扩展包回归或证明所有历史失败均同因。临时目标内存诊断桩已移除，未发布。
- 无诊断桩增量候选实际包扩展回归 `build/t21-package/guard-terminated-extra/result.json` 为 `passed-focused`、26 项、零跳过，包含先前失败的 VB、双宽多线程/可见窗口、宿主故障、两次全局生命周期、启动期间 Ctrl+Break 和启动前无窗口传播。该批串行执行，运行中新无窗口进程用例未覆盖。新的双宽干净包构建中，尚未发布。
- `build/t21-package/guard-terminated-clean/result.json` 同源码双宽干净构建与四文件包门禁通过；快照内双宽 `core-late-guard` 输出 `PSS EXITED HANDLE ... begin=0 end=0`，活线程变化仍为 1237、忙区域为 170、清理及暂停恢复通过。同一干净包 `clean-source/build/t21-package/guard-clean-test/result.json` 为 `passed-focused`、40 项、零跳过，串行覆盖 VB、普通和全局传播、多个窗口线程、故障退出、菜单及启动前无窗口进程。运行中新无窗口进程失败用例未列入，不能视为通过；尚未发布。
- P61 阶段包已发布至 assets 根目录及 `O:/winnt`，四文件摘要分别为 demo `ab5f71fdec0463c6c272abe5cc061e9a5f7c88b93618aed88d09f06ad0e6a640`、32 位 DLL `c4622f56b6c4da397cf780f9c0c60448fda28e4b3378e5ba9ceb5daf3dfcae5e`、64 位 DLL `2f2887863e7af41e1a8d35b7d5db229d2075c8d5a00db9e972891e191083ad10`、启动器 `9f88a95ad73be7eb1c2fcb254cb3007550e53d6a6ca28cce828e6ebb273cc6a9`；两处与测试包一致。发布前识别当前 Codex 和系统/设备进程持有旧 DLL，为保持本会话可继续执行，将旧映像移至 `build/t21-package/guard-stage-retire/` 并保留逐文件备份，未结束这些进程；它们可能继续使用旧映像。发布记录 `build/t21-package/guard-stage-publish/result.json`，提交/推送/远端复核待完成。S13/T21 不收口，运行中新无窗口进程、跨 station 对照、GDI 波动等仍未解决。
- P61 提交 `96f8fe14bcac21967c0d660662b2fce86598ce34` 推送后重新 fetch，读取实际远端四个资产并与 `O:/winnt` 四文件逐项计算 SHA-256，一致。直接使用实际发布路径在私有桌面运行双宽已有无窗口目标测试，启动前就绪、自动接入、活动传播和停止后不传播均通过；系统 x64 全局消息钩子仍返回 126，验证使用后备路径。此复核未测试运行中新无窗口目标，T21/S13 仍开放。
- 使用普通桌面用户环境重新验证进程事件：WMI `Win32_ProcessStartTrace` 订阅仍拒绝访问，独立实时 ETW 会话仍返回 5；这排除了先前沙箱为唯一原因，但不代表有授权的其他环境不可用。实际 P61 中 `global-relay` 在会话 `ready=1` 后立刻创建的 x64 无窗口父/子目标仍 `dll=0 exit=41`；自有诊断等待已存在的 relay 父进程 DLL 加载及额外 250ms 后，目标两层 `dll=1 exit=0`，表明启动就绪早于父进程异步接入可造成漏挂，DLL 存在本身不是初始化证明。主工作树候选跟踪排队及工作数，双位宽主机启动时各有界等待最多 2 秒并泵消息，再宣布就绪；x64 超时向 stderr 报告、x86 超时向调试输出报告，超时不能承诺全覆盖。私有桌面原回归在 x64 父→x86/x64 子及 x86 父→x86/x64 子四组合均通过，启动 Ctrl+Break 和多线程三轮启停通过。未覆盖运行中新出现且其创建者未接入的纯无窗口进程；未干净构建或发布该候选。
- `build/t21-package/startup-drain-focus/result.json` 对增量候选串行实际包验证 `passed-focused`、44 项、零跳过，包括四种父子位宽无窗口立即传播、已有目标、VB、菜单、多线程及生命周期。x86 relay 已纳入构建并注册对应包用例；尚无专门受控饥饿/超时测试，仍需同源码双宽干净包及验证，不代表运行时未接入父进程的覆盖。
- 双宽 `core-global-manager` 加入自有延迟窗口握手：窗口实际进入控制消息延迟后调用 20ms idle 等待，要求返回 `ERROR_TIMEOUT`；随后在 3 秒内要求工作完成，原有附着、交互、退出和清理仍通过。双宽增量构建及私有桌面测试均通过；该对照未证明持续新事件导致的长期饥饿或生产超时日志路径。
- P63 同源码快照 `build/t21-package/startup-drain-clean` 双宽干净构建、ABI、四文件包检查及 `clean-source/build/t21-package/startup-drain-clean-test/result.json` 44 项串行聚焦回归通过、零跳过；双宽管理器延迟对照分别记录 timeout=1460、completed=0。已发布 assets 和 O:/winnt：demo `f98af2f66a5a27d6dbabd0e5e15b651ee8a7dce4da5d3d3d3fb1b0fe036c9796`、32 DLL `e7028a176603fab68ae69b9a0095cbeea7eebb181d702311f40154d1cbca47dc`、64 DLL `88bee1e3564cc3ed2760bd37330740e46c6dc6f09773d7eb3d4c89e0f5b40969`、启动器 `0ea11ca014f899f183c9efdc0e3d12d6a7116514fa5299cfaf78e30455bef8eb`；部署脚本备份并核对两处摘要一致。实际 `O:/winnt` 在私有桌面双宽同位宽父子立即传播通过，x64 全局钩子仍回退 126；跨位宽由干净包验证。提交推送复核待完成，未接入父进程创建的纯无窗口目标、持续事件饥饿及 x86 超时仅调试输出边界保留。
- P63 提交 `c890ef93617ea506e873426461fd108fe01b86fa` 推送后重新 fetch，读取实际远端四个资产并与 `O:/winnt` 四文件逐项计算 SHA-256，全部一致；实际发布路径的 x86→x86 与 x64→x64 无窗口立即传播分别通过。现有旧映像仍可能在长期运行的已加载进程中有效。S13/T21 继续开放。
- 对 P63 使用两个自建私有桌面：先在 sibling 桌面启动自有 broker，随后启动目标桌面的全局模式；broker 确认未加载样式 DLL，再向目标桌面创建自有子进程。x86/x64 的 `headless` 均父子 DLL=0、子退出 41；同一来源的 `normal` 双宽均根窗口实际附着并成功向子孙传播，broker 保持 DLL=0，最终干净退出。这证明当前可在窗口出现后接入，并不意味着先于窗口创建的无窗口过程已传播。新增 `tests/integration/global_cross_origin.c` 和两项有窗口包回归，双宽直接运行通过；负例保留在可选探针而未标记为成功。未改产品二进制，P65 尚未提交和完成包回归。
- 新增的双宽有窗口用例和必选基础检查以 P63 原干净包执行 10 项，`build/t21-package/cross-origin-test-retry/result.json` 为 `passed-focused`、零跳过。首次误传增量构建目录，被包/构建身份门禁以 `mixed build` 拒绝；重试使用原干净产品构建目录，并仅将从当前测试源码编译的新测试驱动复制到该目录，产品文件未变。assets 与 O:/winnt 四文件摘要仍一致；本次测试/文档改动没有重新发布二进制，尚未提交。
- P65 `5aede8dbe7dc04930f9706e647b390b255151379` 已推送并重新 fetch；读取实际远端新增测试源码并核对本地 SHA-256 一致，远端四个资产与 O:/winnt 四文件摘要一致。该交付仅测试和文档，没有更改产品二进制；T21/S13 保持开放。
- P63 实际部署路径使用双私有桌面的未接入 broker 对照，双宽目标首次普通菜单 `draws=3 idle=1`、系统菜单 `selected=1 blue=1` 均通过，broker 明确 `loaded=0`；x64 使用 126 后备路径。这避免把先被注入的测试父进程传播误认为未接入来源覆盖。四个新用例加入包测试，与既有两项跨桌面窗口用例和必选基础检查合计 14 项串行实际包回归 `build/t21-package/cross-origin-menus/result.json` 为 `passed-focused`、零跳过。它们不证明所有窗口的首帧或所有交互均无竞态；仅测试和文档变更，二进制未更改，提交及远端复核待完成。
- P67 `f8e118040a264f86e69a98fe4df9e8bba2047b0a` 已推送并重新 fetch；实际远端测试脚本摘要与本地一致，远端四文件与 O:/winnt 摘要一致。此后 T21/S13 继续开放。
- P63 实际 `O:/winnt/run16s.exe --global` 使用自有私有桌面复测用户指定的 `D:/programs/misc/winmine.exe`：目标先启动，探针读取菜单并向自有窗口发送 Help → About 命令（593），`PrintWindow` 成功捕获 470×414 的 #32770 对话框，截图中可见项目经典标题；关闭对话框及全局模式后目标进程和窗口仍存活，再由探针关闭。日志 `WINMINE ABOUT FOUND` 和 `WINMINE AFTER STOP alive=1 window=1`，x64 系统钩子 126 时使用后备。截图 `build/t21-s13/winmine-about-p63-recheck.bmp`；该单次验证未证明像素完美恢复、所有控件或其他应用。
# P70: unified stop for adopted ordinary processes

User changed the contract: ordinary `run16s` targets that global mode adopts
also stop styling after the global controller exits. Ordinary targets not
adopted by global mode retain their existing behavior. Late initialization
publishes the session cookie before atomically enabling the shared session
policy; the installer emits a cookie only when the captured policy requires
one, so an in-flight adoption cannot form an invalid payload. Existing hooks
remain in place and are gated by the session; physical DLL or trampoline
unloading is not claimed. A concurrent launch at the adoption or stop boundary
and targets that cannot be reached remain limitations.

The revised private-desktop test waits until each ordinary target has actually
been adopted. Against deployed P63 it failed on the first post-stop window
(`index=2 state=1`); the updated incremental candidate passed. The test covers
ordinary x86 and x64 targets started before and after global, their existing
windows, four post-stop windows and four post-stop child processes. On the
clean candidate the 16 selected package cases passed with zero skips in 33.8 s,
including DPI-unaware, lifecycle, common controls and propagation. The
published `O:/winnt` version separately passed the same private-desktop mixed
sequence at 144 DPI. The x64 desktop hook still returned 126 in that session;
the target-thread fallback was used.

Same-snapshot GCC/Ninja/ccache x86/x64 clean build, ABI, import and byte-exact
resource checks passed. Candidate SHA-256: demo
`3c5c0c6e54d641acc66487676ffd2029c6372698fd5a7843ceb55ee576f15709`;
ntw16s32 `60c854ae1e56dd2ef3b9db1d516f6ba66d6f48660aed7bda6696e72dbf7af36a`;
ntw16s64 `dc051b3ced8814cb48e87b8f0471de39ca878426ff51d21d74e1ce214a5ff889`;
run16s `85563bb3529f7c0eadc98f61fb308f2acc477f34ecacb1361812086c8971642b`.
The publication tool verified identical bytes in assets and O:/winnt. The
full test matrix and physical input are not covered. Initial clean build was
rejected by a scratch screenshot link and fixed; the next build was stopped
after the in-flight payload consistency issue was identified, then rebuilt.

# P72: ordinary process repeated global sessions

Extended the existing lifecycle probe with an ordinary `run16s --wait`
target. Each of x86 and x64 starts styled, is adopted by two consecutive
global sessions, creates a styled window and injected child during each
session, and after each stop has its original window restored and creates an
unstyled window and child without the DLL. Both widths and cycles passed
against the deployed O:/winnt P70 package on a private desktop. The registered
actual-package focused run passed 10 cases without skips in 28.34 seconds;
it reused the P70 clean build and package with only the newly built probe
temporarily substituted, then restored the prior probe. No product source or
binary changed and no new binary publication was needed. The x64 hook again
returned 126 and used the target-thread fallback. No general claim is made
about excluded processes, failed attachments or termination races.

# P73: GUI discovery after an uninstrumented headless intermediary

On two private desktops an uninstrumented broker created a headless process
after the global controller reported ready. The headless process verified its
own DLL was absent, created a GUI process on the global desktop, and waited.
The GUI process observed a styled root window, then created a child and
grandchild with the appropriate DLL loaded. Both x86 and x64 passed against
the deployed P70 `O:/winnt` package; the deployed x64 system hook returned
126 and the target-thread fallback performed discovery. This proves the
tested GUI chain can recover after an uninstrumented headless intermediary;
it does not mean that intermediary or other headless processes are injected.
The registered actual-package focused run passed 10 cases with zero skips in
25.2 seconds. It reused the P70 clean product package and temporarily replaced
only the two test probes, restoring them afterward. No product bytes changed.

# P74: remove unmarked hidden ordinary windows at global stop

An owned private-desktop mixed-mode probe created a hidden styled window in
each of four ordinary x86/x64 targets after global activation. On P70, the
stop sweep notified 52 windows without timeout, but the first hidden window
remained attached (`hidden_restore_failed index=2 state=1`). The cause was
that shutdown removal required a global window marker; a hidden window can
have the ordinary attachment without ever receiving that marker. The common
same-thread removal now removes an existing project attachment even without a
global marker. The control-message session and process checks are unchanged.
Only targets reached by the stop sweep and their hook can be restored.

The updated x86/x64 clean GCC/Ninja/ccache package passed 17 focused actual
package tests with zero skips in 35.42 seconds, including both DPI modes,
direct same-thread removal, mixed ordinary/global launch, hidden window
restoration, ordinary restart and propagation. The actual deployed O:/winnt
package also passed the mixed private-desktop test at 144 DPI with all four
hidden windows restored. The session's x64 system hook returned 126 and used
the target-thread fallback. The original resource and source gates passed.
The complete matrix, physical input and inaccessible or hung windows were
not covered. DLLs and trampolines may remain pinned after stop.

Published SHA-256: demo `8b5c7cad7c89cb517a3c1c24a53c35028fb8f92bff75f967fcb14f33c257151b`;
ntw16s32 `2ecb2062bd1d89eca78ed668f19303c4e86c75bd3d65c8d4a61232a0a6aaeb71`;
ntw16s64 `638b42135a9b82b5e3411e17342b9ed5e299aa7b1e4ea58dfa2b5d110eab54f0`;
run16s `dbc175525b0babdfc0b8ad1d013f26e091d6d100dc9db4540629559335bf608d`.
The publication tool checked all four files in both assets and O:/winnt.

# P76: show restored hidden windows after stop

Extended the P74 mixed-mode test to show all four previously hidden ordinary
windows after global shutdown, update them, and verify they remain unstyled.
The published O:/winnt package passed the private-desktop x86/x64 probe at
144 DPI. The P74 clean package passed 10 focused cases without skips in 28.35
seconds, including the ordinary 96-DPI-unaware variant. For that run only the
three test executables were temporarily replaced and then restored; the four
product files were unchanged. This checks window attachment after actual show,
not physical input or a pixel-perfect comparison.

# P77: short-lived first window investigation

Inspection of `discovered` found that events for the same thread while an
entry is working are not separately queued, and a failed entry receives at
most two attempts. Whether that creates a real missed-window condition depends
on the timing of the existing apply and thread-window scan; it is not an
established defect. An owned scratch C probe launched by an uninstrumented
broker after global readiness showed one top-level window for 20 ms, destroyed
it, and immediately created a second visible window on the same thread. One
x86 and one x64 run of the deployed P74 observed the second window attached.
This limited sample does not resolve the theoretical race, and no product
change or binary release was made. Probe source and executables are in
`build/t21-s13/flap-investigation/`.

The x86 probe itself and broker returned success, but the private-desktop
driver returned 8 because `Win16S-Dialog-17968-219947781` was still open after
its cleanup check. Later read-only checks still opened that desktop, found no
top-level windows there, and found no running `probe32`, `probe64`,
`global-cross-origin`, `run16s`, or `rundll32` processes in the process
inventory. The remaining desktop reference and its cause were not identified;
do not treat this x86 run as a clean resource-lifetime pass. The x64 driver's
cleanup check passed. No unrelated process was terminated.

# P78: identify the retained private desktop handle

The P77 desktop remained openable; the sibling broker desktop was absent.
The existing desktop-driver self-test confirmed that an extra handle keeps a
desktop open and that closing that handle permits its removal. A read-only
Toolhelp thread probe checked 790 queryable current-session thread desktops
and found none bound to the P77 desktop. A separate native handle inventory
identified the desktop object type from its own handle, then temporarily
duplicated and closed only accessible desktop handles in the same session.
Of 145 candidate handles, 139 could be duplicated; one matching handle belonged
to PID 30064, `C:/Windows/System32/InputMethod/CHS/ChsIME.exe`. Its reported
creation was 16:40:03, shortly after probe compilation at 16:39:51. A module
query did not find `ntw16s32.dll` or `ntw16s64.dll` in that process. The input
method process was not terminated or modified. The timing alone does not prove
what started it or that the six inaccessible handles were unrelated. The
desktop could not be safely removed without controlling its other owner.
The two read-only probe sources remain in `build/t21-s13/flap-investigation/`.
No product code or binaries changed.

# P79: bounded loader-lock race experiment

An own x64 process on a private desktop deliberately loaded a scratch DLL
whose test-only DllMain held the loader lock for 1500 ms. It attempted to show
and replace a window during that period while the deployed global controller
used its 126 target-thread fallback. In two runs the second window attached;
the measured run showed `ShowWindow` returned after 1547 ms, the first window
was destroyed after 1813 ms, and the second was created after 1860 ms with the
loader thread already exited. Thus the experiment did not establish the
intended concurrent window replacement and does not rule out the manager's
in-flight event/retry race. Both drivers reported clean private-desktop exit.
The test-only source is in `build/t21-s13/loader-race/`; no product code or
binary was changed, and no test-only loader-lock behavior is shipped.

# P80: deployed P74 real Minesweeper check

On a private desktop, the owned `D:/programs/misc/winmine.exe` instance was
started before the actual deployed `O:/winnt/run16s.exe --global`. The native
probe invoked the application's Help → About command (593). `PrintWindow`
captured the visible dialog, whose title uses the classic style. After the
dialog and global controller were closed, the target process and its main
window were still alive; the owned target was then closed and the private
desktop driver exited zero. The x64 desktop hook returned 126 and used the
existing fallback. The single screenshot is at
`build/t21-s13/winmine-about-p80-recheck.bmp`; its capture clips portions of
the client text, and it does not establish all control visuals, input or
pixel-perfect restoration. No product code or binary changed.

# P81: cross-directory ordinary/global instance conflict

An owned private-desktop variant of the mixed-mode probe launched ordinary
targets via `assets/run16s.exe` and the global controller via the published
`O:/winnt/run16s.exe`. The target executables and probe were owned, and both
package copies had equal SHA-256 for each DLL. During the session, each x86
and x64 ordinary target reported two loaded modules: the assets DLL and the
O:/winnt DLL; each module's `Ntw16sWindowAttached` reported 1 for the same
window. After the global controller stopped, the first ordinary target still
reported styled (`restore_failed index=2 state=1`), although the sweep itself
reported no send failures. All owned processes and the private desktop were
cleaned up by the driver. The same failure occurred in both the basic and
instrumented probe runs; the test stopped at the first failed restore, so it
does not independently report post-stop state for all four ordinary targets.

The current manager loads its own directory's DLL, while session owner
validation also ties a module to its neighboring run16s path. Consequently a
second DLL instance has separate attachment and process-policy state, and
removing its attachment does not remove the earlier instance's. A repair must
coordinate or reuse compatible already-loaded instances, validate identity,
retain per-thread safety and handle different versions or inaccessible files;
blindly unloading or suppressing a DLL is not justified. Scratch probe sources
and binaries are in `build/t21-s13/cross-install/`. No product change was made.

# P82: reuse compatible cross-directory modules

The global attacher now selects an already loaded same-named module before
loading another copy. For a different path it compares the two regular files
byte-for-byte, checks the mapped image's machine, timestamp, image size and PE
checksum against the controller's loaded module, and compares the first 16
bytes of the internal worker entry. The target-side owner check retains the
run16s basename and session-owner checks, accepting a different directory only
when the DLL files match. The system-hook callback defers to the first loaded
same-named module. The byte comparison is exact for the files; the mapped-image
checks are bounded checks, not a cryptographic comparison of the entire mapped
image. Incompatible or inaccessible instances are not deliberately reused or
patched a second time; their ordinary behavior can remain after global stop.

The new formal cross-directory regression failed on the deployed P74 package
at `restore_failed index=2 state=1`. The candidate passed on both the x64
system-hook path at a registered scratch path and the deployed-path 126
fallback, covering x86/x64 ordinary targets started before and after global,
existing/hidden/new windows and child processes. The final deployed assets and
O:/winnt paths passed the same probe. A module inventory in the fallback run
showed one assets DLL per ordinary target rather than two. A separate owned
negative used P74 mapped DLLs whose disk paths were replaced by identical new
candidate files: the targets were not adopted (`normal_not_adopted index=2`);
the test intentionally returns failure when adoption does not happen and did
not independently complete all target checks. The differing-version probe also
showed only the original modules on the fallback path. This is not exhaustive
coverage of malicious images, concurrent module load/unload or every Windows
loader ordering.

Clean GCC/Ninja/ccache x86/x64 build, ABI, original-resource and source gates
passed. Actual package focused serial regression: 24 cases, no skips, 67.57 s.
The first two-job run failed `global-attach-x64` with 1460 despite observing an
attached window; the same case passed in the serial batch. The cause of that
timeout is not established and the failed run remains recorded. No physical
input or full matrix is claimed. Four tested files were published and verified
at assets root and O:/winnt: demo `28f548346e758bded774c7178281ee4dd9ef7a3e9b68f16984d1b86fd689947c`;
ntw16s32 `368da50a1cb0cf93961ab7c384a1f1f4939cf2df42f7ff5c3997f8ce3cbd9c2d`;
ntw16s64 `f5bdcbacf1dc9c99b85c2cb637e130f4eb76b83f97203290b1988793fa0edf97`;
run16s `736e245e1e0ee9ad32d81722caff925e7f9c6f6a568c4c77e8edcf8bdc5f44b1`.

# P84: deployed P82 Minesweeper regression

An owned `D:/programs/misc/winmine.exe` instance was started on a private
desktop before the actual P82 `O:/winnt/run16s.exe --global`. The native probe
opened Help → About (command 593); the captured dialog title was visibly in
the classic style. After closing the dialog and global controller, the target
process and main window remained alive. The owned target and private desktop
then closed successfully. The x64 system hook returned 126 and the existing
target-thread fallback was used. The screenshot at
`build/t21-s13/winmine-about-p84-recheck.bmp` clips part of the client area;
it does not prove all control visuals, physical input or pixel-perfect
restoration. No product file changed.

# P85: same-path upgrade compatibility investigation

An owned private-desktop probe started x86/x64 ordinary targets using the P74
package, moved their mapped DLL files and launcher aside, placed P82 files at
the same original paths, and started the P82 global controller from that path.
The probe completed mixed ordinary/global attachment, hidden and visible
window restoration, and post-stop child checks. This shows that this P74/P82
combination worked in the observed sample even though the mapped image and the
new on-disk image have different PE timestamps and checksums; it does not prove
all historical combinations are compatible.

A provisional change rejected every same-path mapped image whose PE header
differed from the current file. A test that modified the timestamp on a copied
fixture detected that change on both widths (old implementation returned 0;
candidate returned 1306). However, the criterion would also reject the working
P74/P82 case. It was therefore removed from the worktree along with its test
and package registration, and the clean build was stopped before completion.
Scratch sources remain under `build/t21-s13/stale-overrestrict/` and
`build/t21-s13/cross-install/`. No product binary was changed or published.
The public ABI is version 3, but that alone is not evidence that all versions
implement the same internal global worker/session contract. Any further repair
must preserve demonstrably compatible old instances while rejecting genuinely
incompatible ones; the compatibility boundary is not yet established.

# P86: real Minesweeper started after global readiness

The deployed P82 controller ran on a private desktop. An owned broker on a
second private desktop, which reported no project DLL loaded, launched a
headless probe on the global desktop after readiness. That probe also reported
no project DLL, then launched an owned `D:/programs/misc/winmine.exe`, waited
for its visible main window and invoked Help → About (command 593). The
captured dialog title visibly used the classic style. The owned target, broker,
controller and private-desktop driver exited normally. The x64 system hook
returned 126 and the target-thread fallback was used.

The screenshot `build/t21-s13/winmine-new-p86.bmp` clips part of the client
area. This one sample establishes the observed title for a new application
from an uninstrumented source, not complete control styling, first-frame
timing, physical input, or coverage of every application. No product file was
changed.

# P87: existing process-event channels

Read-only checks of the current machine found the
`Microsoft-Windows-Kernel-Process/Analytic` channel present but disabled.
Reading Security event 4688 returned access denied; querying the Process
Creation audit policy returned 0x522 (required privilege not held). An initial
sandboxed channel query also returned access denied; the non-sandboxed
ordinary-token check produced the channel configuration above. No channel,
audit policy, security group or system setting was changed. This supplements
the earlier denied ordinary-token WMI and ETW session probes; it does not prove
that every possible event source is unavailable or that a privileged design
would satisfy the timing and lifecycle requirements.

Microsoft documents that controlling ETW sessions and consuming real-time ETW
events require suitable privileges or group membership: [session control](https://learn.microsoft.com/en-us/windows/win32/etw/controlling-event-tracing-sessions)
and [real-time consumption](https://learn.microsoft.com/windows/win32/api/evntrace/nf-evntrace-opentracefromrealtimelogger).
The current product was not changed. In particular, newly created headless
processes from a parent without hooks remain outside the demonstrated coverage;
P86 separately demonstrates later GUI discovery for the tested chain.

# P88: correct the published global-mode contract

The root README and current architecture still claimed that ordinary run16s
targets retain styling and propagation after a global session stops. That was
superseded by the user's unified-stop request and contradicted the P70/P74
behavior. Both now state that an ordinary target adopted by the global session
is stopped with it; ordinary targets that never joined remain independent.
The README also records the unsupported headless-parent chain, asynchronous
fallback first frame, incompatible module and existing shell exclusions. The
launcher component README now makes clear that its no-global-hook statement
describes ordinary launch. This documentation change has no product binary
change; it does not expand tested coverage.

