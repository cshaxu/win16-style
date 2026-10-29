# Project Status

## Current Work

M0 T19 S2续作：用户否决两次字体试用，要求全部撤回并恢复T19 S1原始四文件。T19保持开放。

## M0 T19 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation；用户要求撤回本S两次试用，恢复已发布T19 S1 P208 |
| Admission And Approval | 用户明确要求撤回全部字体试用并发布最初版本 |
| Objective | 还原T19 S1字体实现、S2新增测试与构建配置，并发布P208字节完全一致的四文件 |
| Non-goals | 不变更标题、菜单、原始图形资源、应用布局或应用自绘 |
| Reference Baseline | T19 S1 P208四文件和当前字体实现 |
| Candidate Proposal | [多字号位图字形](../proposals/system-bitmap-strikes.md)及[字体布局](../proposals/font-layout.md)，本S以用户新要求覆盖控件字形缩放策略 |
| Files And ABI Surface | 现有font.c及必要的C探针、测试和文档；不新增公共ABI或产品 |
| Applicable Rules | M0，纯C、MinGW、Ninja、ccache，原资源字节保持 |
| Verification | 与S1代码和测试逐字节对照；双宽干净构建、原始包实测及两处SHA核对 |
| Expected Markers | 字体、原图形资源及原始四文件与P208一致；Canon图标修复保留 |
| Asset Needs | 登记build/t19-s2-rollback及build/t19-package任务scratch；完成后提炼并清理 |
| Reporting Requirements | 撤回范围、原包SHA、双宽构建与实际原包测试、双发布和远端复核 |
| Stop Conditions | 原始包身份、占用或发布验证失败时停止并保留证据 |
| Exit Criteria | 重建并测试回退源码，实际核对原始P208四文件、发布及远端复核；仅关闭S2，保持T19开放 |
| Original Owner Request | 用户不满意字体试用，要求全部撤回并发布最初版本 |
| Similar-Issue Sweep | 区分菜单、标题、VB及标准控件字体获取路径，保留应用自绘 |

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 19
- Last allocated Td S: 15
- Last allocated T18 S: 64
- Last allocated T19 S: 2
- Last delivery: M0 T19 S2 P212，产品实现提交79ebe63abf113f059ec5e3bb398603cb6a05e45f，实际远端源码和四文件已核对。
- Open task: T19 S2 System位图等比例缩小试用。

## Technical Baseline

三个组件、四产品文件；纯C、GCC、Ninja、ccache，原图形资源及来源保持。发布位置为项目assets根目录和O:/winnt根目录。当前有限控件契约见[界面设计](../design/UI.md)，启动/架构见[架构](../design/ARCHITECTURE.md)，具体覆盖及限制见[收口审计](../history/M0-T18-scoped-closure-audit.md)。T19 S1双宽干净包、两处部署摘要、18项聚焦回归和真实程序记录见[交付证据](../history/M0-T19-S1-canon-ownerdraw.md)。历史完整活动包保存在[归档](../history/M0-T18-S64-current-archive.md)，只作历史记录。

用户否决S2两次试用；S1 P208源码和原始四文件已完成本地恢复、原包回归及双位置发布，待提交与远端复核。见[撤回记录](../history/M0-T19-S2-rollback.md)。

## Recent Closure

- M0 T19 S1：Canon自绘按钮图像恢复，实际远端源码和四文件已复核；范围与限制见[交付记录](../history/M0-T19-S1-canon-ownerdraw.md)，[活动包归档](../history/M0-T19-S1-current-archive.md)。
- M0 T18 S64 / T18：按用户确认的常用可见控件范围收口；非视觉、应用自绘不处理，安全契约不足及长尾留TODO。首次焦点回归失败、重入负例不足、物理输入和完整矩阵未覆盖均保留记录。
