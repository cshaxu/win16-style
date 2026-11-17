# M0 T27 S1 控件parity与原版依据冻结

用户准入Win2000常用控件，要求源码/原资源/真实截图，不允许凭印象捏造；自行设定目标并持续执行。本S以完整来源/状态与实施接口设计为交付，不修改产品绘制或把关闭主题当完成。

基线efcf9109：T26三层重组和最后八文件保持；工作区开始时干净。常规沙箱CIM查询权限失败，Get-Process未发现编译进程；没有停止/重启既有构建。隔壁项目及Win2000参考仓库仅只读；以额外只读Git核验确认参考提交fd809a81098565b33f52d0f65925159de8f4c337，status为空。

## 工作顺序

1. 冻结Win3.x实际控件/状态/别名/运行时和demo入口，区分完整绘制、边框/字体及原生保留。
2. 对照用户源码ntuser及shell/comctl32，逐项记录文件SHA、真实绘制入口、资源生成/引用及默认颜色度量。
3. 搜索可核实Win2000原版截图，确认可见状态；截图不包含的按下/禁用等状态不靠猜测补齐。
4. 形成共同状态/消息/资源所有者及Win2000 renderer接入设计，冻结S2后续实际验证矩阵和通过判据。

当前已确认原Win2000按钮区分DrawPushButton与OEM check/radio状态图，不能套Win3.x扁平glyph。公开GUIdebook Win2000截图页及一张accessibility原图已找到，网页相对链接有404，须以实际可访问原图URL记录，不把失败链接视为图像已查看。尚未声称全部原版依据或parity矩阵完成。

登记build/t27-reference/s1-original-images-01：S1拥有，仅下载已查证Win2000截图原图供视觉/像素研究和来源摘要，不作为产品资源或通过URL限制展示媒介。每请求30秒有界；来源/观察/摘要归档后清理原图副本，Win2000原资源另按来源准入。

## 冻结结果与具体设计

冻结24个标准/公共控件核对组、8个VB/VCL/OCX/复合核对组，覆盖221个当前相关C/H输入及其精确摘要/已有别名；不是“32种控件都已画好”。对应32个Win2000原文件的提交/SHA与绘制入口索引、每组状态轴和跨项验证保存在[参考清单](../etc/evidence/t27-s1-controls-reference.json)。原Win3.x范围来自实际guard/classification/render/special-install及公共协议路径；将完整绘制、边框/字体或原生保留区分，不把demo上存在的控件算成已实现painter。后续每个实施S必须完成本组状态算法/原图审批，S1冻结不冒称全部像素已核准。

已实际下载并通过view_image查看GUIdebook的Win2000 Accessibility Options原图（367x443）；System.Drawing像素读取确认按钮面212,208,200，默认按钮外框黑、内高光白；有默认/普通/禁用按钮、未选复选框、分组、标签页。截图未显示的checked/radio/pressed等不由它外推。网页相对图链接返回404，已使用验证成功的根原图URL及SHA。可选PIL不可用，采用既有System.Drawing分析，未安装全局依赖/修改图片或把该截图变成产品资源。

重要差异：kernel/globals.c的gargbInitial是192灰的启动初值，而实际Win2000标准配色截图是212/208/200；不能把启动初值当成安装后系统配色。源码slot语义与真实图像相互核对，已有被验收的Win2000 frame同用212/208/200。保持应用字体所有权，不凭印象强制所有控件Tahoma或套Win3.x位图字形。

## 原资源与版本缺口

ntuser/inctlpan.c明确按状态表调用DrawFrameControl生成OEM check/radio/3state strip；rtl/draw.c含Marlett路径。当前包按文件名未找到Marlett/font文件，不能伪造所谓原版bitmap或默认宿主字体等价。后续S3需以原图/原资源/原API结果验证再决定使用策略；需要额外原文件时记录具体缺口，其他能力继续。包内尚未定位RichEdit实现，当前parity仅保留其原生客户区/已有边缘文字范围。SysLink不是此Win2000 comctl32已确认的原版控件，单列为现代扩展边界，不声称存在原版Win2000 SysLink样式。

## 实施接线

Win2000 common/control提供实际年代palette/edge/renderer及必要资源；产品ntw2ks只装配API/ABI和资格。新控件对象嵌入公共instance及确有需要的state/bindings，直接复用公共install/retire、basic/list/edit/combo/native/scroll/notify/snapshot/present，不复制Win3.x私有control_style或再造子类/资源/列表/传播生命周期。字体和图像按应用借用；通用primitive若确有两个当前消费者才归application，不建未来主题空框架。S2先实现完整普通按钮批次，随后按16S顺序推进；每S内增量，必要完整交付只在能力组合完成后一次进行。

本S无产品/测试/构建输入变化，现有assets/O:/winnt八文件逐项匹配T26最终摘要；未编译或虚构发布T27二进制。治理、提交推送及实际远端复核之后自动S2，不要求用户逐步确认。

S1原图研究已结束，SHA/尺寸/实际观察和缺口已归档，原图scratch核验绝对归属及无reparse后清理，不留下临时图片或产品资源；无编译/原生测试worker。

S1本地冻结与设计完成：24控件组/8运行时组、32原版参考文件和221当前输入；治理13负例及diff检查通过，产品输入和两处八文件均未变。原图scratch9284字节已清理。效率决定：不进一步无界扫源码/重做T26审计；状态算法只在对应实施S前核准，S2以有真实产品消费者的按钮完整能力批次实施，不另建空的基础设施S或逐文件正式交付。

S1 P1 `0742a00f1b25361676269479e9b21b7a24e6f890`已推送并实际fetch，7变更blob及两处原八文件摘要一致，产品/测试/工具/构建输入相对T26无变化。S1按来源与设计冻结范围收口，自动准入S2，T27保持开放。
