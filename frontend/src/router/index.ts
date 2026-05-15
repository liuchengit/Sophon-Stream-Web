import { createRouter, createWebHashHistory } from 'vue-router'
import type { RouteRecordRaw } from 'vue-router'
import { useAuthStore } from '../stores/auth'

const routes: RouteRecordRaw[] = [
  {
    path: '/login',
    name: 'Login',
    component: () => import('../views/Login.vue'),
    meta: { requiresAuth: false },
  },
  {
    path: '/',
    component: () => import('../layouts/MainLayout.vue'),
    redirect: '/dashboard',
    meta: { requiresAuth: true },
    children: [
      {
        path: 'dashboard',
        name: 'Dashboard',
        component: () => import('../views/Dashboard.vue'),
      },
      {
        path: 'devices',
        name: 'Devices',
        component: () => import('../views/devices/DeviceList.vue'),
      },
      {
        path: 'tasks',
        name: 'Tasks',
        component: () => import('../views/tasks/TaskList.vue'),
      },
      {
        path: 'tasks/:id/editor',
        name: 'TaskEditor',
        component: () => import('../views/tasks/TaskEditor.vue'),
      },
      {
        path: 'algorithms',
        name: 'Algorithms',
        component: () => import('../views/algorithms/AlgorithmList.vue'),
      },
      {
        path: 'monitoring',
        name: 'Monitoring',
        component: () => import('../views/monitoring/MonitorPanel.vue'),
      },
      {
        path: 'alarms',
        name: 'Alarms',
        component: () => import('../views/alarms/AlarmList.vue'),
      },
      {
        path: 'settings',
        name: 'Settings',
        component: () => import('../views/settings/SystemSettings.vue'),
      },
      {
        path: 'workflow',
        name: 'WorkflowList',
        component: () => import('../views/WorkflowList.vue'),
      },
      {
        path: 'workflow/:id',
        name: 'WorkflowEditor',
        component: () => import('../views/WorkflowEditor.vue'),
      },
    ],
  },
]

const router = createRouter({
  history: createWebHashHistory(),
  routes,
})

router.beforeEach((to, _from, next) => {
  const authStore = useAuthStore()
  if (to.meta.requiresAuth !== false && !authStore.isAuthenticated) {
    next({ name: 'Login' })
  } else if (to.name === 'Login' && authStore.isAuthenticated) {
    next({ name: 'Dashboard' })
  } else {
    next()
  }
})

export default router
