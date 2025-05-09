<script setup>
import { useConfirm } from 'primevue/useconfirm';
import { useToast } from 'primevue/usetoast';
import { computed, onMounted, ref, toRefs, watch } from 'vue';

const props = defineProps({
    robotStore: { type: Object, required: true },
    r: { type: String, required: true }, // 'r1' or 'r2'
    motor: { type: String, required: true }, // 'm1' or 'm2'
    def: { type: Object, required: true },
    label: { type: String, required: true }
});

const toast = useToast();
const confirmPopup = useConfirm();

const { robotStore: store, r, motor, def } = toRefs(props);

const controller = ref('');
const v_lim = ref(0);
const i_lim = ref(0);
const vel_lim = ref(0);
const R = ref(0);
const I = ref(0);
const kV = ref(0);
const zea = ref(0);
const sdir = ref(0);
const v_kP = ref(0);
const v_kI = ref(0);
const v_kD = ref(0);
const a_kP = ref(0);
const a_kI = ref(0);
const a_kD = ref(0);

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
    sdir.value = store.value[r.value][motor.value].sdir;
};

onMounted(() => {
    loadFromStore();
});

watch(() => store.value[r.value][motor.value], loadFromStore, { deep: true });

const postParams = (r, m) => {
    const params = store.value[r][m];
    // Order: R, L, kV, v_lim, i_lim, vel_lim, sdir, zea, v_kP, v_kI, v_kD, a_kP, a_kI, a_kD, controller
    const values = [params.R, params.I, params.kV, params.v_lim, params.i_lim, params.vel_lim, params.sdir, params.zea, params.v_kP, params.v_kI, params.v_kD, params.a_kP, params.a_kI, params.a_kD, params.controller].join(',');

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
    store.value[r.value][motor.value].sdir = sdir.value;

    postParams(props.r, props.motor);
};
const reset = () => {
    loadFromStore();
    postParams(props.r, props.motor);
};
const modified = computed(() => {
    return (
        controller.value !== store.value[r.value][motor.value].controller ||
        v_lim.value !== store.value[r.value][motor.value].v_lim ||
        R.value !== store.value[r.value][motor.value].R ||
        I.value !== store.value[r.value][motor.value].I ||
        v_kP.value !== store.value[r.value][motor.value].v_kP ||
        v_kI.value !== store.value[r.value][motor.value].v_kI ||
        v_kD.value !== store.value[r.value][motor.value].v_kD ||
        a_kP.value !== store.value[r.value][motor.value].a_kP ||
        a_kI.value !== store.value[r.value][motor.value].a_kI ||
        a_kD.value !== store.value[r.value][motor.value].a_kD ||
        i_lim.value !== store.value[r.value][motor.value].i_lim ||
        vel_lim.value !== store.value[r.value][motor.value].vel_lim ||
        kV.value !== store.value[r.value][motor.value].kV ||
        zea.value !== store.value[r.value][motor.value].zea ||
        sdir.value !== store.value[r.value][motor.value].sdir
    );
});
const modified_from_default = computed(() => {
    return (
        controller.value !== def.value.def_controller ||
        v_lim.value !== def.value.def_v_lim ||
        R.value !== def.value.def_R ||
        I.value !== def.value.def_I ||
        v_kP.value !== def.value.def_v_kP ||
        v_kI.value !== def.value.def_v_kI ||
        v_kD.value !== def.value.def_v_kD ||
        a_kP.value !== def.value.def_a_kP ||
        a_kI.value !== def.value.def_a_kI ||
        a_kD.value !== def.value.def_a_kD ||
        i_lim.value !== def.value.def_i_lim ||
        vel_lim.value !== def.value.def_vel_lim ||
        kV.value !== def.value.def_kV ||
        zea.value !== def.value.def_zea ||
        sdir.value !== def.value.def_sdir
    );
});
function confirm_reset(event) {
    confirmPopup.require({
        target: event.target,
        message: `Reset to default values? (V_lim=${def.value.def_v_lim}V, R=${def.value.def_R}Ω, I=${def.value.def_I}mH)`,
        icon: 'pi pi-exclamation-triangle',
        rejectProps: { label: 'Cancel', severity: 'secondary', outlined: true },
        acceptProps: { label: 'Reset' },
        accept: () => {
            controller.value = def.value.def_controller;
            v_lim.value = def.value.def_v_lim;
            R.value = def.value.def_R;
            I.value = def.value.def_I;
            v_kP.value = def.value.def_v_kP;
            v_kI.value = def.value.def_v_kI;
            v_kD.value = def.value.def_v_kD;
            a_kP.value = def.value.def_a_kP;
            a_kI.value = def.value.def_a_kI;
            a_kD.value = def.value.def_a_kD;
            i_lim.value = def.value.def_i_lim;
            vel_lim.value = def.value.def_vel_lim;
            kV.value = def.value.def_kV;
            zea.value = def.value.def_zea;
            sdir.value = def.value.def_sdir;
            store.value[r.value][motor.value].controller = def.value.def_controller;
            store.value[r.value][motor.value].v_lim = def.value.def_v_lim;
            store.value[r.value][motor.value].R = def.value.def_R;
            store.value[r.value][motor.value].I = def.value.def_I;
            store.value[r.value][motor.value].v_kP = def.value.def_v_kP;
            store.value[r.value][motor.value].v_kI = def.value.def_v_kI;
            store.value[r.value][motor.value].v_kD = def.value.def_v_kD;
            store.value[r.value][motor.value].a_kP = def.value.def_a_kP;
            store.value[r.value][motor.value].a_kI = def.value.def_a_kI;
            store.value[r.value][motor.value].a_kD = def.value.def_a_kD;
            store.value[r.value][motor.value].i_lim = def.value.def_i_lim;
            store.value[r.value][motor.value].vel_lim = def.value.def_vel_lim;
            store.value[r.value][motor.value].kV = def.value.def_kV;
            store.value[r.value][motor.value].zea = def.value.def_zea;
            store.value[r.value][motor.value].sdir = def.value.def_sdir;

            postParams(props.r, props.motor);
        }
    });
}
</script>

<template>
    <div class="card content-center flex flex-col gap-2">
        <div class="flex flex-row items-center">
            <h2 class="w-full select-none">{{ label }}</h2>
            <div class="flex flex-row gap-2">
                <Button label="Save" icon="pi pi-save" :disabled="!modified" :severity="modified ? `success` : `contrast`" @click="save"></Button>
                <ConfirmPopup></ConfirmPopup>
                <Button label="Reset" icon="pi pi-replay" :disabled="!modified_from_default" :severity="modified ? `danger` : `contrast`" @click="modified ? reset() : confirm_reset($event)"></Button>
            </div>
        </div>
        <div class="flow-root">
            <div class="text-xl float-left">Motion Controller</div>
            <Select class="float-right w-36" v-model="controller" :options="[`Position`, `Velocity`, `Torque`, `Disabled`]" />
        </div>
        <div class="flow-root">
            <div class="text-xl float-left">Voltage Limit (V)</div>
            <InputText class="float-right w-36" v-model.number="v_lim" />
        </div>
        <div class="flow-root">
            <div class="text-xl float-left">Current Limit (A)</div>
            <InputText class="float-right w-36" v-model.number="i_lim" />
        </div>
        <div class="flow-root">
            <div class="text-xl float-left">Velocity Limit (rad/s)</div>
            <InputText class="float-right w-36" v-model.number="vel_lim" />
        </div>

        <div class="flow-root">
            <div class="text-xl float-left">Phase resistance (Ω)</div>
            <InputText class="float-right w-36" v-model.number="R" />
        </div>
        <div class="flow-root">
            <div class="text-xl float-left">Phase inductance (mH)</div>
            <InputText class="float-right w-36" v-model.number="I" />
        </div>
        <div class="flow-root">
            <div class="text-xl float-left">
                <em>K<sub>V</sub></em>
            </div>
            <InputText class="float-right w-36" v-model.number="kV" />
        </div>
        <div class="flow-root">
            <div class="text-xl float-left">Zero Electric Angle</div>
            <InputText class="float-right w-36" v-model.number="zea" />
        </div>
        <div class="flow-root">
            <div class="text-xl float-left">Sense Direction</div>
            <InputText class="float-right w-36" v-model.number="sdir" />
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
