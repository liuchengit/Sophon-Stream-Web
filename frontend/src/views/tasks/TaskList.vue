<template>
  <div class="task-list">
    <el-card>
      <template #header>
        <div class="card-header">
          <span>任务管理</span>
          <el-button type="primary" @click="handleCreate">创建任务</el-button>
        </div>
      </template>

      <el-table :data="tasks" v-loading="loading" stripe>
        <el-table-column prop="name" label="任务名称" />
        <el-table-column prop="description" label="描述" show-overflow-tooltip />
        <el-table-column prop="device_id" label="设备 ID" width="100" />
        <el-table-column prop="status" label="状态" width="100">
          <template #default="{ row }">
            <el-tag :type="statusType(row.status)">
              {{ statusLabel(row.status) }}
            </el-tag>
          </template>
        </el-table-column>
        <el-table-column prop="schedule_cron" label="定时任务" width="150" />
        <el-table-column label="操作" width="300">
          <template #default="{ row }">
            <el-button v-if="row.status === 'stopped'" size="small" type="success" @click="handleStart(row.id)">启动</el-button>
            <el-button v-if="row.status === 'running'" size="small" type="warning" @click="handleStop(row.id)">停止</el-button>
            <el-button size="small" @click="handleEdit(row)">编辑</el-button>
            <el-button size="small" type="danger" @click="handleDelete(row.id)">删除</el-button>
          </template>
        </el-table-column>
      </el-table>

      <el-pagination
        v-model:current-page="currentPage"
        v-model:page-size="pageSize"
        :total="total"
        :page-sizes="[10, 20, 50]"
        layout="total, sizes, prev, pager, next"
        @size-change="fetchTasks"
        @current-change="fetchTasks"
        class="pagination"
      />
    </el-card>

    <el-dialog
      v-model="dialogVisible"
      :title="isEdit ? '编辑任务' : '创建任务'"
      width="500px"
      @close="resetForm"
    >
      <el-form :model="form" :rules="rules" ref="formRef" label-width="100px">
        <el-form-item label="任务名称" prop="name">
          <el-input v-model="form.name" placeholder="请输入任务名称" />
        </el-form-item>
        <el-form-item label="描述" prop="description">
          <el-input v-model="form.description" type="textarea" :rows="3" placeholder="请输入任务描述" />
        </el-form-item>
        <el-form-item label="设备 ID" prop="device_id">
          <el-input-number v-model="form.device_id" :min="1" placeholder="请输入设备 ID" />
        </el-form-item>
        <el-form-item label="定时任务" prop="schedule_cron">
          <el-input v-model="form.schedule_cron" placeholder="如: 0 */2 * * * (可选)" />
        </el-form-item>
      </el-form>
      <template #footer>
        <el-button @click="dialogVisible = false">取消</el-button>
        <el-button type="primary" @click="handleSubmit" :loading="submitting">确定</el-button>
      </template>
    </el-dialog>
  </div>
</template>

<script setup lang="ts">
import { ref, onMounted } from 'vue'
import { ElMessage, ElMessageBox, type FormInstance, type FormRules } from 'element-plus'
import { taskApi } from '../../api/tasks'
import type { Task } from '../../api/tasks'

const tasks = ref<Task[]>([])
const loading = ref(false)
const submitting = ref(false)
const currentPage = ref(1)
const pageSize = ref(10)
const total = ref(0)
const dialogVisible = ref(false)
const isEdit = ref(false)
const editId = ref<number | null>(null)
const formRef = ref<FormInstance>()

const form = ref({
  name: '',
  description: '',
  device_id: 1,
  schedule_cron: '',
})

const rules = ref<FormRules>({
  name: [{ required: true, message: '请输入任务名称', trigger: 'blur' }],
  device_id: [{ required: true, message: '请输入设备 ID', trigger: 'blur' }],
})

onMounted(() => {
  fetchTasks()
})

const fetchTasks = async () => {
  loading.value = true
  try {
    const result = await taskApi.list({
      page: currentPage.value,
      limit: pageSize.value,
    })
    tasks.value = result.items
    total.value = result.total
  } catch {
    ElMessage.error('获取任务列表失败')
  } finally {
    loading.value = false
  }
}

const statusType = (status: string) => {
  switch (status) {
    case 'running': return 'success'
    case 'stopped': return 'info'
    case 'paused': return 'warning'
    case 'error': return 'danger'
    default: return 'info'
  }
}

const statusLabel = (status: string) => {
  const map: Record<string, string> = {
    running: '运行中',
    stopped: '已停止',
    paused: '已暂停',
    error: '异常',
  }
  return map[status] || status
}

const handleCreate = () => {
  isEdit.value = false
  editId.value = null
  dialogVisible.value = true
}

const handleEdit = (row: Task) => {
  isEdit.value = true
  editId.value = row.id
  form.value = {
    name: row.name,
    description: row.description,
    device_id: row.device_id,
    schedule_cron: row.schedule_cron,
  }
  dialogVisible.value = true
}

const handleStart = async (id: number) => {
  try {
    await taskApi.start(id)
    ElMessage.success('任务已启动')
    fetchTasks()
  } catch {
    ElMessage.error('启动任务失败')
  }
}

const handleStop = async (id: number) => {
  try {
    await taskApi.stop(id)
    ElMessage.success('任务已停止')
    fetchTasks()
  } catch {
    ElMessage.error('停止任务失败')
  }
}

const handleDelete = async (id: number) => {
  try {
    await ElMessageBox.confirm('确定要删除该任务吗？', '提示', {
      confirmButtonText: '确定',
      cancelButtonText: '取消',
      type: 'warning',
    })
    await taskApi.delete(id)
    ElMessage.success('删除成功')
    fetchTasks()
  } catch (error) {
    if (error !== 'cancel') {
      ElMessage.error('删除失败')
    }
  }
}

const handleSubmit = async () => {
  if (!formRef.value) return
  await formRef.value.validate(async (valid) => {
    if (!valid) return
    submitting.value = true
    try {
      const data = {
        name: form.value.name,
        description: form.value.description,
        device_id: form.value.device_id,
        schedule_cron: form.value.schedule_cron || undefined,
      }
      if (isEdit.value && editId.value !== null) {
        await taskApi.update(editId.value, data)
        ElMessage.success('更新成功')
      } else {
        await taskApi.create(data)
        ElMessage.success('创建成功')
      }
      dialogVisible.value = false
      fetchTasks()
    } catch {
      ElMessage.error(isEdit.value ? '更新失败' : '创建失败')
    } finally {
      submitting.value = false
    }
  })
}

const resetForm = () => {
  form.value = {
    name: '',
    description: '',
    device_id: 1,
    schedule_cron: '',
  }
  formRef.value?.resetFields()
}
</script>

<style scoped>
.card-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.pagination {
  margin-top: 16px;
  justify-content: flex-end;
}
</style>
