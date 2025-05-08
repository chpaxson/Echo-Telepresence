import { calc_fk, calc_ik, projectToWorkspace } from '@/utils/kinematics';
import type { Config, Point } from '@/utils/types';
import { defineStore } from 'pinia';

export const useRobotStore = defineStore('robot', {
    state: () => ({
        r1: {
            eePos: { x: 0, y: 200 } as Point,
            config: { a1: 0, a2: 0 } as Config,
            maxSpeed: 20 as number,
            motor1_R: 1.3 as number,
            motor1_I: 29 as number, // mH
            motor2_R: 1.3 as number,
            motor2_I: 29 as number, // mH
            voltage_limit: 24 as number,
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
            maxSpeed: 20 as number,
            motor1_R: 1.3 as number,
            motor1_I: 29 as number, // mH
            motor2_R: 1.3 as number,
            motor2_I: 29 as number, // mH
            voltage_limit: 24 as number,
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
        boundaryMode: 'slide',
        def_R: 1.3 as number,
        def_I: 29 as number,
        def_v_kP: 0.5 as number,
        def_v_kI: 0.1 as number,
        def_v_kD: 0 as number,
        def_a_kP: 0.5 as number,
        def_a_kI: 0 as number,
        def_a_kD: 0 as number
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

// const postPosition = () => {
//     const angles = calc_ik(robotStore[props.r].eePos);
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
//     () => ({ ...robotStore[props.r].eePos }),
//     () => {
//         postPosition();
//     },
//     { deep: true }
// );
var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
window.addEventListener('load', onLoad);
function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage; // <-- add this line
}
function onOpen(event) {
    console.log('Connection opened');
}
function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
}
function onMessage(event) {
    var state;
    console.log(event.data);
    if (event.data == '1') {
        state = 'ON';
    } else {
        state = 'OFF';
    }
    document.getElementById('state').innerHTML = state;
}
function onLoad(event) {
    initWebSocket();
    initButton();
}
function initButton() {
    document.getElementById('button').addEventListener('click', toggle);
}
function toggle() {
    websocket.send('toggle');
}
