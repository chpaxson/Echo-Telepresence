<script setup lang="ts">
import { computed } from 'vue'

const props = defineProps<{
  label: string
  modelValue: number
  min?: number
  max?: number
  step?: number
  width?: number
  labelWidth?: number
}>()

const emit = defineEmits<{
  (e: 'update:modelValue', value: number): void
}>()

const width = computed(() => props.width ?? 300)
const labelWidth = computed(() => props.labelWidth ?? 60)

function updateValue(val: string | number) {
  let num = typeof val === 'number' ? val : Number(val)
  if (isNaN(num)) num = props.min ?? 0
  if (props.min !== undefined) num = Math.max(props.min, num)
  if (props.max !== undefined) num = Math.min(props.max, num)
  emit('update:modelValue', num)
}
</script>

<template>
  <div class="slider-row" :style="{ width: width + 'px' }">
    <label class="slider-label" :style="{ width: labelWidth + 'px', flex: `0 0 ${labelWidth}px` }">{{ label }}</label>
    <input
      class="slider-range"
      type="range"
      :min="min"
      :max="max"
      :step="step"
      :value="modelValue"
      @input="updateValue(($event.target as HTMLInputElement).value)"
    />
    <input
      class="slider-number"
      type="number"
      :min="min"
      :max="max"
      :step="step"
      :value="modelValue"
      @input="updateValue(($event.target as HTMLInputElement).value)"
    />
  </div>
</template>

<style scoped>
.slider-row {
  display: flex;
  align-items: center;
  gap: 0.5rem;
  width: 400px; /* Overridden by :style from the prop */
}
.slider-label {
  text-align: right;
  margin-right: 0.5rem;
  /* width and flex set inline */
}
.slider-range {
  flex: 1 1 0;
  min-width: 0;
  height: 4px;
  border-radius: 8px;
}
.slider-number {
  flex: 0 0 60px;
  margin-left: 0.5rem;
  border: none;
  border-radius: 10px;
  background: #e6f0fa;
  padding: 0.3em 0.8em;
  text-align: center;
  font-size: 0.8em;
  outline: none;
  box-shadow: 0 1px 2px rgba(0,0,0,0.04);
  transition: box-shadow 0.2s;
  -webkit-appearance: none;
  -moz-appearance: textfield;
}
.slider-number::-webkit-outer-spin-button,
.slider-number::-webkit-inner-spin-button {
  -webkit-appearance: none;
  margin: 0;
}
.slider-number[type=number] {
  -moz-appearance: textfield;
}
.slider-number:focus {
  box-shadow: 0 0 0 2px #4af55a44;
}
</style>
