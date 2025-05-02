<script setup lang="ts">
import type { Point } from '../utils/types.ts'
import '../utils/utils.ts'
import { useRobot1Store } from '../stores/robot1'
import { useRobot2Store } from '../stores/robot2'
import { getDist, getAngle, addPoints, scalePoint, polar } from '../utils/utils'
import { b1, b2, armLen, armRadii, calc_ik } from '../utils/kinematics'
import { boundaryArcs } from '../utils/kinematics'
import { validConfiguration, projectToWorkspace } from '../utils/kinematics'
import { watch, onUnmounted, onMounted } from 'vue'

const props = defineProps({
  robot: { type: String, required: true },
  width: { type: Number, default: 600 },
})

const scale = props.width / 700
const offset = { x: props.width / 2, y: props.width / 8 }
let dragOffset = { x: 0, y: 0 }

function shift(p: Point): Point {
  return addPoints(scalePoint(p, scale), offset)
}

// Dynamically select the store
const robotStore = props.robot === 'robot1'
  ? useRobot1Store()
  : useRobot2Store()

function drawArmSegment(element: string, c1: Point, c2: Point, r1: number, r2: number): void {
  // Shift everything into the canvas
  const dist = getDist(c1, c2)
  const angle = getAngle(c1, c2)
  const dr = Math.abs(r1 - r2)// / 2
  const dangle = Math.acos(dr / dist)
  const angle1 = angle - dangle
  const angle2 = angle + dangle

  const pa0: Point = shift(addPoints(c1, polar(angle1, r1)))
  const pa1: Point = shift(addPoints(c1, polar(angle2, r1)))
  const pb0: Point = shift(addPoints(c2, polar(angle1, r2)))
  const pb1: Point = shift(addPoints(c2, polar(angle2, r2)))

  r1 = r1 * scale
  r2 = r2 * scale

  // Draw the arm segment
  const arc1 = document.getElementById(element)
  arc1?.setAttribute(
    'd',
    `M ${pa0.x} ${pa0.y} A ${r1} ${r1} 0 ${r1>r2?1:0} 0 ${pa1.x} ${pa1.y} L ${pb1.x} ${pb1.y} A ${r2} ${r2} 0 ${r1>r2?0:1} 0 ${pb0.x} ${pb0.y} Z`,
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
  console.log(validConfiguration(shift(newPos), props.robot))
  robotStore.joint3 = validConfiguration(shift(newPos), props.robot) ? newPos : projectToWorkspace(newPos)
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
  robotStore.joint3 = validConfiguration(shift(newPos), props.robot) ? newPos : projectToWorkspace(newPos)
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
  drawArmSegment(`${props.robot}-u1`, b1, j1, armRadii.u1, armRadii.u2)
  drawArmSegment(`${props.robot}-u2`, b2, j2, armRadii.u1, armRadii.u2)
  drawArmSegment(`${props.robot}-l2`, j2, robotStore.joint3, armRadii.l1, armRadii.l2)
  drawArmSegment(`${props.robot}-l1`, j1, robotStore.joint3, armRadii.l1, armRadii.l2)
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

function drawBoundaries() {
  const p1 = shift(boundaryArcs[0].s)
  const p2 = shift(boundaryArcs[0].e)
  const p3 = shift(boundaryArcs[1].e)
  const p4 = shift(boundaryArcs[2].e)
  const p5 = shift(boundaryArcs[3].e)
  const p6 = shift(boundaryArcs[4].e)
  const p7 = shift(boundaryArcs[5].e)
  const r1 = boundaryArcs[0].r * scale
  const r2 = boundaryArcs[2].r * scale
  const r3 = boundaryArcs[3].r * scale
  const r4 = boundaryArcs[4].r * scale

  document.getElementById(props.robot+"-boundary")?.setAttribute(
    'd',
    `M ${p1.x} ${p1.y} A ${r1} ${r1} 0 0 0 ${p2.x} ${p2.y} A ${r2} ${r2} 0 0 0 ${p3.x} ${p3.y} A ${r3} ${r3} 0 0 1 ${p4.x} ${p4.y} A ${r4} ${r4} 0 0 0 ${p5.x} ${p5.y} A ${r3} ${r3} 0 0 1 ${p6.x} ${p6.y} A ${r2} ${r2} 0 0 0 ${p7.x} ${p7.y} A ${r1} ${r1} 0 0 0 ${p1.x} ${p1.y}`,
  )
}
onMounted(() => {
  drawBoundaries()
})

// const ba1 = shift(boundaryArcs[3].s)
// const ba2 = shift(boundaryArcs[3].e)

</script>

<template>
  <svg :width="`${props.width}`" :height="`${props.width * 0.75}`" style="border: 1px solid #ccc" :id="`${props.robot}-svg`">
    <!-- <circle :cx="shift(boundaryArcs[0].c).x" :cy="shift(boundaryArcs[0].c).y" :r="scale*boundaryArcs[0].r" stroke="red" fill="none" />
    <circle :cx="shift(boundaryArcs[1].c).x" :cy="shift(boundaryArcs[1].c).y" :r="scale*boundaryArcs[1].r" stroke="green" fill="none" />
    <circle :cx="shift(boundaryArcs[2].c).x" :cy="shift(boundaryArcs[2].c).y" :r="scale*boundaryArcs[2].r" stroke="blue" fill="none" />
    <circle :cx="shift(boundaryArcs[3].c).x" :cy="shift(boundaryArcs[3].c).y" :r="scale*boundaryArcs[3].r" stroke="purple" fill="none" />
    <circle :cx="shift(boundaryArcs[4].c).x" :cy="shift(boundaryArcs[4].c).y" :r="scale*boundaryArcs[4].r" stroke="white" fill="none" />
    <circle :cx="shift(boundaryArcs[5].c).x" :cy="shift(boundaryArcs[5].c).y" :r="scale*boundaryArcs[5].r" stroke="teal" fill="none" />
    <circle :cx="shift(boundaryArcs[6].c).x" :cy="shift(boundaryArcs[6].c).y" :r="scale*boundaryArcs[6].r" stroke="yellow" fill="none" /> -->

    <path :id="`${props.robot}-boundary`" stroke="#555555" fill="#333333"/>
    <!-- <circle :cx="ba1.x" :cy="ba1.y" r="5" stroke="yellow" fill="none" /> -->
    <!-- <circle :cx="ba2.x" :cy="ba2.y" r="5" stroke="yellow" fill="none" /> -->

    <path :id="`${props.robot}-u1`" />
    <path :id="`${props.robot}-u2`" />
    <path :id="`${props.robot}-l2`" />
    <path :id="`${props.robot}-l1`" />
    <circle
      :cx="scale * robotStore.joint3.x + offset.x"
      :cy="scale * robotStore.joint3.y + offset.y"
      :r="10*scale"
      fill="#4af"
      style="cursor: grab"
      @mousedown="onDragStart"
      @touchstart="onTouchStart"
    />
  </svg>
</template>
