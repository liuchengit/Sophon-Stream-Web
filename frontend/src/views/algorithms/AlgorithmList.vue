<template>
  <div class="algorithm-list">
    <el-card>
      <template #header>
        <div class="card-header">
          <span>算法管理</span>
          <el-button type="primary" @click="showInstallDialog = true">安装算法</el-button>
        </div>
      </template>
      <el-table :data="algorithms" stripe>
        <el-table-column prop="name" label="算法名称" />
        <el-table-column prop="version" label="版本" />
        <el-table-column prop="type" label="类型" />
        <el-table-column prop="model_path" label="模型路径" />
        <el-table-column label="操作" width="150">
          <template #default="{ row }">
            <el-button size="small" @click="viewDetail(row)">详情</el-button>
            <el-button size="small" type="danger" @click="uninstall(row.id)">卸载</el-button>
          </template>
        </el-table-column>
      </el-table>
    </el-card>
  </div>
</template>

<script setup lang="ts">
import { ref, onMounted } from 'vue'

interface Algorithm {
  id: number
  name: string
  version: string
  type: string
  model_path: string
}

const algorithms = ref<Algorithm[]>([])
const showInstallDialog = ref(false)

onMounted(() => {
  algorithms.value = [
    { id: 1, name: 'YOLOX', version: '1.0.0', type: 'detection', model_path: '/models/yolox.bmodel' },
    { id: 2, name: 'YOLOv5', version: '1.0.0', type: 'detection', model_path: '/models/yolov5.bmodel' },
    { id: 3, name: 'ByteTrack', version: '1.0.0', type: 'tracking', model_path: '/models/bytetrack.bmodel' },
  ]
})

const viewDetail = (_algo: Algorithm) => {
  // TODO: Show algorithm details
}

const uninstall = async (_id: number) => {
  // TODO: Implement uninstall
}
</script>

<style scoped>
.card-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
}
</style>
