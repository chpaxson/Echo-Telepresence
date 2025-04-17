<template>
  <div class="robot-settings">
    <h2>Robot Settings</h2>
    <div class="setting">
      <label for="position">Position:</label>
      <input type="text" id="position" v-model="settings.position" />
    </div>
    <div class="setting">
      <label for="max-velocity">Max Velocity:</label>
      <input type="number" id="max-velocity" v-model="settings.maxVelocity" />
    </div>
    <div class="setting">
      <label for="max-acceleration">Max Acceleration:</label>
      <input type="number" id="max-acceleration" v-model="settings.maxAcceleration" />
    </div>
    <div class="setting">
      <label for="spring-constant">Effective Spring Constant:</label>
      <input type="number" id="spring-constant" v-model="settings.springConstant" />
    </div>
    <div class="setting">
      <label for="max-torque">Max Torque:</label>
      <input type="number" id="max-torque" v-model="settings.maxTorque" />
    </div>
    <button @click="saveSettings">Save Settings</button>
  </div>
</template>

<script setup lang="ts">
import { ref } from 'vue'

const settings = ref({
  position: '',
  maxVelocity: 0,
  maxAcceleration: 0,
  springConstant: 0,
  maxTorque: 0,
})

function saveSettings() {
  // Logic to save settings, e.g., send to a server or update a store
  console.log('Settings saved:', settings.value)
}
</script>

<style scoped>
.robot-settings {
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 1rem;
}
.setting {
  display: flex;
  flex-direction: column;
}
label {
  margin-bottom: 0.5rem;
}
input {
  padding: 0.5rem;
  width: 100%;
  max-width: 300px;
}
button {
  padding: 0.5rem 1rem;
  cursor: pointer;
}
</style>