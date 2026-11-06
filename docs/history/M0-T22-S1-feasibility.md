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
