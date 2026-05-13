<template>
  <div class="task-editor">
    <el-card>
      <template #header>
        <div class="card-header">
          <span>任务编排</span>
          <div>
            <el-button @click="saveDag">保存</el-button>
          </div>
        </div>
      </template>
      <div class="dag-container">
        <div class="node-panel">
          <h4>节点库</h4>
          <div class="node-item" draggable="true">解码器</div>
          <div class="node-item" draggable="true">YOLOX 检测</div>
          <div class="node-item" draggable="true">ByteTrack 跟踪</div>
          <div class="node-item" draggable="true">OSD 绘制</div>
          <div class="node-item" draggable="true">编码器</div>
        </div>
        <div class="canvas-area">
          <p>DAG 编排区域（集成 Vue Flow）</p>
        </div>
      </div>
    </el-card>

    <el-card class="roi-card">
      <template #header>
        <span>ROI 区域绘制</span>
      </template>
      <div class="roi-canvas-container">
        <canvas ref="roiCanvasRef" width="640" height="480"></canvas>
      </div>
      <el-button type="primary" @click="saveRoi" style="margin-top: 10px">保存 ROI 配置</el-button>
    </el-card>
  </div>
</template>

<script setup lang="ts">
import { ref, onMounted } from 'vue'
import { useRoute } from 'vue-router'

const route = useRoute()
const roiCanvasRef = ref<HTMLCanvasElement>()

onMounted(() => {
  initRoiCanvas()
})

const initRoiCanvas = () => {
  const canvas = roiCanvasRef.value
  if (!canvas) return
  const ctx = canvas.getContext('2d')
  if (!ctx) return
  ctx.fillStyle = '#f0f0f0'
  ctx.fillRect(0, 0, canvas.width, canvas.height)
  ctx.fillStyle = '#999'
  ctx.font = '16px Arial'
  ctx.fillText('视频预览区域', 250, 240)
}

const saveDag = () => {
  // TODO: Save DAG configuration
}

const saveRoi = () => {
  // TODO: Save ROI configuration
}
</script>

<style scoped>
.card-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.dag-container {
  display: flex;
  gap: 20px;
  height: 500px;
}

.node-panel {
  width: 150px;
  padding: 10px;
  background: #f5f7fa;
  border-radius: 4px;
}

.node-item {
  padding: 8px 12px;
  margin: 8px 0;
  background: #fff;
  border: 1px solid #dcdfe6;
  border-radius: 4px;
  cursor: move;
  text-align: center;
}

.canvas-area {
  flex: 1;
  background: #fafafa;
  border: 2px dashed #dcdfe6;
  border-radius: 4px;
  display: flex;
  align-items: center;
  justify-content: center;
}

.roi-card {
  margin-top: 20px;
}

.roi-canvas-container {
  border: 1px solid #dcdfe6;
  display: inline-block;
}
</style>
