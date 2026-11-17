# M0 T26 S12 现代宿主与外框机制

## 准入与基线

按用户确认的14个S顺序自动准入；M0/T26保持，S11 P1 `f6c5185c`已经实际fetch并逐blob复核67个变更与八文件，见[远端证据](../etc/evidence/t26-s11-remote.json)。S11既有激活失败及更早DWM/宿主限制保留，不把91个有限回归扩大为所有应用。

初始52项源码/头/实际消费者/构建路径与SHA冻结在[清单](../etc/evidence/t26-s12-modern-audit.json)。这是核心入口盘点，尚未逐项审查或完成传递调用闭包；实施前必须核实每项状态所有者、公开入口、实际消费与retained理由，发现漏项显式补入。准入时没有S12代码改动、scratch或新执行结果；后续批次和实际验证见下文，不能把计划记为完成。

## 完整能力批次

1. 顺序审计existing modern host/input/surface/native/target及Win3x modern_frame的全部状态与调用，核对attachment的composed-frame判断/移除和产品/CLI/access实际入口。沿用S3/S5/S6/S7/S8已经共享的引用、菜单和接入，不再造第二套引擎。
2. 将仍混在Win3x中的通用宿主安装/失败回滚/重入退场、action菜单和所有者/标题/表面状态迁入application/adapters的完整对象。profile只给年代几何/命中/绘制和资源策略；具体字体资源归S13，不能用几十个字段指针回调替代状态所有者。
3. 合并实际产品装配、删除旧状态及空包装，按两宽现有适用范围验证native消息/几何/引用/菜单/资源/失败及重复移除。先审worker入口再运行，不使用隔离桌面，不把C模型代替真实框架支持结论。
4. 整组满足有限退出条件后一次干净双宽八文件构建、两处先发布再测试、提交推送及实际远端复核；每轮测试提炼证据并清理scratch。开发持续使用有效Ninja/ccache增量树，不逐文件完整交付。

## 保持的契约与非目标

所属GUI线程应用/撤销、模块和回调生命周期、actions复制与labels/context借用、菜单冲突和先清理后invoke、owner/title/surface坐标与隐藏/重绘排队须保持。普通窗口与XAML表面不是相同坐标系，不强行同一消息入口。传播停用、样式恢复和物理卸载分别报告，不新增后者。

不增加Win2000 UWP/WinUI renderer、Luna/Aero或客户区控件；不扩张全局目标、权限或系统设置。当前没有独立application/adapters/winui目录，按实际共用机制归属，不创建空目录冒称支持。受环境限制的真实宿主验证记录skip/未覆盖，不能当通过；已有缺陷保留原始对照。

## 退出与交接

所有有限输入及实际改动逐项归属、通用机制实际下沉且没有重复路径、公共反向依赖/资源/ABI/双宽及实际包验证、清理和远端复核成立后才收口。S13接具体renderer/字体与图形边界，S14做全范围终审；不提前称T26完成。

## 第一能力批次：完整现代外框对象

恢复时工作区干净；未发现本仓库有效的在途构建，不停止其他构建。已核对实际frame/input/surface/host/menu操作及产品调用：生命周期骨架复用现有host/instance，不复制安装器。现代frame的remove/surface不是DLL导出（对外仍是Ntw16sUwpFrameHook），可以让产品与测试直接消费公共入口。

公共application/adapters/frame拥有host、bitmap、刷新重入、输入pressed/hot、激活、menu owner、tracking及临时action命令；profile嵌入完整对象，一次calloc同时容纳profile和action尾存储。profile只提供几何/菜单锚点、paint、hit/button/command、菜单呈现和字体释放。所有操作表与label/context在完整对象及同步回调期间有效；失败和最后引用释放由既有host/instance结算。删除input/surface字段指针view，直接操作同一公共frame。普通非客户区的profile呈现选择不新增第二条安装路径。

迁移顺序：公共对象和input/surface同批修改 → Win3x实际消费者与公共remove/surface接入 → 公共独立provider及原profile生命周期/失败测试 → 审计其余access/native/target/CLI闭包。第一批不等于52项全部完成；renderer旧frame_view明确交S13，不在S12伪称已消除。

开发scratch仅登记`build/t26-package/s12-modern-root-01`，所有者S12，记录Build-Core命令/toolchains/run.log/run.err/result与双宽JUnit；每宽180秒、有界原生测试每例30秒。使用稳定.ninja和.ccache增量树。测试结束先提炼证据，核验无活动占用/reparse再清理本目录；不逐文件全量打包发布。整S终整后才做一次交付。

第一轮增量编译发现公共frame直接消费menu_owner后，独立provider目标缺少既有menu_popup依赖，x86链接失败，尚未运行x64/测试。保留原始日志于开发证据并修正公共归档依赖，不以产品原先偶然链接成功代替独立性。继续登记`build/t26-package/s12-modern-root-02`，同所有者/预算/日志/清理契约；第一轮目录归档后立即回收。

第一批增量修正后两宽编译/链接、每宽3项CTest通过；另以非沙箱当前桌面运行每宽9项原生测试（18/18通过），包含不链接Win3x的完整provider、256动作复制/校验、错误保留、paint中移除/title销毁、激活/隐藏/重复移除和原Win3x真实菜单嵌套退场。没有创建或切换私有桌面；原测试的`--private-worker`仅用于跳过隔离入口，输出历史private标记不代表本次使用私有桌面。第一次链接失败保留，没有将其记为通过。

效率检查：同一完整对象批次合并12个目标；修正后x86构建1.57秒、x64构建15.17秒，继续复用Ninja/ccache。独立provider暴露并修复了归档依赖缺口，未重复全量发布。下一批只审剩余生产接线/测试闭包与平台识别归属，不重做已通过的原生场景。root-02原始结果已提炼开发证据，立即核验回收；S12仍活动。

## 其余入口与保留边界

56项输入（原52+菜单公共契约2项及实际ABI2项）和3项新输出已逐项登记现有所有者/调用/验证状态；此处“消费/边界复核”不冒称重新运行所有集成程序。公共access/runtime uwp_dispatch/hook和access/global uwp_global本已持完整接入/队列/停止资源；平台target/native、UIA worker/module引用与repaint scheduler本已公共化，保留同一实现。

产品attachment的composed_frame_class是原Win3x支持名单，install_created仍按原style/exstyle条件选择年代呈现，移除已直接调用公共frame入口；不能把名单变成所有风格的强制目标策略。uwp/hook只保留ordinal38/消息/操作资格、模块pin、Win3x呈现选择和菜单标签，实际消息/移除仍公共。uwp/target只给既有x64模块/消息/export/timeout与fullscreen操作选择。CLI保留系统calculator身份/alias/AUMID和参数/wait UX；发现与执行调用公共target/dispatch。以上并非留一套平行生命周期。

S13精确接收Win3x frame_view的renderer字段视图及frame_font的年代字体选择终整；本S公共input/surface视图已删除。宿主DLL仍按既有契约固定，移除表面不称物理卸载。实际OS应用/托盘的环境条件和旧失败继续单列。

准备一次整S交付，登记`build/t26-package/s12-modern-delivery-01`（同快照源码/两宽干净Ninja与ccache构建/八文件candidate/工具链/ABI/公共依赖门禁；每宽600秒）及`build/t26-package/s12-modern-publish-01`（两个固定目的地备份/摘要；发布完成及结果归档后清理），所有者S12。最终测试记录放交付scratch，逐例有限等待，先发布后测试；禁止私有桌面和computer-use。测试-only或文档变化不触发逐文件全量交付。

终审补充实际产品消费者：既有window_thread夹具追加`--composed`，用自有Chrome_WidgetWin_1/NOREDIRECTION窗口通过真实DLL线程入口安装、显示并观察唯一layered表面，再从相同产品移除入口确认表面销毁和原标志保留。此为测试-only补充，不改产品名单/代码；干净构建快照仍保留原测试SHA，完成后明确登记该单文件/receipt快照修订并仅增量重编两宽夹具，核对八文件不变。不重启正在运行的整包构建，不把旧测试输出当新分支证明。

## 整组交付与有限复核

双宽同快照纯C17/GCC16.1.0/Ninja/ccache干净构建通过，约182.53/211.67秒；每宽22个公共样式归档和4个进程归档、外部依赖、ABI、原资源及八文件包身份门禁通过。八文件已先发布assets根与O:/winnt、两处摘要一致，再跑当前桌面正式包验证。1163项src/tests/tools/CMake输入与最终测试-only修订后的快照一致；原snapshot和两次测试/receipt修订均保留，没有将修订冒称初始快照，也未重建/改变八文件。

当前43项有限case中37通过、6未通过、0skip，见[整组证据](../etc/evidence/t26-s12-modern-delivery.json)。通过包括独立公共provider、原profile/frame/隐藏/布局、input/surface/host、native/target、共享hook/dispatch/repaint/global队列、真实产品direct入口与composed表面显示/移除、普通terminal七族。额外完整S11上版八文件作3次对照：两宽原生SetWindowsHookEx均同样126，旧CLI也同样1460。当前及上版计算器probe均core==frame，不符合原ApplicationFrameWindow/child CoreWindow关系；当前global夹具在发现frame前失败，未运行旧global夹具，不能冒称旧global实际通过。

六项未通过保留，不转成skip或全部通过：四项为两宽普通/composed原生产品hook安装126，另两项为真实计算器CLI/global原宿主前提未满足。直接DLL组合入口通过不等于原生hook成功；既有接入/宿主限制由S14接收，本S不扩名单/权限/能力。S13接收renderer字段frame_view和年代字体选择，原S11激活与Win2000 DWM债务仍开放。

终审显式补入11项未变的传递引用/菜单/identity/bitmap/font_pair/renderer依赖，均与固定S11 blob字节相等，复用既有审计并核实本轮实际消费者。合计67项既有输入（52初始+15闭包补录）及3项新输出；逐项归属/保留理由/调用/验证见台账。生产C/H+289/-272净+17，profile主文件249→96行；删除remove/surface及两个空apply包装、私有action类型和公共字段视图。未新建安装引擎或反向依赖。

测试结束已将原始日志/快照/工具链/JUnit/命令/失败及摘要提炼到跟踪证据；核验无活动自有进程和reparse后清理clean-source、上版对照副本、两份大依赖日志和发布备份330,261,871字节。仅暂留candidate及最小结果供提交/远端核对，完成即回收；稳定.ninja/.ccache保留。没有等待全T结束才清理，也没有逐文件完整发布。

另查出GOAL仍写旧“三组件四文件/只支持Win16”目标，已按用户确认的三层框架与五组件八文件契约修订，保留原资源/非侵入运行边界；这是目标文档纠正，不触发产品重新构建或新增能力。

## 有限收口与交接

S12 P1 `a683cc2e7648ec97ac06704b2be48db98183a078`已推送并实际fetch，38个变更blob及八文件与候选/assets/O:/winnt逐字节相等，见[远端证据](../etc/evidence/t26-s12-modern-remote.json)。按本S现有机制抽离范围收口，不称6项原生hook/实际宿主观察通过；相同上版失败和未执行的旧global路径明确留S14。最后candidate/结果5,928,662字节已归档清理，本S临时构建/包/探针/备份全部回收，缓存保留。

自动准入S13呈现/图形边界，初始171项冻结而非已审；T26开放，S14终审仍未完成。不为本次文档交接重建或重发八文件。
