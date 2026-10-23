# Supporting Material Index

| 路径 | 责任、用途及保留条件 |
| --- | --- |
| [release.json](release.json) | 当前开发包的输入/工具链/四文件哈希与验收范围，历史版本从对应Git提交取回 |
| [product-contract.json](product-contract.json) | 架构权威的机器映射；组件/产物变化必须同步审核 |
| [import](import/README.md) | 来源基线与 Git blob 清单；只作导入对照，不能成为构建输入 |
| [operations/source-policy.md](operations/source-policy.md) | 导入来源处置程序；声明/权利未确认的文件不进入发布包 |
| [operations/BUILD.md](operations/BUILD.md) | 本地构建、干净候选包、验证结果和使用/支持限制 |

任务完成证据进入 history；原始日志及抓图保留在任务 scratch，不提交。
