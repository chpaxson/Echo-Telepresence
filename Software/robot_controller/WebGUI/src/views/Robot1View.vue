<script setup>
import { useRobotStore } from '@/stores/robotStore';
import { ref, watch } from 'vue';
import { calc_ik } from '../utils/kinematics';

const robotStore = useRobotStore();

const r1x = ref(robotStore.r1.eePos.x);
const r1y = ref(robotStore.r1.eePos.y);
const initialIK = calc_ik(robotStore.r1.eePos);
const a1 = ref(initialIK.a1);
const a2 = ref(initialIK.a2);

// Watch for changes in r1x and r1y and update a1 and a2 accordingly
watch([r1x, r1y], ([newX, newY]) => {
    const newIK = calc_ik({ x: newX, y: newY });
    a1.value = newIK.a1;
    a2.value = newIK.a2;
});
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
            <div class="font-semibold text-xl">Angle 1 (rad)</div>
            <InputText v-model.number="a1" />
            <Slider v-model="a1" :min="-Math.PI" :max="2 * Math.PI" :step="0.01" />
            <div class="font-semibold text-xl">Angle 2 (rad)</div>
            <InputText v-model.number="a2" />
            <Slider v-model="a2" :min="-Math.PI" :max="Math.PI" :step="0.01" />
        </div>
        <div class="card content-center flex flex-col gap-4">
            <h1>Kinematics</h1>
            <div class="font-semibold text-xl">Max Speed (rad/s)</div>
            <InputText v-model.number="robotStore.r1.maxSpeed" />
            <Slider v-model="robotStore.r1.maxSpeed" :min="0" :max="100" />
            <div class="font-semibold text-xl">Max Acceleration (rad/s<sup>2</sup>)</div>
            <InputText v-model.number="robotStore.r1.maxAcc" />
            <Slider v-model="robotStore.r1.maxAcc" :min="0" :max="100" />
            <div class="font-semibold text-xl">Max Deceleration (rad/s<sup>2</sup>)</div>
            <InputText v-model.number="robotStore.r1.maxDec" />
            <Slider v-model="robotStore.r1.maxDec" :min="0" :max="100" />
            <div class="font-semibold text-xl">Max Jerk (rad/s<sup>3</sup>)</div>
            <InputText v-model.number="robotStore.r1.maxJerk" />
            <Slider v-model="robotStore.r1.maxJerk" :min="0" :max="100" />
        </div>
    </div>
</template>
