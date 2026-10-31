# Project Status

## Current Work

M0 T20 S3准入：优先处理公共对话框的已知错误结果。

## M0 T20 S3 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New；S2矩阵已交付推送并复核 |
| Admission And Approval | 用户授权自动准入后续S，直至T20收口前 |
| Objective | 优先解决Shell文件夹筛选回调丢失；对COM并发与多选已知错误评估低成本安全止损 |
| Non-goals | 不扩展COM布局长尾、不扫描或注入既有进程、不修改系统设置和兄弟仓库 |
| Reference Baseline | S2审计提交27c32be；P295已发布四文件 |
| Candidate Proposal | [公共对话框适配及后续S序列](../proposals/classic-folder-dialogs.md) |
| Files And ABI Surface | 现有DLL及纯C原生测试；三组件四产物，原则上不改公开ABI |
| Applicable Rules | M0、C17、MinGW GCC、Ninja、ccache；原图形资源及来源逐字节保持 |
| Verification | 新旧系统与实际包双宽A/W筛选/选择/取消、回调与PIDL，必要负例；同批双宽干净构建和包回归 |
| Expected Markers | 筛选应用后禁止项不可见、返回语义正确；不能忠实转换则保留系统调用并如实报告 |
| Asset Needs | 登记build/t20-s3-safety及build/t20-package scratch |
| Reporting Requirements | 根因、修复和回退范围、测试及未覆盖面；只以证据认定支持 |
| Stop Conditions | 不伪造Shell对象，不修改共享系统资源；用户已降级COM长尾，超出低成本止损则记录TODO |
| Exit Criteria | 已知筛选错误修复或安全回退；COM两项止损评估有明确结果；产品变更完成双发布、提交推送及实际远端复核 |
| Original Owner Request | 按ROI自动准入后续S并执行至T20收口前 |
| Similar-Issue Sweep | BROWSE_WRAPPER A/W、BFFM_IUNKNOWN、COM文件多选及并发回退 |

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 20
- Last allocated Td S: 15
- Last allocated T18 S: 64
- Last allocated T19 S: 3
- Last allocated T20 S: 3
- Last delivery: M0 T20 S3 P1 修复 Shell 筛选丢失：带回调的受限组合安全回退系统新树，双宽构建、实际包测试及双位置发布。
- Open task: T20 S3 优先处理已知错误结果；[S2矩阵](../history/M0-T20-S2-dialog-audit.md)。
- 打印当前页已纳入受限 PrintDlgEx 转接；跨 DPI、实际打印和本地化仍待覆盖。
- 并发缺口：同进程两个 COM 文件对话框的系统原生双宽对照通过，已发布旧框转接双宽失败；目录串用及超时的具体根因仍待定位。
- 已知多选缺口：O: 自有目录中，COM 多选旧框不列出部分带空格的长文件名；不能仅增加列表项，结果转换也需处理。
- Shell筛选：受影响的带回调组合保留系统新树及原筛选接口；旧树中完整支持该回调仍未实现，其他标志组合未全面验证。详见[S3证据](../history/M0-T20-S3-shell-filter.md)。
- P253 已发布：干净包 58 项通过；系统打印宿主自然更替后，已发布包及无注入对照的双宽 A/W Print/Apply 实际虚拟打印各八项通过。此前 32 位 EndPage 失败的原因未明；详见阶段证据。

## Technical Baseline

三个组件、四产品文件；纯C、GCC、Ninja、ccache，原图形资源及来源保持。发布位置为项目assets根目录和O:/winnt根目录。当前有限控件契约见[界面设计](../design/UI.md)，启动/架构见[架构](../design/ARCHITECTURE.md)，具体覆盖及限制见[收口审计](../history/M0-T18-scoped-closure-audit.md)。T19 S1双宽干净包、两处部署摘要、18项聚焦回归和真实程序记录见[交付证据](../history/M0-T19-S1-canon-ownerdraw.md)。历史完整活动包保存在[归档](../history/M0-T18-S64-current-archive.md)，只作历史记录。

用户否决S2两次试用；S1 P208源码和原始四文件已恢复并经实际远端复核。见[撤回记录](../history/M0-T19-S2-rollback.md)。

S3调查结果见[收口记录](../history/M0-T19-S3-dialog-closure.md)：当前原版真实点击正常；历史异常根因未查明，本S没有产品代码或二进制修改。

T20 S1 的阶段包已发布部分公共对话框和 COM 桥接，P231 默认文件夹新建保留旧式布局并修正选中目录及输入路径后的确认问题；P234 处理旧式列表系统主题动画覆盖项目滚动条，同源码增量候选的双宽真实拖动通过，干净包的双宽聚焦消息驱动测试及 x64 真实拖动通过，干净包 x86 真实鼠标复测因近期输入跳过。按用户最新指示暂停COM布局桥接及其长尾。事件桥接实验默认关闭，定制、隐藏目录及完整矩阵仍未完成；公开新式对话框 API 与 demo 路径已纳入阶段包。详见[阶段证据](../history/M0-T20-S1-preliminary.md)；S1收口，T20保持开放。

## Recent Closure

- M0 T20 S2：入口与回退矩阵已交付、双宽构建及实际包代表回归通过；[证据](../history/M0-T20-S2-dialog-audit.md)。

- M0 T20 S1：按用户指令阶段收口；已交付、已知缺口及后续承接见[记录](../history/M0-T20-S1-closure.md)，T20保持开放。

- M0 T19 S3 / T19：用户确认当前对话框恢复正常并要求收口；历史异常原因不明，字体试验已撤回。

- M0 T19 S2：按用户要求撤回全部字体试验并恢复P208原始四文件；[活动包归档](../history/M0-T19-S2-rollback-current-archive.md)。

- M0 T19 S1：Canon自绘按钮图像恢复，实际远端源码和四文件已复核；范围与限制见[交付记录](../history/M0-T19-S1-canon-ownerdraw.md)，[活动包归档](../history/M0-T19-S1-current-archive.md)。
- M0 T18 S64 / T18：按用户确认的常用可见控件范围收口；非视觉、应用自绘不处理，安全契约不足及长尾留TODO。首次焦点回归失败、重入负例不足、物理输入和完整矩阵未覆盖均保留记录。

2026-10-29 阶段包 P218 已构建、聚焦测试及双位置发布；完整矩阵和公共对话框桥接仍在进行，不代表 T20/S1 收口。失败、跳过与已知回退见阶段证据。
