# GB28181 报警管理模块使用文档

## 1. 概述

本报警管理模块严格遵循 GB/T 28181-2016《安全防范视频监控联网系统信息传输、交换、控制技术要求》国家标准，实现了报警事件的订阅、上报、存储和处置全流程管理。

### 1.1 标准符合性

- **报警订阅（SUBSCRIBE）**: 平台向设备发起报警订阅（布防），指定需要上报的报警类型
- **报警上报（NOTIFY）**: 设备检测到报警事件时，通过 NOTIFY 请求将警情推送给平台
- **报警类型**: 支持 GB28181 标准定义的全部报警类型代码
- **SIP 事务**: 支持 SIP 事务 ID 跟踪和管理

## 2. 报警类型代码

### 2.1 AlarmMethod=5（视频报警）

| 代码 | 报警类型 | 说明 |
|------|---------|------|
| 1 | 人工视频报警 | 人工触发的视频报警 |
| 2 | 运动目标检测报警 | 移动侦测报警 |
| 3 | 遗留物检测报警 | 区域内遗留物体检测 |
| 4 | 物体移除检测报警 | 区域内物体被移除 |
| 5 | 绊线检测报警 | 越界侦测 |
| 6 | 入侵检测报警 | 区域入侵侦测 |
| 7 | 逆行检测报警 | 逆行侦测 |
| 8 | 徘徊检测报警 | 徘徊侦测 |
| 9 | 流量统计报警 | 人/车流量统计 |
| 10 | 密度检测报警 | 人群密度检测 |
| 11 | 视频异常检测报警 | 视频质量异常 |
| 12 | 快速移动报警 | 快速移动侦测 |

### 2.2 AlarmMethod=6（设备报警）

| 代码 | 报警类型 | 说明 |
|------|---------|------|
| 1 | 存储设备磁盘故障 | 硬盘故障报警 |
| 2 | 存储设备风扇故障 | 风扇故障报警 |
| 3 | 存储磁盘满报警 | 磁盘空间不足 |
| 4 | 设备高温报警 | 设备温度过高 |
| 5 | 设备低温报警 | 设备温度过低 |

### 2.3 AlarmMethod=3（其他报警）

| 代码 | 报警类型 | 说明 |
|------|---------|------|
| 1 | 防拆报警 | 设备被拆卸 |

## 3. API 接口说明

### 3.1 报警规则管理

#### 3.1.1 获取报警规则列表

```http
GET /api/v1/alarms/rules
```

**响应示例:**
```json
{
  "code": 0,
  "data": {
    "items": [
      {
        "id": 1,
        "name": "人员入侵检测",
        "condition_expr": "class==\"person\" AND confidence>0.8",
        "debounce_seconds": 5,
        "notification_channels": "webhook,email",
        "enabled": true,
        "gb_alarm_type": "invasion",
        "alarm_method": 5,
        "alarm_priority": "high",
        "subscribe_status": "subscribed",
        "subscribe_expires": "2026-05-15 12:00:00",
        "device_id": 1,
        "channel_id": 1,
        "alarm_description": "主要出入口区域入侵检测"
      }
    ]
  }
}
```

#### 3.1.2 创建报警规则

```http
POST /api/v1/alarms/rules
Content-Type: application/json

{
  "name": "周界防范报警",
  "condition_expr": "roi_violation==true",
  "debounce_seconds": 10,
  "notification_channels": "webhook",
  "enabled": true,
  "gb_alarm_type": "tripwire",
  "alarm_method": 5,
  "alarm_priority": "high",
  "device_id": 1,
  "channel_id": 1,
  "alarm_description": "园区周界绊线检测"
}
```

#### 3.1.3 更新报警规则

```http
PUT /api/v1/alarms/rules/{id}
Content-Type: application/json

{
  "name": "updated name",
  "alarm_priority": "medium",
  "alarm_description": "updated description"
}
```

#### 3.1.4 删除报警规则

```http
DELETE /api/v1/alarms/rules/{id}
```

#### 3.1.5 切换报警规则状态

```http
POST /api/v1/alarms/rules/{id}/toggle
```

#### 3.1.6 订阅报警规则（布防）

```http
POST /api/v1/alarms/rules/{id}/subscribe
```

**响应示例:**
```json
{
  "code": 0,
  "data": {
    "id": 1,
    "subscribe_status": "subscribed",
    "subscribe_expires": "2026-05-15 12:00:00"
  }
}
```

**说明:**
- 订阅成功后，有效期为 24 小时
- 平台需要在过期前重新订阅以维持布防状态
- 对应 GB28181 的 SUBSCRIBE 信令

#### 3.1.7 取消订阅报警规则（撤防）

```http
POST /api/v1/alarms/rules/{id}/unsubscribe
```

### 3.2 报警事件管理

#### 3.2.1 获取报警事件列表

```http
GET /api/v1/alarms/events?page=1&limit=20&status=pending
```

**查询参数:**
- `page`: 页码，默认 1
- `limit`: 每页数量，默认 20
- `status`: 处置状态过滤（pending/handled）

**响应示例:**
```json
{
  "code": 0,
  "data": {
    "items": [
      {
        "id": 1,
        "rule_id": 1,
        "task_id": 1,
        "evidence_path": "/evidences/alarm_001.jpg",
        "context": "{\"class\":\"person\",\"confidence\":0.92}",
        "triggered_at": "2026-05-14 11:31:38",
        "gb_alarm_code": "06",
        "alarm_priority": "high",
        "alarm_type": "invasion",
        "device_id": 1,
        "channel_id": 1,
        "sip_transaction_id": "z9hG4bK5111380649",
        "alarm_description": "人员入侵检测报警",
        "handled_status": "pending",
        "alarm_method": 5
      }
    ],
    "total": 1
  }
}
```

#### 3.2.2 处置报警事件

```http
POST /api/v1/alarms/events/{id}/handle
Content-Type: application/json

{
  "result": "已通知保安现场确认，误报"
}
```

**响应示例:**
```json
{
  "code": 0,
  "data": {
    "id": 1,
    "handled_status": "handled"
  }
}
```

## 4. 报警级别

| 级别 | 说明 | 使用场景 |
|------|------|---------|
| high | 高优先级 | 入侵检测、防拆报警等需要立即响应的警情 |
| medium | 中优先级 | 移动侦测、越界检测等常规警情 |
| low | 低优先级 | 设备状态类警情 |

## 5. 报警处置流程

```
┌─────────┐     ┌─────────┐     ┌─────────┐     ┌─────────┐
│  设备   │────>│  平台   │────>│ 数据库  │────>│  用户   │
│         │     │         │     │         │     │         │
│ 检测到  │     │ 接收并  │     │ 存储报  │     │ 查看并  │
│ 报警    │     │ 校验    │     │ 警事件  │     │ 处置    │
└─────────┘     └─────────┘     └─────────┘     └─────────┘
     │               │               │               │
     │ NOTIFY        │ 200 OK        │               │
     │──────────────>│──────────────>│               │
     │               │               │               │
     │               │               │               │ 处置
     │               │               │               │ 结果
     │               │               │<──────────────│
```

### 5.1 处置状态

| 状态 | 说明 |
|------|------|
| pending | 待处置 |
| handled | 已处置 |

## 6. SIP 信令交互

### 6.1 平台布防订阅（SUBSCRIBE）

```
SUBSCRIBE sip:34020000001320000104@192.168.50.104:5060 SIP/2.0
Via: SIP/2.0/UDP 192.168.50.87:11008
From: <sip:34020000000000000001@3402000000>
To: <sip:34020000001320000104@3402000000>
Call-ID: xxx@192.168.50.87
CSeq: 1 SUBSCRIBE
Contact: <sip:34020000000000000001@192.168.50.87:11008>
Expires: 86400
Event: alarm
Content-Length: 0
```

### 6.2 设备报警上报（NOTIFY）

```
NOTIFY sip:34020000000000000001@192.168.50.87:11008 SIP/2.0
Via: SIP/2.0/UDP 192.168.50.104:5060
From: <sip:34020000001320000104@3402000000>
To: <sip:34020000000000000001@3402000000>
Call-ID: xxx@192.168.50.87
CSeq: 2 NOTIFY
Contact: <sip:34020000001320000104@192.168.50.104:5060>
Event: alarm
Content-Type: Application/MANSCDP+xml
Content-Length: xxx

<?xml version="1.0"?>
<Notify>
  <CmdType>Alarm</CmdType>
  <SourceID>34020000001320000104</SourceID>
  <AlarmMethod>5</AlarmMethod>
  <AlarmType>06</AlarmType>
  <AlarmTime>2026-05-14T11:31:38</AlarmTime>
  <Info>
    <DeviceID>34020000001320000104</DeviceID>
    <AlarmDesc>入侵检测报警</AlarmDesc>
  </Info>
</Notify>
```

## 7. 最佳实践

### 7.1 报警规则配置

1. **合理设置防抖动时间**: 避免频繁误报，建议设置 5-30 秒
2. **明确报警优先级**: 根据实际场景设置高/中/低优先级
3. **配置多渠道通知**: 重要报警同时配置 webhook 和邮件通知

### 7.2 订阅管理

1. **定时续订**: 在订阅过期前 1 小时重新订阅
2. **心跳检测**: 定期检查订阅状态，确保布防有效
3. **批量订阅**: 对同一设备的多个报警类型可以批量订阅

### 7.3 报警处置

1. **优先处置高优先级**: 按优先级顺序处理报警
2. **详细记录处置结果**: 便于后续分析和追溯
3. **定期清理历史**: 建议保留 30 天报警记录

## 8. 常见问题

### Q1: 订阅后多久会过期？
A: 订阅有效期为 24 小时，需要在过期前重新订阅。

### Q2: 如何确认设备已收到订阅？
A: 设备收到 SUBSCRIBE 请求后会返回 200 OK 响应。

### Q3: 报警事件存储多久？
A: 默认永久存储，建议定期清理 30 天前的历史记录。

### Q4: 如何区分不同设备的报警？
A: 通过 `device_id` 和 `channel_id` 字段区分。

## 9. 版本信息

- **模块版本**: 2.0.0
- **GB28181 标准**: GB/T 28181-2016
- **API 版本**: v1
