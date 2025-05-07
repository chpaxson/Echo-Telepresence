<script setup>
import { ref, watch } from 'vue';
const currentTab = ref(0);
const generatorType = ref(null);

// Log currentTab whenever it changes
watch(currentTab, (newValue) => {
    console.log('Current Tab:', newValue);
});
</script>

<template>
    <div class="grid grid-cols-2 gap-8">
        <div class="card">
            <h1>Path Generator Settings</h1>
            <Tabs v-model:value="currentTab">
                <TabList>
                    <Tab value="0">Spirograph</Tab>
                    <Tab value="1">Hatching</Tab>
                    <Tab value="2">Torus</Tab>
                    <Tab value="3">Helix</Tab>
                    <Tab value="4">Chaos</Tab>
                </TabList>
                <TabPanels>
                    <TabPanel value="0">
                        <h2>Spirograph</h2>
                        <p>Generate a spirograph pattern.</p>
                    </TabPanel>
                    <TabPanel value="1">
                        <h2>Hatching</h2>
                        <p>Generate a hatching pattern.</p>
                    </TabPanel>
                    <TabPanel value="2">
                        <h2>Torus</h2>
                        <p>Generate a toroidal path.</p>
                    </TabPanel>
                    <TabPanel value="3">
                        <h2>Helix</h2>
                        <p>Generate a helical path.</p>
                    </TabPanel>
                    <TabPanel value="4">
                        <h2>Chaotic Attractors</h2>
                        <p>Generate a Lorenz attractor path.</p>
                    </TabPanel>
                </TabPanels>
            </Tabs>
        </div>
        <div class="card">
            <h1>GCode Preview</h1>
        </div>
        <div class="card">
            <h1>Real Preview</h1>
        </div>
    </div>
</template>
