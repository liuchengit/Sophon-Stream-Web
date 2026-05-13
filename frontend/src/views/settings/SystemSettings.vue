<template>
  <div class="system-settings">
    <el-tabs v-model="activeTab">
      <el-tab-pane label="系统配置" name="config">
        <el-card>
          <el-form :model="config" label-width="120px">
            <el-form-item label="服务端口">
              <el-input-number v-model="config.port" :min="1" :max="65535" />
            </el-form-item>
            <el-form-item label="日志级别">
              <el-select v-model="config.logLevel">
                <el-option label="DEBUG" value="debug" />
                <el-option label="INFO" value="info" />
                <el-option label="WARNING" value="warning" />
                <el-option label="ERROR" value="error" />
              </el-select>
            </el-form-item>
            <el-form-item label="最大任务数">
              <el-input-number v-model="config.maxTasks" :min="1" :max="16" />
            </el-form-item>
            <el-form-item>
              <el-button type="primary" @click="saveConfig">保存配置</el-button>
            </el-form-item>
          </el-form>
        </el-card>
      </el-tab-pane>
      <el-tab-pane label="系统升级" name="upgrade">
        <el-card>
          <template #header><span>系统升级</span></template>
          <el-upload
            action="/api/v1/upgrade"
            :limit="1"
            accept=".zip,.tar.gz"
          >
            <el-button type="primary">上传升级包</el-button>
          </el-upload>
          <el-table :data="upgradeHistory" style="margin-top: 20px" stripe>
            <el-table-column prop="target_version" label="目标版本" />
            <el-table-column prop="type" label="类型" />
            <el-table-column prop="status" label="状态" />
            <el-table-column prop="started_at" label="开始时间" />
          </el-table>
        </el-card>
      </el-tab-pane>
      <el-tab-pane label="插件市场" name="plugins">
        <el-card>
          <template #header><span>已安装插件</span></template>
          <el-table :data="plugins" stripe>
            <el-table-column prop="name" label="插件名称" />
            <el-table-column prop="version" label="版本" />
            <el-table-column prop="status" label="状态" />
            <el-table-column label="操作" width="150">
              <template #default="{ row }">
                <el-button size="small" @click="enablePlugin(row)">启用</el-button>
                <el-button size="small" type="danger" @click="uninstallPlugin(row.id)">卸载</el-button>
              </template>
            </el-table-column>
          </el-table>
        </el-card>
      </el-tab-pane>
    </el-tabs>
  </div>
</template>

<script setup lang="ts">
import { ref } from 'vue'

const activeTab = ref('config')

const config = ref({
  port: 8080,
  logLevel: 'info',
  maxTasks: 8,
})

const upgradeHistory = ref([
  { target_version: '1.0.0', type: 'full', status: 'success', started_at: '2026-05-13 10:00:00' },
])

const plugins = ref([
  { id: 1, name: 'YOLOX Detector', version: '1.0.0', status: 'active' },
  { id: 2, name: 'ByteTrack Tracker', version: '1.0.0', status: 'active' },
])

const saveConfig = () => {
  // TODO: Implement save config
}

const enablePlugin = (_plugin: any) => {
  // TODO: Implement enable plugin
}

const uninstallPlugin = (_id: number) => {
  // TODO: Implement uninstall plugin
}
</script>
