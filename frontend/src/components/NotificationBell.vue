<template>
  <div>
    <el-badge :value="unreadCount" :hidden="unreadCount === 0" class="notification-badge">
      <el-icon :size="20" class="notification-icon" @click="showDrawer">
        <Bell />
      </el-icon>
    </el-badge>

    <el-drawer v-model="drawerVisible" title="实时通知" size="360px">
      <el-timeline v-if="notifications.length">
        <el-timeline-item
          v-for="(item, index) in notifications"
          :key="index"
          :timestamp="item.time"
          placement="top"
          :type="item.type"
        >
          <el-card shadow="hover">
            <h4>{{ item.title }}</h4>
            <p>{{ item.content }}</p>
          </el-card>
        </el-timeline-item>
      </el-timeline>
      <el-empty v-else description="暂无通知" />
    </el-drawer>
  </div>
</template>

<script setup lang="ts">
import { ref, computed } from 'vue'
import { Bell } from '@element-plus/icons-vue'
import { ElNotification } from 'element-plus'

interface NotificationItem {
  type: 'success' | 'warning' | 'danger' | 'info'
  title: string
  content: string
  time: string
}

const notifications = ref<NotificationItem[]>([])
const drawerVisible = ref(false)

const unreadCount = computed(() => notifications.value.length)

function addAlarmNotification(alarmData: Record<string, unknown>) {
  const now = new Date().toLocaleTimeString('zh-CN')
  const item: NotificationItem = {
    type: alarmData.level === 'critical' ? 'danger' : alarmData.level === 'warning' ? 'warning' : 'info',
    title: String(alarmData.ruleName || '报警通知'),
    content: String(alarmData.message || '触发报警规则'),
    time: now,
  }
  notifications.value.unshift(item)

  ElNotification({
    title: item.title,
    message: item.content,
    type: item.type === 'danger' ? 'error' : item.type,
    duration: 5000,
    position: 'top-right',
  })
}

function showDrawer() {
  drawerVisible.value = true
}

defineExpose({ addAlarmNotification })
</script>

<style scoped>
.notification-badge {
  cursor: pointer;
  display: flex;
  align-items: center;
}
.notification-icon {
  color: #606266;
  transition: color 0.2s;
}
.notification-icon:hover {
  color: #409eff;
}
</style>
