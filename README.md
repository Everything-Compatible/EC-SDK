EC-SDK
======

概览
----

EC-SDK 是为 YR（Yuri's Revenge）/IH 引擎提供的扩展 SDK，整合了以下几部分：

- IH（IronHammer）扩展接口
- SyringeIH 支持（通过 SyringeEx）
- SIWinterIsComing（WIC）扩展支持
- cJSON（作为内置的 JSON 解析库）

此 SDK 提供库初始化、Listener（事件）管理、对象加载/保存（Load/Save）、Swizzle（指针修正）以及与游戏引擎交互的一系列辅助函数。

主要目录与文件
----------------

（注：仓库以头文件和源文件的形式组织，下面列出主要文件）

- EC.* (例如 `EC.h`, `EC.cpp`, `EC.LoadSave.*`, `EC.ObjBase.*`)：EC 顶层初始化、Listener 管理与序列化支持。
- IH.* (例如 `IH.h`, `IH.File.h`, `IH.Loader.h`)：与 IHCore / IHLibList 的交互封装（内存分配、文本变量、文件抽象等）。
- WIC.*：SIWinterIsComing 相关的扩展接口（Buff、广播、属性合并、类管理等）。
- SyringeEx.*：Syringe 扩展与识别（用于注入/识别场景）。
- cJSON.c / cJSON.h：第三方 JSON 库（MIT licensed），用于数据序列化。
- REPO_SUMMARY.md：自动生成的仓库总结（包含模块说明与改进建议）。
- LICENSE：仓库许可（BSD 3-Clause 风格）。

快速开始（示例：在 Visual Studio 中构建 DLL 并集成）
--------------------------------------------------

1. 环境准备
   - Visual Studio 2019/2022（任一支持 C++ 的版本）
   - 目标架构：x86 或 x64（取决于目标游戏/引擎）

2. 新建项目
   - 新建一个 Visual C++ -> DLL 项目（例如 `ec_example.dll`）。
   - 将仓库中的 `.cpp` / `.h` 添加到项目中（至少需要 `EC.*`, `ExtJson.*`, `cJSON.c/h`, `SyringeEx.*`, `IH.*` 中相关文件）。

3. 编译设置
   - 在项目属性的 C/C++ -> 常规中设置包含目录（指向仓库根）。
   - 在链接器中确保链接任何目标平台需要的库，或选择运行时动态解析导出的符号。
   - 定义导出宏并在 DLL 的 `DllMain` 中进行必要的初始化。

4. 初始化示例（伪代码）
   - 在 DLL 的 `DLL_PROCESS_ATTACH` 阶段调用 `ECInitLibrary` 注册库信息与初始化序列：

```cpp
// 伪代码示例
bool OnFirstInit()
{
    // 注册 Listener，准备数据结构
}

bool OnOrderedInit()
{
    // 依赖项加载后执行
}

ECInitLibrary(
    "MyLib",
    1, // Version
    1, // LowestSupportedVersion
    "Example extension for EC-SDK",
    OnFirstInit,
    OnOrderedInit
);
```

5. 注入 / 加载
   - 生成 DLL 后，将其放置在游戏目录或通过 Syringe/IHLoader 注册注入。请参考 `SyringeEx.h` 与 `IH.Loader.h` 的注释以了解详细的加载流程。

依赖与许可
-----------
- cJSON: MIT licensed（源码包含在仓库内）。
- 本仓库 LICENSE: BSD 3-Clause（见 `LICENSE` 文件）。
- Syringe / IH / WIC: 本仓库假设这些运行时组件在目标环境中可用；请根据需要获取对应的运行时文件（IHCore.dll、SyringeIH 等）。

