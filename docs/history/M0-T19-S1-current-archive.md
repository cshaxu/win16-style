# M0 T19 S1 — 活动包归档

## M0 T19 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New；用户明确准入T19 S1处理Canon MF Toolbox图标消失，完成后仅关闭S1、保持T19开放 |
| Admission And Approval | 用户提供原版与run16s截图和启动命令，授权调查及必要修复；先取证再决定实现 |
| Objective | 定位样式启动后原有扫描图标变为空白方框的首个可验证原因，保持应用图片和交互 |
| Non-goals | 不扫描、不操作用户现有实例或设备，不改变应用或系统安装、图形资源及兄弟仓库 |
| Reference Baseline | P207已发布双宽四文件；[T18收口审计](../history/M0-T18-scoped-closure-audit.md) |
| Candidate Proposal | [Delphi控件适配](../proposals/delphi-win32-controls.md)的图形/应用自绘保留契约，本次依据真实MFC故障纠正 |
| Files And ABI Surface | 自有原生C探针、测试及必要现有DLL修复；不增加公共ABI或产品文件 |
| Applicable Rules | M0、C17、GCC、Ninja、ccache；原资源字节不变；仅自有有界实例 |
| Verification | 原版/样式对照，控件身份、图像句柄和绘制路径；修复需双宽负例、实际包、应用复测 |
| Expected Markers | 图像真实显示并保留原句柄、像素及行为；无法确认则回退原生 |
| Asset Needs | 登记build/t19-s1-canon自有诊断、样本副本和包证据，结束前提炼证据并安全清理 |
| Reporting Requirements | 实际根因、路径及摘要、测试失败、未覆盖面、发布和远端复核 |
| Stop Conditions | 无法安全构造自有实例或核实图片所有权则保留证据，不修改用户实例 |
| Exit Criteria | 经证据修复并按执行规范发布，或有明确诊断与后续接收；保持原应用图像 |
| Original Owner Request | run16s启动Canon MF Toolbox时图标消失，原版图标正常 |
| Similar-Issue Sweep | 原生BUTTON、STATIC、owner-draw、自绘与其他图像控件，避免破坏既有图形按钮 |

