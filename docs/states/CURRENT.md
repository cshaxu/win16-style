# Project Status

## Current Work

M0 T8 S3 verified并双发布：菜单仅保留细框，局部抑制宿主投影，修复打印右下未绘制条。
标题和S2字体/字号保持。P1待提交推送及实际复核；T8继续开放。

## M0 T8 S3 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation；S2交付后用户反馈popup边框，使用新S3。 |
| Admission And Approval | 用户提供Win3.1与sol对照，指出右/下黑色厚边；按既有授权调研修复并发布。 |
| Objective | popup四边细边框，清除多余宿主厚边/阴影，保持原生菜单追踪语义。 |
| Non-goals | 不改已确认标题、字体、原图形资源、规则、公开ABI或全局主题设置。 |
| Reference Baseline | 622e8d9；已发布S2四文件及用户两幅截图。 |
| Candidate Proposal | [外观后续修订](../proposals/font-layout.md)。 |
| Files And ABI Surface | menu_scope私有绘制/窗口寿命；实际DLL菜单探针及交付文档；ABI3不变。 |
| Applicable Rules | EXECUTION/ARCHITECTURE/CODING/DOCUMENT及UI/来源策略。 |
| Verification | 双宽实际原生popup边缘像素/窗口client几何/嵌套/取消/恢复；旧包负例；相关core完整覆盖和实际包14项。 |
| Expected Markers | MENU BORDER PASS、MENU CANCEL PASS15、旧包边缘负例、双宽ABI及同包结果/双发布摘要一致。 |
| Asset Needs | 同批四文件发布assets根和O:/winnt/system32；不替换原六资源。 |
| Reporting Requirements | 实际截图/原生探针、有限覆盖、失败/跳过、统计、证据SHA和实际远端复核，不冒称物理输入。 |
| Stop Conditions | 标题/字体/原资源改变，菜单语义/寿命回归或发布占用时停止交付并修复。 |
| Exit Criteria | 双宽构建、同包测试、双发布、提交推送和实际复核后仅关闭S3，T8保持开放。 |
| Original Owner Request | Win3.1菜单与sol不同，右侧及下侧粗黑线不对。 |
| Similar-Issue Sweep | 搜索全部popup nonclient/print/paint/shadow处理；不修改共享菜单类或全局SPI；scratch仅build/t8-package，执行者提炼后回收。 |

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 8
- Last allocated Td S: 5
- Last delivery: M0 T8 S3 P1 (verified/dual-published; actual remote review pending)
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
当前scratch仅build/t8-package，实际复核后由本执行者回收；旧before与Git622e8d9 assets
同字节可恢复。保留稳定.ninja/.ccache、两处发布及聊天原生菜单预览。

## Recent Closure

- [S2 System字形修复](../history/M0-T8-S2-system-font.md)：2a0c7dd实际远端复核通过；S2收口，T8开放。

- [S1字体大小修复](../history/M0-T8-S1-font-layout.md)：1c82c2e收口；本轮仅恢复System字形。
- [菜单取消修复](../history/M0-T7-menu-cancellation.md)：保留原交付事实及修复。
- [历史任务审计](../history/M0-T6-completion.md)：不替代本轮实测。
