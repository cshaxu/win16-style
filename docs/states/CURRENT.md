# Project Status

## Current Work

M0 T12 S1 active：修复缺失 IAT 目录的旧 PE 启动期导入 staging，
由用户实际 DoubleKiller.exe 失败准入。T10按用户指令取消；T8/T9/T11保持关闭，M0开放。

## M0 T12 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New；T11收口后下一未用T12 S1。 |
| Admission And Approval | 用户报告 `run16s "D:\\programs\\misc\\DoubleKiller.exe"` 启动失败并要求修复。 |
| Objective | 保持现有注入架构，为缺失IAT目录的旧PE补齐有界thunk范围；真实DoubleKiller恢复启动和样式。 |
| Non-goals | 不修改目标文件、不加产品/服务/全局hook、不改资源或已确认UI。 |
| Reference Baseline | 0203e09 / 0.11.0.0 ABI3；DoubleKiller裸启动正常、run16s下实际0xC0000005。 |
| Candidate Proposal | [legacy import staging](../proposals/legacy-import-staging.md)。 |
| Files And ABI Surface | staging私有实现、缺失IAT回归与创建契约测试的内核对象身份判据修正；版本0.12.0.0；DLL公开ABI不变。 |
| Applicable Rules | 完整Task Reading Set；C17/GCC/Ninja/ccache；仅自有新进程、四产物不增加。 |
| Verification | 零OFT自包含负例/修后初始化、DoubleKiller裸/带样式对照、双宽回归与同包测试。 |
| Expected Markers | 旧stager负例0xC0000005、LEGACY IAT PASS/REJECT PASS、DoubleKiller实际Win16标题及退出0。 |
| Asset Needs | build/t12-legacy-staging、build/t12-doublekiller-candidate、build/t12-doublekiller-diagnosis、build/t12-package及现存build/t12-*诊断文件均归本S；先提炼证据，再逐路径核验安全回收。 |
| Reporting Requirements | 记录PE形状、断点、查找thunk边界、正常和失败路径、未覆盖面、发布及远端复核。 |
| Stop Conditions | 原thunk范围无法有界读取时返回原生错误并回滚；不改目标文件/全局机制。 |
| Exit Criteria | 修后自包含回归和真实目标通过；双宽同包测试、双发布、推送远端复核及任务审计完成。 |
| Original Owner Request | `run16s "D:\\programs\\misc\\DoubleKiller.exe" 启动失败帮我看看如何修复。 |
| Similar-Issue Sweep | 零OFT、空/外部导入目录、查找thunk边界/终止符、保护恢复、跨宽helper、旧/正常PE、失败回滚。 |

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 12
- Last allocated Td S: 9
- Last delivery: M0 T11 S1 P2 (P1 d77017f实际远端139输入/四产物/八发布副本复核通过；登记scratch回收)
- Open task: M0 T12 S1；T11关闭；T10取消；T8/T9及Td S9保持关闭

## Technical Baseline

0.12.0.0 / M0 T12 S1已本地验证并双目录发布，待P1提交推送及实际远端复核；
三组件四文件、x64 Console启动器、双宽DLL、x64 demo、ABI3保持。
缺失IAT目录时按原FirstThunk范围有界补齐，原bootstrap恢复；不重写lookup数组。
真实DoubleKiller默认命令和--wait均正常，96DPI经典标题/移动/最小化/恢复/关闭通过；
已发布system32实际DLL路径/摘要及原生截图有证据，未执行查重/删除/移动文件。

GCC16.1.0/Ninja/共享ccache双宽干净构建；确切最终包23/23通过，双发布8摘要相同。
首次69项全量65通过3跳过1旧数值句柄判据失败；旧包裸进程亦复现，后改为固定
内核Event身份比较且保留父进程信号断言。修后旧/新各64向量及最终包创建契约通过。
修后再干净构建的71加载映像不变，仅2契约fixture预期变化；不冒称另跑全绿69项。
双宽新缺失IAT测试可检出旧stager的0xC0000005，畸形范围返回193且保持头/挂起。
证据见[本次记录](../history/M0-T12-S1-legacy-iat.md)、[release](../etc/release.json)。

标题/字体/菜单/滚动条renderer及六原资源逐字节保持；T11按需740/UAC、T9原生cmd
和已有传播契约保持。未配置可选后端/clipboard共3跳过，物理输入及任意DPI不计覆盖。
T11原回滚计数债务仍在TODO；本次不宣称解决。UWP研究取消，无现代应用支持发布。
不修改或依赖兄弟仓库；仅处理本次新建进程。两旧T12自有CPU繁忙worker已核验回收。

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
