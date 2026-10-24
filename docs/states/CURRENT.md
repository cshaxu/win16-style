# Project Status

## Current Work

当前无活动任务包。M0 T8 S2 System字形修复已双发布、推送、实际复核和清理，
S2交付收口。用户确认的标题保持；菜单和标准控件恢复System字形，并保留已确认
字号与字符宽度。按用户指令，T8继续开放接收外观反馈，不宣称T全部验收通过。

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 8
- Last allocated Td S: 5
- Last delivery: M0 T8 S2 P2 (P1 2a0c7dd实际远端复核、清理和S2收口；T8开放)
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
[S2记录](../history/M0-T8-S2-system-font.md)。本S build/t8-package已回收：8顶层目录/2076文件/77990133字节，无reparse或活动owned进程。
原始证据先实际复核再提炼SHA及结果；旧before与Git 1c82c2e assets同字节可恢复。
稳定build/.ninja、build/.ccache、两处发布和聊天原生预览保留。

## Recent Closure

- [S2 System字形修复](../history/M0-T8-S2-system-font.md)：2a0c7dd实际远端复核通过；S2收口，T8开放。

- [S1字体大小修复](../history/M0-T8-S1-font-layout.md)：1c82c2e收口；本轮仅恢复System字形。
- [菜单取消修复](../history/M0-T7-menu-cancellation.md)：保留原交付事实及修复。
- [历史任务审计](../history/M0-T6-completion.md)：不替代本轮实测。

P2仅记录实际复核/清理/收口，126编译输入和四文件SHA与P1完全相同，不虚构新构建或发布。
