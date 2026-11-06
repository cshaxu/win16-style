# UWP and XAML support

用户授权的有限实现研究：先识别本机计算器，再检查激活边界、C ABI与诊断TAP，
只有真实接入/正常显示绘制/交互/清理闭环才标注支持。记事本另行分类。

S1按CURRENT活动包执行：不默认增加产品、不改系统安全或共享宿主、不修改经典
Win32效果。候选IApplicationActivationManager和InitializeXamlDiagnosticsEx只作
待实测路线。无载荷DLL被动策略保留；晚初始化不能重跑启动bootstrap。

历史T9已使用，本工作分配T10。任何重大边界变化先提供失败证据和最小方案。

用户放弃本要求；T10/S1取消收口，无产品支持发布。见[记录](../history/M0-T10-S1-xaml-research.md)。

## T22 S1 renewed research

用户重新授权研究；旧 T10 的取消记录保留。目标为普通第三方 UWP，无需目标
预先引用 run16s；先研究，不把诊断接入算作产品换肤完成。

当前源码断点：src/lib-win32/process-global/policy.c 拒绝 TokenIsAppContainer；
src/ntw16s-dll/global/policy.c 排除 ApplicationFrameHost。不能只删除这两处判断：
XAML 客户区和共享宿主标题属于不同绘制、线程和生命周期边界。

建议验证路线：run16s 识别包/实际框架/目标身份；对应位宽现有 DLL 内增加
C ABI TAP 适配，经 XAML Diagnostics 接入并按 UI 线程管理元素。客户区优先
修改现有控件的样式/模板，保留命令、绑定及无障碍；动态树变化继续应用。
保存原始属性来源，解除时恢复本层修改，不能把 ClearProperty 一概当作恢复。
不在 DllMain 做诊断或 UI 初始化，不将新增代码塞入公共 process-global。

标题框架必须单独证明。官方 UWP 定制保留系统 caption buttons；它不能单独
实现完整 Win3.x 左上系统菜单及右上按钮布局。共享宿主适配属于另一路线，
需证明仅目标窗口生效及撤销不影响其他窗口；在本次验证前不放开宿主排除规则。
外部覆盖窗或重设父窗口不推荐为默认方案：输入、Z序、DPI和恢复问题尚未证明。

2026-11-05 阅读 UWPSpy tap.cpp 与 visualtreewatcher.cpp：SetSite 获取诊断
接口，按线程维护观察窗口；订阅和停止有线程/重入要求。其退出代码存在等待
后自卸载，不能直接作为本项目安全物理卸载的证明。源码 GPL-3.0，本次只读研究，
没有复制代码；当前 main 阅读不是固定版本导入。

参考：
- https://learn.microsoft.com/en-us/windows/win32/api/xamlom/nn-xamlom-ivisualtreeservice
- https://learn.microsoft.com/en-us/windows/uwp/ui-input/title-bar
- https://github.com/m417z/UWPSpy/blob/main/UWPSpy/tap.cpp
- https://github.com/m417z/UWPSpy/blob/main/UWPSpy/visualtreewatcher.cpp

研究结论、最小实现设计和原型判据见 [T22 S1研究](../history/M0-T22-S1-feasibility.md)。当前没有产品变更。

## 当前实施范围：窗口优先（覆盖此前先单按钮的建议）

用户明确要求只处理外框、标题栏、系统菜单与窗口按钮；内部XAML控件、字体、
模板和客户区均延期。本阶段不需要先建设TAP/XAML控件树适配。

原生只读窗口探针实测：可见 ApplicationFrameWindow 及其两个标题子窗口、
InputSink 子窗口同属 ApplicationFrameHost 的一个 GUI 线程；其中一个标题
子窗口可见，另一个隐藏。该观察没有修改用户窗口，也没有验证样式效果。
仅按 CoreWindow 类名识别不够：探针同时发现 Explorer、TextInputHost、TabTip
和 ShellHost 中的 CoreWindow，不能把它们当作普通目标应用。

代码审计：Ntw16sWindowAttach 在 attachment.c 623 行附近只附着所给HWND，
并要求同进程同GUI线程；这是可复用的单窗口入口。global/hook.c 则递归处理
子窗口并调度 late propagation，global/thread.c 初始化传播；不能直接作为
本轮宿主适配入口，否则违反“不处理内部控件”的范围。

最小候选：在所属线程对唯一选定的外框调用单窗口附着/移除接口；不递归
子窗口，不启用进程传播。另需对原生标题子窗口的显示、布局和绘制做专门
协调，防止旧标题覆盖新标题；保留原状态以恢复。是否必须隐藏/调整该子窗
只能在自有目标原型中决定，不凭类名直接批量隐藏。

探针只读检查已完成；尚未改变共享宿主排除规则，尚未安装宿主样式、截屏或
验证按钮操作，不能称为已修复/已发布。研究scratch登记在CURRENT。
