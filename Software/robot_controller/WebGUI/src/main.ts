import './assets/main.css'
import { createApp } from 'vue'

import App from './App.vue'
const app = createApp(App)
document.title = 'Echo Robot Control'

import router from './router'
app.use(router)


import PrimeVue from 'primevue/config'
import StyleClass from 'primevue/styleclass'
app.use(PrimeVue, {unstyled: true})
app.directive('styleclass', StyleClass)

import { createPinia } from 'pinia'
app.use(createPinia())

app.mount('#app')
