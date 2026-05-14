import request from './request'

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

export interface CreateWorkflowParams {
  name: string
  description?: string
  status?: string
}

export function getWorkflows(params?: { page?: number; limit?: number; status?: string }) {
  return request.get<{ list: Workflow[]; total: number }>('/api/v1/workflows', { params })
}

export function getWorkflow(id: number) {
  return request.get<Workflow>(`/api/v1/workflows/${id}`)
}

export function createWorkflow(data: CreateWorkflowParams) {
  return request.post<Workflow>('/api/v1/workflows', data)
}

export function updateWorkflow(id: number, data: Partial<Workflow>) {
  return request.put<Workflow>(`/api/v1/workflows/${id}`, data)
}

export function deleteWorkflow(id: number) {
  return request.delete(`/api/v1/workflows/${id}`)
}

export function updateWorkflowStatus(id: number, status: string) {
  return request.put(`/api/v1/workflows/${id}`, { status })
}
