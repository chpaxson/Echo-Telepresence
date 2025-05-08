import { calc_fk, calc_ik, projectToWorkspace } from '@/utils/kinematics';
import type { Config, Point } from '@/utils/types';
import { defineStore } from 'pinia';

export const useRobotStore = defineStore('robot', {
    state: () => ({
        r1: {
            eePos: { x: 0, y: 200 } as Point,
            config: { a1: 0, a2: 0 } as Config,
            maxSpeed: 100 as number,
            motor1_R: 1.3 as number,
            motor1_I: 29 as number, // mH
            motor2_R: 1.3 as number,
            motor2_I: 29 as number, // mH
            voltage_limit: 12 as number,
            v_kP: 0.5 as number,
            v_kI: 0.1 as number,
            v_kD: 0.1 as number,
            a_kP: 0.5 as number,
            a_kI: 0.1 as number,
            a_kD: 0.1 as number,
            j1homed: false as boolean,
            j2homed: false as boolean,
            input_voltage: 0 as number
        },
        r2: {
            eePos: { x: 0, y: 200 } as Point,
            maxSpeed: 100 as number,
            motor1_R: 1.3 as number,
            motor1_I: 29 as number, // mH
            motor2_R: 1.3 as number,
            motor2_I: 29 as number, // mH
            voltage_limit: 12 as number,
            v_kP: 0.5 as number,
            v_kI: 0.1 as number,
            v_kD: 0.1 as number,
            a_kP: 0.5 as number,
            a_kI: 0.1 as number,
            a_kD: 0.1 as number,
            j1homed: false as boolean,
            j2homed: false as boolean,
            input_voltage: 0 as number
        },
        enableLink: false,
        linkRatio: 0.5,
        primaryRobot: 'robot1',
        boundaryMode: 'slide'
    }),
    actions: {
        setEE(robot: 'r1' | 'f2', eePos: Point) {
            this[robot].eePos = projectToWorkspace(eePos);
            this[robot].config = calc_ik(this[robot].eePos);
        },
        setJA(robot: 'r1' | 'f2', jointAngles: Config) {
            this[robot].config = jointAngles;
            this[robot].eePos = projectToWorkspace(calc_fk(jointAngles));
        }
    }
});
