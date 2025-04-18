<script setup lang="ts">
import '../utils/utils.ts'

const props = defineProps({
  label: { type: String, required: true },
  width: { type: Number, default: 600 },
})

const joint3 = { x: 0, y: 0 }
const offset = { x: props.width / 2, y: props.width / 8 }

// Move joint3 around as the mouse moves
const onDragStart = (event: MouseEvent) => {
  const svg = event.target.closest('svg')
  if (!svg) return

  const offsetX = event.clientX - joint3.x
  const offsetY = event.clientY - joint3.y

  const onMouseMove = (e: MouseEvent) => {
    joint3.x = e.clientX - offsetX
    joint3.y = e.clientY - offsetY
    updatePaths()
  }

  const onMouseUp = () => {
    window.removeEventListener('mousemove', onMouseMove)
    window.removeEventListener('mouseup', onMouseUp)
  }

  window.addEventListener('mousemove', onMouseMove)
  window.addEventListener('mouseup', onMouseUp)
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
    />
  </svg>
</template>
