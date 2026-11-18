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
