# M0 T25 S4 P63：双窗口实际激活颜色

使用 P62 发布版 `O:/winnt/run16s.exe --2k` 和增量编译的自有窗口探针，在可见私有桌面分别运行 x86 和 x64。两个同线程窗口通过 `SetActiveWindow` 依次激活；两种位宽均观察到活动窗口蓝色标题和失活窗口灰色标题，输出 `first=1 second=1 colors=1`，像素采样为 `93221c,898989,898989,93221c`（COLORREF 数值）。这补上 [P37](M0-T25-S4-P37-activation-environment.md) 因无前台桌面而跳过的同线程激活验证；它不是鼠标点击、跨线程或跨进程焦点测试。

本次无产品二进制变更，assets 和 `O:/winnt` 八文件摘要与 [P62](M0-T25-S4-P62-caption-separator.md) 一致；仅使用保留的 `build/.ninja` 增量构建目录，没有产生临时构建副本。S4、T25 保持开放。
