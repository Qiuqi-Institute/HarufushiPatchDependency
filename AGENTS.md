# Harufushi Patch Dependency Agent Guide

本文件只保存长期执行规范。完整计划书独立放在 `docs/project-plan.md`，不要再把路线图、剧情企划、Steam 发布计划等长篇计划内容塞回 `AGENTS.md`。

## Technical Boundaries

- 不使用 Unity、Unreal、Godot 等现有游戏引擎。
- 不使用 Qt、Dear ImGui、Electron 等通用 UI 框架。
- C++ 是主实现语言；CMake 是构建入口。
- 第一发布目标为 Windows 桌面版，架构保留平台适配层，后续再评估 Linux/macOS。
- 可使用操作系统 API、C++ 标准库、CMake、官方 Steamworks SDK。
- 第三方库必须先写入 `docs/project-plan.md` 的依赖决策记录，并说明为什么不能由项目自研或系统 API 替代。

## Resource Protection Requirements

资源保护是架构级硬要求，不允许作为发售前补丁临时追加。本项目目标不是“普通 Galgame 封包加密”，而是以超过常见商业 Galgame 引擎静态封包保护强度为设计目标：静态解包、整包批量解密、直接替换 manifest、拷贝裸素材、从缓存目录拿明文素材都应被架构阻断。

必须正视一个边界：客户端游戏只要要显示图像、播放音频或执行脚本，就一定会在某个极短时间窗口内把对应资源转成可用形态。因此规范不写“绝对无法提取”这种不诚实承诺；真正的目标是把明文窗口限制到当前正在展示或播放的资源、限制到最小内存区域、限制到最短生命周期，并在资源离开展示链路后立即销毁明文。

强制要求：

- 原始图像、音频、脚本、语言文件不得直接从 `resources` 裸拷到发售包。
- 运行时资源读取必须经过 `engine::resources` 抽象，不允许业务代码直接打开散文件。
- 加密资源不能提供“读出完整明文字节数组并交给调用方长期持有”的通用 API。
- 展示、播放、执行资源必须通过短生命周期 runtime lease / callback 完成：解密、解析、上传到渲染或音频后端、擦除 CPU 明文缓冲区，必须发生在同一个受控作用域内。
- 同一时刻只允许解析当前场景实际需要展示、播放或执行的资源；禁止启动时整包解密、场景预热整包解密、后台批量导出明文缓存。
- GPU 纹理、音频缓冲、脚本字节码等后端对象必须通过 opaque handle 暴露，不能暴露可逆向还原的源素材字节。
- 资源包默认 fail-closed：缺 key、认证失败、版本不匹配、manifest 被改动、包索引异常、资源类型不匹配时拒绝加载。
- manifest 本身也必须认证；正式包中 manifest 不允许作为可自由编辑的明文索引存在。
- 每个资源或资源分片必须支持独立 nonce / key derivation / authentication tag；禁止全包共享固定密钥和固定 IV。
- 密钥不能硬编码为单个可搜索字符串；开发 key 只允许放在被 git 忽略的本地配置中，正式 key 需要按构建渠道、包版本、资源分组派生。
- 所有解密后的 CPU 明文缓冲区必须显式清零；敏感路径禁止写入普通日志、dump、异常消息和缓存文件。
- 资源保护代码必须有测试覆盖：认证失败、缺 key、缺 cipher、manifest 篡改、资源 ID 越界、生命周期释放后不可访问。

设计方向：

- 使用 authenticated encryption，而不是单纯 XOR、压缩、混淆或只做文件名加密。
- 资源按小块封装，运行时只解密当前展示链路所需分片。
- 渲染路径优先采用“解密到临时缓冲区 -> 解码 -> 上传 GPU -> 清零临时缓冲区”的单向流程。
- 音频路径优先采用流式小窗口解密，不把整首 BGM 或整段 voice 解成完整明文文件。
- 脚本和语言资源进入解释器或字符串表后，源字节立即清零，业务层只持有解析后的最小结构。
- 后续允许加入反调试、完整性校验、代码段签名、包指纹、异常遥测等防篡改措施，但这些措施不能替代 authenticated encryption 和 runtime lease。

## Top-Level Directory Standard

顶级目录固定为：

- `docs/`：计划书、设计文档、技术决策和发布文档。
- `resources/`：源资源与开发期数据，按类型和语言分类。
- `src/`：C++ 源码，按引擎、平台、游戏内容、工具分区。
- `cmake/`：CMake 模块、编译选项、平台检测。
- `build/`：本机构建目录，只保留 `.gitkeep`，生成物不提交。
- `bin/`：本地运行输出目录，只保留 `.gitkeep`，生成物不提交。
- `test/`：测试源码、测试夹具、测试支持代码。

允许的顶级文件：

- `AGENTS.md`
- `CMakeLists.txt`
- `CONTRIBUTING.md`
- `LICENSE`
- `README.md`
- `.gitignore`

## Resource Layout

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
- `resources/localization/en-US/`
- `resources/localization/zh-CN/`
- `resources/localization/ja-JP/`

语言目录只是初始内容示例，不允许在代码里硬编码“只有三种语言”。语言必须通过 locale registry 或资源 manifest 发现。

## Source Layout

- `src/app/`：程序入口和启动编排。
- `src/engine/core/`：生命周期、错误、日志、时间、基础类型。
- `src/engine/include/`：引擎公开框架入口，采用无扩展名 `Haru*` 头文件，例如 `#include <HaruButton>`。
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

## Test Layout

- `test/engine/`：引擎模块测试。
- `test/game/`：游戏逻辑测试。
- `test/support/`：无第三方依赖的最小测试工具。

## Development Rules

- 所有 C++ 行为代码遵循测试先行：先写失败测试，确认失败原因，再写最小实现。
- 每个可独立验证的完成点都要提交，提交信息使用 Conventional Commits。
- 不提交 `build/` 和 `bin/` 里的生成物。
- 不提交未加密的商业素材到发售包。
- 不把语言列表写死在 C++ 分支里。
- 不让游戏逻辑直接依赖平台 API；平台差异在 `src/engine/platform/` 内结束。
- 不让游戏逻辑直接包含 `src/engine/**` 内部路径；游戏层调用引擎必须优先使用 `src/engine/include/` 暴露的 `Haru*` 公开头。
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

- `docs: 剥离项目计划书`
- `feat: 建立自研frame骨架`
- `feat: 游戏开屏动画`
- `test: 覆盖资源manifest解析`
- `fix: 修复语言资源缺失时崩溃`
