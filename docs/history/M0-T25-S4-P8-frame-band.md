# M0 T25 S4 P8：经典外框底色

对现有原生非客户区边框内侧填充经典灰色并保留系统菜单图标区域；先画边框再画标题按钮，避免覆盖高出标题区域一像素的按钮。范围、实测及局限见 [S4 审计](M0-T25-S4-window-audit.md)。窗口几何和命中由当前系统决定，并未证明原版边框厚度一致。

源码快照 SHA-256 `ae75ad691bc4a701c2a5a37e86ef668b4b09e64282a82875123dd34b3855d2ba` 使用纯 C17、MinGW GCC 16.1.0、Ninja 和 ccache 4.13.6 双位宽干净构建；同批正式包的 36 项重点测试通过、无跳过，覆盖侧边框像素、按钮顶部、图标、命令及启动传播。正式包物理鼠标未测试；先前 P6 正式包因屏保跳过的物理测试也未因此补齐。未验证全部 DPI、真实应用和最大化边框。用户视觉验收待进行。

同批产品发布至 assets 根目录和 `O:\winnt`，与测试包 SHA-256 一致：

| 文件 | SHA-256 |
| --- | --- |
| demo.exe | e9de63075025632dfc588c685622305eead9932927c930bd1d404b7909881bc4 |
| ntw16s32.dll | 8bf55568c33b0842d2907b0bc5b349aa881ac50456a62b4be007fa83ceed3b91 |
| ntw16s64.dll | e38cfaef157d382b6a4698bacb047e22c10e9989eb40c4185260a2e09c6a8af9 |
| ntw2ks32.dll | a5886f0b04cf6990436d0d91897d9519852cca1696f82038085e62987e46780d |
| ntw2ks64.dll | d4d76a2f5c40bf8e4d38f9efbe22606b44458eabd5f63038a2c9110ef484a16a |
| ntwshk32.dll | efa3e3b2b41fe69f3e311336cd2591166f661c76fc93c2d37013893b356d0a58 |
| ntwshk64.dll | b2a7fe9c1f752f1d1819430c6642b7010581bb4b811c5cd70140df384c390404 |
| run16s.exe | a9fa9ca25c30cf0f95f1e87fbab34ef0c07651a066f6ceac5595feea5a10568c |

S4 和 T25 保持开放。
