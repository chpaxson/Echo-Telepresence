import { createRouter, createWebHistory } from 'vue-router'
import DashboardView from '../views/DashboardView.vue'
import Robot1View from '../views/Robot1View.vue'
import Robot2View from '../views/Robot2View.vue'
import DrawingView from '../views/DrawingView.vue'
import LinkControlView from '../views/LinkControlView.vue'
import AboutView from '../views/AboutView.vue'

const router = createRouter({
  history: createWebHistory(import.meta.env.BASE_URL),
  routes: [
    { path: '/', name: 'Dashboard', component: DashboardView},
    { path: '/robot1', name: 'Robot 1', component: Robot1View},
    { path: '/robot2', name: 'Robot 2', component: Robot2View},
    { path: '/draw', name: 'Draw', component: DrawingView},
    { path: '/link', name: 'Link', component: LinkControlView},
    { path: '/about', name: 'about', component: AboutView},
  ],
})

export default router
