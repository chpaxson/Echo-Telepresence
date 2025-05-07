<script setup lang="ts">
import SideNavbar from '@/components/SideNavbar.vue'
import { ref } from 'vue';
import LorenzAttractor from './components/LorenzAttractor.vue'
import 'boxicons'


</script>

<template>
  <LorenzAttractor />
  <SideNavbar />
</template>

<!-- <style scoped>
box-icon {
  color: var(--color-text);
  fill: var(--color-text);
}

.rLink {
  display: flex;
  gap: 0.5em;
  align-items: center;
}

</style> -->
