# T26 S1 第九批共享候选

快照清单 SHA-256：`1dcb1890963538232f6dc7511650006c529e83cc129c623c094339e33a2e2c35`。MinGW GCC、Ninja、ccache 双位宽干净构建、ABI、公共依赖边界和资源检查通过；先发布后测试。assets 与 O:\winnt 的八个文件摘要逐项一致。

| 文件 | SHA-256 |
| --- | --- |
| demo.exe | 17d803aa603ce244c4724f6c3dcad45ccd04ab1bca4a695c7c552911943b5bfa |
| ntw16s32.dll | 895c667cdb40a210eb3d202a44ed7764797e9b252a18ca0dd0e6b03a36e33a7b |
| ntw16s64.dll | 1f744411b31e93324dba920dd4a7aba2900dcf8f7289253e134eb75d2bf3a3a1 |
| ntw2ks32.dll | f36b9d6728bd165286536b4c25a918b8f3ad6de806692b99101ce60f3fee1a43 |
| ntw2ks64.dll | ed619257d22a7defcf0aae685edab819b03f656e78120d97e8d8b1c2187ee431 |
| ntwshk32.dll | 11db2d79d7b429c8da9517b10323c0a50a4633584acea11f182d150f2a0c8c60 |
| ntwshk64.dll | 7d5d27608e7b2df7a09ac3390985a642b41b779ab9aed9c33738adbe828da2e1 |
| run16s.exe | 46372ccafaa6d555b642bfd628a7274af6cb24acd26dc086fd787068f55e2d58 |

发布后双位宽测试：公共启动事务（包含准备失败回滚、提交失败）、真实进程启动钩子与 88/89 容量边界、XAML 边缘、外框恢复、现代表面模拟、对话框公共入口、两风格全局子窗口切换和 Win16 原生框架均通过。Win16 使用 O:\winnt\run16s.exe，自建父子进程四个位宽组合均记录窗口附着及完成；Win2000 四个位宽组合的父子外框及控件附着、绘制和移除通过。

未通过或未覆盖：真实分层表面提交返回错误 5，原生呈现跳过；此前跨进程分发错误 126、UWP 全局目标准入错误 5、Win2000 DWM 状态恢复退出 3、Win2000 打印夹具和真实窗口回调异常均未获通过。真实 UWP/WinUI 宿主、正式全局会话、完整包矩阵及用户视觉验收未完成。未创建或切换隔离桌面，未使用 computer-use。此候选不构成 S1 或 T26 收口。

正式启动入口补充测试：x86/x64 自建目标的八个普通对话框入口及六个消息框入口在原始和新建 GUI 线程均完成。首次测试未设置探针要求的 WIN16STYLE_CAPTURE_DIR，四例因环境变量缺失退出 1；设置自有临时目录重跑四例全部通过，并核对捕获标记。
