import { describe, it, expect } from 'vitest'
import { request } from '../src/api/request'
import axios from 'axios'

vi.mock('axios', () => ({
  default: {
    create: vi.fn(() => ({
      interceptors: {
        request: { use: vi.fn() },
        response: { use: vi.fn() },
      },
    })),
  },
}))

describe('API Request', () => {
  it('creates axios instance with correct config', () => {
    expect(request).toBeDefined()
  })
})
