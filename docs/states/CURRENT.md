# Project Status

## Current Work

无活动任务包。M0 T18 S64及T18已按用户于本次收尾明确批准的有限范围关闭；并非所有VB/Delphi控件、版本或变体均已适配。实际覆盖、验收与剩余项见[收口审计](../history/M0-T18-scoped-closure-audit.md)及[TODO](TODO.md)。不自动准入下一T；待办顺序见[QUEUE](QUEUE.md)。

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 18
- Last allocated Td S: 15
- Last allocated T18 S: 64
- Last delivery: M0 T18 S64 P207，产品实现提交478dc307148ea6dc336cab3eb1497d1559bb6094，实际远端已核对。
- Open task: 无活动任务包。

## Technical Baseline

三个组件、四产品文件；纯C、GCC、Ninja、ccache，原图形资源及来源保持。发布位置为项目assets根目录和O:/winnt根目录。当前有限控件契约见[界面设计](../design/UI.md)，启动/架构见[架构](../design/ARCHITECTURE.md)，具体覆盖及限制见[收口审计](../history/M0-T18-scoped-closure-audit.md)。P207双宽干净包及两处部署摘要、22项聚焦回归和真实程序记录见[交付证据](../history/M0-T18-S64-runtime-lists.md)。历史完整活动包保存在[归档](../history/M0-T18-S64-current-archive.md)，只作历史记录。

## Recent Closure

- M0 T18 S64 / T18：按用户确认的常用可见控件范围收口；非视觉、应用自绘不处理，安全契约不足及长尾留TODO。首次焦点回归失败、重入负例不足、物理输入和完整矩阵未覆盖均保留记录。
