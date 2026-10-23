# Project Goal

从 ntvdm64 独立出 `run16 --win16-style-all` 的原生进程启动、钩子传播及
Win16 窗口外观能力。只保留三个组件和四个产品文件。

1. run16s-exe 构建 x64 run16s.exe，启动 Win32/Win64 应用，在目标开始执行前
   安装同位宽 ntw16s32.dll 或 ntw16s64.dll，并启用 Win16 窗口外观。
2. ntw16s-dll 用同一源码家族构建两个 DLL，负责外观及后续原生子进程钩子传播。
3. demo-exe 构建 x64 demo.exe。直接启动显示普通 Win32 窗口；经 run16s.exe
   启动才显示 Win16 外观。demo 不主动加载或链接样式 DLL。

目标运行环境为 x64 Windows，目标程序位宽为 x86/x64。ARM64、DOS/Win16
二进制执行、NTVDM/WOW 服务、模拟器、全局主题替换和系统 DLL 安装不在范围内。
窗口外观不改变应用本身的业务逻辑、文本、原生控件数据和通知语义。

本项目始终为 M0；完成标准见[路线图](ROADMAP.md)，实现状态只见 CURRENT。
