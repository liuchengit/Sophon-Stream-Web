# 部署文档

## 硬件要求

- **处理器**: Sophgo BM1684/BM1684X/BM1688
- **内存**: 最少 4GB RAM
- **存储**: 最少 4GB 可用空间
- **网络**: 以太网连接

## 环境准备

### 系统依赖

```bash
# Ubuntu/Debian
apt-get install -y build-essential cmake libsqlite3-dev libssl-dev

# Drogon Web 框架
# 参考 https://github.com/drogonframework/drogon 安装

# Node.js 18+
apt-get install -y nodejs npm
```

### 依赖库

| 库 | 用途 | 安装方式 |
|---|------|----------|
| Drogon | Web 框架 | 源码编译 |
| SQLite3 | 数据库 | apt install |
| OpenSSL | TLS/加密 | apt install |
| nlohmann/json | JSON 解析 | 头文件库 |

## 编译部署

### 1. 克隆仓库

```bash
git clone <repository-url>
cd sophon-stream-web
```

### 2. 构建项目

```bash
make build
```

或分步构建：

```bash
# 构建前端
cd frontend && npm install && npm run build && cd ..

# 构建后端
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

### 3. 部署

```bash
# 部署到默认目录 /opt/sophon-stream-web
./scripts/deploy.sh

# 部署到指定目录
./scripts/deploy.sh /custom/path
```

### 4. 启动服务

```bash
cd /opt/sophon-stream-web
./bin/start.sh
```

## 配置说明

### 后端配置 (config.json)

```json
{
    "listeners": [{"address": "0.0.0.0", "port": 8080}],
    "app": {
        "document_root": "./www",
        "home_page": "index.html",
        "log": {
            "log_path": "./logs",
            "log_level": "INFO"
        }
    }
}
```

### 引擎配置 (configs/default.json)

```json
{
    "engine": {
        "maxGraphs": 8,
        "maxStreamsPerGraph": 4,
        "enableTpu": true
    }
}
```

## 访问方式

- **Web 界面**: http://<IP>:8080
- **API 接口**: http://<IP>:8080/api/v1
- **WebSocket**: ws://<IP>:8080/ws/monitoring
- **默认管理员**: admin / admin123

## 运维操作

### 查看日志

```bash
tail -f /opt/sophon-stream-web/logs/sophon-web.log
```

### 数据库备份

```bash
cp /opt/sophon-stream-web/data/sophon-web.db /backup/sophon-web-$(date +%Y%m%d).db
```

### 服务重启

```bash
pkill sophon-stream-web
cd /opt/sophon-stream-web && ./bin/start.sh
```

## 故障排查

| 问题 | 解决方案 |
|------|----------|
| 端口被占用 | 修改 config.json 中的 port |
| 数据库无法打开 | 检查 data/ 目录权限 |
| 前端 404 | 确认前端构建文件已拷贝到 www/ |
| TPU 不可用 | 检查 sophon-stream 驱动是否加载 |
