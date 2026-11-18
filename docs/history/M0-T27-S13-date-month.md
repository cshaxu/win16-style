# M0 T27 S13 Windows 2000 日期选择器与月历

原版 MonthCal 和 DateTimePicker 均由同一份 monthcal.c 注册实现，包括
MonthCal 的标题与选中日期、前后翻月按钮、DateTimePicker 的下拉或
UpDown 分支。保留原生年月日排列、locale、值、输入、通知及应用格式。
普通单月及下拉日期选择器先实现，时间微调、复选框、应用解析以及
多月模式按观测结果划定边界。以原来 Win3.x 实际支持的日期按钮和日历
导航、标题、选中角色作为对照。复用公共 native button/read、月历查询、
生命周期、位图临时面和 Win2000 已有滚动箭头。

原版提交 fd809a81098565b33f52d0f65925159de8f4c337。两份源作为只读参考。

| 文件 | Git blob | 原工作副本 SHA-256 |
| --- | --- | --- |
| private/shell/comctl32/monthcal.c | fbbd7147bf5d451bb6759f255aebf1ceee47cef2 | 356661821146427f36c7fa7b49244c099f6200671c0134bcae965fb47e04c127 |
| private/shell/comctl32/monthcal.h | 0e0885fe71f9dbf905b0edeee1da8c41ade13a16 | 6002f9abc39cd76592abcb37d864f24e4887424e6b441a67760a8d0ad4034f8d |

以 [Windows 2000 Professional 实际日期和时间属性](https://guidebookgallery.org/pics/gui/settings/timedate/win2000pro-1-1.png)
作时代外观旁证。独立控件布局以固定源码的 MCPaint、MCPaintArrowBtn、
DPDrawDropdownButton 和实测对应几何为准。原版前后翻月和下拉箭头都是
原生 DFC_SCROLL 图形，按下组合 DFCS_PUSHED 与 DFCS_FLAT。月历标题默认
来自 COLOR_ACTIVECAPTION，选中日期也借用该标题底色。

本机 x64 原生 C 对照已确认，v5 的 DTM_GETDATETIMEPICKERINFO 与
MCM_GETCALENDARGRIDINFO 不返回数据，v6 两接口可用，关闭主题会改变
v6 的几何和日历配色能否绘制。宽190、高32的 v6 DTP 下拉矩形从
(156,0,190,32) 变为 (171,2,188,30)。原生 v5 月历绘制包含自定义
标题色 4105 像素，v6 主题态为 0，关闭主题后为 5060。v6 默认月历量
得 225×160，在原尺寸关闭主题时外露的 header 坐标可能为负，须按实际
稳定、剪裁后的几何判定，不能用主题态快照冒充。v5 日期选择器缺公开
按钮矩形，若无独立证据就保留原生绘制。

完成前以双位宽、两种清单、DPI aware、普通启动和借用 DC 抽样，并
核对日期、通知、应用颜色、无效输入和移除样式。记录不能验证的组合。
同一批完成八件先发布后测、提交推送与实际远端核验。

原图已直接查阅并取样。404×341 PNG 的 SHA-256 为
770f30468a405c051cc93bea6f7ca8205b249d1177258331b69d101040692ede。
(176,171) 日期选择背景 RGB(10,36,106)，(76,149) 月历网格底色
RGB(255,255,255)，(90,114) 星期字头底色 RGB(128,128,128)，
(120,62) 对话框标准底色 RGB(212,208,200)。这是系统设置页布局，
其单独月/年输入不能用来推导通用 MonthCal 上方标题和导航矩形。
本地对照图测量完成后已删除。

第二个原生 v5 对照给出了实绘按钮证据。190×32 的普通日期选择器客户
区为 186×28，右侧按钮在客户 x169 到185，等于公开滚动条度量 17 像素，
按钮外有两像素非客户边缘。带 checkbox 的下拉布局同样；时间微调
控件不绘制下拉箭头，而由真实 msctls_updown32 子窗口负责。若需适配
v5，可沿用原版 DPSize 的边缘关系并核对原生笔画，只替换已匹配颜色。
这是默认 x64 清单和度量样本，不能据此推断所有 DPI 和极窄布局。

标准 v6 日期按钮已有公共 native_overlay 完成方式。它保留原生 WM_PAINT，
只在绘制完成后按原有脏区或打印 DC 剪裁覆绘按钮。优先扩展这个公共读取
契约，以免重复画日期正文、焦点/段选择和客户背景。共用资源由现有
style_dispatch_message 所有并在失败或回退时归还。月历的日期和网格仍
由系统绘制，只处理经原始色值及原生公开颜色共同确认的标准色角色。
