# M0 T8 S1 — 字体与原生布局兼容

基线101e92dc03d52524c007652f983d27080e5c4a04，用户提供sol.exe Options截图：
Draw/Scoring、checkbox等文字裁切/重叠，而程序自绘Score/Time较小。用户批准按
字体职责调整并发布验收，准入Corrective T8 S1，产品版本0.8.0.0，永久M0。

## 失败契约、修复及所有权

旧renderer把宿主System16粗体重采样为20px FNT，frame和标准控件统一使用。
controls安装后强制WM_SETFONT，后续应用WM_SETFONT也被替换，而native dialog
已经按应用原字体完成DLU布局。图中问题不是仅凭视觉猜测：旧实际T7 DLL被新
实际加载探针拒绝，附着前后的字体handle不相同，详见最终证据摘要。

修复保持标准控件的原HFONT（包括NULL默认值）、字重/高度/字形宽度及后续
WM_SETFONT；画图、原生输入和list tab换算使用同一有效font，不改应用rect或DLU。
精确保留原字体是本次按原布局匹配的实现，不另选一个可能更宽的近似face。
控件只借用app字体，卸载保留应用最近设置，不释放或全局安装应用font。
标题用10pt粗体、菜单8pt常规MS Sans Serif，按窗口DPI一次换算；两个HFONT由
GUI线程style_window拥有，原子刷新、失败不丢旧font，销毁各释放一次。
legacy显式公开font_acquire/release仍为原20px兼容API，ABI3不增加/删改名称，
实际frame/control默认不再使用；原font bitmap测试仍验证该显式API而非新默认。

保持tracking_menu、calls/detached重入/清理及原生控件通知；不改启动/传播、
sol.exe、应用自绘文字、全局设置或五份图形头/原声明。原resources.md字节不变，
其中历史20px字体描述不被当成当前默认策略。原资源18表/声明仍单独核验。

## 有限自动验证与失败处置

新增font_layout原生C探针显式LoadLibraryEx候选DLL及公开frame/control/edit入口，
在owned私有desktop验证真实DS_SETFONT dialog、四个checkbox标签完整fit、字体
handle/文字度量/子控件rect不变。覆盖check/radio/group、STATIC、EDIT、LISTBOX、
COMBOBOX七类，12种名义font组合（96/144/192等比例字号、普通/粗体、两个DPI
awareness上下文），84个场景；后改italic font及NULL、重复安装/移除同样检查。
名义字号不是实际192-DPI显示器，实际窗口GetDpi另记；不冒称物理输入或任意应用。
frame三次SettingChange刷新、重复创建/销毁后要求稳定GDI零增长。

开发首轮font_layout对EDIT误用generic install，错误50，改用公开edit专用入口；
tabs旧断言要求附着时font16变20，按新契约改为保持16，应用明确设置20后新tab
请求才使用20度量。demo对照由“styled文字更宽”改为“相同文字宽度”；自动注入
及传播用真正Ntw16sWindowAttached/module/通知证据，控件保持原NULL字体。

资源检查开发记录：首次只预热一轮，GDI22/39；增加消息清理/GdiFlush仍22/34，
逐组日志呈延迟分配而非每组必增。沿用已有control-present/runtime记录的native
USER/theme异步缓存机制，固定8轮同样完整工作负载预热，保留零增长要求，
没有按“等到通过”循环或放宽断言。开发32位实测aware0 43/43、aware1 45/45。
随后增加三次frame刷新与DPI记录并冻结，由最终包重新测试，早期开发通过不代替
最终源身份。所有早期失败保留到原始证据提炼及实际复核后回收。

相似问题搜索：`rg -n 'font_acquire|font_release|WM_GETFONT|WM_SETFONT|System20' src tests`。
生产处置为controls借用及更新、list tab用相同有效font、frame创建/刷新/销毁；
font.c保留显式兼容入口，非默认；旧geometry/原声明保留，不以资源替换修字号。
注入没有TextOut/CreateFont全局hook，应用自绘字体仍由应用自己决定。

## 最终交付状态

最终126输入已冻结；本S干净双宽全构建与ABI各1/1通过。只对两个非产品测试修订预热并重建测试，产品不重链、四SHA不变；保留初始/最终快照。最终自动结果及双发布如下，实际远端复核已通过，见下文。原sol.exe二进制不在本次受控fixture
中，自动对照证明消息/度量/布局契约，用户对其真实外观的验收仍pending。

本任务全集为：原生布局/字体、后改font/NULL/tab、frame角色/DPI/所有权、
菜单/控件/启动传播回归、原资源/ABI与同包双发布、用户外观验收六单元。
只有前五项实际通过才能发布；本轮S1自动交付可在实际复核后关闭，T8按用户最新指令保持开放等待外观验收。

首次最终回归执行会话丢失，x86日志停在menu-dispatch且无JUnit；实际包停在裸demo About阶段、退出1073807364，无完整结果。无残留owned进程；该轮不计通过。保持同一冻结126输入/四文件，从regression-final和package-test-final重跑，不替换包。

用户随后明确“做到本次调研修复收口，保持T任务开放”：本轮S1以修复/自动测试/双发布/推送/实际复核/清理闭环收口，T8保持开放接收外观验收或后续调整，不虚构整任务完成。


完整回归重跑中，x86仅menu-dispatch资源计数失败：测量round10从43到49，消息路由
均通过。新native role font去掉了旧每次慢速FNT光栅化，使原8轮预热只占很短时间。
改为固定64轮完整minimize/restore/menu attach/detach预热，再16轮严格零增长；
不根据计数轮询、不Sleep、不放宽检查。开发实测最终47/47，鼠标/系统菜单损伤
断言及消息转发通过。此后重新冻结测试并重跑完整core，不将旧失败计通过。

这一修订只有menu-dispatch测试和对应回执，125个其余编译输入逐字节不变。
保留原inputs-build.json/snapshot.json，另记最终inputs.json/snapshot-final.json；
在同一干净树仅重建双宽core-menu-dispatch，Ninja dry run两宽无剩余工作，四产物
未重链、SHA不变。已通过14项的实际包仍是同批文件；不能虚称此次测试调整后又
重建了四产物。最终完整回归记录进入regression-verified，原失败JUnit保留待复核。


regression-verified的x86全量26项中，仅core/default资源等式失败15/18；menu已
通过，其他24项通过/1 clipboard权限跳过。default原先只30个快速事务且未排空
原生消息，旧FNT慢路径移除后不足以预热。修为固定64x30同事务+每轮消息排空，
仍严格after==before；实际同DLL复测1/1通过。类似扫描检查所有GetGuiResources：
font-layout完整预热、control-present完整输入closure、runtime三轮+20轮及显式
font API重建保持已有断言；terminal实测通过，不按猜测扩大修改。

default修订仍只测试/回执，其余125输入与此前x86完整执行字节一致；两宽只重建
该测试、四产物仍原SHA。最终32位覆盖明确采用该26项完整运行+唯一失败default
复测替代，保留原失败JUnit和单项JUnit，不伪造一份全新全量运行。64位以最终
测试源码完整运行29项。最终合并只接受被替代的指定失败名、严格核对注册集合，
并记录两次原始来源与输入不变证明；不隐藏开发失败或虚报测试次数。


## 已验证交付结果

32位完整26项仅default资源断言失败，其他24通过/1跳过；修订后default1/1通过，
最终覆盖25通过/1跳过。完整注册集合与全量JUnit逐名一致，只允许该指定失败被
原始单项报告替代，其他125输入不变；没有生成虚假全量JUnit。64位最终29项，
28通过/1跳过，注册与执行集合一致。合计53通过/2隔离clipboard权限跳过，
无未解决自动失败。此前中断、menu/default失败及修后证明均保留并列清。

实际候选包14/14通过，字体两宽各12组合/84控件场景与原生DS_SETFONT dialog、
后改font/NULL、移除/销毁及三次frame刷新通过；DPI context两宽GDI43/43、45/45。
实际窗口DPI96/144，名义192字体向量不作多显示器192DPI证明。实际旧T7 DLL两宽
均在font原handle身份断言line21失败、exit1，负例有旧DLL/探针SHA记录。
裸/注入demo同文本advance均41，caption像素及模块外观区分仍通过；原生抓图
人工检查没有文字挤压。该抓图是demo，不能冒充用户XP sol.exe验收。
菜单两宽15场景及cmd等待6场景、自动注入/四位宽边传播、长路径、CLI回滚均通过。

同一被测包四文件已发布assets根目录及O:/winnt/system32；8条备份/before/after
摘要一致。产品0.8.0.0，ABI3/31名称、六原资源blob/18表/声明不变。四产物SHA、
工具链、126输入、初始构建与测试修订快照、原始报告SHA见release.json。

代码统计基线101e92d，src/tests/tools/CMakeLists.txt：18文件+183/-51、净+132；
CMake1文件+8/-1，src6文件+41/-31，tests7文件+128/-15，tools4文件+6/-4。
原强制20默认分支被借用app字体路径替代，legacy显式API保留；没有替换原图形或
添加外部字体文件。frame两个HFONT各自拥有/释放，controls不再释放借用font；
窗口/控件原calls、detached、tracking_menu及应用业务布局/自绘字体保留。

| S1/任务覆盖单元 | 处置 |
| --- | --- |
| 原生布局与字体 | 原生dialog、原HFONT/rect/文字fit，双宽实际包通过 |
| 后改font/NULL/tab | 七类字体更新/移除及tab同font度量通过；默认按钮功能与资源复测通过 |
| frame角色/DPI/所有权 | title10/menu8各自HFONT；三次刷新/重复销毁GDI无增长；只声明实际96/144与名义字体向量 |
| 菜单/控件/启动传播 | 53项相关通过/2权限跳过，含明确x86单项替代；实际包14/14 |
| 原资源/ABI/发布 | 六原blob/18表/ABI3不变，8条双发布SHA一致；P1推送实际复核通过，S1收口、T8保持开放 |
| 用户外观验收 | 交付供sol.exe Options/标题/菜单验收；T8保持开放，未宣称该单元完成 |


## 实际协调复核、清理与S1收口

P1为980b49693c59b3b4947109990843ec71cfeb3237，已推送origin/main并fetch。
切换协调复核角色，实际读取远端提交/代码diff及原始截图/用户指令，确认修复仅
改变font默认职责/所有权和必要测试，不改应用自绘/布局、启动传播、ABI或原资源。
逐项读回126 Git输入（LICENSE单列checkout换行）、六原blob、四文件远端及
assets/system32 SHA，与已测包完全一致。原始14项exit0、x86完整26项和唯一
修订default1/1、x64完整29项的注册/执行集合和失败处置、两个权限跳过、font
12组合/84场景/两个实际DPI上下文/GDI等式、菜单双宽15场景、demo宽度41/41、
旧DLL负例、初始/最终快照及只有两个测试变更的边界均实际读取，摘要吻合。
不以总结代替检查，不隐藏旧失败或假称重新全量运行/重链产品。

8份before逐项与Git 411ab67 assets相同，恢复性复核通过。核对精确绝对目录
O:/repos.hobby/win16style/build/t8-package、祖先/子项无reparse、无活动owned进程后，
回收10顶层目录、1342文件、68852600字节。保留稳定缓存、源码、两处四文件及
visualizations中的实际demo预览PNG；未动其他任务或用户目录。原始证据SHA及
关键结果已提炼，旧before可从T7 P1恢复。

本轮S1调研/修复交付满足编译、自动覆盖收敛、同包、双发布、推送、实际复核和
清理，P2记录S1收口，产品输入及四文件与P1无变化。**T8保持开放**，待用户实际
sol.exe Options/标题/菜单外观验收或后续调整；没有把T8/M0写成再次全部完成。
