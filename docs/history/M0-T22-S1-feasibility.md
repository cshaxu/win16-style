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
