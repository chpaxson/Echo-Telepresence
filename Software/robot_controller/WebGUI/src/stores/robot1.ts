import { defineStore } from 'pinia'
import type { Point } from '../utils/types'
import { calc_fk, calc_ik } from '@/utils/kinematics'

export const useRobot1Store = defineStore('robot1', {
  state: () => ({
    eePos: { x: 0, y: 200 } as Point,
    ja: {a1: 0, a2: 0},
    angularAccel: 0.5 as number,
    maxAngularVel: 0.5 as number,
    maxTorque: 0.5 as number,
  }),
  actions: {
    setEEPos(pos: Point) {
      this.eePos = pos
      this.ja = calc_ik(pos)
    },
    setJA(ja: {a1: number, a2: number}) {
      this.ja = ja
      this.eePos = calc_fk(ja.a1, ja.a2)
    },
    setMaxAngularAccel(val: number) {
      this.angularAccel = val
    },
    setMaxAngularVel(val: number) {
      this.maxAngularVel = val
    },
    setMaxTorque(val: number) {
      this.maxTorque = val
    }
  },
})
