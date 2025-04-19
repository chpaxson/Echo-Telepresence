<script setup lang="ts">
import RobotVis from '../components/RobotVis.vue'
import RobotControl from '../components/RobotControl.vue'
import RobotSettings from '../components/RobotSettings.vue'
</script>

<template>
  <div class="robot1-layout">
    <div class="vis-col">
      <h1>Robot 1</h1>
      <RobotVis robot="robot1"/>
    </div>
    <div class="control-col">
      <RobotControl robot="robot1"/>
    </div>
  </div>
  <RobotSettings robot="robot1"/>
</template>

<style scoped>
.robot1-layout {
  display: flex;
  gap: 1em;
}
.vis-col {
  flex: 0 0 auto;
}
.control-col {
  width: 100%;
}
</style>
