<template>
  <div class="alarm-list">
    <el-tabs v-model="activeTab">
      <el-tab-pane label="报警规则" name="rules">
        <el-card>
          <template #header>
            <div class="card-header">
              <span>报警规则</span>
              <el-button type="primary" @click="handleAddRule">添加规则</el-button>
            </div>
          </template>
          <el-table :data="rules" v-loading="rulesLoading" stripe>
            <el-table-column prop="name" label="规则名称" />
            <el-table-column prop="condition_expr" label="触发条件" show-overflow-tooltip />
            <el-table-column prop="debounce_seconds" label="防抖动(秒)" width="120" />
            <el-table-column prop="notification_channels" label="通知渠道" width="150" />
            <el-table-column prop="enabled" label="启用" width="80">
              <template #default="{ row }">
                <el-switch v-model="row.enabled" @change="handleToggleRule(row)" />
              </template>
            </el-table-column>
            <el-table-column label="操作" width="150">
              <template #default="{ row }">
                <el-button size="small" @click="handleEditRule(row)">编辑</el-button>
                <el-button size="small" type="danger" @click="handleDeleteRule(row.id)">删除</el-button>
              </template>
            </el-table-column>
          </el-table>
        </el-card>
      </el-tab-pane>
      <el-tab-pane label="报警历史" name="events">
        <el-card>
          <el-table :data="events" v-loading="eventsLoading" stripe>
            <el-table-column prop="rule_id" label="规则 ID" width="100" />
            <el-table-column prop="task_id" label="任务 ID" width="100" />
            <el-table-column prop="evidence_path" label="证据路径" show-overflow-tooltip />
            <el-table-column prop="triggered_at" label="触发时间" width="180" />
            <el-table-column label="操作" width="100">
              <template #default="{ row }">
                <el-button size="small" @click="handleViewEvidence(row)">查看</el-button>
              </template>
            </el-table-column>
          </el-table>

          <el-pagination
            v-model:current-page="eventPage"
            v-model:page-size="eventPageSize"
            :total="eventTotal"
            :page-sizes="[10, 20, 50]"
            layout="total, sizes, prev, pager, next"
            @size-change="fetchEvents"
            @current-change="fetchEvents"
            class="pagination"
          />
        </el-card>
      </el-tab-pane>
    </el-tabs>

    <el-dialog
      v-model="ruleDialogVisible"
      :title="isEditRule ? '编辑规则' : '添加规则'"
      width="600px"
      @close="resetRuleForm"
    >
      <el-form :model="ruleForm" :rules="ruleRules" ref="ruleFormRef" label-width="120px">
        <el-form-item label="规则名称" prop="name">
          <el-input v-model="ruleForm.name" placeholder="请输入规则名称" />
        </el-form-item>
        <el-form-item label="触发条件" prop="condition_expr">
          <el-input v-model="ruleForm.condition_expr" type="textarea" :rows="3" placeholder="如: cpu_usage > 80" />
        </el-form-item>
        <el-form-item label="防抖动(秒)" prop="debounce_seconds">
          <el-input-number v-model="ruleForm.debounce_seconds" :min="0" :max="300" />
        </el-form-item>
        <el-form-item label="通知渠道" prop="notification_channels">
          <el-input v-model="ruleForm.notification_channels" placeholder="如: email,webhook" />
        </el-form-item>
        <el-form-item label="是否启用">
          <el-switch v-model="ruleForm.enabled" />
        </el-form-item>
      </el-form>
      <template #footer>
        <el-button @click="ruleDialogVisible = false">取消</el-button>
        <el-button type="primary" @click="handleRuleSubmit" :loading="ruleSubmitting">确定</el-button>
      </template>
    </el-dialog>
  </div>
</template>

<script setup lang="ts">
import { ref, onMounted } from 'vue'
import { ElMessage, ElMessageBox, type FormInstance, type FormRules } from 'element-plus'
import { alarmApi } from '../../api/monitoring'
import type { AlarmRule, AlarmEvent } from '../../api/monitoring'

const activeTab = ref('rules')

const rules = ref<AlarmRule[]>([])
const rulesLoading = ref(false)
const ruleDialogVisible = ref(false)
const isEditRule = ref(false)
const editRuleId = ref<number | null>(null)
const ruleSubmitting = ref(false)
const ruleFormRef = ref<FormInstance>()

const ruleForm = ref({
  name: '',
  condition_expr: '',
  debounce_seconds: 5,
  notification_channels: '',
  enabled: true,
})

const ruleRules = ref<FormRules>({
  name: [{ required: true, message: '请输入规则名称', trigger: 'blur' }],
  condition_expr: [{ required: true, message: '请输入触发条件', trigger: 'blur' }],
})

const events = ref<AlarmEvent[]>([])
const eventsLoading = ref(false)
const eventPage = ref(1)
const eventPageSize = ref(10)
const eventTotal = ref(0)

onMounted(() => {
  fetchRules()
  fetchEvents()
})

const fetchRules = async () => {
  rulesLoading.value = true
  try {
    rules.value = await alarmApi.listRules()
  } catch {
    ElMessage.error('获取报警规则失败')
  } finally {
    rulesLoading.value = false
  }
}

const fetchEvents = async () => {
  eventsLoading.value = true
  try {
    const result = await alarmApi.listEvents({
      page: eventPage.value,
      limit: eventPageSize.value,
    })
    events.value = result.items
    eventTotal.value = result.total
  } catch {
    ElMessage.error('获取报警事件失败')
  } finally {
    eventsLoading.value = false
  }
}

const handleAddRule = () => {
  isEditRule.value = false
  editRuleId.value = null
  ruleDialogVisible.value = true
}

const handleEditRule = (row: AlarmRule) => {
  isEditRule.value = true
  editRuleId.value = row.id
  ruleForm.value = {
    name: row.name,
    condition_expr: row.condition_expr,
    debounce_seconds: row.debounce_seconds,
    notification_channels: row.notification_channels,
    enabled: row.enabled,
  }
  ruleDialogVisible.value = true
}

const handleToggleRule = async (rule: AlarmRule) => {
  try {
    await alarmApi.updateRule(rule.id, { enabled: rule.enabled })
    ElMessage.success('状态已更新')
  } catch {
    ElMessage.error('更新状态失败')
    rule.enabled = !rule.enabled
  }
}

const handleDeleteRule = async (id: number) => {
  try {
    await ElMessageBox.confirm('确定要删除该规则吗？', '提示', {
      confirmButtonText: '确定',
      cancelButtonText: '取消',
      type: 'warning',
    })
    await alarmApi.deleteRule(id)
    ElMessage.success('删除成功')
    fetchRules()
  } catch (error) {
    if (error !== 'cancel') {
      ElMessage.error('删除失败')
    }
  }
}

const handleRuleSubmit = async () => {
  if (!ruleFormRef.value) return
  await ruleFormRef.value.validate(async (valid) => {
    if (!valid) return
    ruleSubmitting.value = true
    try {
      if (isEditRule.value && editRuleId.value !== null) {
        await alarmApi.updateRule(editRuleId.value, ruleForm.value)
        ElMessage.success('更新成功')
      } else {
        await alarmApi.createRule(ruleForm.value as Omit<AlarmRule, 'id' | 'created_at'>)
        ElMessage.success('添加成功')
      }
      ruleDialogVisible.value = false
      fetchRules()
    } catch {
      ElMessage.error(isEditRule.value ? '更新失败' : '添加失败')
    } finally {
      ruleSubmitting.value = false
    }
  })
}

const handleViewEvidence = (_event: AlarmEvent) => {
  ElMessage.info('证据查看功能开发中')
}

const resetRuleForm = () => {
  ruleForm.value = {
    name: '',
    condition_expr: '',
    debounce_seconds: 5,
    notification_channels: '',
    enabled: true,
  }
  ruleFormRef.value?.resetFields()
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
