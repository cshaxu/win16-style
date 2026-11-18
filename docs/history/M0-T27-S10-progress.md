# M0 T27 S10 Windows 2000 进度条

按已有 Win3.x 实际覆盖和 Windows 2000 原版规则实施进度条。分段、平滑、横纵方向、
零值及短控件、范围/步长与应用提供的颜色进入验收。Marquee、平滑回退动画和现代状态色
先按原版与当前路径区分，保留无对应年代行为的原生模式并记录范围。

复用 application/control 的 progress_snapshot、native 消息、图形呈现与附着/退役。
进度状态、范围和返回值归原生控件。原版 ProPaint 使用客户区内缩 1 像素、分段宽为
横截面的三分之二、2 像素间隔，进度换算用 MulDiv，分段数向上取整。末端保留额外
1 像素。Progress_OnCreate 指定单像素静态凹边，默认底色由原生擦除规则确定。

已查看 [University of Kentucky ET127 Lab 原版 Backup Progress](https://www.uky.edu/~akali2/ET127/Lab.pdf#page=127)。
PDF SHA-256 为 7597aa788d77b1f5cbf607aaa53bccd5ceed4deb687f610637f4c34505cc7fd7。
其中 Slide 15 是浅色凹槽与深蓝分段的原版视觉对照。图片含缩放，精确几何以原版源文件为准。

双位宽 v5/v6 自有控件探针分别验证画面、原生消息数值、借用资源、移除和普通启动。
同一批八件完成后先发布再运行实际包，归档及取回核对后进入 S11。

## 固定原版文件

原始提交 fd809a81098565b33f52d0f65925159de8f4c337，文件 private/shell/comctl32/progress.c，Git blob 164cf1bbdec105e305f9f8bbc578cec2b463134f，工作副本 SHA-256 168e47045d001f530df1a7837609c0c0f8182d173bb53aa7a18dd9479c6e51ab。

`InitProgressClass` 的默认底色为 COLOR_BTNFACE。当前 Win11 原生同类测试显示，
v5 的 PBM_GETSTATE 和两项颜色 getter 返回 0，v6 分别返回 PBST_NORMAL
和 CLR_DEFAULT。v5 默认有 WS_EX_STATICEDGE。v6 的主题抑制后静态边缘位
变化先于客户区几何刷新。S10 先复用 Win3.x 既有的 v5 独立边缘范围及 v6
进度快照；颜色无法公开回读的 v5 客户区保留原生，以免还原时覆盖应用颜色。

原版 PDF、渲染页和原生只读探针已清理，共 12823368 字节。
