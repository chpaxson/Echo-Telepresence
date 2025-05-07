<script setup>
import { useRobotStore } from '@/stores/robotStore';
import { useToast } from 'primevue/usetoast';
import { computed, ref } from 'vue';
const toast = useToast();
function showWarn() {
    toast.add({ severity: 'warn', summary: 'Target out-of-bounds', detail: 'Target position is out of bounds, snapping to reachable workspace.', life: 1000 });
}

const robotStore = useRobotStore();

const r1x = ref(robotStore.r1.eePos.x);
const r1y = ref(robotStore.r1.eePos.y);
const r2x = ref(robotStore.r2.eePos.x);
const r2y = ref(robotStore.r2.eePos.y);

const primaryRobotDropdownValues = ref([
    { name: 'Robot 1', code: 'robot1' },
    { name: 'Robot 2', code: 'robot2' }
]);
const boundaryModeDropdownValues = ref([
    { name: 'Boundaries cause full stop', code: 'stop' },
    { name: 'Boundaries "push" delta', code: 'nudge' },
    { name: 'Slide along boundaries', code: 'slide' }
]);
const r1HomeButtonOptions = ref([
    {
        label: 'Home Joint 1',
        command: () => {
            robotStore.r1.j1Homed = true;
        }
    },
    {
        label: 'Home Joint 2',
        command: () => {
            robotStore.r1.j2Homed = true;
        }
    },
    {
        label: 'Disable Motors',
        command: () => {
            robotStore.r1.j1Homed = false;
            robotStore.r1.j2Homed = false;
        }
    }
]);
const r2HomeButtonOptions = ref([
    {
        label: 'Home Joint 1',
        command: () => {
            robotStore.r2.j1Homed = true;
        }
    },
    {
        label: 'Home Joint 2',
        command: () => {
            robotStore.r2.j2Homed = true;
        }
    },
    {
        label: 'Disable Motors',
        command: () => {
            robotStore.r2.j1Homed = false;
            robotStore.r2.j2Homed = false;
        }
    }
]);
const r1Home = () => {
    robotStore.r1.j1Homed = true;
    robotStore.r1.j2Homed = true;
    toast.add({ severity: 'success', summary: 'Homed Robot 1!' });
};
const r2Home = () => {
    robotStore.r2.j1Homed = true;
    robotStore.r2.j2Homed = true;
    toast.add({ severity: 'success', summary: 'Homed Robot 2!' });
};

const r1Homed = computed(() => robotStore.r1.j1Homed && robotStore.r1.j2Homed);
const r2Homed = computed(() => robotStore.r2.j1Homed && robotStore.r2.j2Homed);
</script>

<template>
    <div class="grid grid-cols-2 gap-8">
        <div class="card content-center flex flex-col gap-4">
            <div class="flow-root">
                <h1 class="float-left">Robot 1</h1>
                <SplitButton class="homeButton float-right" label="Home" :model="r1HomeButtonOptions" :severity="r1Homed ? `` : `danger`" @click="r1Home"></SplitButton>
            </div>
            <RobotVis robot="robot1" />
            <div class="font-semibold text-xl"><em>x</em> Position</div>
            <InputText v-model.number="r1x" />
            <Slider v-model="r1x" :min="-300" :max="300" />
            <div class="font-semibold text-xl"><em>y</em> Position</div>
            <InputText v-model.number="r1y" />
            <Slider v-model="r1y" :min="0" :max="300" />
        </div>
        <div class="card content-center flex flex-col gap-4">
            <div class="flow-root">
                <h1 class="float-left">Robot 2</h1>
                <SplitButton class="homeButton float-right" label="Home" :model="r2HomeButtonOptions" :severity="r2Homed ? `` : `danger`" @click="r2Home"></SplitButton>
            </div>
            <RobotVis robot="robot2" />
            <div class="font-semibold text-xl"><em>x</em> Position</div>
            <InputText v-model.number="r2x" />
            <Slider v-model="r2x" :min="-300" :max="300" />
            <div class="font-semibold text-xl"><em>y</em> Position</div>
            <InputText v-model.number="r2y" />
            <Slider v-model="r2y" :min="0" :max="300" />
        </div>
        <div class="card flex flex-col gap-4">
            <h1>Robot Linking</h1>
            <div class="flex flex-row gap-4">
                <ToggleSwitch v-model="robotStore.enableLink" />
                <div class="font-semibold text-xl">Linked?</div>
            </div>
            <div class="font-semibold text-xl">Primary Robot</div>
            <Select v-model="robotStore.primaryRobot" :options="primaryRobotDropdownValues" optionLabel="name" placeholder="Select" />
            <div class="font-semibold text-xl">Ratio</div>
            <InputText v-model.number="robotStore.linkRatio" />
            <Slider v-model="robotStore.linkRatio" :min="0.05" :max="2" :step="0.05" />

            <div class="font-semibold text-xl">Boundary Mode</div>
            <Select v-model="robotStore.boundaryMode" :options="boundaryModeDropdownValues" optionLabel="name" placeholder="Select" />
        </div>
    </div>
</template>
