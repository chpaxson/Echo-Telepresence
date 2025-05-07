<template>
    <div width="min(100%, 600px)" style="aspect-ratio: 4/3">
        <svg width="100%" height="100%" :id="`${props.robot}-svg`" ref="svg">
            <path :id="`${props.robot}-boundary`" :stroke="`${boundaryStroke}`" :fill="`${boundaryFill}`" />
            <path :id="`${props.robot}-u1`" />
            <path :id="`${props.robot}-u2`" />
            <path :id="`${props.robot}-l2`" />
            <path :id="`${props.robot}-l1`" />
            <circle id="ee" :cx="scale * robotStore.eePos.x + offset.x" :cy="scale * robotStore.eePos.y + offset.y" :r="10 * scale" style="cursor: grab" :fill="`${accentColor}`" @mousedown="onDragStart" @touchstart="onTouchStart" />
        </svg>
    </div>
</template>

<script setup lang="ts">
import { useRobot1Store, useRobot2Store } from '@/stores/robotStore';
import { useResizeObserver } from '@vueuse/core';
import { onMounted, onUnmounted, useTemplateRef, watch } from 'vue';
import { armLen, armRadii, b1, b2, boundaryArcs, calc_ik, projectToWorkspace, validConfiguration } from '../utils/kinematics';
import type { Point } from '../utils/types.ts';
import { addPoints, getAngle, getDist, polar, scalePoint } from '../utils/utils';
import '../utils/utils.ts';

const documentStyle = getComputedStyle(document.documentElement);
const accentColor = documentStyle.getPropertyValue('--p-primary-500');
const armStroke = documentStyle.getPropertyValue('--p-primary-800');
const armFill = documentStyle.getPropertyValue('--p-primary-950');
const boundaryStroke = documentStyle.getPropertyValue('--p-surface-600');
const boundaryFill = documentStyle.getPropertyValue('--p-surface-950');

const props = defineProps({
    robot: { type: String, required: true }
});

let scale = 400 / 700;
let offset = { x: 400 / 2, y: 400 / 8 };
let dragOffset = { x: 0, y: 0 };

// Get the svg element width

const svgElement = useTemplateRef('svg');
useResizeObserver(svgElement, (entries) => {
    const entry = entries[0];
    const { width, height } = entry.contentRect;
    scale = width / 700;
    offset = { x: width / 2, y: width / 8 };
    drawBoundaries();
    drawRobot();
    // Slightly nudge eePos to update the end effector circle
    robotStore.eePos = { x: robotStore.eePos.x + 0.0001, y: robotStore.eePos.y };
    robotStore.eePos = { x: robotStore.eePos.x - 0.0001, y: robotStore.eePos.y };
});

function shift(p: Point): Point {
    return addPoints(scalePoint(p, scale), offset);
}

// Dynamically select the store
const robotStore = props.robot === 'robot1' ? useRobot1Store() : useRobot2Store();

function drawArmSegment(element: string, c1: Point, c2: Point, r1: number, r2: number): void {
    // Shift everything into the canvas
    const dist = getDist(c1, c2);
    const angle = getAngle(c1, c2);
    const dr = Math.abs(r1 - r2); // / 2
    const dangle = Math.acos(dr / dist);
    const angle1 = angle - dangle;
    const angle2 = angle + dangle;

    const pa0: Point = shift(addPoints(c1, polar(angle1, r1)));
    const pa1: Point = shift(addPoints(c1, polar(angle2, r1)));
    const pb0: Point = shift(addPoints(c2, polar(angle1, r2)));
    const pb1: Point = shift(addPoints(c2, polar(angle2, r2)));

    r1 = r1 * scale;
    r2 = r2 * scale;

    // Draw the arm segment
    const arc1 = document.getElementById(element);
    arc1?.setAttribute('d', `M ${pa0.x} ${pa0.y} A ${r1} ${r1} 0 ${r1 > r2 ? 1 : 0} 0 ${pa1.x} ${pa1.y} L ${pb1.x} ${pb1.y} A ${r2} ${r2} 0 ${r1 > r2 ? 0 : 1} 0 ${pb0.x} ${pb0.y} Z`);
    arc1?.setAttribute('stroke', armStroke);
    arc1?.setAttribute('stroke-width', '3');
    arc1?.setAttribute('fill', armFill);
}

function onDragStart(event: MouseEvent) {
    dragOffset = { x: robotStore.eePos.x * scale - event.clientX, y: robotStore.eePos.y * scale - event.clientY };
    window.addEventListener('mousemove', onDragMove);
    window.addEventListener('mouseup', onDragEnd);
}
function onDragMove(event: MouseEvent) {
    const newPos = { x: (event.clientX + dragOffset.x) / scale, y: (event.clientY + dragOffset.y) / scale };
    console.log(validConfiguration(shift(newPos), props.robot));
    robotStore.eePos = validConfiguration(shift(newPos), props.robot) ? newPos : projectToWorkspace(newPos);
}
function onDragEnd() {
    window.removeEventListener('mousemove', onDragMove);
    window.removeEventListener('mouseup', onDragEnd);
}

function onTouchStart(event: TouchEvent) {
    event.preventDefault();
    dragOffset = { x: robotStore.eePos.x * scale - event.touches[0].clientX, y: robotStore.eePos.y * scale - event.touches[0].clientY };
    window.addEventListener('touchmove', onTouchMove, { passive: false });
    window.addEventListener('touchend', onTouchEnd);
}
function onTouchMove(event: TouchEvent) {
    event.preventDefault();
    const newPos = { x: (event.touches[0].clientX + dragOffset.x) / scale, y: (event.touches[0].clientY + dragOffset.y) / scale };
    robotStore.eePos = validConfiguration(shift(newPos), props.robot) ? newPos : projectToWorkspace(newPos);
}
function onTouchEnd() {
    window.removeEventListener('touchmove', onTouchMove);
    window.removeEventListener('touchend', onTouchEnd);
}

let lastDraw = 0;
let pending = false;

function drawRobot() {
    const angles = calc_ik(robotStore.eePos);
    const j1 = addPoints(b1, polar(angles.a1, armLen.u));
    const j2 = addPoints(b2, polar(angles.a2, armLen.u));
    drawArmSegment(`${props.robot}-u1`, b1, j1, armRadii.u1, armRadii.u2);
    drawArmSegment(`${props.robot}-u2`, b2, j2, armRadii.u1, armRadii.u2);
    drawArmSegment(`${props.robot}-l2`, j2, robotStore.eePos, armRadii.l1, armRadii.l2);
    drawArmSegment(`${props.robot}-l1`, j1, robotStore.eePos, armRadii.l1, armRadii.l2);
}
const stop = watch(
    () => ({ ...robotStore.eePos }), // watch for changes in joint3
    () => {
        const now = performance.now();
        if (now - lastDraw > 20) {
            drawRobot();
            lastDraw = now;
        } else if (!pending) {
            pending = true;
            setTimeout(
                () => {
                    drawRobot();
                    lastDraw = performance.now();
                    pending = false;
                },
                20 - (now - lastDraw)
            );
        }
    },
    { deep: true }
);

const postPosition = () => {
    const angles = calc_ik(robotStore.eePos);
    fetch(`/api/v1/${props.robot}/angles`, {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({
            a1: angles.a1,
            a2: angles.a2
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

// Post position to the server every time it changes, at most 10 times per second
watch(
    () => ({ ...robotStore.eePos }),
    () => {
        postPosition();
    },
    { deep: true }
);

onMounted(() => {
    drawRobot();
});
onUnmounted(() => stop());

function drawBoundaries() {
    const p1 = shift(boundaryArcs[0].s);
    const p2 = shift(boundaryArcs[0].e);
    const p3 = shift(boundaryArcs[1].e);
    const p4 = shift(boundaryArcs[2].e);
    const p5 = shift(boundaryArcs[3].e);
    const p6 = shift(boundaryArcs[4].e);
    const p7 = shift(boundaryArcs[5].e);
    const r1 = boundaryArcs[0].r * scale;
    const r2 = boundaryArcs[2].r * scale;
    const r3 = boundaryArcs[3].r * scale;
    const r4 = boundaryArcs[4].r * scale;

    document
        .getElementById(props.robot + '-boundary')
        ?.setAttribute(
            'd',
            `M ${p1.x} ${p1.y} A ${r1} ${r1} 0 0 0 ${p2.x} ${p2.y} A ${r2} ${r2} 0 0 0 ${p3.x} ${p3.y} A ${r3} ${r3} 0 0 1 ${p4.x} ${p4.y} A ${r4} ${r4} 0 0 0 ${p5.x} ${p5.y} A ${r3} ${r3} 0 0 1 ${p6.x} ${p6.y} A ${r2} ${r2} 0 0 0 ${p7.x} ${p7.y} A ${r1} ${r1} 0 0 0 ${p1.x} ${p1.y}`
        );
}
onMounted(() => {
    drawBoundaries();
});
</script>
