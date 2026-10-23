# Documentation Guide

这是唯一文档入口。一个主题只有一个当前权威；其他位置链接而不复制。

## Task Reading Set

每个 S 首次进入及恢复时阅读：本页、[CURRENT](states/CURRENT.md) 及其 brief、
[执行规则](rules/EXECUTION.md)、[Contributing](../CONTRIBUTING.md)。

- 规划、准入、关闭：加读 [QUEUE](states/QUEUE.md)、[TODO](states/TODO.md)、
  [路线图](design/ROADMAP.md)。
- 代码或构建：加读[架构](design/ARCHITECTURE.md)、[源码布局](design/CODING.md)、
  [架构规则](rules/ARCHITECTURE.md)、[编码规则](rules/CODING.md)。
- 文档或治理：加读[文档规则](rules/DOCUMENT.md)及变更的全部权威。
- UI：加读[界面设计](design/UI.md)；导入：加读[来源策略](etc/operations/source-policy.md)。

## Authority Map

| 位置 | 职责 |
| --- | --- |
| design/ | 当前产品目标、架构、源码布局、UI、M0 路线图 |
| rules/ | 可执行的过程、架构、编码和文档约束 |
| states/CURRENT.md | 唯一活动包、编号分配状态、当前技术基线 |
| states/QUEUE.md | 有序且未编号的候选链接 |
| states/TODO.md | 带优先级、责任与准入条件的延后债务 |
| proposals/ | 候选方案细节；准入后由活动包确定实际范围 |
| history/ | 已交付事实与审计记录，不能覆盖当前权威 |
| etc/ | 有索引的支持资料、证据、来源清单 |

项目参考 softpc 的同名文档拓扑；本仓库规则完整自包含，外部仓库或共享
skill 的安装与联网不是执行前提。外部参考不自动升级本仓库规则。
