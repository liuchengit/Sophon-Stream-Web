import { ref, onUnmounted, type Ref } from 'vue'

export interface WebSocketMessage {
  type: string
  data?: Record<string, unknown>
  [key: string]: unknown
}

export function useWebSocket(url?: string) {
  const ws = ref<WebSocket | null>(null)
  const isConnected = ref(false)
  const lastMessage: Ref<WebSocketMessage | null> = ref(null)
  const messageHistory: Ref<WebSocketMessage[]> = ref([])

  const listeners = new Map<string, Set<(msg: WebSocketMessage) => void>>()

  function connect(token: string) {
    if (ws.value) {
      ws.value.close()
    }

    const wsUrl = url || `${window.location.protocol === 'https:' ? 'wss:' : 'ws:'}//${window.location.host}/api/v1/ws/notifications?token=${token}`
    ws.value = new WebSocket(wsUrl)

    ws.value.onopen = () => {
      isConnected.value = true
    }

    ws.value.onmessage = (event) => {
      try {
        const msg = JSON.parse(event.data) as WebSocketMessage
        lastMessage.value = msg
        messageHistory.value.push(msg)

        const typeListeners = listeners.get(msg.type)
        if (typeListeners) {
          typeListeners.forEach((fn) => fn(msg))
        }
      } catch {
        // Ignore non-JSON messages
      }
    }

    ws.value.onclose = () => {
      isConnected.value = false
      // Auto-reconnect after 3s
      setTimeout(() => {
        const t = localStorage.getItem('token')
        if (t) connect(t)
      }, 3000)
    }

    ws.value.onerror = () => {
      isConnected.value = false
    }
  }

  function disconnect() {
    if (ws.value) {
      ws.value.close()
      ws.value = null
    }
    isConnected.value = false
  }

  function send(data: Record<string, unknown>) {
    if (ws.value && ws.value.readyState === WebSocket.OPEN) {
      ws.value.send(JSON.stringify(data))
    }
  }

  function on(type: string, callback: (msg: WebSocketMessage) => void) {
    if (!listeners.has(type)) {
      listeners.set(type, new Set())
    }
    listeners.get(type)!.add(callback)
  }

  function off(type: string, callback: (msg: WebSocketMessage) => void) {
    const typeListeners = listeners.get(type)
    if (typeListeners) {
      typeListeners.delete(callback)
    }
  }

  onUnmounted(() => {
    disconnect()
  })

  return {
    ws,
    isConnected,
    lastMessage,
    messageHistory,
    connect,
    disconnect,
    send,
    on,
    off,
  }
}
