import type { Point } from '../utils/types.ts'
import { getDist, getAngle, addPoints, subPoints, scalePoint, polar } from '../utils/utils'

const jDist = 100
export const b1: Point = { x: -jDist/2, y: 0 }
export const b2: Point = { x: jDist/2, y: 0 }
export const armLen = { u: 150, l: 200 }
export const armRadii = { u1: 20, u2: 17.5, l1: 17.5, l2: 17.5 }
const factor = 0.99
const max_bDist = (armLen.u + armLen.l)

const max_eDist = 2 * armLen.l
const maxAngle = Math.asin(2 * armRadii.u1 / getDist(b1, b2))
const maxAngle2 = Math.PI - Math.asin(armRadii.u1/(jDist/2)) + Math.atan((armRadii.u1 - armRadii.u2) / armLen.u)

const motorWidth = 42.4
const motorDist = 46
const motorAngle = 0
const motorPosAngle = Math.acos((jDist/2 - motorWidth*Math.sqrt(2)/4)/motorDist)
const motorLimitingAngle = armRadii.u1 + 42.4/2


const boundaryArcs = {
  1: {
    // center: polar()
  },
  2: {

  },
  3: {

  }
}

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

  if (Number.isNaN(a1) || Number.isNaN(a2)) {
    return { a1: b1angle, a2: 0 }
  }

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
  const b1dist = getDist(b1, e)
  const b2dist = getDist(b2, e)
  const a1 = getAngle(b1, e)
  const a2 = getAngle(b2, e)
  if (mode == "drag") {
    // if

  } else if (mode == "x") {

  }

  return e


}
