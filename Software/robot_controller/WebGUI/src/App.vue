<script setup lang="ts">
import { RouterLink, RouterView } from 'vue-router'
import { onMounted } from 'vue';

import LorenzAttractor from './components/LorenzAttractor.vue'

</script>

<template>
  <div class="layout">
    <LorenzAttractor />
    <nav class="sidebar">
      <h1 style="text-align: center;"><em>Echo Control</em></h1>
      <img alt="Vue logo" class="logo" src="@/assets/logo.png" width="60" height="60" />
      <RouterLink to="/">Dashboard</RouterLink>
      <RouterLink to="/robot1">Robot 1</RouterLink>
      <RouterLink to="/robot2">Robot 2</RouterLink>
      <RouterLink to="/draw">Drawing</RouterLink>
      <RouterLink to="/about">About</RouterLink>
    </nav>
    <div class="main-content">
      <RouterView />
    </div>
    <footer class="footer">
      <p>
        <strong>Echo Telepresence</strong>
        <br />
        <em>UC Berkeley MECENG 135/235 Final Project</em>
      </p>
      <a href="https://github.com/chpaxson/Echo-Telepresence" target="_blank">
        <img src="@/assets/github.png" alt="GitHub" width="30" height="30" />
      </a>
    </footer>
  </div>
</template>

<style scoped>
  .layout {
    width: 100vw;
    height: 100%;
    display: flex;
  }

  .layout > * {
    position: relative;
    z-index: 1;
  }

  .sidebar {
    width: 175px;
    height: 100%;
    padding: 10px;
    top: 0;
    left: 0;
    display: flex;
    flex-direction: column;
    align-items: center;
    background: var(--color-background-soft);
    border-right: 2px solid var(--color-border);
    user-select: none; /* Prevent text selection */
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
    align-items: center;
    transition: background 0.5s, color 0.5s;
  }
  .sidebar a.router-link-exact-active,
  .sidebar a:hover {
    background: var(--color-border);
    color: var(--color-text);
    /* font-style: italic; */
  }
  .logo {
    margin-top: 1em;
    margin-bottom: 1em;
  }
  .main-content {
    width: 100%;
    /* margin-left: 1em; */
    /* margin-right: 1em; */
    padding: 1em;
    background: rgba(0, 0, 0, 0.3);
  }
  
  .footer {
    position: fixed;
    bottom: 0;
    left: 0;
    display: flex;
    justify-content: center;
    gap: 3em;
    width: 100%;
  }
</style>