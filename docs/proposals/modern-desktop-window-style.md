# Modern desktop window adaptation

T22 UWP外框已收口；用户批准本候选为M0 T23，当前活动 S 及状态以 CURRENT 为准。
仅窗口外框，不含客户区控件；CURRENT拥有活动状态。

## 本机分类基线

2026-11-06，原生只读探针位于 T22 登记的 s1-window-audit/classify.c；
未注入、启动或关闭所观察的 Explorer/Task Manager，未改变系统设置。

| 目标 | 证据 | 后续验证 |
| --- | --- | --- |
| Notepad 11.2607.14.0 | 包清单 FullTrustApplication；最终窗口为 Notepad 类，加载 Microsoft.UI.Xaml；启动中转进程有样式 DLL，最终 /SESSION 子进程无 DLL | 先查最终进程转交，再验证自定义标题栏；不能把模块加载当作样式生效 |
| Windows Terminal 1.24.12741.0 | 包清单 FullTrustApplication，依赖 Microsoft.UI.Xaml.2.8；本次无运行实例观测 | 自有终端窗口启动、标题栏/标签区边界、关闭；不可影响用户现有会话 |
| Explorer | PID8056，GetPackageFullName=15700（无包），TokenIsAppContainer=0；加载 Windows.UI.Xaml、Microsoft.UI.Xaml.CBS、Windows.UI.XamlHost 及传统 Shell 模块 | 区分文件夹窗口和共享桌面/任务栏，禁止仅因 XAML 模块或 ApplicationFrameWindow 类名就当作 UWP |
| Task Manager 10.0.26100.9818 | PID26784，GetPackageFullName=15700，TokenIsAppContainer=0；模块枚举错误5，未规避权限；静态导入含 DUI70/DUser，文件存在 XAML 字符串 | 桌面混合 UI 用例；具体框架版本仍待权限允许的自有实例验证，不以静态字符串代替已加载模块证据 |

WinUI 是界面框架，UWP 是应用模型；两者不能当成互斥分类。
上述本机桌面目标不计入 T22 的 UWP 换肤通过率。分类不是换肤验收。

## 准入后的顺序与退出条件

1. 冻结实际目标、启动转交及窗口承载关系，复用已有公共进程能力。
2. 定位最终目标，按窗口能力选择适配路径，不用逐应用名字映射代替通用识别。
3. 优先复用 Win16 绘制；产品适配器处理自定义标题栏、窗口生命周期和恢复。
4. 自有实例分别验证实际绘制、移动/缩放、按钮、菜单、重复启动与撤销；
   权限失败和未覆盖单列。Explorer 共享 Shell 与 Terminal 会话单独约束。
5. 按准入时治理规范完成同批交付。不得提前声称这些程序已支持。

## T23 planned S sequence

| S | Work and exit evidence |
| --- | --- |
| S1 (closed) | Notepad: native frame restoration, standard `run16s notepad` launch handoff, tab/menu preservation, actual package tests and publication for acceptance. |
| S2 (closed, limited) | Task Manager: standard launch and frame attached; duplicate icon, white strip and resize flicker remain in TODO by owner decision. |
| S3 (closed) | Explorer folder windows: standard `run16s explorer`, shared-shell boundaries; preserve desktop/taskbar. |
| S4 (active) | Terminal and console hosting: standard `run16s cmd` and `run16s conhost`, identify actual window host and preserve console/session semantics; Terminal separately verified. |
| 待准入 | Codex Desktop and Chrome: classify actual window ownership and verify frame behavior on test-owned instances. |
| 待准入 | Consolidated regression and closure audit across the standard launch commands and tested applications. |

After the existing Explorer and Terminal work, verify **Codex Desktop** and
**Chrome Browser** as additional real-application targets before the consolidated
closure audit. Their S identifiers are not allocated here. First identify each
installed executable, process handoff, actual window owner and UI framework;
do not assume either application uses the same XAML path as Task Manager. Use
test-owned instances and profiles where available, preserve existing user sessions,
and verify title rendering, controls in the title area, menus, resize, focus,
restoration and ordinary launch behavior. Record unsupported or skipped cases
without treating them as passes. This work concerns window frames; XAML client
controls remain in their separate candidate.

Subsequent S tasks follow the approved task scope in order, with one active packet.
Evidence may require splitting implementation
steps into additional S tasks; never reuse an allocated identifier or silently
hide an unsupported target. No universal framework support is promised by a single
successful app. Product binary changes require the normal same-snapshot dual-width
build, actual-package tests and assets/O:/winnt publication. Admission alone does
not change the existing0.22.0.0 deployed package or claim a new binary release.

## Subsequent-target preliminary observations

The following are bounded research observations while S3 remains active; they
do not admit a new S, alter the product, or establish application support.
Targets were self-created on private desktops and the existing user windows were
not operated. Screen captures and test profiles are not product artifacts.

* Windows Terminal 1.24.12741.0: a launch with the explicit installed binary,
  `-w new` and a uniquely titled `cmd` tab reached a newly created
  `CASCADIA_HOSTING_WINDOW_CLASS` window outside the launcher's job. Its process
  loaded the published DLL and reported the window attached. At 144 DPI the
  offscreen sample showed a dark title area, not a Win16 caption; maximize and
  restore states were observed. This does not establish physical display,
  standard `run16s cmd` or `run16s conhost`, shared-process behavior or recovery.
  Cleanup used the unique window identity; a separate two-tab experiment needed
  termination of its verified self-created process after a close timeout.
* Chrome 157.0.8103.38: a unique private profile ran natively, but the published
  style-plus-propagation path repeatedly terminated GPU children and eventually
  the browser with `0x80000003`. The local GPU log reported message-window
  creation error 13. A scratch diagnostic observed attachment error 13 for a
  message-only window and a nested system IME window; it does not identify the
  internal failure responsible. Style-only and propagation-only contrasts
  remained alive during their short observation, without establishing general
  compatibility. Existing user Chrome instances were not operated.
* A scratch-only prototype skipped the direct message-window creation scope and
  system IME attachment, and changed `WS_EX_NOREDIRECTIONBITMAP` at creation
  for captioned windows. In these local samples Chrome and Terminal stayed
  alive, their Win16 captions appeared, and UI Automation opened and closed a
  test tab. Chrome's own caption buttons remained duplicated. This is not a
  product design: the broad creation-time flag change can affect composition,
  and both applications returned error 87 when the original flag was restored
  on the owning window thread after removing the frame. Further work must find
  a safe, scoped and verifiable approach, preserve application controls and
  disclose any remaining restoration limit. No universal WinUI/Chromium
  compatibility follows from these samples.

* A separate native Terminal test created two uniquely named windows on a
  private desktop. Both windows belonged to the same self-created Terminal
  process, while each tab's `GetConsoleWindow()` returned a distinct
  `PseudoConsoleWindow` whose owner and root owner were its corresponding
  Terminal window. Both test windows were closed and the process exited. This
  is a useful candidate for identifying the host of an existing `run16s cmd`
  session, not a documented universal contract: other Terminal versions, tab
  changes, default-terminal handoff, classic console and identity/security
  checks remain unverified. No host was attached in this experiment.
