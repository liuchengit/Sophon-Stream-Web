<template>
  <div class="workflow-editor">
    <div class="toolbar">
      <el-button @click="goBack">返回</el-button>
      <span class="title">{{ workflowName }}</span>
      <el-tag v-if="execStatus" :type="execStatusType" size="small" effect="dark" class="exec-tag">
        <el-icon class="is-loading" v-if="execStatus === 'running'"><Loading /></el-icon>
        {{ execStatusLabel }}
      </el-tag>
      <el-button-group class="ml-auto">
        <el-button @click="addNode('source')" type="info" size="small" :disabled="execStatus === 'running'">
          + 视频源
        </el-button>
        <el-button @click="addNode('algorithm')" type="primary" size="small" :disabled="execStatus === 'running'">
          + 算法
        </el-button>
        <el-button @click="addNode('output')" type="success" size="small" :disabled="execStatus === 'running'">
          + 输出
        </el-button>
      </el-button-group>
      <el-button @click="saveWorkflow" type="primary" :loading="saving" :disabled="execStatus === 'running'">保存</el-button>
      <el-button @click="startExecution" type="success" :loading="execLoading" :disabled="execStatus === 'running'" v-if="execStatus !== 'running'">
        <el-icon><VideoPlay /></el-icon> 执行
      </el-button>
      <el-button @click="stopExecution" type="danger" :loading="execLoading" v-if="execStatus === 'running'">
        <el-icon><VideoPause /></el-icon> 停止
      </el-button>
    </div>

    <div class="editor-body">
      <VueFlow
        v-model="elements"
        :default-viewport="{ zoom: 1, x: 0, y: 0 }"
        :min-zoom="0.2"
        :max-zoom="4"
        fit-view-on-init
        @connect="onConnect"
        @node-click="onNodeClick"
        @pane-click="onPaneClick"
      >
        <Background />
        <Controls />
        <MiniMap />

        <template #node-source="nodeProps">
          <div class="custom-node source-node" :class="{
            selected: selectedNode?.id === nodeProps.id,
            'exec-running': nodeExecStates[nodeProps.id] === 'running',
            'exec-completed': nodeExecStates[nodeProps.id] === 'completed',
            'exec-failed': nodeExecStates[nodeProps.id] === 'failed',
          }">
            <div class="node-header">
              <el-icon v-if="nodeExecStates[nodeProps.id] === 'running'" class="is-loading"><Loading /></el-icon>
              <el-icon v-else-if="nodeExecStates[nodeProps.id] === 'completed'" color="#67c23a"><CircleCheck /></el-icon>
              <el-icon v-else-if="nodeExecStates[nodeProps.id] === 'failed'" color="#f56c6c"><CircleClose /></el-icon>
              <el-icon v-else><VideoCamera /></el-icon>
              <span>{{ nodeProps.data.label }}</span>
              <el-tag v-if="nodeExecStates[nodeProps.id]" :type="nodeExecType(nodeExecStates[nodeProps.id])" size="small" class="node-exec-tag">
                {{ nodeExecLabel(nodeExecStates[nodeProps.id]) }}
              </el-tag>
            </div>
            <div class="node-body">
              <div class="node-summary">
                {{ (nodeProps.data as any).config.deviceName || '未选择设备' }}
              </div>
            </div>
            <Handle type="source" :position="Position.Right" />
          </div>
        </template>

        <template #node-algorithm="nodeProps">
          <div class="custom-node algo-node" :class="{
            selected: selectedNode?.id === nodeProps.id,
            'exec-running': nodeExecStates[nodeProps.id] === 'running',
            'exec-completed': nodeExecStates[nodeProps.id] === 'completed',
            'exec-failed': nodeExecStates[nodeProps.id] === 'failed',
          }">
            <Handle type="target" :position="Position.Left" />
            <div class="node-header">
              <el-icon v-if="nodeExecStates[nodeProps.id] === 'running'" class="is-loading"><Loading /></el-icon>
              <el-icon v-else-if="nodeExecStates[nodeProps.id] === 'completed'" color="#67c23a"><CircleCheck /></el-icon>
              <el-icon v-else-if="nodeExecStates[nodeProps.id] === 'failed'" color="#f56c6c"><CircleClose /></el-icon>
              <el-icon v-else><Cpu /></el-icon>
              <span>{{ nodeProps.data.label }}</span>
              <el-tag v-if="nodeExecStates[nodeProps.id]" :type="nodeExecType(nodeExecStates[nodeProps.id])" size="small" class="node-exec-tag">
                {{ nodeExecLabel(nodeExecStates[nodeProps.id]) }}
              </el-tag>
            </div>
            <div class="node-body">
              <div class="node-summary">
                {{ (nodeProps.data as any).config.algorithmName || '未选择算法' }}
              </div>
            </div>
            <Handle type="source" :position="Position.Right" />
          </div>
        </template>

        <template #node-output="nodeProps">
          <div class="custom-node output-node" :class="{
            selected: selectedNode?.id === nodeProps.id,
            'exec-running': nodeExecStates[nodeProps.id] === 'running',
            'exec-completed': nodeExecStates[nodeProps.id] === 'completed',
            'exec-failed': nodeExecStates[nodeProps.id] === 'failed',
          }">
            <Handle type="target" :position="Position.Left" />
            <div class="node-header">
              <el-icon v-if="nodeExecStates[nodeProps.id] === 'running'" class="is-loading"><Loading /></el-icon>
              <el-icon v-else-if="nodeExecStates[nodeProps.id] === 'completed'" color="#67c23a"><CircleCheck /></el-icon>
              <el-icon v-else-if="nodeExecStates[nodeProps.id] === 'failed'" color="#f56c6c"><CircleClose /></el-icon>
              <el-icon v-else><Monitor /></el-icon>
              <span>{{ nodeProps.data.label }}</span>
              <el-tag v-if="nodeExecStates[nodeProps.id]" :type="nodeExecType(nodeExecStates[nodeProps.id])" size="small" class="node-exec-tag">
                {{ nodeExecLabel(nodeExecStates[nodeProps.id]) }}
              </el-tag>
            </div>
            <div class="node-body">
              <div class="node-summary">
                {{ outputTypeLabel((nodeProps.data as any).config.outputType) }}
              </div>
            </div>
          </div>
        </template>
      </VueFlow>

      <!-- Node Config Panel -->
      <Transition name="slide">
        <div v-if="selectedNode" class="config-panel">
          <div class="panel-header">
            <span>节点配置</span>
            <el-button text @click="selectedNode = null" size="small">关闭</el-button>
          </div>

          <div class="panel-body">
            <el-form label-position="top" size="small">
              <el-form-item label="节点名称">
                <el-input v-model="selectedNode.data.label" />
              </el-form-item>

              <template v-if="selectedNode.type === 'source'">
                <el-form-item label="设备">
                  <el-select v-model="sourceForm.deviceId" placeholder="选择设备" style="width: 100%" @change="onSourceChange">
                    <el-option v-for="d in devices" :key="d.id" :label="d.name" :value="d.id" />
                  </el-select>
                </el-form-item>
                <el-form-item label="流地址">
                  <el-input v-model="sourceForm.streamUrl" placeholder="rtsp://..." />
                </el-form-item>
              </template>

              <template v-if="selectedNode.type === 'algorithm'">
                <el-form-item label="算法">
                  <el-select v-model="algoForm.algorithmId" placeholder="选择算法" style="width: 100%" @change="onAlgoChange">
                    <el-option v-for="a in algorithms" :key="a.id" :label="a.name" :value="a.id" />
                  </el-select>
                </el-form-item>
                <el-form-item label="置信度阈值">
                  <el-slider v-model="algoForm.threshold" :min="0" :max="100" show-input />
                </el-form-item>
                <el-form-item label="检测区域 (ROI)">
                  <el-input v-model="algoForm.roi" placeholder="auto" />
                </el-form-item>
              </template>

              <template v-if="selectedNode.type === 'output'">
                <el-form-item label="输出类型">
                  <el-select v-model="outputForm.outputType" style="width: 100%" @change="onOutputChange">
                    <el-option label="RTSP 推流" value="rtsp" />
                    <el-option label="本地录制" value="record" />
                    <el-option label="告警推送" value="alarm" />
                    <el-option label="Webhook" value="webhook" />
                  </el-select>
                </el-form-item>
                <el-form-item v-if="outputForm.outputType === 'rtsp'" label="推流地址">
                  <el-input v-model="outputForm.pushUrl" placeholder="rtsp://..." />
                </el-form-item>
                <el-form-item v-if="outputForm.outputType === 'record'" label="录制路径">
                  <el-input v-model="outputForm.recordPath" placeholder="/data/recordings" />
                </el-form-item>
                <el-form-item v-if="outputForm.outputType === 'webhook'" label="Webhook URL">
                  <el-input v-model="outputForm.webhookUrl" placeholder="https://..." />
                </el-form-item>
              </template>
            </el-form>
          </div>

          <div class="panel-footer">
            <el-button type="danger" size="small" @click="deleteSelectedNode" style="width: 100%">删除节点</el-button>
          </div>
        </div>
      </Transition>
    </div>
  </div>
</template>

<script setup lang="ts">
// @ts-nocheck
import '@vue-flow/core/dist/style.css'
import '@vue-flow/core/dist/theme-default.css'
import '@vue-flow/controls/dist/style.css'
import '@vue-flow/minimap/dist/style.css'

import { ref, computed, onMounted, onUnmounted, watch } from 'vue'
import { useRouter, useRoute } from 'vue-router'
import { ElMessage, ElMessageBox } from 'element-plus'
import { VideoCamera, Cpu, Monitor, VideoPlay, VideoPause, Loading, CircleCheck, CircleClose } from '@element-plus/icons-vue'
import { VueFlow, Position, type Connection, type Elements } from '@vue-flow/core'
import { Background } from '@vue-flow/background'
import { Controls } from '@vue-flow/controls'
import { MiniMap } from '@vue-flow/minimap'
import { workflowApi } from '@/api/workflows'
import { deviceApi } from '@/api/devices'
import { algorithmApi } from '@/api/algorithms'
import { useWebSocket } from '@/composables/useWebSocket'

const router = useRouter()
const route = useRoute()
const workflowId = computed(() => Number(route.params.id))

const workflowName = ref('工作流编辑器')
const elements = ref<Elements>([])
const devices = ref<Array<{ id: number; name: string }>>([])
const algorithms = ref<Array<{ id: number; name: string }>>([])
const saving = ref(false)
const nodeCounter = ref(0)
const selectedNode = ref<any>(null)
const execStatus = ref<string>('')
const execLoading = ref(false)
const nodeExecStates = ref<Record<string, string>>({})

const sourceForm = ref({ deviceId: 0, deviceName: '', streamUrl: '' })
const algoForm = ref({ algorithmId: 0, algorithmName: '', threshold: 50, roi: 'auto' })
const outputForm = ref({ outputType: '', pushUrl: '', recordPath: '/data/recordings', webhookUrl: '' })

const outputTypeLabels: Record<string, string> = {
  rtsp: 'RTSP 推流',
  record: '本地录制',
  alarm: '告警推送',
  webhook: 'Webhook',
  '': '未配置',
}

const execStatusType = computed(() => {
  if (execStatus.value === 'running') return 'success'
  if (execStatus.value === 'completed') return 'info'
  if (execStatus.value === 'failed') return 'danger'
  return 'info'
})

const execStatusLabel = computed(() => {
  const labels: Record<string, string> = { running: '执行中', completed: '已完成', failed: '执行失败', idle: '未执行' }
  return labels[execStatus.value] || '未执行'
})

const nodeExecType = (state: string) => {
  const types: Record<string, string> = { running: 'warning', completed: 'success', failed: 'danger', pending: 'info', cancelled: 'info' }
  return types[state] || 'info'
}

const nodeExecLabel = (state: string) => {
  const labels: Record<string, string> = { running: '运行中', completed: '已完成', failed: '失败', pending: '等待中', cancelled: '已取消' }
  return labels[state] || state
}

const outputTypeLabel = (type: string) => outputTypeLabels[type] || '未配置'

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

const onNodeClick = ({ node }: any) => {
  selectedNode.value = node
  const config = node.data?.config || {}

  if (node.type === 'source') {
    sourceForm.value = { deviceId: config.deviceId || 0, deviceName: config.deviceName || '', streamUrl: config.streamUrl || '' }
  } else if (node.type === 'algorithm') {
    algoForm.value = { algorithmId: config.algorithmId || 0, algorithmName: config.algorithmName || '', threshold: config.threshold || 50, roi: config.roi || 'auto' }
  } else if (node.type === 'output') {
    outputForm.value = { outputType: config.outputType || '', pushUrl: config.pushUrl || '', recordPath: config.recordPath || '/data/recordings', webhookUrl: config.webhookUrl || '' }
  }
}

const onPaneClick = () => {
  selectedNode.value = null
}

watch(sourceForm, (val) => {
  if (!selectedNode.value) return
  const dev = devices.value.find(d => d.id === val.deviceId)
  selectedNode.value.data.config = { ...selectedNode.value.data.config, deviceId: val.deviceId, deviceName: dev?.name || '', streamUrl: val.streamUrl }
}, { deep: true })

watch(algoForm, (val) => {
  if (!selectedNode.value) return
  const algo = algorithms.value.find(a => a.id === val.algorithmId)
  selectedNode.value.data.config = { ...selectedNode.value.data.config, algorithmId: val.algorithmId, algorithmName: algo?.name || '', threshold: val.threshold, roi: val.roi }
}, { deep: true })

watch(outputForm, (val) => {
  if (!selectedNode.value) return
  selectedNode.value.data.config = { ...selectedNode.value.data.config, outputType: val.outputType, pushUrl: val.pushUrl, recordPath: val.recordPath, webhookUrl: val.webhookUrl }
}, { deep: true })

const onSourceChange = () => {
  const dev = devices.value.find(d => d.id === sourceForm.value.deviceId)
  sourceForm.value.deviceName = dev?.name || ''
}

const onAlgoChange = () => {
  const algo = algorithms.value.find(a => a.id === algoForm.value.algorithmId)
  algoForm.value.algorithmName = algo?.name || ''
}

const onOutputChange = () => {
  outputForm.value.pushUrl = ''
  outputForm.value.webhookUrl = ''
}

const deleteSelectedNode = () => {
  if (!selectedNode.value) return
  ElMessageBox.confirm('确认删除该节点?', '提示', { type: 'warning' }).then(() => {
    const nodeId = selectedNode.value.id
    elements.value = elements.value.filter((e: any) => e.id !== nodeId && e.source !== nodeId && e.target !== nodeId)
    selectedNode.value = null
  }).catch(() => {})
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

    await workflowApi.update(workflowId.value, { nodes: nodeData, edges: edgeData } as any)
    ElMessage.success('保存成功')
  } catch {
    ElMessage.error('保存失败')
  } finally {
    saving.value = false
  }
}

const startExecution = async () => {
  execLoading.value = true
  try {
    await workflowApi.startExecution(workflowId.value)
    execStatus.value = 'running'
    nodeExecStates.value = {}
    elements.value.forEach((e: any) => {
      if (!e.source) nodeExecStates.value[e.id] = 'pending'
    })
    nodeExecStates.value[elements.value.find((e: any) => !e.source)?.id] = 'running'
    ElMessage.success('开始执行')
  } catch {
    ElMessage.error('启动执行失败')
  } finally {
    execLoading.value = false
  }
}

const stopExecution = async () => {
  execLoading.value = true
  try {
    await workflowApi.stopExecution(workflowId.value)
    execStatus.value = 'stopped'
    Object.keys(nodeExecStates.value).forEach(key => {
      if (nodeExecStates.value[key] === 'running' || nodeExecStates.value[key] === 'pending') {
        nodeExecStates.value[key] = 'cancelled'
      }
    })
    ElMessage.info('已停止执行')
  } catch {
    ElMessage.error('停止执行失败')
  } finally {
    execLoading.value = false
  }
}

const handleExecutionEvent = (data: any) => {
  if (data.action === 'start') {
    execStatus.value = 'running'
    data.nodes.forEach((n: any) => {
      nodeExecStates.value[n.nodeId] = n.status
    })
  } else if (data.action === 'stop') {
    execStatus.value = 'stopped'
    Object.keys(nodeExecStates.value).forEach(key => {
      if (nodeExecStates.value[key] === 'running') nodeExecStates.value[key] = 'cancelled'
    })
  } else if (data.action === 'nodeUpdate') {
    nodeExecStates.value[data.nodeId] = data.status
  } else if (data.action === 'complete') {
    execStatus.value = 'completed'
  }
}

const goBack = () => {
  router.push('/workflow')
}

onMounted(async () => {
  try {
    const [wf, devRes, algoRes] = await Promise.all([
      workflowApi.get(workflowId.value),
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
      nodeCounter.value = existingNodes.length
    }

    try {
      const statusRes = await workflowApi.getExecutionStatus(workflowId.value)
      if (statusRes.status === 'running' || statusRes.status === 'completed') {
        execStatus.value = statusRes.status
        statusRes.nodes.forEach((n: any) => {
          nodeExecStates.value[n.nodeId] = n.status
        })
      }
    } catch {
      // No execution history, ignore
    }
  } catch {
    ElMessage.error('加载工作流失败')
  }

  const { on } = useWebSocket()
  on('execution', handleExecutionEvent)
})

onUnmounted(() => {
  const { off } = useWebSocket()
  off('execution', handleExecutionEvent)
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
.editor-body {
  flex: 1;
  display: flex;
  position: relative;
  overflow: hidden;
}
.config-panel {
  width: 320px;
  background: #fff;
  border-left: 1px solid #e4e7ed;
  display: flex;
  flex-direction: column;
  box-shadow: -2px 0 8px rgba(0, 0, 0, 0.05);
  z-index: 10;
}
.panel-header {
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 12px 16px;
  border-bottom: 1px solid #ebeef5;
  font-weight: 600;
  font-size: 14px;
}
.panel-body {
  flex: 1;
  overflow-y: auto;
  padding: 16px;
}
.panel-footer {
  padding: 12px 16px;
  border-top: 1px solid #ebeef5;
}
.slide-enter-active,
.slide-leave-active {
  transition: transform 0.3s ease;
}
.slide-enter-from,
.slide-leave-to {
  transform: translateX(100%);
}
.exec-tag {
  margin-left: 8px;
}
.custom-node {
  background: #fff;
  border-radius: 8px;
  border: 2px solid #dcdfe6;
  padding: 0;
  min-width: 200px;
  box-shadow: 0 2px 12px rgba(0, 0, 0, 0.1);
  cursor: pointer;
  transition: box-shadow 0.2s, border-color 0.3s;
}
.custom-node.selected {
  box-shadow: 0 0 0 2px #409eff, 0 4px 16px rgba(64, 158, 255, 0.3);
}
.custom-node.exec-running {
  border-color: #e6a23c;
  box-shadow: 0 0 0 2px #e6a23c, 0 4px 16px rgba(230, 162, 60, 0.3);
  animation: node-pulse 1.5s ease-in-out infinite;
}
.custom-node.exec-completed {
  border-color: #67c23a;
  box-shadow: 0 0 0 2px #67c23a, 0 2px 8px rgba(103, 194, 58, 0.2);
}
.custom-node.exec-failed {
  border-color: #f56c6c;
  box-shadow: 0 0 0 2px #f56c6c, 0 2px 8px rgba(245, 108, 108, 0.2);
}
@keyframes node-pulse {
  0%, 100% { box-shadow: 0 0 0 2px #e6a23c, 0 4px 16px rgba(230, 162, 60, 0.3); }
  50% { box-shadow: 0 0 0 4px #e6a23c, 0 4px 20px rgba(230, 162, 60, 0.5); }
}
.node-exec-tag {
  margin-left: auto;
  font-size: 10px;
  padding: 0 4px;
  line-height: 16px;
  height: 16px;
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
.node-summary {
  font-size: 12px;
  color: #606266;
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
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
