<script setup lang="ts">
import RobotVis from '../components/RobotVis.vue'
</script>

<template>
  <div class="dashboard">
    <div class="viswrapper">
      <RobotVis label="robot1" />
      <RobotVis label="robot2" />
    </div>
  </div>
</template>

<style>
.viswrapper {
  display: flex;
  justify-content: center;
  align-items: center;
  flex-wrap: wrap;
  gap: 2rem;
}

@media (min-width: 1024px) {
  .about {
    min-height: 100vh;
    display: flex;
    align-items: center;
  }
}
</style>
