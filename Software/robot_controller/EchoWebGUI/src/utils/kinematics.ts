import type { Point } from '../utils/types.ts'
import { getDist, getAngle, addPoints, subPoints, scalePoint, polar } from '../utils/utils'

export const b1: Point = { x: -50, y: 0 }
export const b2: Point = { x: 50, y: 0 }
export const armLen = { u: 150, l: 200 }
export const armRadii = { u1: 20.985, u2: 17.5, l1: 17.5, l2: 17.5 }
const factor = 0.99
const max_bDist = (armLen.u + armLen.l) * factor
const max_eDist = 2 * armLen.l * factor
const maxAngle = Math.asin(2 * armRadii.u1 / getDist(b1, b2))

export function calc_fk(a1: number, a2: number): Point {
  const e1: Point = addPoints(b1, polar(a1, armLen.u))
  const e2: Point = addPoints(b2, polar(a2, armLen.u))

  const dist = getDist(e1, e2)
  const angle = getAngle(e1, e2)
  const mid: Point = { x: (e1.x + e2.x) / 2, y: (e1.y + e2.y) / 2 }
  const d = Math.sqrt(Math.pow(armLen.l, 2) - Math.pow(dist / 2, 2))

  return {
    x: mid.x + d * Math.cos(angle + Math.PI / 2),
    y: mid.y + d * Math.sin(angle + Math.PI / 2),
  }
}

export function calc_ik(e: Point): {a1: number, a2: number} {
  const b1dist = getDist(b1, e)
  const b2dist = getDist(b2, e)
  const b1angle = getAngle(b1, e)
  const b2angle = getAngle(b2, e)

  const a1 =
    b1angle + Math.acos(-(armLen.l ** 2 - armLen.u ** 2 - b1dist ** 2) / (2 * armLen.u * b1dist))
  const a2 =
    b2angle - Math.acos(-(armLen.l ** 2 - armLen.u ** 2 - b2dist ** 2) / (2 * armLen.u * b2dist))

  return { a1: a1, a2: a2 }
}

export function validConfiguration(e: Point): boolean {
  const b1dist = getDist(b1, e)
  const b2dist = getDist(b2, e)
  const angles = calc_ik(e)
  const e1: Point = addPoints(b1, polar(angles.a1, armLen.u))
  const e2: Point = addPoints(b2, polar(angles.a2, armLen.u))
  const edist = getDist(e1, e2)

  return (
    b1dist < max_bDist &&
    b2dist < max_bDist &&
    edist < max_eDist &&
    angles.a1 < maxAngle &&
    angles.a2 > Math.PI - maxAngle
  )
}

export function projectToWorkspace(e: Point, mode: String): Point {
  if (mode == "drag") {
    // Project towards the furthest base if out of bounds
    const b1dist = getDist(b1, e)
    const b2dist = getDist(b2, e)
    const angles = calc_ik(e)

    if (b1dist > max_bDist || b2dist > max_bDist) {
      if (b1dist > b2dist) {
        const b1angle = getAngle(b1, e)
        return addPoints(b1, polar(b1angle, max_bDist))
      } else {
        const b2angle = getAngle(b2, e)
        return addPoints(b2, polar(b2angle, max_bDist))
      }
    }

    if (angles.a1 < maxAngle) {
      // a1 should be fixed
      return e
    }
  } else if (mode == "x") {

    return e
  }

  return e

  if (e.x < 0) {

  }


}
