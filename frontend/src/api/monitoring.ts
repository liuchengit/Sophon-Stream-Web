import { request } from './request'

export interface ResourceMetrics {
  tpu_usage: number
  cpu_usage: number
  memory_usage: number
  memory_total: number
  memory_used: number
  timestamp: string
}

export interface AlarmEvent {
  id: number
  rule_id: number
  task_id: number
  evidence_path: string
  context: Record<string, unknown>
  triggered_at: string
  // GB28181 fields
  gb_alarm_code?: string
  alarm_priority?: string
  alarm_type?: string
  device_id?: number
  channel_id?: number
  sip_transaction_id?: string
  alarm_description?: string
  handled_status?: string
  handled_at?: string
  handled_by?: number
  handle_result?: string
  alarm_method?: number
}

export interface AlarmRule {
  id: number
  name: string
  condition_expr: string
  debounce_seconds: number
  notification_channels: string
  enabled: boolean
  created_at: string
  // GB28181 fields
  gb_alarm_type?: string
  alarm_method?: number
  subscribe_status?: string
  subscribe_expires?: string
  device_id?: number
  channel_id?: number
  alarm_priority?: string
  alarm_description?: string
}

export const monitoringApi = {
  getResources: () =>
    request<ResourceMetrics>({
      url: '/monitoring/resources',
      method: 'GET',
    }),

  getHistory: (params: { type: string; start: string; end: string }) =>
    request<Array<{ value: number; recorded_at: string }>>({
      url: '/monitoring/history',
      method: 'GET',
      params,
    }),
}

export const alarmApi = {
  listRules: () =>
    request<AlarmRule[]>({
      url: '/alarms/rules',
      method: 'GET',
    }),

  createRule: (data: Omit<AlarmRule, 'id' | 'created_at'>) =>
    request<AlarmRule>({
      url: '/alarms/rules',
      method: 'POST',
      data,
    }),

  updateRule: (id: number, data: Partial<AlarmRule>) =>
    request<AlarmRule>({
      url: `/alarms/rules/${id}`,
      method: 'PUT',
      data,
    }),

  deleteRule: (id: number) =>
    request<void>({
      url: `/alarms/rules/${id}`,
      method: 'DELETE',
    }),

  toggleRule: (id: number) =>
    request<{ id: number; enabled: boolean }>({
      url: `/alarms/rules/${id}/toggle`,
      method: 'POST',
    }),

  subscribeRule: (id: number) =>
    request<{ id: number; subscribe_status: string; subscribe_expires: string }>({
      url: `/alarms/rules/${id}/subscribe`,
      method: 'POST',
    }),

  unsubscribeRule: (id: number) =>
    request<{ id: number; subscribe_status: string }>({
      url: `/alarms/rules/${id}/unsubscribe`,
      method: 'POST',
    }),

  listEvents: (params?: { page?: number; limit?: number; status?: string }) =>
    request<{ items: AlarmEvent[]; total: number }>({
      url: '/alarms/events',
      method: 'GET',
      params,
    }),

  handleEvent: (id: number, data: { result: string }) =>
    request<{ id: number; handled_status: string }>({
      url: `/alarms/events/${id}/handle`,
      method: 'POST',
      data,
    }),
}
