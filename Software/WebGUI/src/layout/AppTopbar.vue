<script setup>
import { useLayout } from '@/layout/composables/layout';
import { useRobotStore } from '@/stores/robotStore';
import { useToast } from 'primevue/usetoast';
import AppConfigurator from './AppConfigurator.vue';
const toast = useToast();

const robotStore = useRobotStore();

const { toggleMenu, toggleDarkMode, isDarkTheme } = useLayout();

const attemptReconnect = () => {
    toast.add({
        severity: 'info',
        summary: 'Reconnecting...',
        life: 2000
    });
    if (window.initWebSocket) {
        window.initWebSocket();
    }
};
</script>

<template>
    <div class="layout-topbar">
        <div class="layout-topbar-logo-container">
            <button class="layout-menu-button layout-topbar-action" @click="toggleMenu">
                <i class="pi pi-bars"></i>
            </button>
            <router-link to="/" class="layout-topbar-logo">
                <img src="@/assets/logo.svg" alt="Logo" class="layout-topbar-logo-image" />

                <span>Echo Telepresence Controller</span>
            </router-link>
        </div>

        <div class="layout-topbar-actions">
            <div class="layout-config-menu">
                <!-- Websocket reconnect -->
                <Button
                    :label="robotStore.webSocketStatus ? `Connected` : `Lost connection!  Reconnect?`"
                    icon="pi pi-exclamation-triangle"
                    :disabled="robotStore.webSocketStatus"
                    :severity="robotStore.webSocketStatus ? `success` : `danger`"
                    @click="attemptReconnect"
                ></Button>

                <button type="button" class="layout-topbar-action" @click="toggleDarkMode">
                    <i :class="['pi', { 'pi-moon': isDarkTheme, 'pi-sun': !isDarkTheme }]"></i>
                </button>
                <div class="relative">
                    <button
                        v-styleclass="{ selector: '@next', enterFromClass: 'hidden', enterActiveClass: 'animate-scalein', leaveToClass: 'hidden', leaveActiveClass: 'animate-fadeout', hideOnOutsideClick: true }"
                        type="button"
                        class="layout-topbar-action layout-topbar-action-highlight"
                    >
                        <i class="pi pi-palette"></i>
                    </button>
                    <AppConfigurator />
                </div>
            </div>

            <button
                class="layout-topbar-menu-button layout-topbar-action"
                v-styleclass="{ selector: '@next', enterFromClass: 'hidden', enterActiveClass: 'animate-scalein', leaveToClass: 'hidden', leaveActiveClass: 'animate-fadeout', hideOnOutsideClick: true }"
            >
                <i class="pi pi-ellipsis-v"></i>
            </button>
        </div>
    </div>
</template>
