# M0 T21 S44：同步公共交集与共享测试

S44 已本地验证并发布，待提交及远端复核；M0 固定，T21 保持开放。来源固定为 ntvdm64
`967ef3d31fcb2a6d21074151875d7accd1e4c18a`，本地基线 `d27956b4`。
只读取来源 Git 对象，未修改兄弟仓库，也不把它作为构建、运行或测试依赖。

## 有限导入范围

| 组件 | 生产文件 | 结果 |
| --- | ---: | --- |
| handle | 2 | 与固定 Git 来源逐字节相同，保留 |
| process-image | 6 | 与固定 Git 来源逐字节相同，保留 |
| process-launch | 6 | 与固定 Git 来源逐字节相同，保留 |
| process-hook | 32 | 与固定 Git 来源逐字节相同，保留 |

来源工作区的 CRLF 不能作为 Git 源差异；46 个生产文件不需要重复覆盖。
process-global 不在来源交集内，保持原样；样式代码、资源和 ABI 均未修改。
逐文件 source commit/blob、源与本地 SHA-256、处置和声明见
[71 项导入清单](../etc/evidence/t21-s44-shared-import.json)。

导入其余 25 个共享测试及构建文件，保留本地其他测试。
通用测试不再依赖产品私有头或具体样式 DLL 名；新增访问权限、载荷校验、
晚期 guard 竞争和跨宽 helper/staging 矩阵。旧产品生命周期测试通过 git mv
迁至 tests/run16s-exe/process_lifecycle.c，原 core/process-lifecycle 仍运行；
未以通用测试替代产品事务测试。旧 verify-direct-mingw.ps1 由共享矩阵取代。

两项明确的导入差异：image_machine_test.c 将相邻两个独立 if 分行，以满足
本项目 GCC -Werror；upstream-sha256.txt 的许可证摘要按固定 Git 源的 LF
字节修正。生产许可证不改。根 CMake 接入共享测试，并沿用 win32-handle
既有的 GetProcAddress 函数指针转换编译选项，不修改公共生产实现。

## 验证入口与初步结果

- 根构建目标 shared-win32-tests；CTest 精确筛选
  `^(shared-.*|unwind|late_batch|propagate|core/process-lifecycle)$`。
- tests/lib-win32/verify-matrix.ps1：双宽映像访问、四条直接载荷边、四条
  helper/staging 边、两条拒绝、低地址耗尽，共 13 项；目标均由测试自建。
- tests/lib-win32 可独立以 CMake/Ninja 配置，或由项目 add_subdirectory 引用。
- 来源摘要、资源、公共组件依赖及治理门禁；实际包回归不依赖来源仓库。

增量 GCC 两宽各 14 项共享测试和产品生命周期通过。第一次编译因测试缩进
警告失败；第二次 x64 因测试 provider 未继承既有函数转换选项失败，均已最小修正。
第三次筛选误含旧树 propagate/* 集成测试；cmd-nested 的第一项失败：观察到
helper DLL 后读取 live state 失败，等待外层标记失败，自有进程已全部清理。
该旧树产品未在本次增量目标中重建，不作交付依据；保留失败并以干净树复测。

MSVC /std:c17 独立共享测试：x86/x64 各 14 项编译、链接、运行通过，13 项
跨宽矩阵通过。首次后台脚本因 scratch 目录未创建未能启动，建目录后完成。
这证明本次公共源码和测试的 MSVC 可用性，不代表已经验证 ntvdm64 产品。
产品工具链仍为纯 C17、MinGW GCC、Ninja、ccache；不增加第五个产品文件。

## 完整交付验证

s44-clean1 同快照双宽干净构建、ABI、资源、来源及公共源码/静态库依赖门禁
通过；依赖跟踪没有兄弟仓库输入。Ninja 8 并发、既有 ccache：x86 337.4 秒，
629 次命中/2 次未命中；x64 404.0 秒，696 次命中/2 次未命中。

干净树 GCC 两宽各 15 项通过（14 项共享测试及原产品生命周期）；13 项跨宽
矩阵全部通过。真实 cmd 嵌套集成测试通过，4.47 秒；旧增量树观察失败的具体
原因仍未知，不将干净树通过称为产品缺陷修复。所有产品 C 源码未改。

s44-package1 串行 44 项实际包聚焦回归通过，无顶层 skip。global-policy 的
其他 window station 子项双宽仍 skip（start=1/exit=12），不计通过；manager
停止后绘制摘要为 2875890938，与基线相同，但不等原始 343534586。真实托盘
端到端、管理员 ETW 和全项目完整矩阵未在本 S 重跑，不扩大完成结论。

s44-publish1 已备份并发布到 assets 根目录和 O:/winnt；八个部署文件摘要
均与测试包相同，发布前无占用者，没有关闭用户进程。四文件版本 M0 T21，
run16s/demo/ntw16s64 为 x64，ntw16s32 为 x86。


- `run16s.exe`：`8ddf09a5f93846c302e30215bc8ee0dd16ee5c45280a1e99ba716a26d0591ba1`

- `ntw16s32.dll`：`81a121f9e320bb0776cde0c25ee9bdd52b62333ca0c697086f5d6eb884e381c1`

- `ntw16s64.dll`：`ddff9678928ea2f67dddbd64912c930ef8599e3f61af34fa44a7219affd3c3c1`

- `demo.exe`：`af6e90b6e45bf2c4b3816fc4427e02da7cded8155c6ebd37ed0db7438f3efbae`
