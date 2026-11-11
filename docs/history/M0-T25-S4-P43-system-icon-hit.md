# M0 T25 S4 P43：系统图标命中回归

P42 缩窄普通可缩放窗口边框并移动系统图标后，图标中心仍命中 `HTSYSMENU`，但靠近左上角的可见图标区域命中 `HTTOPLEFT`。本机 `WM_GETTITLEBARINFOEX` 的图标字段为空且标为不可见，因此原有命中逻辑不能覆盖该区域。现按系统菜单矩形及 P42 的偏移计算图标命中范围，并限制在标题栏高度内；未改变系统菜单本身或窗口命令。

私有桌面对照中，x64 的 P42 边缘命中 `HTTOPLEFT`，x86、x64 候选中心和边缘均命中 `HTSYSMENU`。尝试向窗口发送合成的系统图标点击时，P42 与未加载样式的原生窗口均未打开菜单，因此该实验不能证明真实鼠标点击或弹出行为；已有键盘系统菜单绘制回归通过，但不能代替物理点击。

纯 C17、MinGW GCC 16.1.0、Ninja 与 ccache 双位宽干净构建；源码快照清单 SHA-256 `e357fac9a59690e48fdd82fcb71de75ae5f6760599336edd37f5936c841086a2`。同批包 56 项重点回归通过、无跳过；包括图标命中、边框四角、标准菜单、对话框及启动传播。已发布到 assets 根目录和 `O:/winnt`，与测试包逐项摘要一致：

| 文件 | SHA-256 |
| --- | --- |
| demo.exe | 2ea015335fce99c66758a53e178fe6c4bf2a6c324009f89d4ac6df27fc5834a2 |
| ntw16s32.dll | 23cb7e0b899a923a9ad9d9e873c50b8554960753ff73f8d267d3d109dcdc7569 |
| ntw16s64.dll | 01446410fb5711fabe51c933e30e6d1482b1c110c7c1de2fba815cfb62814902 |
| ntw2ks32.dll | 9e5de79be8e4f002576e8c627642831b2eb8ba992481a59b44adb28ca7dd3d2c |
| ntw2ks64.dll | 7e36bc033c69b3980b17a779b7410b0914eb51dc5342a93511be4d23861dadd5 |
| ntwshk32.dll | 028c83bc13d80ab51afae91f03be6627a7ab0e65f439ed811c9beaa242d2d9d0 |
| ntwshk64.dll | 05d8ec793fd8878019fdee63a55a0735f18a07ddba487fe58b62707dfa0d379e |
| run16s.exe | 2081f1924f5a50a4bc45b8f6d9e48dabedf915199ce047e4c05981e930d0f558 |

未覆盖物理鼠标、全部 DPI、所有窗口风格和完整集成矩阵。S4 与 T25 保持开放。
