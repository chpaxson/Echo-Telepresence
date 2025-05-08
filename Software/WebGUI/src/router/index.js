import AppLayout from '@/layout/AppLayout.vue';
import { createRouter, createWebHistory } from 'vue-router';

const router = createRouter({
    history: createWebHistory(),
    routes: [
        {
            path: '/',
            component: AppLayout,
            children: [
                {
                    path: '/',
                    name: 'Dashboard',
                    component: () => import('@/views/Dashboard.vue')
                },
                {
                    path: '/dashboard/robot1',
                    name: 'Robot 1',
                    component: () => import('@/views/Robot1View.vue')
                },
                {
                    path: '/dashboard/robot2',
                    name: 'Robot 2',
                    component: () => import('@/views/Robot2View.vue')
                },
                {
                    path: '/draw/canvas',
                    name: 'Drawing Canvas',
                    component: () => import('@/views/Canvas.vue')
                },
                {
                    path: '/draw/generator',
                    name: 'Path Generators',
                    component: () => import('@/views/GeneratorView.vue')
                },
                {
                    path: '/draw/Gcode',
                    name: 'GCode Console',
                    component: () => import('@/views/GcodeConsoleView.vue')
                },
                {
                    path: '/settings/network',
                    name: 'Network Settings',
                    component: () => import('@/views/NetworkSettingsView.vue')
                },
                {
                    path: '/settings/stats',
                    name: 'Statistics',
                    component: () => import('@/views/StatisticsView.vue')
                },
                {
                    path: '/logs',
                    name: 'Logs',
                    component: () => import('@/views/LogConsoleView.vue')
                },
                {
                    path: '/about',
                    name: 'About',
                    component: () => import('@/views/AboutView.vue')
                }
            ]
        }
    ]
});

export default router;
