# Sophon-Stream Web 管理系统需求文档

## 介绍

基于开源 sophon-stream 流处理框架，构建生产级边缘 AI 视频分析管理平台。实现从设备接入、算法编排、任务调度到结果上报的全流程管理，提供 Web 可视化界面，满足安防、交通、工业质检等场景的落地需求。

适用硬件：Sophgo BM1684/BM1684X/BM1688 (PCIe/SoC)

核心原则：控制面与数据面分离、配置热更新、安全认证、低资源占用

## 术语表

| 术语 | 定义 |
|------|------|
| Sophon-Stream | Sophgo 开源流处理框架，提供基于 TPU 的视频分析流水线 |
| Graph | 流处理图中的计算图，由多个 Element 组成 |
| Element | 流处理图中的基本处理单元（解码、推理、跟踪等） |
| TPU | 张量处理单元，Sophgo BM1684X 内置的 AI 加速芯片 |
| BMCV | Bitmain Media Compute Vision，Sophgo 多媒体计算视觉库 |
| Drogon | 高性能 C++ Web 框架，用于构建后端 API 服务 |
| ROI | Region of Interest，感兴趣区域 |
| DAG | 有向无环图，用于任务编排 |
| RBAC | 基于角色的访问控制 |
| MQTT | Message Queuing Telemetry Transport，物联网消息协议 |
| GB28181 | 中国公共安全视频监控联网系统信息传输标准 |
| 控制面 | 管理平台，负责配置、调度、监控 |
| 数据面 | 流处理引擎，负责实际视频数据处理 |
| 热更新 | 运行中动态更新配置，无需重启服务 |

## 需求

### 需求 1：前端 Web 管理界面

**用户故事：** 作为系统管理员，我想要一套完整的 Web 管理界面，以便可视化地管理设备、任务、算法和系统状态。

#### 验收标准

1. WHEN 管理员访问系统 URL，系统 SHALL 展示登录页面，包含用户名和密码输入框
2. WHEN 管理员提交登录表单，系统 SHALL 验证凭据并在成功后跳转至仪表盘页面
3. WHILE 管理员处于已认证会话状态，系统 SHALL 在侧边栏展示设备管理、任务管理、算法管理、监控告警、系统设置五个导航模块
4. WHEN 管理员点击设备管理导航，系统 SHALL 展示设备列表页面，支持按状态、类型筛选
5. WHEN 管理员点击任务管理导航，系统 SHALL 展示任务列表页面，支持创建、编辑、删除任务
6. WHEN 管理员点击算法管理导航，系统 SHALL 展示已安装算法插件列表，支持查看算法详情和配置参数
7. WHEN 管理员点击监控告警导航，系统 SHALL 展示实时监控面板，包含 TPU/CPU/内存使用率图表
8. WHEN 管理员点击系统设置导航，系统 SHALL 展示系统配置页面，支持网络、存储、安全参数配置
9. IF 前端收到 WebSocket 推送的资源数据，系统 SHALL 实时更新监控面板图表
10. WHILE 管理员在任务编排页面，系统 SHALL 提供 Canvas 区域支持 ROI 绘制，坐标数据 SHALL 实时同步至后端

### 需求 2：后端 API 服务

**用户故事：** 作为系统开发者，我想要基于 Drogon 框架构建嵌入式 C++ Web 服务器，以便提供高性能的 RESTful API 服务。

#### 验收标准

1. WHEN 系统启动，后端服务 SHALL 在端口 8080 监听 HTTP 请求
2. WHEN 收到 GET /api/v1/devices 请求，系统 SHALL 返回设备列表 JSON 响应
3. WHEN 收到 POST /api/v1/devices 请求，系统 SHALL 创建设备记录并返回 201 状态码
4. WHEN 收到 GET /api/v1/tasks 请求，系统 SHALL 返回任务列表 JSON 响应
5. WHEN 收到 POST /api/v1/tasks 请求，系统 SHALL 创建任务并启动对应的 sophon-stream Graph
6. WHEN 收到 PUT /api/v1/tasks/{id}/config 请求，系统 SHALL 热更新任务配置并返回 200 状态码
7. WHEN 收到 DELETE /api/v1/tasks/{id} 请求，系统 SHALL 停止任务并删除记录
8. WHEN 收到 GET /api/v1/monitoring/resources 请求，系统 SHALL 返回当前 TPU/CPU/内存使用率
9. WHEN 收到 POST /api/v1/alarm-rules 请求，系统 SHALL 创建报警规则并启动规则引擎
10. IF 请求缺少有效的 JWT Token，系统 SHALL 返回 401 状态码

### 需求 3：数据库持久化

**用户故事：** 作为系统架构师，我想要使用 SQLite 数据库存储系统配置和运行数据，以便实现边缘设备的本地持久化。

#### 验收标准

1. WHEN 系统首次启动，系统 SHALL 初始化 SQLite 数据库并创建所有必需的表
2. WHEN 创建设备记录，系统 SHALL 将设备信息持久化至 devices 表
3. WHEN 创建任务记录，系统 SHALL 将任务配置持久化至 tasks 表
4. WHEN 创建报警规则，系统 SHALL 将规则配置持久化至 alarm_rules 表
5. WHEN 系统记录报警事件，系统 SHALL 将事件数据持久化至 alarm_events 表
6. WHEN 系统采集资源监控数据，系统 SHALL 将指标数据持久化至 monitoring_metrics 表
7. IF 数据库文件损坏，系统 SHALL 从最近备份恢复数据库文件
8. WHEN 系统执行数据查询，系统 SHALL 通过 ORM 接口访问数据库

### 需求 4：认证与授权

**用户故事：** 作为系统管理员，我想要实现 JWT 认证和 RBAC 权限模型，以便控制系统访问安全。

#### 验收标准

1. WHEN 用户提交登录凭据，系统 SHALL 验证用户名和密码并在成功后签发 JWT Token
2. WHEN 用户访问受保护 API 端点，系统 SHALL 验证请求头中的 JWT Token 有效性
3. IF JWT Token 已过期，系统 SHALL 返回 401 状态码并要求重新登录
4. WHILE 用户会话处于活跃状态，系统 SHALL 根据用户角色限制可访问的资源
5. WHEN 管理员创建用户，系统 SHALL 分配角色并存储角色权限映射
6. WHEN 用户尝试访问未授权资源，系统 SHALL 返回 403 状态码
7. IF 启用 HTTPS，系统 SHALL 使用 TLS 1.2 或更高版本加密通信

### 需求 5：配置热更新

**用户故事：** 作为运维工程师，我想要动态更新 Engine 配置，以便在不重启服务的情况下调整流处理参数。

#### 验收标准

1. WHEN 管理员提交配置更新请求，系统 SHALL 验证配置格式合法性
2. WHEN 配置验证通过，系统 SHALL 调用 Engine updateConfig 接口应用新配置
3. WHEN 配置更新成功，系统 SHALL 返回 200 状态码和新配置版本号
4. WHILE 配置更新过程中，系统 SHALL 保持现有 Graph 正常运行
5. IF 配置更新失败，系统 SHALL 回滚至上一版本配置并返回错误信息
6. WHEN 管理员查询配置历史，系统 SHALL 返回配置版本列表
7. WHEN 管理员请求回滚配置，系统 SHALL 恢复至指定版本的配置

### 需求 6：ROI 绘制与交互

**用户故事：** 作为算法工程师，我想要在视频画面上绘制 ROI 区域，以便限定算法检测范围。

#### 验收标准

1. WHEN 管理员在任务详情页面点击 ROI 绘制按钮，系统 SHALL 展示视频画面 Canvas
2. WHEN 管理员在 Canvas 上绘制矩形区域，系统 SHALL 记录矩形的左上角坐标和宽高
3. WHEN 管理员保存 ROI 配置，系统 SHALL 将坐标数据发送至后端并更新至对应 Element
4. WHILE ROI 区域已配置，系统 SHALL 在视频画面上叠加显示 ROI 区域标识
5. WHEN 管理员修改 ROI 区域，系统 SHALL 实时更新后端配置
6. IF 视频画面尺寸变化，系统 SHALL 按比例缩放 ROI 坐标

### 需求 7：任务调度与 DAG 编排

**用户故事：** 作为系统管理员，我想要可视化编排任务 DAG 和配置定时任务，以便灵活管理视频分析流程。

#### 验收标准

1. WHEN 管理员创建任务，系统 SHALL 展示 DAG 编排界面
2. WHEN 管理员在 DAG 界面添加节点，系统 SHALL 将节点添加至有向无环图
3. WHEN 管理员连接两个节点，系统 SHALL 验证连接不形成环路
4. WHEN 管理员保存 DAG 配置，系统 SHALL 将图结构持久化并生成 sophon-stream Graph 配置
5. WHEN 管理员配置定时任务，系统 SHALL 注册定时调度器并按时触发任务
6. WHILE 任务处于运行状态，系统 SHALL 支持暂停、恢复、停止操作
7. IF 任务执行失败，系统 SHALL 记录错误日志并发送告警通知

### 需求 8：资源监控

**用户故事：** 作为运维工程师，我想要实时监控 TPU/CPU/内存使用情况，以便及时发现系统瓶颈。

#### 验收标准

1. WHEN 系统启动监控采集器，系统 SHALL 每 5 秒采集一次 TPU/CPU/内存指标
2. WHEN 指标数据更新，系统 SHALL 通过 WebSocket 推送至前端
3. WHEN 前端收到 WebSocket 数据，系统 SHALL 更新实时图表
4. WHEN TPU 使用率超过 85%，系统 SHALL 触发资源告警
5. WHEN CPU 使用率超过 90%，系统 SHALL 触发资源告警
6. WHEN 内存使用率超过 85%，系统 SHALL 触发资源告警
7. WHILE 告警处于活跃状态，系统 SHALL 在监控面板高亮显示告警指标

### 需求 9：报警规则引擎

**用户故事：** 作为业务分析师，我想要配置报警规则和防抖动策略，以便减少误报并留存证据。

#### 验收标准

1. WHEN 管理员创建报警规则，系统 SHALL 存储规则配置至数据库
2. WHEN 报警条件满足，系统 SHALL 触发报警事件并记录时间戳
3. IF 报警规则配置防抖动时间，系统 SHALL 在防抖动窗口内抑制重复报警
4. WHEN 报警触发，系统 SHALL 保存报警时的视频帧截图作为证据
5. WHEN 报警触发，系统 SHALL 发送通知至配置的接收渠道
6. WHEN 管理员查询报警历史，系统 SHALL 返回报警事件列表及证据文件
7. IF 报警规则已禁用，系统 SHALL 停止该规则的评估

### 需求 10：上下行通信集成

**用户故事：** 作为系统集成工程师，我想要集成 MQTT/HTTP/GB28181 协议栈，以便实现与上级平台的通信。

#### 验收标准

1. WHEN 系统配置 MQTT Broker，系统 SHALL 建立 MQTT 连接并发布设备状态
2. WHEN 上级平台通过 HTTP 查询设备状态，系统 SHALL 返回 JSON 格式响应
3. WHEN 系统注册至 GB28181 平台，系统 SHALL 完成 SIP 注册流程
4. WHEN 上级平台请求视频流，系统 SHALL 通过 GB28181 协议推送流媒体
5. IF MQTT 连接断开，系统 SHALL 自动重连并重发未确认消息
6. WHEN 收到上级平台控制指令，系统 SHALL 解析指令并执行对应操作
7. WHILE 系统处于离线状态，系统 SHALL 缓存数据并在恢复后同步

### 需求 11：系统升级

**用户故事：** 作为运维工程师，我想要支持固件/模型/程序包的差分升级和回滚，以便安全地更新系统。

#### 验收标准

1. WHEN 管理员上传升级包，系统 SHALL 验证升级包的数字签名
2. WHEN 升级包验证通过，系统 SHALL 计算差分并应用更新
3. WHEN 升级完成，系统 SHALL 重启相关服务并验证升级结果
4. IF 升级后验证失败，系统 SHALL 自动回滚至上一版本
5. WHEN 管理员手动触发回滚，系统 SHALL 恢复至指定版本
6. WHEN 系统记录升级历史，系统 SHALL 存储升级时间、版本和结果
7. IF 升级过程中断，系统 SHALL 保持当前版本并标记升级失败

### 需求 12：插件市场

**用户故事：** 作为算法开发者，我想要注册和加载动态插件，以便扩展系统功能。

#### 验收标准

1. WHEN 管理员安装插件，系统 SHALL 验证插件数字签名
2. WHEN 插件验证通过，系统 SHALL 将插件注册表添加至插件注册中心
3. WHEN 系统加载插件，系统 SHALL 使用 dlopen 加载动态 .so 文件
4. IF 插件加载失败，系统 SHALL 记录错误日志并跳过该插件
5. WHEN 管理员查询可用插件，系统 SHALL 返回插件注册表列表
6. WHEN 管理员卸载插件，系统 SHALL 从运行环境卸载并更新注册表
7. IF 插件版本不兼容，系统 SHALL 拒绝加载并返回版本错误信息

### 需求 13：sophon-stream 集成

**用户故事：** 作为系统架构师，我想要完整集成 sophon-stream 框架，以便利用其流水线架构和硬件加速能力。

#### 验收标准

1. WHEN 系统启动，系统 SHALL 初始化 sophon-stream Engine 并加载默认配置
2. WHEN 创建视频分析任务，系统 SHALL 构建 sophon-stream Graph 并配置 Element 链
3. WHEN 任务启动，系统 SHALL 启动 Graph 并监控运行状态
4. WHILE 任务运行中，系统 SHALL 通过 sophon-stream API 获取处理结果
5. IF 任务运行异常，系统 SHALL 停止 Graph 并记录错误日志
6. WHEN 系统调用解码器，系统 SHALL 使用 VPU 硬件解码 H.264/H.265 流
7. WHEN 系统调用推理器，系统 SHALL 使用 TPU 执行 YOLOX/YOLOv5 等模型推理
8. WHEN 系统调用跟踪器，系统 SHALL 使用 ByteTrack 算法跟踪目标
9. IF 多路视频流同时处理，系统 SHALL 并行运行多个 Graph 实例

### 需求 14：日志系统

**用户故事：** 作为运维工程师，我想要完整的日志记录能力，以便排查问题和审计操作。

#### 验收标准

1. WHEN 系统启动，系统 SHALL 初始化日志系统并配置输出目标
2. WHEN 记录日志，系统 SHALL 包含时间戳、级别、模块和消息
3. WHILE 日志级别为 DEBUG，系统 SHALL 输出详细调试信息
4. WHEN 日志文件达到 100MB，系统 SHALL 轮转日志文件
5. WHEN 系统保留最近 7 天的日志，系统 SHALL 自动清理过期日志
6. IF 日志写入失败，系统 SHALL 输出至控制台作为降级方案
7. WHEN 管理员查询操作日志，系统 SHALL 返回用户操作记录

### 需求 15：性能与资源占用

**用户故事：** 作为系统架构师，我想要控制系统的资源占用，以便在边缘设备上高效运行。

#### 验收标准

1. WHEN 系统空闲运行，系统 SHALL 占用内存不超过 512MB
2. WHEN 系统处理 4 路 1080P 视频流，系统 SHALL 占用 TPU 不超过 80%
3. WHEN 系统处理 8 路 1080P 视频流，系统 SHALL 占用 CPU 不超过 60%
4. WHILE 系统运行，系统 SHALL 保持磁盘占用不超过 2GB
5. IF 系统资源使用超过阈值，系统 SHALL 触发降级策略
6. WHEN 系统启动，系统 SHALL 在 10 秒内完成初始化并就绪
