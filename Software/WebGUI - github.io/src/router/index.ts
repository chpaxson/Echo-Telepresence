import { createRouter, createWebHashHistory } from 'vue-router';

import AppLayout from '@/layout/AppLayout.vue';
import AboutView from '@/views/AboutView.vue';
import Canvas from '@/views/Canvas.vue';
import Dashboard from '@/views/Dashboard.vue';
import GcodeConsoleView from '@/views/GcodeConsoleView.vue';
import GeneratorView from '@/views/GeneratorView.vue';
import LogConsoleView from '@/views/LogConsoleView.vue';
import MotorParamView from '@/views/MotorParamView.vue';
import NetworkSettingsView from '@/views/NetworkSettingsView.vue';
import Robot1View from '@/views/Robot1View.vue';
import Robot2View from '@/views/Robot2View.vue';
import StatisticsView from '@/views/StatisticsView.vue';

const router = createRouter({
    history: createWebHashHistory(),
    routes: [
        {
            path: '/',
            component: AppLayout,
            children: [
                {
                    path: '/',
                    name: 'Dashboard',
                    component: Dashboard
                },
                {
                    path: '/dashboard/robot1',
                    name: 'Robot 1',
                    component: Robot1View
                },
                {
                    path: '/dashboard/robot2',
                    name: 'Robot 2',
                    component: Robot2View
                },
                {
                    path: '/motors',
                    name: 'Motor Parameters',
                    component: MotorParamView
                },
                {
                    path: '/draw/canvas',
                    name: 'Drawing Canvas',
                    component: Canvas
                },
                {
                    path: '/draw/generator',
                    name: 'Path Generators',
                    component: GeneratorView
                },
                {
                    path: '/draw/Gcode',
                    name: 'GCode Console',
                    component: GcodeConsoleView
                },
                {
                    path: '/settings/network',
                    name: 'Network Settings',
                    component: NetworkSettingsView
                },
                {
                    path: '/settings/stats',
                    name: 'Statistics',
                    component: StatisticsView
                },
                {
                    path: '/logs',
                    name: 'Logs',
                    component: LogConsoleView
                },
                {
                    path: '/about',
                    name: 'About',
                    component: AboutView
                }
            ]
        }
    ]
});

export default router;
