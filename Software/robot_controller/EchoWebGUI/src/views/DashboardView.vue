<script setup lang="ts">
import RobotVis from '../components/RobotVis.vue'
</script>

<template>
  <div class="dashboard">
    <div class="viswrapper">
      <div>
        <h2>Robot 1</h2>
        <RobotVis robot="robot1" :width="500" />
      </div>
      <div>
        <h2>Robot 2</h2>
        <RobotVis robot="robot2" :width="500" />
      </div>
    </div>
  </div>
</template>

<style>
.viswrapper {
  display: flex;
  justify-content: space-between;
  align-items: center;
  flex-wrap: nowrap;
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
