import { request } from './request'

export interface Algorithm {
  id: number
  name: string
  version: string
  type: string
  model_path: string
  config_schema: string
  plugin_path: string
  status: string
  created_at: string
}

export const algorithmApi = {
  list: (params?: { page?: number; limit?: number }) =>
    request<{ items: Algorithm[]; total: number }>({
      url: '/algorithms',
      method: 'GET',
      params,
    }),

  get: (id: number) =>
    request<Algorithm>({
      url: `/algorithms/${id}`,
      method: 'GET',
    }),

  create: (data: Omit<Algorithm, 'id' | 'created_at'>) =>
    request<Algorithm>({
      url: '/algorithms',
      method: 'POST',
      data,
    }),

  update: (id: number, data: Partial<Algorithm>) =>
    request<Algorithm>({
      url: `/algorithms/${id}`,
      method: 'PUT',
      data,
    }),

  delete: (id: number) =>
    request<void>({
      url: `/algorithms/${id}`,
      method: 'DELETE',
    }),
}
