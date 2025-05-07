<script setup lang="ts">
import { ref, onMounted, onUnmounted, nextTick } from 'vue'
import '../utils/utils.ts'
import RobotVis from '../components/RobotVis.vue'
import { useRobot1Store } from '../stores/robot1'
import { useRobot2Store } from '../stores/robot2'


const visWidth = ref(400)
const viswrapper = ref<HTMLElement | null>(null)
const gap = 100 // 2rem gap in px

const mappingScale = ref(1)
const enableLink = ref(false)

const robot1Store = useRobot1Store()
const robot2Store = useRobot2Store()

let resizeObserver: ResizeObserver | null = null

function updateWidth() {
  if (viswrapper.value) {
    // Subtract the gap between the two columns and limit to 500 max per column
    visWidth.value = viswrapper.value.clientWidth - gap
  }
}

onMounted(() => {
  nextTick(() => {
    updateWidth()
    if (viswrapper.value) {
      resizeObserver = new ResizeObserver(() => {
        updateWidth()
      })
      resizeObserver.observe(viswrapper.value)
    }
  })
})

onUnmounted(() => {
  if (resizeObserver && viswrapper.value) {
    resizeObserver.unobserve(viswrapper.value)
  }
})
</script>

<template>
  <div style="width: 100%; justify-content: center; align-items: center; display: flex;">
    <div class="dashboard">
      <div class="vis" ref="viswrapper">
        <h1 style="user-select: none;">Robot 1</h1>
        <RobotVis robot="robot1" :width="visWidth" />
      </div>
      <div class="vis">
        <h1 style="user-select: none;">Robot 2</h1>
        <RobotVis robot="robot2" :width="visWidth" />
      </div>
      <div id="linkcontrolwrapper">
        <h1 style="user-select: none;">Link Controls</h1>
        <div class="flex gap-2 items-center">
          <label
            class="text-white text-[15px] leading-none pr-[15px] select-none"
            for="linkEnable"
          >
            Position Link:
          </label>
          <!-- <input type="checkbox" class="toggle" disabled /> -->
        </div>
      </div>
    </div>
  </div>
</template>

<style>
.dashboard {
  width: min(100%, 1200px);
  height: 100%;
  display: grid;
  gap: 32px;
  grid-template-columns: 2;
  grid-template-rows: auto auto;

  align-items: start;
  justify-items: center;
}

.viswrapper {
  display: contents; /* Allows children to participate directly in the grid */
}

.vis {
  border-radius: 1rem;
  background: var(--color-background);
  align-self: flex-start;
  border: 2px solid var(--color-border);
  width: min(100%, 600px);
  min-width: 0;
  display: flex;
  flex-direction: column;
  align-items: center;
}

#linkcontrolwrapper {
  grid-column: 1 / span 2; /* Span both columns */
  margin-top: 2rem;
  width: 100%;
  display: flex;
  flex-direction: column;
  /* align-items: center; */
}
</style>
