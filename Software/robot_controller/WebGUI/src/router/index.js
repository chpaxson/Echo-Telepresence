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
                    path: '/dashboard',
                    name: 'Dashboard',
                    component: () => import('@/views/Dashboard.vue')
                },
                {
                    path: '/dashboard/robot1',
                    name: 'Robot 1',
                    component: () => import('@/views/Dashboard.vue')
                },
                {
                    path: '/dashboard/robot2',
                    name: 'Robot 2',
                    component: () => import('@/views/Dashboard.vue')
                },
                {
                    path: '/draw/canvas',
                    name: 'Drawing Canvas',
                    component: () => import('@/views/Canvas.vue')
                },
                {
                    path: '/draw/generator',
                    name: 'Generator',
                    component: () => import('@/views/Canvas.vue')
                },
                {
                    path: '/draw/Gcode',
                    name: 'GCode Console',
                    component: () => import('@/views/Canvas.vue')
                },

                {
                    path: '/settings/link',
                    name: 'Robot Linking',
                    component: () => import('@/views/Dashboard.vue')
                },
                {
                    path: '/settings/kinematics',
                    name: 'Kinematics Settings',
                    component: () => import('@/views/Dashboard.vue')
                },
                {
                    path: '/settings/network',
                    name: 'Network Settings',
                    component: () => import('@/views/Dashboard.vue')
                },
                {
                    path: '/settings/stats',
                    name: 'Statistics',
                    component: () => import('@/views/Dashboard.vue')
                },
                {
                    path: '/logs',
                    name: 'Logs',
                    component: () => import('@/views/Dashboard.vue')
                }
            ]
        }
    ]
});

export default router;
