# Modern desktop window adaptation

T22 UWP外框已收口；用户批准本候选为M0 T23，当前S1按用户批准扩展为记事本实现与交付。
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
| S1 (active) | Notepad: native frame restoration, standard `run16s notepad` launch handoff, tab/menu preservation, actual package tests and publication for acceptance. |
| S2 | Task Manager: standard `run16s taskmgr`, privilege and native frame verification. |
| S3 | Explorer folder windows: standard `run16s explorer`, shared-shell boundaries; preserve desktop/taskbar. |
| S4 | Terminal and console hosting: standard `run16s cmd` and `run16s conhost`, identify actual window host and preserve console/session semantics; Terminal separately verified. |
| S5 | Consolidated regression and closure audit across the standard launch commands. |

Only S1 is currently admitted; subsequent S tasks follow this approved task scope
in order, with one active packet. Evidence may require splitting implementation
steps into additional S tasks; never reuse an allocated identifier or silently
hide an unsupported target. No universal framework support is promised by a single
successful app. Product binary changes require the normal same-snapshot dual-width
build, actual-package tests and assets/O:/winnt publication. Admission alone does
not change the existing0.22.0.0 deployed package or claim a new binary release.
