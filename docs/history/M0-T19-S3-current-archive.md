# M0 T19 S3 活动包归档

## M0 T19 S3 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New；T19续作S3，S1和S2已关闭 |
| Admission And Approval | 用户报告并准入MadShow Copyright消息框样式回归，称曾正确处理 |
| Objective | 原生探针复现并定位消息框未样式化原因，验证历史后修复 |
| Non-goals | 不改字体策略、标题菜单资源、兄弟仓库，不操作已有用户进程 |
| Reference Baseline | P208原始四文件，S2撤回后原样发布 |
| Candidate Proposal | [MadShow控件](../proposals/madshow-controls.md)和[字体布局](../proposals/font-layout.md)既有契约 |
| Files And ABI Surface | 现有注入及窗口附着代码、纯C原生探针；不增加产品或ABI |
| Applicable Rules | M0，C17、MinGW、Ninja、ccache，原图形资源逐字节保持 |
| Verification | 独立自有MadShow实例，真实消息框、创建路径、附着和绘制对照，双宽回归及实际包测试 |
| Expected Markers | Copyright消息框及标准控件应用样式，返回与关闭语义保持，旧版负例和历史对照如实记录 |
| Asset Needs | build/t19-s3-dialog及build/t19-package任务scratch，证据提炼后清理 |
| Reporting Requirements | 回归事实、根因、覆盖与未覆盖、测试、双发布及远端复核 |
| Stop Conditions | 无法安全证明窗口归属或功能语义则不发布猜测修复 |
| Exit Criteria | 原生实证修复、同批双宽包测试、双发布、提交推送复核；保持T19开放 |
| Original Owner Request | MadShow Copyright弹出对话框以前正确，现在未采用Win16样式 |
| Similar-Issue Sweep | MessageBox A/W/Ex/Indirect、dialog创建、VB运行库创建路径和重入 |
