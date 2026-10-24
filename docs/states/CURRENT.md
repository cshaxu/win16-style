# Project Status

## Current Work

M0 Td S10 verified locally：调查扫雷英雄榜中文控件字体偏小；双宽4例定位中文回退字形缩小；只读产品，不进行字体修复。
证据见[调查记录](../history/M0-Td-S10-leaderboard-font.md)，待推送/实际远端复核后关闭。
M0 T12 S1已完成缺失IAT目录的DoubleKiller启动兼容、验证、双发布及
P1实际远端复核；T12收口，M0开放。T8/T9/T11关闭、T10取消的状态保持。

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 12
- Last allocated Td S: 10
- Last delivery: M0 T12 S1 P2 (P1 507847c实际远端139输入/四产物及8发布摘要通过；12登记scratch根/文件回收)
- Open task: Td S10；T12/T11/T9/T8关闭；T10取消；Td S9保持关闭

## Technical Baseline

0.12.0.0 / M0 T12 S1已验证、双发布及实际远端复核并收口；
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
不修改或依赖兄弟仓库；仅处理本次新建进程。两旧T12自有CPU繁忙worker及12登记scratch根/文件已核验回收；
持久Ninja/cache保留，双宽no work。

## Recent Closure

- [T12缺失IAT启动修复](../history/M0-T12-S1-legacy-iat.md)：507847c实际远端139输入/四产物、8发布摘要及真实DoubleKiller通过；S1/T12收口，M0开放。

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

## M0 Td S10 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Governance; 新的只读诊断和证据文档，不分配产品版本或重开已关闭任务 |
| Admission And Approval | 用户确认上一S解决并批准收口，明确要求新S调查扫雷英雄榜字体偏小 |
| Objective | 对照原生字体、System变体、中文实际墨迹和DPI，定位客户端缩小原因 |
| Non-goals | 不改产品、标题/菜单/滚动条、不扩大中文支持承诺、不修改兄弟仓库 |
| Reference Baseline | ea8876f；0.12.0.0，准入时工作树干净；T12已关闭 |
| Candidate Proposal | [英雄榜字体调查](../proposals/leaderboard-font.md) |
| Files And ABI Surface | 仅状态/调查文档；ABI、四文件及源码保持；临时纯C探针在build/td-s10-font |
| Applicable Rules | Task Reading Set已读；EXECUTION、DOCUMENT、ARCHITECTURE、CODING及UI字体边界 |
| Verification | 有限等待的原生私有桌面对照、实际DLL字体替换、字高/墨迹/字符集；门禁和摘要 |
| Expected Markers | FONT SAMPLE、FONT INK、TARGET IDENTITY；失败及未覆盖项分别记录 |
| Asset Needs | 用户截图只作问题输入；不导入图形资源，保留原资源字节 |
| Reporting Requirements | 原因证据、实际覆盖、最小修复建议及未确定事项，不把ASCII通过称为中文通过 |
| Stop Conditions | 不注入既有进程，不为NE启动接触用户服务；无法安全复现则明确限制 |
| Exit Criteria | 调查证据提炼、治理检查、提交推送及实际远端复核；产品修复另准入 |
| Original Owner Request | “批准收口本S任务！准入一个新的S任务调查，为啥扫雷英雄榜的字体比正常的小？” |
| Similar-Issue Sweep | 搜索font.c、controls.c、font_layout.c及既有T8证据；覆盖中文与ASCII、字体回退和度量 |
