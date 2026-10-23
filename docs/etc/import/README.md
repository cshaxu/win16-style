# Import Reference Inventory

来源为 ntvdm64 commit `5b0b65f66c281050aa4ce1454c6cd31441a6d484`。
[source-inventory.tsv](source-inventory.tsv) 由该提交的 git ls-tree 生成，记录
mode、type、Git blob、size、path；不是已准入复制清单，也不是内容 SHA-256。
它覆盖三个主要来源目录、Hook 家族及相关测试/构建候选。依赖闭包还须在首次
导入 S 逐项追踪，不能凭目录 inventory 声称依赖已全部闭合。

复核命令（只在准备/导入研究时使用，不是构建或运行依赖）：

```powershell
git -C <source> ls-tree -r -l 5b0b65f66c281050aa4ce1454c6cd31441a6d484 -- src/ntw16s-dll src/nthook32-dll src/run16-exe src/w16style-exe
git -C <source> show <commit>:<path>
```

源工作区存在无关未提交修改，尤其 run16 及 CURRENT 等；固定提交才是候选基线。
后续如升级来源必须新记录 commit、文件差异和影响，不能静默追随 HEAD。

T1 的逐文件处置及静态依赖：[BASELINE](BASELINE.md) 与
[source-manifest.json](source-manifest.json)。清单在候选 105 项之外明确记录
19 个排除边界，含原字节 SHA-256 和每条 include 的处置；没有复制源码。

独立化决定见 [INDEPENDENCE](INDEPENDENCE.md) 与
[逐导出 ABI 清单](abi-decisions.json)；工具链真实结果见 [TOOLCHAIN](TOOLCHAIN.md)。
本地检查和后续导入回执规则见 [VALIDATION](VALIDATION.md)。

T6 S1 纯 C 后端准入：[MinHook 固定清单](minhook-manifest.json)，官方
TsudaKageyu/minhook v1.3.4 commit c3fcafdc10146beb5919319d0683e44e3c30d537。
14项含许可证、作者、双宽 HDE、buffer/trampoline；不导入官方 DLL、构建脚本或
公开 API 层。hook.c 仅适配为初始加载的私有补丁事务，保留完整原声明；不使用
其线程冻结、Sleep/spinlock 或运行中全局附着。BSD-2-Clause 的 MinHook/HDE
完整声明须随产物资源保留。此来源与既有124项 ntvdm64 清单分开，不冒充原创。
新后端尚未构建整包通过，旧 Detours 待完整替换后清除。
