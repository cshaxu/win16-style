# M0 T27 S2 Win2000基础呈现与普通按钮

按已批准16S规划自动进入，T27/M0保持。先读取S1参考清单的btnctl/draw/inctlpan与原图事实，补齐本组状态算法后实现；不把关闭Win11主题或原生DrawFrameControl未对照结果当原版证明。

## 完整能力与验收

本S提供Win2000语义palette/edge与普通/默认push按钮呈现，覆盖已有支持的按下、禁用、焦点、文字/图像、字体、UI提示和查询重入；遇到尚未验证的模式保留明确原生边界，不能算parity完成。先验证96DPI与真实Accessibility Options原图可见的边缘/色值，再验证当前适用DPI/aware与原生输入/打印/资源。

年代实现归appearance/win2000/common/control。产品ntw2ks只选择并调用安装/移除/原API适配；直接复用公共instance/install/retire、control/basic、button_snapshot、present/DC等完整机制，不复制Win3.x私有状态或共享原字形。此实现不安装另一套全局/传播后端，不改已确认样式选择UX。

S1源码确认xxxDrawButton对default加外边框，WIN40COMPAT使用BF_SOFT、default pressed使用BF_FLAT；DrawPushButton使用raised/sunken与checked dither。颜色使用源码slot语义和原图已验证标准方案（face212/208/200），不是globals启动色表的192灰。应用字体/图像保持借用，底层原生数据/点击/键盘与通知保留。

开发用稳定Ninja/ccache增量树；相关实现/测试合批后才作本S必要双宽干净八文件交付，先发布assets/O:/winnt再跑实际包回归，提交推送实际fetch复核并清理。当前尚未修改生产代码，不宣称呈现完成。
