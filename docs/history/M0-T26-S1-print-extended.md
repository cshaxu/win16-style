# T26 S1 传统扩展打印桥接

`PrintDlgExA/W` 到传统 `PrintDlgA/W` 的受限桥接由
`win16-dialog/print_extended` 移至 `lib-style/common/print_extended`，并由
`style-dialog-common` 提供。公共符号和对话框窗口属性更名为风格无关名称；
页面范围、复制资源、结果、支持条件和回退判断保持原实现。Win16 产品
拦截器仍决定是否调用桥接及何时回退原生 API；Win2000 尚未接入。

两种位宽的 MinGW 增量构建通过。新增独立原生测试以调用方提供的假
`PrintDlgA/W` 验证确认、取消和不支持参数不调用桥接；两种位宽均通过。
已有 `core-win16-dialog-link` 设置测试临时目录后双位宽通过，测试自行
删除了其临时目录。首次未设置 `WIN16STYLE_CAPTURE_DIR` 时该测试返回
41，补齐环境后重试通过。测试不覆盖真实打印机、交互式对话框、Apply
或完整异常和资源路径；已有 `tests/ntw16s-dll/print_dialog_target.c` 是
后续产品集成验证入口。

后续双位宽独立测试追加了 ANSI 和 Unicode 的设备模式、设备名称资源
所有权：取消时原句柄仍由调用方持有，确认时结果使用复制的句柄和数据。
两种位宽通过；未模拟打印驱动、Apply 快照或资源分配失败。

修改尚未干净打包、发布或提交；之前 UWP 分发及 Win2000 DWM 恢复失败
仍未解决。迁移代码及资源声明已更新来源清单。
