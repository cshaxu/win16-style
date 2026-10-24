# Project Status

## Current Work

M0 Td S6英文README与真实截图任务已实现并verified，待P1推送及实际复核；不改变M0 T8产品基线或关闭T8。

## M0 Td S6 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Governance；下一未用Td S6，M0固定，T8开放。 |
| Admission And Approval | 用户要求全英文丰富截图README供GitHub读者，可展示demo/真实sol.exe；授权文档、精选PNG和提交推送。 |
| Objective | 准确英文入口、快速使用/下载/工作原理/构建/限制/来源，真实运行画廊。 |
| Non-goals | 不改任何产品/测试/构建输入、原图形资源或二进制，不下载/提交游戏exe，不修改系统设置，不关闭T8。 |
| Reference Baseline | fde1c67b7aa25dbc69e54dcc6215bd1596aba395；工作区干净，当前四文件0.8.0.0。 |
| Candidate Proposal | [README showcase](../proposals/readme-showcase.md)。 |
| Files And ABI Surface | README.md、assets/README.md、docs/etc截图/元数据及索引、必要过期操作指针、任务文档；ABI/126输入/四产品不变。 |
| Applicable Rules | 已读Task Reading Set/UI/来源政策；用户公开截图要求优先于CONTRIBUTING/etc索引不提交截图的一般约束，仅精选公开展示PNG例外。 |
| Verification | 全英文/相对链接/PNG解码与视觉检查、真实应用模块/命令/DPI/图像来源、发布与原资源SHA保持、治理SelfTest、实际远端diff复核；纯文档不重建发布。 |
| Expected Markers | SHOWCASE CAPTURE PASS、README LINKS/ENGLISH PASS、BINARY IDENTITY PASS、Governance PASS。 |
| Asset Needs | 精选截图存docs/etc/screenshots并索引；scratch精确build/td-s6-readme，自有C拍摄器/日志，复核提炼后回收；游戏本体不随仓库公开。 |
| Reporting Requirements | 记录应用/包SHA、DPI/拍摄方式、实际画廊和未覆盖/失败处置，Td收口但T8保持开放。 |
| Stop Conditions | 产品摘要改变、图形资源改变、截图失真或来历不明、不可复核的真实sol来源；先定位，不伪造截图或擅自下载游戏。 |
| Exit Criteria | 英文README丰富真实图、链接/PNG/门禁检查、产品身份不变、提交推送及实际复核后仅关闭Td S6。 |
| Original Owner Request | 全英文带丰富截图的项目README供GitHub读者搜索查看，可以demo和真实sol.exe截图。 |
| Similar-Issue Sweep | 搜索根/assets/BUILD过期T6/S3/菜单8pt描述，修为当前权威指针；检查旧截图不冒称当前版本，无AI替代画面。 |

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 8
- Last allocated Td S: 6
- Last delivery: M0 Td S6 P1 (verified locally，待推送及实际复核；T8开放)
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

- [S4菜单比例修复](../history/M0-T8-S4-menu-proportions.md)：64e3fd3实际远端复核通过，S4收口；T8保持开放。
- [S3菜单厚黑边修复](../history/M0-T8-S3-popup-shadow.md)：aa28427实际复核通过；S3收口。
- [S2 System字形修复](../history/M0-T8-S2-system-font.md)：2a0c7dd实际复核通过；S2收口。
- [S1字体大小修复](../history/M0-T8-S1-font-layout.md)：1c82c2e收口。
- [菜单取消修复](../history/M0-T7-menu-cancellation.md)与[历史产品审计](../history/M0-T6-completion.md)保留历史边界。
