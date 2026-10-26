# M0 T17 S15：无窗口内建控件承载审计

S15 审计 Image、Shape、Line 与 Timer 的真实可视边界。它们不是遗漏的标准控件：
前三者由 VB 在所属 Form/PictureBox 的 DC 上绘制，图片、透明、点击、填充、线宽、
颜色和坐标都属于应用；Timer 则是非可见业务对象。MadShow 的真实 runtime60 timer
helper 是零尺寸，不能把它当作有待换肤的界面。

因此本 S 不增加类别名、子类或绘图覆盖。这样既保留用户图片和几何，也不干预 timer
回调；原有 PictureBox/PictureBoxDC 的有限同步文字 scope 不会被暗中扩大。固定样本、
矩阵行和 runtime60 观察的完整证据见
[S15 证据](../etc/evidence/vb-windowless-controls.json)。

本 S 仅记录正确的原生保留边界，没有产品源码/二进制变化或重新发布。推送复核后关闭，
自动顺序准入 S16 继续审核真实可见的 Data/OLE/MDI/UserControl 宿主边界；M0/T17保持开放。
