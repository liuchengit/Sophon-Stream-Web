<template>
  <div class="task-list">
    <el-card>
      <template #header>
        <div class="card-header">
          <span>任务管理</span>
          <el-button type="primary" @click="$router.push('/tasks/new')">创建任务</el-button>
        </div>
      </template>
      <el-table :data="tasks" v-loading="loading" stripe>
        <el-table-column prop="name" label="任务名称" />
        <el-table-column prop="status" label="状态">
          <template #default="{ row }">
            <el-tag :type="statusType(row.status)">{{ row.status }}</el-tag>
          </template>
        </el-table-column>
        <el-table-column prop="schedule_cron" label="定时任务" />
        <el-table-column label="操作" width="300">
          <template #default="{ row }">
            <el-button size="small" @click="$router.push(`/tasks/${row.id}/editor`)">编排</el-button>
            <el-button v-if="row.status === 'stopped'" size="small" type="success" @click="startTask(row.id)">启动</el-button>
            <el-button v-if="row.status === 'running'" size="small" type="warning" @click="stopTask(row.id)">停止</el-button>
            <el-button size="small" @click="editTask(row)">编辑</el-button>
            <el-button size="small" type="danger" @click="deleteTask(row.id)">删除</el-button>
          </template>
        </el-table-column>
      </el-table>
    </el-card>
  </div>
</template>

<script setup lang="ts">
import { ref, onMounted } from 'vue'
import { taskApi } from '../../api/tasks'
import type { Task } from '../../api/tasks'

const tasks = ref<Task[]>([])
const loading = ref(false)

onMounted(() => {
  fetchTasks()
})

const fetchTasks = async () => {
  loading.value = true
  try {
    const result = await taskApi.list()
    tasks.value = result.items
  } catch {
    // API not available
  } finally {
    loading.value = false
  }
}

const statusType = (status: string) => {
  switch (status) {
    case 'running': return 'success'
    case 'paused': return 'warning'
    case 'error': return 'danger'
    default: return 'info'
  }
}

const startTask = async (id: number) => {
  try {
    await taskApi.start(id)
    fetchTasks()
  } catch {
    // Error handled by interceptor
  }
}

const stopTask = async (id: number) => {
  try {
    await taskApi.stop(id)
    fetchTasks()
  } catch {
    // Error handled by interceptor
  }
}

const editTask = (_task: Task) => {
  // TODO: Implement edit
}

const deleteTask = async (id: number) => {
  try {
    await taskApi.delete(id)
    fetchTasks()
  } catch {
    // Error handled by interceptor
  }
}
</script>

<style scoped>
.card-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
}
</style>
