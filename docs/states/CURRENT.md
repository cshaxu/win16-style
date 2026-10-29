# Project Status

## Current Work

M0 T19 S2按用户请求准入：试用System位图控件字体等比例缩小，并交付同批四文件。T19保持开放。

## M0 T19 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New；T19续作S2，S1已关闭 |
| Admission And Approval | 用户要求试用System位图字体等比例缩小并编译发布 |
| Objective | 在应用原字符advance及行高内等比例绘制System位图字形，供MadShow验收 |
| Non-goals | 不变更标题、菜单、原始图形资源、应用布局或应用自绘 |
| Reference Baseline | T19 S1 P208四文件和当前字体实现 |
| Candidate Proposal | [字体布局](../proposals/font-layout.md)，本S以用户新要求覆盖控件字形缩放策略 |
| Files And ABI Surface | 现有font.c及必要的C探针、测试和文档；不新增公共ABI或产品 |
| Applicable Rules | M0，纯C、MinGW、Ninja、ccache，原资源字节保持 |
| Verification | 双位宽字形比例、advance与行高、菜单保留、VB及标准控件、实际包测试 |
| Expected Markers | 控件字符格内等比例缩放；不裁切；菜单标题路径保持；探针检出旧实现 |
| Asset Needs | 登记build/t19-s2-font和build/t19-package任务scratch；完成后提炼并清理 |
| Reporting Requirements | 精确算法、测试与未覆盖项、两处发布和远端复核；视觉效果待用户试用 |
| Stop Conditions | 字体生命周期或布局契约无法验证时不得发布 |
| Exit Criteria | 双宽同批包测试、发布、提交推送复核；仅关闭S2，保持T19开放 |
| Original Owner Request | 等比例缩小System位图字形，编译四文件发布O:/winnt试用 |
| Similar-Issue Sweep | 区分菜单、标题、VB及标准控件字体获取路径，保留应用自绘 |

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 19
- Last allocated Td S: 15
- Last allocated T18 S: 64
- Last allocated T19 S: 2
- Last delivery: M0 T19 S1 P208，产品实现提交8799eaee313ac3d830cff4f8d39b55836000ec9e，实际远端源码和四文件已核对。
- Open task: T19 S2 System位图等比例缩小试用。

## Technical Baseline

三个组件、四产品文件；纯C、GCC、Ninja、ccache，原图形资源及来源保持。发布位置为项目assets根目录和O:/winnt根目录。当前有限控件契约见[界面设计](../design/UI.md)，启动/架构见[架构](../design/ARCHITECTURE.md)，具体覆盖及限制见[收口审计](../history/M0-T18-scoped-closure-audit.md)。T19 S1双宽干净包、两处部署摘要、18项聚焦回归和真实程序记录见[交付证据](../history/M0-T19-S1-canon-ownerdraw.md)。历史完整活动包保存在[归档](../history/M0-T18-S64-current-archive.md)，只作历史记录。

S2试用包已完成本地构建、聚焦测试及双位置发布；等待提交和远端复核，外观等待用户试用。见[交付记录](../history/M0-T19-S2-proportional-font.md)。

## Recent Closure

- M0 T19 S1：Canon自绘按钮图像恢复，实际远端源码和四文件已复核；范围与限制见[交付记录](../history/M0-T19-S1-canon-ownerdraw.md)，[活动包归档](../history/M0-T19-S1-current-archive.md)。
- M0 T18 S64 / T18：按用户确认的常用可见控件范围收口；非视觉、应用自绘不处理，安全契约不足及长尾留TODO。首次焦点回归失败、重入负例不足、物理输入和完整矩阵未覆盖均保留记录。
