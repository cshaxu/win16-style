# Project Status

## Current Work

当前无活动任务包。M0 T8 S3菜单厚黑边修复已双发布、推送、实际复核及清理，
S3交付收口。标题和已确认System字体/字号保持；按用户指令T8继续开放接收外观反馈，
不把本S收口等同全任务验收通过。

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 8
- Last allocated Td S: 5
- Last delivery: M0 T8 S3 P2 (P1 aa28427实际远端复核、清理和S3收口；T8开放)
- Open task: M0 T8 (System/menu repairs delivered; owner appearance feedback pending)

## Technical Baseline

0.8.0.0 / M0 T8 S3，纯C17 MinGW-w64 GCC16.1.0双宽、Ninja/ccache4.13.6，
无C++/MSVC/MSYS运行库或兄弟仓库依赖。三组件四文件：x64启动器、双宽DLL、x64 demo。
ABI3/31名称ordinal2..32及ordinal1 NONAME helper，六原资源字节/两DLL各18表保持。
Console等待/退出码及后代传播保持。原17逻辑像素滚动条和菜单取消契约保留。

S2已确认标题、System字体/字号实现源码font.c/font.h/controls.c/win16_style.c逐字节不变。
菜单按线程/z序/原点/扩展几何，仅关联当前样式popup的SysShadow，NOACTIVATE局部
阻止显示；不改共享类和全局SPI，不销毁USER窗口，scope/终结/缓存重用解除适配。
保留完整原生WM_PRINT标志，再画细框，避免破坏客户区打印坐标；窗口/客户区几何不变。
字体沿用S2：title10pt粗体MS Sans Serif，内部/菜单System字形保留原度量，DPI一次，
应用自绘不改；位图范围/失败回退及原20px兼容API边界见[UI](../design/UI.md)。

最终s3-flat干净双宽全构建/ABI各1/1，126编译输入与冻结树一致，无构建后源码修订。
相关注册全集26/29：剩余CTest24/27与实际包的双宽font/menu组合，53通过/2clipboard
权限跳过；不是一次完整CTest。实际包14/14通过；两宽96/144 DPI各15菜单场景（每宽30），
全NC细框、客户区哨兵完整、投影隐藏、嵌套/取消/焦点/捕获/命令、原生前后/类/SPI保持。
旧S2两宽各两负例分别拒绝未绘制角落和可见SysShadow。额外x64自然Window DC两DPI各5
有效样本无黑角；这不是物理输入或真实桌面合成截图。字体探针GDI43/43与45/45保持。
严格资源fixture固定64完整负载预热后无增长，不条件等待或降低断言。来源124/294/123，
负例门禁23+9+6+13通过。原XP sol.exe/真实192DPI/其他宿主和未支持菜单未扩大承诺。

被测同批四文件已先备份8条，再发布assets根及O:/winnt/system32，8摘要完全一致。
源快照、工具链、四文件SHA/PE/imports、原始失败及最终证据SHA见
[release.json](../etc/release.json)和[S3记录](../history/M0-T8-S3-popup-shadow.md)。
本S build/t8-package已回收：10目录/3223文件/132312573字节，无reparse或活动owned进程。
原始证据先实际复核再提炼；旧before与Git622e8d9 assets同字节可恢复。
保留稳定.ninja/.ccache、两处发布及聊天原生菜单/调查预览。

## Recent Closure

- [S3菜单厚黑边修复](../history/M0-T8-S3-popup-shadow.md)：aa28427实际远端复核通过；S3收口，T8开放。

- [S2 System字形修复](../history/M0-T8-S2-system-font.md)：2a0c7dd实际远端复核通过；S2收口，T8开放。

- [S1字体大小修复](../history/M0-T8-S1-font-layout.md)：1c82c2e收口；本轮仅恢复System字形。
- [菜单取消修复](../history/M0-T7-menu-cancellation.md)：保留原交付事实及修复。
- [历史任务审计](../history/M0-T6-completion.md)：不替代本轮实测。

P2仅记录实际复核/清理/收口，126编译输入与四产品SHA与P1完全相同，不虚构新构建发布。
