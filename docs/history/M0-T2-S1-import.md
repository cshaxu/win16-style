# M0 T2 S1 核心导入与本地构建

用户“收口并准入下一个t 开始执行”准入队首双位宽样式核心。T1 已在 80eccb2
关闭；本次不再打开 T1，不准入启动器/传播/demo。S1 verified，待交付后实际复核。

从固定 Git 对象导入 33 个已审核心源码/头，不从脏工作区复制。五原资源头零复制，
改为 tools/generate_glyphs.py 的自编线/矩形/三角/圆几何，生成相同维度接口的
五个替代头。生成物已跟踪，构建不生成/改写来源源码。

ABI 3 由显式双架构 .def 定义 31 个稳定 C 名称，public ordinal 从 2 开始，
为后续真实 helper 保留 ordinal 1，不增加 stub；本任务不实现 helper。
原 native binding 改 DLL 私有，删除 WOW combo binding，public umbrella 不再
暴露 private header。attachment.c 为继续消费私有 native 服务显式加 include，
其处置从 retain 细化为 adapt。原线程通知/空 GUI DllMain 保留。

根 CMake 显式列举唯一核心编译源，使用 MSVC C17、/MT /W4 /WX；稳定两架构
Ninja 图位于 build/.ninja。新版本资源记录 M0 T2，不产生额外产品组件。
本地后台入口 tools/Build-Core.ps1 有 PID、日志/result 与硬超时。
导入回执记录每个实际文件及原/新 SHA-256；新素材为 project-authored 替代。

治理自检在 product import_status 变为 partial 后，原 premature_source 负例
不再触发原来 not-imported 的谓词；将该 fixture 显式切回 not-imported 再注入
源码，保持测试真实拒绝同一失败契约。产品规则未改。

有限覆盖：33 源文件、5 替代头及少量自编 .def/版本/文档；两个实际 DLL 的
位宽/ABI/export，零源资源复制，独立本地构建。实际验证结果如下；
不以本 S 的版本烟测宣称整个 T2 或实际窗口外观通过。

首轮 s1-build 的 x86 .def 以多余 underscore alias 无法解析，构建失败；此外
PATH 中 cmake 返回负 native exit code，旧 if errorlevel 1 未停止，CTest 随后
对无效 DLL 报错。该轮不计通过。修正 .def 为稳定普通 C 名及显式 ordinal，
两个后台构建工具使用“非零”判断；永久 exit guard 用 +1/-1 两种失败证明停止。
相似问题搜索同时修正 T1 的 Probe-Toolchain 相同判断，未改产品规则。

s1-relink PID 38504 成功：x86/x64 Release /MT /W4 /WX 各 CTest ABI 1/1，
真实 LoadLibrary/GetProcAddress、ABI 3、31 public 与两个 private bind 缺席均通过。
PE 独立解析为 I386/AMD64、精确 31 名字，与 ABI 决策集相等；直接 imports 只有
USER32/GDI32/COMCTL32/dwmapi/UxTheme/KERNEL32，没有服务/RPC/动态 CRT。
本轮哈希 x86=4fa84d10e7253815f64418e0cf378a85373ec22f8159c21ae21458a80494bb9f；
x64=60e78dcb864a01466733b261b8ba4c19c5fbf1e358232fa000823709b2e20d79。
hash 是本次构建事实，后续重建以新记录为准，不假设可重现 timestamp。

来源回执 43 项（42 src + ABI fixture）；本地 23 负例、治理 13 负例、
几何头再生成一致、exit guard 和 diff --check 通过。二进制仍在 build，未发布。
