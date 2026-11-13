# T26 S1 顺序第 8 项：VCL CoolBar 身份

VCL CoolBar 的窗口、父子、线程、类、模块和窗口过程来源校验移入公共 `control_vcl_identity`；调用方提供当前附着对象判定。Win16 产品直接使用公共入口，原 Win16 私有身份函数已删除；检查顺序不变，公共组件不依赖 Win16 状态。

双位宽 MinGW GCC、Ninja、ccache 干净构建及公共库依赖检查通过，先发布到 assets 和 O:\winnt，再用正式程序验证双位宽 CoolBar 生命周期：正常附着、嵌套、异常、撤销、父窗口变化、错误所有者及窗口过程均通过。公共单测覆盖错误父窗口、所有者和附着对象。测试使用直接工作入口，没有创建或切换桌面。真实 VCL 程序和其他风格尚未验证。

| 文件 | SHA-256 |
| --- | --- |
| demo.exe | dc6977afd7e09e3646659f5406f2780020d5343aedc93b9acb775ac63b64c0f3 |
| ntw16s32.dll | 269b6806cac23f3ee0ee98aa647e7ac8e13fdc584fef62b66c2648baad5cb043 |
| ntw16s64.dll | f9a8c09d41e7df7300a2992e9ee6047eeff015e0cace3c1724e0a68ec9025663 |
| ntw2ks32.dll | fa52e3444f2cff5017aaba37dccec3fe63bc3beee9c1c81b43264198b84400a5 |
| ntw2ks64.dll | c6527a1fe2e222e2cbf6568f0175401c0b7a42bc2276e5ea5f76e52a39cab988 |
| ntwshk32.dll | 929ba2f139dc152ce69d9c26a532c9cec2b2e2a717c779d9ee1bbe0ce018434b |
| ntwshk64.dll | 44fb414760e556ad348f5317b408d501c4c5ca3177b505a3baf5349ebde74ffa |
| run16s.exe | f4520e0d3d6fd3bc76247cafc2125196455a09ebe0a67af22c79ff8f57353e4d |

第 8 项其他职责仍在审计；既有导航焦点、Win2000 DWM 恢复和现代窗口覆盖限制继续开放。
