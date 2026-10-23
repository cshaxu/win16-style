# Product package

当前开发交付为M0 T6 S1 P1，版本0.6.0.0，恰四个产品：
run16s.exe(x64)、ntw16s32.dll(x86)、ntw16s64.dll(x64)、demo.exe(x64)。
本README是仓库说明，不是第五个产品；运行时将四文件放在同一目录。

[发布清单](../../docs/etc/release.json)记录源码输入指纹、GCC工具链、逐文件
SHA-256/PE/imports及测试范围。[S1记录](../../docs/history/M0-T6-S1-package.md)
保留干净复制树构建、失败处置及实际同包测试。各S的包从对应Git提交复取，
当前四文件来自同一干净构建，已经直接通过9项同包测试。

这是阶段开发包。完整矩阵、必要真实鼠标/键盘/焦点/DPI验收及M0全任务审计
仍待后续S完成；不把私有桌面、自动消息或截图称为物理输入通过。
用法及支持边界见[构建与运行说明](../../docs/etc/operations/BUILD.md)。
包从自身目录定位DLL，不安装到Windows系统目录。
