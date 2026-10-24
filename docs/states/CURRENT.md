# Project Status

## Current Work

M0 T8 S2 verified并完成双发布，P1待提交推送及实际远端复核；当前活动包如下。
用户确认的标题保持；菜单和标准控件恢复System字形并保留已确认字号/文字宽度。
T8继续开放，不能将本轮S收口等同用户外观验收或T关闭。

## M0 T8 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation；S1关闭后使用S2，T8继续开放。 |
| Admission And Approval | 用户要求保持最新标题，内部包括菜单恢复System且保持当前字号。 |
| Objective | System原生字形适配既有字高/字符advance；不改变控件布局；保留最新标题。 |
| Non-goals | 不改标题、原图形资源、公开ABI、启动策略、规则或应用自绘文字。 |
| Reference Baseline | 1c82c2e / S1同批四文件；字体方案见brief。 |
| Candidate Proposal | [字体与布局](../proposals/font-layout.md)。 |
| Files And ABI Surface | DLL私有font/controls/frame；相关原生测试及交付文档；ABI3及31名称保持。 |
| Applicable Rules | EXECUTION/ARCHITECTURE/CODING/DOCUMENT及UI、来源策略。 |
| Verification | 双宽System字形/尺寸/更新/NULL/卸载/资源探针，旧S1负例；完整相关core回归，同包14项及双发布哈希。 |
| Expected Markers | FONT LAYOUT PASS、System位图校验、旧包拒绝、core及package实际结果、发布8摘要一致。 |
| Asset Needs | 同批四文件发布assets根及O:/winnt/system32；原六资源不变。 |
| Reporting Requirements | 原始失败/跳过保留；记录源码身份、统计、探针边界、实际远端复核；不冒称真实输入。 |
| Stop Conditions | 标题变化、原资源变化、ABI变化、尺寸/字形/资源失败或发布占用即停止交付并修复。 |
| Exit Criteria | 编译/同包测试/双发布/提交推送/实际复核完成后仅关闭S2；T8保持开放。 |
| Original Owner Request | 最新标题很好继续保持；内部包括菜单字号对了，字体必须恢复System。 |
| Similar-Issue Sweep | 搜索全部WM_SETFONT/font创建/菜单度量及绘制；明确借用app字体与拥有替换字体的寿命；scratch仅build/t8-package，执行者负责提炼后回收。 |

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 8
- Last allocated Td S: 5
- Last delivery: M0 T8 S2 P1 (verified/dual-published; remote review pending)
- Open task: M0 T8 (System repair delivered; owner appearance feedback pending)

## Technical Baseline

0.8.0.0 / M0 T8 S2，同批x64 run16s.exe、x86 ntw16s32.dll、x64 ntw16s64.dll、
x64 demo.exe；纯C17、MinGW-w64 GCC16.1.0、Ninja、ccache4.13.6，不用C++/MSVC或
MSYS运行库。ABI3/31名称ordinal2..32与ordinal1 NONAME helper不变，无兄弟依赖。

标题沿用已确认10pt粗体MS Sans Serif及原绘制；内部/菜单使用宿主System位图字形，
以应用原字体/菜单8pt度量匹配字符advance、字高、基线与内外行距，不重排布局。
System字体及NULL原生默认直接保留；WM_SETFONT重新适配，卸载恢复最新app字体。
变体由GUI线程引用计数拥有，回调入口和出站font固定引用防重入失效，原app字体只借用。
私有FNT范围CP1252、1..64px字高/最多128px字宽和v2容量，失败保留app字体；不冒称
任意字号/字符集覆盖。显式20px兼容API和应用自绘Score/Time保持。DPI只转换一次。

六原资源逐字节保留，两DLL各18表；滚动条17逻辑像素/端钮正方形、菜单取消修复保留。
Console启动策略、CUI等待及退出码、GUI异步/--wait、双宽注入和后代传播保持。

最终126编译输入与冻结s2-final源码逐项一致；干净双宽全构建及ABI各1/1通过。
其余相关CTest x86选择24（23通过/1剪贴板权限跳过）、x64选择27（26通过/1跳过）；
字体和菜单取消各双宽由实际包提供，核对完整相关注册全集26/29，最终53通过/2跳过。
这是明确组合覆盖，不伪称单次完整CTest。实际包14/14通过，四文件测试期间未变。
双宽字体各12组合/84控件、DS_SETFONT、System像素/ASCII宽度、基线行距、菜单
WM_DRAWITEM及重入/NULL/卸载通过，GDI43/43与45/45；实际窗口DPI96/144，192只是
字号向量。旧S1两DLL均被新字形断言拒绝，demo裸/注入文字advance均41。

探针仅私有桌面，无computer-use/物理输入。两个clipboard CreateWindowStation错误5
跳过不计通过；原XP sol.exe及真实192-DPI显示器不在受控证明中。T8继续接收用户外观反馈。

已先备份8文件，再将被测同批四文件发布assets根和O:/winnt/system32，8摘要完全一致。
原始失败、重配置失败及候选不变/恢复边界如实保留，不算通过；最终双宽完整相关复测均通过。
身份/工具链/命令/原始证据SHA/覆盖/统计见[release.json](../etc/release.json)与
[S2记录](../history/M0-T8-S2-system-font.md)。当前scratch仅build/t8-package，待实际复核后回收；
旧before均与Git 1c82c2e assets同字节，可恢复。稳定build/.ninja和build/.ccache保留。

## Recent Closure

- [S1字体大小修复](../history/M0-T8-S1-font-layout.md)：1c82c2e收口；本轮仅恢复System字形。
- [菜单取消修复](../history/M0-T7-menu-cancellation.md)：保留原交付事实及修复。
- [历史任务审计](../history/M0-T6-completion.md)：不替代本轮实测。
