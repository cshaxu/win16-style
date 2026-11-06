# M0 T22 S1：UWP Win3.x 样式可行性结论

本记录是研究交付，不是 UWP 产品实现或新二进制发布。S1 保持开放。
结论基于当前源码、官方接口、原始项目源码和本项目 T10 原生研究证据。

## 结论矩阵

| 目标 | 结论 | 证据/限制 |
| --- | --- | --- |
| 普通 UWP 无需主动配合而接入 XAML 树 | 已有本机原型证明 | T10 计算器 TAP 加载、SetSite、QI、订阅成功；不是换肤证明 |
| XAML 颜色、边框、模板等运行时适配 | 有可实施接口，具体 Win3.x 模板尚需原型 | IVisualTreeService 的属性与树接口；UWPSpy 实际工具路线 |
| 经典 System 位图字体完整保真 | 未证明 | XAML 字体管线不等于 GDI，FontFamily=System 不构成证据 |
| UWP 官方标题 API 换完整 Win3.x 系统按钮 | 该 API 路线不满足 | 系统保留 caption controls 及其输入区 |
| 复用现有 renderer 改共享宿主标题 | 条件性候选，未验证 | 原型返回1444；当前策略还排除宿主，不能仅删除检查 |
| 任意 UWP、多个框架、所有弹层完整一致 | 不承诺 | 需要逐框架及控件验证，不能以计算器外推 |
| 不修改应用原文件、系统注册表 | 设计可保持 | 运行时状态；诊断 DLL 可读性可能需要同一产物的临时副本 |
| 完全无磁盘写入、退出必然物理卸载 | 不承诺 | AppContainer 可读路径、COM 回调及在途调用需要管理 |

因此推荐“两条适配路径、共用外观资源”：XAML 客户区走 Diagnostics/TAP，
标题边框按实际 HWND 所有者单独适配。完整 Win3.x 窗口效果尚未证实，
不能宣称现有全局钩子打开 UWP 名单即可完成，也不能断言整个方向不可行。

## 当前断点与版本

src/lib-win32/process-global/policy.c 的 inspect 拒绝 TokenIsAppContainer；
src/ntw16s-dll/global/policy.c 的 ntw16s_global_image_allowed 排除
ApplicationFrameHost。现有 HWND renderer 的线程约束仍应保留。
这些是当前产品覆盖范围，不是 Windows 禁止任何 UWP 诊断接入的证据。

2026-11-05 宿主只读 Get-AppxPackage 复核：计算器11.2607.0.0 x64，
记事本11.2607.14.0 x64，与历史证据版本一致。首次参数数组错误、沙箱查询
访问拒绝均未产生有效结果；改为逐包宿主只读查询后成功。
本轮未启动应用或重新测量 HWND/绘制，不能将版本相同当作重新运行通过。
计算器 AppContainer/System XAML+WinUI2.8、记事本 FullTrust/WindowsAppRuntime2
的历史分类及摘要见[原生研究证据](../etc/evidence/xaml-research.json)。

## 最小实现设计

1. run16s 保留协调职责：识别 AUMID、实际 PID/创建时间、机器类型、框架及
   CoreWindow/宿主归属。激活可能复用实例，不能以父子关系或包名代替身份。
   全局模式和指定启动共用该分类器，普通 Win32 路径不变。
2. ntw16s32/64 内部增加 C ABI COM 工厂、IObjectWithSite 和 XAML 适配器。
   TAP 实现不需要独立第五产品；拟增 DllGetClassObject 等标准入口须审计
   导出名、调用约定和既有 ordinal，不注册 COM 到系统注册表。
3. Diagnostics 显式初始化与现有启动载荷隔离。被动 LoadLibrary 仍被动；
   不重新执行启动 bootstrap，不在 DllMain 初始化 GUI、等待或创建线程。
   多 UI 线程分别调度，不能跨线程直接操作树或 HWND。
4. 首个控件使用真实 Button 对象，仅替换视觉属性/模板，不用新的假按钮
   遮盖旧按钮。保留 Command、绑定、自动化 peer、焦点、按下/禁用状态；
   监听树变化处理新页面及弹层。先验证一类按钮，不发明全控件框架。
5. 原始资源只读复用。XAML 绘制需要适配原图资源和 DPI，不把现有 GDI
   subclass 机械套到 XAML 对象。字体单列验收，暂不改已有经典字体策略。
6. DLL 路径使用绝对路径、位宽和摘要核对，先测试 AppContainer 可读性；
   若包目录不可读，考虑有明确所有权和清理契约的同字节临时副本。
   这是临时存储写入，必须明确报告，不能称为完全零写入；不扩大目录ACL。

## 标题栏决策

对历史计算器，标题按钮属于共享 ApplicationFrameHost；只在 CalculatorApp
内部操作 XAML 无法接管另一个进程的 HWND。SetWindowSubclass 也不支持跨线程。
官方 ExtendViewIntoTitleBar 可自定义标题内容，但保留系统按钮，不能满足完整目标。

若要完整效果，下一原型必须验证“仅匹配目标的宿主窗口适配”：验证 HWND 与
目标 CoreWindow 的关联，在所属线程安装适配器，阻止原框架重绘覆盖，并保留
拖动、尺寸、系统菜单和最小化/最大化/恢复语义。宿主适配不应启用进程传播，
更不能把宿主中其他应用窗口一起换肤。这是对旧共享宿主排除边界的调整，
本次仅给出设计，不默默开放或注入用户既有宿主。

若宿主不允许可靠定向适配，则完整标题要求仍未满足；不能以客户区成功收口
完整换肤目标。外部覆盖窗口/重设父窗口仅为备选，不推荐：尚无输入、DPI、
Z序、任务栏及生命周期保持证据。不建议系统文件、注册表或主题签名修改。

## 停用与恢复

每项修改保存原始属性来源与本层写入值；停止订阅并在对应 UI 线程移除本层
修改。若应用期间更新属性，不盲目写回旧值覆盖应用；冲突明确报告。
保存值不足以还原绑定/资源表达式，模板替换须验证原对象和表达式恢复。
宿主保存窗口自身状态并仅移除自己的 subclass。停用成功不等于物理卸载；
有在途回调时保留模块引用，不能以 Sleep 后 FreeLibrary 证明安全卸载。

## 有限原型与验收顺序

| 阶段 | 必须观察 | 失败后的结论 |
| --- | --- | --- |
| A 接入 | 自有计算器实例、身份/SHA、TAP、树回调和 UI 线程；重复连接与关闭 | 只报实际错误，不以模块存在算成功 |
| B 单按钮 | 正常显示截图、经典边框、点击/键盘/禁用语义、恢复 | 树读取成功不能替代绘制与交互 |
| C 标题宿主 | 实际目标标题 Win3.x 按钮、系统菜单、移动/缩放/最小化恢复；同宿主另一个窗口不受影响 | 不通过则完整窗口支持仍未证明 |
| D 生命周期 | 新页面/弹层、多UI线程、重复启停、退出/复用、实际DPI、资源与回调释放 | 列明未支持项，不扩大应用覆盖 |
| E 交付 | Win32 双宽回归、同批四文件包测试、assets 与 O:/winnt 发布 | 不发布混合包 |

先 A/B 后 C，C 是完整窗口目标的硬门槛；不把它降级为可忽略项。
全局接入放在单目标闭环之后，避免同时调试发现、绘制、宿主和恢复。
使用纯 C 原生探针、自有实例、有限等待；不依赖 computer-use 或用户整套手测。
无真实绘制原型之前，以上是可执行设计与条件性可行性结论，不是产品支持声明。

## 来源审查

- [微软 Diagnostics 入口](https://learn.microsoft.com/en-us/windows/win32/api/xamlom/nf-xamlom-initializexamldiagnosticsex)：指定 PID、TAP DLL 和 COM CLSID。
- [微软树接口](https://learn.microsoft.com/en-us/windows/win32/api/xamlom/nn-xamlom-ivisualtreeservice)：属性与树变化接口。
- [微软 UWP 标题](https://learn.microsoft.com/en-us/windows/uwp/ui-input/title-bar)：系统按钮区域保留。
- [微软 subclass 线程约束](https://learn.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-setwindowsubclass)。
- [UWPSpy TAP](https://github.com/m417z/UWPSpy/blob/main/UWPSpy/tap.cpp)及[观察器](https://github.com/m417z/UWPSpy/blob/main/UWPSpy/visualtreewatcher.cpp)：只读研究，GPL-3.0；未复制 C++ 实现。其按 UI 线程观察、异步订阅和自卸载做法是研究材料，不证明本项目的恢复安全。

本轮不改生产代码、资源、注册表、目标文件或系统安全设置，不增加产品文件，
不修改兄弟仓库。旧 C ABI 双宽成功只证明本地接口原型，不等于新模板完整可编译运行。

## T22 S1 runtime frame prototype — 2026-11-05

This section supersedes the earlier requirement to prove a XAML button first.
Only the exterior frame is in scope. The user explicitly permits runtime injection,
including the selected application's shared frame host; target files, registry and
system persistent configuration must remain untouched. No production code or ABI
has changed in this experiment.

### Target identity and presentation

The x64 native COM activator returned own CalculatorApp PID 24216 (18:34:21 local),
from Microsoft.WindowsCalculator_11.2607.0.0_x64__8wekyb3d8bbwe. Its CoreWindow
0x0f90062e / TID 16460 was a top-level HWND with DWMWA_CLOAKED=2. Thus GA_ROOT
and parent traversal alone did not identify the displayed frame.

The new ApplicationFrameWindow 0x06870694 / TID 26232 belonged to the pre-existing
ApplicationFrameHost PID 10220. Its window property PKEY_AppUserModel_ID was
Microsoft.WindowsCalculator_8wekyb3d8bbwe!App, cloak=0, DPI=144. The pre-existing
Realtek Audio Console frame had a different AUMID and was not targeted. AUMID alone
is not a unique window identity: a second activation later returned PID 37344
(18:51:10), producing a second calculator frame 0x0af40d8c / TID 24064. Product
selection must also account for activation generation/new-frame identity and
ambiguous or reused windows; title text alone is insufficient.

Both own calculator processes subsequently had all 44 observed threads in the
Suspended wait reason. CoAllowSetForegroundWindow and ActivateApplication returned
S_OK, but normal ShowWindowAsync/SetForegroundWindow did not make the second
CoreWindow respond: foreground request returned 0, GetForegroundWindow was NULL.
Input desktop was Default and WTS reported console session 4 Active. The cause of
the suspension is not established; it must not be reported as proven lock-screen,
permission, injection or renderer failure. No thread was forcibly resumed.

### Finite tests and evidence

Scratch: build/t22-package/s1-window-audit, owned by S1. Native C17 x64 GCC probes
were compiled in hidden background processes with warnings as errors. These are
research probes, not additional product files or a product release.

| Test | Actual result | Limit |
| --- | --- | --- |
| Private-desktop independent frame model | PASS: root attached, child title/button unstyled, button pixel hash unchanged, CreateProcessW entry bytes unchanged, root rectangle and WM_PRINT hash restored | Artificial ordinary HWND, not a UWP compatibility pass |
| Exact-thread WH_CALLWNDPROC on own calculator frame | Attach and remove returned success/error 0; observer UnhookWindowsHookEx succeeded | No process propagation installed; success is not visual proof |
| Normal screen BitBlt | Returned success but all-black image | Failed visual evidence |
| External PrintWindow | Captured calculator splash/composited native buttons; no confirmed Win3.x title | App had not reached normal UI; initial driver also lacked DPI awareness and its capture was cropped |
| Temporarily remove WS_EX_NOREDIRECTIONBITMAP on selected frame | Attach/remove succeeded; PrintWindow still did not demonstrate desired title | Flag alone is not a verified fix; original flag and saved rectangle restored by the prototype |
| Explicit external WM_PRINT | All-black output | Not treated as renderer success |
| In-target UI-thread WM_PRINT, pixels returned via pagefile-backed mapping | Real Win3.x title text, system-menu box, min/max glyphs and border visible in inthread-styled.bmp | Offscreen renderer output only; client area black; not normal on-screen composition or interaction proof |

Private model hashes: native/restored 6725876898959568569; styled
17996494764306535664. A first model run failed its visible-child assertion because
the hidden test launcher supplied STARTF_USESHOWWINDOW/SW_HIDE; consuming that first
ShowWindow override and explicitly showing the private model corrected the fixture.
This was not a product visibility fix.

The in-target probe called existing Ntw16sWindowAttach/Ntw16sWindowRemove on the
exact host HWND/GUI thread. No bootstrap, recursive child attachment, XAML change,
CreateProcess hook or global target-policy change was made. It used the existing
O:/repos.hobby/win16style/assets/ntw16s64.dll with SHA-256
DDFF9678928EA2F67DDDBD64912C930EF8599E3F61AF34FA44A7219AFFD3C3C1.
Module enumeration confirmed that absolute path in the host; temporary observer
DLLs were absent after their tests. Three passive renderer LoadLibrary references were retained across the independent
observer runs (the redirection variant borrowed the existing module). Removal is
not physical module unloading; reference cleanup is still outstanding.

Do not claim exact real-host geometry restoration: the first, DPI-unaware driver
left the frame width at 512 rather than its initial 502 physical pixels. Subsequent
local state restoration and the private model do not retroactively prove that first
host cycle reversible. This remains a required adapter regression case.

### Implementation conclusion and next gate

Runtime-only, frame-specific entry is demonstrated. Existing Win3.x rendering can
produce the intended exterior on the actual host UI thread. It is therefore not
necessary to invent another title renderer or to begin with internal XAML controls.
However, neither module loading nor the offscreen picture proves normal UWP frame
presentation. A plain call to the existing attachment API is not yet a validated
complete solution.

Next: obtain a normally running own calculator; verify visible composition, native
title-child coexistence, hit tests/buttons, movement/resize/minimize/restore, and
exact DPI-aware cleanup. Only then decide whether a small host-specific presentation
adapter is necessary. Do not preemptively add overlays, reparenting, private host
protocols or another renderer. Real application activation/reuse needs explicit
identity handling. Product integration, x86 coverage, normal-display screenshot,
interaction and full delivery remain uncompleted; S1/T22 stay open.

References for the read-only identity and rendering distinction:
- https://learn.microsoft.com/en-us/windows/win32/properties/props-system-appusermodel-id
- https://learn.microsoft.com/en-us/windows/win32/winmsg/extended-window-styles
- https://learn.microsoft.com/en-us/windows/win32/api/objbase/nf-objbase-coallowsetforegroundwindow

### Follow-up on the normal desktop

After the user returned to the desktop, both earlier own calculator processes had
already exited; the shared host PID 10220 retained its original start time. A fresh
activation (foreground grant S_OK) returned own PID 27640 at 19:05:07 local. This
instance responded normally, and its CoreWindow 0x093c0d4c became a child of frame
0x0d0d059a (host TID 9612), cloak=0. Native screen capture showed the fully rendered
calculator at 144 DPI. The earlier disconnected CoreWindow topology is therefore
not a stable ready-window model; product code must wait for normal association.

On this ready target, direct attachment still failed the visual goal: native screen
capture showed the modern title/content shifted beneath an empty new nonclient
band. Native geometry restoration was false. A second experiment temporarily
cleared WS_EX_NOREDIRECTIONBITMAP and explicitly restored the saved outer rectangle;
geometry then matched, but the visible title was still modern. Clearing that flag
alone is not a solution. Both experiments removed their window attachment and
observer hook; no production policy or persistent configuration changed.

A following small presentation prototype uses the existing renderer on an invisible
own model window, and presents its title pixels in a removable layered child over
the selected frame's title area. It does not attach the real frame, move its client
area, change XAML controls or create a separate product renderer. Layer/model
creation and removal succeeded and real outer geometry stayed unchanged. However,
the input desktop had switched to Screen-saver before capture: screen calls failed,
so visible composition is NOT proved. A bounded one-pixel native mouse wake attempt
could not return to Default and skipped without executing the test. It did not
change screen-saver/security settings or interact with authentication UI.

This child-layer test is visual only, not a complete adapter: the 48-pixel title
height is a temporary 144-DPI fixture, and hit testing, command forwarding, active
state, DPI/resize updates and border coverage remain unimplemented. It must not be
shipped or described as working UWP styling on the strength of successful creation.

Automatic approval review rejected a proposed three-reference FreeLibrary cleanup
inside the shared host because uncertain ownership could crash it. That cleanup
was NOT executed. The user subsequently explicitly accepted host-test/cleanup risk;
no retry has yet occurred. Passive research module references remain outstanding,
separate from the already removed observer hooks and window attachments.

### Visible title surface result

The self-contained owned-layer driver now activates only when no calculator exists,
waits at most 10 seconds for its returned PID's CoreWindow to join an uncloaked
ApplicationFrameWindow, marks only that exact HWND, installs a thread-specific
observer, captures before/after/removal, and closes its own application. It refuses
stale HWNDs and existing application instances. No shared host is terminated.

The child-layer variant on own PID 38260 returned success for creation/removal and
screen sampling, but its actual screenshot remained entirely modern: visual test
FAILED despite successful API calls. A separate owned top-level layered title
surface on own PID 6960 / frame 0x0c3c04fa / host thread 32936 then visibly presented
the existing Win3.x title glyphs and caption on the normally running calculator.
The top-level surface has the selected host frame as its owner, uses NOACTIVATE and
TOOLWINDOW, and is created/deleted in the host GUI thread. It does not alter target
files, the registry, internal XAML controls or real-frame geometry.

Evidence: owned-overlay-styled.bmp, SHA-256
0276A5DF8D669E931472AB69DCD53E01A6D68A7DB3454242BF5E8B099E9B06E2,
in the registered scratch. This is an actual screen capture, unlike the earlier
in-thread WM_PRINT. The finite run reported before/styled/restored captures=1,
geometry=1, apply/remove=1 and observer unhook=1. A later process/module inspection
confirmed that all recorded own calculator PIDs had exited and temporary observer
DLLs were absent. The passive product DLL remains mapped.

Visible restoration is not certified pixel-identical: before/restored screenshot
SHA values differ (8C75BB7329821A58337BE76E1CF860C15F6F622F197C80B7CCE7469310248A6D
versus 1CAE68B067632FC276943B18FD6A38290C019BA2E5945213CB7072574C012036).
Focus/activation and normal application animation must be measured, not assumed.

The feasible presentation direction is therefore a small owned frame surface using
existing Win16 drawing, not direct GDI attachment to the composition host and not a
child HWND layer. The current fixture has visible edge/padding defects and a fixed
48-pixel title strip. It is NOT a releasable adapter. Required next implementation:
reuse frame layout/paint/hit-test primitives directly (remove the hidden render-model
fixture), implement geometry/DPI/activation/lifecycle synchronization, native system
commands and menu, full border coverage, and tests of input and recovery. Keep the
client area untouched. Integration into existing ntw16s DLL and run16s app activation
must preserve the three-component/four-file architecture. No product release yet.

### Adapter development (not a delivery)

The new private uwp/frame and frame_layout modules directly use existing frame
painting and fonts; the hidden render-model fixture is no longer needed by this
candidate. They currently build through test targets only, not the product DLL.
The private-desktop fixture covers a client-content hole, posted minimize command,
hide/show, duplicate apply, three removals, owner destruction, and steady GDI/USER
counts after warmup. Layout checks cover 96/144/192 DPI. s1-frame-core4 passed
two tests per architecture (x86/x64), with no skips. It does not prove physical
input, full move/resize/minimize/restore, or actual screen appearance.

The first test run failed because the private worker startup hid the fixture;
explicitly showing it corrected the test. The real host exposed a separate
assumption error: its title child can cover only the right-hand caption buttons.
The adapter now accepts that native band while retaining its client boundary.
This partial-width title case is included in the repeated lifecycle test.

Real own Calculator runs adapter-run4 (PID8488) and adapter-run5 (PID26376) in
the registered window-audit scratch reported apply/remove=1, geometry=1 and
observer unhook=1. Both processes were confirmed exited. Captures failed with
BitBlt error5, including after clipping the negative outer coordinates to the
virtual screen. Session4 was RDP Active; no evidence justifies blaming a locked
desktop or claiming visual success. A subsequent host module check confirmed
the temporary probe DLL absent. Earlier passive renderer references remain
outstanding. System menu styling, real input/DPI/restoration, product integration
and deployment remain unfinished. No new product binaries were published.

The subsequent menu implementation reuses menu_message/frame_state directly.
The private-desktop test checks system-menu owner-draw conversion, nonzero item
measurement, and restoration of native type/data on exit. s1-frame-menu1 passed
two tests per width. s1-frame-product1 linked the adapter into both actual DLLs
and passed three tests per width, including ABI export verification. The added
ordinal38 hook validates a frame/CoreWindow association and operates only on
that GUI thread. A successful installation pins the containing DLL until host
exit; removing the frame adapter does not unload the module. No propagation is
installed in the shared host.

The new development --app AUMID launcher path activates a new process, waits for
its visible frame association, and uses the adjacent x64 DLL. Existing processes
and non-ApplicationFrameWindow frameworks remain explicitly unsupported by this
entry. Compilation/runtime verification of this launcher and final package
delivery are pending. The s1-frame-launch1 all-target build was unnecessarily
broad; subsequent incremental builds should use a dedicated dual-width aggregate.

s1-frame-launch1's monitor failed reading the busy architecture log; its worker
and descendants were confirmed gone before a retry. The uwp-development aggregate
then completed s1-frame-launch2: three tests per width, x64 launcher linked,
no skips. Invalid AUMID input returned Windows error87 and launcher exit10.
This is not yet a successful real-activation launcher test.

product-run1 used the actual x64 product DLL copy in scratch/product1 (SHA256
A2A165EF9CDEAC9982D04E7713C163B7962E98F6F7C41D8A96585C5C28117156).
Its own Calculator PID27056 and host10220/thread39600 reported actual export
apply/remove success, unchanged geometry and coordinator hook removal. Neither
CAPTUREBLT (error5) nor SRCCOPY fallback (error6) captured a screenshot; visual
acceptance remains unproven. This candidate DLL is deliberately pinned for host
lifetime. The product DLL reference is not a lingering installed coordinator
hook and is not claimed physically unloaded.

The launcher production activation function passed launch-real1 (own PID38040),
with a live owned surface, removal and unchanged geometry. The optional native
test is tests/run16s-exe/uwp.c; without --activate it skips explicitly. It refuses
an existing Calculator and cleans only the PID confirmed newly created by
activation. It does not run implicitly in the ordinary test suite.

The frame now observes the selected native title child's layout/lifetime as well
as its owner. motion5/buttons1 passed three cases per architecture, including
two moves, native minimize/maximize/restore, title destruction and owner teardown.
Early resource counts before native animations settled grew; after a bounded
500ms message drain GDI counts were equal and USER sometimes decreased. The
final assertion is no growth after warmup, not identical counts or a universal
leak-freedom claim. One intermediate test compile failed misleading-indentation
and was fixed. All earlier failures remain in registered scratch.

Real button runs real2/3 initially failed: native frame state returned before
the CoreWindow was re-associated, so early removal was rejected with error1400.
Title child identities did not change. After waiting at most1.5s for both native
state and the real CoreWindow/surface association, launch-real4 (PID33052) passed
maximize/restore, minimize, restore visibility, removal and geometry. This uses
product2/ntw16s64.dll SHA5284FB5439A73B6F17D1EE4BF08ED7F4DAD1BE7F5B7BA2AF8DEA39BA6554AEC8.
The own process was subsequently confirmed exited. No physical mouse or screen
appearance is inferred from these message/API checks.

Independent capture-diagnostic, without styling DLLs, had valid DC object types:
memory fill/copy succeeded; screen SRCCOPY failed6, CAPTUREBLT failed5 and screen
GetPixel returned CLR_INVALID. duplicate-diagnostic created a D3D11 device/output
but DXGI DuplicateOutput returned E_ACCESSDENIED(0x80070005). No registry, security
setting or application file was changed. Visual acceptance remains outstanding.

### Desktop recovery: production-adapter visual evidence

用户恢复普通桌面后，独立 capture-diagnostic 的 SRCCOPY/CAPTUREBLT/GetPixel
均成功。本轮 `s1-window-audit/visual2.out` 使用既有 product2 候选 DLL 的
Ntw16sUwpFrameHook，激活自有 Calculator PID 10716，关联宿主 PID 10220 /
TID 31884，实际 DPI 144。应用/撤销回调均返回1，三张真实屏幕截图成功，
窗口几何不变，定向 hook 已撤销，自有进程确认退出，未结束共享宿主。

`visual2-styled.bmp` 显示经典标题/按钮和未修改的计算器客户区；
`visual2-restored.bmp` 显示恢复原生标题。这是已固定 product2 原型的屏幕证据，
不是待发布0.22干净候选的实包验证。最大化时顶部/左右外缘被屏幕裁切，
标题按钮几何还需修正与复验，不能据此宣称视觉验收完成。

`s1-clean2` x64在tray-fixture链接时缺少DwmGetWindowAttribute；其复用启动器
源码但未同步dwmapi库，已补齐uuid/dwmapi，并纳入uwp-development依赖。
`s1-link-fix1` x86构建和ABI/布局通过，但生命周期测试资源计数失败：
iteration1 GDI44/31，USER54/45；没有放宽断言，原因待查。该失败使工作器
未进入x64，故链接修正仍待实编译验证。未启动s1-clean3，不重复完整打包。
### Maximized geometry and resource-control follow-up

The maximized adapter now intersects the native outer rectangle with the target
monitor work area before calculating its surface geometry. This removes invisible
native sizing margins from the classic caption without moving XAML content.
The layout contract and native button probe now use surface-relative coordinates.

Resource diagnostics preserved in s1-window-audit:
- native-resources: current-desktop native window remains GDI9/USER15; it did not
  match the failing private-desktop activation conditions.
- native-private: no adapter or styling DLL, same private desktop and activation,
  GDI31 ->44 ->44 ->42, USER47 ->50 ->50 ->49; after owner destruction GDI22/USER9.
- resource-trace: six adapter cycles plateau GDI42/USER52; after owner destruction
  GDI22/USER10. Diagnostic omits the resource assertion, so is not an acceptance pass.

This establishes that the original 31->44 increase is reproducible without the
adapter; it does not establish the origin of every retained USER object. The test
keeps its original first complete adapter/menu-cycle baseline and strict subsequent
no-growth assertions, but first exercises native private-desktop activation.
s1-frame-clip2 tried a native-only baseline and failed USER52/50; this failure is
preserved, not presented as a pass. s1-frame-clip3 checks the amended test.
### Actual host menu presentation and current incomplete candidate

s1-frame-clip3 and s1-frame-host-menu1/2/3 each passed six core cases across x86/x64.
visual3 confirmed maximized caption glyphs fully visible; geometry/apply/remove
passed. launch-real5 failed because the observed maximize flag preceded surface
geometry; launch-ready-max then passed after waiting for actual clipped surface
bounds as well as CoreWindow association. This waiting is in the test only.

Native SC_KEYMENU on the tested host produced a white popup even without applying
the adapter. menu-trace2 proved its hook was live but received no classic menu
init/measure/draw messages. The new UWP adapter therefore tracks the existing
GetSystemMenu handle using its own surface as menu owner, reusing the existing
menu-message renderer and forwarding chosen WM_SYSCOMMAND to the original frame.
It does not copy menu items or alter client controls.

The tracked #32768 popup (style94800000/ex189) still appeared white on screen,
while menu4-print.bmp showed correct classic rows/border through PrintWindow.
menu4-layered verified that temporarily setting WS_EX_LAYERED and opaque alpha
on that exact popup presented the same rows on screen; its prior ex-style was
restored. Product menu presentation now uses a thread-local hook limited to the
matching HMENU, then restores its added flag and removes the hook at tracking end.
This is runtime-only and does not change shared menu classes or system settings.

visual5/visual6 confirm readable classic rows and successful cancellation/removal,
but their popup position was 0,0 instead of below the title button. This remains
an acceptance failure. s1-frame-host-menu4 restricts the presentation change to the
SWP_SHOWWINDOW notification rather than earlier placement messages; its actual
screen placement still requires verification. Do not claim menu completion.

s1-clean3 completed both clean architectures (350.6s/438.3s) and ABI checks, but
failed candidate version validation: FileVersion strings still said M0 T21 although
numeric version was0.22. The three resource strings are corrected in the worktree.
The clean3 snapshot also predates the menu changes and is not publishable as the
current candidate. No T22 product publication/commit has occurred yet.
Follow-up: host-menu4 passed six core tests, but visual7 was white again at the correct position. The SWP_SHOWWINDOW pre-dispatch filter is therefore insufficient. host-menu5 uses WH_CALLWNDPROCRET after a visible popup's native position handler, preserving coordinates and repainting after enabling layered presentation. Results pending; no menu completion claim. Temporary observation DLL hooks were removed by their drivers; the product candidate DLLs remain pinned to host lifetime by design.

Final observed result this turn: host-menu5 six core cases PASS; visual8 (own Calculator30580, host10220/thread26500,144DPI) still shows a white popup. The external exact-HWND layered prototype succeeded, but the production in-thread timing/identity path is not yet proven. Cancel/remove/geometry still pass. Next diagnostic must inspect actual presenter callback identity and API return values rather than infer success from native tests. No current candidate is declared visually complete or published.


### Presenter breakpoint resolved, final clean candidate underway

presenter-diagnostic: hook=1, seen=1, visible=matched=106433460,
error=0, layered=0. The correct popup was matched but already had USER animation's
WS_EX_LAYERED, so the guard skipped it. The temporary properties and extra hook/TLS
were removed. The adapter now uses the owner WM_ENTERIDLE HWND and disables animation
for this one TrackPopupMenuEx call with TPM_NOANIMATION; system settings are untouched.
References: https://learn.microsoft.com/en-us/windows/win32/dlgbox/wm-enteridle and
https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-trackpopupmenu .

host-menu8: x86/x64 each3 passed, no skips. visual11 real screen shows a complete
classic menu below the title button, own Calculator21380 / host10220 thread38944,
144DPI, apply/remove/geometry all pass. DLL SHA256
7fcadf84a65b8efbd4e2e5be780144084f83ff53ac196dc33dc317bdf6aac8a2.
visual10 was occluded by another foreground window, not valid product evidence;
those screenshots were deleted and the native screenshot driver now checks foreground
identity before capture. No unrelated window was modified.

clean4 starts after receipts and version strings were updated. A documentation
compaction command failed on default Python encoding before changing CURRENT;
this did not modify product inputs. Snapshot froze successfully; CURRENT was then
updated with explicit UTF-8. Final package testing and publication remain pending.

### Window library extraction requested during final build

User requested `lib-win32/win16-window-uwp` for a future parallel
`win2k-window-uwp`. Moved frame/layout implementation and matching tests,
renamed local APIs to `win16_uwp_frame_*`, added a real static-library target;
the DLL links it rather than compiling a duplicate implementation. Product
activation, host selection, callback protocol and module pin remain product-owned.
No product headers are included by the new library. Tests instantiate the library
without process injection; the live activation fixture also links this library.

First incremental attempt failed due to the existing palette API's DLL-import
annotation; using its static-link declaration resolved this. `s1-library2` passed
three tests on each architecture with zero skips. Pre-migration clean4 remains
baseline evidence only, not a publishable candidate after this relocation.
The final snapshot is clean5. Graphics/provenance bytes are unchanged.

### P4 final candidate delivery

Final snapshot `s1-clean5`: x86/x64 clean builds and ABI gates passed. Independent
UWP library tests passed two per architecture, no skips. Actual four-file package
`s1-package1`: 14 focused cases passed in 41.15 seconds, zero skips, including
ordinary injection/propagation, menu cancellation, first display and system menus.
This is not the full matrix. All resource headers/provenance and embedded original
graphics passed the byte-identity checks. Production inputs match the frozen snapshot.

Real same-SHA package test: new Calculator PID11840,144DPI; activation, maximize/
restore, minimize/restore, removal and unchanged geometry all passed. Final screen
probe: ownPID27468,hostPID12040,thread34468,144DPI; before/styled/menu/restored
captures passed, apply/remove returned1, geometry matched and hook removal succeeded.
Screenshot `build/t22-package/s1-window-audit/visual12-menu.bmp` shows the actual
classic title and popup on the visible Calculator; client XAML remains unchanged.

Publication `s1-publish1` succeeded: all four tested files are identical at assets/
and O:/winnt. Previous exact occupant ApplicationFrameHost PID10220 (start
2026-11-03T07:30:25.1227778-08:00) was revalidated, normal close requested, and exit
confirmed under permanent user authorization. No unrelated process was targeted.
Checksums, toolchains and selected cases are in docs/etc/release.json.

Limits remain: explicit --app/new-instance/x64-host only; global, reused targets,
WinUI3 and XAML controls are not implemented. Physical input, multi-monitor DPI,
full system-menu command and enabled-state matrix remain unverified. Native
resize cursors and replacement title-child rebinding are not implemented by this
adapter. Removing it releases owned windows/GDI resources; the containing DLL is
pinned until host exit, not physically unloaded. S1/T22 remain open.

Staged code footprint against P3: src/ +581/-13 (net +568), tests/ +259/-0;
new library implementation is 382 C lines plus 23 header lines. No duplicate old
frame/layout files remain in the DLL directory. Existing rendering and resource
owners are unchanged; adapter state belongs to its GUI-thread instance.
Clean build timings: x86 348.43s, x64 418.92s, Ninja8/ccache; ABI tests both passed.

### P5 remote delivery review

Fetched origin/main at c06ea3edbcb2f3146a278ee9297deab438112997, read and compared
all 33 changed blobs byte-for-byte, and verified SHA256 of all four remote
binaries against the tested package and both deployment directories (eight files).
P5 changes only delivery review records; binaries remain the tested P4 batch.
S1/T22 remain open with the limitations above; no full UWP/global claim.

### P6 menu-state and input work (not yet delivered)

Negative `s1-menu-state-negative1` failed on the published logic: normal Restore
and Maximize both returned0x100 (owner-draw, enabled). The TrackPopup path does not
perform the original window's automatic system-menu state update. Added a bounded
five-command state scope: preserve original flags, add only state-dependent
restrictions, notify original WM_INITMENU, and restore temporary restrictions
while the original window/menu still exist. Application-disabled Close is untouched.
Reference: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getsystemmenu

State tests cover normal/maximized Restore/Maximize/Move/Size, original notification,
restoration and application-disabled Close. `s1-menu-state1/3` each passed three
cases per architecture, no skips. `s1-menu-state2` failed because posted keyboard
messages on the private desktop did not select a real menu command; that invalid
input method was removed, not reported as a product pass.

Real visual16/17/18/19/20: keyboard Up/Up selectsSC_MAXIMIZE(0xf030), Return causes
IsZoomed=1, removal and original geometry restore pass. Earlier End/Up probe
selected Close and the own window exited; hostPID12040 remained alive, not a host
crash. visual13 and21 lacked target foreground and are not visual/input passes.

Cursor code selects native cursor by actual border hit. Temporary diagnostic
confirmed HTLEFT=10 and target GetCursor=IDC_SIZEWE, but visible GetCursorInfo did
not match; this is not a visible-cursor pass. Native no-DLL control initially
failed due to inherited hidden startup/zero movement; explicit visibility and
one-pixel input passed. UWP one-pixel run showed pointer at54,281 instead of requested
324,528; subsequent absolute-input run lacked foreground. Visible cursor remains
unverified. All temporary T22Cursor properties were removed before final build.
Scratch is the registered s1-window-audit; input targets were self-created and
closed, pointer restored, no shared host termination in this series.

### Native move/resize verification on user-confirmed idle desktop

User confirmed availability for the limited native-input test. visual22 used the
non-diagnostic final incremental DLL (`s1-menu-state4`, x64 SHA256
 a1548da1c7efe919db448f8c64c756951105f73292172b2a75ed50286b66e2ba), ownCalculator40740,
host12040/TID14508,144DPI. Real SendInput title drag moved root from
(323,123,825,933) to (343,143,845,953); left-edge resize expanded it to
(303,123,825,933). Both were restored. Real menu Up/Up selectedSC_MAXIMIZE, Return
maximized; adaptation removal and original geometry restoration passed, hook removed.
Before/styled/menu/restored captures succeeded. Probe closed its own calculator.

Visible resize cursor did NOT pass: pointer confirmed at(325,528) on the surface,
but GetCursorInfo returned arrow0x10003 instead ofSIZEWE0x10011. This is now an
observed limitation, not only missing coverage. Earlier diagnostic proved the
host thread could set SIZEWE; global visible cursor does not follow in this case.
Native no-DLL control passed after explicit ShowWindow override of hidden startup
and nonzero input movement. No claim of full cursor correctness is made.

Final incremental s1-menu-state4 passed x86/x64 each3 tests with zero skips.
No T22Cursor diagnostic properties remain. Clean snapshot s1-clean6 was launched
in background with PID30928; publication is pending the actual package gates.

P6本地交付完成：clean6双宽干净构建及ABI/资源/依赖门禁通过，最终库双宽各2项通过；s1-package2共14项聚焦实包通过，36.72秒，无skip。初次测试命令误写Task22参数未执行，改为Task 22；沙箱后台无结果退出后确认已结束，再以获准的后台工作器完成。
最终包真实计算器PID5240的激活、最大化/还原、最小化/还原及撤销几何通过；visual23初次沙箱激活E_ACCESSDENIED，获准环境重跑PID16800/宿主12040/线程5924，144DPI标题、菜单截图、撤销与几何通过。未改系统设置。
s1-publish2已将同批四文件发布assets根目录及O:/winnt并校验摘要。原生输入visual22移动/缩放各一次及键盘菜单最大化通过，但可见缩放光标仍为箭头；该失败不能被其他通过项替代。S1/T22保持开放。产品差异仅frame.c及对应测试，31/1与25/4行增删；无第二套渲染路径、无资源变更。当前提交推送及远端复核待完成。

P7复核：P6 b1db5d2aa0f4349f8ef978aa6d6be7d724f1e7de 已推送；重新fetch后读取10个变更blob逐字节一致，远端四产物与两处八个部署文件均匹配实包SHA256。仅登记复核，无二进制变化；S1/T22保持开放，可见边框光标失败及其他有限覆盖边界不变。

## P8 cursor routing audit (no product change)

All probes used fresh Calculator instances on Default desktop, 144 DPI, host
PID12040, bounded execution and owned-instance cleanup. The already registered
`s1-window-audit` scratch contains these immutable experiment directories:

| Experiment | Own PID / host UI TID | Result |
| --- | --- | --- |
| cursor-nchit1 | 35236 / 25640 | Report native resize hits and forward NC press: visible cursor still arrow; move/resize and restore pass. |
| cursor-events1 | 35952 / 18168 | Real WM_SETCURSOR received three times; lParam33554442 = mouse-move/HTLEFT, thread cursor0x10011, global cursor0x10003. NC mouse move received three times. |
| cursor-move1 | 11108 / 5724 | SetCursor after DefSubclassProc on NC mouse move also does not change global cursor. No drag repeated. |
| cursor-class1 | 35236 / 16492 | Private owned class instead of STATIC still fails; real cursor messages arrive. No shared class changed. PID was reused by the OS, not assumed same process. |
| cursor-native-frame1 | 26940 / 8224 | Before applying style, exact point(325,528) hits root/HTLEFT and global cursor is expected SIZEWE0x10011. Native baseline passes. |
| cursor-transparent1 | 26272 / 35960 | Ask native root hit and return HTTRANSPARENT for sizing: visible cursor still arrow; move/resize/menu/restore pass. |

The original native Calculator works at the same edge. The adapter changes that
behavior. This is not only a faulty expected cursor handle, absent input, or a
missing WM_SETCURSOR message. A correct thread-local SetCursor observation does
not establish the globally displayed cursor. Correct NC hits, delayed SetCursor,
a private surface class, and HTTRANSPARENT did not fix the observed failure.
The exact host/input routing cause is still unresolved; do not assert DWM,
CoreWindow.PointerCursor or an OS bug without further evidence. Failed variants
remain scratch only; no extra hooks, polling, thread attachment or global cursor
settings were added to the product. Prototype DLLs are pinned in host lifetime;
removal restores only the owned window adapter, not physical DLL unloading.

References reviewed:
- https://learn.microsoft.com/en-us/windows/win32/menurc/wm-setcursor
- https://learn.microsoft.com/en-us/windows/win32/inputdev/wm-nchittest
- https://learn.microsoft.com/en-us/uwp/api/windows.ui.core.corewindow.pointercursor

Microsoft documents HTTRANSPARENT routing to underlying windows of the same
thread. This was a candidate mechanism, not proof that it fixes this host.
Next useful test should identify the actual original-root cursor/input dispatch
when overlay is present, rather than repeat SetCursor variants. Other remaining
scope includes activation reuse/ordinary entry integration, title replacement,
and additional real UWP applications. No product or binary changes in P8;
P6 tested/deployed hashes remain the current baseline. S1/T22 stay open.

## P9 original-root and plain Win32 cursor controls

No product changes. All experiments remain under registered s1-window-audit.
The common menu-message dispatcher does not intercept WM_NCHITTEST or
WM_SETCURSOR; inspection ruled out that suspected query contamination.

- cursor-root1: fresh Calculator PID15468, host12040/TID27836. Native root hit
  is10; root actually receives two WM_SETCURSOR calls (lParam33554442) and
  returns with thread cursor0x10011. Global cursor remains0x10003. Thus the
  HTTRANSPARENT test reached the original root; missing delivery is not the cause.
- cursor-layered-pass1: PID29320/TID9492. WS_EX_TRANSPARENT on the complete
  layered surface makes WindowFromPoint return the original root; root cursor
  messages still execute, but visible cursor remains arrow. This experiment
  disables pointer access to custom title buttons, so it is not a candidate fix.
- cursor-observer1: PID24900/TID28336 using unchanged clean6 product DLL. Removed
  the WindowFromPoint observation before polling; cursor still fails. That
  observer call alone is not the cause.
- cursor-fixture.exe: pure C owned ordinary Win32 window plus the same clean6
  win16-window-uwp static library. Native/styled/restored visible cursor all
  equal expected0x10011; apply/remove succeed. Independent UI thread pumps real
  mouse input; pointer restored and own window closed. No injection or UWP used.

These results bound the failure to this host environment rather than a universal
failure of the adapter's cursor selection. They do not prove the exact UWP input
owner or justify changing host-wide mouse/pointer settings. No polling/cursor
forcing, shared-class modifications, extra input hooks or UI worker threads
were added to the product. A different input-thread placement is only a future
candidate and would require explicit lifetime and cross-thread UI design plus a
real prototype; it is not implemented or validated here.

The P6 package is unchanged. This is diagnostic progress, not cursor repair or
complete UWP support. S1/T22 remain open.

## P10 additional applications and dual-title boundary

Same clean6 DLL/current production activation code, native owned instances,
144DPI, shared hostPID12040; no alarm, recording or scan action invoked. Scratch
under s1-window-audit: alarms-app1, recorder-app1, scan-app1, cursor-unhook1.

| Target | Live PID | Screenshot PID / host TID | Actual result |
| --- | --- | --- | --- |
| WindowsAlarms | 10728 | 2012 / 28764 | Activation, caption buttons and remove geometry pass; visual33 is update-required placeholder, not normal app UI. |
| WindowsSoundRecorder | 31228 | 32440 / 16356 | Same exterior checks pass; visual34 also update-required placeholder, not normal app UI. |
| WindowsScan | 8240 | 11696 / 20516 | Real CoreWindow and host found; apply fails ERROR_NOT_SUPPORTED50, no styled surface. |

Scan captures exist but do not prove styling: result51 encodes error50; remove
result1169 encodes NOT_FOUND1168. GeometryPID28028/TID13572 and menuPID37080/
TID32544 confirm maximized root(-11,-11,1931,1019), client1920x1019 at(0,-11).
Two visible ApplicationFrameTitleBarWindow children:(0,0,69,48),(0,0,1920,48).
Product title_child picks the first visible child; geometry rejects right edge69
instead of1920. This is the confirmed first unsupported node. Native system menu
has only Restore/Move/Size/Minimize/Maximize/Close; separate left hamburger is not
represented there. Selecting the full-width title alone would cover that entry.
No naive selection patch merged. Correct support needs true-caption selection
and preservation of auxiliary host-menu semantics, without XAML client changes.
Those semantics remain unverified. No target files/registry/settings modified.

Read-only local package identities (not required build/test dependencies):

- Microsoft.WindowsAlarms_1.1.158.0_x64__8wekyb3d8bbwe; Microsoft.WindowsAlarms_8wekyb3d8bbwe!App; `C:\Program Files\WindowsApps\Microsoft.WindowsAlarms_1.1.158.0_x64__8wekyb3d8bbwe\Time.exe`; SHA256 `6b7ca93a5e65ac50bdddad10288892a662c28a1862e435b51a7d85393d93a494`.

- Microsoft.WindowsSoundRecorder_1.1.127.0_x64__8wekyb3d8bbwe; Microsoft.WindowsSoundRecorder_8wekyb3d8bbwe!App; `C:\Program Files\WindowsApps\Microsoft.WindowsSoundRecorder_1.1.127.0_x64__8wekyb3d8bbwe\SoundRec.exe`; SHA256 `63504f33392834e5de090141092fde15248cff3f4babf7f1f9dba7a1ddfda446`.

- Microsoft.WindowsScan_6.3.9654.17133_x64__8wekyb3d8bbwe; Microsoft.WindowsScan_8wekyb3d8bbwe!App; `C:\Program Files\WindowsApps\Microsoft.WindowsScan_6.3.9654.17133_x64__8wekyb3d8bbwe\ScanApp.exe`; SHA256 `cb6ec8fab98458bc9019a264c936b4996fb2bef0a323fd23abfe057ac5578d6b`.

Calculator cursor-unhook1 PID2644/TID27768 removes installer hook immediately
after apply, matching product lifetime. Cursor still arrow; reinstall solely to
remove succeeds. Retained installer hook does not explain cursor failure.
No product changes: P6 package remains published. S1/T22 open; placeholder tests
are not normal-app acceptance, and unsupported Scan is not a styling pass.

## P11 dual-title functional prototype

Scratch scan-app1/aux: first owned activationPID36892 did not produce a ready
associated frame within10s; cleaned, not a pass. SecondPID12704 uses new host
PID18304/TID28816 (host change observed, not initiated by this task). Posting
native left-title button messages opens a menu with Share and Settings; down
succeeds, synchronous up times out in modal handling. visual39 confirms menu.
No menu item/scan/recording action selected. This proves the entry must survive.

scan-title1 selects the widest visible title child only in a scratch adapter.
OwnPID15928/host18304/TID22752,144DPI: apply, actual visual40 title/system menu,
remove and geometry all pass on normal Scan UI. Thus title selection explains
ERROR_NOT_SUPPORTED50. This prototype alone hides the auxiliary entry and is
not suitable for product delivery.

scan-aux-route1 adds a scratch Application menu... system-menu command and
routes it by asynchronously posting the original auxiliary child's button
messages. OwnPID19524/host18304/TID1720: visual41 classic frame/system menu and
visual41-aux original Share/Settings popup visible; apply/remove/geometry pass.
No XAML changes or cloned menu actions. Prototype deliberately not production:
fixed command ID, unchecked subclass/menu setup, duplicate install and teardown
contracts require proper implementation/tests; direct command dispatch proves
routing but does not yet prove selecting the added item by keyboard/mouse.
Original popup remains native, consistent with no client-control styling scope.
Next implement caption selection plus optional auxiliary action in window library,
with product supplying actual HWND/behavior and owned command lifetime; do not
leave scratch's ad hoc product-side renderer or unchecked command allocation.

cursor-unhook1/new-host: unchanged P6 DLL, own CalculatorPID34000, new host18304/
TID14964 still shows arrow at confirmed edge after installer unhook. Old-host
lifetime alone does not explain cursor issue. No product binary changes. All
scratch covered by registered s1-window-audit; pinned prototypes remain until
host exit. S1/T22 open; deployed P6 package unchanged.

## P12 implementation and incremental verification (delivery pending)

Public frame_action supplies borrowed label/context and GUI-thread callback.
Only menu tracking temporarily inserts an available16-aligned command from
0xeff0..0xe000; cleanup verifies item ownership before deletion. No permanent
system-menu replacement. Product selects widest visible native title and retains
one optional auxiliary title action; ambiguous additional titles rejected.
Callback posts native down/up after tracking ends; does not clone Share/Settings.
Static C API addition only; DLL exported ABI unchanged. Original resources intact.

s1-aux-action1 x86/x64 each3 passed, zero skips; conflict with existing0xeff0,
repeated apply/remove, invalid action, no menu mutation outside tracking,
cancel does not invoke callback, temporary0xefe0 removed and original0xeff0
preserved. Existing lifecycle/menu-state/resource tests still pass.
scan-action-product1 uses actual incremental DLL SHA256
033f2a95edad126cdee5539e4e3489b90e92a451cabe673da40da9697e0a9d71.
Owned ScanPID12420/host18304/TID23204,144DPI: actual UP/ENTER selects0xeff0,
visual42-aux shows original Share/Settings; before/styled/restored captures,
remove/geometry/unhook pass. No menu business item selected. Unlike P11 direct
command probe, this verifies the real menu selection route. Complete build,
actual package tests and publication remain pending; S1/T22 open.

P12交付：clean7双宽干净构建、ABI、原图形资源及依赖门禁通过；最终库各2项通过。s1-package3共14项聚焦通过、无skip、33.97秒。最终包ScanPID11928/宿主18304/TID16224、144DPI实际UP/ENTER选择eff0后原Share/Settings打开，visual43截图及移除几何通过；CalculatorPID38488的窗口按钮、恢复/撤销几何通过。s1-publish3两处四文件摘要匹配测试包。静态库增加apply_action，不新增产品导出/第五产物；src三文件69增6删，测试26增1删。公开库无产品私有依赖，原资源未改。原生附加菜单不重新绘制，缩放光标仍未修复；S1/T22保持开放。当前提交推送复核待完成。

P13复核：P12 3c334b3b8ecf0aef9bdddbf259e3a15edc536703 已推送；重新fetch并读取12个变更blob逐字节一致，远端四文件及两处八个部署文件与实包SHA一致。无新增二进制变化，S1/T22保持开放。

## P14 title visibility and omitted full-screen action audit

The P12 native Scan capture `s1-window-audit/visual43-before.bmp` shows a
full-screen glyph to the left of minimize. Production frame_layout currently
contains only system/minimize/maximize, and the optional action forwards the
left Share/Settings button. Thus this visible native full-screen entry has no
replacement in the styled title/system menu. P12 preserves the application
menu, not every native title action. Do not infer that full-screen itself is
unsupported by the OS or that maximize is an equivalent operation.

A pure C MSAA probe in registered scratch
`s1-window-audit/fullscreen-audit1` was compiled to enumerate the new owned
Scan frame/title accessible objects (name, role, location, default action).
PID13728 started 2026-11-05 23:50:53 local and exited with
`SKIP input desktop not Default`, before activation. No host injection or app
operation occurred. Native invocation/interface evidence remains unavailable;
no guessed coordinate or undocumented command entered product code.

The private-desktop production-library test now checks title hide/show:
surface disappears while the native title is hidden, attachment remains,
same surface returns when title is shown, and owner client geometry remains
unchanged. This tests lifecycle prerequisites only; it is not a real UWP
full-screen acceptance result. No product source, export or resource change.

P14验证完成：s1-title-visibility1双宽各3项通过、无skip；构建分别11.97/6.48秒，
测试4.60/4.35秒。仅测试新增7行及审计文档更新，产品源码和资源未改变；
assets与O:/winnt四文件仍逐项等于clean7实测包，无需重新发布相同产品。
全屏入口缺失已确认；真实调用探针因屏保桌面跳过，未计通过。S1/T22开放。

## P15 native full-screen interface and styled round trip

2026-11-05/06, registered `s1-window-audit` scratch; pure C17 MinGW GCC
compiled metadata and invocation probes. No production changes. The ordinary
visual probe retry PID30396 again skipped before activation because input
was not Default. Subsequent probes explicitly permit metadata/API testing on
the inactive desktop; none capture pixels or send global keyboard/mouse input.

- `fullscreen-audit2`: own ScanPID25664, host18304/TID24084,144DPI.
  MSAA root/title traversal succeeds but does not expose the full-screen action.
- `fullscreen-uia1`: own ScanPID32088, host18304/TID8260,144DPI.
  UIA native title `TitleBar` exposes button AutomationId `EnterFullScreen`,
  name `Full screen Scan`, bounds1644,0,1713,48. Left title exposes `Actions`.
  These IDs are observed host implementation details, not a universal API promise.
- `fullscreen-invoke1`: own ScanPID33424, host18304/TID25136,144DPI.
  Non-null IUIAutomationInvokePattern calls return S_OK. Enter changes root
  from(-11,-11,1931,1019) to(0,0,1920,1080), hides native title; querying the
  retained title HWND exposes `ExitFullScreen`. Exit returns S_OK, title visible,
  original geometry restored. This is distinct from maximize.
- `fullscreen-styled1`: own ScanPID12532, host18304/TID1420,144DPI;
  loads exact clean7 tested x64 DLL SHA
  9fceda211bdae7dc337bb0388ed2a0f9ce482879fba2f7f93cf3f550e4bb03b0.
  Apply callback succeeds. Surface HWND0x110ad4 remains valid but hidden during
  native full screen, becomes visible after exit, geometry restored; removal
  callback succeeds. Temporary thread hook removed; only own Scan closed.

All four probes exited; no shared host termination, no global propagation,
no target-file/registry changes. API state evidence is not normal-display visual
acceptance. The styled system menu still lacks a user-accessible full-screen
entry; this experiment does not implement that entry.

Next implementation constraint: do not run UIA client queries against the
host's own UI synchronously inside the GUI/menu callback. Microsoft documents
using a separate thread when a UIA client accesses its own UI. Candidate route:
query supported native title actions outside that GUI thread, expose only
observed actions in the classic menu, asynchronously invoke the original
provider after menu tracking ends, release COM objects on their owning thread.
First verify this in-host thread/lifetime route; preserve three components/four
files and existing renderer. No permanent provider cache or client-control tree
walk is needed. Do not blindly replace full screen with SC_MAXIMIZE, guessed
coordinates, or an undocumented command.

References reviewed (primary documentation):
- https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-threading
- https://learn.microsoft.com/en-us/windows/win32/winauto/using-iaccessible
- https://learn.microsoft.com/en-us/uwp/api/windows.ui.viewmanagement.applicationview.isfullscreenmode

Known open items remain full-screen menu integration/normal display validation,
visible resizing cursor, title replacement, ordinary/global/reused UWP paths,
and broader app/DPI coverage. No S/T closure or new binary publication.

## P16 in-host UIA worker proof and unsupported-action control

`fullscreen-host1` in registered s1-window-audit compiled pure C17 with
-Wall -Wextra -Werror. The temporary selected-thread hook validates root/title
ownership and starts one independent MTA worker from its explicit request;
DllMain has no added work. Worker acquires its own module reference, locates
the direct native title child by AutomationId, invokes the original provider,
releases COM objects/apartment and exits through FreeLibraryAndExitThread.
No propagation, XAML traversal, registry or target-file modification.

2026-11-06 00:06 local: own ScanPID19028, shared host18304/TID29396,144DPI.
Exact clean7 style DLL applies successfully. In-host EnterFullScreen and
ExitFullScreen both return S_OK; root geometry returns from fullscreen
(0,0,1920,1080) to original(-11,-11,1931,1019). Same overlay0x4808ee is valid/
hidden during fullscreen and visible afterward. Style removal succeeds.
DriverPID36188 exits. Subsequent host module enumeration finds zero mapped
fullscreen-host1 prototype modules. This proves that prototype worker's finite
module reference release, not physical unloading of the pinned product DLL.
Timeout/hung UIA provider behavior is not proven by this successful run.

`fullscreen-calculator1`, own CalculatorPID20852, host18304/TID2448,144DPI:
native TitleBar exposes Minimize/Maximize/Close, no EnterFullScreen or
ExitFullScreen. Thus do not unconditionally add a full-screen command to all
UWP frames. Both runs used inactive input desktop, no visible-pixel or real
mouse/keyboard acceptance; all instances were probe-owned and closed.

Minimum next product change, constrained to witnessed Calculator/Scan:
1. Extend the public frame optional-action input to a copied array, preserving
   the single-action wrapper. Temporary menu insertion/collision/cleanup and
   callbacks remain in the common frame implementation, no UIA in renderer.
2. Coordinator probes native title full-screen capability outside host GUI
   thread before requesting apply; pass only a capability flag, not a COM
   pointer or foreign heap pointer. No button on witnessed unsupported target.
3. Product callback schedules the validated independent-thread native toggle;
   no UIA client work/wait in DllMain or GUI callback. Original auxiliary menu
   remains accessible. Module reference belongs to each outstanding worker.
4. Test two menu actions, command collision/cancel/cleanup, absent capability,
   real in-host enter/exit, release and product menu invocation. Keep failure,
   provider timeout and normal-display validation distinct. Only after these
   pass perform the required frozen dual-width package/release.

No product changes or republishing in P16. Existing cursor, lifecycle,
ordinary/global/reuse and broader-target limitations remain; S1/T22 open.

## P17 implementation and pre-package evidence

Public apply_actions copies up to 256 optional callbacks (the natural reserved
command range); single-action and no-action APIs retained. Per-menu insertion
uses free aligned IDs and owned item data, and unwinds partial insertion before
callbacks. Product shares native title selection between coordinator and DLL.
Coordinator's UIA8 query adds APPLY_FULLSCREEN only for an actual native invoke
provider; failed query reports HRESULT without silently removing an unknown
action. Callback invokes native EnterFullScreen/ExitFullScreen on independent
MTA worker, one module reference per request; GUI thread never waits. UIA
connection/transaction timeouts are 1000ms each, not a proven overall worker
lifetime deadline; provider hang/unload failure is not claimed solved. Product
module remains pinned. No global integration or client-control changes.

New DLL/launcher system import OLEAUT32 is explicitly allowed in exact package
checks; no external runtime, export, ABI number or fifth product added. First
incremental x86 PE gate correctly rejected the previously undeclared import.
Second round double-width each3 passed. Third round native selection fixture
failed because its parent inherited hidden startup; explicit ShowWindow fixes
the fixture. Final s1-native-actions4 double-width each4 passed, no skips;
build9.51/11.15s, test4.66/4.95s. Includes multiple actions, collision, cancel,
partial insertion rollback, title selection and refusal of same-GUI-thread UIA.

API-only real probes use inactive input desktop (no pixel/input acceptance):
- native-menu1 Calculator activation ownPID5276,144DPI: new product capability
  query, attachment, max/restore/min/restore and remove/geometry pass.
- native-menu1 Scan same product activation/button/removal path passes.
- native-worker1 fixed short wait observed no change, not a successful test.
  native-worker2 adds scratch-only completion HRESULT, waits bounded3s:
  actual production native.c worker enters/exits successfully, same overlay
  hidden/restored, original geometry and removal pass. No diagnostic property
  enters production.
- native-menu-select1 synthetic mouse messages caused target window to vanish;
  host18304 remained alive, no corresponding application crash event found.
  Cause not established; this is a failed probe, not proof of a product crash.
- select2 native MSAA exact menu item returns S_OK without state change; select3
  posted keyboard does not highlight the target and refuses submission.
  select4 waits4s for real state: no change despite MSAA S_OK. Thus actual
  menu-selection end-to-end remains unverified, not passed. Stop repeating
  inactive-desktop input substitutes; normal-desktop selection remains needed.

Frozen package s1-clean8, actual package s1-package4 and deployment s1-publish4
are reserved for this change. Keep S1/T22 open; no full-screen UI acceptance
claim until the real menu route and normal display have evidence.

P17本地交付：clean8双位宽干净构建、ABI/原资源/来源/依赖门禁通过；最终UWP
测试双宽各3项通过无skip。s1-package4共14项聚焦通过无skip，32.06秒。
最终DLL计算器PID36124与ScanPID18888、144DPI的API接入/按钮/撤销通过；
全屏工作线程的生产源码诊断对照通过，但菜单实际选择仍未验证（屏保下替代
消息/无障碍均未证明状态变化）。s1-publish4两处四文件与测试摘要相同。
本次没有正常桌面截图，不以API验证替代外观/真实菜单选择验收；S1/T22保持开放。
