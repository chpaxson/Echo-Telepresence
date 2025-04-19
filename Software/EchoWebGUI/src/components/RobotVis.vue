<script setup lang="ts">
import { ref } from 'vue'
import type { Point } from '../utils/types.ts'
import '../utils/utils.ts'

const props = defineProps({
  label: { type: String, required: true },
  width: { type: Number, default: 600 },
})

const scale = props.width / 600

const joint3 = ref<Point>({ x: 0, y: 0 })
const offset = { x: props.width / 2, y: props.width / 8 }
// const draggingEffector = ref(false)
let dragOffset = { x: 0, y: 0 }

function onDragStart(event: MouseEvent) {
  // Calculate offset from mouse to joint3 center
  dragOffset = {
    x: joint3.value.x / scale - event.clientX,
    y: joint3.value.y / scale - event.clientY,
  }
  window.addEventListener('mousemove', onDragMove)
  window.addEventListener('mouseup', onDragEnd)
}

function onDragMove(event: MouseEvent) {
  // Convert mouse position to robot coordinates
  joint3.value.x = (event.clientX + dragOffset.x) * scale
  joint3.value.y = (event.clientY + dragOffset.y) * scale
}

function onDragEnd() {
  window.removeEventListener('mousemove', onDragMove)
  window.removeEventListener('mouseup', onDragEnd)
}

function onTouchStart(event: TouchEvent) {
  event.preventDefault()
  dragOffset = {
    x: joint3.value.x / scale - event.touches[0].clientX,
    y: joint3.value.y / scale - event.touches[0].clientY,
  }
  window.addEventListener('touchmove', onTouchMove, { passive: false })
  window.addEventListener('touchend', onTouchEnd)
}
function onTouchMove(event: TouchEvent) {
  event.preventDefault()
  joint3.value.x = (event.touches[0].clientX + dragOffset.x) * scale
  joint3.value.y = (event.touches[0].clientY + dragOffset.y) * scale
}
function onTouchEnd() {
  window.removeEventListener('touchmove', onTouchMove)
  window.removeEventListener('touchend', onTouchEnd)
}
</script>

<template>
  <svg :width="`${props.width}`" :height="`${props.width * 0.75}`" style="border: 1px solid #ccc">
    <path :id="`${props.label}-u1`" />
    <path :id="`${props.label}-u2`" />
    <path :id="`${props.label}-l2`" />
    <path :id="`${props.label}-l1`" />
    <circle
      :cx="joint3.x + offset.x"
      :cy="joint3.y + offset.y"
      r="12"
      fill="#4af"
      style="cursor: grab"
      @mousedown="onDragStart"
      @touchstart="onTouchStart"
    />
  </svg>
</template>
