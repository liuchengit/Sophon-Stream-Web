<template>
  <div class="monitor-panel">
    <div class="header">
      <h2>实时监控</h2>
      <el-tag :type="wsConnected ? 'success' : 'danger'" size="small">
        {{ wsConnected ? '实时连接中' : '未连接' }}
      </el-tag>
    </div>

    <el-row :gutter="20">
      <el-col :span="8">
        <el-card>
          <template #header><span>CPU 使用率</span></template>
          <div ref="cpuChartRef" class="chart-container"></div>
          <div class="metric-value">{{ metrics.cpu_usage }}%</div>
        </el-card>
      </el-col>
      <el-col :span="8">
        <el-card>
          <template #header><span>内存使用率</span></template>
          <div ref="memChartRef" class="chart-container"></div>
          <div class="metric-value">{{ metrics.memory_usage }}%</div>
        </el-card>
      </el-col>
      <el-col :span="8">
        <el-card>
          <template #header><span>GPU/TPU 使用率</span></template>
          <div ref="gpuChartRef" class="chart-container"></div>
          <div class="metric-value">{{ metrics.gpu_usage }}%</div>
        </el-card>
      </el-col>
    </el-row>

    <el-card style="margin-top: 20px">
      <template #header><span>资源趋势 (最近60秒)</span></template>
      <div ref="trendChartRef" class="trend-chart"></div>
    </el-card>

    <el-card style="margin-top: 20px">
      <template #header><span>资源告警</span></template>
      <el-alert v-if="metrics.gpu_usage > 85" title="GPU/TPU 使用率超过 85%" type="warning" show-icon />
      <el-alert v-if="metrics.cpu_usage > 90" title="CPU 使用率超过 90%" type="warning" show-icon />
      <el-alert v-if="metrics.memory_usage > 85" title="内存使用率超过 85%" type="warning" show-icon />
      <el-alert v-if="metrics.gpu_usage <= 85 && metrics.cpu_usage <= 90 && metrics.memory_usage <= 85" title="系统资源正常" type="success" show-icon />
    </el-card>
  </div>
</template>

<script setup lang="ts">
import { ref, onMounted, onUnmounted } from 'vue'
import * as echarts from 'echarts'
import { monitoringApi } from '../../api/monitoring'
import { useWebSocket } from '../../composables/useWebSocket'
import type { ECharts } from 'echarts'

const metrics = ref({
  cpu_usage: 0,
  memory_usage: 0,
  gpu_usage: 0,
})

const cpuChartRef = ref<HTMLElement>()
const memChartRef = ref<HTMLElement>()
const gpuChartRef = ref<HTMLElement>()
const trendChartRef = ref<HTMLElement>()
let cpuChart: ECharts | null = null
let memChart: ECharts | null = null
let gpuChart: ECharts | null = null
let trendChart: ECharts | null = null

const historyData = ref<{ time: string; cpu: number; mem: number; gpu: number }[]>([])

const { isConnected: wsConnected, connect, on, disconnect } = useWebSocket()

onMounted(() => {
  initCharts()
  fetchMetrics()

  const token = localStorage.getItem('token')
  if (token) {
    connect(token)
    on('metrics', (msg) => {
      if (msg.data) {
        const type = msg.data.metricType as string
        const value = Math.round(msg.data.value as number)
        const ts = msg.data.timestamp as string

        if (type === 'cpu_usage') metrics.value.cpu_usage = value
        else if (type === 'memory_usage') metrics.value.memory_usage = value
        else if (type === 'gpu_usage') metrics.value.gpu_usage = value

        const timeStr = new Date(ts).toLocaleTimeString('zh-CN')
        historyData.value.push({ time: timeStr, cpu: metrics.value.cpu_usage, mem: metrics.value.memory_usage, gpu: metrics.value.gpu_usage })
        if (historyData.value.length > 20) historyData.value.shift()

        updateGaugeCharts()
        updateTrendChart()
      }
    })
  }
})

onUnmounted(() => {
  disconnect()
  cpuChart?.dispose()
  memChart?.dispose()
  gpuChart?.dispose()
  trendChart?.dispose()
})

const initCharts = () => {
  const createGauge = (el: HTMLElement | undefined, name: string, color: string) => {
    if (!el) return null
    const chart = echarts.init(el)
    chart.setOption({
      series: [{
        type: 'gauge',
        max: 100,
        detail: { formatter: '{value}%' },
        data: [{ value: 0, name }],
        axisLine: { lineStyle: { color: [[1, color]] } },
      }],
    })
    return chart
  }

  cpuChart = createGauge(cpuChartRef.value, 'CPU', '#67c23a')
  memChart = createGauge(memChartRef.value, '内存', '#e6a23c')
  gpuChart = createGauge(gpuChartRef.value, 'GPU/TPU', '#409eff')

  if (trendChartRef.value) {
    trendChart = echarts.init(trendChartRef.value)
    trendChart.setOption({
      tooltip: { trigger: 'axis' },
      legend: { data: ['CPU', '内存', 'GPU/TPU'] },
      grid: { left: '3%', right: '4%', bottom: '3%', containLabel: true },
      xAxis: { type: 'category', data: [] },
      yAxis: { type: 'value', max: 100, axisLabel: { formatter: '{value}%' } },
      series: [
        { name: 'CPU', type: 'line', data: [], smooth: true, itemStyle: { color: '#67c23a' } },
        { name: '内存', type: 'line', data: [], smooth: true, itemStyle: { color: '#e6a23c' } },
        { name: 'GPU/TPU', type: 'line', data: [], smooth: true, itemStyle: { color: '#409eff' } },
      ],
    })
  }
}

const updateGaugeCharts = () => {
  cpuChart?.setOption({ series: [{ data: [{ value: metrics.value.cpu_usage, name: 'CPU' }] }] })
  memChart?.setOption({ series: [{ data: [{ value: metrics.value.memory_usage, name: '内存' }] }] })
  gpuChart?.setOption({ series: [{ data: [{ value: metrics.value.gpu_usage, name: 'GPU/TPU' }] }] })
}

const updateTrendChart = () => {
  if (!trendChart) return
  trendChart.setOption({
    xAxis: { data: historyData.value.map((d) => d.time) },
    series: [
      { data: historyData.value.map((d) => d.cpu) },
      { data: historyData.value.map((d) => d.mem) },
      { data: historyData.value.map((d) => d.gpu) },
    ],
  })
}

const fetchMetrics = async () => {
  try {
    const data = await monitoringApi.getResources()
    metrics.value = {
      cpu_usage: Math.round(data.cpu_usage || 0),
      memory_usage: Math.round(data.memory_usage || 0),
      gpu_usage: Math.round(data.tpu_usage || 0),
    }
    updateGaugeCharts()
  } catch {
    // API not available, rely on WebSocket
  }
}
</script>

<style scoped>
.monitor-panel {
  padding: 0;
}
.header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 20px;
}
.header h2 {
  margin: 0;
}
.chart-container {
  height: 200px;
}
.trend-chart {
  height: 300px;
}
.metric-value {
  text-align: center;
  font-size: 24px;
  font-weight: bold;
  margin-top: -20px;
}
</style>
