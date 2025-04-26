<script setup lang="ts">
import type { Point } from '../utils/types.ts'
import '../utils/utils.ts'
import { useRobot1Store } from '../stores/robot1'
import { useRobot2Store } from '../stores/robot2'
import { getDist, getAngle, addPoints, scalePoint, polar } from '../utils/utils'
import { b1, b2, armLen, armRadii, calc_ik } from '../utils/kinematics'
import { validConfiguration, projectToWorkspace } from '../utils/kinematics'
import { watch, onUnmounted, onMounted } from 'vue'

const props = defineProps({
  robot: { type: String, required: true },
  width: { type: Number, default: 600 },
})

const scale = props.width / 600
const offset = { x: props.width / 2, y: props.width / 8 }
let dragOffset = { x: 0, y: 0 }

// Dynamically select the store
const robotStore = props.robot === 'robot1'
  ? useRobot1Store()
  : useRobot2Store()

function drawArmSegment(element: string, scale: number, c1: Point, c2: Point, r1: number, r2: number): void {
  // Shift everything into the canvas
  c1 = addPoints(c1, offset)
  c2 = addPoints(c2, offset)
  const dist = getDist(c1, c2)
  const angle = getAngle(c1, c2)
  const dr = Math.abs(r1 - r2)// / 2
  const dangle = Math.acos(dr / dist)
  const angle1 = angle - dangle
  const angle2 = angle + dangle

  const pa0: Point = scalePoint(addPoints(c1, polar(angle1, r1)), scale)
  const pa1: Point = scalePoint(addPoints(c1, polar(angle2, r1)), scale)

  const pb0: Point = scalePoint(addPoints(c2, polar(angle1, r2)), scale)
  const pb1: Point = scalePoint(addPoints(c2, polar(angle2, r2)), scale)

  // Draw the arm segment
  const arc1 = document.getElementById(element)
  arc1?.setAttribute(
    'd',
    `M ${pa0.x} ${pa0.y} A ${r1*scale} ${r1*scale} 0 ${r1>r2?1:0} 0 ${pa1.x} ${pa1.y} L ${pb1.x} ${pb1.y} A ${r2*scale} ${r2*scale} 0 ${r1>r2?0:1} 0 ${pb0.x} ${pb0.y} Z`,
  )
  arc1?.setAttribute('stroke', 'white')
}

function onDragStart(event: MouseEvent) {
  dragOffset = { x: robotStore.joint3.x * scale - event.clientX, y: robotStore.joint3.y * scale - event.clientY }
  window.addEventListener('mousemove', onDragMove)
  window.addEventListener('mouseup', onDragEnd)
}
function onDragMove(event: MouseEvent) {
  const newPos = { x: (event.clientX + dragOffset.x) / scale, y: (event.clientY + dragOffset.y) / scale }
  robotStore.joint3 = validConfiguration(newPos) ? newPos : projectToWorkspace(newPos, "drag")
}
function onDragEnd() {
  window.removeEventListener('mousemove', onDragMove)
  window.removeEventListener('mouseup', onDragEnd)
}

function onTouchStart(event: TouchEvent) {
  event.preventDefault()
  dragOffset = { x: robotStore.joint3.x * scale - event.touches[0].clientX, y: robotStore.joint3.y * scale - event.touches[0].clientY }
  window.addEventListener('touchmove', onTouchMove, { passive: false })
  window.addEventListener('touchend', onTouchEnd)
}
function onTouchMove(event: TouchEvent) {
  event.preventDefault()
  const newPos = { x: (event.touches[0].clientX + dragOffset.x) / scale, y: (event.touches[0].clientY + dragOffset.y) / scale }
  robotStore.joint3 = validConfiguration(newPos) ? newPos : projectToWorkspace(newPos, "drag")
}
function onTouchEnd() {
  window.removeEventListener('touchmove', onTouchMove)
  window.removeEventListener('touchend', onTouchEnd)
}

let lastDraw = 0
let pending = false

function drawRobot() {
  const angles = calc_ik(robotStore.joint3)
  const j1 = addPoints(b1, polar(angles.a1, armLen.u))
  const j2 = addPoints(b2, polar(angles.a2, armLen.u))
  drawArmSegment(`${props.robot}-u1`, scale, b1, j1, armRadii.u1, armRadii.u2)
  drawArmSegment(`${props.robot}-u2`, scale, b2, j2, armRadii.u1, armRadii.u2)
  drawArmSegment(`${props.robot}-l2`, scale, j2, robotStore.joint3, armRadii.l1, armRadii.l2)
  drawArmSegment(`${props.robot}-l1`, scale, j1, robotStore.joint3, armRadii.l1, armRadii.l2)
}
const stop = watch(
  () => ({ ...robotStore.joint3 }), // watch for changes in joint3
  () => {
    const now = performance.now()
    if (now - lastDraw > 20) {
      drawRobot()
      lastDraw = now
    } else if (!pending) {
      pending = true
      setTimeout(() => {
        drawRobot()
        lastDraw = performance.now()
        pending = false
      }, 20 - (now - lastDraw))
    }
  },
  { deep: true }
)

onMounted(() => {drawRobot()})
onUnmounted(() => stop())
</script>

<template>
  <svg :width="`${props.width}`" :height="`${props.width * 0.75}`" style="border: 1px solid #ccc">
    <path :id="`${props.robot}-u1`" />
    <path :id="`${props.robot}-u2`" />
    <path :id="`${props.robot}-l2`" />
    <path :id="`${props.robot}-l1`" />
    <circle
      :cx="scale * (robotStore.joint3.x + offset.x)"
      :cy="scale * (robotStore.joint3.y + offset.y)"
      :r="10*scale"
      fill="#4af"
      style="cursor: grab"
      @mousedown="onDragStart"
      @touchstart="onTouchStart"
    />
  </svg>
</template>
