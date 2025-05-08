<script setup>
import { useRobotStore } from '@/stores/robotStore';
import { useConfirm } from 'primevue/useconfirm';
import { useToast } from 'primevue/usetoast';
import { computed, onMounted, ref } from 'vue';
import { calc_fk, calc_ik, projectToWorkspace } from '../utils/kinematics';

const props = defineProps({
    r: { type: String, required: true }
});

const toast = useToast();
const confirmPopup = useConfirm();

const robotStore = useRobotStore();

const a1 = ref(0);
const a2 = ref(0);

const m1_R = ref(0);
const m1_I = ref(0);
const m2_R = ref(0);
const m2_I = ref(0);

onMounted(() => {
    m1_R.value = robotStore.r1.motor1_R;
    m1_I.value = robotStore.r1.motor1_I;
    m2_R.value = robotStore.r1.motor2_R;
    m2_I.value = robotStore.r1.motor2_I;
    const initialIK = calc_ik(robotStore[props.r].eePos);
    a1.value = (initialIK.a1 * 180) / Math.PI;
    a2.value = (initialIK.a2 * 180) / Math.PI;
});

const m1_save = () => {
    robotStore.r1.motor1_R = m1_R.value;
    robotStore.r1.motor1_I = m1_I.value;
    toast.add({ severity: 'success', summary: 'Saved Motor 1 parameters!', detail: `Phase resistance R=${m1_R.value}, inductance I=${m1_I.value}`, life: 3000 });
};
const m1_reset = () => {
    m1_R.value = robotStore.r1.motor1_R;
    m1_I.value = robotStore.r1.motor1_I;
    toast.add({ severity: 'success', summary: 'Reset Motor 1 Parameters!', life: 3000 });
};
const m1_modified = computed(() => {
    return m1_R.value !== robotStore.r1.motor1_R || m1_I.value !== robotStore.r1.motor1_I;
});
const m1_modified_from_default = computed(() => {
    return m1_R.value !== robotStore.def_R || m1_I.value !== robotStore.def_I;
});

function m1_confirm_reset(event) {
    confirmPopup.require({
        target: event.target,
        message: `Reset to default values? (R=${robotStore.def_R}Ω, I=${robotStore.def_I}mH)`,
        icon: 'pi pi-exclamation-triangle',
        rejectProps: { label: 'Cancel', severity: 'secondary', outlined: true },
        acceptProps: { label: 'Reset' },
        accept: () => {
            m1_R.value = robotStore.def_R;
            m1_I.value = robotStore.def_I;
            robotStore.r1.motor1_R = robotStore.def_R;
            robotStore.r1.motor1_I = robotStore.def_I;
            toast.add({ severity: 'success', summary: 'Reset Motor 1 parameters to default!', detail: `Phase resistance R=${m1_R.value}, inductance I=${m1_I.value}`, life: 3000 });
        }
    });
}
const m2_save = () => {
    robotStore.r1.motor1_R = m2_R.value;
    robotStore.r1.motor1_I = m2_I.value;
    toast.add({ severity: 'success', summary: 'Saved Motor 1 parameters!', detail: `Phase resistance R=${m2_R.value}, inductance I=${m2_I.value}`, life: 3000 });
};
const m2_reset = () => {
    m2_R.value = robotStore.r1.motor1_R;
    m2_I.value = robotStore.r1.motor1_I;
    toast.add({ severity: 'success', summary: 'Reset Motor 1 Parameters!', life: 3000 });
};
const m2_modified = computed(() => {
    return m2_R.value !== robotStore.r1.motor1_R || m2_I.value !== robotStore.r1.motor1_I;
});
const m2_modified_from_default = computed(() => {
    return m2_R.value !== robotStore.def_R || m2_I.value !== robotStore.def_I;
});

function m2_confirm_reset(event) {
    confirmPopup.require({
        target: event.target,
        message: `Reset to default values? (R=${robotStore.def_R}Ω, I=${robotStore.def_I}mH)`,
        icon: 'pi pi-exclamation-triangle',
        rejectProps: { label: 'Cancel', severity: 'secondary', outlined: true },
        acceptProps: { label: 'Reset' },
        accept: () => {
            m2_R.value = robotStore.def_R;
            m2_I.value = robotStore.def_I;
            robotStore.r1.motor1_R = robotStore.def_R;
            robotStore.r1.motor1_I = robotStore.def_I;
            toast.add({ severity: 'success', summary: 'Reset Motor 1 parameters to default!', detail: `Phase resistance R=${m2_R.value}, inductance I=${m2_I.value}`, life: 3000 });
        }
    });
}

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
const r1Home = () => {
    robotStore.r1.j1Homed = true;
    robotStore.r1.j2Homed = true;
    toast.add({ severity: 'success', summary: 'Homed Robot 1!', life: 3000 });
};

const r1Homed = computed(() => robotStore.r1.j1Homed && robotStore.r1.j2Homed);

const rc = () => {
    robotStore.r1.eePos = projectToWorkspace(robotStore.r1.eePos);
    const ik = calc_ik(robotStore.r1.eePos);
    a1.value = (ik.a1 * 180) / Math.PI;
    a2.value = (ik.a2 * 180) / Math.PI;
};
const rac = () => {
    robotStore.r1.eePos = projectToWorkspace(
        calc_fk({
            a1: (a1.value * Math.PI) / 180,
            a2: (a2.value * Math.PI) / 180
        })
    );
    const ik = calc_ik(robotStore.r1.eePos);
    a1.value = (ik.a1 * 180) / Math.PI;
    a2.value = (ik.a2 * 180) / Math.PI;
};
</script>

<template>
    <div class="grid grid-cols-2 gap-8">
        <div class="card content-center flex flex-col gap-4">
            <div class="flow-root">
                <h1 class="float-left">Robot 1</h1>
                <SplitButton class="homeButton float-right" icon="pi pi-home" :label="r1Homed ? `Homed` : `Home`" :model="r1HomeButtonOptions" :severity="r1Homed ? `` : `danger`" @click="r1Home"></SplitButton>
            </div>
            <RobotVis r="r1" />
            <div class="flex flex-row gap-8 items-center">
                <div class="font-semibold text-xl">
                    <em>x</em>
                </div>
                <Slider class="w-full" v-model="robotStore.r1.eePos.x" @change="rc" :min="-300" :max="300" :step="0.1" />
                <InputText class="w-24" v-model.number="robotStore.r1.eePos.x" />
            </div>
            <div class="flex flex-row gap-8 items-center">
                <div class="font-semibold text-xl">
                    <em>y</em>
                </div>
                <Slider class="w-full" v-model="robotStore.r1.eePos.y" @change="rc" :min="0" :max="350" :step="0.1" />
                <InputText class="w-24" v-model.number="robotStore.r1.eePos.y" />
            </div>
            <div class="flex flex-row gap-8 items-center">
                <div class="font-semibold text-xl">
                    <em>a<sub>1</sub></em>
                </div>
                <Slider class="w-full" v-model="a1" @change="rac" :min="0" :max="270" :step="0.1" />
                <InputText class="w-24" v-model.number="a1" />
            </div>
            <div class="flex flex-row gap-8 items-center">
                <div class="font-semibold text-xl">
                    <em>a<sub>2</sub></em>
                </div>
                <Slider class="w-full" v-model="a2" @change="rac" :min="-90" :max="180" :step="0.1" />
                <InputText class="w-24" v-model.number="a2" />
            </div>
        </div>
        <div class="card content-center flex flex-col gap-2">
            <h2>Kinematics</h2>
            <div class="font-semibold text-xl">Max Speed (rad/s)</div>
            <div class="flex flex-row gap-8 items-center">
                <Slider class="w-full" v-model="robotStore.r1.maxSpeed" :min="0" :max="30" :step="0.1" />
                <InputText class="w-24" v-model.number="robotStore.r1.maxSpeed" />
            </div>
            <div class="font-semibold text-xl">Voltage Chopper Limit (V)</div>
            <div class="flex flex-row gap-8 items-center">
                <Slider class="w-full" v-model="robotStore.r1.voltage_limit" :min="0" :max="30" :step="0.1" />
                <InputText class="w-24" v-model.number="robotStore.r1.voltage_limit" />
            </div>
        </div>
        <div class="card content-center flex flex-col gap-2">
            <div class="flex flex-row items-center">
                <h2 class="w-full">Motor 1 Parameters</h2>
                <div class="flex flex-row gap-2">
                    <Button label="Save" icon="pi pi-save" :disabled="!m1_modified" :severity="m1_modified ? `success` : `contrast`" @click="m1_save"></Button>
                    <ConfirmPopup></ConfirmPopup>
                    <Button label="Reset" icon="pi pi-replay" :disabled="!m1_modified_from_default" :severity="m1_modified ? `danger` : `contrast`" @click="m1_modified ? m1_reset() : m1_confirm_reset($event)"></Button>
                </div>
            </div>
            <div class="flow-root">
                <div class="font-semibold text-xl float-left">Phase resistance (Ω)</div>
                <InputText class="float-right w-24" v-model.number="m1_R" />
            </div>
            <div class="flow-root">
                <div class="font-semibold text-xl float-left">Phase inductance (mH)</div>
                <InputText class="float-right w-24" v-model.number="m1_I" />
            </div>
            <h2>Velocity PID</h2>
            <div class="flex flex-row gap-8 items-center">
                <div class="font-semibold text-xl">
                    <em>K<sub>P</sub></em>
                </div>
                <Slider class="w-full" v-model="robotStore.r1.v_kP" :min="0" :max="5" :step="0.01" />
                <InputText class="w-24" v-model.number="robotStore.r1.v_kP" />
            </div>
            <div class="flex flex-row gap-8 items-center">
                <div class="font-semibold text-xl">
                    <em>K<sub>I</sub></em>
                </div>
                <Slider class="w-full" v-model="robotStore.r1.v_kI" :min="0" :max="1" :step="0.01" />
                <InputText class="w-24" v-model.number="robotStore.r1.v_kI" />
            </div>
            <div class="flex flex-row gap-8 items-center">
                <div class="font-semibold text-xl">
                    <em>K<sub>D</sub></em>
                </div>
                <Slider class="w-full" v-model="robotStore.r1.v_kD" :min="0" :max="10" :step="0.01" />
                <InputText class="w-24" v-model.number="robotStore.r1.v_kD" />
            </div>
            <h2>Position PID</h2>
            <div class="flex flex-row gap-8 items-center">
                <div class="font-semibold text-xl">
                    <em>K<sub>P</sub></em>
                </div>
                <Slider class="w-full" v-model="robotStore.r1.a_kP" :min="0" :max="5" :step="0.01" />
                <InputText class="w-24" v-model.number="robotStore.r1.a_kP" />
            </div>
            <div class="flex flex-row gap-8 items-center">
                <div class="font-semibold text-xl">
                    <em>K<sub>I</sub></em>
                </div>
                <Slider class="w-full" v-model="robotStore.r1.a_kI" :min="0" :max="1" :step="0.01" />
                <InputText class="w-24" v-model.number="robotStore.r1.a_kI" />
            </div>
            <div class="flex flex-row gap-8 items-center">
                <div class="font-semibold text-xl">
                    <em>K<sub>D</sub></em>
                </div>
                <Slider class="w-full" v-model="robotStore.r1.a_kD" :min="0" :max="10" :step="0.01" />
                <InputText class="w-24" v-model.number="robotStore.r1.a_kD" />
            </div>
        </div>
    </div>
</template>
