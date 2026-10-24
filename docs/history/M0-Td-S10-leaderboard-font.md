# M0 Td S10：扫雷英雄榜中文字体偏小调查

用户确认上一S问题解决并批准收口；T12已经关闭。本次请求仅调查，准入为Td S10，
不分配产品版本、不重开T8、不改源码和四产物。基线ea8876f，准入时工作树干净。

## 已定位的断点

实际发布DLL在controls.c WM_SETFONT中，将应用HFONT交给font.c system_acquire。
该函数接受实际字符集134的SimSun字体，却生成ANSI_CHARSET的私有System位图字体；
仅遍历CP1252字节32..255，字重取宿主System的700。原中文字体400字重和中文
字形覆盖都丢失。源字体字符单元高度和CP1252 advance虽保留，中文实际绘制不再
同源，出现回退尺寸变化。font_layout.c只检查ASCII advance和GQag012像素，
不能检出这个缺陷。不是窗口或控件矩形缩小，也没有证据支持标题DPI错误。

## 原生证据

读取D:/programs/misc/winmine.exe的原始对话框700，用纯C CreateDialogIndirectParam
重建同一资源，加载本项目实际发布DLL，依次裸绘制、显式安装、移除。资源的六个
空分数/名字框填入代表值“999 秒”“匿名”，仅为绘制测量，不执行游戏逻辑。
目标是PE32/i386，SHA504177279992377c3c2c435c9340b9529e8060d1f09966defb5bf425aacfae20。
O:/winnt/winmine.exe另为NE，没有为此启动后端或接触用户服务。截图未提供实际路径，
本次证明该匹配英雄榜资源的断点，不宣称已识别用户截图进程。

| 文本 | 144 DPI裸/样式宽度 | 裸/样式实际墨迹高度 | 解释 |
| --- | --- | --- | --- |
| 确定 | 36 / 32 | 17 / 13 | 中文明显缩小 |
| 秒 | 18 / 16 | 17 / 13 | 非CP1252字符回退变化 |
| 999 | 27 / 27 | 12 / 12 | 数字没有整体缩小，但System字形像素分布不同 |

两种字体tmHeight都为18、ascent都为15，不能以行高未变宣称字形大小保真。
两宽均复现；实际显示器144 DPI，另用DPI-unaware的96逻辑DPI复验：
“确定”24->20宽、12->8墨迹高度；不是物理96DPI显示器覆盖。
四个最终用例全部exit0，每例11控件，矩形均保持；移除控件适配后原HFONT的
名称、字重、字符集、度量、文字宽度和逐项墨迹边界/像素数完全恢复。标题适配
保留至控件移除之后才清理；未改标题代码。原生私有桌面PrintWindow成对截图
保留于聊天visualizations，属于资源重建对照，不能冒称游戏运行抓图或物理输入。

测量：MM_TEXT内存DIB，DrawTextW后GdiFlush，扫描非白像素边界，独立记录
GetTextMetrics/GetObject/GetTextFace/GetTextExtentPoint32。对应位宽实际assets DLL
加载，随后释放自有HWND/GDI/DLL；worker90秒截止，不操作已有用户窗口。
精选逐项数据、目标/探针/日志/截图SHA及四文件摘要见[证据](../etc/evidence/leaderboard-font.json)。

早期探索枚举其他对话框时，用普通控件安装器处理EDIT返回50，整轮exit1；
该轮不计通过。最终仅英雄榜700，无EDIT，4例exit0。最初探针缺_WIN32_WINNT
宏导致编译失败，补齐0x0A00后使用C17 GCC双宽编译成功。无全局设置修改。

## 最小后续方案与边界

优先在字体变体入口拒绝不适用的原字体字符集，针对本例GB2312_CHARSET直接
保留原应用HFONT，包括同一中文对话框的数字；沿用现有借用/恢复生命周期。
不要把整个对话框乘一个字号系数，也不要修改菜单或已确认标题。加入真实中文
字体和文字的控件集成回归，测墨迹及中文advance、WM_SETFONT更新与卸载，
用当前DLL作为应检出的负例。此方案不增加资源或ABI，产品修复仍须单独准入、
验证和四文件发布。本调查未原型验证该分支，不宣称修复已完成。

更广范围的ANSI字体+混合中文文本需要按字符覆盖设计；仅按charset保护不能承诺
所有Unicode文本。没有确定GDI逐字回退到的具体字体家族，不以GetTextFace返回的
基字体冒充回退字体证据；也未拆分FNT固定8pt元数据对回退选择的独立影响。
[微软字体链接/回退说明](https://learn.microsoft.com/en-us/globalization/fonts-layout/fonts)
与[TEXTMETRIC说明](https://learn.microsoft.com/windows/win32/api/wingdi/ns-wingdi-textmetrica)
用于解释缺字与度量边界，实际结论以上述本机对照为依据。

## 交付范围

仅文档/调查证据；src/tests/tools/CMakeLists产品输入零改动，ABI、原图形资源及
已发布0.12.0.0保持。assets与system32八摘要一致，无binary change，不虚构编译、
包回归或新发布。临时纯C探针独立编译不构成新产品。未运行真实游戏交互、NE链、
完整经典回归或资源压力测试，这些不是本次只读诊断的通过项。
治理SelfTest、差异审查、提交推送和实际远端复核后收口调查S；M0开放。
