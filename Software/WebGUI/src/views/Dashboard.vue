<script setup>
import { useRobotStore } from '@/stores/robotStore';
import { projectToWorkspace } from '@/utils/kinematics';
import { useToast } from 'primevue/usetoast';
import { computed, onMounted, ref, watch } from 'vue';
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

const r1DriveMode = ref();
const r2DriveMode = ref();
onMounted(() => {
    // Get mode from store, adding the label and value
    switch (robotStore.r1.driver) {
        case 'gui':
            r1DriveMode.value = { label: 'GUI', value: 'gui' };
            break;
        case 'robot':
            r1DriveMode.value = { label: 'Robot Data', value: 'robot' };
            break;
        case 'path':
            r1DriveMode.value = { label: 'Path Planner', value: 'path' };
            break;
    }
    switch (robotStore.r2.driver) {
        case 'gui':
            r2DriveMode.value = { label: 'GUI', value: 'gui' };
            break;
        case 'robot':
            r2DriveMode.value = { label: 'Robot Data', value: 'robot' };
            break;
        case 'path':
            r2DriveMode.value = { label: 'Path Planner', value: 'path' };
            break;
    }
});

const driveModeOptions = [
    { label: 'GUI', value: 'gui' },
    { label: 'Robot Data', value: 'robot' },
    { label: 'Path Planner', value: 'path' }
];

const r1DriveModeChange = () => {
    robotStore.r1.driver = r1DriveMode.value.value;
};
const r2DriveModeChange = () => {
    robotStore.r2.driver = r2DriveMode.value.value;
};

// Create virtual link between robot positions if robotStore.enableLink is true
const enableLink = ref(false);
const linkRatio = ref(1);
const primaryRobot = ref(robotStore.primaryRobot);
const scaleMode = ref('Absolute');
const el = () => {
    console.log('Linking robots', el.value);
    robotStore.enableLink = !robotStore.enableLink;
};
const lr = () => (robotStore.linkRatio = linkRatio.value);
watch(
    () => robotStore.r1.ee,
    (newPos) => {
        console.log(robotStore.enableLink);
        if (robotStore.enableLink && robotStore.primaryRobot.code === 'r1') {
            robotStore.r2.ee.x = newPos.x * robotStore.linkRatio;
            robotStore.r2.ee.y = newPos.y * robotStore.linkRatio;
        }
    }
);
watch(
    () => robotStore.r2.ee,
    (newPos) => {
        if (robotStore.enableLink && robotStore.primaryRobot.code === 'r2') {
            robotStore.r1.ee.x = newPos.x / robotStore.linkRatio;
            robotStore.r1.ee.y = newPos.y / robotStore.linkRatio;
        }
    }
);
</script>

<template>
    <div class="grid grid-cols-2 gap-8">
        <div class="card content-center flex flex-col gap-4">
            <div class="flex flex-row items-center justify-between">
                <h1 class="select-none m-px">Robot 1</h1>
                <div class="flex flex-row gap-4 h-10">
                    <Select v-model="r1DriveMode" @change="r1DriveModeChange" :options="driveModeOptions" optionLabel="label" placeholder="Drive Source" />
                    <SplitButton class="homeButton" icon="pi pi-home" :label="r1Homed ? `Homed` : `Home`" :model="r1HomeButtonOptions" :severity="r1Homed ? `` : `danger`" @click="r1Home"></SplitButton>
                </div>
            </div>
            <RobotVis r="r1" />
            <div class="flex flex-row gap-8 items-center">
                <div class="font-semibold text-xl">
                    <em>x</em>
                </div>
                <Slider class="w-full" v-model="robotStore.r1.ee.x" @change="r1_c" :min="-300" :max="300" :step="0.1" :disabled="robotStore.r1.driver !== 'gui'" />
                <InputText class="w-24" v-model.number="robotStore.r1.ee.x" :disabled="robotStore.r1.driver !== 'gui'" />
            </div>
            <div class="flex flex-row gap-8 items-center">
                <div class="font-semibold text-xl">
                    <em>y</em>
                </div>
                <Slider class="w-full" v-model="robotStore.r1.ee.y" @change="r1_c" :min="0" :max="350" :step="0.1" :disabled="robotStore.r1.driver !== 'gui'" />
                <InputText class="w-24" v-model.number="robotStore.r1.ee.y" :disabled="robotStore.r1.driver !== 'gui'" />
            </div>
        </div>
        <div class="card content-center flex flex-col gap-4">
            <div class="flex flex-row items-center justify-between">
                <h1 class="select-none m-px">Robot 2</h1>
                <div class="flex flex-row gap-4 h-10">
                    <Select v-model="r2DriveMode" @change="r2DriveModeChange" :options="driveModeOptions" optionLabel="label" placeholder="Drive Source" />
                    <SplitButton class="homeButton" icon="pi pi-home" :label="r2Homed ? `Homed` : `Home`" :model="r2HomeButtonOptions" :severity="r2Homed ? `` : `danger`" @click="r2Home"></SplitButton>
                </div>
            </div>
            <RobotVis r="r2" />
            <div class="flex flex-row gap-8 items-center">
                <div class="font-semibold text-xl">
                    <em>x</em>
                </div>
                <Slider class="w-full" v-model="robotStore.r2.ee.x" @change="r2_c" :min="-300" :max="300" :step="0.1" :disabled="robotStore.r2.driver !== 'gui'" />
                <InputText class="w-24" v-model.number="robotStore.r2.ee.x" :disabled="robotStore.r2.driver !== 'gui'" />
            </div>
            <div class="flex flex-row gap-8 items-center">
                <div class="font-semibold text-xl">
                    <em>y</em>
                </div>
                <Slider class="w-full" v-model="robotStore.r2.ee.y" @change="r2_c" :min="0" :max="350" :step="0.1" :disabled="robotStore.r2.driver !== 'gui'" />
                <InputText class="w-24" v-model.number="robotStore.r2.ee.y" :disabled="robotStore.r2.driver !== 'gui'" />
            </div>
        </div>
        <div class="card flex flex-col gap-4">
            <h1>Robot Linking</h1>
            <div class="flex flex-row gap-4">
                <ToggleSwitch @change="el" />
                <div class="font-semibold text-xl">Linked?</div>
            </div>
            <div class="flex flex-row w-full items-center justify-between">
                <div class="font-semibold text-xl">Primary Robot</div>
                <Select v-model="primaryRobot" :options="['Robot 1', 'Robot 2']" placeholder="Select" />
            </div>
            <div class="flex flex-row gap-8 items-center">
                <div class="font-semibold text-xl">Ratio</div>
                <Slider class="w-full" v-model="linkRatio" @change="lr" :min="0.05" :max="2" :step="0.05" />
                <InputText class="w-24" v-model.number="linkRatio" @change="lr" />
            </div>

            <div class="flex flex-row w-full items-center justify-between">
                <div class="font-semibold text-xl">Scale Mode</div>
                <Select v-model="scaleMode" :options="['Absolute', 'Relative']" placeholder="Select" />
            </div>
        </div>
    </div>
</template>
