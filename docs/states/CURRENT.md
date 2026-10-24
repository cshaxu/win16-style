# Project Status

## Current Work

无活动任务包。M0 T11 S1已完成按需UAC修复、双目录发布、实际远端复核及scratch回收；
T11收口供用户验收。T10按用户指令取消，无UWP产品支持；T8/T9保持关闭，M0开放。

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 11
- Last allocated Td S: 9
- Last delivery: M0 T11 S1 P2 (P1 d77017f实际远端139输入/四产物/八发布副本复核通过；登记scratch回收)
- Open task: none；T11/S1关闭；T10取消；T8/T9及Td S9保持关闭

## Technical Baseline

产品0.11.0.0 / M0 T11 S1已双目录发布、实际远端复核通过并收口；
三组件四文件、x64 Console启动器、双宽DLL、x64 demo、ABI3/31名称及ordinal1不变。
仅入口真实740按系统runas提升启动器，再沿用挂起创建/安装/恢复；非提升重入被拒绝。
系统提升环境及独立控制台；取消1223；DLL子孙CreateProcess不自动请求UAC。
方法、测试入口与未覆盖面见[UAC](../etc/operations/UAC.md)。标题/字体/菜单/滚动条
renderer对fb1fc0a无改动，六原资源逐字节保留。

纯C17/GCC16.1.0、Ninja1.13.2/ccache4.13.6；持久.ninja/toolchain和共享cache保留，
不恢复false-fresh、不清缓存；默认8 build jobs/4 test slots、重UI占2。
双宽干净构建、注册69项66通过3明确跳过；实际最终包21/21；实际系统授权7根及7个
带样式子孙；双目录8摘要相同。完整矩阵与LF候选73加载映像对照先于仅测试的回滚
失败诊断改动，确切最终包另做21项；不冒称同一raw PE。证据与各阶段摘要见
[release](../etc/release.json)、[本次记录](../history/M0-T11-S1-uac.md)。

两次并行包旧回滚计数断言失败根因未明，最终严格断言包通过并不消除该事实；
保留TODO，不宣称修复。clipboard权限/可选后端共3跳过、物理输入、真实UAC取消
点击、跨管理员凭据、映射盘及任意高DPI不计覆盖。UWP研究已取消且未进入产品。
T9外部导入表/原生cmd修复保持；NE间歇问题及裸NTSRV复用边界不扩张为本次修复。
README英文8张精选图保持；不依赖或修改兄弟仓库，不扫描注入既有进程。

## Recent Closure

- [T11按需UAC](../history/M0-T11-S1-uac.md)：d77017f实际远端139输入/四产物及8发布摘要通过；S1/T11收口，M0开放。

- [T10取消研究](../history/M0-T10-S1-xaml-research.md)：用户取消，无产品支持发布；T8按用户明确要求收口。

- [Td S9构建与测试效率](../history/M0-Td-S9-test-efficiency.md)：425b7df实际远端复核通过；S9收口，M0/T8开放。

- [T9 S1原生嵌套cmd及外部导入表](../history/M0-T9-S1-native-cmd-staging.md)：P1实际远端复核及清理完成；仅本轮S/T9收口，T8开放。

- [Td S8传播诊断](../history/M0-Td-S8-vdm-propagation.md)：ce20f15实际远端复核通过；原NE间歇失败不作修复声明，独立staging缺陷移交当前产品任务。

- [Td S7 README编辑](../history/M0-Td-S7-readme-edit.md)：894e012实际远端复核通过；指定沿革段落已删除。

- [Td S6英文README与真实截图](../history/M0-Td-S6-english-readme.md)：02df11a实际远端复核通过；8张公开PNG保留，scratch已安全回收。

- [S4菜单比例修复](../history/M0-T8-S4-menu-proportions.md)：64e3fd3实际远端复核通过，S4收口；T8保持开放。
- [S3菜单厚黑边修复](../history/M0-T8-S3-popup-shadow.md)：aa28427实际复核通过；S3收口。
- [S2 System字形修复](../history/M0-T8-S2-system-font.md)：2a0c7dd实际复核通过；S2收口。
- [S1字体大小修复](../history/M0-T8-S1-font-layout.md)：1c82c2e收口。
- [菜单取消修复](../history/M0-T7-menu-cancellation.md)与[历史产品审计](../history/M0-T6-completion.md)保留历史边界。
