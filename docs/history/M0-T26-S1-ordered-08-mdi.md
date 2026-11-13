# T26 S1 顺序第 8 项：VCL MDI 身份

VCL MDI 子窗体、MDIClient 和宿主之间的类名、父子关系、线程、模块和样式位验证由公共 `control_vcl_identity` 提供；调用方回调负责判断宿主是否已附着。Win16 产品实际调用公共入口，旧 Win16 私有身份函数已删除。判断顺序保持不变；公共组件不依赖 Win16 附着状态。

双位宽 MinGW GCC、Ninja、ccache 干净构建及公共库依赖检查通过，先发布到 assets 和 O:\winnt，再用正式程序验证双位宽 MDI 模型：正常接入、错误宿主、最大化还原、父窗口变更、角色变化和宿主移除均通过；强制子类安装失败时引用计数不泄漏的测试双位宽通过。公共身份单测覆盖错误宿主、模块、回调及样式位。测试使用直接工作入口，没有创建或切换桌面。真实 VCL 程序和其他风格尚未验证。

| 文件 | SHA-256 |
| --- | --- |
| demo.exe | 1844e31eea6379917c85e1e2a8ae0199c1b654a807c99565fd03a5d618a4eac5 |
| ntw16s32.dll | 1e13175b67bfd965613b72ad38fc4f13addfd3b46510c497f7767092babf3ceb |
| ntw16s64.dll | 44c8fa35ec655f0f240742caac3a459dc478ce98a9d91419cab7731d34b1dfa4 |
| ntw2ks32.dll | da98938c1ea001bd69ee55704e25eb427b5391ce40880cbf16c66d491477cf4b |
| ntw2ks64.dll | 67983974cc3739465a68f76f1f75f4b7bc8de96abfe3a71289827dfba6b9ff59 |
| ntwshk32.dll | 7432c325506c62d1d1850dbad4c486d4f93d30427a187147c2a12e04cd001a4e |
| ntwshk64.dll | 59ed685b8a15176e5c0a3692b4041536a0ea177afa08918b13f420461e5f86bd |
| run16s.exe | 1587bb0c93365eaf42c8c84d34dcdb2302903a4f9c95ac39942b4a042746c38d |

第 8 项其他职责仍在审计；既有导航焦点、Win2000 DWM 恢复和现代窗口覆盖限制继续开放。
