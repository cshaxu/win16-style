# M0 T27 S11 Windows 2000 滑块与微调按钮

核对固定原版 trackbar.c 的 GetChannelRect、DrawChannel、DrawThumb 与刻度，
以及 updown.c 的原生 UDS 分割、两向按钮、共享凹边、禁用、热态和按下标志。
普通控件的原生位置、应用回调、通知、边界和编辑伙伴继续由 comctl32 持有。
先证实共用 custom-draw、上一个阶段已验证的 USER 浅/深色边线与滚动箭头、
以及既有 UpDown 原生绘制坐标路径是否足用，再实施缺失外观。

滑块验收横/纵、三角/方形、两侧和单侧刻度、范围、禁用与按下。
微调验收横/纵、奇数尺寸、编辑伙伴、应用取消通知、禁用和撤回。
双宽、自有控件、两种清单和普通启动。不能从公开绘制状态可靠得到的样式须登记。
完成合批八件、先发布后测、远端取回及清理后准入 S12。

实际查看三张原版图。颜色其余校验以固定源码及 Windows Standard 为准。

| 原图 | 原样 SHA-256 | 所见 |
| --- | --- | --- |
| [Keyboard Properties](https://guidebookgallery.org/pics/gui/settings/keyboard/win2000pro-1-1.png) | 496151998575a57a2395bf52102cc5b0d14c35fe61e0787e26c95f528aa3f56b | 三个水平滑块及下尖滑块 |
| [Volume Control](https://guidebookgallery.org/pics/gui/applications/multimedia/volume/win2000pro-2-1.png) | be7d1a89348a34e42a7801741ba92a903a93116a5abc960b1cd3f28650b49b6d | 水平尖滑块，纵向方形滑块，两侧刻度 |
| [Display Properties Screen Saver](https://guidebookgallery.org/pics/gui/settings/screensaver/win2000pro.png) | 7cf067c10459c47059a7d087a2183c8803a62577c829a5d7ad6bdb4606317023 | 实际编辑伙伴旁的禁用微调按钮 |

原版提交 fd809a81098565b33f52d0f65925159de8f4c337。

| 文件 | Git blob | 原工作副本 SHA-256 |
| --- | --- | --- |
| private/shell/comctl32/trackbar.c | be140a25da3aae273bed095d425f1fe5f14173c6 | 0ba4b54faa734d915d8c6e51f3494ac484641205604f9eeabb79a0c0c8119981 |
| private/shell/comctl32/updown.c | 7f37ffb493acaa7547ac90362fa1e509015da8fa | bc20d94285afe85bbeeab0a75b66397c136fdd800744cdc062c4a68ad54bdf98 |

本机 v5/v6 原生绘制均给 TrackBar 发送 TBCD_CHANNEL、TBCD_THUMB 和 TBCD_TICS，
也保留了各版本真实矩形。相同测试的 UpDown 无 NM_CUSTOMDRAW。复用公共通知可直接
实现滑块。微调按钮先核对已有可提供原生绘制范围和状态的机制，再选没有第二套输入的方案。

已对 Keyboard Properties 的第一个滑块逐像素核对。y115–130 为矩形，
y131–135 逐行收尖，末两行分别为亮/阴影/深阴影与单个深阴影点。
绘制复用原生矩形、原有通知和 Win2000 凹槽/软凸边。应用自绘结果优先。
保留原生刻度和键盘/鼠标通知，选区通道、镜像及特殊几何按原生处理。

两种清单原生微调探针均呈现颜色槽一致的经典 DrawFrameControl 像素，
垂直 17×39 有 17 个未绘制缝像素，水平 39×19 有 19 个。只在原生绘制
写入的像素上映射 USER 标准配色，原生箭头、按下、禁用、编辑伙伴和缝保持。
复用现有 32 位位图缓冲与绘制消息，不引入新挂钩。热点颜色仍沿用本机原生。

开发中先修正原生缓存对应用自绘探针的影响，再处理清单下的 DPI 尺寸。
用例只发送自有窗口消息，物理键鼠和跨屏切换另计。

合并增量回归双宽 22/22，八个标准 `run16s --2k` 启动用例 8/8。保留所有失败复盘。
[开发实测证据](../etc/evidence/t27-s11-track-spin.json)。开发、原图和见证临时目录已清理。
正式构建 s11-track-spin-release-01、发布 s11-track-spin-publish-01 和实际验证 s11-track-spin-test-01 已登记。
