<script setup>
import { useRobotStore } from '@/stores/robotStore';
import { ref, watch, computed } from 'vue';
import { calc_ik } from '../utils/kinematics';

const robotStore = useRobotStore();

const initialIK = calc_ik(robotStore.r1.eePos);
const a1 = ref((initialIK.a1 * 180) / Math.PI);
const a2 = ref((initialIK.a2 * 180) / Math.PI);
const config = computed(() => calc_ik(robotStore.r1.eePos));
watch(config, (newPos) => {
    a1.value = (newPos.a1 * 180) / Math.PI;
    a2.value = (newPos.a2 * 180) / Math.PI;
});

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
    toast.add({ severity: 'success', summary: 'Homed Robot 1!' });
};

const r1Homed = computed(() => robotStore.r1.j1Homed && robotStore.r1.j2Homed);
</script>

<template>
    <div class="grid grid-cols-2 gap-8">
        <div class="card content-center flex flex-col gap-4">
            <div class="flow-root">
                <h1 class="float-left">Robot 1</h1>
                <SplitButton class="homeButton float-right" label="Home" :model="r1HomeButtonOptions" :severity="r1Homed ? `` : `danger`" @click="r1Home"></SplitButton>
            </div>
            <RobotVis robot="robot1" />
            <div class="flex flex-row gap-8 items-center">
                <div class="font-semibold text-xl">
                    <em>x</em>
                </div>
                <Slider class="w-full" v-model="robotStore.r1.eePos.x" :min="-300" :max="300" :step="0.1" />
                <InputText v-model.number="robotStore.r1.eePos.x" />
            </div>
            <div class="flex flex-row gap-8 items-center">
                <div class="font-semibold text-xl">
                    <em>y</em>
                </div>
                <Slider class="w-full" v-model="robotStore.r1.eePos.y" :min="0" :max="350" :step="0.1" />
                <InputText v-model.number="robotStore.r1.eePos.y" />
            </div>
            <div class="flex flex-row gap-8 items-center">
                <div class="font-semibold text-xl">
                    <em>a<sub>1</sub></em>
                </div>
                <Slider class="w-full" v-model="a1" :min="0" :max="270" :step="0.1" />
                <InputText v-model.number="a1" />
            </div>
            <div class="flex flex-row gap-8 items-center">
                <div class="font-semibold text-xl">
                    <em>a<sub>2</sub></em>
                </div>
                <Slider class="w-full" v-model="a2" :min="-90" :max="180" :step="0.1" />
                <InputText v-model.number="a2" />
            </div>
        </div>
        <div class="card content-center flex flex-col gap-4">
            <h2>Kinematics</h2>
            <div class="font-semibold text-xl">Max Speed (rad/s)</div>
            <div class="flex flex-row gap-8 items-center">
                <Slider class="w-full" v-model="robotStore.r1.maxSpeed" :min="0" :max="30" :step="0.1" />
                <InputText v-model.number="robotStore.r1.maxSpeed" />
            </div>
            <div class="font-semibold text-xl">Voltage Chopper Limit (V)</div>
            <div class="flex flex-row gap-8 items-center">
                <Slider class="w-full" v-model="robotStore.r1.voltage_limit" :min="0" :max="30" :step="0.1" />
                <InputText v-model.number="robotStore.r1.voltage_limit" />
            </div>
            <h2>Velocity PID</h2>
            <div class="flex flex-row gap-8 items-center">
                <div class="font-semibold text-xl">
                    <em>K<sub>P</sub></em>
                </div>
                <Slider class="w-full" v-model="robotStore.r1.v_kP" :min="0" :max="5" :step="0.01" />
                <InputText v-model.number="robotStore.r1.v_kP" />
            </div>
            <div class="flex flex-row gap-8 items-center">
                <div class="font-semibold text-xl">
                    <em>K<sub>I</sub></em>
                </div>
                <Slider class="w-full" v-model="robotStore.r1.v_kI" :min="0" :max="1" :step="0.01" />
                <InputText v-model.number="robotStore.r1.v_kI" />
            </div>
            <div class="flex flex-row gap-8 items-center">
                <div class="font-semibold text-xl">
                    <em>K<sub>D</sub></em>
                </div>
                <Slider class="w-full" v-model="robotStore.r1.v_kD" :min="0" :max="10" :step="0.01" />
                <InputText v-model.number="robotStore.r1.v_kD" />
            </div>
            <h2>Position PID</h2>
            <div class="flex flex-row gap-8 items-center">
                <div class="font-semibold text-xl">
                    <em>K<sub>P</sub></em>
                </div>
                <Slider class="w-full" v-model="robotStore.r1.a_kP" :min="0" :max="5" :step="0.01" />
                <InputText v-model.number="robotStore.r1.a_kP" />
            </div>
            <div class="flex flex-row gap-8 items-center">
                <div class="font-semibold text-xl">
                    <em>K<sub>I</sub></em>
                </div>
                <Slider class="w-full" v-model="robotStore.r1.a_kI" :min="0" :max="1" :step="0.01" />
                <InputText v-model.number="robotStore.r1.a_kI" />
            </div>
            <div class="flex flex-row gap-8 items-center">
                <div class="font-semibold text-xl">
                    <em>K<sub>D</sub></em>
                </div>
                <Slider class="w-full" v-model="robotStore.r1.a_kD" :min="0" :max="10" :step="0.01" />
                <InputText v-model.number="robotStore.r1.a_kD" />
            </div>
        </div>
        <div class="card content-center flex flex-col gap-4">
            <h2>Motor Parameters</h2>
            <h2>Motor 1</h2>
            <div class="font-semibold text-xl">Phase resistance (Ω)</div>
            <InputText v-model.number="robotStore.r1.motor1_R" />
            <Slider v-model="robotStore.r1.motor1_R" :min="0" :max="10" :step="0.01" />
            <div class="font-semibold text-xl">Phase inductance (mH)</div>
            <InputText v-model.number="robotStore.r1.motor1_I" />
            <Slider v-model="robotStore.r1.motor1_I" :min="0" :max="100" :step="0.1" />
            <h2>Motor 2</h2>
            <div class="font-semibold text-xl">Phase resistance (Ω)</div>
            <InputText v-model.number="robotStore.r1.motor2_R" />
            <Slider v-model="robotStore.r1.motor2_R" :min="0" :max="10" :step="0.01" />
            <div class="font-semibold text-xl">Phase inductance (mH)</div>
            <InputText v-model.number="robotStore.r1.motor2_I" />
            <Slider v-model="robotStore.r1.motor2_I" :min="0" :max="100" :step="0.1" />
        </div>
    </div>
</template>
