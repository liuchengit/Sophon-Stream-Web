<template>
  <div class="device-list">
    <el-card>
      <template #header>
        <div class="card-header">
          <span>设备管理</span>
          <el-button type="primary" @click="handleAdd">添加设备</el-button>
        </div>
      </template>

      <div class="filter-bar">
        <el-select v-model="statusFilter" placeholder="状态筛选" clearable @change="fetchDevices" style="width: 150px">
          <el-option label="全部" value="" />
          <el-option label="在线" value="online" />
          <el-option label="离线" value="offline" />
          <el-option label="异常" value="error" />
        </el-select>
      </div>

      <el-table :data="devices" v-loading="loading" stripe>
        <el-table-column prop="name" label="设备名称" />
        <el-table-column prop="type" label="类型" />
        <el-table-column prop="ip_address" label="IP 地址" />
        <el-table-column prop="port" label="端口" />
        <el-table-column prop="status" label="状态">
          <template #default="{ row }">
            <el-tag :type="row.status === 'online' ? 'success' : row.status === 'error' ? 'danger' : 'info'">
              {{ row.status === 'online' ? '在线' : row.status === 'offline' ? '离线' : '异常' }}
            </el-tag>
          </template>
        </el-table-column>
        <el-table-column prop="model" label="型号" />
        <el-table-column prop="firmware_version" label="固件版本" />
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
        @size-change="fetchDevices"
        @current-change="fetchDevices"
        class="pagination"
      />
    </el-card>

    <el-dialog
      v-model="dialogVisible"
      :title="isEdit ? '编辑设备' : '添加设备'"
      width="500px"
      @close="resetForm"
    >
      <el-form :model="form" :rules="rules" ref="formRef" label-width="100px">
        <el-form-item label="设备名称" prop="name">
          <el-input v-model="form.name" placeholder="请输入设备名称" />
        </el-form-item>
        <el-form-item label="类型" prop="type">
          <el-input v-model="form.type" placeholder="请输入设备类型" />
        </el-form-item>
        <el-form-item label="IP 地址" prop="ip_address">
          <el-input v-model="form.ip_address" placeholder="请输入 IP 地址" />
        </el-form-item>
        <el-form-item label="端口" prop="port">
          <el-input-number v-model="form.port" :min="1" :max="65535" />
        </el-form-item>
        <el-form-item label="型号" prop="model">
          <el-input v-model="form.model" placeholder="请输入型号" />
        </el-form-item>
        <el-form-item label="固件版本" prop="firmware_version">
          <el-input v-model="form.firmware_version" placeholder="请输入固件版本" />
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
import { deviceApi } from '../../api/devices'
import type { Device } from '../../api/devices'

const devices = ref<Device[]>([])
const loading = ref(false)
const submitting = ref(false)
const currentPage = ref(1)
const pageSize = ref(10)
const total = ref(0)
const statusFilter = ref('')
const dialogVisible = ref(false)
const isEdit = ref(false)
const editId = ref<number | null>(null)
const formRef = ref<FormInstance>()

const form = ref({
  name: '',
  type: '',
  ip_address: '',
  port: 80,
  model: '',
  firmware_version: '',
})

const rules = ref<FormRules>({
  name: [{ required: true, message: '请输入设备名称', trigger: 'blur' }],
  type: [{ required: true, message: '请输入设备类型', trigger: 'blur' }],
  ip_address: [
    { required: true, message: '请输入 IP 地址', trigger: 'blur' },
    { pattern: /^\d{1,3}(\.\d{1,3}){3}$/, message: '请输入有效的 IP 地址', trigger: 'blur' },
  ],
  port: [{ required: true, message: '请输入端口号', trigger: 'blur' }],
})

onMounted(() => {
  fetchDevices()
})

const fetchDevices = async () => {
  loading.value = true
  try {
    const params: Record<string, string | number> = {
      page: currentPage.value,
      limit: pageSize.value,
    }
    if (statusFilter.value) {
      params.status = statusFilter.value
    }
    const result = await deviceApi.list(params as { status?: string; page?: number; limit?: number })
    devices.value = result.items
    total.value = result.total
  } catch {
    ElMessage.error('获取设备列表失败')
  } finally {
    loading.value = false
  }
}

const handleAdd = () => {
  isEdit.value = false
  editId.value = null
  dialogVisible.value = true
}

const handleEdit = (row: Device) => {
  isEdit.value = true
  editId.value = row.id
  form.value = {
    name: row.name,
    type: row.type,
    ip_address: row.ip_address,
    port: row.port,
    model: row.model,
    firmware_version: row.firmware_version,
  }
  dialogVisible.value = true
}

const handleDelete = async (id: number) => {
  try {
    await ElMessageBox.confirm('确定要删除该设备吗？', '提示', {
      confirmButtonText: '确定',
      cancelButtonText: '取消',
      type: 'warning',
    })
    await deviceApi.delete(id)
    ElMessage.success('删除成功')
    fetchDevices()
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
        await deviceApi.update(editId.value, form.value)
        ElMessage.success('更新成功')
      } else {
        await deviceApi.create(form.value)
        ElMessage.success('添加成功')
      }
      dialogVisible.value = false
      fetchDevices()
    } catch {
      ElMessage.error(isEdit.value ? '更新失败' : '添加失败')
    } finally {
      submitting.value = false
    }
  })
}

const resetForm = () => {
  form.value = {
    name: '',
    type: '',
    ip_address: '',
    port: 80,
    model: '',
    firmware_version: '',
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

.filter-bar {
  margin-bottom: 16px;
}

.pagination {
  margin-top: 16px;
  justify-content: flex-end;
}
</style>
