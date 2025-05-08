<script setup>
import { useLayout } from '@/layout/composables/layout';
import { useRobotStore } from '@/stores/robotStore';
import { $dt } from '@primeuix/themes';
import { useConfirm } from 'primevue/useconfirm';
import { useToast } from 'primevue/usetoast';
import { computed, onMounted, ref, watch } from 'vue';
import { calc_fk, calc_ik, projectToWorkspace } from '../utils/kinematics';

const props = defineProps({
    r: { type: String, required: true } // 'r1' or 'r2'
});

const toast = useToast();
const confirmPopup = useConfirm();

const robotStore = useRobotStore();

const a1 = ref(0);
const a2 = ref(0);

const postHomeToRobot = (r1m1, r1m2, r2m1, r2m2) => {
    fetch(`/api/v1/homing`, {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({
            r1: {
                m1: r1m1,
                m2: r1m2
            },
            r2: {
                m1: r2m1,
                m2: r2m2
            }
        })
    })
        .then((response) => {
            if (!response.ok) {
                throw new Error(`HTTP error! status: ${response.status}`);
            }
            return response.json();
        })
        .then((data) => {
            console.log('Success:', data);
        })
        .catch((error) => {
            console.error('Error:', error);
        });
};

onMounted(() => {
    const initialIK = calc_ik(robotStore[props.r].ee);
    a1.value = (initialIK.a1 * 180) / Math.PI;
    a2.value = (initialIK.a2 * 180) / Math.PI;
});

const homeButtonOptions = ref([
    {
        label: 'Home Joint 1',
        command: () => {
            robotStore[props.r].m1.homed = true;
            postHomeToRobot(props.r === 'r1' ? 1 : 0, 0, props.r === 'r2' ? 1 : 0, 0);
        }
    },
    {
        label: 'Home Joint 2',
        command: () => {
            robotStore[props.r].m2.homed = true;
            postHomeToRobot(0, props.r === 'r1' ? 1 : 0, 0, props.r === 'r2' ? 1 : 0);
        }
    },
    {
        label: 'Disable Motors',
        command: () => {
            robotStore[props.r].m1.homed = false;
            robotStore[props.r].m2.homed = false;
            if (props.r === 'r1') {
                postHomeToRobot(2, 2, 0, 0);
            } else {
                postHomeToRobot(0, 0, 2, 2);
            }
        }
    }
]);
const home = () => {
    robotStore[props.r].m1.homed = true;
    robotStore[props.r].m2.homed = true;
    if (props.r === 'r1') {
        postHomeToRobot(1, 1, 0, 0);
    } else {
        postHomeToRobot(0, 0, 1, 1);
    }
    toast.add({ severity: 'success', summary: `Homed Robot!`, life: 3000 });
};

const homed = computed(() => robotStore[props.r].m1.homed && robotStore[props.r].m2.homed);

const rc = () => {
    robotStore[props.r].ee = projectToWorkspace(robotStore[props.r].ee);
    const ik = calc_ik(robotStore[props.r].ee);
    a1.value = (ik.a1 * 180) / Math.PI;
    a2.value = (ik.a2 * 180) / Math.PI;
};
const rac = () => {
    robotStore[props.r].ee = projectToWorkspace(
        calc_fk({
            a1: (a1.value * Math.PI) / 180,
            a2: (a2.value * Math.PI) / 180
        })
    );
    const ik = calc_ik(robotStore[props.r].ee);
    a1.value = (ik.a1 * 180) / Math.PI;
    a2.value = (ik.a2 * 180) / Math.PI;
};

// --- Joint angle chart logic ---

const maxT = 5;
const deltat = 0.1;

// Use plain arrays for raw data storage
const rawA1 = [];
const rawA2 = [];
const rawLabels = [];

const dataA1 = ref([]); // For Chart binding
const dataA2 = ref([]); // For Chart binding
const labels = ref([]); // For Chart binding

const lineDataA1 = ref(null);
const lineDataA2 = ref(null);
const lineOptionsA1 = ref(null);
const lineOptionsA2 = ref(null);
let initialTime = 0;

const updateJointData = () => {
    if (initialTime === 0) {
        initialTime = performance.now() / 1000;
    }
    const t = performance.now() / 1000 - initialTime - maxT;

    // Calculate joint angles from current end effector position
    const ik = calc_ik(robotStore[props.r].ee);
    const angle1 = (ik.a1 * 180) / Math.PI;
    const angle2 = (ik.a2 * 180) / Math.PI;

    if (rawA1.length > maxT / deltat) {
        rawA1.shift();
        rawA2.shift();
    } else {
        rawLabels.push(t);
    }
    rawA1.push(angle1);
    rawA2.push(angle2);

    dataA1.value = rawA1.slice();
    dataA2.value = rawA2.slice();
    labels.value = rawLabels.slice();

    lineDataA1.value = {
        labels: labels.value,
        datasets: [
            {
                label: 'Target',
                data: dataA1.value,
                fill: false,
                borderColor: $dt('primary.500').value,
                tension: 0.4,
                pointRadius: 0
            }
        ]
    };
    lineDataA2.value = {
        labels: labels.value,
        datasets: [
            {
                label: 'Target',
                data: dataA2.value,
                fill: false,
                borderColor: $dt('primary.500').value,
                tension: 0.4,
                pointRadius: 0
            }
        ]
    };
};

const lineInterval = setInterval(() => {
    // console.log('updateJointData', performance.now());
    updateJointData();
}, deltat * 1000);

const { getPrimary, getSurface, isDarkTheme } = useLayout();
onMounted(() => {
    setColorOptions();

    // --- Pre-fill the graph with the current position ---
    initialTime = performance.now() / 1000 - maxT;
    const ik = calc_ik(robotStore[props.r].ee);
    const angle1 = (ik.a1 * 180) / Math.PI;
    const angle2 = (ik.a2 * 180) / Math.PI;

    // Fill arrays so the graph is full on load
    rawA1.length = 0;
    rawA2.length = 0;
    rawLabels.length = 0;
    for (let i = 0; i < maxT / deltat; i++) {
        rawA1.push(angle1);
        rawA2.push(angle2);
        rawLabels.push(i * deltat - maxT);
    }
    dataA1.value = rawA1.slice();
    dataA2.value = rawA2.slice();
    labels.value = rawLabels.slice();
});

function setColorOptions() {
    const documentStyle = getComputedStyle(document.documentElement);
    const textColor = documentStyle.getPropertyValue('--text-color');
    const textColorSecondary = documentStyle.getPropertyValue('--text-color-secondary');
    const surfaceBorder = documentStyle.getPropertyValue('--surface-border');

    lineOptionsA1.value = {
        animation: false,
        plugins: { legend: { labels: { fontColor: textColor } } },
        scales: {
            x: {
                title: {
                    display: true,
                    text: 'Time (s)'
                },
                min: -maxT,
                max: 0,
                type: 'linear',
                ticks: { color: textColorSecondary },
                grid: { color: surfaceBorder, drawBorder: false }
            },
            y: {
                title: {
                    display: true,
                    text: 'Angle (°)'
                },
                min: 0,
                max: 270,
                ticks: { color: textColorSecondary },
                grid: { color: surfaceBorder, drawBorder: false }
            }
        }
    };
    lineOptionsA2.value = {
        animation: false,
        plugins: { legend: { labels: { fontColor: textColor } } },
        scales: {
            x: {
                title: {
                    display: true,
                    text: 'Time (s)'
                },
                min: -maxT,
                max: 0,
                type: 'linear',
                ticks: { color: textColorSecondary },
                grid: { color: surfaceBorder, drawBorder: false }
            },
            y: {
                title: {
                    display: true,
                    text: 'Angle (°)'
                },
                min: -90,
                max: 180,
                ticks: { color: textColorSecondary },
                grid: { color: surfaceBorder, drawBorder: false }
            }
        }
    };
}

watch(
    [getPrimary, getSurface, isDarkTheme],
    () => {
        setColorOptions();
    },
    { immediate: true }
);
</script>

<template>
    <div class="grid grid-cols-2 gap-8">
        <div class="card content-center flex flex-col gap-4">
            <div class="flow-root">
                <h1 class="float-left select-none">Robot {{ props.r === 'r1' ? '1' : '2' }}</h1>
                <SplitButton class="homeButton float-right" icon="pi pi-home" :label="homed ? `Homed` : `Home`" :model="homeButtonOptions" :severity="homed ? `` : `danger`" @click="home"></SplitButton>
            </div>
            <RobotVis :r="props.r" />
            <div class="flex flex-row gap-8 items-center">
                <div class="text-xl">
                    <em>x</em>
                </div>
                <Slider class="w-full" v-model="robotStore[props.r].ee.x" @change="rc" :min="-300" :max="300" :step="0.1" />
                <InputText class="w-24" v-model.number="robotStore[props.r].ee.x" />
            </div>
            <div class="flex flex-row gap-8 items-center">
                <div class="text-xl">
                    <em>y</em>
                </div>
                <Slider class="w-full" v-model="robotStore[props.r].ee.y" @change="rc" :min="0" :max="350" :step="0.1" />
                <InputText class="w-24" v-model.number="robotStore[props.r].ee.y" />
            </div>
            <div class="flex flex-row gap-8 items-center">
                <div class="text-xl">
                    <em>a<sub>1</sub></em>
                </div>
                <Slider class="w-full" v-model="a1" @change="rac" :min="0" :max="270" :step="0.1" />
                <InputText class="w-24" v-model.number="a1" />
            </div>
            <div class="flex flex-row gap-8 items-center">
                <div class="text-xl">
                    <em>a<sub>2</sub></em>
                </div>
                <Slider class="w-full" v-model="a2" @change="rac" :min="-90" :max="180" :step="0.1" />
                <InputText class="w-24" v-model.number="a2" />
            </div>
            <div class="flex flex-col gap-2 mt-4">
                <div class="flex flex-row gap-8 items-center">
                    <div class="text-xl w-52">Max Speed (rad/s)</div>
                    <Slider class="w-full" v-model="robotStore[props.r].maxSpeed" :min="0" :max="30" :step="0.1" />
                    <InputText class="w-24" v-model.number="robotStore[props.r].maxSpeed" />
                </div>
            </div>
        </div>
        <div class="card">
            <h2 class="select-none">Joint Angles</h2>
            <h3 class="select-none">Joint 1 (°)</h3>
            <Chart type="line" :data="lineDataA1" :options="lineOptionsA1"></Chart>
            <h3 class="select-none">Joint 2 (°)</h3>
            <Chart type="line" :data="lineDataA2" :options="lineOptionsA2"></Chart>
        </div>
        <RobotMotorParams :robotStore="robotStore" :r="props.r" motor="m1" :def="robotStore" label="Motor 1 Parameters" />
        <RobotMotorParams :robotStore="robotStore" :r="props.r" motor="m2" :def="robotStore" label="Motor 2 Parameters" />
    </div>
</template>
