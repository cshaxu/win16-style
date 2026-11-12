# T26 S1 第十一批共享候选

快照清单 SHA-256：`9c15f2d22d9cc21396d235eafa7232cfa95d906d8549729fde73f00fa953b451`。MinGW GCC、Ninja、ccache 双位宽干净构建及来源、原资源、ABI、公共依赖边界检查通过；先发布后测试。assets 与 O:\winnt 八文件摘要一致。

| 文件 | SHA-256 |
| --- | --- |
| demo.exe | 5994c089314f7c35a26d2227c4c41b32d6dda125dbc03d278f643d91e0c5e28c |
| ntw16s32.dll | 1351d631bdbb78c379985ad3d181c3cea9e60883da9a94071a6b11e2d0202e76 |
| ntw16s64.dll | 800661bdfcfd5c2d768358183dd22f5de6d75cc8617825e1cd9b7646b127e2de |
| ntw2ks32.dll | cfb7f023d29fad809782075641c2fd1a45fb35130438cd2f47451b5959516e24 |
| ntw2ks64.dll | 5957c6320d114bc9b309e59f25a5e395b530632fe4044f1ab2a1e6f153e69c02 |
| ntwshk32.dll | ed78b22ccf704aea563607ad382e31205deb22116a096778e97862a5e357fde9 |
| ntwshk64.dll | d0331801ea9390fc49e4ad7f7f5a7dd0c76214e0236b9f310a2e5915f60ec1bd |
| run16s.exe | 583f6ef7ab68ac9a57f360d8bb7e0b2a574798995386d57a7589dc98a15917fa |

发布后双位宽：公共创建记录、原生组合框/日期身份、v5/v6 状态栏身份、公共颜色对话框和完整公共对话框库链接测试通过；真实发布 DLL 的嵌套创建作用域通过。正式启动器配合自建目标，ANSI/Wide 颜色对话框键盘与色块、VCL 颜色/日期/状态栏、VB 组合框与编辑框回归通过。

未通过或未覆盖：真实 UWP 分层表面呈现错误 5、跨进程分发错误 126、UWP 全局目标准入错误 5、Win2000 DWM 状态恢复退出 3、Win2000 打印夹具和真实窗口回调异常仍未解决；真实 UWP/WinUI、正式全局会话、完整矩阵和用户视觉验收未完成。未使用 computer-use，未创建或切换隔离桌面。本候选不构成 S1/T26 收口。
