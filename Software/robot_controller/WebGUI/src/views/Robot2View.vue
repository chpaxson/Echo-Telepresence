<script setup>
import { useRobotStore } from '@/stores/robotStore';
import { ref, watch } from 'vue';
import { calc_ik } from '../utils/kinematics';

const robotStore = useRobotStore();

const r1x = ref(robotStore.r2.eePos.x);
const r1y = ref(robotStore.r2.eePos.y);
const initialIK = calc_ik(robotStore.r2.eePos);
const a1 = ref(initialIK.a1);
const a2 = ref(initialIK.a2);

// Watch for changes in r1x and r1y and update a1 and a2 accordingly
watch([r1x, r1y], ([newX, newY]) => {
    const newIK = calc_ik({ x: newX, y: newY });
    a1.value = newIK.a1;
    a2.value = newIK.a2;
});

const r1HomeButtonOptions = ref([
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
</script>

<template>
    <div class="grid grid-cols-2 gap-8">
        <div class="card content-center flex flex-col gap-4">
            <div class="flow-root">
                <h1 class="float-left">Robot 2</h1>
                <SplitButton class="homeButton float-right" label="Home" :model="r1HomeButtonOptions" :severity="r1Homed ? `` : `danger`" @click="r1Home"></SplitButton>
            </div>
            <RobotVis robot="robot1" />
            <div class="font-semibold text-xl"><em>x</em> Position</div>
            <InputText v-model.number="r1x" />
            <Slider v-model="r1x" :min="-300" :max="300" />
            <div class="font-semibold text-xl"><em>y</em> Position</div>
            <InputText v-model.number="r1y" />
            <Slider v-model="r1y" :min="0" :max="300" />
            <div class="font-semibold text-xl">Angle 1 (rad)</div>
            <InputText v-model.number="a1" />
            <Slider v-model="a1" :min="-Math.PI" :max="2 * Math.PI" :step="0.01" />
            <div class="font-semibold text-xl">Angle 2 (rad)</div>
            <InputText v-model.number="a2" />
            <Slider v-model="a2" :min="-Math.PI" :max="Math.PI" :step="0.01" />
        </div>
        <div class="card content-center flex flex-col gap-4">
            <h2>Kinematics</h2>
            <div class="font-semibold text-xl">Max Speed (rad/s)</div>
            <div class="flex flex-row gap-8 items-center">
                <Slider class="w-full" v-model="robotStore.r2.maxSpeed" :min="0" :max="30" :step="0.1" />
                <InputText v-model.number="robotStore.r2.maxSpeed" />
            </div>
            <div class="font-semibold text-xl">Voltage Chopper Limit (V)</div>
            <div class="flex flex-row gap-8 items-center">
                <Slider class="w-full" v-model="robotStore.r2.voltage_limit" :min="0" :max="30" :step="0.1" />
                <InputText v-model.number="robotStore.r2.voltage_limit" />
            </div>
            <h2>Velocity PID</h2>
            <div class="flex flex-row gap-8 items-center">
                <div class="font-semibold text-xl">
                    <em>K<sub>P</sub></em>
                </div>
                <Slider class="w-full" v-model="robotStore.r2.v_kP" :min="0" :max="5" :step="0.01" />
                <InputText v-model.number="robotStore.r2.v_kP" />
            </div>
            <div class="flex flex-row gap-8 items-center">
                <div class="font-semibold text-xl">
                    <em>K<sub>I</sub></em>
                </div>
                <Slider class="w-full" v-model="robotStore.r2.v_kI" :min="0" :max="1" :step="0.01" />
                <InputText v-model.number="robotStore.r2.v_kI" />
            </div>
            <div class="flex flex-row gap-8 items-center">
                <div class="font-semibold text-xl">
                    <em>K<sub>D</sub></em>
                </div>
                <Slider class="w-full" v-model="robotStore.r2.v_kD" :min="0" :max="10" :step="0.01" />
                <InputText v-model.number="robotStore.r2.v_kD" />
            </div>
            <h2>Position PID</h2>
            <div class="flex flex-row gap-8 items-center">
                <div class="font-semibold text-xl">
                    <em>K<sub>P</sub></em>
                </div>
                <Slider class="w-full" v-model="robotStore.r2.a_kP" :min="0" :max="5" :step="0.01" />
                <InputText v-model.number="robotStore.r2.a_kP" />
            </div>
            <div class="flex flex-row gap-8 items-center">
                <div class="font-semibold text-xl">
                    <em>K<sub>I</sub></em>
                </div>
                <Slider class="w-full" v-model="robotStore.r2.a_kI" :min="0" :max="1" :step="0.01" />
                <InputText v-model.number="robotStore.r2.a_kI" />
            </div>
            <div class="flex flex-row gap-8 items-center">
                <div class="font-semibold text-xl">
                    <em>K<sub>D</sub></em>
                </div>
                <Slider class="w-full" v-model="robotStore.r2.a_kD" :min="0" :max="10" :step="0.01" />
                <InputText v-model.number="robotStore.r2.a_kD" />
            </div>
        </div>
        <div class="card content-center flex flex-col gap-4">
            <h2>Motor Parameters</h2>
            <h2>Motor 1</h2>
            <div class="font-semibold text-xl">Phase resistance (Ω)</div>
            <InputText v-model.number="robotStore.r2.motor1_R" />
            <Slider v-model="robotStore.r2.motor1_R" :min="0" :max="10" :step="0.01" />
            <div class="font-semibold text-xl">Phase inductance (mH)</div>
            <InputText v-model.number="robotStore.r2.motor1_I" />
            <Slider v-model="robotStore.r2.motor1_I" :min="0" :max="100" :step="0.1" />
            <h2>Motor 2</h2>
            <div class="font-semibold text-xl">Phase resistance (Ω)</div>
            <InputText v-model.number="robotStore.r2.motor2_R" />
            <Slider v-model="robotStore.r2.motor2_R" :min="0" :max="10" :step="0.01" />
            <div class="font-semibold text-xl">Phase inductance (mH)</div>
            <InputText v-model.number="robotStore.r2.motor2_I" />
            <Slider v-model="robotStore.r2.motor2_I" :min="0" :max="100" :step="0.1" />
        </div>
    </div>
</template>
