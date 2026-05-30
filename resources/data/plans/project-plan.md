# Harufushi Patch Dependency Project Plan

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

## Resource Protection Plan

资源保护目标是强于常见商业 Galgame 引擎的静态封包保护：不依赖“文件名改掉、压缩一下、统一密钥全包加密”这种弱方案，而是从资源生命周期、包格式、manifest 认证、运行时解析边界和明文清理一起设计。

必须承认：资源要展示给玩家时，客户端本机最终会产生可被显卡、声卡或解释器消费的数据，因此任何客户端 DRM 都不能给出绝对不可提取承诺。本项目的强度目标是让攻击者不能通过简单解包获得素材，不能一次性批量解出全包，不能通过替换 manifest 绕过校验，不能从运行目录或缓存目录拿到明文文件，并且只能在极短的当前展示窗口内接触单个资源或分片的明文。

架构要求：

- 资源包采用 authenticated encryption，manifest 和 payload 都做认证。
- 每个资源或资源分片使用独立 nonce，并通过构建渠道、包版本、资源分组、资源 ID 派生 key material。
- 运行时只开放 presentation lease：当前画面、当前音频、当前脚本执行片段需要哪个资源，就只在受控作用域内解密哪个资源或分片。
- 展示路径为 `读取密文分片 -> 认证 -> 临时解密 -> 解码 -> 上传 GPU/音频后端/解释器 -> 立即清零 CPU 明文`。
- 禁止整包解密、启动时批量解密、场景切换时把整个场景素材解成长期明文缓存。
- 业务层只能拿 opaque handle，不能拿可持久保存的明文字节数组。
- 包索引、资源类型、locale、版本、依赖关系全部参与认证。
- 资源缺失、认证失败、密钥派生失败、manifest 被篡改、资源类型不匹配时 fail-closed。
- Debug build 可以使用开发 key，但正式 build 不允许开发 key、裸资源读取开关或明文导出工具进入发售包。

阶段实现：

1. `Milestone 0`：在 C++ frame 中建立 runtime lease / callback 资源访问契约，移除“长期返回明文字节”的默认路径。
2. `Milestone 1`：资源包格式加入 authenticated manifest、资源分片索引和类型校验。
3. `Milestone 2`：packager 将 `resources` 编译为加密包；运行时只读包，不读裸资源。
4. `Milestone 3`：图像和音频接入流式/短窗口解密，上传后清零 CPU 明文。
5. `Milestone 4`：正式包加入渠道 key 派生、包签名、完整性校验和篡改错误遥测。
6. `Milestone 5`：按平台加入反调试、dump 抑制和发布构建自检，但不把这些当作加密本体。

## Implementation Roadmap

### Milestone 0: Repository and Frame Baseline

- 建立 Git、目录规范、CMake 工程、构建输出规则。
- 建立最小测试框架。
- 建立 `engine::core::Application` 生命周期。
- 建立资源 ID、manifest 和加密资源接口。
- 建立 locale registry，不硬编码语言数量。
- 输出可运行的 console bootstrap，用于验证启动链路。
- 建立 presentation-only runtime resource lease，确保明文只在当前展示作用域内存在。

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

## Dependency Decision Record

当前没有已批准第三方运行时依赖。
