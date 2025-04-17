<template>
  <div class="slider-container" :style="{ width }">
    <label class="slider-label">{{ label }}</label>
    <input
      type="range"
      :min="min"
      :max="max"
      :step="step"
      v-model.number="internalValue"
      @input="emitValue"
      class="slider"
    />
    <input
      type="number"
      :min="min"
      :max="max"
      :step="step"
      v-model.number="internalValue"
      @input="emitValue"
      class="slider-textbox"
    />
  </div>
</template>

<script setup lang="ts">
import { ref, watch, defineProps, defineEmits } from 'vue'

const props = defineProps({
  modelValue: { type: Number, required: true },
  min: { type: Number, required: true },
  max: { type: Number, required: true },
  step: { type: Number, default: 1 },
  width: { type: String, default: '300px' },
  label: { type: String, default: '' },
})

const emit = defineEmits(['update:modelValue'])

const clamp = (val: number) => Math.min(props.max, Math.max(props.min, val))

const internalValue = ref(clamp(props.modelValue))

watch(
  () => props.modelValue,
  (val) => {
    internalValue.value = clamp(val)
  },
)

function emitValue() {
  internalValue.value = clamp(internalValue.value)
  emit('update:modelValue', internalValue.value)
}
</script>

<style scoped>
.slider-container {
  display: flex;
  align-items: center;
  gap: 0.5em;
}
.slider-label {
  min-width: 70px;
}
.slider {
  flex: 1;
}
.slider-textbox {
  width: 70px;
  padding: 4px 8px;
  border: 1px solid #ccc;
  border-radius: 8px;
  background: #f0f0f0; /* Match slider track background */
  color: #222; /* Match text color */
  font-family: inherit;
  font-size: 1em;
  outline: none;
  transition: border-color 0.2s;
}
.slider-input:focus {
  border-color: #888;
}
</style>
