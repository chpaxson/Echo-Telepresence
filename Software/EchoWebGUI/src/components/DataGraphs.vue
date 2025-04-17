<script setup lang="ts">
import { ref, watch, onMounted, computed } from 'vue'
import { Line } from 'vue-chartjs'
import {
  Chart,
  LineController,
  LineElement,
  PointElement,
  LinearScale,
  Title,
  CategoryScale,
  Legend,
  Tooltip,
} from 'chart.js'

Chart.register(
  LineController,
  LineElement,
  PointElement,
  LinearScale,
  Title,
  CategoryScale,
  Legend,
  Tooltip,
)

// Props: arrays of data for each trace
const props = defineProps<{
  time: number[]
  pos1: number[]
  pos2: number[]
  pos1Target: number[]
  pos2Target: number[]
  vel1: number[]
  vel2: number[]
  vel1Target: number[]
  vel2Target: number[]
  torque1: number[]
  torque2: number[]
  torque1Target: number[]
  torque2Target: number[]
}>()

const WINDOW_SECONDS = 10

const time = ref(props.time)

// Chart options (shared)
const options = computed(() => ({
  responsive: true,
  animation: { duration: 0 }, // <-- Fix here
  scales: {
    x: {
      title: { display: true, text: 'Time (s)' },
      min: Math.max(0, props.time.length ? props.time[props.time.length - 1] - WINDOW_SECONDS : 0),
      max: props.time.length ? props.time[props.time.length - 1] : WINDOW_SECONDS,
    },
  },
}))
</script>

<template>
  <div>
    <h3>Angular Position</h3>
    <Line
      :data="{
        labels: props.time,
        datasets: [
          { label: 'Joint 1', data: props.pos1, borderColor: '#1976d2', fill: false },
          { label: 'Joint 2', data: props.pos2, borderColor: '#388e3c', fill: false },
          {
            label: 'Joint 1 Target',
            data: props.pos1Target,
            borderColor: '#1976d2',
            borderDash: [5, 5],
            fill: false,
          },
          {
            label: 'Joint 2 Target',
            data: props.pos2Target,
            borderColor: '#388e3c',
            borderDash: [5, 5],
            fill: false,
          },
        ],
      }"
      :options="options"
      style="height: 200px"
    />

    <h3>Angular Velocity</h3>
    <Line
      :data="{
        labels: props.time,
        datasets: [
          { label: 'Joint 1', data: props.vel1, borderColor: '#fbc02d', fill: false },
          { label: 'Joint 2', data: props.vel2, borderColor: '#d32f2f', fill: false },
          {
            label: 'Joint 1 Target',
            data: props.vel1Target,
            borderColor: '#fbc02d',
            borderDash: [5, 5],
            fill: false,
          },
          {
            label: 'Joint 2 Target',
            data: props.vel2Target,
            borderColor: '#d32f2f',
            borderDash: [5, 5],
            fill: false,
          },
        ],
      }"
      :options="options"
      style="height: 200px"
    />

    <h3>Torque</h3>
    <Line
      :data="{
        labels: props.time,
        datasets: [
          { label: 'Joint 1', data: props.torque1, borderColor: '#7b1fa2', fill: false },
          { label: 'Joint 2', data: props.torque2, borderColor: '#0288d1', fill: false },
          {
            label: 'Joint 1 Target',
            data: props.torque1Target,
            borderColor: '#7b1fa2',
            borderDash: [5, 5],
            fill: false,
          },
          {
            label: 'Joint 2 Target',
            data: props.torque2Target,
            borderColor: '#0288d1',
            borderDash: [5, 5],
            fill: false,
          },
        ],
      }"
      :options="options"
      style="height: 200px"
    />
  </div>
</template>
