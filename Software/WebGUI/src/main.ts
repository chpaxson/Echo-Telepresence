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
import { calc_ik } from './utils/kinematics';

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
var gateway = `ws://echo.local/ws`;
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
    const ID = event.data.split(',')[0].split(':')[1].trim();
    const data = event.data.split(',')[1].split(':')[1].trim();
    console.log('Message received:', event.data);
    switch (ID) {
        case '0x18':
            robotStore.r1.realConfig.a1 = parseFloat(data) / 4.5 + Math.PI * 0.75;
            break;
        case '0x118':
            robotStore.r1.realConfig.a2 = parseFloat(data) / 4.5 - Math.PI / 2;
            break;
        case '0x218':
            robotStore.r2.realConfig.a1 = parseFloat(data) / 4.5 + Math.PI;
            break;
        case '0x318':
            robotStore.r2.realConfig.a2 = parseFloat(data) / 4.5 - Math.PI / 4;
            break;
    }
}
function onLoad() {
    initWebSocket();
}

// Watch robotStore.r1.ee and robotStore.r2.ee for changes and calculate ik
// let lastSentTime = 0;
// watch(

//     () => {
//         if (lastSentTime + 20 < performance.now()) {
//             if (websocket && websocket.readyState === WebSocket.OPEN) {
//                 websocket.send(robotStore.r1.a1 + ',' + robotStore.r1.a2 + ',' + robotStore.r2.a1 + ',' + robotStore.r2.a2);
//             }
//             lastSentTime = performance.now();
//         }
//     },
//     { deep: true }
// );

let lastSentTime = 0;
watch(
    () => [robotStore.r1.ee, robotStore.r2.ee], //     () => [robotStore.r1.a1, robotStore.r1.a2, robotStore.r2.a1, robotStore.r2.a2],
    () => {
        if (lastSentTime + 20 < performance.now()) {
            const r1ang = calc_ik(robotStore.r1.ee);
            const r2ang = calc_ik(robotStore.r2.ee);
            if (websocket && websocket.readyState === WebSocket.OPEN) {
                websocket.send(r1ang.a1 * 4.5 + ',' + r1ang.a2 * 4.5 + ',' + r2ang.a1 * 4.5 + ',' + r2ang.a2 * 4.5);
                // websocket.send(robotStore.r1.a1 + ',' + robotStore.r1.a2 + ',' + robotStore.r2.a1 + ',' + robotStore.r2.a2);
            }
            lastSentTime = performance.now();
        }
    },
    { deep: true }
);
