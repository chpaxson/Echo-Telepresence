<script setup>
import { useRobotStore } from '@/stores/robotStore';
import { projectToWorkspace } from '@/utils/kinematics';
import { useToast } from 'primevue/usetoast';
import { computed, ref } from 'vue';
const toast = useToast();
function showWarn() {
    toast.add({ severity: 'warn', summary: 'Target out-of-bounds', detail: 'Target position is out of bounds, snapping to reachable workspace.', life: 1000 });
}

const robotStore = useRobotStore();

const primaryRobotDropdownValues = ref([
    { name: 'Robot 1', code: 'r1' },
    { name: 'Robot 2', code: 'r2' }
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
    toast.add({ severity: 'success', summary: 'Homed Robot 1!', life: 3000 });
};
const r2Home = () => {
    robotStore.r2.j1Homed = true;
    robotStore.r2.j2Homed = true;
    toast.add({ severity: 'success', summary: 'Homed Robot 2!', life: 3000 });
};

const r1Homed = computed(() => robotStore.r1.j1Homed && robotStore.r1.j2Homed);
const r2Homed = computed(() => robotStore.r2.j1Homed && robotStore.r2.j2Homed);

const r1_c = () => {
    robotStore.r1.ee = projectToWorkspace(robotStore.r1.ee);
};
const r2_c = () => {
    robotStore.r2.ee = projectToWorkspace(robotStore.r2.ee);
};
</script>

<template>
    <div class="grid grid-cols-2 gap-8">
        <div class="card content-center flex flex-col gap-2">
            <div class="flow-root">
                <h1 class="float-left">Robot 1</h1>
                <SplitButton class="homeButton float-right" :label="r1Homed ? `Homed` : `Home`" icon="pi pi-home" :model="r1HomeButtonOptions" :severity="r1Homed ? `` : `danger`" @click="r1Home"></SplitButton>
            </div>
            <RobotVis r="r1" />
            <div class="flex flex-row gap-8 items-center">
                <div class="font-semibold text-xl">
                    <em>x</em>
                </div>
                <Slider class="w-full" v-model="robotStore.r1.ee.x" @change="r1_c" :min="-300" :max="300" :step="0.1" />
                <InputText class="w-24" v-model.number="robotStore.r1.ee.x" />
            </div>
            <div class="flex flex-row gap-8 items-center">
                <div class="font-semibold text-xl">
                    <em>y</em>
                </div>
                <Slider class="w-full" v-model="robotStore.r1.ee.y" @change="r1_c" :min="0" :max="350" :step="0.1" />
                <InputText class="w-24" v-model.number="robotStore.r1.ee.y" />
            </div>
        </div>
        <div class="card content-center flex flex-col gap-2">
            <div class="flow-root">
                <h1 class="float-left">Robot 2</h1>
                <SplitButton class="homeButton float-right" :label="r2Homed ? `Homed` : `Home`" icon="pi pi-home" :model="r2HomeButtonOptions" :severity="r2Homed ? `` : `danger`" @click="r2Home"></SplitButton>
            </div>
            <RobotVis r="r2" />
            <div class="flex flex-row gap-8 items-center">
                <div class="font-semibold text-xl">
                    <em>x</em>
                </div>
                <Slider class="w-full" v-model="robotStore.r2.ee.x" @change="r2_c" :min="-300" :max="300" :step="0.1" />
                <InputText class="w-24" v-model.number="robotStore.r2.ee.x" />
            </div>
            <div class="flex flex-row gap-8 items-center">
                <div class="font-semibold text-xl">
                    <em>y</em>
                </div>
                <Slider class="w-full" v-model="robotStore.r2.ee.y" @change="r2_c" :min="0" :max="350" :step="0.1" />
                <InputText class="w-24" v-model.number="robotStore.r2.ee.y" />
            </div>
        </div>
        <div class="card flex flex-col gap-4">
            <h1>Robot Linking</h1>
            <div class="flex flex-row gap-4">
                <ToggleSwitch v-model="robotStore.enableLink" />
                <div class="font-semibold text-xl">Linked?</div>
            </div>
            <div class="flex flex-row w-full items-center justify-between">
                <div class="font-semibold text-xl">Primary Robot</div>
                <Select v-model="robotStore.primaryRobot" :options="primaryRobotDropdownValues" optionLabel="name" placeholder="Select" />
            </div>
            <div class="flex flex-row gap-8 items-center">
                <div class="font-semibold text-xl">Ratio</div>
                <Slider class="w-full" v-model="robotStore.linkRatio" :min="0.05" :max="2" :step="0.05" />
                <InputText class="w-24" v-model.number="robotStore.linkRatio" />
            </div>

            <div class="flex flex-row w-full items-center justify-between">
                <div class="font-semibold text-xl">Boundary Mode</div>
                <Select v-model="robotStore.boundaryMode" :options="boundaryModeDropdownValues" optionLabel="name" placeholder="Select" />
            </div>
        </div>
    </div>
</template>
