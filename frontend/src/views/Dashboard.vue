<template>
  <div class="dashboard">
    <el-row :gutter="20" class="stats-row">
      <el-col :span="6">
        <el-card shadow="hover">
          <div class="stat-card">
            <div class="stat-icon" style="background-color: #409eff">
              <el-icon :size="30"><Monitor /></el-icon>
            </div>
            <div class="stat-info">
              <div class="stat-value">{{ deviceCount }}</div>
              <div class="stat-label">设备总数</div>
            </div>
          </div>
        </el-card>
      </el-col>
      <el-col :span="6">
        <el-card shadow="hover">
          <div class="stat-card">
            <div class="stat-icon" style="background-color: #67c23a">
              <el-icon :size="30"><VideoCamera /></el-icon>
            </div>
            <div class="stat-info">
              <div class="stat-value">{{ runningTaskCount }}</div>
              <div class="stat-label">运行任务</div>
            </div>
          </div>
        </el-card>
      </el-col>
      <el-col :span="6">
        <el-card shadow="hover">
          <div class="stat-card">
            <div class="stat-icon" style="background-color: #e6a23c">
              <el-icon :size="30"><Warning /></el-icon>
            </div>
            <div class="stat-info">
              <div class="stat-value">{{ alarmCount }}</div>
              <div class="stat-label">今日报警</div>
            </div>
          </div>
        </el-card>
      </el-col>
      <el-col :span="6">
        <el-card shadow="hover">
          <div class="stat-card">
            <div class="stat-icon" style="background-color: #f56c6c">
              <el-icon :size="30"><Cpu /></el-icon>
            </div>
            <div class="stat-info">
              <div class="stat-value">{{ algorithmCount }}</div>
              <div class="stat-label">算法插件</div>
            </div>
          </div>
        </el-card>
      </el-col>
    </el-row>

    <el-row :gutter="20" class="charts-row">
      <el-col :span="12">
        <el-card>
          <template #header>
            <div class="card-header">
              <span>资源使用率</span>
            </div>
          </template>
          <div ref="resourceChartRef" class="chart-container"></div>
        </el-card>
      </el-col>
      <el-col :span="12">
        <el-card>
          <template #header>
            <div class="card-header">
              <span>任务状态分布</span>
            </div>
          </template>
          <div ref="taskChartRef" class="chart-container"></div>
        </el-card>
      </el-col>
    </el-row>
  </div>
</template>

<script setup lang="ts">
import { ref, onMounted, onUnmounted } from 'vue'
import * as echarts from 'echarts'
import { Monitor, VideoCamera, Warning, Cpu } from '@element-plus/icons-vue'
import { monitoringApi } from '../api/monitoring'
import type { ECharts } from 'echarts'

const deviceCount = ref(0)
const runningTaskCount = ref(0)
const alarmCount = ref(0)
const algorithmCount = ref(0)

const resourceChartRef = ref<HTMLElement>()
const taskChartRef = ref<HTMLElement>()
let resourceChart: ECharts | null = null
let taskChart: ECharts | null = null
let ws: WebSocket | null = null

onMounted(() => {
  initCharts()
  connectWebSocket()
  fetchStats()
})

onUnmounted(() => {
  ws?.close()
  resourceChart?.dispose()
  taskChart?.dispose()
})

const initCharts = () => {
  if (resourceChartRef.value) {
    resourceChart = echarts.init(resourceChartRef.value)
    resourceChart.setOption({
      tooltip: { trigger: 'axis' },
      legend: { data: ['TPU', 'CPU', '内存'] },
      xAxis: { type: 'category', data: [] },
      yAxis: { type: 'value', max: 100 },
      series: [
        { name: 'TPU', type: 'line', data: [], smooth: true, lineStyle: { color: '#409eff' } },
        { name: 'CPU', type: 'line', data: [], smooth: true, lineStyle: { color: '#67c23a' } },
        { name: '内存', type: 'line', data: [], smooth: true, lineStyle: { color: '#e6a23c' } },
      ],
    })
  }

  if (taskChartRef.value) {
    taskChart = echarts.init(taskChartRef.value)
    taskChart.setOption({
      tooltip: { trigger: 'item' },
      series: [
        {
          type: 'pie',
          radius: ['40%', '70%'],
          data: [
            { value: 0, name: '运行中' },
            { value: 0, name: '已停止' },
            { value: 0, name: '异常' },
          ],
        },
      ],
    })
  }
}

const connectWebSocket = () => {
  const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:'
  const wsUrl = `${protocol}//${window.location.host}/ws/monitoring`
  ws = new WebSocket(wsUrl)

  ws.onmessage = (event) => {
    const data = JSON.parse(event.data)
    updateResourceChart(data)
  }

  ws.onclose = () => {
    setTimeout(connectWebSocket, 3000)
  }
}

const updateResourceChart = (_data: { tpu_usage: number; cpu_usage: number; memory_usage: number }) => {
  if (!resourceChart) return
  const option = resourceChart.getOption() as any
  const xAxis = option?.xAxis?.[0]
  const time = new Date().toLocaleTimeString()

  if (xAxis?.data) {
    xAxis.data.push(time)
    if (xAxis.data.length > 20) xAxis.data.shift()
  }
}

const fetchStats = async () => {
  try {
    await monitoringApi.getResources()
    deviceCount.value = 0
    runningTaskCount.value = 0
    alarmCount.value = 0
    algorithmCount.value = 0
  } catch {
    // API not available during development
  }
}
</script>

<style scoped>
.dashboard {
  padding: 10px;
}

.stats-row {
  margin-bottom: 20px;
}

.stat-card {
  display: flex;
  align-items: center;
  gap: 16px;
}

.stat-icon {
  width: 60px;
  height: 60px;
  border-radius: 8px;
  display: flex;
  align-items: center;
  justify-content: center;
  color: #fff;
}

.stat-info {
  flex: 1;
}

.stat-value {
  font-size: 28px;
  font-weight: bold;
  color: #303133;
}

.stat-label {
  font-size: 14px;
  color: #909399;
}

.charts-row {
  margin-top: 20px;
}

.chart-container {
  height: 300px;
}

.card-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
}
</style>
