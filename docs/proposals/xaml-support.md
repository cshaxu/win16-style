# UWP and XAML support

用户授权的有限实现研究：先识别本机计算器，再检查激活边界、C ABI与诊断TAP，
只有真实接入/正常显示绘制/交互/清理闭环才标注支持。记事本另行分类。

S1按CURRENT活动包执行：不默认增加产品、不改系统安全或共享宿主、不修改经典
Win32效果。候选IApplicationActivationManager和InitializeXamlDiagnosticsEx只作
待实测路线。无载荷DLL被动策略保留；晚初始化不能重跑启动bootstrap。

历史T9已使用，本工作分配T10。任何重大边界变化先提供失败证据和最小方案。

用户放弃本要求；T10/S1取消收口，无产品支持发布。见[记录](../history/M0-T10-S1-xaml-research.md)。
