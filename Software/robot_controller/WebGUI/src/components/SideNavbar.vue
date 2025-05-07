<script setup lang="ts">
import Menu from '@/volt/Menu.vue';
import { ref } from 'vue';
// import { useRouter } from 'vue-router';

// const router = useRouter();
const items = ref([
  {
    label: 'Dashboard',
    icon: 'pi pi-palette',
    route: '/'
  }, {
    label: 'Kinematics',
    icon: 'pi pi-link',
    route: '/kinematics'
  }, {
    label: 'Link Settings',
    icon: 'pi pi-link',
    route: '/link'
  }, {
    label: 'Draw',
    icon: 'pi pi-link',
    route: '/draw'
  }, {
    label: 'GCode',
    icon: 'pi pi-link',
    route: '/code'
  }, {
    label: 'Stats',
    icon: 'pi pi-link',
    route: '/stats'
  },

]);
</script>

<template>
  <div class="card flex justify-center">
    <Menu :model="items">
      <template #item="{ item, props }">
        <router-link v-if="item.route" v-slot="{ href, navigate }" :to="item.route" custom>
          <a :href="href" v-bind="props.action" @click="navigate">
            <span :class="item.icon" />
            <span class="ml-2">{{ item.label }}</span>
          </a>
        </router-link>
        <a v-else :href="item.url" :target="item.target" v-bind="props.action" target="_self" rel="noopener noreferrer">
          <span :class="item.icon" />
          <span class="ml-2">{{ item.label }}</span>
        </a>
      </template>
    </Menu>
  </div>
</template>
