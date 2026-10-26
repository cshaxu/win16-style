# Ordered Candidates

这是 T 级任务队列：每一行是可独立准入、交付和关闭的 T 候选，各自再分 S。
全部属于 M0。以下候选待准入，未分配数值 T；顺序号不属于 MTSP 标识。
CURRENT 是唯一活动工作权威；本文件拥有顺序，各 proposal 拥有本 T 的 S 分解。
[导入总览](../proposals/native-style-import.md)保留共同来源边界和跨任务验收矩阵。
已完成的[来源冻结与独立化设计](../proposals/source-baseline.md)不在候选表中，证据见其完成记录。
已完成的[双位宽样式核心](../proposals/style-core.md)不在候选表中，证据见其完成审计。

| 顺序 | T 候选 | 前置完成条件 | 核心交付/任务级退出条件 |
| --- | --- | --- | --- |
| 1 | [Delphi Win32控件完整审计与Win3.x风格适配](../proposals/delphi-win32-controls.md) | 用户要求首位排队；正式准入后先冻结版本/框架/控件/变体全集与真实样本，不覆盖当前T17 | 沿用VB5/VB6专项方式：全貌审计、demo对应展示、分S实施、独立全集收口；保留数据/事件/图像/资源所有权，原生证据、同批四文件交付及真实应用验收 |
| 2 | [Win3.x 文件夹选择与公共对话框适配](../proposals/classic-folder-dialogs.md) | 用户仅要求排队；正式准入后先确认真实调用接口、经典资源/布局依据及回调/返回语义 | 在现有双宽DLL内有限适配经典目录选择布局与图标，保留路径/PIDL/对象、取消/回调及所有权；不分发旧系统DLL，原生证据、回退矩阵及四文件交付 |
| 3 | [run16s 登录全局样式模式](../proposals/global-style-mode.md) | 用户另行准入；协调现有活动包，并先完成全局接入架构规则修订及双宽原型验证 | 复用run16s管理桌面双宽钩子，保持三组件四产物；登录启动/托盘/排除/退出语义及实际包验证，退出后保留已有样式与DLL传播 |

[VB6框架控件外观](../proposals/madshow-controls.md)已按用户请求准入产品实现，范围及自动续S授权由CURRENT管理。

[启动器与首次自动附着](../proposals/launcher-injection.md)已关闭；
[子孙传播与创建语义](../proposals/descendant-propagation.md)已关闭。
[纯原生demo与启动对照](../proposals/native-demo.md)已关闭。
[独立包与M0产品验收](../proposals/package-acceptance.md)已关闭，证据见
CURRENT中的完成审计。待准入候选及顺序见上表。
[菜单取消修复](../proposals/menu-cancellation.md)已关闭，实际交付及复核见CURRENT。
后续项依赖前项交付，不能跨过失败项
直接发布；不增加额外 milestone、产品组件或产品文件。

[字体与布局兼容](../proposals/font-layout.md)本轮修复交付及任务按用户明确指令收口，状态由CURRENT管理。

[英文README与真实截图展示](../proposals/readme-showcase.md)为已交付收口的治理文档工作，
不是新增产品候选；状态仍由CURRENT管理。

[VDM传播诊断](../proposals/vdm-propagation.md)及其T9本地修复/原生cmd测试已交付收口，状态由CURRENT管理。

[构建与测试效率](../proposals/build-test-efficiency.md)已完成Td S9工具修复并收口，证据见其完成记录，状态由CURRENT管理。

[UWP/XAML接入](../proposals/xaml-support.md)已按用户请求取消，无产品支持发布。

[UAC提权与样式注入](../proposals/uac-elevation.md)已完成并收口，实际支持边界和交付见CURRENT。

[旧式PE导入staging](../proposals/legacy-import-staging.md)已完成缺失IAT目录兼容修复并收口；
实际交付与验证边界见CURRENT及对应完成记录。

[英雄榜字体调查](../proposals/leaderboard-font.md)已完成；多语言保护代码和测试已按用户要求由T13 S2原样撤回并收口，状态见CURRENT。

[Win+R启动差异](../proposals/run-dialog-launch.md)已完成T14 S3条件错误路由并收口，保留S1/S2历史，状态见CURRENT。

[DoubleKiller控件适配](../proposals/doublekiller-controls.md)已按用户请求准入，当前范围由CURRENT管理。

[MD5summer控件调查](../proposals/md5summer-controls.md)为用户请求的只读兼容调查，不新增产品T。

[HD Tune Pro控件调查](../proposals/hdtune-controls.md)为只读兼容调查，不新增产品T。

[框架与公共控件完成计划](../proposals/framework-completion.md)已获自动续S授权，状态由CURRENT管理。

[Win3.x局部轮廓拖动](../proposals/outline-window-move.md)已按用户要求由T16 S2整体撤回，不重新进入候选队列；状态由CURRENT管理。

[VB5/VB6完整盘点与适配](../proposals/vb5-vb6-controls.md)已由用户准入T17，S顺序由CURRENT管理，不再占候选队列。
