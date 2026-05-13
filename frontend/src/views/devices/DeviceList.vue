<template>
  <div class="device-list">
    <el-card>
      <template #header>
        <div class="card-header">
          <span>设备管理</span>
          <el-button type="primary" @click="showAddDialog = true">添加设备</el-button>
        </div>
      </template>
      <el-table :data="devices" v-loading="loading" stripe>
        <el-table-column prop="name" label="设备名称" />
        <el-table-column prop="type" label="类型" />
        <el-table-column prop="ip_address" label="IP 地址" />
        <el-table-column prop="status" label="状态">
          <template #default="{ row }">
            <el-tag :type="row.status === 'online' ? 'success' : row.status === 'error' ? 'danger' : 'info'">
              {{ row.status }}
            </el-tag>
          </template>
        </el-table-column>
        <el-table-column prop="model" label="型号" />
        <el-table-column label="操作" width="200">
          <template #default="{ row }">
            <el-button size="small" @click="editDevice(row)">编辑</el-button>
            <el-button size="small" type="danger" @click="deleteDevice(row.id)">删除</el-button>
          </template>
        </el-table-column>
      </el-table>
    </el-card>
  </div>
</template>

<script setup lang="ts">
import { ref, onMounted } from 'vue'
import { deviceApi } from '../../api/devices'
import type { Device } from '../../api/devices'

const devices = ref<Device[]>([])
const loading = ref(false)
const showAddDialog = ref(false)

onMounted(() => {
  fetchDevices()
})

const fetchDevices = async () => {
  loading.value = true
  try {
    const result = await deviceApi.list()
    devices.value = result.items
  } catch {
    // API not available
  } finally {
    loading.value = false
  }
}

const editDevice = (_device: Device) => {
  // TODO: Implement edit
}

const deleteDevice = async (_id: number) => {
  // TODO: Implement delete
}
</script>

<style scoped>
.card-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
}
</style>
