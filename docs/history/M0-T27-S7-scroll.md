# M0 T27 S7 Windows 2000 滚动条与尺寸角

按既定顺序推进。原 Win2000 `sbctl.c` 是滚动条几何和跟踪的依据，`rtl/draw.c` 是按钮和尺寸角绘制的依据。初读确认短轨箭头受半长度约束，比例滑块至少占半个标准轴向度量，滑块使用无 BF_SOFT 的凸边。先固定原对象摘要和真实图证，再落实普通和边界状态。

单独控件与内嵌轴复用 application/control 的 scroll controller、message、surface 和 native state。S6 已有源于原版的共享箭头操作。验收覆盖横纵、页与拖动、禁用和短轨、保留原生滚动通知、尺寸角、DC、恢复及两宽实际启动。只在原任务范围内扩充。
