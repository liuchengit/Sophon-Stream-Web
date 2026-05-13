# Sophon-Stream Web API 文档

## 基础信息

- **Base URL**: `http://localhost:8080/api/v1`
- **认证方式**: Bearer Token (JWT)
- **内容类型**: `application/json`

## 认证

### POST /auth/login

用户登录获取 JWT Token。

**请求体:**
```json
{
  "username": "admin",
  "password": "admin123"
}
```

**响应:**
```json
{
  "code": 0,
  "message": "success",
  "data": {
    "token": "eyJhbGc...",
    "user": {
      "id": 1,
      "username": "admin",
      "role": "admin"
    }
  }
}
```

### POST /auth/logout

退出登录。

## 设备管理

### GET /devices

获取设备列表。

**查询参数:**
| 参数 | 类型 | 说明 |
|------|------|------|
| status | string | 过滤状态 (online/offline/error) |
| type | string | 过滤类型 |
| page | int | 页码 (默认 1) |
| limit | int | 每页数量 (默认 20) |

### POST /devices

创建设备。

**请求体:**
```json
{
  "name": "Camera-01",
  "type": "camera",
  "ip_address": "192.168.1.100",
  "port": 554,
  "model": "HIKVISION-DS-2CD"
}
```

### GET /devices/{id}

获取设备详情。

### PUT /devices/{id}

更新设备信息。

### DELETE /devices/{id}

删除设备。

## 任务管理

### GET /tasks

获取任务列表。

### POST /tasks

创建任务。

**请求体:**
```json
{
  "name": "Entrance Detection",
  "description": "Main entrance video analysis",
  "device_id": 1,
  "graph_config": "{\"elements\":[\"decoder\",\"detector\",\"tracker\"]}",
  "schedule_cron": "0 8 * * 1-5"
}
```

### POST /tasks/{id}/start

启动任务。

### POST /tasks/{id}/stop

停止任务。

### POST /tasks/{id}/pause

暂停任务。

### POST /tasks/{id}/resume

恢复任务。

### PUT /tasks/{id}/config

热更新任务配置。

## 监控数据

### GET /monitoring/resources

获取当前资源使用率。

**响应:**
```json
{
  "code": 0,
  "data": {
    "tpu_usage": 45.2,
    "cpu_usage": 32.1,
    "memory_usage": 65.8,
    "memory_total": 8192,
    "memory_used": 5390
  }
}
```

### WebSocket /ws/monitoring

实时资源数据推送（每 5 秒）。

## 报警管理

### GET /alarms/rules

获取报警规则列表。

### POST /alarms/rules

创建报警规则。

**请求体:**
```json
{
  "name": "High Confidence Alert",
  "condition_expr": "confidence>0.9",
  "debounce_seconds": 30,
  "notification_channels": "webhook,email",
  "enabled": true
}
```

### PUT /alarms/rules/{id}

更新报警规则。

### DELETE /alarms/rules/{id}

删除报警规则。

### GET /alarms/events

获取报警事件历史。

## 配置管理

### GET /config/{key}

获取配置值。

### PUT /config/{key}

更新配置（触发热更新）。

### GET /config/{key}/history

获取配置历史版本。

### POST /config/{key}/rollback

回滚到指定版本。

## 错误码

| 错误码 | 说明 |
|--------|------|
| 0 | 成功 |
| 400 | 请求参数错误 |
| 401 | 未认证/Token 无效 |
| 403 | 权限不足 |
| 404 | 资源不存在 |
| 409 | 资源冲突 |
| 500 | 服务器内部错误 |
| 1001 | 设备离线 |
| 1002 | 任务已在运行 |
| 1003 | Graph 创建失败 |
| 1004 | 配置更新失败 |
| 1005 | 插件加载失败 |
| 1006 | 升级失败 |
| 1007 | ROI 坐标超出范围 |
| 1008 | DAG 存在环路 |
