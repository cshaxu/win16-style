# T26 S1 第二十四批共享候选

双位宽 MinGW、Ninja、ccache 干净构建通过，先发布至 assets 与 O:\winnt，再运行测试。

CoolBar 自定义绘制协商由公共控件组件负责；原生身份核验、目标策略和 Win16 像素仍由 Win16 提供。删除旧 Win16 专属通知实现；其他风格尚未接入此入口。

双位宽公共真实窗口消息测试覆盖应用拒绝、接受、各阶段和身份失效；正式发布包运行自有 CoolBar 测试，双位宽通过嵌套、异常、移除、重设父窗口、应用自定义内容与颜色回退。未覆盖其他风格或真实 Delphi 应用。

真实可见 UWP/WinUI、真实全局会话、钩子错误 126、Win2000 DWM 恢复及打印夹具等已知失败和未覆盖范围继续开放；S1 和 T26 尚未收口。

| 文件 | SHA-256 |
| --- | --- |
| run16s.exe | 44beec7e49a6dbfc4f47ee154507dce2b4035b89028a6b0dfb3c80b38c273e6c |
| ntw16s32.dll | 2f9ae5a4019ff4a796e804ec23703a01f2ba072e1e887f78631579ec8ca0c031 |
| ntw16s64.dll | 59fb8c8b8c6cdab8ec9c064c939552a9e17561d9c7047807f452d41eab608337 |
| ntw2ks32.dll | dc3371767016900c92c4d358a780380f9ae06cf31933fff50bd5e9cb68c7961c |
| ntw2ks64.dll | 42339a18ba2423e48250d7cf29683384f2cb66428311622b2b7ddf834b2a5ff4 |
| ntwshk32.dll | 2d704675b92a6631e1846b6f230f02be610def8f24cc09de69f42a89f9d5afd0 |
| ntwshk64.dll | a02b491bd26178f2421466b3999abdf11ca89923be862c192404b33daf04ca5f |
| demo.exe | fd30dafaaeb859345900adcdf018262928b70ba53dc4324e1a1301cddacfcd7e |

源码快照清单 SHA-256：`dbdc4ed9c31e4a9a88ae36604f171e7ed6dfdea28434d035adfcf43d5c49e76d`。测试后清理本批构建、候选及发布备份。

提交差异统计（不含本记录）：8 个文本文件，新增 66 行、删除 25 行、净增 41 行，另有 8 个二进制文件。公共库持有协商机制；作用域、身份核验和绘制由 Win16 调用方持有，旧 Win16 私有协商入口已删除。
