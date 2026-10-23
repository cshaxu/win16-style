# Documentation Rules

docs/ 直属目录恰为 design、rules、states、proposals、history、etc。
design 文件恰为 GOAL、ARCHITECTURE、CODING、UI、ROADMAP（.md）；
rules 恰为 EXECUTION、ARCHITECTURE、CODING、DOCUMENT（.md）；
states 恰为 CURRENT、QUEUE、TODO（.md）。
权威职责以[入口](../README.md)为准；根 README、AGENTS、CONTRIBUTING 是指针。

CURRENT 保持紧凑，保留技术基线、编号状态、至多一个活动包和近期交付链接。
QUEUE 只含有序未编号候选；每个候选链接 proposals 文件，不承诺开始时间。
TODO 只含未解决债务：优先级、责任、证据和准入条件。历史事实不能回写成当前
实现能力，导入准备不能标注为产品已构建。支持资料必须在 etc/README 索引。

治理脚本检查拓扑、局部链接、候选引用、活动包形状、M0 及产品映射。
修改规则和检查器时保持一致，并用负例证明门禁可拒绝违反项。
