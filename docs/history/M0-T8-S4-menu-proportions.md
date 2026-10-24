# M0 T8 S4：菜单System比例与布局

用户明确授权，M0/T8不变；S4纠正S2菜单把16px System字形逐字缩到8pt
MS Sans Serif字高/advance的失真。基线a9ab2ab，工作区原先干净；S3有效工作保留。
本S只收口菜单修复，T8继续开放，最终交付状态见CURRENT。

## 实现与所有权

菜单栏/popup使用独立GUI线程引用计数菜单字体，以宿主System16原字形宽高按
窗口DPI统一比例生成私有FNT；整数像素advance仅作相同比例取整，不再借用
MS Sans Serif逐字宽度。标准控件原度量适配路径/NULL默认/更新/卸载不变；
公开20px兼容API原代码和ABI3不变。标题创建表达式、所有refresh前的绘制/布局
代码（仅owner struct删menu_source字段）、controls.c和S3 menu_scope.c逐字节保持。
菜单字体与控制字体变体用独立cache键，原生调用结束后释放，未新增公开导出。

popup按整组最大标签宽/快捷键宽，统一快捷键起点；左勾选区18逻辑像素、
快捷键间隔8、子菜单箭头区12（没有子菜单4），行高至少容纳原14px勾选图和
字体高度，上下各一个DPI单位。菜单栏自然字宽和6px两侧留白，换行测绘同源。
USER还会在ownerdraw请求宽度外增加宿主菜单字体两个平均字符格，本机96/144
DPI分别14/20px。按查询得到的宿主菜单alphabet平均格扣除该原生留量一次；
宿主字体不参与System像素生成。独立自然System布局与实际GetMenuItemRect宽
严格相等，避免重复留白。查询失败只保守多留空间，不裁切/压缩字形。

不改HMENU样式/共享类/SPI全局设置；S3完整WM_PRINT坐标和局部SysShadow抑制
原代码保留。新菜单尺寸下复验同GUI线程/z序/原点/扩展边界关联的独立SysShadow
隐藏，原生窗口仍由USER拥有。本机扩展向右/下各5px。六原资源/来源声明字节不变。
应用自绘文字没有安装字体或绘制拦截，sol信息栏保持应用责任。

## 有限覆盖与探索处置

有限全集：两宽实际96/144 DPI菜单栏/popup System像素、整组快捷键列和实际宽高，
勾选/箭头/嵌套/禁用/分隔符；NC四边/打印客户区完整/独立投影隐藏；15取消场景
每DPI含焦点/捕获/命令/数据恢复，菜单样式及共享类/SPI前后身份；控件原策略
像素/度量/字体更新/NULL/重入/卸载及GDI；标题源码/原生像素对照；原资源/ABI；
同批包/双发布/实际远端复核。原生WM_PRINT截图仅拥有的私有窗口，不冒称物理输入
或用户桌面合成截屏；XP sol.exe、真实192DPI及任意宿主/混合owner-draw菜单不扩大承诺。

搜索：font.c/h、win16_style.c的menu_font/refresh/measure，menu_paint/items/bar/layout
的measure/draw/shortcut/check/arrow，以及menu_scope的shadow/print/NCPAINT调用。
菜单测绘同源修正；controls.c、menu_bar.c/paint和menu_scope.c无改动；既有frame
DWM恢复疑点保留TODO，未扩围修改标题路径。

探索失败如实保留：

- 探针一处if后同一行递增触发-Werror，已改为明确独立语句，警告要求不变。
- 菜单扩大后，旧外部点击(40,90)落进系统菜单，case11焦点断言失败；改用实际
  客户区右下角外部点后保持全部取消/焦点断言。不是放宽为忽略焦点。
- 新菜单栏像素探针将HMENU挂到窗口后，DestroyWindow已回收该菜单，旧显式
  DestroyMenu报1401；先SetMenu(NULL)再逐项销毁，资源所有权明确。
- 额外留量不能由SM_CXMENUCHECK直接推断：96是15但实际追加14，144是25但
  实际追加20。局部MNS_NOCHECK实验未改变ownerdraw留量，已完全撤回，无菜单
  style修改随产品交付。最终依据宿主alphabet格并由实际矩形独立断言复验。
- 一次探索构建与自有x64探针重叠，DLL正在被加载导致链接输出占用，探索包未交付；
  此后结束原生进程再构建，最终独立干净源树不重配置/链接正在测试的树。
- 首次干净树回归dc-failure仍断言旧popup/bar固定留白差24，公开row helper无submenu参数，必须保守预留箭头区，
  新公开契约为18+12-12=18（实际HMENU无箭头时才用4px尾边）；
  自审补齐公开helper箭头空间，避免独立measure+draw(submenu)重叠；更新该布局断言，保留DC状态/绘制/失败/寿命断言。最终四文件从更新测试的
  同一干净源码重新构建并重跑实际包，不沿用首批包的二进制证明。
- 临时旧标题预览先后遗漏私有头目录/Windows版本宏，编译拒绝后使用正式同等
  C17/Unicode/Windows10宏；仅标题对照，明确允许旧比例不符，不作回归通过。
- 短菜单/预览探针打印ONLY标记，不计入完整控件/菜单回归；最终结果另记。

探索批次s4-final实际包14/14，但相关dc-failure旧断言失败；s4-release在公开
helper箭头自审后被替代，均未发布。最终仅s4-delivery同批四文件作为交付候选。
## 最终验证与双发布

最终s4-delivery独立干净双宽全构建/ABI各1/1，GCC16.1.0 C17/Ninja/ccache4.13.6。
126编译输入与冻结源码完全相同，无构建后源码修订；六资源及两DLL各18表不变。
来源124/294/123，Source23/MinHook9/Resource6/Governance13负例门禁通过。

实际包14/14；相关注册全集26/29，由剩余CTest24/27和包内双宽font/menu组合，
53通过/2clipboard私有windowstation权限错误5/77跳过，不是一次完整CTest。
两宽96/144 DPI菜单栏和popup独立System原像素/advance等比、快捷键列与实际
popup自然宽108/162均通过。每宽30取消场景、全NC和客户区角落、嵌套/命令/
焦点/捕获/样式恢复、同线程关联5px SysShadow隐藏及原生前后路径/共享类/SPI
身份通过。控件84场景类型/12组合及GDI43/43、45/45保持；legacy20的224字形/
39100像素及GDI7/7通过。公开row helper/DC状态/失败/菜单回调终结探针通过。

两宽旧S3各两个负例exit1，分别拒绝压缩菜单栏像素和不符合自然System的实际宽度；
最终fixture源码与执行负例者逐字节相同。原生完整标题RGB对照：96 DPI比较
642x25区域48150字节，144比较640x35区域67200字节，差异均0；alpha不作颜色。
旧标题临时预览明确仅允许旧布局不符作截图，不当完整回归通过。最终打开菜单图
来自实际包probe，BMP到PNG仅格式转换、不重采样，非XP sol.exe/物理桌面截图。

实际sol.exe、真实192DPI、任意宿主/外语字体及混合ownerdraw菜单未扩大承诺；
可选完整creation/dialog/message integration本S未重跑。应用自绘不改，既有DWM
卸载恢复债务仍由TODO接收。无额外产品、兄弟仓库修改/依赖或全局/共享类修改。

测试后同批四文件先备份8条，再发布assets根与O:/winnt/system32；8摘要一致，
全部before与Gita9ab2ab assets相同，可从该提交恢复。具体快照/摘要/工具链、
PE/imports、统计和原始证据SHA见release.json；统计src/tests/tools/CMakeLists
共9路径，增加274、删除68、净206。未移动旧路径，删除不再需要的menu_source
句柄；所有者仍为GUI线程引用计数字体/菜单适配器，USER拥有native窗口。

## 实际复核及S4收口

P1 64e3fd30634bcfa426fe1dbc4eec5a0f00c24504已推送；读取origin/main实际提交/
完整diff并逐项复核126输入（LICENSE换行约定明确）、六原资源blob/SHA、四远端
产品和两处8副本、14实际包exit0、注册全集26/29与JUnit/package精确组合、每宽
30菜单场景/独立System像素/原生宽度/细框与SysShadow/控制及标题保留、四旧包
负例，167份原始证据SHA全部一致。额外核对保留PNG、负例fixture源码同一身份、
旧DLL及8份before与a9ab2ab Git assets相同。未改rules/AGENTS/CMake及保留源码。

复核通过后确认精确绝对目标/祖先与子项无reparse、无活动owned进程，仅删除
登记build/t8-package：21顶层目录、1671目录、3316文件、162916014字节。稳定
.ninja/.ccache、两发布位置和原生菜单PNG保留。原始日志/位图先实际检查并提炼
结果/SHA再回收，旧包8份before均可从Gita9ab2ab assets逐字节恢复。

P2只更新实际复核/清理/收口文档，126编译输入与四产品和8发布副本与P1完全
相同；没有新编译/发布。本S4闭环完成，M0/T8保持开放，等待本轮用户外观反馈。
