<template>
  <div class="algorithm-list">
    <el-card>
      <template #header>
        <div class="card-header">
          <span>算法管理</span>
          <el-button type="primary" @click="handleInstall">安装算法</el-button>
        </div>
      </template>

      <el-table :data="algorithms" v-loading="loading" stripe>
        <el-table-column prop="name" label="算法名称" />
        <el-table-column prop="version" label="版本" width="120" />
        <el-table-column prop="type" label="类型" width="120" />
        <el-table-column prop="model_path" label="模型路径" show-overflow-tooltip />
        <el-table-column prop="status" label="状态" width="100">
          <template #default="{ row }">
            <el-tag :type="row.status === 'active' ? 'success' : row.status === 'error' ? 'danger' : 'info'">
              {{ statusLabel(row.status) }}
            </el-tag>
          </template>
        </el-table-column>
        <el-table-column label="操作" width="200">
          <template #default="{ row }">
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
        @size-change="fetchAlgorithms"
        @current-change="fetchAlgorithms"
        class="pagination"
      />
    </el-card>

    <el-dialog
      v-model="dialogVisible"
      :title="isEdit ? '编辑算法' : '安装算法'"
      width="500px"
      @close="resetForm"
    >
      <el-form :model="form" :rules="rules" ref="formRef" label-width="100px">
        <el-form-item label="算法名称" prop="name">
          <el-input v-model="form.name" placeholder="请输入算法名称" />
        </el-form-item>
        <el-form-item label="版本" prop="version">
          <el-input v-model="form.version" placeholder="如: 1.0.0" />
        </el-form-item>
        <el-form-item label="类型" prop="type">
          <el-select v-model="form.type" placeholder="请选择类型" style="width: 100%">
            <el-option label="检测 (detection)" value="detection" />
            <el-option label="跟踪 (tracking)" value="tracking" />
            <el-option label="分类 (classification)" value="classification" />
            <el-option label="分割 (segmentation)" value="segmentation" />
          </el-select>
        </el-form-item>
        <el-form-item label="模型路径" prop="model_path">
          <el-input v-model="form.model_path" placeholder="如: /models/yolox.bmodel" />
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
import { algorithmApi } from '../../api/algorithms'
import type { Algorithm } from '../../api/algorithms'

const algorithms = ref<Algorithm[]>([])
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
  version: '',
  type: '',
  model_path: '',
})

const rules = ref<FormRules>({
  name: [{ required: true, message: '请输入算法名称', trigger: 'blur' }],
  version: [{ required: true, message: '请输入版本号', trigger: 'blur' }],
  type: [{ required: true, message: '请选择算法类型', trigger: 'change' }],
  model_path: [{ required: true, message: '请输入模型路径', trigger: 'blur' }],
})

onMounted(() => {
  fetchAlgorithms()
})

const fetchAlgorithms = async () => {
  loading.value = true
  try {
    const result = await algorithmApi.list({
      page: currentPage.value,
      limit: pageSize.value,
    })
    algorithms.value = result.items
    total.value = result.total
  } catch {
    ElMessage.error('获取算法列表失败')
  } finally {
    loading.value = false
  }
}

const statusLabel = (status: string) => {
  const map: Record<string, string> = {
    active: '已激活',
    inactive: '未激活',
    error: '异常',
  }
  return map[status] || status
}

const handleInstall = () => {
  isEdit.value = false
  editId.value = null
  dialogVisible.value = true
}

const handleEdit = (row: Algorithm) => {
  isEdit.value = true
  editId.value = row.id
  form.value = {
    name: row.name,
    version: row.version,
    type: row.type,
    model_path: row.model_path,
  }
  dialogVisible.value = true
}

const handleDelete = async (id: number) => {
  try {
    await ElMessageBox.confirm('确定要删除该算法吗？', '提示', {
      confirmButtonText: '确定',
      cancelButtonText: '取消',
      type: 'warning',
    })
    await algorithmApi.delete(id)
    ElMessage.success('删除成功')
    fetchAlgorithms()
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
      if (isEdit.value && editId.value !== null) {
        await algorithmApi.update(editId.value, form.value)
        ElMessage.success('更新成功')
      } else {
        await algorithmApi.create(form.value as Omit<Algorithm, 'id' | 'created_at'>)
        ElMessage.success('安装成功')
      }
      dialogVisible.value = false
      fetchAlgorithms()
    } catch {
      ElMessage.error(isEdit.value ? '更新失败' : '安装失败')
    } finally {
      submitting.value = false
    }
  })
}

const resetForm = () => {
  form.value = {
    name: '',
    version: '',
    type: '',
    model_path: '',
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
