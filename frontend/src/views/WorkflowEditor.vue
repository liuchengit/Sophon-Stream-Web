<template>
  <div class="workflow-editor">
    <div class="toolbar">
      <el-button @click="goBack">返回</el-button>
      <span class="title">{{ workflowName }}</span>
      <el-button-group class="ml-auto">
        <el-button @click="addNode('source')" type="info" size="small">
          + 视频源
        </el-button>
        <el-button @click="addNode('algorithm')" type="primary" size="small">
          + 算法
        </el-button>
        <el-button @click="addNode('output')" type="success" size="small">
          + 输出
        </el-button>
      </el-button-group>
      <el-button @click="saveWorkflow" type="primary" :loading="saving">保存</el-button>
    </div>

    <VueFlow
      v-model="elements"
      :default-viewport="{ zoom: 1, x: 0, y: 0 }"
      :min-zoom="0.2"
      :max-zoom="4"
      fit-view-on-init
      @connect="onConnect"
    >
      <Background />
      <Controls />
      <MiniMap />

      <template #node-source="nodeProps">
        <div class="custom-node source-node">
          <div class="node-header">
            <el-icon><VideoCamera /></el-icon>
            <span>{{ nodeProps.data.label }}</span>
          </div>
          <div class="node-body">
            <el-select v-model="(nodeProps.data as any).config.deviceId" size="small" placeholder="选择设备">
              <el-option v-for="d in devices" :key="d.id" :label="d.name" :value="d.id" />
            </el-select>
          </div>
          <Handle type="source" :position="Position.Right" />
        </div>
      </template>

      <template #node-algorithm="nodeProps">
        <div class="custom-node algo-node">
          <Handle type="target" :position="Position.Left" />
          <div class="node-header">
            <el-icon><Cpu /></el-icon>
            <span>{{ nodeProps.data.label }}</span>
          </div>
          <div class="node-body">
            <el-select v-model="(nodeProps.data as any).config.algorithmId" size="small" placeholder="选择算法">
              <el-option v-for="a in algorithms" :key="a.id" :label="a.name" :value="a.id" />
            </el-select>
          </div>
          <Handle type="source" :position="Position.Right" />
        </div>
      </template>

      <template #node-output="nodeProps">
        <div class="custom-node output-node">
          <Handle type="target" :position="Position.Left" />
          <div class="node-header">
            <el-icon><Monitor /></el-icon>
            <span>{{ nodeProps.data.label }}</span>
          </div>
          <div class="node-body">
            <el-select v-model="(nodeProps.data as any).config.outputType" size="small" placeholder="输出类型">
              <el-option label="RTSP 推流" value="rtsp" />
              <el-option label="本地录制" value="record" />
              <el-option label="告警推送" value="alarm" />
            </el-select>
          </div>
        </div>
      </template>
    </VueFlow>
  </div>
</template>

<script setup lang="ts">
// @ts-nocheck
import '@vue-flow/core/dist/style.css'
import '@vue-flow/core/dist/theme-default.css'
import '@vue-flow/controls/dist/style.css'
import '@vue-flow/minimap/dist/style.css'

import { ref, computed, onMounted } from 'vue'
import { useRouter, useRoute } from 'vue-router'
import { ElMessage } from 'element-plus'
import { VideoCamera, Cpu, Monitor } from '@element-plus/icons-vue'
import { VueFlow, Position, type Connection, type Elements } from '@vue-flow/core'
import { Background } from '@vue-flow/background'
import { Controls } from '@vue-flow/controls'
import { MiniMap } from '@vue-flow/minimap'
import { getWorkflow, updateWorkflow } from '@/api/workflows'
import { deviceApi } from '@/api/devices'
import { algorithmApi } from '@/api/algorithms'

const router = useRouter()
const route = useRoute()
const workflowId = computed(() => Number(route.params.id))

const workflowName = ref('工作流编辑器')
const elements = ref<Elements>([])
const devices = ref<Array<{ id: number; name: string }>>([])
const algorithms = ref<Array<{ id: number; name: string }>>([])
const saving = ref(false)

const nodeCounter = ref(0)

const onConnect = (params: Connection) => {
  elements.value.push({
    id: `e-${params.source}-${params.target}`,
    source: params.source || '',
    target: params.target || '',
    sourceHandle: params.sourceHandle || undefined,
    targetHandle: params.targetHandle || undefined,
    animated: true,
    style: { stroke: '#409EFF' },
  })
}

const addNode = (type: string) => {
  nodeCounter.value++
  const id = `${type}-${nodeCounter.value}`
  const labels: Record<string, string> = { source: '视频源', algorithm: '算法处理', output: '输出' }

  elements.value.push({
    id,
    type,
    position: { x: 100 + Math.random() * 300, y: 100 + Math.random() * 200 },
    data: { label: `${labels[type]} ${nodeCounter.value}`, config: {} },
  })
}

const saveWorkflow = async () => {
  saving.value = true
  try {
    const nodeData = elements.value
      .filter((e: any) => !e.source)
      .map((n: any) => ({
        id: n.id,
        type: n.type || 'default',
        position: n.position,
        data: n.data,
      }))

    const edgeData = elements.value
      .filter((e: any) => e.source)
      .map((e: any) => ({
        id: e.id,
        source: e.source,
        target: e.target,
        sourceHandle: e.sourceHandle || 'default',
        targetHandle: e.targetHandle || 'default',
      }))

    await updateWorkflow(workflowId.value, { nodes: nodeData, edges: edgeData } as any)
    ElMessage.success('保存成功')
  } catch {
    ElMessage.error('保存失败')
  } finally {
    saving.value = false
  }
}

const goBack = () => {
  router.push('/workflow')
}

onMounted(async () => {
  try {
    const [wf, devRes, algoRes] = await Promise.all([
      getWorkflow(workflowId.value),
      deviceApi.list({ page: 1, limit: 100 }),
      algorithmApi.list({ page: 1, limit: 100 }),
    ])

    workflowName.value = wf.name

    const devData = devRes.items || []
    const algoData = algoRes.items || []
    devices.value = devData
    algorithms.value = algoData

    const existingNodes = wf.nodes || []
    const existingEdges = wf.edges || []

    if (existingNodes.length > 0) {
      elements.value = [
        ...existingNodes.map((n: any) => ({
          id: n.id,
          type: n.type,
          position: n.position,
          data: n.data,
        })),
        ...existingEdges.map((e: any) => ({
          id: e.id,
          source: e.source,
          target: e.target,
          sourceHandle: e.sourceHandle,
          targetHandle: e.targetHandle,
          animated: true,
          style: { stroke: '#409EFF' },
        })),
      ]
    }
  } catch {
    ElMessage.error('加载工作流失败')
  }
})
</script>

<style scoped>
.workflow-editor {
  display: flex;
  flex-direction: column;
  height: calc(100vh - 60px);
}
.toolbar {
  display: flex;
  align-items: center;
  gap: 12px;
  padding: 8px 16px;
  background: #f5f7fa;
  border-bottom: 1px solid #e4e7ed;
}
.title {
  font-weight: 600;
  font-size: 16px;
}
.ml-auto {
  margin-left: auto;
}
.custom-node {
  background: #fff;
  border-radius: 8px;
  border: 2px solid #dcdfe6;
  padding: 0;
  min-width: 200px;
  box-shadow: 0 2px 12px rgba(0, 0, 0, 0.1);
}
.node-header {
  display: flex;
  align-items: center;
  gap: 8px;
  padding: 8px 12px;
  font-weight: 600;
  font-size: 14px;
  border-radius: 6px 6px 0 0;
}
.node-body {
  padding: 8px 12px 12px;
}
.source-node {
  border-color: #67c23a;
}
.source-node .node-header {
  background: #f0f9eb;
  color: #67c23a;
}
.algo-node {
  border-color: #409eff;
}
.algo-node .node-header {
  background: #ecf5ff;
  color: #409eff;
}
.output-node {
  border-color: #e6a23c;
}
.output-node .node-header {
  background: #fdf6ec;
  color: #e6a23c;
}
</style>
