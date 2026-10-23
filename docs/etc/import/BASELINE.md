# T1 固定来源与去留基线

范围：固定 ntvdm64 `5b0b65f66c281050aa4ce1454c6cd31441a6d484`，
原 105 个候选文件，加首层本地依赖边界发现 19 个文件，共 124 个条目。
[source-manifest.json](source-manifest.json) 对每项记录 source、Git mode/blob/size、
原字节 SHA-256、去留、目的路径、声明分类、原因和接收任务。
所有字节从固定 Git 对象读取，未读取脏工作区作为内容来源，也未把原源码写入本仓库。

## 有限覆盖与依赖边界

覆盖单位是原 105 条候选及其 C/C++/H/RC 字面 include；共 294 条 include 边。
扫描保守保留条件编译分支，不声称能识别宏生成 include。实际两架构编译的依赖
图由后续产品任务复核。每条边有本地已审候选、SDK/工具链或排除边界处置，
没有未解析的字面引号 include。

19 个新增条目是 RPC、broker、frontend/console 协议、原形 Native classifier
及其声明等排除边界。到达排除边界即停止递归：不会引入其实现，也不声称追踪
整个 NTVDM 服务闭包。保留 renderer/Detours 的本地 include 全在审查集合内；
Hook 的外部服务 include 随其调用逻辑删除，launcher 从原生目标重新实现。
所有排除项目的路径为空，没有待定的隐含外部构建输入。

## 去留决策

- renderer：保留唯一实现家族；修改 DllMain、公共门面、native binding 及
  消费被替换资源的绘制单元。绘制算法不另建副本。
- Hook：保留思路并逐文件改造至 ntw16s-dll/injection；不沿用服务身份、
  launcher redirect 模式、RPC 分配接口、frontend/execution 句柄或旧导出名。
- run16/common 分类器：仅参考/排除；不导入 OpenNT 的 nt.h、base_classifier.h、
  SEC_IMAGE/SectionImageInformation 路径。以 Windows SDK 的已创建进程位宽查询
  和有限 PE 校验实现本地注入选择；原生程序不进入 VDM 路由。
- demo：改造项目示例为纯原生 UI，删除所有样式 API 和主动自绘资源依赖。
- tests：保留契约、按本地包改造，图形原字节相等的 oracle 不作为新素材验收。
- 原 CMake/历史任务编号脚本：仅参考，不成为本地构建图。

## 声明及五资源处置

Detours 切片带 MIT LICENSE 和源文件版权标记，接收时保留，且按源实际 blob
记录本地 helper 差异，不能当作未改 upstream。项目自编 renderer/适配代码
依据源组件 README 的自编实现说明接收；若实际导入发现外部声明则保留并重新
记录该条目的处置，不能由根 MIT 覆盖外部声明。

固定 resources.md 明确记录五头来自原 Windows 3.1 VGA.DRV，公开分发依据未建立。
处理统一选定为 **不导入原字节，后续样式核心任务自编几何绘图/栅格替代**：

| 原头 | 消费面 | 接收与验收 |
| --- | --- | --- |
| frame_buttons.h | 标题系统框、最小/最大/还原按钮 | 核心任务自编线条/几何，检查 normal/pressed、命中区及 DPI |
| control_glyphs.h | checkbox/radio/mixed | 自编状态图形，保持原生 checked/disabled/notification |
| combo_glyph.h | combo 下拉标记 | 自编箭头和边线，保留原生展开/选择 |
| menu_glyphs.h | 菜单 check/submenu 标记 | 自编图形，保留原生状态/测量/交互 |
| scroll_glyphs.h | 各方向及 disabled/pressed 箭头 | 自编几何，保留滚动范围/通知及已支持交互 |

替代保持 Win16 外观目标，不宣称原 VGA 每个像素相等。五头的原 blob 仅记录在
manifest，不保存原图字节。后续任务必须记录新素材来源与视觉差异、实际验收。
资源选择已经确定，不以研究未定为由留下默认复制路线。

当前 font.c 已从宿主 System 栅格生成进程私有 FNT，不包含旧 VGASYS.FON；
不再导入历史 system_font.h。保持“运行时生成、不保存或发布宿主字形”的边界。
Unicode fallback、CP1252 之外及无该宿主字体的失败处理由核心任务明确复验。

## S1 证明及局限

124 个文件逐项从固定对象读取并计算 SHA-256，与 tree 的 blob/size 对照；
105 个原始 inventory 条目纳入集合，19 个新增边界均有排除原因。
源码/资源复制数为 0。该记录证明去留和静态依赖设计，不证明产品能构建或注入。
ABI/引导/跨位宽与工具链证明在本 T 的 S2 完成；本地 manifest 校验工具在 S3 完成。
