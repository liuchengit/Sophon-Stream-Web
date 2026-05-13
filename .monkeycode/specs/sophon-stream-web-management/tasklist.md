# Sophon-Stream Web 管理系统实施计划

- [x] 1. 设置项目结构和构建系统
   - 创建前端 frontend/ 目录结构（Vue3 + TypeScript + Vite）
   - 创建后端 backend/ 目录结构（Drogon C++）
   - 创建引擎集成 engine/ 目录结构
   - 创建插件系统 plugins/ 目录结构
   - 配置根 CMakeLists.txt 整合后端和引擎构建
   - 配置 frontend/vite.config.ts 添加 allowedHosts: ['.monkeycode-ai.online'] 和 API 代理
   - 设置 .gitignore 排除构建产物
   - 创建 Makefile 提供便捷构建命令
   - 配置测试框架（Google Test 用于 C++，Vitest 用于前端）

- [x] 2. 实现数据库层和 ORM
   - [x] 2.1 创建 SQLite 数据库管理器
     - 编写 DatabaseManager 类实现数据库连接和初始化
     - 实现数据库迁移脚本创建所有表结构
     - 添加数据库备份和恢复功能

   - [x] 2.2 实现数据模型类
     - 定义 User、Role、Permission、Device、Task、Algorithm、AlarmRule、AlarmEvent、MonitoringMetric、ConfigVersion、Plugin、UpgradeRecord 模型类
     - 使用 sqlite_orm 定义表映射和关系
     - 为每个模型实现 CRUD 仓储方法

   - [x] 2.3 实现数据库事务管理
     - 编写事务封装确保原子操作
     - 实现连接池管理（如需要）

    - [x] 2.4 为数据库层编写单元测试
      - 为每个模型 CRUD 操作创建单元测试
      - 测试事务回滚和异常处理
      - 测试数据库初始化和迁移

    - [x] 3.4 为认证授权编写测试
      - 测试 JWT 签发和验证
      - 测试密码哈希和验证
      - 测试 RBAC 权限检查逻辑
      - 测试未授权访问拦截

- [x] 4. 实现 sophon-stream 引擎集成层
   - [x] 4.1 创建 StreamEngine 封装类
     - 编写 StreamEngine 类封装 sophon-stream Engine 生命周期
     - 实现 Engine 初始化和配置加载
     - 实现 Engine::updateConfig() 热更新接口

   - [x] 4.2 创建 GraphManager 管理类
     - 编写 GraphManager 实现 Graph 实例创建/销毁
     - 实现多 Graph 并行处理管理
     - 实现 Graph 状态监控和异常处理

   - [x] 4.3 创建 ElementRegistry 注册器
     - 编写 ElementRegistry 管理 Element 注册和配置
     - 实现 JSON 配置文件解析和验证
     - 实现 ROI 坐标同步接口

   - [x] 4.4 创建 ResultCollector 结果收集器
     - 编写 ResultCollector 通过回调机制收集处理结果
     - 实现结果数据格式化和存储
     - 实现报警条件触发检测

   - [x] 4.5 创建 ConfigHotUpdater 热更新器
     - 编写配置版本管理机制
     - 实现配置验证和回滚逻辑
     - 实现配置变更通知机制

- [x] 5. 实现后端 RESTful API 服务
   - [x] 5.1 配置 Drogon Web 服务器
     - 配置 Drogon config.json 设置端口和静态文件路径
     - 注册所有 Controller 路由
     - 配置 CORS 和静态文件服务
     - 添加 allowedHosts 配置

   - [x] 5.2 实现设备管理 API
     - 实现 DeviceController CRUD 接口
     - 实现设备状态查询和更新
     - 实现设备在线/离线检测

   - [x] 5.3 实现任务管理 API
     - 实现 TaskController CRUD 接口
     - 实现任务启动/停止/暂停/恢复操作
     - 实现任务与 Graph 实例的绑定

   - [x] 5.4 实现算法管理 API
     - 实现 AlgorithmController CRUD 接口
     - 实现算法插件上传和安装
     - 实现算法配置参数管理

   - [x] 5.5 实现监控数据 API
     - 实现 MonitorController 资源数据查询
     - 实现 TPU/CPU/内存指标采集
     - 实现 WebSocket 实时推送 Handler

   - [x] 5.6 实现报警管理 API
     - 实现 AlarmController 报警规则 CRUD
     - 实现报警事件查询和证据下载
     - 实现报警规则引擎和防抖动逻辑

   - [x] 5.7 实现配置管理 API
     - 实现 ConfigController 配置查询和更新
     - 实现配置版本历史查询
     - 实现配置回滚操作

   - [x] 5.8 实现系统升级 API
     - 实现 UpgradeController 升级包上传和验证
     - 实现差分升级和回滚逻辑
     - 实现升级历史记录查询

   - [x] 5.9 实现插件管理 API
     - 实现 PluginController 插件 CRUD
     - 实现插件签名验证和动态加载
     - 实现插件注册和卸载

    - [x] 5.10 为 API 编写集成测试
      - 测试所有 RESTful 端点
      - 测试错误码和异常处理
      - 测试认证和权限拦截

    - [x] 6.13 为前端组件编写测试
      - 测试主要组件渲染
      - 测试表单验证逻辑
      - 测试 API 调用 mock

- [x] 7. 实现外部通信集成
   - [x] 7.1 实现 MQTT 通信
     - 集成 MQTT 客户端库
     - 实现 MQTT Broker 连接和重连
     - 实现设备状态发布和指令订阅

   - [x] 7.2 实现 HTTP 上级平台接口
     - 实现 HTTP 查询响应接口
     - 实现设备状态和设备列表 API

   - [x] 7.3 实现 GB28181 协议栈
     - 实现 SIP 注册流程
     - 实现流媒体推送接口
     - 实现上级平台控制指令解析

- [x] 8. 实现日志系统
   - [x] 8.1 集成 spdlog 日志库
     - 配置多输出目标（控制台 + 文件）
     - 实现日志轮转（100MB 限制）
     - 实现日志级别动态配置

   - [x] 8.2 实现操作审计日志
     - 记录所有用户操作到数据库
     - 实现操作日志查询 API

   - [x] 8.3 实现系统运行日志
     - 记录 Engine/Graph/Element 运行日志
     - 记录错误和异常日志

- [x] 9. 实现资源监控采集器
   - [x] 9.1 实现系统资源采集
     - 读取 /proc/stat 获取 CPU 使用率
     - 读取 /proc/meminfo 获取内存使用率
     - 调用 Sophgo BM 接口获取 TPU 使用率

   - [x] 9.2 实现定时采集调度
     - 实现 5 秒定时采集循环
     - 实现指标数据持久化

   - [x] 9.3 实现资源告警检测
     - 检测 TPU/CPU/内存阈值
     - 触发资源告警事件

- [x] 10. 实现插件系统
   - [x] 10.1 实现 PluginLoader 动态加载器
     - 使用 dlopen/dlsym 加载 .so 文件
     - 实现插件接口定义和调用
     - 实现插件卸载和清理

   - [x] 10.2 实现 PluginRegistry 注册中心
     - 维护插件注册表
     - 实现插件版本兼容性检查
     - 实现插件查询和状态管理

   - [x] 10.3 实现 PluginValidator 验证器
     - 实现插件数字签名验证
     - 实现插件格式和接口验证

- [x] 11. 实现系统升级管理
   - [x] 11.1 实现 UpgradeManager 升级管理器
     - 实现升级包上传和存储
     - 实现升级包签名验证
     - 实现差分升级逻辑（xdelta3）

   - [x] 11.2 实现回滚机制
     - 实现升级失败自动回滚
     - 实现手动回滚到指定版本
     - 实现版本快照管理

- [x] 12. 集成测试和性能优化
   - [x] 12.1 端到端集成测试
     - 测试前端到后端的完整流程
     - 测试 sophon-stream Engine 集成
     - 测试 WebSocket 实时推送

- [x] 13. 部署配置和文档
   - [x] 13.1 编写部署脚本
     - 创建 build.sh 构建脚本
     - 创建 deploy.sh 部署脚本
     - 创建 start.sh 启动脚本

   - [x] 13.2 编写文档
     - 编写 API 文档（OpenAPI/Swagger）
     - 编写部署文档
     - 编写架构文档
     - 编写用户操作手册
