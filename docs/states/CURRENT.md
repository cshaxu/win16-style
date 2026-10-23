# Project Status

## Current Work

M0 T7 S1 Corrective菜单取消修复已verified并双发布；P1待提交、推送和实际复核，
复核及owned scratch清理前不关闭。用户原始缺陷与有限审计见[菜单修复](../history/M0-T7-menu-cancellation.md)。

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 7
- Last allocated Td S: 5
- Last delivery: M0 T7 S1 P1 (verified locally; remote review pending)

## Technical Baseline

当前同批产品0.7.0.0 / M0 T7，纯C17、MinGW-w64 GCC16.1.0双工具链、Ninja及
ccache4.13.6；不使用C++或MSVC、不依赖MSYS运行时。三组件四产物：x64 run16s.exe、
x86 ntw16s32.dll、x64 ntw16s64.dll、x64 demo.exe。ABI3，31公开名称ordinal2..32
及ordinal1 NONAME helper；无兄弟仓库构建、链接、运行或验收依赖。

鼠标切换菜单后取消会清理残留键盘模式，焦点/capture/菜单项状态恢复，取消后
hover不重开；保留键盘Escape层级、子菜单、system-menu及命令行为。
新探针双宽各15场景通过；两个实际旧T6 DLL在case0被拒绝。状态仍归GUI线程，
tracking_menu栈重入及calls/detached生命期保留。

五个原图形头及原resources.md与固定来源5b0b65f逐字节相同，两DLL各18原表及完整
声明，不替换图形。滚动条17逻辑像素，端钮/固定滑块按实际横截面保持正方形。
124来源处置/294 includes、122本地回执及MinHook14清单通过。原资源外观用户此前已确认。

启动器保持Console：控制台目标默认等待并返回实际退出码，GUI默认异步返回0，
--wait均等待；同包六场景真实cmd/held目标通过。样式和后代传播默认启用，demo
本身仅原生UI。当前原生demo探针直接/注入均通过，本机DPI144/150%、scrollbar厚26物理像素。

本S125输入冻结，干净双宽全构建/ABI各1项通过。相同树未重建的相关回归注册/执行
25/28一致：24+27通过、两个隔离clipboard权限跳过、0失败；覆盖完整core及相关
launch/demo。实际候选包12/12通过，含两宽菜单取消各15场景及传播/等待/错误回滚。
本次未重跑完整propagate contract/dialog/messages矩阵，T6历史完整矩阵58通过/2
跳过只作历史证据，不冒称T7全局矩阵。

用户指定不用computer-use，自己用原生探针；不再要求整套手工操作。原生队列探针
不冒称物理键鼠；CreateWindowStation错误5导致两个隔离clipboard用例跳过77，
不计通过、不触碰共享剪贴板。支持范围及未验证边界见[BUILD](../etc/operations/BUILD.md)。

被测四文件已发布assets根目录及O:/winnt/system32，8条before/after摘要核对一致。
身份、证据摘要及备份定位见[release.json](../etc/release.json)。原始T6发布可从
5b5ee58376d3ff9a98e5852cb2c3c5247e202e02 assets复取。

## Recent Closure

- [T7菜单修复审计](../history/M0-T7-menu-cancellation.md)：当前verified，实际远端复核及清理待完成。
- [T6及M0历史审计](../history/M0-T6-completion.md)：S5 P1 5b5ee58/P2 89fe6b9关闭，当时未覆盖本次popup取消缺陷。
- [滚动条修复](../history/M0-T6-S4-acceptance.md)：原资源和方形几何保留。
- [执行规范](../history/M0-Td-S5-publish-policy.md)：每S同包测试、assets根目录和system32双发布、自己用探针。

## Retained Assets

本任务精确scratch为build/t7-package，实际复核后提炼证据并回收；稳定build/.ninja
及build/.ccache保留。T6 scratch此前已回收，其历史证据摘要保留，不作本次实时证据。

## M0 T7 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Corrective；T6已关闭后的新缺陷，分配T7 S1，不复用旧标识 |
| Admission And Approval | 用户报告menu点击无法取消、移动鼠标即打开；已授权每S构建测试双发布提交推送，自己用原生探针，不用computer-use |
| Objective | 菜单鼠标打开/切换后能取消并恢复闲置，取消后悬停不再重新打开；保留Alt/F10、层级导航及命令行为 |
| Non-goals | 不更换图形资源/字体，不改启动传播/ABI，不改兄弟项目或全局设置，不用物理输入冒充探针 |
| Reference Baseline | 89fe6b9；产品T6/0.6.0.0，纯C/GCC、ABI3、原资源六blob及双DLL18表；用户本次反馈为真实缺陷依据 |
| Candidate Proposal | [菜单取消修复](../proposals/menu-cancellation.md)；本任务只有S1，含有限覆盖与T级审计 |
| Files And ABI Surface | win16_style菜单循环/过滤与原生菜单探针；版本T7、构建打包工具Task参数、回执/清单/文档；公开ABI和图形不变 |
| Applicable Rules | Task Reading Set、Corrective新任务、失败负例、原资源身份、每S四文件闭环及双位置发布 |
| Verification | 原生私有桌面菜单队列/idle探针验证旧失败和修后状态；双宽core完整及当前包；焦点/捕获/菜单depth/owner-draw恢复；新四文件PE/ABI/声明/资源及SHA |
| Expected Markers | 鼠标切换后Esc/外部点击/重复点击/CancelMode取消能返回，焦点恢复且无残留键盘模式、scope或capture；随后hover不打开；键盘/子菜单行为回归通过 |
| Asset Needs | build/t7-package/probe、probe-fixed、probe-complete及probe-tree开发/负例，probe-final最终focused，negative旧DLL负例，final初批与release最终干净快照，package-test实际包，publish两处备份及cleanup记录；hidden owned单架构600秒、包1800秒、scratch1GB |
| Reporting Requirements | 列出每个取消/切换场景及实际状态；区分原生消息探针和物理输入；保留失败证据和历史权限跳过，不虚报全平台通过 |
| Stop Conditions | 取消后再开、键盘导航/层级/命令损坏、资源改变或发布混包先修复；占用/权限问题不关闭无关进程 |
| Exit Criteria | 冻结覆盖全部通过或明确无关限制，新双宽四文件构建/实际包测试/双发布/提交推送/实际复核，完成T7任务级审计并回收owned scratch |
| Original Owner Request | menu点击后无法取消，鼠标移到哪个菜单哪个就打开，要求修复；沿用原资源、纯C/GCC、自己用探针及每S双发布规范 |
| Similar-Issue Sweep | menu_filter所有退出及坐标、open_bar跨项模式切换、keyboard Escape/外部取消、重复顶层点击、CancelMode/失活/焦点以及owner-draw和scope清理 |
