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
              <div class="stat-value">{{ stats.devices.total }}</div>
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
              <div class="stat-value">{{ stats.tasks.running }}</div>
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
              <div class="stat-value">{{ stats.alarms.today }}</div>
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
              <div class="stat-value">{{ stats.algorithms.total }}</div>
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
import { dashboardApi } from '../api/dashboard'
import { monitoringApi } from '../api/monitoring'
import type { ECharts } from 'echarts'
import { ElMessage } from 'element-plus'

const stats = ref({
  devices: { total: 0, online: 0, offline: 0 },
  tasks: { total: 0, running: 0, stopped: 0 },
  alarms: { total: 0, today: 0 },
  algorithms: { total: 0, active: 0 },
})

const resourceChartRef = ref<HTMLElement>()
const taskChartRef = ref<HTMLElement>()
let resourceChart: ECharts | null = null
let taskChart: ECharts | null = null
let ws: WebSocket | null = null

onMounted(() => {
  initCharts()
  connectWebSocket()
  fetchStats()
  fetchResourceMetrics()
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
      xAxis: { type: 'category', data: ['TPU', 'CPU', '内存'] },
      yAxis: { type: 'value', max: 100 },
      series: [
        { name: 'TPU', type: 'bar', data: [0], itemStyle: { color: '#409eff' } },
        { name: 'CPU', type: 'bar', data: [0], itemStyle: { color: '#67c23a' } },
        { name: '内存', type: 'bar', data: [0], itemStyle: { color: '#e6a23c' } },
      ],
    })
  }

  if (taskChartRef.value) {
    taskChart = echarts.init(taskChartRef.value)
    taskChart.setOption({
      tooltip: { trigger: 'item' },
      legend: { bottom: '5%' },
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

const updateResourceChart = (data: { tpu_usage: number; cpu_usage: number; memory_usage: number }) => {
  if (!resourceChart) return
  resourceChart.setOption({
    series: [
      { name: 'TPU', data: [data.tpu_usage] },
      { name: 'CPU', data: [data.cpu_usage] },
      { name: '内存', data: [data.memory_usage] },
    ],
  })
}

const fetchStats = async () => {
  try {
    const data = await dashboardApi.getStats()
    stats.value = data
    updateTaskChart()
  } catch {
    ElMessage.error('获取统计数据失败')
  }
}

const updateTaskChart = () => {
  if (!taskChart) return
  taskChart.setOption({
    series: [
      {
        data: [
          { value: stats.value.tasks.running, name: '运行中' },
          { value: stats.value.tasks.stopped, name: '已停止' },
          { value: stats.value.tasks.total - stats.value.tasks.running - stats.value.tasks.stopped, name: '异常' },
        ],
      },
    ],
  })
}

const fetchResourceMetrics = async () => {
  try {
    const data = await monitoringApi.getResources()
    if (resourceChart) {
      resourceChart.setOption({
        series: [
          { name: 'TPU', data: [data.tpu_usage] },
          { name: 'CPU', data: [data.cpu_usage] },
          { name: '内存', data: [data.memory_usage] },
        ],
      })
    }
  } catch {
    ElMessage.error('获取资源使用数据失败')
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
