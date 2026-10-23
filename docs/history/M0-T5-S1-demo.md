# M0 T5 S1 — native demo

T4关闭和T5准入已推送于7f0bff7。固定来源仍为
5b0b65f66c281050aa4ce1454c6cd31441a6d484；本S读取其Git blobs的main.c、
gallery.c和组件README，未复制有未提交状态的来源工作区。

选择原gallery的标准菜单、六类控件、复选/单选、列表/组合选项、原生滚动及
消息循环内容，整理为一个可直接比较的普通窗口；不是28页完整gallery移植。
main移除ABI检查，gallery移除主动install/creation scope、私有font、palette、
owner-drawn菜单和计数。没有LoadLibrary/GetProcAddress、样式链接、参数/环境
外观选择或自测接口。没有复制源资源头、像素、图标或字体。

demo只构建x64，采用C17、Release /MT /W4 /WX。File/Reset重置普通控件数据，
Apply将edit内容展示到状态label，checkbox/radio/list/combo/scroll响应原生通知，
Help/About为普通消息框。Windows拥有控件数据/外观及输入，应用拥有菜单/主窗口/
消息循环；失败路径释放尚未交给窗口的菜单，窗口销毁由Windows回收所属菜单和子窗口。
无自建字体/GDI资源。PMv2只设置本进程，不修改全局DPI。

根CMake只在x64注册demo，不链接ntw16s-installer或DLL。三个组件四产物均重建
为数值0.5.0.0、FileVersion M0 T5，ABI3不变。91个来源回执包含两个adapt文件
及新README/version，固定来源blob元数据不变。

s1-build PID16872最终exit0，两宽core/abi各1/1；MSVC /W4 /WX无警告。
demo真实PE为AMD64/PE32+/GUI，只直接导入USER32/KERNEL32；不存在x86 demo。
SHA256 3cd94f25800a56d4593e52cc3293a1645a70bf396590c608f32e651e42d38d34。
逐文件源码无Ntw16s/win16_style_/LoadLibrary/GetProcAddress/环境或命令行选择。
来源91回执、23负例、治理13负例和diff --check均通过。

同文件两路业务/附着/截图由S2外部
observer接收，完整回归/任务审计由S3接收。这里不把构建成功当作UI验收。
暂存统计（本行加入前）：9文件+246/-9；src（含说明/版本）+173/-6，CMake+5/-1；没有移动旧路径。

P1 9d94cdf已推送，实际9文件+247/-9；协调复核实际main入口、x64独立链接和
完整gallery业务/资源分支，无样式消费或测试逻辑。关闭S1，进入S2独立observer。
