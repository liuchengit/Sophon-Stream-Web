<template>
  <div class="monitor-panel">
    <el-row :gutter="20">
      <el-col :span="8">
        <el-card>
          <template #header><span>TPU 使用率</span></template>
          <div ref="tpuChartRef" class="chart-container"></div>
          <div class="metric-value">{{ metrics.tpu_usage }}%</div>
        </el-card>
      </el-col>
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
    </el-row>

    <el-card style="margin-top: 20px">
      <template #header><span>资源告警</span></template>
      <el-alert v-if="metrics.tpu_usage > 85" title="TPU 使用率超过 85%" type="warning" show-icon />
      <el-alert v-if="metrics.cpu_usage > 90" title="CPU 使用率超过 90%" type="warning" show-icon />
      <el-alert v-if="metrics.memory_usage > 85" title="内存使用率超过 85%" type="warning" show-icon />
      <el-alert v-if="metrics.tpu_usage <= 85 && metrics.cpu_usage <= 90 && metrics.memory_usage <= 85" title="系统资源正常" type="success" show-icon />
    </el-card>
  </div>
</template>

<script setup lang="ts">
import { ref, onMounted, onUnmounted } from 'vue'
import * as echarts from 'echarts'
import { monitoringApi } from '../../api/monitoring'
import type { ECharts } from 'echarts'

const metrics = ref({
  tpu_usage: 0,
  cpu_usage: 0,
  memory_usage: 0,
})

const tpuChartRef = ref<HTMLElement>()
const cpuChartRef = ref<HTMLElement>()
const memChartRef = ref<HTMLElement>()
let tpuChart: ECharts | null = null
let cpuChart: ECharts | null = null
let memChart: ECharts | null = null
let ws: WebSocket | null = null

onMounted(() => {
  initCharts()
  connectWebSocket()
  fetchMetrics()
})

onUnmounted(() => {
  ws?.close()
  tpuChart?.dispose()
  cpuChart?.dispose()
  memChart?.dispose()
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

  tpuChart = createGauge(tpuChartRef.value, 'TPU', '#409eff')
  cpuChart = createGauge(cpuChartRef.value, 'CPU', '#67c23a')
  memChart = createGauge(memChartRef.value, '内存', '#e6a23c')
}

const connectWebSocket = () => {
  const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:'
  ws = new WebSocket(`${protocol}//${window.location.host}/ws/monitoring`)
  ws.onmessage = (event) => {
    const data = JSON.parse(event.data)
    metrics.value = {
      tpu_usage: Math.round(data.tpu_usage),
      cpu_usage: Math.round(data.cpu_usage),
      memory_usage: Math.round(data.memory_usage),
    }
    updateCharts()
  }
  ws.onclose = () => setTimeout(connectWebSocket, 3000)
}

const updateCharts = () => {
  tpuChart?.setOption({ series: [{ data: [{ value: metrics.value.tpu_usage }] }] })
  cpuChart?.setOption({ series: [{ data: [{ value: metrics.value.cpu_usage }] }] })
  memChart?.setOption({ series: [{ data: [{ value: metrics.value.memory_usage }] }] })
}

const fetchMetrics = async () => {
  try {
    const data = await monitoringApi.getResources()
    metrics.value = {
      tpu_usage: Math.round(data.tpu_usage),
      cpu_usage: Math.round(data.cpu_usage),
      memory_usage: Math.round(data.memory_usage),
    }
    updateCharts()
  } catch {
    // API not available
  }
}
</script>

<style scoped>
.chart-container {
  height: 200px;
}

.metric-value {
  text-align: center;
  font-size: 24px;
  font-weight: bold;
  margin-top: -20px;
}
</style>
