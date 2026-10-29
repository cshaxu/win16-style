# Project Status

## Current Work

无活动任务包。用户确认MadShow对话框已恢复，并要求收口S3和T19。

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 19
- Last allocated Td S: 15
- Last allocated T18 S: 64
- Last allocated T19 S: 3
- Last delivery: M0 T19 S2 P214，撤回提交940a8d42bfda18da413577319e72970102d52156，实际远端源码和P208原始四文件已核对。
- Open task: 无；T19 S3及T19按用户要求收口。

## Technical Baseline

三个组件、四产品文件；纯C、GCC、Ninja、ccache，原图形资源及来源保持。发布位置为项目assets根目录和O:/winnt根目录。当前有限控件契约见[界面设计](../design/UI.md)，启动/架构见[架构](../design/ARCHITECTURE.md)，具体覆盖及限制见[收口审计](../history/M0-T18-scoped-closure-audit.md)。T19 S1双宽干净包、两处部署摘要、18项聚焦回归和真实程序记录见[交付证据](../history/M0-T19-S1-canon-ownerdraw.md)。历史完整活动包保存在[归档](../history/M0-T18-S64-current-archive.md)，只作历史记录。

用户否决S2两次试用；S1 P208源码和原始四文件已恢复并经实际远端复核。见[撤回记录](../history/M0-T19-S2-rollback.md)。

S3调查结果见[收口记录](../history/M0-T19-S3-dialog-closure.md)：当前原版真实点击正常；历史异常根因未查明，本S没有产品代码或二进制修改。

## Recent Closure

- M0 T19 S3 / T19：用户确认当前对话框恢复正常并要求收口；历史异常原因不明，字体试验已撤回。

- M0 T19 S2：按用户要求撤回全部字体试验并恢复P208原始四文件；[活动包归档](../history/M0-T19-S2-rollback-current-archive.md)。

- M0 T19 S1：Canon自绘按钮图像恢复，实际远端源码和四文件已复核；范围与限制见[交付记录](../history/M0-T19-S1-canon-ownerdraw.md)，[活动包归档](../history/M0-T19-S1-current-archive.md)。
- M0 T18 S64 / T18：按用户确认的常用可见控件范围收口；非视觉、应用自绘不处理，安全契约不足及长尾留TODO。首次焦点回归失败、重入负例不足、物理输入和完整矩阵未覆盖均保留记录。
