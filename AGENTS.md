# Harufushi Patch Dependency Project Guide

本仓库目标是从零制作一款围绕“秋起”和“春伏”的自研 C++ 日常互动游戏，并最终以可审核、可维护、可发布的形态登陆 Steam。

## Game Name

首选标题：

- 中文名：`春伏补丁依存症`
- 片假名名：`ハルフシ・パッチディペンデンシー`
- 副标题：`秋起的妄想模组日常` / `アキオキのモッド・デイズ`

命名理由：

- `春伏`把角色放在第一信号位。
- `补丁 / パッチ`直接对应 modder / coder 玩法。
- `依存症 / ディペンデンシー`表达病态亲密、自我幻想和系统依赖感。
- 副标题保留秋起视角，避免主标题只像角色名而不像游戏。

备选标题：

- `秋起补丁日志` / `アキオキ・パッチログ`
- `春伏妄想编译中` / `ハルフシ・ファントムビルド`
- `猫妹模组依存日常` / `ネコミミ・モッドディペンデンス`

## Product Direction

玩家扮演学生兼 modder/coder 的秋起，在现实学习、个人 mod 制作、直播/网络反馈、幻想兄妹关系和春伏的病态傲娇互动之间分配时间与精神资源。春伏既是“妹妹”，也是秋起同一人格的幻想化投影；系统设计要允许玩家在可爱日常、依赖关系和自我侵蚀之间感到拉扯。

核心玩法必须服务三件事：

- `日程与状态管理`：学习、写代码、做 mod、休息、陪春伏、处理网络反馈。
- `互动叙事`：春伏的语气、亲密度、嫉妒、依赖、崩坏倾向随玩家行为变化。
- `modding 生活模拟`：玩家在游戏内推进虚构 mod 项目，遇到 bug、需求、发布、评论、维护等事件。

## Technical Boundaries

- 不使用 Unity、Unreal、Godot 等现有游戏引擎。
- 不使用 Qt、Dear ImGui、Electron 等通用 UI 框架。
- C++ 是主实现语言；CMake 是构建入口。
- 第一发布目标为 Windows 桌面版，架构保留平台适配层，后续再评估 Linux/macOS。
- 可使用操作系统 API、C++ 标准库、CMake、官方 Steamworks SDK。
- 第三方库必须先写入本文件的依赖决策记录，并说明为什么不能由项目自研或系统 API 替代。

## Resource Protection Policy

资源保护从架构第一天开始存在，不能作为发售前补丁临时追加。

原则：

- 原始图像、音频、脚本、语言文件不得直接从 `resources` 裸拷到发售包。
- 运行时资源读取必须经过 `engine::resources` 抽象，不允许业务代码直接打开散文件。
- 资源包默认 fail-closed：缺 key、认证失败、版本不匹配、manifest 被改动时拒绝加载。
- 密钥不能硬编码在资源文件中；本地开发 key 只允许放在被 git 忽略的开发配置中。
- 需要承认客户端加密无法做到绝对不可提取；目标是提高盗用成本、防止普通解包、检测篡改，并为商业素材授权留下可审计链路。

路线：

1. 第一阶段建立加密资源包接口、manifest、资源 ID、读取错误模型和测试。
2. 第二阶段加入打包工具，把散资源编译为加密包。
3. 第三阶段在 Windows 后端接入系统加密 API 或项目内审计过的 authenticated encryption 实现。
4. 第四阶段加入资源签名、版本绑定、按构建渠道派生 key、异常遥测。

## Top-Level Directory Standard

顶级目录固定为：

- `resources/`：源资源与开发期数据，按类型和语言分类。
- `src/`：C++ 源码，按引擎、平台、游戏内容、工具分区。
- `cmake/`：CMake 模块、编译选项、平台检测。
- `build/`：本机构建目录，只保留 `.gitkeep`，生成物不提交。
- `bin/`：本地运行输出目录，只保留 `.gitkeep`，生成物不提交。
- `test/`：测试源码、测试夹具、测试支持代码。

允许的顶级文件：

- `AGENTS.md`
- `CMakeLists.txt`
- `.gitignore`

### Resource Layout

- `resources/images/characters/`
- `resources/images/backgrounds/`
- `resources/images/cg/`
- `resources/images/ui/`
- `resources/icons/app/`
- `resources/icons/ui/`
- `resources/audio/bgm/`
- `resources/audio/se/`
- `resources/audio/voice/`
- `resources/data/scripts/`
- `resources/data/schemas/`
- `resources/data/plans/`
- `resources/localization/en-US/`
- `resources/localization/zh-CN/`
- `resources/localization/ja-JP/`

语言目录只是初始内容示例，不允许在代码里硬编码“只有三种语言”。语言必须通过 locale registry 或资源 manifest 发现。

### Source Layout

- `src/app/`：程序入口和启动编排。
- `src/engine/core/`：生命周期、错误、日志、时间、基础类型。
- `src/engine/platform/`：Windows、文件系统、窗口、线程、系统服务适配。
- `src/engine/graphics/`：渲染抽象、纹理、字体、后端接口。
- `src/engine/audio/`：音频抽象、混音、音量、后端接口。
- `src/engine/input/`：键鼠、手柄、文本输入。
- `src/engine/ui/`：项目专用 UI 树、布局、控件和动效。
- `src/engine/resources/`：资源包、manifest、资源 ID、加载缓存。
- `src/engine/security/`：加密、签名、key provider、认证错误。
- `src/engine/localization/`：语言 registry、字符串表、格式化。
- `src/game/content/`：游戏数据模型、剧情节点、角色状态。
- `src/game/scenes/`：开屏、标题、日常、modding、结局场景。
- `src/game/systems/`：日程、状态、存档、事件触发、mod 项目模拟。
- `src/tools/packager/`：资源打包、manifest 生成、加密工具。

### Test Layout

- `test/engine/`：引擎模块测试。
- `test/game/`：游戏逻辑测试。
- `test/support/`：无第三方依赖的最小测试工具。

## Implementation Roadmap

### Milestone 0: Repository and Frame Baseline

- 建立 Git、目录规范、CMake 工程、构建输出规则。
- 建立最小测试框架。
- 建立 `engine::core::Application` 生命周期。
- 建立资源 ID、manifest 和加密资源接口。
- 建立 locale registry，不硬编码语言数量。
- 输出可运行的 console bootstrap，用于验证启动链路。

完成提交：`feat: 建立自研frame骨架`

### Milestone 1: Native Window and Loop

- 用 Win32 建立窗口、消息泵、输入事件。
- 统一 frame tick、固定/变长更新时间、退出流程。
- 写窗口生命周期和输入映射测试。

完成提交示例：`feat: 建立原生窗口循环`

### Milestone 2: Render and UI Core

- 建立渲染后端接口，先做 2D sprite、纯色矩形、文本占位。
- 建立项目专用 UI 树、布局、按钮、文本框、tooltip、转场。
- 不引入通用 UI 库。

完成提交示例：`feat: 实现项目专用ui树`

### Milestone 3: Encrypted Asset Pipeline

- 实现 packager，把 `resources` 编译为加密包。
- 加入 manifest 认证、包版本、资源类型、locale 资源索引。
- 游戏运行时只读资源包，不读裸资源。

完成提交示例：`feat: 实现加密资源包读取`

### Milestone 4: Opening, Title, and Daily Loop Prototype

- 实现开屏动画、标题界面、基础日程选择。
- 建立秋起/春伏状态模型。
- 做第一轮可玩的“写 mod / 学习 / 和春伏互动 / 休息”循环。

完成提交示例：`feat: 游戏开屏动画`

### Milestone 5: Modding Simulation and Narrative Branches

- 加入虚构 mod 项目需求、bug、发布、评论、维护事件。
- 春伏响应系统接入亲密度、嫉妒、依赖和崩坏倾向。
- 加入多结局条件。

完成提交示例：`feat: 加入mod维护事件系统`

### Milestone 6: Save, Audio, Localization, Content Tools

- 实现存档格式和迁移。
- 加入音频后端、音量设置、BGM/SE/voice 通道。
- 完成语言切换和文本导入流程。
- 建立内容校验工具，防止剧情引用缺失资源。

完成提交示例：`feat: 支持多语言字符串表`

### Milestone 7: Steam Release Track

- 接入 Steamworks SDK 和成就/云存档的适配层。
- 准备商店页素材、胶囊图、截图、预告片、隐私与内容声明。
- 提交 Valve 审核前，确保商店页功能描述和实际 build 一致。
- Valve 官方文档显示 Steam Direct 每个 app 需 100 USD 或等值费用；商店页和产品 build 需要审核，通常为 3-5 个工作日，建议至少提前 7 个工作日提交。
- 官方参考：
  - https://partner.steamgames.com/doc/gettingstarted/appfee
  - https://partner.steamgames.com/doc/store/releasing
  - https://partner.steamgames.com/doc/store/review_process

完成提交示例：`feat: 接入steam发布配置`

## Development Rules

- 所有 C++ 行为代码遵循测试先行：先写失败测试，确认失败原因，再写最小实现。
- 每个可独立验证的完成点都要提交，提交信息使用 Conventional Commits。
- 不提交 `build/` 和 `bin/` 里的生成物。
- 不提交未加密的商业素材到发售包。
- 不把语言列表写死在 C++ 分支里。
- 不让游戏逻辑直接依赖平台 API；平台差异在 `src/engine/platform/` 内结束。
- 不让游戏逻辑直接依赖资源文件路径；业务层只使用资源 ID 或内容句柄。

## Commit Convention

格式：`type: 中文动词短语`

常用类型：

- `feat:` 新功能或新阶段能力。
- `fix:` 修复已确认 bug。
- `test:` 只改测试。
- `docs:` 文档、计划、规范。
- `build:` 构建系统。
- `refactor:` 不改变行为的重构。
- `chore:` 仓库维护。

示例：

- `docs: 制定游戏计划与目录规范`
- `feat: 建立自研frame骨架`
- `feat: 游戏开屏动画`
- `test: 覆盖资源manifest解析`
- `fix: 修复语言资源缺失时崩溃`

