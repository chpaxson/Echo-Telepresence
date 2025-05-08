<script setup>
import { ref } from 'vue';

import AppMenuItem from './AppMenuItem.vue';

const model = ref([
    {
        label: 'Dashboards',
        items: [
            { label: 'Overview', icon: 'pi pi-fw pi-home', to: '/' },
            { label: 'Robot 1', icon: 'pi pi-fw pi-home', to: '/dashboard/robot1' },
            { label: 'Robot 2', icon: 'pi pi-fw pi-home', to: '/dashboard/robot2' }
        ]
    },
    {
        label: 'Draw',
        items: [
            { label: 'Canvas', icon: 'pi pi-fw pi-palette', to: '/draw/canvas' },
            { label: 'Generators', icon: 'pi pi-fw pi-code', to: '/draw/generator' },
            { label: 'GCode', icon: 'pi pi-fw pi-print', to: '/draw/gcode' }
        ]
    },
    {
        label: 'Settings',
        items: [
            { label: 'Network', icon: 'pi pi-fw pi-wifi', to: '/settings/network' },
            { label: 'Statistics', icon: 'pi pi-fw pi-chart-line', to: '/settings/stats' },
            { label: 'Logs', icon: 'pi pi-fw pi-inbox', to: '/logs' },
            { label: 'About', icon: 'pi pi-info-circle', to: '/about' }
        ]
    }
]);
</script>

<template>
    <ul class="layout-menu">
        <template v-for="(item, i) in model" :key="item">
            <app-menu-item v-if="!item.separator" :item="item" :index="i"></app-menu-item>
            <li v-if="item.separator" class="menu-separator"></li>
        </template>
    </ul>
</template>

<style lang="scss" scoped></style>
