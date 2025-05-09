<script setup>
import { useRobotStore } from '@/stores/robotStore';
import { projectToWorkspace, centroid } from '@/utils/kinematics';
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
    toast.add({ severity: 'primary', summary: 'Homed Robot 1!', life: 3000 });
};
const r2Home = () => {
    robotStore.r2.j1Homed = true;
    robotStore.r2.j2Homed = true;
    toast.add({ severity: 'primary', summary: 'Homed Robot 2!', life: 3000 });
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
    // Get mode from store
    r1DriveMode.value = robotStore.r1.driver;
    r2DriveMode.value = robotStore.r2.driver;
});

const driveModeOptions = ['GUI', 'Robot Data', 'Path Planner', 'Linked'];

const r1DriveModeChange = () => {
    robotStore.r1.driver = r1DriveMode.value;
};
const r2DriveModeChange = () => {
    robotStore.r2.driver = r2DriveMode.value;
};

// Create virtual link between robot positions if robotStore.enableLink is true
const enableLink = ref(false);
const primaryRobot = ref('Robot 1');
const scaleMode = ref('Relative');
let prevDriveMode;
const el = () => {
    enableLink.value = !enableLink.value;
    robotStore.enableLink = enableLink.value;
    if (enableLink.value) {
        toast.add({ severity: 'primary', summary: 'Linked Robots!', life: 3000 });
        if (primaryRobot.value === 'Robot 1') {
            prevDriveMode = robotStore.r2.driver;
            robotStore.r2.driver = 'Linked';
            r2DriveMode.value = 'Linked';
        } else {
            prevDriveMode = robotStore.r1.driver;
            robotStore.r1.driver = 'Linked';
            r1DriveMode.value = 'Linked';
        }
    } else {
        toast.add({ severity: 'warn', summary: 'Unlinked Robots!', life: 3000 });
        if (primaryRobot.value === 'Robot 1') {
            robotStore.r2.driver = prevDriveMode;
            r2DriveMode.value = prevDriveMode;
        } else {
            robotStore.r1.driver = prevDriveMode;
            r1DriveMode.value = prevDriveMode;
        }
    }
};

// Watch for changes in the primary robot and find the dx/dy of it's movement, then apply a scaled version of that to the other robot
const primaryRobotRef = computed(() => (primaryRobot.value === 'Robot 1' ? robotStore.r1 : robotStore.r2));
const secondaryRobotRef = computed(() => (primaryRobot.value === 'Robot 1' ? robotStore.r2 : robotStore.r1));
const primaryRobotEE = computed(() => primaryRobotRef.value.ee);
const secondaryRobotEE = computed(() => secondaryRobotRef.value.ee);
const primaryRobotEEWatch = watch(
    () => primaryRobotEE.value,
    (newVal, oldVal) => {
        if (enableLink.value) {
            if (scaleMode.value === 'Relative') {
                // Relative mode - magnitude of displacement is scaled by linkRatio
                const x = secondaryRobotRef.value.ee.x + (newVal.x - oldVal.x) * robotStore.linkRatio;
                const y = secondaryRobotRef.value.ee.y + (newVal.y - oldVal.y) * robotStore.linkRatio;
                secondaryRobotRef.value.ee = projectToWorkspace({ x: x, y: y });
            } else {
                // Absolute mode - displacement about the centroid
                const x = centroid.x + (newVal.x - centroid.x) * robotStore.linkRatio;
                const y = centroid.y + (newVal.y - centroid.y) * robotStore.linkRatio;
                secondaryRobotRef.value.ee = projectToWorkspace({ x: x, y: y });
            }
        }
    },
    { deep: true }
);

watch(primaryRobot, (newVal, oldVal) => {
    if (enableLink.value && newVal !== oldVal) {
        enableLink.value = false;
        robotStore.enableLink = false;
        toast.add({
            severity: 'warn',
            summary: 'Link Disabled',
            detail: 'Switching the primary robot disables the link.',
            life: 3000
        });
        // Restore previous drive mode
        if (oldVal === 'Robot 1') {
            robotStore.r2.driver = prevDriveMode;
            r2DriveMode.value = prevDriveMode;
        } else {
            robotStore.r1.driver = prevDriveMode;
            r1DriveMode.value = prevDriveMode;
        }
    }
});
</script>

<template>
    <div class="grid grid-cols-2 gap-8">
        <div class="card content-center flex flex-col gap-4">
            <div class="flex flex-row items-center justify-between">
                <h1 class="select-none m-px">Robot 1</h1>
                <div class="flex flex-row gap-4 h-10">
                    <Select v-model="r1DriveMode" @change="r1DriveModeChange" :options="driveModeOptions" />
                    <SplitButton class="homeButton" icon="pi pi-home" :label="r1Homed ? `Homed` : `Home`" :model="r1HomeButtonOptions" :severity="r1Homed ? `` : `danger`" @click="r1Home"></SplitButton>
                </div>
            </div>
            <RobotVis r="r1" />
            <div class="flex flex-row gap-8 items-center">
                <div class="font-semibold text-xl">
                    <em>x</em>
                </div>
                <Slider class="w-full" v-model="robotStore.r1.ee.x" @change="r1_c" :min="-300" :max="300" :step="0.1" :disabled="robotStore.r1.driver !== 'GUI'" />
                <InputText class="w-24" v-model.number="robotStore.r1.ee.x" :disabled="robotStore.r1.driver !== 'GUI'" />
            </div>
            <div class="flex flex-row gap-8 items-center">
                <div class="font-semibold text-xl">
                    <em>y</em>
                </div>
                <Slider class="w-full" v-model="robotStore.r1.ee.y" @change="r1_c" :min="0" :max="350" :step="0.1" :disabled="robotStore.r1.driver !== 'GUI'" />
                <InputText class="w-24" v-model.number="robotStore.r1.ee.y" :disabled="robotStore.r1.driver !== 'GUI'" />
            </div>
        </div>
        <div class="card content-center flex flex-col gap-4">
            <div class="flex flex-row items-center justify-between">
                <h1 class="select-none m-px">Robot 2</h1>
                <div class="flex flex-row gap-4 h-10">
                    <Select v-model="r2DriveMode" @change="r2DriveModeChange" :options="driveModeOptions" />
                    <SplitButton class="homeButton" icon="pi pi-home" :label="r2Homed ? `Homed` : `Home`" :model="r2HomeButtonOptions" :severity="r2Homed ? `` : `danger`" @click="r2Home"></SplitButton>
                </div>
            </div>
            <RobotVis r="r2" />
            <div class="flex flex-row gap-8 items-center">
                <div class="font-semibold text-xl">
                    <em>x</em>
                </div>
                <Slider class="w-full" v-model="robotStore.r2.ee.x" :min="-300" :max="300" :step="0.1" :disabled="robotStore.r2.driver !== 'GUI'" />
                <InputText class="w-24" v-model.number="robotStore.r2.ee.x" :disabled="robotStore.r2.driver !== 'GUI'" />
            </div>
            <div class="flex flex-row gap-8 items-center">
                <div class="font-semibold text-xl">
                    <em>y</em>
                </div>
                <Slider class="w-full" v-model="robotStore.r2.ee.y" :min="0" :max="350" :step="0.1" :disabled="robotStore.r2.driver !== 'GUI'" />
                <InputText class="w-24" v-model.number="robotStore.r2.ee.y" :disabled="robotStore.r2.driver !== 'GUI'" />
            </div>
        </div>
        <div class="card flex flex-col gap-4">
            <h1>Robot Linking</h1>
            <div class="flex flex-row gap-4">
                <Button :label="enableLink ? `Disable Link` : `Enable Link`" :severity="enableLink ? `danger` : `primary`" @click="el"></Button>
            </div>
            <div class="flex flex-row w-full items-center justify-between">
                <div class="font-semibold text-xl">Primary Robot</div>
                <SelectButton v-model="primaryRobot" :options="[`Robot 1`, `Robot 2`]" />
            </div>
            <div class="flex flex-row gap-8 items-center">
                <div class="font-semibold text-xl">Ratio</div>
                <Slider class="w-full" v-model="robotStore.linkRatio" :min="0.05" :max="2" :step="0.05" />
                <InputText class="w-24" v-model.number="robotStore.linkRatio" />
            </div>

            <div class="flex flex-row w-full items-center justify-between">
                <div class="font-semibold text-xl">Scale Mode</div>
                <SelectButton v-model="scaleMode" :options="[`Absolute`, `Relative`]" />
            </div>
        </div>
    </div>
</template>
