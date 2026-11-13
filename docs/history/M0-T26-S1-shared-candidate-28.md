# T26 S1 第二十八批共享候选

双位宽 MinGW、Ninja、ccache 干净构建通过，先发布至 assets 和 O:\winnt，再测试。

原生列表顶行、选择和焦点状态的采集与比较移入公共组件；Win16 继续决定消息分派及重绘。

双位宽公共原生测试覆盖单选、滚动与多选焦点项，正式包双位宽自建列表附着及鼠标消息测试通过。其他风格及物理鼠标输入未验证。

导航焦点、Win2000 DWM 恢复等先前失败及 UWP/WinUI、真实全局等未覆盖项仍开放；S1 和 T26 未收口。用户随后要求按文件顺序审计，见顺序台账。

| 文件 | SHA-256 |
| --- | --- |
| run16s.exe | e1831502d9b9d27430aa2fe3b866dddc6dd2843c5c05760f1c252665313f64ea |
| ntw16s32.dll | eb77057a637a53afd0ab0cf086de6f5ba1e7794e6c8bac15ae2db6919a494972 |
| ntw16s64.dll | 691b7445582f533f36389b2d0820b0315267881df1b0246a09c7250e5399dad5 |
| ntw2ks32.dll | 3b6a4293329776d7f280b4c70e1869844adc0c9c061e1e2e38f409893930f390 |
| ntw2ks64.dll | 24b97ef13a2f1ae902e3b39ffd5e57689246e9d37e9989b3c729266d7cfffc61 |
| ntwshk32.dll | ec3c6d57c0f9a55a5a196bff010b5f15cf24bb89fecdc9f10e907c958001350c |
| ntwshk64.dll | f69666efb2c9aeca0ec342639dbab75f4bea22a27da2d9d4ce6d30424e679d0a |
| demo.exe | 4e7015e200944e813e6a1551c6eaaea9f31ae6647825b4fd7cbccc4f5dcc2613 |

源码快照清单 SHA-256：`8c1c68ca767dfa089051ee3179f375d00b998c2880e1a75c27cedd5e59fabcba`。测试后清理本批构建、候选及发布备份。

提交差异统计（不含本记录）：9 个文本文件，新增 283 行、删除 20 行、净增 263 行，其中 166 行是用户要求的顺序台账；另有 8 个二进制文件。公共层拥有原生列表状态快照，Win16 负责消息和重绘；旧私有快照实现已删除。
