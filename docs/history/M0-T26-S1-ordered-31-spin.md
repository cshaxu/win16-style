# T26 S1 顺序第 31 项：VCL 微调编辑框区域保护

字体变更前获取编辑区域，之后验证相同按钮与附着状态并恢复预留区域的机制迁入 `lib-style/common/control_vcl_spin`；Win16 提供自身脱离状态。

双位宽 MinGW GCC、Ninja、ccache 干净构建、资源身份及公共依赖检查通过。先发布 assets 和 O:\winnt 并核对八文件，再对发布包运行双位宽自建 VCL 微调编辑框夹具，字体变化预留区域、重入移除及清理通过；公共非目标和错误码测试双位宽通过。未使用隔离桌面。真实 Delphi、其他风格及所有重入条件未覆盖。

| 文件 | SHA-256 |
| --- | --- |
| demo.exe | f8ee41473f3414180a8b0207872c55c2d6259fde226f5920b31cac8ed76602c8 |
| ntw16s32.dll | 1f5e1c87c6e30d250638999168424ee05feb60f77e0a6b71fd88d859ce2f4fce |
| ntw16s64.dll | 8596f6e369eb41ad32a4d2fd82ab732b049f32ba9f14cf972bb176cc65241dc1 |
| ntw2ks32.dll | 42526f0c4f33537fd39ccebd92fa8bcb9a15418b3bb9a66cd235249ce9d2f633 |
| ntw2ks64.dll | e4af750e35f8bbdb67f8fdaf5abbf659e4222b2e7a918537a2a2bf124580a71a |
| ntwshk32.dll | e98d4b4e763fb0255ca8689861ffd2530ef761a8725c7bc564d4e3c50a9ad8a0 |
| ntwshk64.dll | ab0531a4b053ad1119fca4271110b087a1d21112eabd91a7a4b80accd2c7c0e8 |
| run16s.exe | 6ad1de7aac02ddb229cfb6b40eaa231a5090e2fc507528c4143d26727f556cc9 |
