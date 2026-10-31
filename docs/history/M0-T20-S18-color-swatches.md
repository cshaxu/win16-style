# M0 T20 S18：颜色对话框平面色块

## 原版依据与实际断点

用户截图的基本色和自定义色块仍有现代凹边。1992 年微软 [Windows 3.1 SDK
程序员参考](https://bitsavers.trailing-edge.com/pdf/microsoft/windows_3.1/Windows_3.1_Programmers_Reference_Volume_1_Overview_1992.pdf)
第 132 页颜色框插图显示平面色块及细黑框。本机原始 `O:/W31/SYSTEM/COMMDLG.DLL`
SHA-256 `853574d46a8e0103028354883cc597e762bafd034b60c15aedc8cdc097629ec8`；
对该 NE 文件的导入与同目录 USER/GDI 导出做只读对照，可见 USER 的 FillRect、
FrameRect、DrawFocusRect 和 GDI 的 Rectangle 等；这只是整个模块的导入，不能
声称已定位各函数内的具体调用。`USER.EXE` SHA-256
`98036967f8396b387da431699a9d3be95eb56fcd0118f529e1fc2f25105efac6`；
`GDI.EXE` 为 `6d28146961ab4733e167b1657289206ce16a856e431f843e4ee3d504836dfb6f`。
未复制原系统二进制或其他仓库代码。OpenNT 的 WOW16 COMMDLG 是后来的 thunk，
不能把它当作 Win3.1 原始绘制实现。

自有原生探针确认当前系统 COMDLG32 在父对话框 DC 中对 48 个基本色和 16 个
自定义色调用 `DrawEdge(EDGE_SUNKEN, BF_RECT|BF_ADJUST)`，双宽各观察 64 次；
调用矩形位于对应的 720/721 网格中。原版发布包在 96 DPI 下，第一色块角点为
`a0a0a0`、内缘为 `696969`，而候选分别为黑色与原本的色块颜色 `8080ff`；
自定义色块亦得到黑框及调用者提供的 `RGB(12,34,56)`。

## 最小改动与验证

仅在当前线程的受控 ChooseColor 作用域、已确认系统对话框及其两块网格中，且
调用方确为 comdlg32、DC 属于该对话框、参数为已观察到的下凹边框时，将系统
边缘覆盖为一像素黑框并调整填色范围；系统继续绘制颜色、选择、焦点与通知。
其他绘制走原路径；嵌套和异常清理恢复作用域与原错误值。未替换任何图形资源。

增量双宽 A/W 共 12 组确认/取消/展开通过，颜色与自定义色正确、非相关内存 DC
DrawEdge 保持系统矩形收缩。旧发布包对新增像素断言失败，证明能检出原问题。
首个键盘探针只发右箭头而未发空格，四组返回原色；这是测试误解系统行为，
添加空格确认后双宽 A/W 四组实际返回相邻颜色并通过。此为消息驱动，非物理
键鼠；实际 DPI 为 96，其他 DPI 与自定义模板尚未证明。

`s18-color-final` 双位宽干净构建、来源/资源字节及 ABI 检查通过；同批四文件
`s18-color-focus` 30 项聚焦回归通过、零跳过、耗时 49.65 秒，包含双宽 A/W
确认、取消、展开、键盘、原生调用追踪以及字体、滚动和必选包检查。已发布到
assets 根和 O:/winnt，两处与测试包摘要逐项一致；从 O:/winnt 直接启动的
x64 Unicode 原生探针验证颜色和细框并捕获截图。`window_titles.c` 生产代码
变动 +27/-7；测试和 CMake 变动见提交 diff。来源声明与原五图形资源未修改。

SHA-256：run16s `79d4eb4cc4d3039e5e000d078310b2c6f99069c76570a92175c57a2a111c4a4e`；
ntw16s32 `16ff15b91f879b3f7fa3f292dbc01964337d1b7084eff61d633262b7db09a623`；
ntw16s64 `30eac28f3c9bce4472c18d9a014906319b64b987841850dfe60a84596fd7ddd1`；
demo `ef030d5a5acc332ebea7fac20b50a5c9d2cad7a9aceecac791daa608633082cc`。
实际探针使用 96 DPI 和合成消息；其他 DPI、RTL、自定义模板与长期资源压力
测试未覆盖。无法识别为目标色块的绘制继续使用系统原路径。

S18 P1 `ccdedbb` 已推送，实际 GitHub 源码、测试、记录及四个资产对象，
以及 assets 与 O:/winnt 四文件摘要均复核一致；S18 收口。
