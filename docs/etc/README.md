# Supporting Material Index

| 路径 | 责任、用途及保留条件 |
| --- | --- |
| [screenshots/README.md](screenshots/README.md) | 用户明确要求公开的精选demo/真实XP Solitaire截图及英文来源/摘要；非原始测试抓图 |
| [evidence/vdm-propagation-diagnosis.json](evidence/vdm-propagation-diagnosis.json) | 原生命令传播、冷/热/历史与双钩子独立诊断的精选状态/原始摘要；不替代产品包回归 |
| [evidence/native-cmd-staging.json](evidence/native-cmd-staging.json) | T9真实嵌套cmd、外部导入表正负例及脱敏进程图/模块SHA；完整身份不公开 |
| [evidence/test-efficiency.json](evidence/test-efficiency.json) | Td S9同集合串/并行、热增量、共享cache、完整回归和实际包耗时；仅公开计数/原始摘要 |
| [evidence/uac-elevation.json](evidence/uac-elevation.json) | T11双宽UAC、实际高令牌/窗口/子孙、负例、回归及未定位回滚计数失败；原始身份不公开 |
| [evidence/xaml-research.json](evidence/xaml-research.json) | 用户取消的XAML研究事实及原始摘要；无换肤或产品支持声明 |
| [evidence/legacy-iat-staging.json](evidence/legacy-iat-staging.json) | T12缺失IAT原生断点、双宽负例、真实DoubleKiller、创建对象身份判据及包验证证据 |
| [evidence/leaderboard-font.json](evidence/leaderboard-font.json) | Td S10双宽英雄榜同资源字体/墨迹/DPI对照及卸载恢复；无产品修改 |
| [evidence/multilingual-font.json](evidence/multilingual-font.json) | 已被T13 S2撤回的历史多语言控件字体保护：71项逐项覆盖、25实际包、旧DLL负例、真实扫雷/标题、发布及授权占用处置 |
| [evidence/multilingual-font-revert.json](evidence/multilingual-font-revert.json) | T13 S2原样撤回的源身份、加载映像对照、双宽回归及实际包/双发布证据 |
| [evidence/system-shell-launch.json](evidence/system-shell-launch.json) | 已由S2撤回的T14 S1历史系统UI方案及交付证据 |
| [evidence/legacy-retry-cleanup.json](evidence/legacy-retry-cleanup.json) | T14 S2清理二次Shell、首次stderr/193、原生cmd和自有树清理及发布审计 |
| [evidence/console-error-routing.json](evidence/console-error-routing.json) | T14 S3按控制台分流的原生UI/首次诊断、双旧版负例及双发布/审计证据 |
| [evidence/madshow-controls.json](evidence/madshow-controls.json) | Td S11真实VB6应用的控件分类/原生状态/附着断点、有限候选及产品未变证据 |
| [release.json](release.json) | 当前开发包的输入/工具链/四文件哈希与验收范围，历史版本从对应Git提交取回 |
| [product-contract.json](product-contract.json) | 架构权威的机器映射；组件/产物变化必须同步审核 |
| [import](import/README.md) | 来源基线与 Git blob 清单；只作导入对照，不能成为构建输入 |
| [operations/source-policy.md](operations/source-policy.md) | 导入来源处置及用户明确的原资源身份要求；完整保留原声明 |
| [evidence/m0-t445-s4-scrollbar-fidelity-audit.md](evidence/m0-t445-s4-scrollbar-fidelity-audit.md) | 原resources.md历史链接的来源定位，不作为本项目验收证据 |
| [operations/UAC.md](operations/UAC.md) | 按需提升方法、原生测试入口、环境/控制台边界及验收范围 |
| [operations/BUILD.md](operations/BUILD.md) | 本地构建、干净候选包、验证结果和使用/支持限制 |

任务完成证据进入 history；原始日志及测试抓图保留在任务 scratch，不提交。用户明确要求的精选README展示图作为文档资源按上表索引，原始诊断材料仍不提交。
