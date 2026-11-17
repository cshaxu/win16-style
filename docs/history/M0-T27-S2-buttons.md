# M0 T27 S2 Win2000基础呈现与普通按钮

按已批准16S规划自动进入，T27/M0保持。先读取S1参考清单的btnctl/draw/inctlpan与原图事实，补齐本组状态算法后实现；不把关闭Win11主题或原生DrawFrameControl未对照结果当原版证明。

## 完整能力与验收

本S提供Win2000语义palette/edge与普通/默认push按钮呈现，覆盖已有支持的按下、禁用、焦点、文字/图像、字体、UI提示和查询重入；遇到尚未验证的模式保留明确原生边界，不能算parity完成。先验证96DPI与真实Accessibility Options原图可见的边缘/色值，再验证当前适用DPI/aware与原生输入/打印/资源。

年代实现归appearance/win2000/common/control。产品ntw2ks只选择并调用安装/移除/原API适配；直接复用公共instance/install/retire、control/basic、button_snapshot、present/DC等完整机制，不复制Win3.x私有状态或共享原字形。此实现不安装另一套全局/传播后端，不改已确认样式选择UX。

S1源码确认xxxDrawButton对default加外边框，WIN40COMPAT使用BF_SOFT、default pressed使用BF_FLAT；DrawPushButton使用raised/sunken与checked dither。颜色使用源码slot语义和原图已验证标准方案（face212/208/200），不是globals启动色表的192灰。应用字体/图像保持借用，底层原生数据/点击/键盘与通知保留。

开发用稳定Ninja/ccache增量树；相关实现/测试合批后才作本S必要双宽干净八文件交付，先发布assets/O:/winnt再跑实际包回归，提交推送实际fetch复核并清理。先实现并验证本组后交付，不宣称整个控件parity完成。

首批实现：Win2000 palette/rectangular edge、按钮frame/content与控件对象直接接入公共basic/snapshot/present/install/retire/dispatch，产品原导出与普通创建路径调用新profile。共享theme状态仍唯一归ntwshk，不将style-theme-state静态复制到Win2000 DLL。基础scope中borrow应用font，无位图字体替换或私有字体租约；绘制所需DC/文本/图像遵从原有借用/正常返回合同，外层引用复用已有native unwind dispatch，不虚构物理卸载。

补查btnctl的BNMultiExtent/Draw对齐：源代码使用DT_EDITCONTROL、水平CXEDGE/垂直CYBORDER留量及文本高度加一；default pressed须使用BF_FLAT，普通pressed不同。checked dither的8x8单色模式已在kernel/init.c LW_BrushInit确认，源文件是S2显式追加参考，不伪造bitmap。截图已有可见状态与源算法相互核验；现代imagelist-only内容当前保留原生，不冒称Win2000原版。

登记build/t27-package/s2-buttons-dev-01，S2拥有；稳定.ninja/.ccache双宽增量，每宽180秒，总360秒，后台监控同一worker和日志。只跑新edge/实际按钮和已有theme共享合同的聚焦编译/测试，不作逐文件正式发布。结果/失败/来源先归档，结束即清理本轮scratch，保留缓存。

首轮dev-01编译未通过：生产清理语句触发misleading-indentation；测试缺少_WIN32_WINNT版本声明。保留原始失败日志，不计运行通过。修正后登记build/t27-package/s2-buttons-dev-02，同一稳定缓存/预算继续；本S产品版本统一提升M0 T27。

首轮运行保留失败：精确BUTTON类额外比较GetClassInfo过程地址，遇到系统A/W分派thunk而误判，实际仍走旧原生主题分支。改用与既有Win3.x一致的已注册精确类名条件（别名留后续S），x86实际产品ABI像素/点击/重入移除销毁已通过初测。追加ANSI/Unicode、应用位图借用、禁用浮雕和64次资源平衡测试；登记dev-03双宽180秒增量聚焦验证。

dev-03新增位图用例首先失败，原因是测试误用BM_SETSTYLE更改BS_BITMAP（该消息并不设置此模式），改用实际GWL_STYLE并验证原生BM_SETIMAGE借用，x86直接原生测试通过。登记dev-04双宽聚焦复验，180秒/宽，不将失败计通过。

效率检查：本组8个生产C/H一次能力合批，复用既有公共生命周期；dev-04双宽各4项通过，追加消息焦点/UI提示、按下文字位移及多行文本x86实测通过。失败日志/工具链/缓存与源SHA已归档[t27-s2-buttons](../etc/evidence/t27-s2-buttons.json)，不再为各小文件交付。登记build/t27-package/s2-buttons-release-01：同快照双宽干净构建，每宽600秒总1200秒，后台监控；发布备份登记s2-buttons-publish-01，先发布后实际包测试。开发scratch归档后清理，稳定Ninja/ccache保留。

旧导出测试审查发现原测试把当前Win11 DrawFrameControl的整块像素当Win2000 golden，现标准方案原色已独立，必须同步改为原版语义色/状态与调用方矩形合同，并更新包marker，不能把测试遗漏当兼容通过。release-01有效构建不打断，先保留其结果；相同产品代码/修正测试合入release-02后作最终发布，使用缓存复用。

最终实际包聚焦计划：公共/原生按钮、aware、启动期自动按钮接入（必须先观察已附着，再显式幂等attach）、共享theme所有权/错误线程/恢复、Win16可见性与公共present、ABI、demo原生边界/配对、启动传播及双宽Win2000接入。使用已有--private-worker仅绕过旧测试创建桌面的外壳，并由dialog-test-driver --direct的Job约束自建测试树；不创建或切换桌面、不跑会无条件CreateDesktop的旧default测试（明确未覆盖，不计通过）。只对选定能力聚焦回归，不宣称全仓全部历史用例通过。

登记build/t27-package/s2-buttons-current-desktop-01，S2拥有有限实际包计划/日志/自有fixture；原生driver Job每项35秒，工具项最多90秒，总预算900秒，串行不争抢绘制。测试结束后归档命令/标记/退出码并删除此scratch；没有通过项可代替未运行项。

release-01双宽完整构建/ABI、两宽23公共样式归档及4进程归档、原资源/来源、实际八文件包检查通过；x86编译271.915秒，x64编译310.172秒。该轮未发布/未称实际包测试通过。结果与缓存计数归档后删除完整副本；登记release-02合入旧导出标准配色测试、实际启动期已附着观测及分配失败向上传递错误，600秒/宽预算与同缓存，作为最终交付。

兼容性自审补证：现代无BS_ICON/BS_BITMAP的ImageList按钮须原生保留，不得被纯文本呈现丢掉图像；补查询和V6实际资源/基线/打印DC合同，同源码两manifest消费者，复用既有RC，不新增图形。release-02继续有效构建不打断，最终交付前将此组增量验证合入最终快照。效率纠正：此前完整交付快照冻结早于所有兼容分支测试，导致额外构建；下一S先完成v5/v6/启动/资源有限合同的增量验证再冻结，不逐项提前冻结。

登记build/t27-package/s2-buttons-dev-05：最终v5/v6+aware/原生导出/共享theme共双宽聚焦，每宽180秒；先确保相关测试与源SHA一致再冻结最后发布快照。v6x86已通过，x64首次使用旧可执行文件（窄扫描区未重编译）而未找到位于x4的原生图像，重编译后原生marker=12和保留/资源合同通过；保留真实失败，不算产品修复或通过。

dev-05最终聚焦双宽各7项通过，无skip；x64实际v6启动期已附着、ImageList保留/调用DC、原生输入/资源/重入也通过。release-02完整构建与包门禁通过但未发布，已归档。登记build/t27-package/s2-buttons-release-03为最终同快照双宽八文件构建（600秒/宽），所有相关实现/实际测试已合批，不再拆项冻结。release-02和dev-05用途结束立即清理，缓存保留。正式发布仍用登记s2-buttons-publish-01；实际包测试脚本已指向release-03。

入口覆盖边界：S2已实证CreateWindowExA/W普通创建与显式API；现有controls.c dialog_created仅调用attach_frame，frame.c无子控件枚举，不能据此声称DialogManager内部创建的按钮也已覆盖。接收者明确为S4既有对话框/容器子控件统一应用及S15实际组合测试，规划已补充；不在S2冻结后凭空增加第二套发现机制。本T结束前必须实际验证该路径，不把此边界写成全T完成。

最终release-03双宽干净构建、同快照八文件包/ABI/来源/原资源/公共依赖通过；先发布assets根与O:/winnt再跑36项实际包聚焦，36/36通过、0 skip，所有目标由测试创建并Job有限回收，未创建/切换桌面。双宽各7项增量通过与此36项不是同一个计数。两处八文件摘要与候选逐项一致，生产/测试/tools/CMake快照输入逐项复核一致，详情归档证据。对话框内创建、原生实键鼠/连续画面及其他控件不在本组通过声明中。源快照/候选/测试fixtures/备份用途结束，立即按已登记路径清理，稳定缓存保留。准备S2 P1提交推送，实际远端复核前不关闭。

发布文档自审同时修正assets README仍要求四文件及release.json仍标T22的过时身份：当前按固定五组件八文件更新为T27本批摘要/工具链/有限36项范围。历史旧manifest仍可从Git取回；这是发布元数据更新，不更改已验证生产/测试/build输入或触发额外二进制构建。

代码审查计数（src/tests/tools/CMake含版本与测试映射，排除文档生成物/二进制）：21文件，+587/-37，净550行；新增生产8个C/H共337行。共同状态/安装/退役/缓冲/派发均复用，theme状态唯一留ntwshk，产品普通路径调用新profile；旧主机色primitive已替换，没有两套同一Win2000按钮renderer。暂存八文件逐字节SHA已核对测试批次；S2所有已登记scratch为空，缓存保留。治理13负例与diff-check通过后提交。

P1已提交推送4912b70d并实际fetch：38个变更blob及八个已测试正式文件逐字节一致。全1181输入复核中1172原字节一致，9个未由本S修改的旧文件仅工作区CRLF与Git LF不同，逐项哈希/差异见[实际远端](../etc/evidence/t27-s2-remote.json)；不虚构全树原始字节相同，不扩展本S去改原声明/旧用例。全部S2登记临时目录已删除，稳定缓存保留。基于有限S2完成条件收口，T27不关闭，按批准顺序自动准入S3复选/单选/三态。P2只记录复核/关闭和下组准入，无产品输入或八文件变更，不另造二进制发布。
