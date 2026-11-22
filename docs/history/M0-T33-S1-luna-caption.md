# M0 T33 S1 Luna 标题栏

用户报告边框颜色、三个标题按钮始终凹下、悬停时标题闪烁及无按下反馈。
发布版原生探针复现三个按钮状态均为 STATE_SYSTEM_PRESSED。
原版 Luna 标题位图含外沿，旧绘制仅覆盖内侧。普通位图标题使用完整画面、
单次提交，悬停和按下由样式实际跟踪状态提供。测试采用自有窗口和原生消息。

## 发布与验证

- 原版三个配色的 FrameCaption 位图各有激活和非激活帧。将顶部第 2 屏幕像素
  与 144 DPI 九切片映射的原始像素核对，蓝色、银灰、橄榄绿两状态均一致。
- 两处正式发布后，双位宽三配色共 12 次。每个自有窗分别检查最小化、最大化、
  关闭按钮的命中，以及普通、悬停、按下、取消、离开后的状态和实际像素。
  按住一个按钮时其他两个维持普通样式。下层宿主没有收到本层处理的悬停消息；
  位图标题离屏合成后一次提交。未做物理鼠标／显示器刷新率测量。
- 双宽干净构建、ABI、公共归档、原版资源和正式七件身份通过。源码快照
  SHA-256 为 24e6e036a4982ab98d7943ddfe5a8ca2aed2efd08470a525c4a1e02cae828f58。先向 assets、O:/winnt 发布，再验证。
- 正式包聚焦回归 13/13，包含 Win2000 绘制、普通启动和参数、
  经典真实示例以及共享窗口兼容路径。原版 Luna.msstyles 摘要保持。

## 远端复核与收口

首次发布因旧会话占用正式 ntwshk64.dll 回滚。核对会话拥有者的 PID、
绝对映像路径和启动时间，发出该产品已有的停止请求，文件占用随后解除。
复试发布两处通过。

从 origin/main 取回 P1 提交 997cdb7b2087eb3ee630014f31ef866ddbda04d4，复核 1,284 项构建输入及两处
七件文件的摘要。清理本次正式构建、测试目录、探针与发布备份，
共 483,110,138 字节。保留公共 Ninja 和 ccache 目录。
旧发布备份仍被系统映射，保留至宿主退出： O:\repos.hobby\win16style\build\t33-package\caption-live\before-3、O:\repos.hobby\win16style\build\t33-package\caption-live\before-4、O:\repos.hobby\win16style\build\t33-package\caption-live\before-5、O:\repos.hobby\win16style\build\t33-package\caption-live\result.json
