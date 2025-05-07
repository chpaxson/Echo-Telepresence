<script setup>
import { useRobotStore } from '@/stores/robotStore';

const robotStore = useRobotStore();
</script>

<template>
    <div class="grid grid-cols-2 gap-8">
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
    </div>
</template>
