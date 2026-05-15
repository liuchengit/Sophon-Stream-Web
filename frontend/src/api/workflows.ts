import { request } from './request'

export interface Workflow {
  id: number
  name: string
  description: string
  status: string
  nodes?: WorkflowNode[]
  edges?: WorkflowEdge[]
  created_at: string
  updated_at: string
}

export interface WorkflowNode {
  id: string
  type: string
  position: { x: number; y: number }
  data: {
    label: string
    config: Record<string, unknown>
  }
}

export interface WorkflowEdge {
  id: string
  source: string
  target: string
  sourceHandle?: string
  targetHandle?: string
}

export interface ExecutionStatus {
  executionId: number
  workflowId: number
  status: string
  startedAt: string
  finishedAt?: string
  errorMessage?: string
  nodes: ExecutionNode[]
}

export interface ExecutionNode {
  nodeId: string
  nodeType: string
  label: string
  status: string
  startedAt?: string
  finishedAt?: string
  errorMessage?: string
}

export const workflowApi = {
  list: (params?: { page?: number; limit?: number; status?: string }) =>
    request<{ items: Workflow[]; total: number }>({
      url: '/workflows',
      method: 'GET',
      params,
    }),

  get: (id: number) =>
    request<Workflow>({
      url: `/workflows/${id}`,
      method: 'GET',
    }),

  create: (data: { name: string; description?: string; status?: string }) =>
    request<Workflow>({
      url: '/workflows',
      method: 'POST',
      data,
    }),

  update: (id: number, data: Partial<Workflow>) =>
    request<Workflow>({
      url: `/workflows/${id}`,
      method: 'PUT',
      data,
    }),

  delete: (id: number) =>
    request<void>({
      url: `/workflows/${id}`,
      method: 'DELETE',
    }),

  updateStatus: (id: number, status: string) =>
    request<Workflow>({
      url: `/workflows/${id}`,
      method: 'PUT',
      data: { status },
    }),

  startExecution: (id: number) =>
    request<{ executionId: number; workflowId: number; status: string; nodeCount: number }>({
      url: `/workflows/${id}/start`,
      method: 'POST',
    }),

  stopExecution: (id: number) =>
    request<void>({
      url: `/workflows/${id}/stop`,
      method: 'POST',
    }),

  getExecutionStatus: (id: number) =>
    request<ExecutionStatus>({
      url: `/workflows/${id}/status`,
      method: 'GET',
    }),

  getExecutionHistory: (id: number, params?: { page?: number; limit?: number }) =>
    request<{ items: Array<{ id: number; status: string; startedAt: string; finishedAt?: string; errorMessage?: string }>; total: number }>({
      url: `/workflows/${id}/history`,
      method: 'GET',
      params,
    }),
}
