<script setup lang="ts">
import '../utils/utils.ts'
import { useRobot1Store } from '../stores/robot1'
import { useRobot2Store } from '../stores/robot2'
import DataSlider from './DataSlider.vue'
import { computed } from 'vue'

const props = defineProps({
  robot: { type: String, required: true },
  width: { type: Number, default: 600 },
})

const robotStore = props.robot === 'robot1'
  ? useRobot1Store()
  : useRobot2Store()

const maxAngularVelocity = computed({
  get() {
    return robotStore.maxAngularVel
  },
  set(maxAngularVelocity: number) {
    robotStore.maxAngularVel = maxAngularVelocity
  }
})
const angularAcceleration = computed({
  get() {
    return robotStore.angularAccel
  },
  set(angularAcceleration: number) {
    robotStore.angularAccel = angularAcceleration
  }
})
const maxTorque = computed({
  get() {
    return robotStore.maxTorque
  },
  set(maxTorque: number) {
    robotStore.maxTorque = maxTorque
  }
})

</script>

<template>
  <div class="robot-control-layout">
    <h2>Kinematic Settings</h2>
    <DataSlider
      label="Max Angular Velocity (rad/s)"
      v-model="maxAngularVelocity"
      :min="0"
      :max="270"
      :step="0.1"
      :width="400"
      :labelWidth="140"
    />
    <DataSlider
      label="Angular Acceleration (rad/s²)"
      v-model="angularAcceleration"
      :min="0"
      :max="100"
      :step="0.1"
      :width="400"
      :labelWidth="140"
    />
    <DataSlider
      label="Max Torque (Nm)"
      v-model="maxTorque"
      :min="0"
      :max="100"
      :step="0.1"
      :width="400"
      :labelWidth="140"
    />

  </div>
</template>

<style scoped>
.robot-control-layout {
  width: 400px;
}
.slider-pair {
  display: flex;
  flex-direction: row;
  gap: 1.5rem;
}
</style>
