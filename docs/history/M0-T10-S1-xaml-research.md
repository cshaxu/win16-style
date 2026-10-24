# M0 T10 S1: cancelled XAML research

用户明确放弃本要求，转入UAC工作。T10/S1取消收口，不称为实现完成或支持发布。
T8已按用户先前明确指令关闭；其四S及之后经典回归事实保留，独立TODO不消失。

原生C/GCC/Ninja/ccache双宽COM本地自测通过。计算器11.2607.0.0为x64 AppContainer、
系统XAML/WinUI2.8；记事本manifest为FullTrust桌面应用/WindowsAppRuntime2依赖，
未用UWP结论代替其运行验证。144DPI正常显示：CoreWindow属于新激活的自有计算器，
标题按钮属于既有ApplicationFrameHost中的ApplicationFrameTitleBarWindow。

自有新进程令牌打开仓库TAP返回5；不改ACL，复制同一测试DLL至应用已有私有缓存后
可读并实际加载，SetSite/诊断QI/树QI/订阅均S_OK，收到623变更和45个Button类型Add
记录（不是唯一可交互按钮数）。发布DLL相同SHA被动加载ABI3，现有renderer针对
宿主HWND提前拒绝，1444；未向共享宿主注入或改变其样式。Diagnostics返回S_OK
但TAP尚未加载的早期例子，不能计作接入成功。UWP标题API保留系统按钮控制。

没有实现或验证按钮换肤、点击/键盘/IME/无障碍及完整经典标题，没有生产TAP、
启动期bootstrap重跑、第五产品或资源改动。CoreWindow WM_CLOSE未使进程在3秒内
退出，最终仅终止自己已核验的新实例；不冒称优雅解绑或资源零泄漏。x86仅编译和
本地C ABI验证，不等于跨宽诊断。详细脱敏计数及原始摘要见
[研究证据](../etc/evidence/xaml-research.json)。研究源归登记scratch，不进入产品。

微软参考：[激活](https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/nn-shobjidl_core-iapplicationactivationmanager)、
[Diagnostics入口](https://learn.microsoft.com/en-us/windows/win32/api/xamlom/nf-xamlom-initializexamldiagnosticsex)、
[UWP标题边界](https://learn.microsoft.com/en-us/windows/uwp/ui-input/title-bar)。
UWPSpy固定d959bec865801731dc6474fe5786264463b7596e，GPLv3，仅阅读研究，未导入其代码。

提炼后检查无reparse/活动研究进程，已回收登记scratch的181文件/6,101,704字节，
以及精确核验SHA的应用私有缓存两文件和空目录；未改ACL，未关闭共享宿主。
