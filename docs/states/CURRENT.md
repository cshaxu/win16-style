# Project Status

## Current Work

M0 T25 S2 已按用户要求准入：将 Win16 与 Win2000 可共用的 DLL 基础能力抽离至 `ntwshk-dll`，风格专有代码保留在各自样式组件。P1 已发布、推送并复核，重点回归通过；托盘端到端及 UWP 基线限制见[交付记录](../history/M0-T25-S2-P1-delivery.md)。S2 保持开放，等待验收；后续任务规划暂缓。

## M0 T25 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New；T25 内新 S。 |
| Admission And Approval | 用户明确要求把基础能力抽离至 ntwshk-dll，且本工作放在 S2。 |
| Objective | 审计并提取两种风格共用的进程内基础机制，让生产调用方实际复用；风格绘制、布局与专有行为留在 ntw16s/ntw2ks。 |
| Non-goals | 不实现完整 Win2000 外观，不擅自更改钩子覆盖范围、用户体验或其他仓库。 |
| Reference Baseline | main cd22f0a9，S1 已发布六文件；保留当前未提交的历史索引与规划。 |
| Candidate Proposal | docs/proposals/windows-2000-classic.md；S2 共用能力审计随工作记录。 |
| Files And ABI Surface | 审查双位宽 ntwshk、两套样式 DLL、启动器及测试；明确导出、载荷、动态依赖与所有权。 |
| Applicable Rules | M0；纯 C17、MinGW GCC、Ninja、ccache；用户新增共享 DLL 指令优先于旧四/六文件清单。 |
| Verification | 审计实际调用链；双位宽启动、传播、样式选择、默认 Win16、故障清理与现有包回归；未覆盖和跳过如实记录。 |
| Expected Markers | 明确公共/风格职责、真实产品接入、资源所有权、构建及部署摘要；不得仅建立空 DLL。 |
| Asset Needs | 用户要求新增共享 DLL；双位宽 `ntwshk32/64.dll` 与既有六文件组成八文件目标包，保留原图形资源及来源声明。 |
| Reporting Requirements | 明确哪些代码保留样式侧及原因，真实加载和显示分别报告；不把构建成功称为视觉验收。 |
| Stop Conditions | 不修改系统安全设置、目标文件或兄弟仓库；发现不可安全拆分时记录证据并继续其他可做工作。 |
| Exit Criteria | 共用能力真正被两风格使用；测试、发布、提交推送与远端复核按适用规则完成，明确未覆盖项。 |
| Original Owner Request | 基础代码放 ntwshk-dll，样式代码留 ntw16s/ntw2ks；该工作放入 S2，后续规划暂缓。 |
| Similar-Issue Sweep | 检查启动和晚期路径、全局模式、UWP、托盘、对话框、跨位宽 helper、显式 API 及测试消费者。 |

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 25
- Last allocated Td S: 19
- Last allocated T18 S: 64
- Last allocated T19 S: 3
- Last allocated T20 S: 19
- Last allocated T21 S: 44
- Last allocated T25 S: 2
- Last verified remote delivery: M0 T25 S2 P1，`9aecf5305714a8105072790afab2464c5bf4e4bc`。
- Open task: M0 T25 S2；共用 DLL 分离。

## Technical Baseline

已发布基线为五组件、八产品文件；纯 C17、MinGW GCC、Ninja 和 ccache。T25 S2 P1 八文件已发布于项目 assets 根目录与 O:/winnt，摘要和验证范围见[交付记录](../history/M0-T25-S2-P1-delivery.md)。历史状态见[T24 工作快照](../history/M0-T24-S1-working-log.md)和[原状态快照](../history/M0-T24-current-state-archive.md)。
