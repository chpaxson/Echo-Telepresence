<script setup>
import { useConfirm } from 'primevue/useconfirm';
import { useToast } from 'primevue/usetoast';
import { ref, onMounted, toRefs, watch, computed } from 'vue';

const props = defineProps({
    robotStore: { type: Object, required: true },
    r: { type: String, required: true }, // 'r1' or 'r2'
    motor: { type: String, required: true }, // 'm1' or 'm2'
    def: { type: Object, required: true },
    label: { type: String, required: true }
});

const toast = useToast();
const confirmPopup = useConfirm();

const { robotStore: store, r, motor } = toRefs(props);

const controller = ref('');
const v_lim = ref(0);
const i_lim = ref(0);
const vel_lim = ref(0);
const R = ref(0);
const I = ref(0);
const kV = ref(0);
const zea = ref(0);
const sense_dir = ref(0);
const v_kP = ref(0);
const v_kI = ref(0);
const v_kD = ref(0);
const a_kP = ref(0);
const a_kI = ref(0);
const a_kD = ref(0);

const defaultOptions = [
    { label: 'Store Defaults', value: 'store' },
    { label: 'Component Defaults', value: 'component' }
];
const resetSource = ref('store');

const loadFromStore = () => {
    controller.value = store.value[r.value][motor.value].controller;
    v_lim.value = store.value[r.value][motor.value].v_lim;
    R.value = store.value[r.value][motor.value].R;
    I.value = store.value[r.value][motor.value].I;
    v_kP.value = store.value[r.value][motor.value].v_kP;
    v_kI.value = store.value[r.value][motor.value].v_kI;
    v_kD.value = store.value[r.value][motor.value].v_kD;
    a_kP.value = store.value[r.value][motor.value].a_kP;
    a_kI.value = store.value[r.value][motor.value].a_kI;
    a_kD.value = store.value[r.value][motor.value].a_kD;
    i_lim.value = store.value[r.value][motor.value].i_lim;
    vel_lim.value = store.value[r.value][motor.value].vel_lim;
    kV.value = store.value[r.value][motor.value].kV;
    zea.value = store.value[r.value][motor.value].zea;
    sense_dir.value = store.value[r.value][motor.value].sense_dir;
};

const loadDefaults = () => {
    let defaults;
    if (resetSource.value === 'store') {
        defaults = store.value.default_params;
    } else {
        defaults = props.def;
    }
    controller.value = defaults.controller;
    v_lim.value = defaults.v_lim;
    R.value = defaults.R;
    I.value = defaults.I;
    v_kP.value = defaults.v_kP;
    v_kI.value = defaults.v_kI;
    v_kD.value = defaults.v_kD;
    a_kP.value = defaults.a_kP;
    a_kI.value = defaults.a_kI;
    a_kD.value = defaults.a_kD;
    i_lim.value = defaults.i_lim;
    vel_lim.value = defaults.vel_lim;
    kV.value = defaults.kV;
    zea.value = defaults.zea;
    sense_dir.value = defaults.sense_dir;
};

onMounted(() => {
    loadFromStore();
});

watch(() => store.value[r.value][motor.value], loadFromStore, { deep: true });

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

const save = () => {
    store.value[r.value][motor.value].controller = controller.value;
    store.value[r.value][motor.value].v_lim = v_lim.value;
    store.value[r.value][motor.value].R = R.value;
    store.value[r.value][motor.value].I = I.value;
    store.value[r.value][motor.value].v_kP = v_kP.value;
    store.value[r.value][motor.value].v_kI = v_kI.value;
    store.value[r.value][motor.value].v_kD = v_kD.value;
    store.value[r.value][motor.value].a_kP = a_kP.value;
    store.value[r.value][motor.value].a_kI = a_kI.value;
    store.value[r.value][motor.value].a_kD = a_kD.value;
    store.value[r.value][motor.value].i_lim = i_lim.value;
    store.value[r.value][motor.value].vel_lim = vel_lim.value;
    store.value[r.value][motor.value].kV = kV.value;
    store.value[r.value][motor.value].zea = zea.value;
    store.value[r.value][motor.value].sense_dir = sense_dir.value;

    postParams(props.r, props.motor);
    toast.add({ severity: 'success', summary: 'Parameters saved', life: 2000 });
};

defineExpose({ save });

const reset = () => {
    loadFromStore();
    toast.add({ severity: 'info', summary: 'Parameters reset to current values', life: 2000 });
};

const resetToDefaults = () => {
    loadDefaults();
    toast.add({ severity: 'info', summary: 'Parameters set to defaults', life: 2000 });
};

const currentPos = computed(() => {
    const a1 = props.robotStore[props.r].realConfig.a1;
    const a2 = props.robotStore[props.r].realConfig.a2;
    return (props.motor === 'm1' ? a1 : a2) / 4.5;
});
</script>

<template>
    <div class="card content-center flex flex-col gap-2">
        <div class="flex flex-row items-center">
            <h2 class="w-full select-none">{{ label }}</h2>
            <div class="flex flex-row gap-2">
                <Button label="Save" icon="pi pi-save" @click="save"></Button>
                <Button label="Reset" icon="pi pi-replay" @click="reset"></Button>
                <Button label="Defaults" icon="pi pi-refresh" @click="resetToDefaults"></Button>
            </div>
        </div>
        <div class="grid grid-cols-1 md:grid-cols-2 gap-4">
            <div class="flex items-center justify-between">
                <div class="text-xl">Current Position (rad):</div>

                <InputText class="w-36" v-model="currentPos" :readonly="true" />
            </div>
            <div class="flex items-center justify-between">
                <div class="text-xl">Motion Controller</div>
                <Select class="w-36" v-model="controller" :options="[`Position`, `Velocity`, `Torque`, `Disabled`]" />
            </div>
            <div class="flex items-center justify-between">
                <div class="text-xl">Voltage Limit (V)</div>
                <InputText class="w-36" v-model.number="v_lim" />
            </div>
            <div class="flex items-center justify-between">
                <div class="text-xl">Current Limit (A)</div>
                <InputText class="w-36" v-model.number="i_lim" />
            </div>
            <div class="flex items-center justify-between">
                <div class="text-xl">Velocity Limit (rad/s)</div>
                <InputText class="w-36" v-model.number="vel_lim" />
            </div>
            <div class="flex items-center justify-between">
                <div class="text-xl">Phase resistance (Ω)</div>
                <InputText class="w-36" v-model.number="R" />
            </div>
            <div class="flex items-center justify-between">
                <div class="text-xl">Phase inductance (H)</div>
                <InputText class="w-36" v-model.number="I" />
            </div>
            <div class="flex items-center justify-between">
                <div class="text-xl">
                    <em>K<sub>V</sub></em>
                </div>
                <InputText class="w-36" v-model.number="kV" />
            </div>
            <div class="flex items-center justify-between">
                <div class="text-xl">Zero Electric Angle</div>
                <InputText class="w-36" v-model.number="zea" />
            </div>
            <div class="flex items-center justify-between">
                <div class="text-xl">Sense Direction</div>
                <InputText class="w-36" v-model.number="sense_dir" />
            </div>
        </div>
        <h3 class="select-none">Velocity PID</h3>
        <div class="flex flex-row gap-8 items-center">
            <div class="text-xl">
                <em>K<sub>P</sub></em>
            </div>
            <Slider class="w-full" v-model="v_kP" :min="0" :max="5" :step="0.01" />
            <InputText class="w-36" v-model.number="v_kP" />
        </div>
        <div class="flex flex-row gap-8 items-center">
            <div class="text-xl">
                <em>K<sub>I</sub></em>
            </div>
            <Slider class="w-full" v-model="v_kI" :min="0" :max="1" :step="0.01" />
            <InputText class="w-36" v-model.number="v_kI" />
        </div>
        <div class="flex flex-row gap-8 items-center">
            <div class="text-xl">
                <em>K<sub>D</sub></em>
            </div>
            <Slider class="w-full" v-model="v_kD" :min="0" :max="10" :step="0.01" />
            <InputText class="w-36" v-model.number="v_kD" />
        </div>
        <h3 class="select-none">Position PID</h3>
        <div class="flex flex-row gap-8 items-center">
            <div class="text-xl">
                <em>K<sub>P</sub></em>
            </div>
            <Slider class="w-full" v-model="a_kP" :min="0" :max="5" :step="0.01" />
            <InputText class="w-36" v-model.number="a_kP" />
        </div>
        <div class="flex flex-row gap-8 items-center">
            <div class="text-xl">
                <em>K<sub>I</sub></em>
            </div>
            <Slider class="w-full" v-model="a_kI" :min="0" :max="1" :step="0.01" />
            <InputText class="w-36" v-model.number="a_kI" />
        </div>
        <div class="flex flex-row gap-8 items-center">
            <div class="text-xl">
                <em>K<sub>D</sub></em>
            </div>
            <Slider class="w-full" v-model="a_kD" :min="0" :max="10" :step="0.01" />
            <InputText class="w-36" v-model.number="a_kD" />
        </div>
    </div>
</template>
