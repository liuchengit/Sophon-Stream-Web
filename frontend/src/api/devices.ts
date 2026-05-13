import { request } from './request'

export interface Device {
  id: number
  name: string
  type: string
  ip_address: string
  port: number
  status: 'online' | 'offline' | 'error'
  model: string
  firmware_version: string
  created_at: string
  updated_at: string
}

export interface DeviceParams {
  name: string
  type: string
  ip_address: string
  port?: number
  model?: string
}

export const deviceApi = {
  list: (params?: { status?: string; type?: string; page?: number; limit?: number }) =>
    request<{ items: Device[]; total: number }>({
      url: '/devices',
      method: 'GET',
      params,
    }),

  get: (id: number) =>
    request<Device>({
      url: `/devices/${id}`,
      method: 'GET',
    }),

  create: (data: DeviceParams) =>
    request<Device>({
      url: '/devices',
      method: 'POST',
      data,
    }),

  update: (id: number, data: Partial<DeviceParams>) =>
    request<Device>({
      url: `/devices/${id}`,
      method: 'PUT',
      data,
    }),

  delete: (id: number) =>
    request<void>({
      url: `/devices/${id}`,
      method: 'DELETE',
    }),
}
