<template>
    <div width="min(100%, 600px)" style="aspect-ratio: 4/3">
        <svg width="100%" height="100%" :id="`${props.r}-svg`" ref="svg">
            <path :id="`${props.r}-boundary`" :stroke="`var(--p-surface-600)`" :fill="`var(--p-surface-950)`" />
            <path :id="`${props.r}-u1`" />
            <path :id="`${props.r}-u2`" />
            <path :id="`${props.r}-l2`" />
            <path :id="`${props.r}-l1`" />
            <circle id="ee" :cx="scale * robotStore[props.r].ee.x + offset.x" :cy="scale * robotStore[props.r].ee.y + offset.y" :r="10 * scale" style="cursor: grab" :fill="`var(--p-primary-500)`" @mousedown="onDragStart" @touchstart="onTouchStart" />
            <text :x="scale * b1.x + offset.x" :y="scale * (b1.y - 30) + offset.y" font-size="12" text-anchor="middle" fill="var(--p-surface-100)" class="select-none">b1</text>
            <text :x="scale * b2.x + offset.x" :y="scale * (b2.y - 30) + offset.y" font-size="12" text-anchor="middle" fill="var(--p-surface-100)" class="select-none">b2</text>
        </svg>
    </div>
</template>

<script setup lang="ts">
import { useRobotStore } from '@/stores/robotStore';
import { useResizeObserver } from '@vueuse/core';
import { onMounted, onUnmounted, useTemplateRef, watch } from 'vue';
import { armLen, armRadii, b1, b2, boundaryArcs, calc_ik, projectToWorkspace, validConfiguration } from '../utils/kinematics';
import type { Point } from '../utils/types.ts';
import { addPoints, getAngle, getDist, polar, scalePoint } from '../utils/utils';
import '../utils/utils.ts';

const props = defineProps({
    r: { type: String, required: true }
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
    // Slightly nudge e to update the end effector circle
    robotStore[props.r].ee = { x: robotStore[props.r].ee.x + 0.0001, y: robotStore[props.r].ee.y };
    robotStore[props.r].ee = { x: robotStore[props.r].ee.x - 0.0001, y: robotStore[props.r].ee.y };
});

function shift(p: Point): Point {
    return addPoints(scalePoint(p, scale), offset);
}

const robotStore = useRobotStore();
// Dynamically select the store
// const robotStore[props.r] = props.r === 'robot1' ? useRobot1Store() : useRobot2Store();

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
    arc1?.setAttribute('stroke', 'var(--p-primary-800)');
    arc1?.setAttribute('stroke-width', '3');
    arc1?.setAttribute('fill', 'var(--p-primary-950)');
}

function onDragStart(event: MouseEvent) {
    dragOffset = { x: robotStore[props.r].ee.x * scale - event.clientX, y: robotStore[props.r].ee.y * scale - event.clientY };
    window.addEventListener('mousemove', onDragMove);
    window.addEventListener('mouseup', onDragEnd);
}
function onDragMove(event: MouseEvent) {
    if (robotStore[props.r].driver === 'gui') {
        const newPos = { x: (event.clientX + dragOffset.x) / scale, y: (event.clientY + dragOffset.y) / scale };
        robotStore[props.r].ee = validConfiguration(shift(newPos), props.r) ? newPos : projectToWorkspace(newPos);
    }
}
function onDragEnd() {
    window.removeEventListener('mousemove', onDragMove);
    window.removeEventListener('mouseup', onDragEnd);
}

function onTouchStart(event: TouchEvent) {
    event.preventDefault();
    dragOffset = { x: robotStore[props.r].ee.x * scale - event.touches[0].clientX, y: robotStore[props.r].ee.y * scale - event.touches[0].clientY };
    window.addEventListener('touchmove', onTouchMove, { passive: false });
    window.addEventListener('touchend', onTouchEnd);
}
function onTouchMove(event: TouchEvent) {
    event.preventDefault();
    if (robotStore[props.r].driver === 'gui') {
        const newPos = { x: (event.touches[0].clientX + dragOffset.x) / scale, y: (event.touches[0].clientY + dragOffset.y) / scale };
        robotStore[props.r].ee = validConfiguration(shift(newPos), props.r) ? newPos : projectToWorkspace(newPos);
    }
}
function onTouchEnd() {
    window.removeEventListener('touchmove', onTouchMove);
    window.removeEventListener('touchend', onTouchEnd);
}

let lastDraw = 0;
let pending = false;

function drawRobot() {
    const angles = calc_ik(robotStore[props.r].ee);
    const j1 = addPoints(b1, polar(angles.a1, armLen.u));
    const j2 = addPoints(b2, polar(angles.a2, armLen.u));
    drawArmSegment(`${props.r}-u1`, b1, j1, armRadii.u1, armRadii.u2);
    drawArmSegment(`${props.r}-u2`, b2, j2, armRadii.u1, armRadii.u2);
    drawArmSegment(`${props.r}-l2`, j2, robotStore[props.r].ee, armRadii.l1, armRadii.l2);
    drawArmSegment(`${props.r}-l1`, j1, robotStore[props.r].ee, armRadii.l1, armRadii.l2);
}
const stop = watch(
    () => ({ ...robotStore[props.r].ee }), // watch for changes in joint3
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
        .getElementById(props.r + '-boundary')
        ?.setAttribute(
            'd',
            `M ${p1.x} ${p1.y} A ${r1} ${r1} 0 0 0 ${p2.x} ${p2.y} A ${r2} ${r2} 0 0 0 ${p3.x} ${p3.y} A ${r3} ${r3} 0 0 1 ${p4.x} ${p4.y} A ${r4} ${r4} 0 0 0 ${p5.x} ${p5.y} A ${r3} ${r3} 0 0 1 ${p6.x} ${p6.y} A ${r2} ${r2} 0 0 0 ${p7.x} ${p7.y} A ${r1} ${r1} 0 0 0 ${p1.x} ${p1.y}`
        );
}
onMounted(() => {
    drawBoundaries();
});
</script>
