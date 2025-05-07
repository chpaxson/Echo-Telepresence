import { createPinia } from 'pinia';
import { createApp } from 'vue';
import App from './App.vue';
import router from './router';

import { definePreset } from '@primeuix/themes';
import Aura from '@primeuix/themes/aura';
import PrimeVue from 'primevue/config';
import ConfirmationService from 'primevue/confirmationservice';
import ToastService from 'primevue/toastservice';

import '@/assets/styles.scss';

const app = createApp(App);
app.use(createPinia());

const primaryColors = { 50: '#eef2ff', 100: '#e0e7ff', 200: '#c7d2fe', 300: '#a5b4fc', 400: '#818cf8', 500: '#6366f1', 600: '#4f46e5', 700: '#4338ca', 800: '#3730a3', 900: '#312e81', 950: '#1e1b4b' };
const soho = { 0: '#ffffff', 50: '#f4f4f4', 100: '#e8e9e9', 200: '#d2d2d4', 300: '#bbbcbe', 400: '#a5a5a9', 500: '#8e8f93', 600: '#77787d', 700: '#616268', 800: '#4a4b52', 900: '#34343d', 950: '#1d1e27' };

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

app.use(router);
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
