<script setup lang="ts">
import RobotVis from '../components/RobotVis.vue'
import RobotControl from '../components/RobotControl.vue'
import RobotSettings from '../components/RobotSettings.vue'
</script>


<template>
  <div class="robot2-layout">
    <div class="vis-col">
      <h1>Robot 2</h1>
      <RobotVis robot="robot2"/>
    </div>
    <div class="control-col">
      <RobotControl robot="robot2"/>
    </div>
  </div>
  <RobotSettings robot="robot2"/>
</template>

<style scoped>
.robot2-layout {
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
