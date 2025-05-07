import { createRouter, createWebHistory } from 'vue-router'
import DashboardView    from '../views/DashboardView.vue'
import KinematicsView   from '../views/KinematicsView.vue'
import LinkSettingsView from '../views/LinkSettingsView.vue'
import DrawingView      from '../views/DrawingView.vue'
import GcodeView        from '../views/GcodeView.vue'
import StatsView        from '../views/StatsView.vue'

const router = createRouter({
  history: createWebHistory(import.meta.env.BASE_URL),
  routes: [
    { path: '/', name: 'Dashboard', component: DashboardView},
    { path: '/kinematics', name: 'kinematics', component: KinematicsView},
    { path: '/link', name: 'link', component: LinkSettingsView},
    { path: '/draw', name: 'draw', component: DrawingView},
    { path: '/code', name: 'code', component: GcodeView},
    { path: '/stats', name: 'stats', component: StatsView},
  ],
})

export default router
