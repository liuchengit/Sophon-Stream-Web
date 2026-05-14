<template>
  <div class="workflow-list">
    <div class="header">
      <h2>工作流管理</h2>
      <el-button type="primary" @click="showCreateDialog">新建工作流</el-button>
    </div>

    <el-table :data="workflows" v-loading="loading" stripe>
      <el-table-column prop="id" label="ID" width="80" />
      <el-table-column prop="name" label="名称" min-width="150" />
      <el-table-column prop="description" label="描述" min-width="200" />
      <el-table-column prop="status" label="状态" width="100">
        <template #default="{ row }">
          <el-tag :type="statusType(row.status)">{{ statusLabel(row.status) }}</el-tag>
        </template>
      </el-table-column>
      <el-table-column prop="updated_at" label="更新时间" width="180" />
      <el-table-column label="操作" width="200" fixed="right">
        <template #default="{ row }">
          <el-button size="small" @click="openEditor(row)">编辑</el-button>
          <el-button size="small" type="danger" @click="handleDelete(row)">删除</el-button>
        </template>
      </el-table-column>
    </el-table>

    <el-pagination
      v-model:current-page="page"
      :page-size="limit"
      :total="total"
      layout="total, prev, pager, next"
      class="pagination"
      @current-change="loadWorkflows"
    />

    <!-- Create Dialog -->
    <el-dialog v-model="createVisible" title="新建工作流" width="500px">
      <el-form :model="form" label-width="80px">
        <el-form-item label="名称">
          <el-input v-model="form.name" placeholder="输入工作流名称" />
        </el-form-item>
        <el-form-item label="描述">
          <el-input v-model="form.description" type="textarea" placeholder="输入描述" />
        </el-form-item>
      </el-form>
      <template #footer>
        <el-button @click="createVisible = false">取消</el-button>
        <el-button type="primary" @click="handleCreate" :loading="creating">创建</el-button>
      </template>
    </el-dialog>
  </div>
</template>

<script setup lang="ts">
import { ref, onMounted } from 'vue'
import { useRouter } from 'vue-router'
import { ElMessage, ElMessageBox } from 'element-plus'
import { getWorkflows, createWorkflow, deleteWorkflow, type Workflow } from '@/api/workflows'

const router = useRouter()
const workflows = ref<Workflow[]>([])
const loading = ref(false)
const page = ref(1)
const limit = ref(20)
const total = ref(0)

const createVisible = ref(false)
const creating = ref(false)
const form = ref({ name: '', description: '' })

const statusType = (status: string) => {
  const map: Record<string, string> = { draft: '', running: 'success', stopped: 'warning' }
  return map[status] || 'info'
}

const statusLabel = (status: string) => {
  const map: Record<string, string> = { draft: '草稿', running: '运行中', stopped: '已停止' }
  return map[status] || status
}

const loadWorkflows = async () => {
  loading.value = true
  try {
    const res = await getWorkflows({ page: page.value, limit: limit.value })
    workflows.value = res.data.list
    total.value = res.data.total
  } catch {
    ElMessage.error('加载工作流失败')
  } finally {
    loading.value = false
  }
}

const showCreateDialog = () => {
  form.value = { name: '', description: '' }
  createVisible.value = true
}

const handleCreate = async () => {
  if (!form.value.name.trim()) {
    ElMessage.warning('请输入工作流名称')
    return
  }
  creating.value = true
  try {
    await createWorkflow(form.value)
    ElMessage.success('创建成功')
    createVisible.value = false
    loadWorkflows()
  } catch {
    ElMessage.error('创建失败')
  } finally {
    creating.value = false
  }
}

const openEditor = (wf: Workflow) => {
  router.push(`/workflow/${wf.id}`)
}

const handleDelete = async (wf: Workflow) => {
  try {
    await ElMessageBox.confirm(`确定删除工作流 "${wf.name}"？`, '确认删除', { type: 'warning' })
    await deleteWorkflow(wf.id)
    ElMessage.success('删除成功')
    loadWorkflows()
  } catch {
    // User cancelled or error
  }
}

onMounted(loadWorkflows)
</script>

<style scoped>
.workflow-list {
  padding: 20px;
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
.pagination {
  margin-top: 16px;
  justify-content: flex-end;
}
</style>
