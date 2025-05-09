import { calc_fk, calc_ik, projectToWorkspace } from '@/utils/kinematics';
import type { Config, Point } from '@/utils/types';
import { defineStore } from 'pinia';

export const useRobotStore = defineStore('robot', {
    state: () => ({
        r1: {
            driver: 'GUI' as string,
            homed: false as boolean,
            ee: { x: 0, y: 200 } as Point,
            realConfig: { a1: 0 as number, a2: 0 as number },
            v: { j1: 0 as number, j2: 0 as number },
            maxSpeed: 20 as number,
            input_voltage: 0 as number,
            m1: {
                R: 1.5 as number,
                I: 0.029 as number,
                kV: 25 as number,
                v_lim: 24 as number,
                i_lim: 2 as number,
                vel_lim: 10 as number,
                sense_dir: 1 as number,
                zea: 0 as number,
                v_kP: 0.2 as number,
                v_kI: 20 as number,
                v_kD: 0.001 as number,
                a_kP: 20 as number,
                a_kI: 0.0 as number,
                a_kD: 0.0 as number,
                controller: 'Position' as string
            },
            m2: {
                R: 1.5 as number,
                I: 0.029 as number,
                kV: 25 as number,
                v_lim: 24 as number,
                i_lim: 2 as number,
                vel_lim: 10 as number,
                sense_dir: 1 as number,
                zea: 0 as number,
                v_kP: 0.2 as number,
                v_kI: 20 as number,
                v_kD: 0.001 as number,
                a_kP: 20 as number,
                a_kI: 0.0 as number,
                a_kD: 0.0 as number,
                controller: 'Position' as string
            },
            a1: 0 as number,
            a2: 0 as number
        },
        r2: {
            driver: 'GUI' as string,
            homed: false as boolean,
            ee: { x: 0, y: 200 } as Point,
            realConfig: { a1: 0 as number, a2: 0 as number },
            v: { j1: 0 as number, j2: 0 as number },
            maxSpeed: 20 as number,
            input_voltage: 0 as number,
            m1: {
                R: 1.3 as number,
                I: 0.029 as number,
                kV: 25 as number,
                v_lim: 24 as number,
                i_lim: 2 as number,
                vel_lim: 10 as number,
                sense_dir: 1 as number,
                zea: 0 as number,
                v_kP: 0.5 as number,
                v_kI: 0.1 as number,
                v_kD: 0.0 as number,
                a_kP: 0.5 as number,
                a_kI: 0.0 as number,
                a_kD: 0.0 as number,
                controller: 'Position' as string
            },
            m2: {
                R: 1.3 as number,
                I: 0.029 as number,
                kV: 25 as number,
                v_lim: 24 as number,
                i_lim: 2 as number,
                vel_lim: 10 as number,
                sense_dir: 1 as number,
                zea: 0 as number,
                v_kP: 0.5 as number,
                v_kI: 0.1 as number,
                v_kD: 0.0 as number,
                a_kP: 0.5 as number,
                a_kI: 0.0 as number,
                a_kD: 0.0 as number,
                controller: 'Position' as string
            },
            a1: 0 as number,
            a2: 0 as number
        },
        webSocketStatus: false as boolean,
        enableLink: false,
        linkRatio: 0.5,
        primaryRobot: 'robot1',

        default_params: {
            R: 1.3 as number,
            I: 0.029 as number,
            kV: 25 as number,
            v_lim: 24 as number,
            i_lim: 2 as number,
            vel_lim: 10 as number,
            sense_dir: 1 as number,
            zea: 0 as number,
            v_kP: 0.5 as number,
            v_kI: 0.1 as number,
            v_kD: 0.0 as number,
            a_kP: 0.5 as number,
            a_kI: 0.0 as number,
            a_kD: 0.0 as number,
            controller: 'Position' as string
        }
    }),
    actions: {
        setEE(robot: 'r1' | 'f2', e: Point) {
            this[robot].ee = projectToWorkspace(e);
            this[robot].config = calc_ik(this[robot].ee);
        },
        setJA(robot: 'r1' | 'f2', jointAngles: Config) {
            this[robot].config = jointAngles;
            this[robot].ee = projectToWorkspace(calc_fk(jointAngles));
        }
    }
});

// const postPosition = () => {
//     const angles = calc_ik(robotStore[props.r].ee);
//     fetch(`/api/v1/${props.r}/angles`, {
//         method: 'POST',
//         headers: {
//             'Content-Type': 'application/json'
//         },
//         body: JSON.stringify({
//             a1: angles.a1,
//             a2: angles.a2
//         })
//     })
//         .then((response) => {
//             if (!response.ok) {
//                 throw new Error(`HTTP error! status: ${response.status}`);
//             }
//             return response.json();
//         })
//         .then((data) => {
//             console.log('Success:', data);
//         })
//         .catch((error) => {
//             console.error('Error:', error);
//         });
// };

// Post position to the server every time it changes, at most 10 times per second
// watch(
//     () => ({ ...robotStore[props.r].ee }),
//     () => {
//         postPosition();
//     },
//     { deep: true }
// );
