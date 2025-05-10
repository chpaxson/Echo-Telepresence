<script setup>
import { ref, nextTick } from 'vue';
import { useLayout } from '@/layout/composables/layout';
import { useRobotStore } from '@/stores/robotStore';
import { $dt } from '@primeuix/themes';
import { useConfirm } from 'primevue/useconfirm';
import { useToast } from 'primevue/usetoast';
import { computed, onMounted, watch } from 'vue';
import { calc_fk, calc_ik, projectToWorkspace } from '../utils/kinematics';

const toast = useToast();
const confirm = useConfirm();

const robotStore = useRobotStore();

const r1m1Ref = ref();
const r1m2Ref = ref();
const r2m1Ref = ref();
const r2m2Ref = ref();

const postParams = (r, m) => {
    const params = store.value[r][m];
    let controllerNum = 0;
    switch (params.controller) {
        case 'Torque':
            controllerNum = 1;
            break;
        case 'Velocity':
            controllerNum = 2;
            break;
        case 'Position':
            controllerNum = 3;
            break;
        case 'Disabled':
            controllerNum = 0;
            break;
    }
    const values = [params.R, params.I, params.kV, params.v_lim, params.i_lim, params.vel_lim, params.sense_dir, params.zea, params.v_kP, params.v_kI, params.v_kD, params.a_kP, params.a_kI, params.a_kD, controllerNum].join(',');

    fetch(`http://echo.local/api/v1/${r}${m}_params`, {
        method: 'POST',
        headers: {
            'Content-Type': 'text/plain'
        },
        body: values
    })
        .then((response) => {
            if (!response.ok) {
                throw new Error(`HTTP error! status: ${response.status}`);
            }
            return response.text();
        })
        .then((data) => {
            console.log('Success:', data);
        })
        .catch((error) => {
            console.error('Error:', error);
        });
};

const saveAllMotorParams = async () => {
    await nextTick();
    r1m1Ref.value?.save();
    await new Promise(resolve => setTimeout(resolve, 1000));
    r1m2Ref.value?.save();
    await new Promise(resolve => setTimeout(resolve, 1000));
    r2m1Ref.value?.save();
    await new Promise(resolve => setTimeout(resolve, 1000));
    r2m2Ref.value?.save();
    r1m2Ref.value?.save();
    r2m1Ref.value?.save();
    r2m2Ref.value?.save();
};

const confirmSaveAll = () => {
    confirm.require({
        message: 'Is the robot power supply on?',
        header: 'Confirm Save',
        icon: 'pi pi-exclamation-triangle',
        acceptLabel: 'Yes',
        rejectLabel: 'No',
        accept: saveAllMotorParams
    });
};
</script>

<template>
    <div class="grid grid-cols-2 gap-8">
        <ConfirmPopup />
        <div class="card flex flex-col items-center justify-center mt-4 col-span-2">
            <Button
                label="Save All Motor Params"
                icon="pi pi-save"
                severity="info"
                @click="confirmSaveAll"
                class="mt-2"
            />
        </div>
        <RobotMotorParams ref="r1m1Ref" :robotStore="robotStore" r="r1" motor="m1" :def="robotStore" label="Robot 1 Motor 1 (M0)" />
        <RobotMotorParams ref="r1m2Ref" :robotStore="robotStore" r="r1" motor="m2" :def="robotStore" label="Robot 1 Motor 2 (M1)" />
        <RobotMotorParams ref="r2m1Ref" :robotStore="robotStore" r="r2" motor="m1" :def="robotStore" label="Robot 2 Motor 1 (M2)" />
        <RobotMotorParams ref="r2m2Ref" :robotStore="robotStore" r="r2" motor="m2" :def="robotStore" label="Robot 2 Motor 2 (M3)" />
        <div class="card">
            <h2 class="select-none">Large Parameters (testing)</h2>
            <div class="flex flex-row gap-8 items-center">
                <div class="text-xl">
                    <em>r<sub>1</sub>a<sub>1</sub></em>
                </div>
                <Slider class="w-full" v-model="robotStore.r1.a1" :min="-5" :max="5" :step="0.1" />
                <InputText class="w-24" v-model.number="robotStore.r1.a1" />
            </div>
            <div class="flex flex-row gap-8 items-center">
                <div class="text-xl">
                    <em>r<sub>1</sub>a<sub>2</sub></em>
                </div>
                <Slider class="w-full" v-model="robotStore.r1.a2" :min="-5" :max="5" :step="0.1" />
                <InputText class="w-24" v-model.number="robotStore.r1.a2" />
            </div>
            <div class="flex flex-row gap-8 items-center">
                <div class="text-xl">
                    <em>r<sub>2</sub>a<sub>1</sub></em>
                </div>
                <Slider class="w-full" v-model="robotStore.r2.a1" :min="-5" :max="5" :step="0.1" />
                <InputText class="w-24" v-model.number="robotStore.r2.a1" />
            </div>
            <div class="flex flex-row gap-8 items-center">
                <div class="text-xl">
                    <em>r<sub>2</sub>a<sub>2</sub></em>
                </div>
                <Slider class="w-full" v-model="robotStore.r2.a2" :min="-5" :max="5" :step="0.1" />
                <InputText class="w-24" v-model.number="robotStore.r2.a2" />
            </div>
        </div>
    </div>
</template>
