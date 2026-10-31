# Project Status

## Current Work

M0 T20 S7：新旧多选契约及双宽干净包已验证，待提交推送与实际远端复核；保留原生回退。

## M0 T20 S7 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New；S6 P1已提交推送并读取实际远端复核 |
| Admission And Approval | 用户授权后续S自动准入，T20文件对话框桥接范围内 |
| Objective | 验证GetOpenFileName A/W多选在新旧布局的真实选择结果、长名及缓冲区契约，若安全可行则桥接 |
| Non-goals | 不扩展已降级的COM布局长尾，不降低多选语义，不改变原图形资源，不使用computer-use |
| Reference Baseline | S6 P1 ed45c47；S3 P2发布包；S2审计多选回退 |
| Candidate Proposal | [公共对话框适配](../proposals/classic-folder-dialogs.md) |
| Files And ABI Surface | 原生多选探针、必要时file_dialogs.c及包测试，无新增产品文件或ABI |
| Applicable Rules | M0、C17、MinGW GCC、Ninja、ccache、原资源逐字节保留 |
| Verification | 双宽A/W系统新旧布局对照、实际长名、多选结果与缓冲区、取消、超时清理；双宽干净包及回归 |
| Expected Markers | 区分布局、所选文件身份、返回分隔格式和错误，不能以窗口存在推定兼容 |
| Asset Needs | build/t20-s7及build/t20-package内自有scratch |
| Reporting Requirements | 先报告对照证据和最小方案；记录失败与未覆盖并按需发布、推送和远端复核 |
| Stop Conditions | 不能忠实保持语义时保留安全回退；不修改系统设置、共享宿主或兄弟仓库 |
| Exit Criteria | 真实契约可行性和限制有证据，合适时修复并完成交付；不能以回退冒充旧布局实现 |
| Original Owner Request | 自动准入执行后续S，不等待手工验收，优先常用对话框 |
| Similar-Issue Sweep | 核查A/W打开与保存共用宏、多选参数和所有结果写回点 |
## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 20
- Last allocated Td S: 15
- Last allocated T18 S: 64
- Last allocated T19 S: 3
- Last allocated T20 S: 7
- Last delivery: M0 T20 S3 P2：COM多选安全回退系统新框，双宽构建、实际包测试及双位置发布；Shell筛选P1同样采用安全回退。
- Open task: T20 S7 Win32文件多选旧布局可行性；T20 保持开放。
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

- M0 T20 S6：探针修订、双宽干净包26项及远端复核；临时目录旧Shell对照超时仍未解决。[证据](../history/M0-T20-S6-folder-probe.md)。

- M0 T20 S5：最终交接、双宽构建、部署包18项及实际路径探针通过；P1 87216a1 已推送并读取远端对象复核。T20 保持开放。[记录](../history/M0-T20-S5-handoff.md)。

- M0 T20 S4：双宽干净构建、实际重建包40项通过；部署包字节未改，旧探针超时保留。[记录](../history/M0-T20-S4-common.md)。

- M0 T20 S3：Shell筛选与COM多选已安全回退，双宽构建、实际包测试及远端复核；[Shell](../history/M0-T20-S3-shell-filter.md)、[COM](../history/M0-T20-S3-com-safety.md)。

- M0 T20 S2：入口与回退矩阵已交付、双宽构建及实际包代表回归通过；[证据](../history/M0-T20-S2-dialog-audit.md)。

- M0 T20 S1：按用户指令阶段收口；已交付、已知缺口及后续承接见[记录](../history/M0-T20-S1-closure.md)，T20保持开放。

- M0 T19 S3 / T19：用户确认当前对话框恢复正常并要求收口；历史异常原因不明，字体试验已撤回。

- M0 T19 S2：按用户要求撤回全部字体试验并恢复P208原始四文件；[活动包归档](../history/M0-T19-S2-rollback-current-archive.md)。

- M0 T19 S1：Canon自绘按钮图像恢复，实际远端源码和四文件已复核；范围与限制见[交付记录](../history/M0-T19-S1-canon-ownerdraw.md)，[活动包归档](../history/M0-T19-S1-current-archive.md)。
- M0 T18 S64 / T18：按用户确认的常用可见控件范围收口；非视觉、应用自绘不处理，安全契约不足及长尾留TODO。首次焦点回归失败、重入负例不足、物理输入和完整矩阵未覆盖均保留记录。

2026-10-29 阶段包 P218 已构建、聚焦测试及双位置发布；完整矩阵和公共对话框桥接仍在进行，不代表 T20/S1 收口。失败、跳过与已知回退见阶段证据。
