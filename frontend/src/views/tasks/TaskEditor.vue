<template>
  <div class="task-editor">
    <div class="header">
      <el-button @click="goBack">
        <el-icon><ArrowLeft /></el-icon>
        返回
      </el-button>
      <h2>任务编排 - {{ taskName }}</h2>
      <el-button type="primary" @click="saveTask" :loading="saving">保存</el-button>
    </div>

    <el-row :gutter="20">
      <el-col :span="8">
        <el-card>
          <template #header><span>基本信息</span></template>
          <el-form :model="taskForm" label-width="100px">
            <el-form-item label="任务名称">
              <el-input v-model="taskForm.name" />
            </el-form-item>
            <el-form-item label="描述">
              <el-input v-model="taskForm.description" type="textarea" :rows="3" />
            </el-form-item>
            <el-form-item label="关联设备">
              <el-select v-model="taskForm.device_id" placeholder="选择设备" style="width: 100%">
                <el-option v-for="d in devices" :key="d.id" :label="d.name" :value="d.id" />
              </el-select>
            </el-form-item>
            <el-form-item label="关联工作流">
              <el-select v-model="taskForm.workflow_id" placeholder="选择工作流" style="width: 100%">
                <el-option v-for="w in workflows" :key="w.id" :label="w.name" :value="w.id" />
              </el-select>
            </el-form-item>
            <el-form-item label="定时任务">
              <el-input v-model="taskForm.schedule_cron" placeholder="如: */5 * * * *" />
            </el-form-item>
          </el-form>
        </el-card>
      </el-col>

      <el-col :span="16">
        <el-card>
          <template #header>
            <div class="card-header">
              <span>ROI 区域设置</span>
              <div>
                <el-button size="small" @click="clearRoi">清除</el-button>
                <el-button size="small" type="primary" @click="addRoiPoint">添加顶点</el-button>
              </div>
            </div>
          </template>
          <div class="roi-canvas-wrapper">
            <canvas ref="roiCanvasRef" width="800" height="450" @mousedown="onCanvasClick" @contextmenu.prevent></canvas>
          </div>
          <div class="roi-info">
            <span>已绘制 {{ roiPoints.length }} 个顶点</span>
            <el-tag v-if="roiPoints.length >= 3" type="success" size="small">ROI 区域已闭合</el-tag>
            <el-tag v-else type="info" size="small">至少需要 3 个顶点</el-tag>
          </div>
        </el-card>
      </el-col>
    </el-row>
  </div>
</template>

<script setup lang="ts">
import { ref, onMounted, watch } from 'vue'
import { useRouter, useRoute } from 'vue-router'
import { ElMessage } from 'element-plus'
import { ArrowLeft } from '@element-plus/icons-vue'
import { taskApi, type Task } from '@/api/tasks'
import { deviceApi } from '@/api/devices'
import { workflowApi, type Workflow } from '@/api/workflows'

const router = useRouter()
const route = useRoute()
const taskId = ref(Number(route.params.id))

const taskName = ref('新任务')
const saving = ref(false)
const devices = ref<Array<{ id: number; name: string }>>([])
const workflows = ref<Workflow[]>([])

const taskForm = ref({
  name: '',
  description: '',
  device_id: 0,
  workflow_id: 0,
  schedule_cron: '',
  graph_config: '',
})

const roiCanvasRef = ref<HTMLCanvasElement>()
const roiPoints = ref<{ x: number; y: number }[]>([])

const goBack = () => {
  router.push('/tasks')
}

const saveTask = async () => {
  if (!taskForm.value.name.trim()) {
    ElMessage.warning('请输入任务名称')
    return
  }

  saving.value = true
  try {
    const config = JSON.stringify({
      workflow_id: taskForm.value.workflow_id,
      roi_points: roiPoints.value,
    })

    const taskData: Partial<Task> = {
      name: taskForm.value.name,
      description: taskForm.value.description,
      device_id: taskForm.value.device_id,
      schedule_cron: taskForm.value.schedule_cron,
      graph_config: config,
    }

    if (taskId.value > 0) {
      await taskApi.update(taskId.value, taskData)
      ElMessage.success('更新成功')
    } else {
      const res = await taskApi.create(taskData as any)
      taskId.value = res.id
      ElMessage.success('创建成功')
    }
  } catch {
    ElMessage.error('保存失败')
  } finally {
    saving.value = false
  }
}

const onCanvasClick = (e: MouseEvent) => {
  const canvas = roiCanvasRef.value
  if (!canvas) return
  const rect = canvas.getBoundingClientRect()
  const x = Math.round((e.clientX - rect.left) * (canvas.width / rect.width))
  const y = Math.round((e.clientY - rect.top) * (canvas.height / rect.height))
  roiPoints.value.push({ x, y })
  drawRoi()
}

const addRoiPoint = () => {
  roiPoints.value.push({ x: Math.random() * 700 + 50, y: Math.random() * 350 + 50 })
  drawRoi()
}

const clearRoi = () => {
  roiPoints.value = []
  drawRoi()
}

const drawRoi = () => {
  const canvas = roiCanvasRef.value
  if (!canvas) return
  const ctx = canvas.getContext('2d')
  if (!ctx) return

  ctx.clearRect(0, 0, canvas.width, canvas.height)

  // Draw grid
  ctx.strokeStyle = '#eee'
  for (let x = 0; x < canvas.width; x += 50) {
    ctx.beginPath(); ctx.moveTo(x, 0); ctx.lineTo(x, canvas.height); ctx.stroke()
  }
  for (let y = 0; y < canvas.height; y += 50) {
    ctx.beginPath(); ctx.moveTo(0, y); ctx.lineTo(canvas.width, y); ctx.stroke()
  }

  if (roiPoints.value.length === 0) {
    ctx.fillStyle = '#999'
    ctx.font = '14px sans-serif'
    ctx.textAlign = 'center'
    ctx.fillText('点击画布添加 ROI 顶点', canvas.width / 2, canvas.height / 2)
    return
  }

  // Draw ROI polygon
  ctx.beginPath()
  ctx.moveTo(roiPoints.value[0].x, roiPoints.value[0].y)
  for (let i = 1; i < roiPoints.value.length; i++) {
    ctx.lineTo(roiPoints.value[i].x, roiPoints.value[i].y)
  }
  ctx.closePath()

  ctx.fillStyle = 'rgba(64, 158, 255, 0.15)'
  ctx.fill()
  ctx.strokeStyle = '#409EFF'
  ctx.lineWidth = 2
  ctx.stroke()

  // Draw vertices
  roiPoints.value.forEach((p, i) => {
    ctx.beginPath()
    ctx.arc(p.x, p.y, 5, 0, Math.PI * 2)
    ctx.fillStyle = '#409EFF'
    ctx.fill()
    ctx.strokeStyle = '#fff'
    ctx.lineWidth = 2
    ctx.stroke()

    ctx.fillStyle = '#333'
    ctx.font = '12px sans-serif'
    ctx.textAlign = 'center'
    ctx.fillText(`${i + 1}`, p.x, p.y - 10)
  })
}

onMounted(async () => {
  try {
    const [devRes, wfRes] = await Promise.all([
      deviceApi.list({ page: 1, limit: 100 }),
      workflowApi.list({ page: 1, limit: 100 }),
    ])

    devices.value = devRes.items || []
    workflows.value = wfRes.items || []

    if (taskId.value > 0) {
      const task = await taskApi.get(taskId.value)
      taskName.value = task.name
      taskForm.value = {
        name: task.name,
        description: task.description,
        device_id: task.device_id,
        workflow_id: 0,
        schedule_cron: task.schedule_cron || '',
        graph_config: task.graph_config || '',
      }

      try {
        const config = JSON.parse(task.graph_config || '{}')
        if (config.roi_points) roiPoints.value = config.roi_points
        if (config.workflow_id) taskForm.value.workflow_id = config.workflow_id
      } catch {
        // Invalid JSON, ignore
      }
    }
  } catch {
    ElMessage.error('加载任务数据失败')
  }

  drawRoi()
})

watch(roiPoints, () => {
  drawRoi()
})
</script>

<style scoped>
.task-editor {
  padding: 0;
}
.header {
  display: flex;
  align-items: center;
  gap: 16px;
  margin-bottom: 20px;
}
.header h2 {
  margin: 0;
  flex: 1;
}
.card-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
}
.roi-canvas-wrapper {
  border: 1px solid #dcdfe6;
  border-radius: 4px;
  overflow: hidden;
  display: flex;
  justify-content: center;
}
canvas {
  cursor: crosshair;
  display: block;
  max-width: 100%;
  height: auto;
}
.roi-info {
  margin-top: 12px;
  display: flex;
  align-items: center;
  gap: 12px;
  font-size: 14px;
  color: #606266;
}
</style>
