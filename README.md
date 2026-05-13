# Sophon-Stream Web 管理系统

基于 sophon-stream 流处理框架的边缘 AI 视频分析管理平台。

## 功能特性

- **设备管理**：视频设备接入、状态监控、配置管理
- **任务管理**：视频分析任务创建、启停控制、DAG 编排
- **算法管理**：算法插件安装、配置、版本管理
- **实时监控**：TPU/CPU/内存使用率实时图表
- **报警管理**：报警规则配置、防抖动、证据留存
- **系统设置**：配置热更新、系统升级、插件市场

## 技术栈

- **前端**：Vue3 + TypeScript + Element Plus + ECharts + Vue Flow
- **后端**：C++17 + Drogon Web 框架 + SQLite
- **引擎**：sophon-stream 流处理框架
- **适用硬件**：Sophgo BM1684/BM1684X/BM1688

## 快速开始

### 构建

```bash
# 完整构建
make build

# 或分步构建
cd frontend && npm install && npm run build
cd ../backend && mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release && make -j$(nproc)
```

### 运行

```bash
# 启动后端服务
./scripts/start.sh

# 或开发模式（前端热更新）
make dev
```

### 访问

- 前端：http://localhost:3000
- 后端 API：http://localhost:8080
- 默认管理员：admin / admin123

## 项目结构

```
├── frontend/           # Vue3 前端
├── backend/            # Drogon C++ 后端
├── engine/             # sophon-stream 引擎集成
├── plugins/            # 插件系统
├── scripts/            # 构建和部署脚本
├── docs/               # 文档
└── .monkeycode/        # 项目规范和文档
```

## API 文档

| 端点 | 方法 | 描述 |
|------|------|------|
| /api/v1/auth/login | POST | 用户登录 |
| /api/v1/devices | GET/POST | 设备管理 |
| /api/v1/tasks | GET/POST | 任务管理 |
| /api/v1/tasks/{id}/start | POST | 启动任务 |
| /api/v1/tasks/{id}/stop | POST | 停止任务 |
| /api/v1/monitoring/resources | GET | 资源监控 |
| /ws/monitoring | WebSocket | 实时数据推送 |

## 许可证

MIT
