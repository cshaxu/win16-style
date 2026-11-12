# T26 S1 第十批共享候选

快照清单 SHA-256：`333843a28bcfbb1f40b898b58c12ad0ee0801895ea7b704fdb6a63a9dcb3ff4f`。MinGW GCC、Ninja、ccache 双位宽干净构建及来源、原资源、ABI、依赖边界检查通过；先发布后测试。assets 与 O:\winnt 的八文件摘要一致。

| 文件 | SHA-256 |
| --- | --- |
| demo.exe | 1224941a452398295588e402d04280312bb0824db7dfdb30a77f8dec21710e3a |
| ntw16s32.dll | f2f0089d1bd1436758fcb5700bc08edf0b6279f66988db6072eed0fd3b699ef5 |
| ntw16s64.dll | dbda78b7784c2249eea428d8222b4551b21c4044b88986416b453fa35a219aa4 |
| ntw2ks32.dll | f08479b07e5eea28a36c0a92eadbb0b3c8733b481d8326905d3c639d73f4f918 |
| ntw2ks64.dll | ffa3db80d0a55c81ccf8ea1442b141f470e0cbb8d60f218d4437e9253520a629 |
| ntwshk32.dll | dd34b3f65c8c96d6e5b93c4c0d94f3081986a89d0efc2030354e2d5aa6888fc5 |
| ntwshk64.dll | 0ed1af516a231b67b9483965297baac9aee0dfbb53e34d82af1a6f56ababc09e |
| run16s.exe | 17aa0d754d834ee1364982c821f4f83991fd307493c7fc15a6f787f6885c495c |

发布后双位宽：公共现代宿主安装失败及重入、原生子类脱离/标题刷新重入、Win16 隐藏外框重复安装和销毁、菜单快照及 Win2000 实际系统菜单打印、公共表面模拟、两风格全局窗口切换桥接通过。正式 `O:\winnt\run16s.exe` 加载自建目标的 Win16 系统菜单和父子四个位宽组合通过，8 个父子记录均确认附着及完成；Win2000 父子四个位宽组合的外框和控件附着、绘制和移除通过。

未通过或未覆盖：真实分层表面呈现返回错误 5，标记跳过；此前跨进程分发错误 126、UWP 全局目标准入错误 5、Win2000 DWM 状态恢复退出 3、Win2000 打印夹具和真实窗口回调异常仍未解决。真实 UWP/WinUI、正式全局会话、完整矩阵及视觉验收未完成。未使用 computer-use，未创建或切换隔离桌面。本候选不构成 S1/T26 收口。
