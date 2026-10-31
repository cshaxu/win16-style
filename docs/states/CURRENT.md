# Project Status

## Current Work

M0 T20 S5：交接及真实部署包验证完成，待提交推送和实际远端复核；T20保持开放。

## M0 T20 S5 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New；S4已提交推送并远端复核 |
| Admission And Approval | 用户授权后续S自动执行并在T20收口前停止 |
| Objective | 整理最终入口与回退、测试和未解决项，验证同批包与实际部署，形成T20可审查的收口交接 |
| Non-goals | 不关闭T20，不扩展COM布局长尾，不穷举所有参数，不使用computer-use |
| Reference Baseline | S4交付4298cdb；S3 P2发布b0a78c4 |
| Candidate Proposal | [公共对话框适配及后续S序列](../proposals/classic-folder-dialogs.md) |
| Files And ABI Surface | 审计、TODO与必要自有测试；维持三组件四产物及现有ABI |
| Applicable Rules | M0、C17、MinGW GCC、Ninja、ccache；原资源及来源逐字节保持 |
| Verification | 双宽干净构建、同批包实际测试、部署包摘要和实际部署代表探针、远端复核 |
| Expected Markers | 明确哪些已支持、哪些安全回退、哪些未覆盖；保留S4旧探针超时与COM并发边界 |
| Asset Needs | 登记build/t20-s5-handoff及build/t20-package scratch |
| Reporting Requirements | 提供最终证据、demo路径、限制与明确待用户验收状态；禁止将T20标为关闭 |
| Stop Conditions | 测试不通过如实记录及处理；不更改系统设置、共享资源或兄弟仓库 |
| Exit Criteria | 交接矩阵、TODO、构建/实包/部署对照及远端复核完成；关闭S5后停在T20收口前 |
| Original Owner Request | 自动执行各S并在T任务收口前停止 |
| Similar-Issue Sweep | 审计S2矩阵、S3安全回退、S4常用窗口/滚动及最终四文件部署 |

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 20
- Last allocated Td S: 15
- Last allocated T18 S: 64
- Last allocated T19 S: 3
- Last allocated T20 S: 5
- Last delivery: M0 T20 S3 P2：COM多选安全回退系统新框，双宽构建、实际包测试及双位置发布；Shell筛选P1同样采用安全回退。
- Open task: T20 S5 最终验证与交接；[S2矩阵](../history/M0-T20-S2-dialog-audit.md)、[S4结果](../history/M0-T20-S4-common.md)。
- 打印当前页已纳入受限 PrintDlgEx 转接；跨 DPI、实际打印和本地化仍待覆盖。
- COM并发：已观察的双多选并发由系统原生路径止损；旧框目录串用根因、非多选并发仍未查明，详见[S3证据](../history/M0-T20-S3-com-safety.md)。
- COM多选：默认回退系统新框，O: 长名、单项、取消、链接及双宽并发经有限验证；旧框实现及其他参数仍未全面解决。
- Shell筛选：受影响的带回调组合保留系统新树及原筛选接口；旧树中完整支持该回调仍未实现，其他标志组合未全面验证。详见[S3证据](../history/M0-T20-S3-shell-filter.md)。
- P253 已发布：干净包 58 项通过；系统打印宿主自然更替后，已发布包及无注入对照的双宽 A/W Print/Apply 实际虚拟打印各八项通过。此前 32 位 EndPage 失败的原因未明；详见阶段证据。

## Technical Baseline

三个组件、四产品文件；纯C、GCC、Ninja、ccache，原图形资源及来源保持。发布位置为项目assets根目录和O:/winnt根目录。当前有限控件契约见[界面设计](../design/UI.md)，启动/架构见[架构](../design/ARCHITECTURE.md)，具体覆盖及限制见[收口审计](../history/M0-T18-scoped-closure-audit.md)。T19 S1双宽干净包、两处部署摘要、18项聚焦回归和真实程序记录见[交付证据](../history/M0-T19-S1-canon-ownerdraw.md)。历史完整活动包保存在[归档](../history/M0-T18-S64-current-archive.md)，只作历史记录。

用户否决S2两次试用；S1 P208源码和原始四文件已恢复并经实际远端复核。见[撤回记录](../history/M0-T19-S2-rollback.md)。

S3调查结果见[收口记录](../history/M0-T19-S3-dialog-closure.md)：当前原版真实点击正常；历史异常根因未查明，本S没有产品代码或二进制修改。

T20 S1 的阶段包已发布部分公共对话框和 COM 桥接，P231 默认文件夹新建保留旧式布局并修正选中目录及输入路径后的确认问题；P234 处理旧式列表系统主题动画覆盖项目滚动条，同源码增量候选的双宽真实拖动通过，干净包的双宽聚焦消息驱动测试及 x64 真实拖动通过，干净包 x86 真实鼠标复测因近期输入跳过。按用户最新指示暂停COM布局桥接及其长尾。事件桥接实验默认关闭，定制、隐藏目录及完整矩阵仍未完成；公开新式对话框 API 与 demo 路径已纳入阶段包。详见[阶段证据](../history/M0-T20-S1-preliminary.md)；S1收口，T20保持开放。

## Recent Closure

- M0 T20 S4：双宽干净构建、实际重建包40项通过；部署包字节未改，旧探针超时保留。[记录](../history/M0-T20-S4-common.md)。

- M0 T20 S3：Shell筛选与COM多选已安全回退，双宽构建、实际包测试及远端复核；[Shell](../history/M0-T20-S3-shell-filter.md)、[COM](../history/M0-T20-S3-com-safety.md)。

- M0 T20 S2：入口与回退矩阵已交付、双宽构建及实际包代表回归通过；[证据](../history/M0-T20-S2-dialog-audit.md)。

- M0 T20 S1：按用户指令阶段收口；已交付、已知缺口及后续承接见[记录](../history/M0-T20-S1-closure.md)，T20保持开放。

- M0 T19 S3 / T19：用户确认当前对话框恢复正常并要求收口；历史异常原因不明，字体试验已撤回。

- M0 T19 S2：按用户要求撤回全部字体试验并恢复P208原始四文件；[活动包归档](../history/M0-T19-S2-rollback-current-archive.md)。

- M0 T19 S1：Canon自绘按钮图像恢复，实际远端源码和四文件已复核；范围与限制见[交付记录](../history/M0-T19-S1-canon-ownerdraw.md)，[活动包归档](../history/M0-T19-S1-current-archive.md)。
- M0 T18 S64 / T18：按用户确认的常用可见控件范围收口；非视觉、应用自绘不处理，安全契约不足及长尾留TODO。首次焦点回归失败、重入负例不足、物理输入和完整矩阵未覆盖均保留记录。

2026-10-29 阶段包 P218 已构建、聚焦测试及双位置发布；完整矩阵和公共对话框桥接仍在进行，不代表 T20/S1 收口。失败、跳过与已知回退见阶段证据。
