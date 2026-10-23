# Project Status

## Current Work

M0 Td S1 已准入：建立治理体系并准备导入。状态 verified，待提交推送与交付后复核。
产品源码导入尚未准入，无活动实现 T。

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 0
- Last allocated Td S: 1
- Last delivery: none

## Technical Baseline

远端初始基线 4666cd745da53bf16067365032f4a22a09a12e58 仅有 MIT LICENSE。
本次只新增治理、检查和来源元数据。没有产品源码、构建或运行验收。
参考 softpc 889b309aabb24d3b4748dcc209ec520a3c35c375 的治理拓扑；
导入候选固定 ntvdm64 5b0b65f66c281050aa4ce1454c6cd31441a6d484。
其工作区有无关未提交修改，本项目未复制其工作区内容。

## M0 Td S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Governance；首次 M0 Td S1，不分配 T |
| Admission And Approval | 用户 2026-10-23 请求建立整套治理、MTSP、准备导入并推送指定仓库 |
| Objective | 建立自包含权威、固定三组件四产物、检查门禁及可执行导入候选 |
| Non-goals | 不导入产品源码、不构建/注入运行、不修改兄弟仓库、不发布二进制 |
| Reference Baseline | 本页 Technical Baseline 及来源 inventory |
| Candidate Proposal | [导入准备](../proposals/native-style-import.md)，仅准备不准入实现 |
| Files And ABI Surface | 根入口、docs、tools、CI、assets 包说明；无产品 ABI 变化 |
| Applicable Rules | 全部 rules 与设计权威；来源策略 |
| Verification | 治理正向及负例自检、文档链接、产品映射、来源 blob 清单、diff --check、实际 diff 复核、远端 HEAD |
| Expected Markers | Governance PASS；SelfTest PASS；产品产物未实现；远端提交与本地一致 |
| Asset Needs | 无二进制/图形资产；仅文件名与 Git blob 元数据 |
| Reporting Requirements | 汇报提交、验证、导入候选与已知前置条件；不声称产品完成 |
| Stop Conditions | 远端并发冲突、无法确认来源、越界要求；保留当前成果并记录具体原因 |
| Exit Criteria | 权威与有限准备清单逐项完成、验证通过、推送、实际变更复核及后续治理 P 记录 |
| Original Owner Request | 比照隔壁 softpc 建立整套治理；MTSP；分离 run16 --win16-style-all；3 组件 4 产物；先治理后准备导入；总是 milestone0；推送 https://github.com/cshaxu/win16-style |
| Similar-Issue Sweep | 检查全部入口、链接、M0/MTSP、四产物映射和候选编号；发现源 demo 主动着色、Hook 服务依赖、资源发布限制，均转导入方案显式处置 |
