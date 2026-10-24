# Project Status

## Current Work

M0 Td S7 README编辑修订已实现/verified，待推送复核；M0固定，T8继续开放。

## M0 Td S7 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Governance；下一未用Td S7。 |
| Admission And Approval | 用户明确要求README无需提及ntvdm64项目，删除所引沿革段落。 |
| Objective | 删除README项目沿革介绍，保持独立产品介绍。 |
| Non-goals | 不改产品、截图、原始资源/版权声明、来源审计或历史事实；不关闭T8。 |
| Reference Baseline | d70e5513770b5def1b13ab4856d083620b70f757，干净工作区。 |
| Candidate Proposal | [README showcase](../proposals/readme-showcase.md)，本次仅编辑反馈。 |
| Files And ABI Surface | README及本次任务记录；无ABI/产品输入变化。 |
| Applicable Rules | Task Reading Set、DOCUMENT、EXECUTION；用户请求定义编辑范围。 |
| Verification | 精确diff、读者入口同类搜索、治理SelfTest/diff检查、实际远端复核。 |
| Expected Markers | README EDIT PASS、Governance PASS。 |
| Asset Needs | 无新资源或scratch。 |
| Reporting Requirements | 记录删除范围及实际复核；不虚构构建发布。 |
| Stop Conditions | diff越出编辑范围或涉及原声明则定位保留。 |
| Exit Criteria | 指定段落删除、验证推送实际复核后收口本S。 |
| Original Owner Request | 这个毫无必要，不需要提到ntvdm64项目。 |
| Similar-Issue Sweep | rg检查根/assets/截图README，仅根所引段落命中。 |

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 8
- Last allocated Td S: 7
- Last delivery: M0 Td S6 P2 (P1 02df11a实际远端复核通过，文档收口；T8开放)
- Open task: M0 T8 (menu appearance feedback; remains open)

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

- [Td S6英文README与真实截图](../history/M0-Td-S6-english-readme.md)：02df11a实际远端复核通过；8张公开PNG保留，scratch已安全回收。

- [S4菜单比例修复](../history/M0-T8-S4-menu-proportions.md)：64e3fd3实际远端复核通过，S4收口；T8保持开放。
- [S3菜单厚黑边修复](../history/M0-T8-S3-popup-shadow.md)：aa28427实际复核通过；S3收口。
- [S2 System字形修复](../history/M0-T8-S2-system-font.md)：2a0c7dd实际复核通过；S2收口。
- [S1字体大小修复](../history/M0-T8-S1-font-layout.md)：1c82c2e收口。
- [菜单取消修复](../history/M0-T7-menu-cancellation.md)与[历史产品审计](../history/M0-T6-completion.md)保留历史边界。
