<template>
  <div class="system-settings">
    <el-tabs v-model="activeTab">
      <el-tab-pane label="系统配置" name="config">
        <el-card>
          <template #header>
            <div class="card-header">
              <span>系统配置</span>
              <el-button type="primary" @click="saveAllConfig" :loading="saving">保存配置</el-button>
            </div>
          </template>
          <el-form v-loading="configLoading" label-width="150px">
            <el-form-item
              v-for="item in configs"
              :key="item.key"
              :label="item.key"
            >
              <el-input v-model="item.value" :disabled="!editingKeys.has(item.key)" />
              <el-button
                v-if="!editingKeys.has(item.key)"
                size="small"
                @click="startEdit(item.key)"
                style="margin-left: 8px"
              >
                编辑
              </el-button>
              <el-button
                v-else
                size="small"
                type="primary"
                @click="saveSingleConfig(item.key)"
              >
                保存
              </el-button>
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
          <el-table :data="plugins" v-loading="pluginsLoading" stripe>
            <el-table-column prop="name" label="插件名称" />
            <el-table-column prop="version" label="版本" width="120" />
            <el-table-column prop="status" label="状态" width="100">
              <template #default="{ row }">
                <el-tag :type="row.status === 'active' ? 'success' : 'info'">
                  {{ row.status === 'active' ? '已启用' : '已禁用' }}
                </el-tag>
              </template>
            </el-table-column>
            <el-table-column label="操作" width="150">
              <template #default="{ row }">
                <el-button size="small" @click="handleTogglePlugin(row)">
                  {{ row.status === 'active' ? '禁用' : '启用' }}
                </el-button>
                <el-button size="small" type="danger" @click="handleUninstallPlugin(row.id)">卸载</el-button>
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
import { ElMessage, ElMessageBox } from 'element-plus'
import { configApi } from '../../api/config'
import { pluginApi } from '../../api/plugins'
import type { Config } from '../../api/config'
import type { Plugin } from '../../api/plugins'

const activeTab = ref('config')

const configs = ref<Config[]>([])
const configLoading = ref(false)
const saving = ref(false)
const editingKeys = ref<Set<string>>(new Set())

const upgradeHistory = ref([
  { target_version: '1.0.0', type: 'full', status: 'success', started_at: '2026-05-13 10:00:00' },
])

const plugins = ref<Plugin[]>([])
const pluginsLoading = ref(false)

onMounted(() => {
  fetchConfigs()
  fetchPlugins()
})

const fetchConfigs = async () => {
  configLoading.value = true
  try {
    const result = await configApi.list()
    configs.value = result.items
  } catch {
    ElMessage.error('获取配置列表失败')
  } finally {
    configLoading.value = false
  }
}

const fetchPlugins = async () => {
  pluginsLoading.value = true
  try {
    const result = await pluginApi.list()
    plugins.value = result.items
  } catch {
    ElMessage.error('获取插件列表失败')
  } finally {
    pluginsLoading.value = false
  }
}

const startEdit = (key: string) => {
  editingKeys.value.add(key)
}

const saveSingleConfig = async (key: string) => {
  const item = configs.value.find((c) => c.key === key)
  if (!item) return
  saving.value = true
  try {
    await configApi.update(key, item.value)
    ElMessage.success('配置已更新')
    editingKeys.value.delete(key)
  } catch {
    ElMessage.error('更新配置失败')
  } finally {
    saving.value = false
  }
}

const saveAllConfig = async () => {
  saving.value = true
  try {
    const savePromises = Array.from(editingKeys.value).map((key) => {
      const item = configs.value.find((c) => c.key === key)
      if (item) {
        return configApi.update(key, item.value)
      }
      return Promise.resolve()
    })
    await Promise.all(savePromises)
    ElMessage.success('所有配置已保存')
    editingKeys.value.clear()
    fetchConfigs()
  } catch {
    ElMessage.error('保存配置失败')
  } finally {
    saving.value = false
  }
}

const handleTogglePlugin = (_plugin: Plugin) => {
  ElMessage.info('插件状态切换功能开发中')
}

const handleUninstallPlugin = async (id: number) => {
  try {
    await ElMessageBox.confirm('确定要卸载该插件吗？', '提示', {
      confirmButtonText: '确定',
      cancelButtonText: '取消',
      type: 'warning',
    })
    await pluginApi.uninstall(id)
    ElMessage.success('卸载成功')
    fetchPlugins()
  } catch (error) {
    if (error !== 'cancel') {
      ElMessage.error('卸载失败')
    }
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
