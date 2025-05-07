import type { Point } from './types.ts';

export function getDist(p1: Point, p2: Point): number {
    return Math.sqrt((p1.x - p2.x) ** 2 + (p1.y - p2.y) ** 2);
}
export function getAngle(p1: Point, p2: Point): number {
    return Math.atan2(p2.y - p1.y, p2.x - p1.x);
}

export function addPoints(p1: Point, p2: Point): Point {
    return { x: p1.x + p2.x, y: p1.y + p2.y };
}
export function subPoints(p1: Point, p2: Point): Point {
    return { x: p1.x - p2.x, y: p1.y - p2.y };
}
export function scalePoint(p: Point, s: number): Point {
    return { x: p.x * s, y: p.y * s };
}

export function polar(angle: number, length: number): Point {
    return { x: Math.cos(angle) * length, y: Math.sin(angle) * length };
}
