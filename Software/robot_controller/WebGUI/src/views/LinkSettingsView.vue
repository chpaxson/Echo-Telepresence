<script setup lang="ts">
import Knob from 'primevue/knob';
import Slider from 'primevue/slider';
import ToggleSwitch from 'primevue/toggleswitch';
import { ref } from 'vue';

const enableLink = ref(false);

</script>

<template>
  <ToggleSwitch v-model="enableLink" />
  <Slider v-model="value" class="w-56" />
</template>

<style></style>
