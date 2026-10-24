# Project Status

## Current Work

M0 T11 S1 active：调研实现按需UAC提权及样式注入。T10/S1按用户指令取消，
不宣称UWP支持；T8与T9保持关闭，M0开放。

## M0 T11 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New；T10取消后下一未用T11 S1。 |
| Admission And Approval | 用户明确放弃UWP要求，准入UAC调研实现并要求收口验收；永久提交推送授权。 |
| Objective | 确认740/完整性边界，按需经系统runas提升本启动器，再沿用挂起创建/双宽注入，完成有限闭环交付。 |
| Non-goals | 不绕过UAC/保存凭据/自动点安全桌面，不加第五产品，不改UI资源，不扫描既有高权限进程，不承诺DLL内所有子孙自动提升。 |
| Reference Baseline | fb1fc0a / 0.9.0.0 ABI3；T10研究取消且未改产品；T8关闭。 |
| Candidate Proposal | [UAC方案](../proposals/uac-elevation.md)。 |
| Files And ABI Surface | run16s main及内部elevation模块；测试纯C探针及requireAdministrator fixture；版本0.11.0.0；DLL公开ABI不改。 |
| Applicable Rules | 完整Task Reading Set；C17/GCC/Ninja/ccache；只新建目标、系统合法授权；环境/句柄跨UAC边界不得冒充原始继承。 |
| Verification | 740修前复现，shell调用/取消/错误/递归原生测试，双宽真实管理员fixture及GUI像素/模块/附着，普通启动传播/完整同包回归；实际UAC人工同意另列。 |
| Expected Markers | UAC BASELINE、UAC UNIT PASS、UAC TARGET PASS、UAC PACKAGE PASS；取消1223；实际令牌/目标DLL/初始化/窗口状态分别验证。 |
| Asset Needs | build/t11-uac为原生诊断/聚焦/交付证据；build/t11-package为干净包/实际包/备份；前任build/t10-xaml及其登记LocalCache暂存先提炼后安全回收。 |
| Reporting Requirements | 根因与最小方案先报；支持直接入口、普通环境/提升新环境、工作目录/参数/等待、取消/高令牌/位宽及未覆盖分别报告，双发布/远端复核。 |
| Stop Conditions | 需新服务/产物/安全绕过或不能确认自有目标时停止；缺真实同意覆盖标未覆盖，不捏造通过。 |
| Exit Criteria | 本项目范围实现及完整双宽同包交付、发布远端复核；有限实际授权验收有明确入口和边界，收口本S与T。 |
| Original Owner Request | 放弃现代应用要求，准入UAC提权，调研正确实施方法做到收口验收。 |
| Similar-Issue Sweep | 入口740/其他错误区别；Shell直接启动目标丢注入；重复提升/当前已提升、原始参数/绝对路径/CWD、系统环境/控制台隔离、取消/等待及子孙API契约。 |

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 11
- Last allocated Td S: 9
- Last delivery: M0 Td S9 P2 (P1 425b7df实际远端复核通过；清理及清理后增量通过，T8开放)
- Open task: M0 T11 S1；T10取消；T8/T9及Td S9保持关闭

## Technical Baseline

产品0.11.0.0 / M0 T11 S1 verified并已双目录发布，待实际远端复核；
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
