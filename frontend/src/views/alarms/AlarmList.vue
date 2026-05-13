<template>
  <div class="alarm-list">
    <el-tabs v-model="activeTab">
      <el-tab-pane label="报警规则" name="rules">
        <el-card>
          <template #header>
            <div class="card-header">
              <span>报警规则</span>
              <el-button type="primary" @click="showAddRule = true">添加规则</el-button>
            </div>
          </template>
          <el-table :data="rules" stripe>
            <el-table-column prop="name" label="规则名称" />
            <el-table-column prop="condition_expr" label="触发条件" />
            <el-table-column prop="debounce_seconds" label="防抖动(秒)" />
            <el-table-column prop="enabled" label="状态">
              <template #default="{ row }">
                <el-switch v-model="row.enabled" @change="toggleRule(row)" />
              </template>
            </el-table-column>
            <el-table-column label="操作" width="150">
              <template #default="{ row }">
                <el-button size="small" @click="editRule(row)">编辑</el-button>
                <el-button size="small" type="danger" @click="deleteRule(row.id)">删除</el-button>
              </template>
            </el-table-column>
          </el-table>
        </el-card>
      </el-tab-pane>
      <el-tab-pane label="报警历史" name="events">
        <el-card>
          <el-table :data="events" stripe>
            <el-table-column prop="rule_id" label="规则 ID" />
            <el-table-column prop="task_id" label="任务 ID" />
            <el-table-column prop="evidence_path" label="证据" />
            <el-table-column prop="triggered_at" label="触发时间" />
            <el-table-column label="操作" width="100">
              <template #default="{ row }">
                <el-button size="small" @click="viewEvidence(row)">查看</el-button>
              </template>
            </el-table-column>
          </el-table>
        </el-card>
      </el-tab-pane>
    </el-tabs>
  </div>
</template>

<script setup lang="ts">
import { ref, onMounted } from 'vue'
import { alarmApi } from '../../api/monitoring'
import type { AlarmRule, AlarmEvent } from '../../api/monitoring'

const activeTab = ref('rules')
const rules = ref<AlarmRule[]>([])
const events = ref<AlarmEvent[]>([])
const showAddRule = ref(false)

onMounted(() => {
  fetchRules()
  fetchEvents()
})

const fetchRules = async () => {
  try {
    rules.value = await alarmApi.listRules()
  } catch {
    // API not available
  }
}

const fetchEvents = async () => {
  try {
    const result = await alarmApi.listEvents()
    events.value = result.items
  } catch {
    // API not available
  }
}

const toggleRule = async (rule: AlarmRule) => {
  try {
    await alarmApi.updateRule(rule.id, { enabled: rule.enabled })
  } catch {
    // Error handled by interceptor
  }
}

const editRule = (_rule: AlarmRule) => {
  // TODO: Implement edit
}

const deleteRule = async (id: number) => {
  try {
    await alarmApi.deleteRule(id)
    fetchRules()
  } catch {
    // Error handled by interceptor
  }
}

const viewEvidence = (_event: AlarmEvent) => {
  // TODO: Show evidence image
}
</script>

<style scoped>
.card-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
}
</style>
