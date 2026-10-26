# Source Admission Policy

导入 ntvdm64 时从固定 Git commit 读取，不从有未提交改动的工作区复制。
导入前记录 source path、commit/blob、destination、保留/修改/排除理由、声明、
依赖、验证接收者；导入后用本地 manifest 对照。不要求用户保留来源仓库来运行
本项目。原始行为与独立化改动分别审查。

用户明确要求“不得替换图形资源，必须使用相同的”。五个图形头必须逐字节保留
固定来源提交的frame_buttons.h、control_glyphs.h、combo_glyph.h、menu_glyphs.h、
scroll_glyphs.h；禁止自绘近似、重采样后回写、生成器覆盖或换用其他素材。
原resources.md逐字节保留，清单标记retain；SHA和Git blob核对必须通过。
构建与包检查仅使用本仓库的已固定资源，兄弟仓库不是构建或验收依赖。

原资源的Microsoft来源和原有说明保持可见，两个DLL内嵌完整resources.md。
根MIT不覆盖第三方资源，用户要求恢复资源也不被写成新的许可或权利结论。
历史自编替代方案已被用户否决，历史交付记录保留但不再作为当前外观完成依据。
其他源码保留原有声明；当前MinHook/HDE遵循独立固定清单及完整BSD声明。

以tools/verify_resource_identity.py校验五头及原说明的固定摘要；来源清单和
回执拒绝replace或修改后假称retain。包校验同时核对两DLL内18个原始栅格表，
不能只看文件名或生成器自己比对自己。窗口布局、字体及DPI行为另按实际绘制
验收，不把资源相同扩张为所有运行环境像素完全相同。

T17 S5沿用S4同一原作者样例，只扩展准确runtime50/60 Combo/List/Scroll角色与
本层字体所有权；没有复制外部实现或新增图形资源。EDIT身份通过原生公开类及
COMBOBOXINFO确认，包括USER32的系统分派过程与COMCTL32 v6实际注册类，
不读取或解释私有过程令牌。32字体租约上界、源字体归还及回调存活均属本DLL。

T17 S7用固定Code Architects镜像提交的原作者Photo Album作可选本地研究样本，
实际PE依赖MSVBVM50而非目录标签VB6。原作者Salvatore Totaro/PaperQuack的
About.frm声明freeware/as-is；不据此推断整个镜像的源码许可。样本实现、EXE和
图像不进入产品或必需测试，不安装/注册外部组件。实际图像仍由原运行库绘制；
独立C模型只生成测试辨识色块，不是替代产品资源。固定身份与边界见
[文件控件证据](../evidence/vb-file-controls.json)。

S8原作者Benjamin Dowse的Compress-Z-It示例来自vbAccelerator固定归档提交，
readme1st.txt明确免费修改/使用；样本及依赖只在本地可选研究中运行。其Form_Load
仅构造压缩类，类初始化仅赋默认值；不调用压缩按钮/文件写入方法。测试侧本地
清单用公开类型库GUID作免注册COM绑定，不运行DllRegisterServer/安装zlib/改系统
注册表。原EXE/DLL/源码/声明保持，未进入产品或必需测试；见
[真实VB6文件组](../evidence/vb6-file-controls.json)。
