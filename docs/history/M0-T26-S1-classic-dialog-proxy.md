# T26 S1 传统对话框 COM 代理

COM 文件对话框代理及操作分发从 `win16-dialog/proxy*` 迁入
`lib-style/common/classic_dialog_proxy*`，公共接口使用风格无关名称。
代理负责已有 COM 身份和引用计数、配置、结果和经典对话框状态；展示、
事件订阅和文件夹设置等产品策略仍通过调用方提供的分发和服务执行。
Win16 产品、事件适配与测试已切换，旧路径和旧符号已删除。

双位宽 MinGW 增量 DLL 构建通过，`core-win16-dialog-chrome`、
`core-win16-dialog-link`、`core-style-dialog-adapter` 均通过。测试覆盖
既有代理状态、回调和 COM 适配，但未验证真实交互式文件对话框的全部
参数及其他样式接入。测试临时目录已清理；来源清单和 diff 格式检查通过。

工作区尚未干净打包、发布或提交。已知 UWP 分发和 Win2000 DWM 恢复失败
仍未解决。

后续增加仅链接 `style-dialog-common` 的独立原生测试，创建系统文件
对话框 COM 对象并验证代理创建、`IUnknown` 身份、`IOleWindow` 查询、
调用方回调与引用释放；x86、x64 均通过。测试不打开交互式对话框，也
不证明完整接口和错误路径。
