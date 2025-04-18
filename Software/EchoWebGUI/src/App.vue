<script setup lang="ts">
import { RouterLink, RouterView } from 'vue-router'
</script>

<template>
  <div class="layout">
    <nav class="sidebar">
      <img alt="Vue logo" class="logo" src="@/assets/logo.svg" width="40" height="40" />
      <RouterLink to="/">Dashboard</RouterLink>
      <RouterLink to="/robot1">Robot 1</RouterLink>
      <RouterLink to="/robot2">Robot 2</RouterLink>
      <RouterLink to="/link">Link Control</RouterLink>
      <RouterLink to="/about">About</RouterLink>
    </nav>
    <div class="main-content">
      <RouterView />
    </div>
  </div>
</template>

<style scoped>
.layout {
  display: flex;
  min-height: 100vh;
}

.sidebar {
  position: fixed;
  top: 0;
  left: 0;
  height: 100vh;
  width: 150px;
  background: var(--color-background, #f8f9fa);
  display: flex;
  flex-direction: column;
  align-items: center;
  padding: 2rem 1rem 1rem 1rem;
  box-shadow: 2px 0 8px rgba(0, 0, 0, 0.03);
  z-index: 100;
}

.logo {
  margin-bottom: 2rem;
}

.sidebar a {
  display: block;
  width: 100%;
  padding: 0.75rem 1rem;
  color: var(--color-text, #333);
  text-decoration: none;
  border-radius: 4px;
  margin-bottom: 0.5rem;
  text-align: left;
  transition: background 0.2s;
}

.sidebar a.router-link-exact-active,
.sidebar a:hover {
  background: var(--color-border, #e0e0e0);
  color: var(--color-text, #222);
}

.main-content {
  margin-left: 150px; /* Same as sidebar width */
  padding: 2rem;
  margin-right: auto;
  margin-left: 150px; /* Keep space for sidebar */
  box-sizing: border-box;
}

header {
  margin-bottom: 2rem;
}

@media (max-width: 800px) {
  .layout {
    flex-direction: column;
  }
  .sidebar {
    position: static;
    width: 100%;
    height: auto;
    flex-direction: row;
    box-shadow: none;
    padding: 1rem;
    justify-content: flex-start;
    overflow-x: auto;
  }
  .sidebar a {
    margin: 0 0.5rem 0 0;
    width: auto;
    padding: 0.5rem 1rem;
    text-align: center;
  }
  .main-content {
    margin-left: 0;
    padding: 1rem;
  }
}
</style>
