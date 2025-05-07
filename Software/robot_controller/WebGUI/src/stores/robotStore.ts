import { calc_fk, calc_ik } from '@/utils/kinematics';
import type { Point } from '@/utils/types';
import { defineStore } from 'pinia';

export const useRobot1Store = defineStore('robot1', {
    state: () => ({
        eePos: { x: 0, y: 200 } as Point,
        maxSpeed: 100 as number,
        maxAcc: 100 as number,
        maxDec: 100 as number,
        maxJerk: 100 as number
    }),
    getters: {
        ja: (state) => calc_ik(state.eePos)
    },
    actions: {
        setEEPos(pos) {
            if (pos.a1 !== undefined && pos.a2 !== undefined) {
                this.eePos = calc_fk(pos);
            } else {
                this.eePos = pos;
            }
        }
    }
});

export const useRobot2Store = defineStore('robot2', {
    state: () => ({
        eePos: { x: 0, y: 200 } as Point,
        maxSpeed: 100 as number,
        maxAcc: 100 as number,
        maxDec: 100 as number,
        maxJerk: 100 as number
    }),
    getters: {
        ja: (state) => calc_ik(state.eePos)
    },
    actions: {
        setEEPos(pos) {
            if (pos.a1 !== undefined && pos.a2 !== undefined) {
                this.eePos = calc_fk(pos);
            } else {
                this.eePos = pos;
            }
        }
    }
});

export const useSettingsStore = defineStore('settings', {
    state: () => ({
        useLink: true,
        mappingFactor: 0.5,
        primaryRobot: 'robot1'
    })
});
