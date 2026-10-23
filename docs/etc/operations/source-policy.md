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
