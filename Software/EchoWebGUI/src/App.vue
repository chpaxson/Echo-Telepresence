<script setup lang="ts">
import { ref } from 'vue'
import { RouterView } from 'vue-router'
import DataGraphs from './components/DataGraphs.vue'
import RobotVis from './components/RobotVis.vue'
import Slider from './components/Slider.vue'
</script>

<template>
  <div style="display: flex; gap: 2em">
    <div style="width: 600px">
      <RobotVis label="Robot 1" :initialAngle1="135" :initialAngle2="45" />
    </div>
    <div style="width: 600px">
      <RobotVis label="Robot 2" :initialAngle1="90" :initialAngle2="90" />
    </div>
  </div>
</template>
