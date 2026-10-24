# Project Status

## Current Work

M0 T8 S4菜单System比例/布局修复已验证并双发布，P1待提交推送及实际远端复核。
S4暂不关闭；M0固定、T8继续开放接收外观反馈。原生打开菜单图已保存供验收。

## M0 T8 S4 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Corrective；T8下一未用编号S4，M0不变；implemented/verified，双发布完成，P1待实际远端复核。 |
| Admission And Approval | 用户明确授权继续菜单外观修复、原生探针、自行双发布/提交推送/复核，保持T8开放。 |
| Objective | 菜单System16字形等比DPI缩放，按字体重排标签/快捷键/勾选/箭头；复验薄边和局部SysShadow。 |
| Non-goals | 标题字体/字号/绘制、标准控件策略、应用自绘信息栏、原资源及ABI不改；不改全局设置/共享菜单类。 |
| Reference Baseline | a9ab2ab954af6710748de2fc7bab43285335bdd0；干净工作区；S3 shadow/WM_PRINT修复及已有探针沿用。 |
| Candidate Proposal | [字体布局修复](../proposals/font-layout.md)；菜单8pt字符压缩失败契约由本S纠正。 |
| Files And ABI Surface | font.c/h、win16_style.c、menu_paint.c/h、menu_items.c、既有原生测试及回执/设计/交付文档；ABI3不变，无新增产品。 |
| Applicable Rules | 已读Task Reading Set、执行/架构/编码/文档规则及来源政策；纯C17/GCC/Ninja/ccache；兄弟仓库只读且不依赖。 |
| Verification | 有限全集：两宽实际96/144 DPI比例/像素/布局、嵌套/取消/投影/资源，标题源码及控件像素/度量对照；旧包比例负例；干净双宽同批四文件及实际包测试，相关core/demo/launch回归；192仅向量不冒称实屏。 |
| Expected Markers | FONT MENU SYSTEM PASS、FONT LAYOUT PASS、MENU CANCEL PASS、MENU SHADOW SCOPE PASS；实际失败/skip如实记录。 |
| Asset Needs | 六原资源逐字节保持；原生打开菜单PNG只作验收图；scratch精确build/t8-package，执行者负责提炼后安全回收，保留.ninja/.ccache。 |
| Reporting Requirements | 记录统计、来源/ABI、包与两发布位置摘要、失败/跳过、原生截图、实际远端复核；仅关闭S4。 |
| Stop Conditions | 资源身份变化、标题/控件策略回归、ABI变化、全局设置修改、包测试失败或发布占用时停止交付并定位，不关闭无关进程。 |
| Exit Criteria | 相关原生探针及回归通过/明确skip，干净同包测试、双发布摘要一致、提交推送实际复核后S4收口，T8继续开放。 |
| Original Owner Request | 本轮菜单System正常比例、布局适配、查清右下投影、保持标题/控件/应用自绘/资源；不用computer-use，自行探针验收发布。 |
| Similar-Issue Sweep | 搜索font/measure/draw/shortcut/check/arrow/shadow/print调用；菜单栏与popup共用比例字体、测绘同源；controls.c保持，DWM恢复债务维持TODO不扩围。 |

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 8
- Last allocated Td S: 5
- Last delivery: M0 T8 S4 P1 (verified/dual-published; actual remote review pending)
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
登记scratch build/t8-package待实际复核后安全回收；.ninja/.ccache及两处发布保留。

## Recent Closure

- [S4菜单比例修复](../history/M0-T8-S4-menu-proportions.md)：验证/双发布完成，待实际远端复核后仅关闭S4。
- [S3菜单厚黑边修复](../history/M0-T8-S3-popup-shadow.md)：aa28427实际复核通过；S3收口。
- [S2 System字形修复](../history/M0-T8-S2-system-font.md)：2a0c7dd实际复核通过；S2收口。
- [S1字体大小修复](../history/M0-T8-S1-font-layout.md)：1c82c2e收口。
- [菜单取消修复](../history/M0-T7-menu-cancellation.md)与[历史产品审计](../history/M0-T6-completion.md)保留历史边界。
