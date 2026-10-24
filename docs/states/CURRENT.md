# Project Status

## Current Work

M0 T9 S1 implemented/verified：0.9.0.0已完成双宽构建、19项同包验证与双目录发布，
等待P1推送实际复核及安全清理后收口。M0固定、T8保持开放；此前NE间歇失败
未证实根因，既有裸NTSRV复用无传播仍为用户接受边界。

## M0 T9 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New；T9为下一未用产品任务，S1；M0固定，T8保持开放。 |
| Admission And Approval | 用户授权本项目范围内确定根因/最小方案后完成修复发布；后续双钩子竞争调研发现staging拒绝有效外部表，已先说明根因与最小方案。 |
| Objective | 读取有效重定位导入表，保持两钩子调用链与新子进程传播，不误报193；新增真实交互式cmd嵌套回归。 |
| Non-goals | 不将其当作原NE间歇故障根因，不改兄弟协议/旧实现/既有用户进程/全局设置，不改标题字体菜单绘制及六原资源。 |
| Reference Baseline | ce20f15诊断已推送复核；产品仍T8 S4，原失败PE证据见Td S8。 |
| Candidate Proposal | [传播诊断及最小修复](../proposals/vdm-propagation.md)。 |
| Files And ABI Surface | staging.c及自包含双宽导入表契约测试、T9版本/操作入口、发布/交付文档；ABI3不变。 |
| Applicable Rules | Task Reading Set/架构编码UI来源及每S四文件闭环；纯C17/GCC/Ninja/ccache，兄弟只读兼容实验不作必需构建测试依赖。 |
| Verification | 双宽外部导入表正例/无效地址/越界/未终止负例及真实初始化；真实cmd四位宽边，仅ntw必需、nthook/双钩子独立RPC身份可选并明确skip；各层PE/DLL SHA/标记/退出/清理；旧包负例检出。全部相关回归、标题/控件/菜单代码与资源保留；干净双宽同批四文件包测试；可选实际双钩子与NE私有自有实例复测。 |
| Expected Markers | STAGING EXTERNAL PASS、CMD NESTED PASS、原相关矩阵标记、PACKAGE PASS、资源/治理负例门禁、实际远端复核。 |
| Asset Needs | 接收build/td-s8-vdm原始证据/临时探针清理责任；新增精确build/t9-package，候选/测试/备份发布；本轮build/.ninja/x86/Testing及build/.ninja/x64/Testing中的LastTest*.log亦提炼回收，保留CTestCostData及编译缓存；先提炼再检查进程/reparse回收。 |
| Reporting Requirements | 初始负例与修后、自包含/可选后端分别报告；四SHA/PE/ABI/输入/双发布/远端身份、未覆盖面，T8开放。公开证据脱敏。 |
| Stop Conditions | 原资源或UI意外改变、调用者挂起语义/回滚失败、真实包失败则修复后重建；兄弟变更需求止于提案。 |
| Exit Criteria | 最小修复/自包含正负例及双宽回归、同包测试、双发布、提交推送实际复核和安全清理后收口本S；不宣称原NE间歇故障已修复。 |
| Original Owner Request | 传播链诊断及本项目范围内根因/最小修复交付，补充调查双CreateProcess钩子竞争；接受裸NTSRV无传播边界；用户新增明确授权真实cmd嵌套测试与修复，已先说明两层设计及候选失败点。 |
| Similar-Issue Sweep | 同一staging入口的初始/同宽/跨宽helper、已经stage重入、read/restore外部导入表和权限/溢出边界；不更换trampoline或服务策略。 |

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 9
- Last allocated Td S: 8
- Last delivery: M0 T9 S1 P1 (verified locally；待推送实际复核，T8开放)
- Open task: M0 T9 S1 (dual-hook import staging); M0 T8 remains open

## Technical Baseline

0.9.0.0 / M0 T9 S1，纯C17、MinGW GCC16.1.0双宽、Ninja1.13.2/ccache4.13.6。
三组件四文件、x64 Console启动器/双宽DLL/x64 demo；ABI3、31名称及ordinal1
NONAME helper不变，无C++/MSVC或兄弟必需依赖。唯一运行时代码改动为staging
允许有效可读外部导入表，保留溢出、终止、权限、挂起和回滚契约。

标题、System字体/菜单、控件、局部SysShadow及应用自绘策略保持T8 S4代码；
六原资源及声明逐字节保留，两DLL各18表。96/144DPI实际包字体和15菜单场景
均通过；物理输入、真实192DPI及本S新包NE冷/热复用未覆盖，不借历史结果。

独立干净双宽构建及ABI各1/1，128源码/测试输入与冻结快照一致；同批四文件
实际包19/19，包括完整创建/回滚/dialog/message集成。注册全集66项：63通过、
两宽clipboard权限5/77及默认可选后端77共3跳过；最终观测/清理修订另以聚焦和
实际包复验，不冒称所有结果来自一次构建。真实cmd仅ntw必需4边、nthook/双钩子
可选各4边均通过，独有子标记、37/23退出及内外job归零。双宽有效外部表真实
初始化；NOACCESS299、未终止/RVA越界193。旧生产源双宽和旧包均能检出缺陷。

四文件已先备份后发布assets根及O:/winnt/system32，8摘要与实际测试包一致；
8份before与Gite6d6b55 assets一致可恢复。版本、PE/imports、输入和脱敏证据见
[release.json](../etc/release.json)与[S1记录](../history/M0-T9-S1-native-cmd-staging.md)。
P1尚待推送实际复核；两个登记scratch及本轮Testing日志尚待安全回收。

英文README及精选8张demo/真实XP Solitaire PNG原样保留，其展示与来源记录见
[截图清单](../etc/screenshots/manifest.json)，不扩写成当前新包的游戏验收。

## Recent Closure

- [Td S8传播诊断](../history/M0-Td-S8-vdm-propagation.md)：ce20f15实际远端复核通过；原NE间歇失败不作修复声明，独立staging缺陷移交当前产品任务。

- [Td S7 README编辑](../history/M0-Td-S7-readme-edit.md)：894e012实际远端复核通过；指定沿革段落已删除。

- [Td S6英文README与真实截图](../history/M0-Td-S6-english-readme.md)：02df11a实际远端复核通过；8张公开PNG保留，scratch已安全回收。

- [S4菜单比例修复](../history/M0-T8-S4-menu-proportions.md)：64e3fd3实际远端复核通过，S4收口；T8保持开放。
- [S3菜单厚黑边修复](../history/M0-T8-S3-popup-shadow.md)：aa28427实际复核通过；S3收口。
- [S2 System字形修复](../history/M0-T8-S2-system-font.md)：2a0c7dd实际复核通过；S2收口。
- [S1字体大小修复](../history/M0-T8-S1-font-layout.md)：1c82c2e收口。
- [菜单取消修复](../history/M0-T7-menu-cancellation.md)与[历史产品审计](../history/M0-T6-completion.md)保留历史边界。
