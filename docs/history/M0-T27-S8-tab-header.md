# M0 T27 S8 Windows 2000 标签页与表头

按既定顺序推进。先核对固定 Windows 2000 comctl32 的 tab.c 和 header.c，保留原图可见状态和源码状态规则的区别。Windows 2000 的边缘、图文和焦点呈现应接入已有应用层快照与消息机制，原生控件继续负责排列、选择、列顺序、调整列宽及应用回调。

普通、带图标、选中、按下与禁用进入有限验收矩阵。应用自绘、回调内容与非标准变体按现有 Win3.x 证据和原版契约逐个确认，未覆盖项如实登记。发行用例包含实际两宽普通启动、借用 DC、移除，以及 S1 至 S7 的回归。

固定参考为 Windows 2000 源码提交 `fd809a81098565b33f52d0f65925159de8f4c337`。
`tab.c` 的 Tab_Paint、DrawBody、DoCorners 和 `header.c` 的 Header_DrawButtonEdges、
Header_DrawItem 提供普通与选中标签、按钮式与普通表头的规则。
`rtl/draw.c` 提供部分边线及右/下角颜色优先级。
实际查看了 [Display Properties](https://guidebookgallery.org/pics/gui/settings/appearance/win2000pro-1-2.png)
和 [Task Manager Processes](https://guidebookgallery.org/pics/gui/system/managers/tasks/win2000pro-1-2.png) 原版图。
Display Properties 的选中标签顶边和角使用 `#FFFFFF`、`#D4D0C8`、`#808080`、`#404040`，
Task Manager 的第一列表头 x29..135、y86..101 可核对同一标准配色。按下、输入与自绘规则依据源码。

Tab 复用公共原生消息和快照，Header 复用公共 custom-draw 交涉。应用自选绘制阶段、
排序图标、过滤框、复杂表头格式和不支持的标签方向沿用原生绘制。
冻结的本批测试以自有控件的绘制、消息及状态结果为准，实际鼠标和焦点验证另计。

首轮边线测试双宽 2/2。合并控件探针后 x86 3/5，两个 v6 失败均为原生创建自动加入
`TCS_HOTTRACK`，修正为识别该样式并按原线索处理热点文字。双宽随后 10/10。
一次增量命令误把仅有 x64 的 run16s 加进 x86 目标列表，Ninja 当即拒绝。
修正目标后，含原图边角和借用图标的双宽回归再通过 10/10。

交付前复核发现 Header 父窗口通知可直接使用 `style_control_notify_message`，
于是合并进原有状态和转发路径。主动结束首个正式快照，发布等待进程一并结束，
该快照未发布。双宽正式构建暂存登记 `build/t27-package/s8-tab-header-release-02`。完成后先于
`s8-tab-header-publish-01` 备份与发布，再于 `s8-tab-header-test-01` 验证同批摘要。
[来源摘要及范围](../etc/evidence/t27-s8-tab-header.json)。
