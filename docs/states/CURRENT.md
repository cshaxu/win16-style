# Project Status

## Current Work

M0 T20 S1准入：Win3.x文件夹选择与公共对话框桥接：调查、实现与验证进行中。

## M0 T20 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New；用户准入队列首位，T20 S1 |
| Admission And Approval | 用户要求T19收口并准入文件夹选择与公共对话框任务 |
| Objective | 用户明确两维标准：系统存在旧式布局时桥接，所有对话框接管样式；覆盖公共对话框及相关对话框入口，逐项测试并继续实现COM等缺口；用户要求系统审计并修复水平和垂直滚动条滚动后样式变化 |
| Non-goals | 不分发旧系统DLL，不更改原图形资源、共享Shell宿主、系统设置或用户现有进程 |
| Reference Baseline | T19 S1 P208原始四文件，S2撤回，S3用户确认恢复 |
| Candidate Proposal | [Win3.x文件夹选择与公共对话框适配](../proposals/classic-folder-dialogs.md) |
| Files And ABI Surface | 纯C原生探针及必要的现有DLL模块，维持三组件四产物；按用户要求增加公开的强制新式文件对话框 API，供任意主动调用者使用 |
| Applicable Rules | M0，MinGW GCC、Ninja、ccache；原资源与来源声明逐字节保留 |
| Verification | 真实目标、自有有限实例、调用接口与参数、PIDL/路径/回调、原始经典参考及双宽对照 |
| Expected Markers | 双宽实际对话框、选择结果、取消、回调及不支持情况回退均有证据；不以外观或模块加载证明功能完成 |
| Asset Needs | 登记build/t20-s1-dialog及build/t20-package scratch；成果提炼后清理 |
| Reporting Requirements | 实际证据、支持和回退矩阵、许可、不可兼容条件及可行路线 |
| Stop Conditions | 无法保证调用语义与资源许可时保留原对话框，不伪造结果 |
| Exit Criteria | 真实入口及实现范围有证据；产品变更依规范构建、测试、双发布、推送复核；T20后续继续COM等未完成范围 |
| Original Owner Request | 准入队列首位，处理Win3.x文件夹选择与公共对话框外观 |
| Similar-Issue Sweep | SHBrowseForFolder、GetOpenFileName、GetSaveFileName、IFileDialog及框架自绘 |

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 20
- Last allocated Td S: 15
- Last allocated T18 S: 64
- Last allocated T19 S: 3
- Last allocated T20 S: 1
- Last delivery: M0 T20 S1 P233 阶段包已完成双宽构建、聚焦包测试及双位置发布；提交和远端复核以实际结果为准。
- Open task: T20 S1 文件夹选择和公共对话框接口及经典参考调查。

## Technical Baseline

三个组件、四产品文件；纯C、GCC、Ninja、ccache，原图形资源及来源保持。发布位置为项目assets根目录和O:/winnt根目录。当前有限控件契约见[界面设计](../design/UI.md)，启动/架构见[架构](../design/ARCHITECTURE.md)，具体覆盖及限制见[收口审计](../history/M0-T18-scoped-closure-audit.md)。T19 S1双宽干净包、两处部署摘要、18项聚焦回归和真实程序记录见[交付证据](../history/M0-T19-S1-canon-ownerdraw.md)。历史完整活动包保存在[归档](../history/M0-T18-S64-current-archive.md)，只作历史记录。

用户否决S2两次试用；S1 P208源码和原始四文件已恢复并经实际远端复核。见[撤回记录](../history/M0-T19-S2-rollback.md)。

S3调查结果见[收口记录](../history/M0-T19-S3-dialog-closure.md)：当前原版真实点击正常；历史异常根因未查明，本S没有产品代码或二进制修改。

T20 S1 的阶段包已发布部分公共对话框和 COM 桥接，P231 默认文件夹新建保留旧式布局并修正选中目录及输入路径后的确认问题；用户反馈滚动条交互回退仍未解决，事件桥接实验默认关闭，部分定制、隐藏目录及完整对话框矩阵仍未完成，公开新式对话框 API 与 demo 路径已纳入阶段包。详见[阶段证据](../history/M0-T20-S1-preliminary.md)；T20 和 S1 保持开放。

## Recent Closure

- M0 T19 S3 / T19：用户确认当前对话框恢复正常并要求收口；历史异常原因不明，字体试验已撤回。

- M0 T19 S2：按用户要求撤回全部字体试验并恢复P208原始四文件；[活动包归档](../history/M0-T19-S2-rollback-current-archive.md)。

- M0 T19 S1：Canon自绘按钮图像恢复，实际远端源码和四文件已复核；范围与限制见[交付记录](../history/M0-T19-S1-canon-ownerdraw.md)，[活动包归档](../history/M0-T19-S1-current-archive.md)。
- M0 T18 S64 / T18：按用户确认的常用可见控件范围收口；非视觉、应用自绘不处理，安全契约不足及长尾留TODO。首次焦点回归失败、重入负例不足、物理输入和完整矩阵未覆盖均保留记录。

2026-10-29 阶段包 P218 已构建、聚焦测试及双位置发布；完整矩阵和公共对话框桥接仍在进行，不代表 T20/S1 收口。失败、跳过与已知回退见阶段证据。
