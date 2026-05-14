import { request } from './request'

export interface Config {
  key: string
  value: string
  version: number
  created_at: string
}

export const configApi = {
  list: () =>
    request<{ items: Config[]; total: number }>({
      url: '/config',
      method: 'GET',
    }),

  get: (key: string) =>
    request<Config>({
      url: `/config/${key}`,
      method: 'GET',
    }),

  update: (key: string, value: string) =>
    request<Config>({
      url: `/config/${key}`,
      method: 'PUT',
      data: { value },
    }),

  history: (key: string) =>
    request<{ items: Config[]; total: number }>({
      url: `/config/${key}/history`,
      method: 'GET',
    }),
}
