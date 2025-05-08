import App from '@/App.vue';
import router from '@/router/index.js';
import { createPinia } from 'pinia';
import { createApp } from 'vue';

import { definePreset } from '@primeuix/themes';
import Aura from '@primeuix/themes/aura';
import PrimeVue from 'primevue/config';
import ConfirmationService from 'primevue/confirmationservice';
import ToastService from 'primevue/toastservice';

import { useRobotStore } from '@/stores/robotStore';
import { watch } from 'vue';

import '@/assets/styles.scss';

const app = createApp(App);
app.use(router);
app.use(createPinia());

const stylePreset = definePreset(Aura, {
    semantic: {
        primary: {
            50: '{indigo.50}',
            100: '{indigo.100}',
            200: '{indigo.200}',
            300: '{indigo.300}',
            400: '{indigo.400}',
            500: '{indigo.500}',
            600: '{indigo.600}',
            700: '{indigo.700}',
            800: '{indigo.800}',
            900: '{indigo.900}',
            950: '{indigo.950}'
        },
        colorScheme: {
            light: {
                surface: {
                    0: '#ffffff',
                    50: '#f4f4f4',
                    100: '#e8e9e9',
                    200: '#d2d2d4',
                    300: '#bbbcbe',
                    400: '#a5a5a9',
                    500: '#8e8f93',
                    600: '#77787d',
                    700: '#616268',
                    800: '#4a4b52',
                    900: '#34343d',
                    950: '#1d1e27'
                }
            },
            dark: {
                surface: {
                    0: '#ffffff',
                    50: '#f4f4f4',
                    100: '#e8e9e9',
                    200: '#d2d2d4',
                    300: '#bbbcbe',
                    400: '#a5a5a9',
                    500: '#8e8f93',
                    600: '#77787d',
                    700: '#616268',
                    800: '#4a4b52',
                    900: '#34343d',
                    950: '#1d1e27'
                }
            }
        }
    }
});
app.use(PrimeVue, {
    theme: {
        preset: stylePreset,
        options: {
            darkModeSelector: '.app-dark'
        }
    }
});
app.use(ToastService);
app.use(ConfirmationService);

app.mount('#app');

// Websocket connection and message handling
var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
window.addEventListener('load', onLoad);
function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage; // <-- add this line
}
function onOpen(event) {
    console.log('Connection opened');
}
function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
}
function onMessage(event) {
    console.log(event.data);
}

initWebSocket();

robotStore = useRobotStore();
// Watch robotStore.r1.eePos and send a message to the server when it changes
watch(
    robotStore.r1.eePos,
    (newValue) => {
        if (websocket && websocket.readyState === WebSocket.OPEN) {
            websocket.send(JSON.stringify({ eePos: newValue }));
        }
    },
    { immediate: true }
);
