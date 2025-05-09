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
import { calc_ik } from '@/utils/kinematics';
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

const robotStore = useRobotStore();

// Websocket connection and message handling
var gateway = `ws://echo1.local/ws`;
var websocket;
window.addEventListener('load', onLoad);
function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}
function onOpen(event) {
    console.log('Connection opened');
    robotStore.webSocketStatus = true;
}
function onClose(event) {
    console.log('Connection closed');
    robotStore.webSocketStatus = false;
    setTimeout(initWebSocket, 2000);
}

// Extend the Window interface to include initWebSocket
declare global {
    interface Window {
        initWebSocket: () => void;
    }
}
window.initWebSocket = initWebSocket;
function onMessage(event) {
    // Split the message by commas
    const data = event.data.split(',');
    const r1a1 = (parseInt(data[0]) / 4095.0) * 1.5 * Math.PI;
    const r1a2 = (parseInt(data[1]) / 4095.0) * 1.5 * Math.PI - Math.PI / 2;
    robotStore.r1.realConfig = { a1: r1a1, a2: r1a2 };
}
function onLoad() {
    initWebSocket();
}

// Watch robotStore.r1.ee and robotStore.r2.ee for changes and calculate ik
let lastSentTime = 0;
watch(
    () => [robotStore.r1.ee, robotStore.r2.ee],
    ([r1, r2]) => {
        if (lastSentTime + 20 < performance.now()) {
            let r1angles, r2angles;
            if (robotStore.r1.driver !== 'Robot Data') {
                r1angles = calc_ik(r1);
            }
            if (robotStore.r2.driver !== 'Robot Data') {
                r2angles = calc_ik(r2);
            }

            if (websocket && websocket.readyState === WebSocket.OPEN) {
                // r1a1, r1a2, r2a1, r2a2
                websocket.send(Math.floor(r1angles.a1 * 16384) + ',' + Math.floor(r1angles.a2 * 16384) + ',' + Math.floor(r2angles.a1 * 16384) + ',' + Math.floor(r2angles.a2 * 16384));
            }
            lastSentTime = performance.now();
        }
    },
    { deep: true }
);
