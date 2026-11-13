# T26 S1 第二十三批共享候选

双位宽 MinGW、Ninja、ccache 干净构建通过，先发布至 assets 与 O:\winnt，再运行针对性测试。

现代文件对话框的线程局部重入状态及作用域转入公共组件，产品 COM 入口直接读取；Win16 保留样式作用域。删除产品侧 VB 绘制纯转发，改为调用控件实现并注入产品判定策略。

双位宽公共现代对话框重入、线程隔离与失败清理，以及 VB 字体和绘制作用域测试通过；正式发布版双位宽 Open、Save As、Select Folder 自动取消及 VB 控件绘制、恢复回归通过。该验证不覆盖用户确认或所有对话框选项。

真实可见 UWP/WinUI、真实全局会话、钩子错误 126、Win2000 DWM 恢复及打印夹具等失败和未覆盖范围仍保持开放；S1 全面审计和 T26 尚未收口。

| 文件 | SHA-256 |
| --- | --- |
| run16s.exe | ca9a844d12c8cf12b343bf528cf7deaccbb2338ddd7a05c7918e931fdd5876a8 |
| ntw16s32.dll | ccc6e0ba213b8b9c3e1049a6684f7571eb20689cfb2326229d35a405363c01bb |
| ntw16s64.dll | 2a8db082ba60edaa990d4269c22a1c81a6cb9c14f8748350b7736e9f6d39b91f |
| ntw2ks32.dll | 8601b6635604234b66e0a8c968a816d5895dda4ff4a322fe51e3b4a5c37b9bb8 |
| ntw2ks64.dll | ccea475339e0212c9c0609bfe39e8eef7b830e79ea34231db9afd3adcf232ee7 |
| ntwshk32.dll | aed51213a4f67dedccd964916a3d5b6158c23a72f18352da450b9d573cdb60c4 |
| ntwshk64.dll | 632ef363401d2bb8b3e7a5696ffef11bf929790f581eba1037935fe85a7e30d5 |
| demo.exe | 9c70cd0da645237eb394a8f334ed725f140e9bf18070fab63119facde74ffe42 |

源码快照清单 SHA-256：`a35ebd0cc96eabbf66a75c42385f0067f551a85f7a71717334802eba9bf42fde`。本批临时构建、候选包及发布备份在验证后清理。
