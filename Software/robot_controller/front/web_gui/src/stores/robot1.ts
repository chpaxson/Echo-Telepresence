import { defineStore } from 'pinia'
import type { Point } from '../utils/types'

export const useRobot1Store = defineStore('robot1', {
  state: () => ({
    joint3: { x: 0, y: 200 } as Point,
    angularAccel: 0.5 as number,
    maxAngularVel: 0.5 as number,
    maxTorque: 0.5 as number,
  }),
  actions: {
    setJoint3(pos: Point) {
      this.joint3 = pos
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