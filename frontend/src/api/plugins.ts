import { request } from './request'

export interface Plugin {
  id: number
  name: string
  version: string
  path: string
  signature: string
  status: string
  created_at: string
}

export const pluginApi = {
  list: () =>
    request<{ items: Plugin[]; total: number }>({
      url: '/plugins',
      method: 'GET',
    }),

  install: (data: { name: string; version: string; path: string }) =>
    request<Plugin>({
      url: '/plugins',
      method: 'POST',
      data,
    }),

  update: (id: number, data: Partial<Plugin>) =>
    request<Plugin>({
      url: `/plugins/${id}`,
      method: 'PUT',
      data,
    }),

  uninstall: (id: number) =>
    request<void>({
      url: `/plugins/${id}`,
      method: 'DELETE',
    }),
}
