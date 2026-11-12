# M0 T26 S1: shared frame helpers and XAML child detection

第十六批之后，XAML 桥窗口处理 `WM_GETICON` / `WM_SETICON` 时临时调整并恢复原生窗口样式、保持重入及错误值的机制已从 Win16 分派器移入公共 `frame.h`，Win16 直接使用；具体何时调用和状态所有权仍属于风格。x86/x64 增量构建及自建外框回归通过，覆盖图标读取/设置、客户区和外框几何保持、样式位恢复及样式变化期间销毁。未创建或切换隔离桌面；真实 XAML 宿主和 Win2000 目前未验证，改动尚未发布。

发布状态更新：本改动已纳入[第十七批候选](M0-T26-S1-shared-candidate-17.md)，发布后验证及限制以该记录为准。

The Win16 frame no longer defines pass-through wrappers for frame refresh,
client-screen geometry or client restoration. Its callers directly use the
same `lib-style/common/frame.h` helpers already used by the Win2000 frame.
Existing style-specific activation, DWM, geometry and removal decisions stay
with their owners.

The existing Win16 XAML child detection for `Windows.UI.Composition.DesktopWindowContentBridge`
and `Microsoft.UI.Content.DesktopChildSiteBridge` is now `style_frame_xaml_bridge`
in the shared frame header. Its class matching and optional visibility behavior
are unchanged; Win16 frame dispatch and layout use it directly. It is a detection
primitive only: it does not style Win2000 UWP/WinUI or implement XAML repaint.
The new standalone test registers both names in its own process, creates hidden
child windows, and exercises matching and the visibility gate; it does not show
or switch desktops. Real XAML rendering and the prior UWP lifecycle access-denied
problem remain unverified. T26's wider lifecycle and rendering audit stays open.

The x86/x64 clean MinGW GCC, Ninja and ccache build passed from snapshot
`5ebda72a6a7b09b1746b72c1be53b36e07fc3d6a1cf3e63130febcfdc74c0ad0`.
All eight files were published to assets and `O:\winnt` with matching hashes
before testing. Both widths passed 14 component tests, two published-launcher
probes and two direct control-protocol workers, for 36 focused checks. The
Win16 control link test includes client geometry restoration; the new XAML
case is synthetic and not a real UWP application. Existing UWP lifecycle and
visual limitations remain open.
# 补充：工作区边缘保护

`style_frame_keep_outer` 从 Win16 外框布局中抽出：它只接受窗口、期望客户区
和风格提供的客户区矩形，根据 XAML 桥接及显示器工作区判断是否保留外框；
不依赖 Win16 几何类型。Win16 的实际附着路径直接调用它，旧专用实现删除。

x86/x64 公共测试使用两种自建桥接窗口类，覆盖边缘、工作区内及无效参数；
两种位宽的 Win16 完整自建外框工作线程测试也通过，其中包含工作区边缘、
外框移除、客户区恢复、最大化和最小化。测试直接执行工作线程分支，没有
创建或切换隔离桌面。这不证明真实 WinUI/UWP 程序或其他风格已使用该接口；
本次改动尚未干净构建或发布。

## 最大化工作区交集

普通 Win16 外框测量和现代表面边界均调用 `style_frame_work_area` 计算
窗口矩形与当前显示器工作区的交集。公共函数只在交集有效时写入输出：
普通外框仍在失败时沿用原几何，现代表面仍将失败向调用方返回。
x86/x64 的公共交集及失败不改输出测试、现代表面最大化几何测试和 Win16
自建外框完整工作线程测试通过；真实分层表面提交仍因错误 5 标为 SKIP。
不创建或切换隔离桌面；改动尚未干净构建或发布。
