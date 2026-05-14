import { request } from './request'

export interface DashboardStats {
  devices: { total: number; online: number; offline: number }
  tasks: { total: number; running: number; stopped: number }
  alarms: { total: number; today: number }
  algorithms: { total: number; active: number }
}

export const dashboardApi = {
  getStats: () =>
    request<DashboardStats>({
      url: '/monitoring/stats',
      method: 'GET',
    }),
}
