# UWP/XAML 内部标准控件换肤

## 状态与目标

用户已准入 T25 承接 T24 的研究和控件移植；当前活动 S 以 CURRENT 为准。当前任务及外框
支持范围由 CURRENT 管理。本候选在外框工作之外，研究并实现普通 UWP
应用内部标准 XAML 控件的 Win3.x 外观，不要求目标预先引用 run16s。

## 架构与共享边界

- UWP 系统 XAML / WinUI 2：研究控件树接入、UI 线程调度、属性、资源及
  ControlTemplate 适配；保留原控件对象、绑定、命令和交互语义。
- WinUI 3：可共享视觉规范及模板适配思路，但运行时、接口和控件版本需要
  单独适配及验证，不能宣称同一份接入代码直接通用。首轮优先 UWP。
- WinForms 不属于 XAML；原生 HWND 控件优先沿用现有 Win32 路线，托管
  自绘另行分类，不纳入本候选的 XAML 覆盖承诺。
- 复用原图形资源与视觉规范，保留三组件四产物和纯 C 工具链。运行时
  适配不修改目标文件、注册表或系统安全设置。

## 建议实施顺序

先证明真实应用中一个 Button 的接入、经典外观、点击、
键盘和撤销；再按 ROI 扩展 CheckBox、RadioButton、TextBox、ComboBox、
列表及滚动控件。监听新页面、弹层和动态控件；复杂自定义模板、自绘、
DirectX 和 WebView 内容明确排除或延期，不无限追逐长尾。

## 验收标准

用原生探针区分 DLL 加载、XAML 接入、控件识别及实际绘制，提供真实截图。
验证正常/悬停/按下/禁用/焦点状态、键盘、输入法、绑定、命令、无障碍、
布局与 DPI、动态创建及撤销资源释放。保存并恢复本层改动，不能无条件
ClearProperty 充当恢复。不同框架与应用分别列通过、失败、跳过及未覆盖项。
产品变更按仓库规范构建、实包测试与发布，不影响现有窗口及 Win32 控件效果。

## 研究依据

- [UWP 控件模板](https://learn.microsoft.com/en-us/windows/uwp/design/controls-and-patterns/control-templates)
- [XAML Diagnostics](https://learn.microsoft.com/en-us/windows/win32/api/_xaml_diagnostics/)
- [WinUI 2](https://learn.microsoft.com/en-us/windows/apps/winui/winui2/)

这些接口提供候选机制，不代表第三方应用的完整换肤已经实现或验证。
