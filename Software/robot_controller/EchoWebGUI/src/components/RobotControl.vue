<script setup lang="ts">
import '../utils/utils.ts'
import { useRobot1Store } from '../stores/robot1'
import { useRobot2Store } from '../stores/robot2'
import DataSlider from './DataSlider.vue'
import { calc_fk, calc_ik, projectToWorkspace } from '../utils/kinematics'
import { computed } from 'vue'

const props = defineProps({
  robot: { type: String, required: true },
  width: { type: Number, default: 600 },
})

const robotStore = props.robot === 'robot1'
  ? useRobot1Store()
  : useRobot2Store()

// Computed for Joint 1 angle in degrees
const joint1Deg = computed({
  get() {
    return (calc_ik(robotStore.joint3).a1 * 180) / Math.PI
  },
  set(deg: number) {
    const angles = calc_ik(robotStore.joint3)
    const newAngles = { a1: (deg * Math.PI) / 180, a2: angles.a2 }
    robotStore.joint3 = calc_fk(newAngles.a1, newAngles.a2)
  }
})

// Computed for Joint 2 angle in degrees
const joint2Deg = computed({
  get() {
    return (calc_ik(robotStore.joint3).a2 * 180) / Math.PI
  },
  set(deg: number) {
    const angles = calc_ik(robotStore.joint3)
    const newAngles = { a1: angles.a1, a2: (deg * Math.PI) / 180 }
    robotStore.joint3 = calc_fk(newAngles.a1, newAngles.a2)
  }
})

const xPos = computed({
  get() {
    return robotStore.joint3.x
  },
  set(x: number) {
    const projected = projectToWorkspace({ x, y: robotStore.joint3.y }, "x")
    robotStore.joint3 = projected
  }
})

const yPos = computed({
  get() {
    return robotStore.joint3.y
  },
  set(y: number) {
    const projected = projectToWorkspace({ x: robotStore.joint3.x, y }, "y")
    robotStore.joint3 = projected
  }
})
</script>

<template>
  <div class="robot-control-layout">
    <h2>Joint Angles</h2>
    <DataSlider
      label="Joint 1"
      v-model="joint1Deg"
      :min="-90"
      :max="270"
      :step="1"
      :width="400"
    />
    <DataSlider
      label="Joint 2"
      v-model="joint2Deg"
      :min="-90"
      :max="270"
      :step="1"
      :width="400"
    />
    <h2>End Effector Position</h2>
    <DataSlider
      label="X"
      v-model="xPos"
      :min="-300"
      :max="300"
      :step="1"
      :width="400"
    />
    <DataSlider
      label="Y"
      v-model="yPos"
      :min="-100"
      :max="400"
      :step="1"
      :width="400"
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
