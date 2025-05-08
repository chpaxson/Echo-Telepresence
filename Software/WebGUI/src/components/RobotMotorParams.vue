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

const v_lim = ref(0);
const R = ref(0);
const I = ref(0);
const v_kP = ref(0);
const v_kI = ref(0);
const v_kD = ref(0);
const a_kP = ref(0);
const a_kI = ref(0);
const a_kD = ref(0);

const loadFromStore = () => {
    v_lim.value = store.value[r.value][motor.value].v_lim;
    R.value = store.value[r.value][motor.value].R;
    I.value = store.value[r.value][motor.value].I;
    v_kP.value = store.value[r.value][motor.value].v_kP;
    v_kI.value = store.value[r.value][motor.value].v_kI;
    v_kD.value = store.value[r.value][motor.value].v_kD;
    a_kP.value = store.value[r.value][motor.value].a_kP;
    a_kI.value = store.value[r.value][motor.value].a_kI;
    a_kD.value = store.value[r.value][motor.value].a_kD;
};

onMounted(() => {
    loadFromStore();
});

watch(() => store.value[r.value][motor.value], loadFromStore, { deep: true });

const postParamsToRobot = () => {
    fetch(`/api/v1/mParams`, {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({
            r1: {
                m1: store.value.r1.m1,
                m2: store.value.r1.m2
            },
            r2: {
                m1: store.value.r2.m1,
                m2: store.value.r2.m2
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

const save = () => {
    store.value[r.value][motor.value].v_lim = v_lim.value;
    store.value[r.value][motor.value].R = R.value;
    store.value[r.value][motor.value].I = I.value;
    store.value[r.value][motor.value].v_kP = v_kP.value;
    store.value[r.value][motor.value].v_kI = v_kI.value;
    store.value[r.value][motor.value].v_kD = v_kD.value;
    store.value[r.value][motor.value].a_kP = a_kP.value;
    store.value[r.value][motor.value].a_kI = a_kI.value;
    store.value[r.value][motor.value].a_kD = a_kD.value;
    postParamsToRobot();
};
const reset = () => {
    loadFromStore();
    postParamsToRobot();
};
const modified = computed(() => {
    return (
        v_lim.value !== store.value[r.value][motor.value].v_lim ||
        R.value !== store.value[r.value][motor.value].R ||
        I.value !== store.value[r.value][motor.value].I ||
        v_kP.value !== store.value[r.value][motor.value].v_kP ||
        v_kI.value !== store.value[r.value][motor.value].v_kI ||
        v_kD.value !== store.value[r.value][motor.value].v_kD ||
        a_kP.value !== store.value[r.value][motor.value].a_kP ||
        a_kI.value !== store.value[r.value][motor.value].a_kI ||
        a_kD.value !== store.value[r.value][motor.value].a_kD
    );
});
const modified_from_default = computed(() => {
    return (
        v_lim.value !== def.value.def_v_lim ||
        R.value !== def.value.def_R ||
        I.value !== def.value.def_I ||
        v_kP.value !== def.value.def_v_kP ||
        v_kI.value !== def.value.def_v_kI ||
        v_kD.value !== def.value.def_v_kD ||
        a_kP.value !== def.value.def_a_kP ||
        a_kI.value !== def.value.def_a_kI ||
        a_kD.value !== def.value.def_a_kD
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
            v_lim.value = def.value.def_v_lim;
            R.value = def.value.def_R;
            I.value = def.value.def_I;
            v_kP.value = def.value.def_v_kP;
            v_kI.value = def.value.def_v_kI;
            v_kD.value = def.value.def_v_kD;
            a_kP.value = def.value.def_a_kP;
            a_kI.value = def.value.def_a_kI;
            a_kD.value = def.value.def_a_kD;
            store.value[r.value][motor.value].v_lim = def.value.def_v_lim;
            store.value[r.value][motor.value].R = def.value.def_R;
            store.value[r.value][motor.value].I = def.value.def_I;
            store.value[r.value][motor.value].v_kP = def.value.def_v_kP;
            store.value[r.value][motor.value].v_kI = def.value.def_v_kI;
            store.value[r.value][motor.value].v_kD = def.value.def_v_kD;
            store.value[r.value][motor.value].a_kP = def.value.def_a_kP;
            store.value[r.value][motor.value].a_kI = def.value.def_a_kI;
            store.value[r.value][motor.value].a_kD = def.value.def_a_kD;
            postParamsToRobot();
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
            <div class="text-xl float-left">Voltage Limit (V)</div>
            <InputText class="float-right w-24" v-model.number="v_lim" />
        </div>
        <div class="flow-root">
            <div class="text-xl float-left">Phase resistance (Ω)</div>
            <InputText class="float-right w-24" v-model.number="R" />
        </div>
        <div class="flow-root">
            <div class="text-xl float-left">Phase inductance (mH)</div>
            <InputText class="float-right w-24" v-model.number="I" />
        </div>
        <h3 class="select-none">Velocity PID</h3>
        <div class="flex flex-row gap-8 items-center">
            <div class="text-xl">
                <em>K<sub>P</sub></em>
            </div>
            <Slider class="w-full" v-model="v_kP" :min="0" :max="5" :step="0.01" />
            <InputText class="w-24" v-model.number="v_kP" />
        </div>
        <div class="flex flex-row gap-8 items-center">
            <div class="text-xl">
                <em>K<sub>I</sub></em>
            </div>
            <Slider class="w-full" v-model="v_kI" :min="0" :max="1" :step="0.01" />
            <InputText class="w-24" v-model.number="v_kI" />
        </div>
        <div class="flex flex-row gap-8 items-center">
            <div class="text-xl">
                <em>K<sub>D</sub></em>
            </div>
            <Slider class="w-full" v-model="v_kD" :min="0" :max="10" :step="0.01" />
            <InputText class="w-24" v-model.number="v_kD" />
        </div>
        <h3 class="select-none">Position PID</h3>
        <div class="flex flex-row gap-8 items-center">
            <div class="text-xl">
                <em>K<sub>P</sub></em>
            </div>
            <Slider class="w-full" v-model="a_kP" :min="0" :max="5" :step="0.01" />
            <InputText class="w-24" v-model.number="a_kP" />
        </div>
        <div class="flex flex-row gap-8 items-center">
            <div class="text-xl">
                <em>K<sub>I</sub></em>
            </div>
            <Slider class="w-full" v-model="a_kI" :min="0" :max="1" :step="0.01" />
            <InputText class="w-24" v-model.number="a_kI" />
        </div>
        <div class="flex flex-row gap-8 items-center">
            <div class="text-xl">
                <em>K<sub>D</sub></em>
            </div>
            <Slider class="w-full" v-model="a_kD" :min="0" :max="10" :step="0.01" />
            <InputText class="w-24" v-model.number="a_kD" />
        </div>
    </div>
</template>
