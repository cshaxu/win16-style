# Project Status

## Current Work

M0 Td S8诊断已推送实际复核并收口；M0 T9 S1已准入/active，修复双钩子外部导入表
兼容。原NE间歇失败未证实根因，T8仍开放；裸NTSRV复用无传播为用户接受边界。

## M0 T9 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New；T9为下一未用产品任务，S1；M0固定，T8保持开放。 |
| Admission And Approval | 用户授权本项目范围内确定根因/最小方案后完成修复发布；后续双钩子竞争调研发现staging拒绝有效外部表，已先说明根因与最小方案。 |
| Objective | 读取有效重定位导入表，保持两钩子调用链与新子进程传播，不误报193。 |
| Non-goals | 不将其当作原NE间歇故障根因，不改兄弟协议/旧实现/既有用户进程/全局设置，不改标题字体菜单绘制及六原资源。 |
| Reference Baseline | ce20f15诊断已推送复核；产品仍T8 S4，原失败PE证据见Td S8。 |
| Candidate Proposal | [传播诊断及最小修复](../proposals/vdm-propagation.md)。 |
| Files And ABI Surface | staging.c及自包含双宽导入表契约测试、T9版本/操作入口、发布/交付文档；ABI3不变。 |
| Applicable Rules | Task Reading Set/架构编码UI来源及每S四文件闭环；纯C17/GCC/Ninja/ccache，兄弟只读兼容实验不作必需构建测试依赖。 |
| Verification | 双宽外部导入表正例/无效地址/越界/未终止负例及真实初始化；全部相关回归、标题/控件/菜单代码与资源保留；干净双宽同批四文件包测试；可选实际双钩子与NE私有自有实例复测。 |
| Expected Markers | STAGING EXTERNAL PASS、原相关矩阵标记、PACKAGE PASS、资源/治理负例门禁、实际远端复核。 |
| Asset Needs | 接收build/td-s8-vdm原始证据/临时探针清理责任；新增精确build/t9-package，候选/测试/备份发布；先提炼再检查进程/reparse回收。 |
| Reporting Requirements | 初始负例与修后、自包含/可选后端分别报告；四SHA/PE/ABI/输入/双发布/远端身份、未覆盖面，T8开放。公开证据脱敏。 |
| Stop Conditions | 原资源或UI意外改变、调用者挂起语义/回滚失败、真实包失败则修复后重建；兄弟变更需求止于提案。 |
| Exit Criteria | 最小修复/自包含正负例及双宽回归、同包测试、双发布、提交推送实际复核和安全清理后收口本S；不宣称原NE间歇故障已修复。 |
| Original Owner Request | 传播链诊断及本项目范围内根因/最小修复交付，补充调查双CreateProcess钩子竞争；接受裸NTSRV无传播边界。 |
| Similar-Issue Sweep | 同一staging入口的初始/同宽/跨宽helper、已经stage重入、read/restore外部导入表和权限/溢出边界；不更换trampoline或服务策略。 |

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 9
- Last allocated Td S: 8
- Last delivery: M0 Td S8 P2 (P1 ce20f15已实际远端复核，证据移交T9 S1；T8开放)
- Open task: M0 T9 S1 (dual-hook import staging); M0 T8 remains open

## Technical Baseline

0.8.0.0 / M0 T8 S4，纯C17、MinGW GCC16.1.0双宽、Ninja/ccache4.13.6。
三组件四文件，x64 Console启动器/双宽DLL/x64 demo，ABI3/31名称与ordinal1
NONAME helper保持；六原资源字节/两个DLL各18表不变，无C++/MSVC或兄弟仓库依赖。

菜单栏和popup独立System16原比例按窗口DPI缩放，不再套用8pt MS Sans Serif
字符宽高。popup整组标签/快捷键列测绘同源，勾选/箭头空间及行高适配字形；
原生USER附加宽度只扣一次，实际矩形与独立自然System布局一致。公开row测量
接口保守预留箭头，实际无submenu菜单减少尾部留白。HMENU样式/共享类/SPI不改。

已确认标题创建/绘制代码保持，原生96/144 DPI完整标题RGB对照零差异；controls.c
逐字节不变，控件原策略/System像素/字高/advance/更新/NULL/卸载/重入保持，应用
自绘信息栏不干预。legacy20显式字体API不变。S3局部SysShadow/完整WM_PRINT
坐标修复源码原样保留，新尺寸下复验隐藏同线程关联的5px独立投影及全NC细框/
客户区角落完整，嵌套/取消/焦点/捕获/命令/恢复通过。

最终s4-delivery独立干净双宽全构建及ABI各1/1，126输入与当前树完全相同。
实际四文件包14/14；相关注册全集26/29由剩余CTest24/27和包内font/menu组成，
53通过/2私有桌面clipboard权限错误5/77跳过；不是一次完整CTest。每宽实际
96/144 DPI各15菜单场景、System菜单栏/popup像素及自然宽108/162、84控件场景，
GDI43/43及45/45。四个旧S3负例exit1，最终负例fixture源码身份不变。
来源124/294/123，23+9+6+13负例门禁通过。真实192DPI、XP sol.exe及物理输入
未覆盖；可选完整创建/dialog/message integration本S未重跑，不借历史结果。

同批四文件先备份后发布assets根及O:/winnt/system32，8摘要与实际测试包相同；
8份before均与Gita9ab2ab assets一致可恢复。源快照、工具链、SHA/PE/imports、
原始证据摘要与截图见[release.json](../etc/release.json)及[S4记录](../history/M0-T8-S4-menu-proportions.md)。
P1 64e3fd30634bcfa426fe1dbc4eec5a0f00c24504实际远端复核通过：126输入、六资源
Git blob、四远端/八发布摘要、167份原始证据、完整注册覆盖和旧包负例/标题像素。
登记scratch build/t8-package已安全回收：21顶层目录/3316文件/162916014字节，
无reparse或活动owned进程。结果及原始SHA先提炼，8份before可从a9ab2ab恢复；
.ninja/.ccache、两处发布及两DPI聊天PNG保留。P2仅文档收口，126输入及四文件
与P1完全相同，不虚构新编译或发布。

Td S6使用未改的发布包拍摄真实XP sol.exe/匹配cards.dll及demo，精选8张原生PNG
和全英文README；仅展示图中场景，不追写成S4覆盖或完整游戏测试。来源/摘要见
[截图清单](../etc/screenshots/manifest.json)，本次验证见[记录](../history/M0-Td-S6-english-readme.md)。

## Recent Closure

- [Td S8传播诊断](../history/M0-Td-S8-vdm-propagation.md)：ce20f15实际远端复核通过；原NE间歇失败不作修复声明，独立staging缺陷移交当前产品任务。

- [Td S7 README编辑](../history/M0-Td-S7-readme-edit.md)：894e012实际远端复核通过；指定沿革段落已删除。

- [Td S6英文README与真实截图](../history/M0-Td-S6-english-readme.md)：02df11a实际远端复核通过；8张公开PNG保留，scratch已安全回收。

- [S4菜单比例修复](../history/M0-T8-S4-menu-proportions.md)：64e3fd3实际远端复核通过，S4收口；T8保持开放。
- [S3菜单厚黑边修复](../history/M0-T8-S3-popup-shadow.md)：aa28427实际复核通过；S3收口。
- [S2 System字形修复](../history/M0-T8-S2-system-font.md)：2a0c7dd实际复核通过；S2收口。
- [S1字体大小修复](../history/M0-T8-S1-font-layout.md)：1c82c2e收口。
- [菜单取消修复](../history/M0-T7-menu-cancellation.md)与[历史产品审计](../history/M0-T6-completion.md)保留历史边界。
