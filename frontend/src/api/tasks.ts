import { request } from './request'

export interface Task {
  id: number
  name: string
  description: string
  device_id: number
  graph_config: string
  status: 'stopped' | 'running' | 'paused' | 'error'
  schedule_cron: string
  created_at: string
  updated_at: string
}

export interface TaskParams {
  name: string
  description?: string
  device_id: number
  graph_config?: string
  schedule_cron?: string
}

export const taskApi = {
  list: (params?: { status?: string; page?: number; limit?: number }) =>
    request<{ items: Task[]; total: number }>({
      url: '/tasks',
      method: 'GET',
      params,
    }),

  get: (id: number) =>
    request<Task>({
      url: `/tasks/${id}`,
      method: 'GET',
    }),

  create: (data: TaskParams) =>
    request<Task>({
      url: '/tasks',
      method: 'POST',
      data,
    }),

  update: (id: number, data: Partial<TaskParams>) =>
    request<Task>({
      url: `/tasks/${id}`,
      method: 'PUT',
      data,
    }),

  delete: (id: number) =>
    request<void>({
      url: `/tasks/${id}`,
      method: 'DELETE',
    }),

  start: (id: number) =>
    request<Task>({
      url: `/tasks/${id}/start`,
      method: 'POST',
    }),

  stop: (id: number) =>
    request<Task>({
      url: `/tasks/${id}/stop`,
      method: 'POST',
    }),

  pause: (id: number) =>
    request<Task>({
      url: `/tasks/${id}/pause`,
      method: 'POST',
    }),

  resume: (id: number) =>
    request<Task>({
      url: `/tasks/${id}/resume`,
      method: 'POST',
    }),

  updateConfig: (id: number, config: Record<string, unknown>) =>
    request<Task>({
      url: `/tasks/${id}/config`,
      method: 'PUT',
      data: config,
    }),
}
