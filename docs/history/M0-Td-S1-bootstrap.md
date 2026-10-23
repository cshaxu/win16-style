# M0 Td S1 治理建立与导入准备

2026-10-23。用户授权范围：比照 softpc 建立整套治理、MTSP、固定 M0、三组件
四产物、准备分离 ntvdm64 的 --win16-style-all，并推送 win16-style 仓库。
状态 closed：P1 推送后完成实际 commit 复核，P2 记录本次治理与准备关闭。

## 变更与有限覆盖

权威拓扑、任务生命周期、编号、单活动包、来源策略、包边界、UI 行为、源码布局、
候选拆分与验收矩阵已经落盘。机器契约及治理检查器负责结构一致性，CI 执行
相同命令。本次没有产品 ABI 或生产源码变更。

准备清单七单位的接收文件见[候选](../proposals/native-style-import.md)。
冻结来源只记录路径、Git blob 与大小，不复制原始第三方图形或二进制。
候选说明已接收原生 demo 改造、服务去耦、双 DLL 合并、跨位宽语义与资源来源
问题；没有声称这些问题已实现或修复。

## 验证与交付

`pwsh -NoProfile -File tools/Verify-Governance.ps1 -SelfTest`：正向通过，
覆盖缺权威、错误拓扑、断链、多活动包、非法包标识、缺字段、非零 milestone、
错误产品、demo 自行链接样式、提前导入源码、候选预编号、孤立候选与错误来源。
检查器最初把当前 Packet 标题本身当成分隔符，正向验证暴露该错误；现已修正为
从标题后读取表格，负例自检同时证明非空 packet 的字段验证实际生效。

来源 inventory 共 105 个固定提交条目；由 git ls-tree 直接生成，包含 mode、
blob 与大小，不包含源码字节。`git diff --check` 通过。
无产品构建或运行验收；未启动产品进程，未产生 build scratch。
规则门禁的通过仅证明结构。首次 Git 暂存受 .git 写权限限制，改用授权提升执行；
无自动审批拒绝或产品工作阻塞。

## 实际提交复核与关闭

P1 为 `bd013545d5fd81423c20154c8ba39811c579981e`，提交标题
`M0 Td S1 P1: establish governance and prepare native style import`。
git push origin main 成功，git ls-remote origin refs/heads/main 精确匹配该提交。
原始完整活动包保留在 P1 的 docs/states/CURRENT.md 中。

交付后切换协调复核角色，读取 git show 的实际路径、统计、设计/规则/候选差异，
对照用户原始请求与准备七单位：范围未扩大，M0 固定、MTSP 和单活动包完整，
三组件四产物与 demo 双行为明确，无产品源码/资产字节被提前导入。
P1 共 28 文件 +843/-0；生产代码和产品测试改动均为 0，治理脚本 +188 行。
既有 LICENSE 不变，没有旧生产路径待删除；没有生成 build 目录。

对 105 个 inventory 条目再次独立读取完整固定提交 tree，每条 mode/type/blob/
size/path 均精确匹配（105/105）。git show --check 和干净工作区检查通过。
本地正向及 13 个拒绝用例通过；CI 工作流已经配置，其云端执行状态未在本次
本地验证中声称通过。没有发现交付阻塞，按原请求完成条件关闭 Td S1；
关闭不等于 M0 产品完成，也不准入导入候选。

P2 仅更新 CURRENT 与本记录，移除过期活动包、保留编号状态和候选；
再次通过空闲 CURRENT 的治理检查及全部 13 个负例。后续执行须重新准入
实现任务，首个数值任务可从 T1 开始。
