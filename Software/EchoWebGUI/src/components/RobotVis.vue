<script setup lang="ts">
import { ref, computed, watch, onMounted } from 'vue'
import Slider from './Slider.vue'
import DataGraphs from './DataGraphs.vue'

const props = defineProps({
  label: { type: String, default: 'Robot' },
  initialAngle1: { type: Number, default: 135.0 },
  initialAngle2: { type: Number, default: 45.0 },
})

interface Point {
  x: number
  y: number
}
interface Configuration {
  a1: number
  a2: number
}
const maxAngularVelocity = ref(40) // deg/s
const maxAngularAcceleration = ref(180) // deg/s^2
const maxTorque = ref(0.5) // Nm

const b1: Point = { x: -50, y: 0 }
const b2: Point = { x: 50, y: 0 }
const armLen = { u: 136.515, l: 175 }
const armRadii = { u1: 20.985, u2: 17.5, l1: 17.5, l2: 17.5 }

function distance(a: Point, b: Point): number {
  return Math.sqrt(Math.pow(a.x - b.x, 2) + Math.pow(a.y - b.y, 2))
}
function getAngle(a: Point, b: Point): number {
  return Math.atan2(b.y - a.y, b.x - a.x)
}
function addPoints(a: Point, b: Point): Point {
  return { x: a.x + b.x, y: a.y + b.y }
}
function dirLen(angle: number, length: number): Point {
  return { x: length * Math.cos(angle), y: length * Math.sin(angle) }
}

// Forward Kinematics - Calculate end effector position based on joint angles (a1, a2, in radians)
function calc_fk(a: Configuration): Point {
  const e1: Point = addPoints(b1, dirLen(a.a1, armLen.u))
  const e2: Point = addPoints(b2, dirLen(a.a2, armLen.u))

  const dist = distance(e1, e2)
  const angle = getAngle(e1, e2)
  const mid: Point = { x: (e1.x + e2.x) / 2, y: (e1.y + e2.y) / 2 }
  const d = Math.sqrt(Math.pow(armLen.l, 2) - Math.pow(dist / 2, 2))

  return {
    x: mid.x + d * Math.cos(angle + Math.PI / 2),
    y: mid.y + d * Math.sin(angle + Math.PI / 2),
  }
}

// Inverse Kinematics - Calculate joint angles based on end effector position (x, y)
function calc_ik(e: Point): Configuration {
  const b1dist = distance(b1, e)
  const b2dist = distance(b2, e)
  const b1angle = getAngle(b1, e)
  const b2angle = getAngle(b2, e)

  const a1 =
    b1angle + Math.acos(-(armLen.l ** 2 - armLen.u ** 2 - b1dist ** 2) / (2 * armLen.u * b1dist))
  const a2 =
    b2angle - Math.acos(-(armLen.l ** 2 - armLen.u ** 2 - b2dist ** 2) / (2 * armLen.u * b2dist))

  return { a1: a1, a2: a2 }
}

const angle1 = ref(props.initialAngle1)
const angle2 = ref(props.initialAngle2)

const a1 = computed(() => (angle1.value * Math.PI) / 180)
const a2 = computed(() => (angle2.value * Math.PI) / 180)

const joint1 = computed(() => addPoints(b1, dirLen(a1.value, armLen.u)))
const joint2 = computed(() => addPoints(b2, dirLen(a2.value, armLen.u)))
const joint3 = computed(() => calc_fk({ a1: a1.value, a2: a2.value }))

// Add refs for joint3 position (in robot coordinates)
const joint3X = ref(joint3.value.x)
const joint3Y = ref(joint3.value.y)

const draggingEffector = ref(false)
let dragOffset = { x: 300, y: 75 }

function onDragStart(event: MouseEvent) {
  draggingEffector.value = true
  // Calculate offset from mouse to joint3 center
  dragOffset = {
    x: joint3.value.x + 300 - event.clientX,
    y: joint3.value.y + 75 - event.clientY,
  }
  window.addEventListener('mousemove', onDragMove)
  window.addEventListener('mouseup', onDragEnd)
}

function onDragMove(event: MouseEvent) {
  if (!draggingEffector.value) return
  // Convert mouse position to robot coordinates
  const intendedX = event.clientX + dragOffset.x - 300
  const intendedY = event.clientY + dragOffset.y - 75
  const projected = projectToWorkspace(intendedX, intendedY)
  joint3X.value = projected.x
  joint3Y.value = projected.y
}

function onDragEnd() {
  draggingEffector.value = false
  window.removeEventListener('mousemove', onDragMove)
  window.removeEventListener('mouseup', onDragEnd)
}

onMounted(() => {
  // Draw the arm segments
  drawArmSegment(`${props.label}-u1`, b1, joint1.value, armRadii.u1, armRadii.u2)
  drawArmSegment(`${props.label}-u2`, b2, joint2.value, armRadii.u1, armRadii.u2)
  drawArmSegment(`${props.label}-l2`, joint2.value, joint3.value, armRadii.l1, armRadii.l2)
  drawArmSegment(`${props.label}-l1`, joint1.value, joint3.value, armRadii.l1, armRadii.l2)
})

// Keep joint3X/Y in sync with angles, but project to workspace if needed
watch([angle1, angle2], ([newAngle1, newAngle2]) => {
  // Compute intended joint3 position from angles
  const fk = calc_fk({
    a1: (newAngle1 * Math.PI) / 180,
    a2: (newAngle2 * Math.PI) / 180,
  })
  // Project to workspace to ensure validity
  const projected = projectToWorkspace(fk.x, fk.y)
  joint3X.value = projected.x
  joint3Y.value = projected.y

  drawArmSegment(`${props.label}-u1`, b1, joint1.value, armRadii.u1, armRadii.u2)
  drawArmSegment(`${props.label}-u2`, b2, joint2.value, armRadii.u1, armRadii.u2)
  drawArmSegment(`${props.label}-l2`, joint2.value, joint3.value, armRadii.l1, armRadii.l2)
  drawArmSegment(`${props.label}-l1`, joint1.value, joint3.value, armRadii.l1, armRadii.l2)
})

// When joint3X/Y change, update angles via IK, and project to workspace if needed
watch([joint3X, joint3Y], ([newX, newY]) => {
  const projected = projectToWorkspace(newX, newY)
  const ik = calc_ik({ x: projected.x, y: projected.y })
  angle1.value = (ik.a1 * 180) / Math.PI
  angle2.value = (ik.a2 * 180) / Math.PI
})

function drawArmSegment(element: string, c1: Point, c2: Point, r1: number, r2: number): void {
  // Shift everything into the canvas
  c1 = addPoints(c1, { x: 300, y: 75 })
  c2 = addPoints(c2, { x: 300, y: 75 })
  const dist = distance(c1, c2)
  const angle = getAngle(c1, c2)
  const dr = Math.abs(r1 - r2) / 2
  const dangle = Math.acos(dr / dist)
  const angle1 = angle - dangle
  const angle2 = angle + dangle

  const pa0: Point = addPoints(c1, dirLen(angle1, r1))
  const pac: Point = addPoints(c1, dirLen(angle, -r1))
  const pa1: Point = addPoints(c1, dirLen(angle2, r1))

  const pb0: Point = addPoints(c2, dirLen(angle1, r2))
  const pbc: Point = addPoints(c2, dirLen(angle, r2))
  const pb1: Point = addPoints(c2, dirLen(angle2, r2))

  // Draw the arm segment
  const arc1 = document.getElementById(element)
  arc1?.setAttribute(
    'd',
    `M ${pa0.x} ${pa0.y} A ${r1} ${r1} 0 0 0 ${pac.x} ${pac.y} A ${r1} ${r1} 0 0 0 ${pa1.x} ${pa1.y} L ${pb1.x} ${pb1.y} A ${r1} ${r1} 0 0 0 ${pbc.x} ${pbc.y} A ${r1} ${r1} 0 0 0 ${pb0.x} ${pb0.y} Z`,
  )
  arc1?.setAttribute('stroke', 'white')
}

function isValidConfiguration(a1: number, a2: number): boolean {
  const j1 = addPoints(b1, dirLen(a1, armLen.u))
  const j2 = addPoints(b2, dirLen(a2, armLen.u))
  const j3 = calc_fk({ a1, a2 })
  const elbow = distance(j1, j2)
  const b1d = distance(j3, b1)
  const b2d = distance(j3, b2)
  return elbow < 2 * armLen.l && b1d < armLen.l + armLen.u && b2d < armLen.l + armLen.u
}

function projectToWorkspace(x: number, y: number): { x: number; y: number } {
  // Try the intended point first
  const ik = calc_ik({ x, y })
  if (isValidConfiguration(ik.a1, ik.a2)) {
    return { x, y }
  }

  // Binary search from the center of the workspace toward the intended point
  const cx = 0
  const cy = 0
  let lo = 0
  let hi = 1
  let best = { x: joint3.value.x, y: joint3.value.y }

  // Find the maximum t in [0,1] such that the point is valid
  for (let i = 0; i < 16; i++) {
    const t = (lo + hi) / 2
    const testX = cx + (x - cx) * t
    const testY = cy + (y - cy) * t
    const ikTest = calc_ik({ x: testX, y: testY })
    if (isValidConfiguration(ikTest.a1, ikTest.a2)) {
      best = { x: testX, y: testY }
      lo = t
    } else {
      hi = t
    }
  }
  return best
}
</script>

<template>
  <div>
    <h2>{{ label }}</h2>
    <svg :width="600" :height="425" style="border: 1px solid #ccc; touch-action: none">
      <path :id="`${props.label}-u1`" />
      <path :id="`${props.label}-u2`" />
      <path :id="`${props.label}-l2`" />
      <path :id="`${props.label}-l1`" />
      <circle
        :cx="joint3.x + 300"
        :cy="joint3.y + 75"
        r="12"
        fill="#4af"
        stroke="#222"
        stroke-width="2"
        style="cursor: grab"
        @mousedown="onDragStart"
      />
    </svg>
    <h3>Joint Angles</h3>
    <div style="display: flex; gap: 1em">
      <Slider v-model="angle1" :min="-180" :max="180" :step="0.1" width="300px" label="Angle 1" />
      <Slider v-model="angle2" :min="-180" :max="180" :step="0.1" width="300px" label="Angle 2" />
    </div>
    <h3>End Effector Position</h3>
    <div style="display: flex; gap: 1em">
      <Slider v-model="joint3X" :min="-300" :max="300" :step="1" width="300px" label="End X" />
      <Slider v-model="joint3Y" :min="-50" :max="350" :step="1" width="300px" label="End Y" />
    </div>
    <h3>Maximum Kinematic Limits</h3>
    <div style="display: flex; gap: 1em; margin-bottom: 0.5em">
      <Slider
        v-model="maxAngularVelocity"
        :min="1"
        :max="120"
        :step="1"
        width="300px"
        label="Angular Velocity (deg/s)"
      />
      <Slider
        v-model="maxTorque"
        :min="0.1"
        :max="0.6"
        :step="0.01"
        width="300px"
        label="Torque (Nm)"
      />
    </div>
    <div style="display: flex; gap: 1em">
      <Slider
        v-model="maxAngularAcceleration"
        :min="1"
        :max="720"
        :step="1"
        width="600px"
        label="Angular Acceleration (deg/s²)"
      />
    </div>
  </div>
</template>
