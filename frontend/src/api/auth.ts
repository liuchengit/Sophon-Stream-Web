import { request } from './request'

export interface LoginParams {
  username: string
  password: string
}

export interface LoginResult {
  token: string
  user: {
    id: number
    username: string
    role: string
  }
}

export const authApi = {
  login: (params: LoginParams) =>
    request<LoginResult>({
      url: '/auth/login',
      method: 'POST',
      data: params,
    }),

  logout: () =>
    request<void>({
      url: '/auth/logout',
      method: 'POST',
    }),
}
