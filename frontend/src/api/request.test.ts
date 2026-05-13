import { describe, it, expect } from 'vitest'
import { request } from './request'

describe('API Request', () => {
  it('request function is defined', () => {
    expect(request).toBeDefined()
  })
})
