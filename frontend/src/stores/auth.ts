import { defineStore } from 'pinia'
import { authApi } from '../api/auth'

interface User {
  id: number
  username: string
  role: string
}

interface AuthState {
  token: string | null
  user: User | null
}

export const useAuthStore = defineStore('auth', {
  state: (): AuthState => ({
    token: null,
    user: null,
  }),

  getters: {
    isAuthenticated: (state) => !!state.token,
    currentUser: (state) => state.user,
  },

  actions: {
    initToken() {
      const token = localStorage.getItem('token')
      const userStr = localStorage.getItem('user')
      if (token) {
        this.token = token
        this.user = userStr ? JSON.parse(userStr) : null
      }
    },

    async login(username: string, password: string) {
      const result = await authApi.login({ username, password })
      this.token = result.token
      this.user = result.user
      localStorage.setItem('token', result.token)
      localStorage.setItem('user', JSON.stringify(result.user))
    },

    async logout() {
      try {
        await authApi.logout()
      } finally {
        this.token = null
        this.user = null
        localStorage.removeItem('token')
        localStorage.removeItem('user')
      }
    },
  },
})
