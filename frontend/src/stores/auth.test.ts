import { describe, it, expect, vi } from 'vitest'
import { useAuthStore } from '../src/stores/auth'
import { createPinia, setActivePinia } from 'pinia'

describe('Auth Store', () => {
  beforeEach(() => {
    setActivePinia(createPinia())
    localStorage.clear()
  })

  it('initializes with no token', () => {
    const store = useAuthStore()
    expect(store.isAuthenticated).toBe(false)
    expect(store.token).toBeNull()
  })

  it('initializes token from localStorage', () => {
    localStorage.setItem('token', 'test-token')
    localStorage.setItem('user', JSON.stringify({ id: 1, username: 'admin', role: 'admin' }))

    const store = useAuthStore()
    store.initToken()

    expect(store.isAuthenticated).toBe(true)
    expect(store.token).toBe('test-token')
    expect(store.currentUser?.username).toBe('admin')
  })

  it('login sets token and user', async () => {
    const store = useAuthStore()
    // Mock the API call
    vi.mock('../src/api/auth', () => ({
      authApi: {
        login: vi.fn().mockResolvedValue({
          token: 'mock-token',
          user: { id: 1, username: 'admin', role: 'admin' },
        }),
      },
    }))

    await store.login('admin', 'admin123')

    expect(store.isAuthenticated).toBe(true)
    expect(store.token).toBe('mock-token')
    expect(localStorage.getItem('token')).toBe('mock-token')
  })

  it('logout clears token and user', async () => {
    const store = useAuthStore()
    localStorage.setItem('token', 'test-token')
    localStorage.setItem('user', JSON.stringify({ id: 1, username: 'admin', role: 'admin' }))
    store.initToken()

    await store.logout()

    expect(store.isAuthenticated).toBe(false)
    expect(store.token).toBeNull()
    expect(localStorage.getItem('token')).toBeNull()
  })
})
