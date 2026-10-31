# Project Status

## Current Work

M0 T20 S2：公共对话框入口、转接及回退审计已验证，等待提交推送及复核。

## M0 T20 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New；T20 S1按用户指令阶段收口 |
| Admission And Approval | 用户授权自动准入后续S，直至T20收口前 |
| Objective | 冻结常见公共对话框入口与覆盖单位，逐项审计新旧布局、转接、接管、回退、已知缺陷及证据，按ROI交付后续S输入 |
| Non-goals | 不穷举参数组合，不扩展COM布局长尾，不宣称全部Windows窗口均已接管 |
| Reference Baseline | P295同批四文件及S1阶段证据；S1收口提交6e4c63b |
| Candidate Proposal | [公共对话框适配及后续S序列](../proposals/classic-folder-dialogs.md) |
| Files And ABI Surface | 审计文档与必要的自包含验证；不计划新增产品ABI |
| Applicable Rules | M0、C17、MinGW GCC、Ninja、ccache；原资源及来源逐字节保持 |
| Verification | 源码门控、现有测试与已发布包证据对照；双宽干净构建、同批四文件实际包测试、远端复核 |
| Expected Markers | 每类入口都有明确支持、回退、缺陷或未知；历史与最新包证据不混淆 |
| Asset Needs | 登记build/t20-s2-audit及build/t20-package scratch |
| Reporting Requirements | 按ROI输出剩余项目、实际证据和后续承接；未验证明确记录 |
| Stop Conditions | 不伪造覆盖，不越过已有活动包或修改兄弟仓库；不可安全测试的项目记录限制 |
| Exit Criteria | 审计范围、逐项矩阵、缺陷与后续S承接完成；规定的包测试、提交推送及远端复核完成 |
| Original Owner Request | S1收口并自动执行后续S，停止于T20收口前 |
| Similar-Issue Sweep | 文件打开保存、Shell与COM文件夹、Find/Replace、Font/Color、Print/PrintEx/PageSetup、Message/TaskDialog/PropertySheet及跨进程系统错误框 |

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 20
- Last allocated Td S: 15
- Last allocated T18 S: 64
- Last allocated T19 S: 3
- Last allocated T20 S: 2
- Last delivery: M0 T20 S1 P295 将无回调、带路径输入校验且禁止新建的 Shell 目录选择转接至系统旧树；双宽干净构建、实际包测试及双位置发布。文件夹筛选回调缺口仍未解决。
- Open task: T20 S2 公共对话框入口与回退审计；[矩阵与验证](../history/M0-T20-S2-dialog-audit.md)。
- 打印当前页已纳入受限 PrintDlgEx 转接；跨 DPI、实际打印和本地化仍待覆盖。
- 并发缺口：同进程两个 COM 文件对话框的系统原生双宽对照通过，已发布旧框转接双宽失败；目录串用及超时的具体根因仍待定位。
- 已知多选缺口：O: 自有目录中，COM 多选旧框不列出部分带空格的长文件名；不能仅增加列表项，结果转换也需处理。
- 已知回调缺口：现有 BIF_USENEWUI 带回调桥接未发送系统原有 BFFM_IUNKNOWN，可能影响目录自定义筛选；新式无编辑框带回调路径暂回退系统，旧树转接及筛选语义仍待解决。详见阶段证据。
- P253 已发布：干净包 58 项通过；系统打印宿主自然更替后，已发布包及无注入对照的双宽 A/W Print/Apply 实际虚拟打印各八项通过。此前 32 位 EndPage 失败的原因未明；详见阶段证据。

## Technical Baseline

三个组件、四产品文件；纯C、GCC、Ninja、ccache，原图形资源及来源保持。发布位置为项目assets根目录和O:/winnt根目录。当前有限控件契约见[界面设计](../design/UI.md)，启动/架构见[架构](../design/ARCHITECTURE.md)，具体覆盖及限制见[收口审计](../history/M0-T18-scoped-closure-audit.md)。T19 S1双宽干净包、两处部署摘要、18项聚焦回归和真实程序记录见[交付证据](../history/M0-T19-S1-canon-ownerdraw.md)。历史完整活动包保存在[归档](../history/M0-T18-S64-current-archive.md)，只作历史记录。

用户否决S2两次试用；S1 P208源码和原始四文件已恢复并经实际远端复核。见[撤回记录](../history/M0-T19-S2-rollback.md)。

S3调查结果见[收口记录](../history/M0-T19-S3-dialog-closure.md)：当前原版真实点击正常；历史异常根因未查明，本S没有产品代码或二进制修改。

T20 S1 的阶段包已发布部分公共对话框和 COM 桥接，P231 默认文件夹新建保留旧式布局并修正选中目录及输入路径后的确认问题；P234 处理旧式列表系统主题动画覆盖项目滚动条，同源码增量候选的双宽真实拖动通过，干净包的双宽聚焦消息驱动测试及 x64 真实拖动通过，干净包 x86 真实鼠标复测因近期输入跳过。按用户最新指示暂停COM布局桥接及其长尾。事件桥接实验默认关闭，定制、隐藏目录及完整矩阵仍未完成；公开新式对话框 API 与 demo 路径已纳入阶段包。详见[阶段证据](../history/M0-T20-S1-preliminary.md)；S1收口，T20保持开放。

## Recent Closure

- M0 T20 S1：按用户指令阶段收口；已交付、已知缺口及后续承接见[记录](../history/M0-T20-S1-closure.md)，T20保持开放。

- M0 T19 S3 / T19：用户确认当前对话框恢复正常并要求收口；历史异常原因不明，字体试验已撤回。

- M0 T19 S2：按用户要求撤回全部字体试验并恢复P208原始四文件；[活动包归档](../history/M0-T19-S2-rollback-current-archive.md)。

- M0 T19 S1：Canon自绘按钮图像恢复，实际远端源码和四文件已复核；范围与限制见[交付记录](../history/M0-T19-S1-canon-ownerdraw.md)，[活动包归档](../history/M0-T19-S1-current-archive.md)。
- M0 T18 S64 / T18：按用户确认的常用可见控件范围收口；非视觉、应用自绘不处理，安全契约不足及长尾留TODO。首次焦点回归失败、重入负例不足、物理输入和完整矩阵未覆盖均保留记录。

2026-10-29 阶段包 P218 已构建、聚焦测试及双位置发布；完整矩阵和公共对话框桥接仍在进行，不代表 T20/S1 收口。失败、跳过与已知回退见阶段证据。
