# MSStyles v3／v4 主题加载与应用（Luna／Aero）

本任务已准入 M0 T36，活动阶段见 [CURRENT](../states/CURRENT.md)。
目标是让同一套双位宽运行时直接读取并应用 XP Luna（v3）、Vista／Win7 Aero（v4），
并用 Win8／8.1、Win10、Win11 原版文件验证 v4 兼容范围。

## 使用契约

```text
run16s.exe <theme-name> <exe-name> <exe-args...>
```

主题名是 `run16s.exe` 同级的直接子目录名。每个目录保存原版 `.theme` 和该描述引用的
`.msstyles`，例如 `Luna/Luna.theme` 和 `Luna/Luna.msstyles`。
原版描述使用系统主题路径时，应解析到主题目录内的对应文件。以启动器位置为基准，
从不同工作目录启动都能找到同一组主题，后续参数照常传给目标程序。两类原版样本以
文件字节保持一致为验收条件，复用 `ntwshk32.dll` 和 `ntwshk64.dll`。
主题由项目运行时按目录加载，无需安装为系统主题或修改系统配置。

## 共享边界

- 共用 `.theme` 描述、PE 资源访问和按类／部件／状态查询的只读属性接口。v3 读取
  XP 的内嵌 INI、配色、尺寸和位图引用。v4 复用 `v4_tables` 与 `v4_property`，
  补齐基类继承、全局属性回退、全彩透明图像、图集和缩放边距。
- 按源主题解释部件和资源，按运行宿主适配窗口布局、命中和绘制提交。普通控件复用
  共同部件映射，Win32、UWP、WinUI 外框共用布局与绘制。DWM、Shell 专属类及动画
  只按实际样本和功能需求补齐。
- `CLASSIC_STYLE` 等项目扩展按需消费。原版文件缺少这些扩展时仍可加载标准外观，
  对话框沿用原生路径。持续回归 Win16 与 Win2K 的既有扩展行为。
- Aero 静态图像及透明素材、实时模糊透景与染色分别验收。合成能力按宿主选择，
  记录实际结果及回退，验证停用与窗口恢复。

## S 实施顺序

| 阶段 | 交付与退出证据 |
| --- | --- |
| 1. 确认现有 Luna v3 基线 | 核对原版 XP SP3 样本和已发布的双宽路径，记录三配色标题的用户验收、已验证部件及其边界。 |
| 2. Aero 样本与 v4 加载 | 固定 Vista 和 Win7 Aero 原版样本、来源及摘要。复用现有 v3 路径，补齐 v4 属性查询和资源读取，验证变体、类与状态映射、属性回退、图像读回及错误诊断。 |
| 3. Luna 与 Aero 静态应用 | 使用原版主题目录按同一命令启动 x86／x64 自有程序，验证外框、标题按钮、代表性 Win32 控件的几何、像素、交互和撤除。验证已支持现代宿主的共同外框，核对参数传递、不同工作目录和 Win16／Win2K 回归。 |
| 4. Aero 玻璃合成 | 用实际背景变化、窗口移动及激活状态核验模糊、透景与染色，测试切换和恢复。按宿主记录实际合成、静态回退与未覆盖项。 |
| 5. 后续 v4 兼容与终审 | 用 Win8／8.1、Win10、Win11 原版样本验证共用 v4 解析和适用的外观应用，报告各版部件及资源覆盖。一次终审核对 v3、v4、原有主题及适用实包交付。 |

格式依据见 [Wine v3 读取实现](https://github.com/wine-mirror/wine/blob/master/dlls/uxtheme/msstyles.c)、
[v4 格式记录](https://github.com/nptr/msstyleEditor/wiki/.msstyles-Format)、
[微软 Aero 部件表](https://learn.microsoft.com/en-us/windows/win32/controls/aero-style-classes-parts-and-states)
和 [微软 DWM 模糊说明](https://learn.microsoft.com/en-us/windows/win32/dwm/blur-ovw)。
