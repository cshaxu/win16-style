# M0 T27 S3 Win2000复选、单选和三态

S2已交付P1 4912b70d，双宽干净同快照八文件先发布后36/36实际包聚焦通过，实际远端八文件/38变更字节复核；9个旧输入行尾差异已披露。T27/M0保持，按用户已批准范围和16S顺序自动准入，不另开T。

## 先依据，再实现

逐项读取并核对固定Win2000SRC的btnctl.c、rtl/draw.c、kernel/inctlpan.c及原资源构造：六类checkbox/auto-checkbox、radio/auto-radio、3state/auto-3state，以及适用pushlike。确认unchecked/checked/mixed、pressed/disabled/focus、UI提示、标签布局/字体、父背景、字形来源和实际原图可见状态。不要将Win3.x位图、主机原生色或凭印象矢量当原版。源内缺Marlett文件时先查构造/真实图证据，不能凭字体名字宣称原字形已取得。

产品继续使用S2单一Win2000控件对象与公共instance/install/retire/basic/snapshot/present/dispatch/theme所有权；扩展真实角色与呈现，不复制Win16状态/输入/自动radio分组。复用S2有据pushlike边缘/灰模式，原生继续拥有BM状态、点击/通知、键盘和应用字体/图像。其他控件与DialogManager容器统一应用仍由原S4及后续承接，不把单项变成整T完成。

## 有限验收与效率

实施前冻结本组实际类型/状态矩阵与来源、真实截图/资源证据。v5/v6、normal/aware、双宽、已有/新创建、角色切换、字体/图像借用、移除/恢复/重入销毁、重复资源平衡及旧push按钮回归都合批增量验证。消息输入、物理输入、原生保留、真正年代呈现分别记录；未运行/skip不算通过。

吸取S2教训：先把所有本组兼容分支与实际启动入口纳入开发测试，再冻结必要的完整交付快照，不提前多次完整构建。每批实施后/约30分钟自查效率并归档，明确下一步；不为无新增证据反复测试。只用自有有界当前桌面原生探针，不创建/切换隔离桌面、不computer-use、不开全局样式测试或扩权限。

有二进制变化时按C17/GCC/Ninja/ccache双宽干净同快照八文件，先发布assets根与O:/winnt再测试实际包；提交推送/实际远端复核与即时scratch清理后自动下一S。原图形/声明保持，外部源码只读参考，无兄弟仓库依赖。不将停用称物理卸载。

本S初始恢复时尚未登记S3临时构建或修改S3生产代码；先恢复Task Reading Set和本组依据审计。

P2准入实际复核：353646af已push/fetch，7个文档变更字节一致，src/tests/tools/CMake/assets树及八文件与S2 P1不变；恢复时无活动构建/未提交源码。当前开始本组来源审计。登记build/t27-reference/s3-controls-original-images-01仅用于已核实真实原图与原生字形对照，S3拥有，下载/探针完成先归档URL/SHA/可见状态再清理，不作为产品资源或构建依赖。

原版新增依据：inctlpan.c DFC_BUTTON明确cx=cy=13；DrawBox用Marlett的gfedcb/nmlkji六层和3DLIGHT/3DDKSHADOW/3DHILIGHT/3DSHADOW角色，非自绘猜测。实际已看Mouse/Effects/TimeZone真实图，Effects像素(37,245)和默认OK内右缘(231,425)均64/64/64，外框(232,425)为0，确认S2的dark=0取错；本S改正共同palette并新增此原图负例，不追写旧测试为充分。当前13像素原生字形对照原型仅验证当前系统Marlett与原图覆盖状态，不声称取得原版字体文件。登记同一参考scratch内glyph_reference.c/.exe/.bmp；C17/GCC/ccache20秒预算，纯内存GDI，无窗口/桌面切换，完成对照即归档删除。

本组审计/设计冻结见[证据](../etc/evidence/t27-s3-choice-controls.json)：四种原版13×13字形全169像素逐点匹配当前系统Marlett分层原型；未选probe对选中原图差21点，非空白自比。六种native类型共14个check状态，ordinary/pushlike两种×normal/pressed/disabled三种=84次实际render/模式，v5/v6及aware共4模式×双宽计划672（仅计划，须实际输出计数）；额外独立输入、背景/裁剪、UI提示、图像/字体/退役/资源与旧push回归。

职责设计：将common/edge.{c,h}实质合并命名为common/paint.{c,h}，拥有该年代fill/edge/dither/push-frame/choice-glyph；Win2000普通按钮和选择控件、公用Ntw2ksDrawButton导出消费同一原语。原型不是另一套生产路径。common palette改为年代通用名称并更正deep-shadow64。低层common独立静态目标只依赖Win32 GDI，控制器继续依赖共同application对象，产品仅接线；不移动或复制Win16渲染，不改窗框布局/全局机制。对应common测试同名paint，控制类所有BUTTON子型仍由button_paint对应测试一次覆盖，保持单一native输入/生命周期。

效率检查：本批先从原图/SHA/原源码确认13px、字形层、父刷坐标、半个0字宽留量及mixed的DSS_MONO，而非先实现后补猜。源Marl文件未取得但四个覆盖状态输出完全等同，未覆盖pressed/disabled/mixed截图只按已固定源算法报告；不伪称字体文件原样移植。下批合并公共原语/配色、6类型和全部增量兼容测试，验证实际入口后才一次冻结交付。参考原型及图片用途结束，归档后立即清理，无构建/运行网络依赖。

登记build/t27-package/s3-choice-dev-01：相关common/实际DLL BUTTON及v5/v6/aware、导出/theme合批增量，180秒/宽，稳定Ninja/ccache，无其他活动worker。当前首次编译实施批，不作正式发布/整包冻结；源回执同步重命名/摘要，低层common仅GDI、产品生命周期仍共享既有对象。参考scratch151048字节已清理。

首轮dev-01已结束：x86实际v5/v6及aware四模式各84样本、原生auto3/radio及odd-offset绘制均通过；common低层断言把default-pressed的WINDOWFRAME外框也误改为64而失败。修正该测试为原图确认的外框黑0，内3DDKSHADOW64仍严格保留；不改生产代码掩盖失败。失败日志已归档，x64尚未由该worker执行，不计通过。继续补全相关背景/对齐/重入与启动契约后再下一轮增量，不作整包冻结。

登记build/t27-package/s3-choice-dev-02：合并底层断言修正、真实BUTTON的右字形及上下对齐/角色变化、焦点与Space消息、flat/多行、应用父刷、父回调移除/销毁；同一7项模式双宽增量，180秒/宽。dev-01原始CTest日志已保留，清理其已完成scratch，不保留重复构建副本。

登记build/t27-package/s3-choice-startup-dev-01：仅临时合放稳定增量树同批八文件，验证普通启动v5/v6双宽真实自动附着；不是正式包、不发布。4次自有有界当前桌面启动，全部完成即归档/清理；不使用全局模式。dev-02双宽14/14已通过。

增量完成：双宽7×2=14/14，8个模式共672次choice render及各自原生输入/背景/重入契约通过；普通启动v5/v6双宽4/4另336次render通过，先证明自动附着后才显式attach。来源/边界自检通过，三个已完成开发scratch归档后清理。登记build/t27-package/s3-choices-release-01，冻结本组全部相关生产/测试/工具，一次同源双宽干净交付构建（600秒/宽，1200秒总），保留ccache；正式八文件仍S2直到发布，未提前称S3包通过。

登记build/t27-package/s3-choices-current-desktop-01：完整构建发布后运行同S2必要产品组合并新增choice标记共36项，全部针对O:/winnt同摘要文件；本目录仅含有界runner/日志及隔离DLL搜索的两宽theme测试EXE，不含私有桌面，900秒总预算，完成先归档再清理。

## 本组职责与相似问题收尾

| 能力 | 唯一生产所有者 / 实际消费者 | 验证 |
| --- | --- | --- |
| 标准颜色、矩形边缘、灰纹、按钮框、选择字形 | common/palette.c、paint.c；独立win2k-common目标；button_paint与Ntw2ksDrawButton调用 | 原图4×169点、旧dark负例、default/pressed/mono与DC/GDI；实际导出 |
| 选择/标签/父刷布局呈现 | control/button_paint.c；同一Win2000控件对象paint回调 | 每模式84状态；right/top/bottom、native Space/focus/auto3/radio、odd-offset、custom parent、重入remove/destroy |
| 安装、重入持有、退役、主题归还 | 既有application instance/install/retire/dispatch/basic/present、ntwshk唯一theme所有者；control.c组合 | 原生销毁/移除、重复/错线程和Win16到Win2k主题所有权；不复制状态机 |
| 普通启动 / 既有DLL接口 | 既有产品CreateWindowEx装配与Ntw2ksWindowAttach | 双宽v5/v6启动先assert已附着，现有启动/传播与demo回归 |

删除旧common/edge及control内重复push框实现；新common/paint是完整同年代原语集合而非空转发；产品button导出只依赖common，不引入控件生命周期。Win3.x renderer/图形头/来源声明未改，不另造输入/传播/全局路径。Win2000不支持的owner-draw、RTL、nonpush图像和v6新image-list依既有native边界保留，不能把保留称年代实现。

色值相似搜索`rg -n '3DDKSHADOW|RGB\(0,0,0\)' appearance/win2000`：本S公共控件palette及所有旧按钮测试期望已改为dark64（WINDOWFRAME外黑0仍正确）。window/frame.c约202行旧caption DFC局部配色仍将3DDKSHADOW映射0；不是本S公共控件调用方，交既定S15窗口/控件横向配色一致性核对，不在本组暗改窗框/DWM或宣称全Win2000配色已审完。DialogManager已创建子控件观察缺口由固定S4接收。

登记build/t27-package/s3-choices-publish-01：同批八文件先发布assets根/O:/winnt，保留本S两处原八文件备份仅供实际包验证失败回滚，测试通过并归档摘要后立即清理；不增加产品文件或发布目录。

正式交付本地验证：双宽干净GCC/Ninja/ccache（x86 259.06秒，908hit/3miss；x64 281.87秒，996hit/3miss）、8文件包/原资源/依赖门禁通过；先发布assets根/O:/winnt，再实际36/36聚焦通过，无skip。12个实际BUTTON模式各84状态共1008次render，不称1008独立进程用例。首轮32/36中4项仅runner遗漏WIN16STYLE_CAPTURE_DIR，原始失败保留，补环境后仅4项重跑通过；无产品修改或再次整包构建。按证据记录回收3个正式scratch及两处备份，保留稳定Ninja/ccache；S3待提交推送及实际远端复核，未提前关闭。
