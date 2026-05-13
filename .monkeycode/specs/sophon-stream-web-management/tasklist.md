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

   - [ ]* 2.4 为数据库层编写单元测试
     - 为每个模型 CRUD 操作创建单元测试
     - 测试事务回滚和异常处理
     - 测试数据库初始化和迁移

- [x] 3. 实现认证和授权系统
   - [x] 3.1 实现 JWT 认证
     - 集成 jwt-cpp 库实现 Token 签发
     - 编写 AuthController 处理登录/注册请求
     - 实现 Token 验证中间件
     - 实现密码 bcrypt 哈希存储

   - [x] 3.2 实现 RBAC 权限模型
     - 编写 RBACMiddleware 检查请求权限
     - 实现角色-权限-资源三级控制逻辑
     - 添加权限缓存机制提升性能

   - [ ] 3.3 创建默认管理员账户
     - 在数据库初始化时创建 admin 角色
     - 创建默认管理员用户（admin/admin123）
     - 初始化系统基础权限

   - [ ]* 3.4 为认证授权编写测试
     - 测试 JWT 签发和验证
     - 测试密码哈希和验证
     - 测试 RBAC 权限检查逻辑
     - 测试未授权访问拦截

- [ ] 4. 实现 sophon-stream 引擎集成层
   - [ ] 4.1 创建 StreamEngine 封装类
     - 编写 StreamEngine 类封装 sophon-stream Engine 生命周期
     - 实现 Engine 初始化和配置加载
     - 实现 Engine::updateConfig() 热更新接口

   - [ ] 4.2 创建 GraphManager 管理类
     - 编写 GraphManager 实现 Graph 实例创建/销毁
     - 实现多 Graph 并行处理管理
     - 实现 Graph 状态监控和异常处理

   - [ ] 4.3 创建 ElementRegistry 注册器
     - 编写 ElementRegistry 管理 Element 注册和配置
     - 实现 JSON 配置文件解析和验证
     - 实现 ROI 坐标同步接口

   - [ ] 4.4 创建 ResultCollector 结果收集器
     - 编写 ResultCollector 通过回调机制收集处理结果
     - 实现结果数据格式化和存储
     - 实现报警条件触发检测

   - [ ] 4.5 创建 ConfigHotUpdater 热更新器
     - 编写配置版本管理机制
     - 实现配置验证和回滚逻辑
     - 实现配置变更通知机制

- [ ] 5. 实现后端 RESTful API 服务
   - [ ] 5.1 配置 Drogon Web 服务器
     - 配置 Drogon config.json 设置端口和静态文件路径
     - 注册所有 Controller 路由
     - 配置 CORS 和静态文件服务
     - 添加 allowedHosts 配置

   - [ ] 5.2 实现设备管理 API
     - 实现 DeviceController CRUD 接口
     - 实现设备状态查询和更新
     - 实现设备在线/离线检测

   - [ ] 5.3 实现任务管理 API
     - 实现 TaskController CRUD 接口
     - 实现任务启动/停止/暂停/恢复操作
     - 实现任务与 Graph 实例的绑定

   - [ ] 5.4 实现算法管理 API
     - 实现 AlgorithmController CRUD 接口
     - 实现算法插件上传和安装
     - 实现算法配置参数管理

   - [ ] 5.5 实现监控数据 API
     - 实现 MonitorController 资源数据查询
     - 实现 TPU/CPU/内存指标采集
     - 实现 WebSocket 实时推送 Handler

   - [ ] 5.6 实现报警管理 API
     - 实现 AlarmController 报警规则 CRUD
     - 实现报警事件查询和证据下载
     - 实现报警规则引擎和防抖动逻辑

   - [ ] 5.7 实现配置管理 API
     - 实现 ConfigController 配置查询和更新
     - 实现配置版本历史查询
     - 实现配置回滚操作

   - [ ] 5.8 实现系统升级 API
     - 实现 UpgradeController 升级包上传和验证
     - 实现差分升级和回滚逻辑
     - 实现升级历史记录查询

   - [ ] 5.9 实现插件管理 API
     - 实现 PluginController 插件 CRUD
     - 实现插件签名验证和动态加载
     - 实现插件注册和卸载

   - [ ]* 5.10 为 API 编写集成测试
     - 测试所有 RESTful 端点
     - 测试错误码和异常处理
     - 测试认证和权限拦截

- [ ] 6. 实现前端 Web 界面 (Vue3)
   - [ ] 6.1 配置 Vue3 项目
     - 配置 vite.config.ts 设置 API 代理到后端 8080 端口
     - 安装 Element Plus、ECharts、Vue Flow、Pinia 依赖
     - 配置 Vue Router 路由和导航守卫

   - [ ] 6.2 实现布局组件
     - 创建主布局（侧边栏 + 头部 + 内容区）
     - 实现路由驱动的侧边栏导航
     - 创建登录页面布局

   - [ ] 6.3 实现认证相关页面
     - 创建登录页面（用户名/密码表单）
     - 实现登录逻辑和 Token 存储
     - 实现路由守卫拦截未认证访问

   - [ ] 6.4 实现仪表盘页面
     - 创建 Dashboard 页面展示系统概览
     - 集成 ECharts 展示资源使用率图表
     - 实现 WebSocket 连接接收实时数据
     - 显示设备统计、任务统计、报警统计

   - [ ] 6.5 实现设备管理页面
     - 创建设备列表页面（表格 + 筛选）
     - 实现设备添加/编辑/删除对话框
     - 实现设备状态实时显示

   - [ ] 6.6 实现任务管理页面
     - 创建任务列表页面（表格 + 状态标签）
     - 实现任务创建/编辑/删除功能
     - 实现任务启动/停止/暂停操作按钮

   - [ ] 6.7 实现任务编排页面 (DAG)
     - 集成 Vue Flow 实现 DAG 可视化编排
     - 实现节点添加和连接功能
     - 实现环路检测验证
     - 实现 DAG 配置保存和加载

   - [ ] 6.8 实现 ROI 绘制组件
     - 创建 ROICanvas 组件支持矩形绘制
     - 实现坐标计算和缩放适配
     - 实现 ROI 配置保存和同步

   - [ ] 6.9 实现算法管理页面
     - 创建算法列表页面
     - 实现算法详情查看和配置编辑
     - 实现算法安装/卸载操作

   - [ ] 6.10 实现监控面板页面
     - 创建实时监控面板页面
     - 集成 ECharts 实时图表（TPU/CPU/内存）
     - 实现 WebSocket 数据订阅和更新

   - [ ] 6.11 实现报警管理页面
     - 创建报警规则配置页面
     - 创建报警事件列表页面
     - 实现证据图片查看功能
     - 实现报警规则启用/禁用开关

   - [ ] 6.12 实现系统设置页面
     - 创建系统配置表单页面
     - 实现配置参数保存
     - 创建升级管理页面
     - 创建插件市场页面

   - [ ]* 6.13 为前端组件编写测试
     - 测试主要组件渲染
     - 测试表单验证逻辑
     - 测试 API 调用 mock

- [ ] 7. 实现外部通信集成
   - [ ] 7.1 实现 MQTT 通信
     - 集成 MQTT 客户端库
     - 实现 MQTT Broker 连接和重连
     - 实现设备状态发布和指令订阅

   - [ ] 7.2 实现 HTTP 上级平台接口
     - 实现 HTTP 查询响应接口
     - 实现设备状态和设备列表 API

   - [ ] 7.3 实现 GB28181 协议栈
     - 实现 SIP 注册流程
     - 实现流媒体推送接口
     - 实现上级平台控制指令解析

- [ ] 8. 实现日志系统
   - [ ] 8.1 集成 spdlog 日志库
     - 配置多输出目标（控制台 + 文件）
     - 实现日志轮转（100MB 限制）
     - 实现日志级别动态配置

   - [ ] 8.2 实现操作审计日志
     - 记录所有用户操作到数据库
     - 实现操作日志查询 API

   - [ ] 8.3 实现系统运行日志
     - 记录 Engine/Graph/Element 运行日志
     - 记录错误和异常日志

- [ ] 9. 实现资源监控采集器
   - [ ] 9.1 实现系统资源采集
     - 读取 /proc/stat 获取 CPU 使用率
     - 读取 /proc/meminfo 获取内存使用率
     - 调用 Sophgo BM 接口获取 TPU 使用率

   - [ ] 9.2 实现定时采集调度
     - 实现 5 秒定时采集循环
     - 实现指标数据持久化

   - [ ] 9.3 实现资源告警检测
     - 检测 TPU/CPU/内存阈值
     - 触发资源告警事件

- [ ] 10. 实现插件系统
   - [ ] 10.1 实现 PluginLoader 动态加载器
     - 使用 dlopen/dlsym 加载 .so 文件
     - 实现插件接口定义和调用
     - 实现插件卸载和清理

   - [ ] 10.2 实现 PluginRegistry 注册中心
     - 维护插件注册表
     - 实现插件版本兼容性检查
     - 实现插件查询和状态管理

   - [ ] 10.3 实现 PluginValidator 验证器
     - 实现插件数字签名验证
     - 实现插件格式和接口验证

   - [ ] 10.4 创建示例插件
     - 创建 YOLOX 检测算法示例插件
     - 创建自定义 Element 示例插件

- [ ] 11. 实现系统升级管理
   - [ ] 11.1 实现 UpgradeManager 升级管理器
     - 实现升级包上传和存储
     - 实现升级包签名验证
     - 实现差分升级逻辑（xdelta3）

   - [ ] 11.2 实现回滚机制
     - 实现升级失败自动回滚
     - 实现手动回滚到指定版本
     - 实现版本快照管理

   - [ ] 11.3 实现升级流程编排
     - 实现升级前检查和准备
     - 实现升级中进度跟踪
     - 实现升级后验证

- [ ] 12. 集成测试和性能优化
   - [ ] 12.1 端到端集成测试
     - 测试前端到后端的完整流程
     - 测试 sophon-stream Engine 集成
     - 测试 WebSocket 实时推送

   - [ ] 12.2 API 性能测试
     - 测试 API 响应时间（< 200ms）
     - 测试并发连接数（> 100）

   - [ ] 12.3 资源占用优化
     - 优化内存占用（< 512MB 空闲）
     - 优化磁盘占用（< 2GB）
     - 验证 TPU 使用率（< 80% 4路）

- [ ] 13. 部署配置和文档
   - [ ] 13.1 编写部署脚本
     - 创建 build.sh 构建脚本
     - 创建 deploy.sh 部署脚本
     - 创建 start.sh 启动脚本

   - [ ] 13.2 编写文档
     - 编写 API 文档（OpenAPI/Swagger）
     - 编写部署文档
     - 编写架构文档
     - 编写用户操作手册
