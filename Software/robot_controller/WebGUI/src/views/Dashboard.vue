<script setup>
import { useRobot1Store, useRobot2Store, useSettingsStore } from '@/stores/robotStore';
import { projectToWorkspace } from '@/utils/kinematics';
import { useToast } from 'primevue/usetoast';
import { ref, watch } from 'vue';
const toast = useToast();
function showWarn() {
    toast.add({ severity: 'warn', summary: 'Target out-of-bounds', detail: 'Target position is out of bounds, snapping to reachable workspace.', life: 1000 });
}

const robot1Store = useRobot1Store();
const robot2Store = useRobot2Store();
const settingsStore = useSettingsStore();

const r1x = ref(robot1Store.eePos.x);
const r1y = ref(robot1Store.eePos.y);
const r2x = ref(robot2Store.eePos.x);
const r2y = ref(robot2Store.eePos.y);

// Watch for changes in the robot positions and update the store, passing through kinematics/projectToWorkspace
watch([r1x, r1y], ([x, y]) => {
    const newPos = { x: r1x, y: r1y };
    const projected = projectToWorkspace(newPos);
    if (newPos === projected) {
        robot1Store.eePos = newPos;
    } else {
        console.warn('Invalid target');
        robot1Store.eePos = projected;
        r1x.value = robot1Store.eePos.x;
        r1y.value = robot1Store.eePos.y;
        showWarn();
    }
});
</script>

<template>
    <div class="grid grid-cols-2 gap-8">
        <div class="card content-center flex flex-col gap-4">
            <h1>Robot 1</h1>
            <RobotVis robot="robot1" />
            <div class="font-semibold text-xl"><em>x</em> Position</div>
            <InputText v-model.number="r1x" />
            <Slider v-model="r1x" :min="-300" :max="300" />
            <div class="font-semibold text-xl"><em>y</em> Position</div>
            <InputText v-model.number="r1y" />
            <Slider v-model="r1y" :min="0" :max="300" />
        </div>
        <div class="card content-center flex flex-col gap-4">
            <h1>Robot 2</h1>
            <RobotVis robot="robot2" />
            <div class="font-semibold text-xl"><em>x</em> Position</div>
            <InputText v-model.number="r2x" />
            <Slider v-model="r2x" :min="-300" :max="300" />
            <div class="font-semibold text-xl"><em>y</em> Position</div>
            <InputText v-model.number="r2y" />
            <Slider v-model="r2y" :min="0" :max="300" />
        </div>
        <div class="card">
            <h1>Charts</h1>
        </div>
    </div>
</template>
