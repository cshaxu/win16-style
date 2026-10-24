# M0 T8 S2：恢复System字形，保持已确认尺寸及标题

P1已验证并双发布，提交推送及实际远端复核待完成。S2尚未关闭；T8保持开放。

用户确认最新标题与内部/菜单字号，要求恢复System字形。准入为S1之后的S2，
基线1c82c2e5e5e33ef082d4a0797295fe04488cacb7，不重开或重用S1。

## 有限覆盖与实现

1. 标准控件：应用HFONT只借用，记录最新原字体；以宿主System位图字形生成
   匹配原字高、基线、内外行距和CP1252字符advance的私有FNT。不移动/放大控件。
   已经System的字体及NULL默认字体直接保留。应用自绘Score/Time不干预。
2. 菜单：仍以原8pt字体度量为尺寸来源，实际绘制用匹配的System字形；按窗口
   DPI换算一次。title10pt粗体MS Sans Serif创建表达式及整个frame绘制段保持原样。
3. 寿命：GUI线程共享变体引用计数；每次回调固定借用及出站字体，重入换字体、
   移除、销毁时原生回调返回前不销毁其字体。卸载恢复最新app HFONT。
4. 原资源与接口：六原文件逐字节保留、两DLL各18表；ABI3/31名称及helper不变；
   显式20px兼容字体API实现不改，System变体接口只在DLL内部使用。
5. 原生探针：实际候选DLL、两种DPI感知、12字号/字重组合、七类84控件场景，
   DS_SETFONT对话框、ASCII advance、System独立像素oracle、更新/NULL/卸载，
   菜单实际WM_DRAWITEM及重入换字体/移除。原8次完整预热后严格GDI不增长。
6. 交付：最终快照干净双宽全构建、完整相关core回归、实际四文件包14项测试、
   双位置发布、推送和实际远端复核。只关闭S2，不把自动探针当成用户外观确认。

搜索：`rg -n 'font|WM_SETFONT|dispose_control' src/ntw16s-dll/controls.c`；
`rg -n 'menu_font|title_font|refresh_font' src/ntw16s-dll/win16_style.c`；
`rg -n 'WM_GETFONT|font' tests/integration tests/ntw16s-dll`。
生产命中由controls（原生及自绘）、frame/menu（菜单字形）和font（资源所有权）
各自处理。tab/输入继续使用实际同一字体；已经System的旧tab oracle无需改写。

## 失败与边界

首次创建因System零宽字符被错误拒绝；修正为合法零宽/空字形后继续。
扩展像素及变体创建工作量超过旧CTest60秒，随后又命中隔离worker原90秒；两次
均记失败/未定，不计通过。最终专用worker170秒、CTest及包单项180秒，固定8次
预热和严格资源断言不放宽。探索构建s2-release不交付；最终从s2-final独立干净
快照重建四文件，不以探索包代替最终源码构建。

原生探针在私有桌面执行，无computer-use、物理键鼠或全局字体安装。
位图范围CP1252；字高1..64/字宽最多128及FNT v2容量，失败保留原app字体。
不承诺任意字符集/字号全覆盖，名义192字号不冒称真实192-DPI显示器。
原XP sol.exe不在本仓库受控fixture中，最终外观由用户继续反馈，T8保持开放。

## 最终交付验证

最终s2-final独立源码快照干净双宽全构建，ABI各1/1，126编译输入与当前源码一致。
纯C17 GCC16.1.0/Ninja/ccache4.13.6；两DLL ABI3/31名称及helper保持。来源门禁
124处置/294 include/123回执，SelfTest23/第三方9/原资源6/治理13负例均通过。
完整相关覆盖以注册全集x86 26/x64 29冻结：其余CTest分别24/27，字体和菜单取消
每宽两项由实际包直接提供。最终53通过/2 clipboard权限跳过，不虚构一次完整CTest。

实际包14/14，双宽System像素、全部可打印ASCII advance、字高/基线/内外行距、
原生DLU对话框、菜单WM_DRAWITEM像素、重入变更/移除和NULL/更新/卸载通过；
实际窗口DPI96/144，各84控件场景，固定8次预热后GDI43/43与45/45。旧S1两宽
DLL均在System字形断言失败，exit1。原显式20px兼容字体逐字像素及EDIT行为保留。
菜单取消每宽15场景通过，demo裸/注入advance均41；等待、传播、长路径及错误回滚
使用实际包通过。完整传播contract/dialog/message矩阵未在本S重跑，不借历史数字。

首次直接CTest缺捕获环境造成runtime/native/failures失败；另control-present缓存
39/45首次失败。正确环境完整重跑后双宽均通过，控件各类严格无增长断言未修改。
期间误用Build-Core重配置触发scratch重链，正在运行font fixture导致链接失败；此
失败不记通过。最终候选始终未变；scratch x86 DLL恢复为原候选四文件中的同一DLL，
随后完整相关复测及包/构建摘要门禁通过，126源码输入无变化。不是另发一批新文件。
所有失败、最终JUnit和包记录SHA提炼于release，不删失败冒称全程无异常。

已将四文件发布assets根及O:/winnt/system32，先备份8条并逐项核对before/after。
8份before与Git 1c82c2e assets相同，可从该提交恢复；新SHA、PE/版本/imports、
快照、命令和工具链身份见[发布清单](../etc/release.json)。新原生demo预览保留于
本聊天visualizations，不是第五产品，不是实际XP sol.exe或物理输入验收。

统计范围src/tests/tools/CMakeLists：7路径，增加236、删除21、净215；无旧路径移动。
GUI线程拥有变体及其资源，应用字体借用；S1原字体直用策略由度量匹配System策略
取代，title及显式compat API保留。当前仅待提交推送及实际远端代码/证据复核。
