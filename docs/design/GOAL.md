# Project Goal

构建运行时 Windows 外观框架，使用统一接入和样式应用机制装配不同年代的呈现。
当前提供的 Win3.x、Win2000 能力及 Win32/UWP/WinUI 实际覆盖、失败和限制由
[CURRENT](../states/CURRENT.md)管理；Luna、Aero 是后续方向，不表示已实现。

所有样式能力归 `src/lib-win32-style`，按接入 access（startup/runtime/global）、
样式应用 application（窗口、菜单、控件、对话框、资源和框架适配）、风格呈现
appearance 三层划分。无样式含义的进程、句柄和托盘能力留在 `src/lib-win32`。
依赖、状态和资源所有者以[架构](ARCHITECTURE.md)为权威，T26按[任务规划](../proposals/style-library-reorganization.md)有序完成现有能力重组。

当前五组件八文件以[产品契约](../etc/product-contract.json)为唯一产物权威：
run16s启动器、ntwshk共享运行时、ntw16s/ntw2ks风格模块及原生demo。
启动器按目标位宽选择包内模块；普通启动保持原参数和后代传播语义，全局模式按
已有用户选择与策略组合发现、接入、样式切换和尽力恢复。移除样式、停用传播与
物理卸载分别报告，不承诺未具备的能力。

demo直接启动保持系统原生行为，经启动器后才显示所选样式，不加载或链接样式
代码。运行时外观保留应用业务、内容、控件数据和通知语义，不修改目标文件、
注册表或系统安全配置，不替换Windows自带DLL。UWP/WinUI现有外框适配不代表
客户区控件或所有应用均已覆盖。原图形资源及来源声明按[来源策略](../etc/operations/source-policy.md)保留，不用近似重绘替代原资源。

目标为x64 Windows及已有x86/x64原生应用；ARM64、DOS/Win16二进制执行、
NTVDM/WOW服务和模拟器不在范围。发布到assets根及O:/winnt的是同批八文件，
发布位置和兄弟仓库均不成为构建或运行依赖。开发按完整能力合批增量验证，整S
再做必要的干净构建、发布和实际包测试，不逐文件完整交付。

本项目始终为M0；完成标准见[路线图](ROADMAP.md)，实现状态只见CURRENT。
