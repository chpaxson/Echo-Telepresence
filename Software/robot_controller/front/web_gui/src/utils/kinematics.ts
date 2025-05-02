import type { Point } from '../utils/types.ts'
import { getDist, getAngle, addPoints, scalePoint, polar } from '../utils/utils'

interface Circle {
  c: Point
  r: number
  add?: boolean
  s?: Point
  e?: Point
}

const jDist = 100
export const b1: Point = { x: -jDist/2, y: 0 }
export const b2: Point = { x: jDist/2, y: 0 }
export const armLen = { u: 150, l: 200 }
export const armRadii = { u1: 20, u2: 17.5, l1: 17.5, l2: 17.5 }

const motorWidth = 42.4
const motorDist = 46

// Calculate robot geometry
// Motor limiting angles
const uArmAngle = Math.asin((armRadii.u1 - armRadii.u2)/armLen.u)
const a = 3*Math.PI/4 - Math.acos((motorWidth/2 + armRadii.u1)/motorDist)
const d = Math.sqrt(motorDist**2 + (motorWidth*Math.sqrt(2)/2)**2 - 2 * motorDist * (motorWidth*Math.sqrt(2)/2) * Math.cos(a))
const b = Math.sqrt(d**2 - (jDist/2)**2)
const maxMotorAngle = -(Math.PI - (Math.asin(Math.sin(a) / d * (motorWidth*Math.sqrt(2)/2)) + Math.asin(b/d)) - Math.asin((motorWidth/2+armRadii.u1)/motorDist) + uArmAngle)
const minMotorAngle = Math.asin(2*armRadii.u1 / jDist)

function flipX(p: Point): Point {return { x: -p.x, y: p.y }}

export function findCircleIntersection(p1: Circle, p2: Circle): Point[] {
  const c1 = p1.c
  const c2 = p2.c
  const r1 = p1.r
  const r2 = p2.r
  const d = getDist(c1, c2)
  if (d > r1 + r2 || d < Math.abs(r1 - r2)) {
    return []
  }

  const a = (r1 ** 2 - r2 ** 2 + d ** 2) / (2 * d)
  const h = Math.sqrt(r1 ** 2 - a ** 2)
  const p = { x: c1.x + a * (c2.x - c1.x) / d, y: c1.y + a * (c2.y - c1.y) / d }

  return [
    { x: p.x + h * (c2.y - c1.y) / d, y: p.y - h * (c2.x - c1.x) / d },
    { x: p.x - h * (c2.y - c1.y) / d, y: p.y + h * (c2.x - c1.x) / d },
  ]
}


export const boundaryArcs = [
  {
    c: b1,
    r: armLen.u + armLen.l,
    add: true,
    s: {x: 0, y: 0},
    e: {x: 0, y: 0},
  },{
    c: b2,
    r: armLen.u + armLen.l,
    add: true,
    s: {x: 0, y: 0},
    e: {x: 0, y: 0},
  }, {
    c: addPoints(b1, polar(minMotorAngle, armLen.u)),
    r: armLen.l,
    add: true,
    s: {x: 0, y: 0},
    e: {x: 0, y: 0},
  }, {
    c: addPoints(b2, polar(maxMotorAngle, armLen.u)),
    r: armLen.l,
    add: false,
    s: {x: 0, y: 0},
    e: {x: 0, y: 0},
  }, {
    c: {x: 0, y: Math.tan(Math.PI - maxMotorAngle) * jDist/2*0.999},
    r: (jDist/2) / Math.cos(maxMotorAngle) + armLen.u  - armLen.l,
    add: true,
    s: {x: 0, y: 0},
    e: {x: 0, y: 0},
  }, {
    c: flipX(addPoints(b2, polar(maxMotorAngle, armLen.u))),
    r: armLen.l,
    add: false,
    s: {x: 0, y: 0},
    e: {x: 0, y: 0},
  }, {
    c: flipX(addPoints(b1, polar(minMotorAngle, armLen.u))),
    r: armLen.l,
    add: true,
    s: {x: 0, y: 0},
    e: {x: 0, y: 0},
  }
]
boundaryArcs[0].s = findCircleIntersection(boundaryArcs[0], boundaryArcs[1])[1]
boundaryArcs[0].e = findCircleIntersection(boundaryArcs[0], boundaryArcs[2])[1]
boundaryArcs[1].s = boundaryArcs[0].e
boundaryArcs[1].e = findCircleIntersection(boundaryArcs[2], boundaryArcs[3])[1]
boundaryArcs[2].s = boundaryArcs[1].e
boundaryArcs[2].e = findCircleIntersection(boundaryArcs[3], boundaryArcs[4])[0]
boundaryArcs[3].s = boundaryArcs[2].e
boundaryArcs[3].e = findCircleIntersection(boundaryArcs[4], boundaryArcs[5])[0]
boundaryArcs[4].s = boundaryArcs[3].e
boundaryArcs[4].e = findCircleIntersection(boundaryArcs[5], boundaryArcs[6])[1]
boundaryArcs[5].s = boundaryArcs[4].e
boundaryArcs[5].e = findCircleIntersection(boundaryArcs[6], boundaryArcs[1])[1]
boundaryArcs[6].s = boundaryArcs[5].e
boundaryArcs[6].e = boundaryArcs[0].s



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
    return calc_ik(scalePoint(e, 0.999))
  }

  return { a1: a1, a2: a2 }
}

export function validConfiguration(p: Point, robotString: string): boolean {
  // Use SVG .isPointInFill to check if the point is inside the boundary path
  const svg = document.getElementById(robotString+"-svg") as unknown as SVGSVGElement
  const path = (document.getElementById(robotString+"-boundary") as unknown) as SVGGeometryElement
  const pointObj = svg?.createSVGPoint()
  pointObj.x = p.x
  pointObj.y = p.y
  return path?.isPointInFill(pointObj) || path?.isPointInStroke(pointObj)


}

function projectToArc(p: Point, arc: Circle): Point {
  return addPoints(arc.c, polar(getAngle(arc.c, p), arc.r))
}

export function projectToWorkspace(e: Point): Point {
  const dist = getDist(boundaryArcs[4].c, e)
  const proj = projectToArc(e, boundaryArcs[4])
  if (proj.x < boundaryArcs[3].s.x && proj.x > boundaryArcs[3].e.x && proj.y < boundaryArcs[3].e.y) {
    if (dist > boundaryArcs[4].r) {
      return proj
    }
  }

  if (e.x < 0) {
    // Closer to b1, project to further b2
    const b2dist = getDist(b2, e)
    const b2proj = projectToArc(e, boundaryArcs[1])
    if (b2proj.y > boundaryArcs[0].e.y) {
      if (b2dist > boundaryArcs[1].r) {
        return b2proj
      }
    } else {
      const dist1 = getDist(boundaryArcs[6].c, e)
      const dist2 = getDist(boundaryArcs[5].c, e)
      const proj1 = projectToArc(e, boundaryArcs[6])
      const proj2 = projectToArc(e, boundaryArcs[5])
      const dproj1 = getDist(proj1, boundaryArcs[5].c)
      const dproj2 = getDist(proj2, boundaryArcs[6].c)
      if (dist1 > boundaryArcs[6].r && dist2 < boundaryArcs[5].r) {
        return boundaryArcs[5].s
      } else if (dist1 > boundaryArcs[6].r && dist2 > boundaryArcs[5].r) {
        if (dproj1 < boundaryArcs[5].r) {
          return boundaryArcs[5].s
        }
        return proj1
      } else if (dist1 < boundaryArcs[6].r && dist2 < boundaryArcs[5].r) {
        if (dproj2 > boundaryArcs[6].r) {
          return boundaryArcs[5].s
        }
        return proj2
      }

    }
  } else {
    // Closer to b2, project to further b1
    const b1dist = getDist(b1, e)
    const b1proj = projectToArc(e, boundaryArcs[0])
    if (b1proj.y > boundaryArcs[0].e.y) {
      if (b1dist > boundaryArcs[0].r) {
        return b1proj
      }
    } else {
      const dist1 = getDist(boundaryArcs[2].c, e)
      const dist2 = getDist(boundaryArcs[3].c, e)
      const proj1 = projectToArc(e, boundaryArcs[2])
      const proj2 = projectToArc(e, boundaryArcs[3])
      const dproj1 = getDist(proj1, boundaryArcs[3].c)
      const dproj2 = getDist(proj2, boundaryArcs[2].c)
      if (dist1 > boundaryArcs[2].r && dist2 < boundaryArcs[3].r) {
        return boundaryArcs[1].e
      } else if (dist1 > boundaryArcs[2].r && dist2 > boundaryArcs[3].r) {
        if (dproj1 < boundaryArcs[3].r) {
          return boundaryArcs[1].e
        }
        return proj1
      } else if (dist1 < boundaryArcs[2].r && dist2 < boundaryArcs[3].r) {
        if (dproj2 > boundaryArcs[2].r) {
          return boundaryArcs[1].e
        }
        return proj2
      }
    }
  }
  return e

}
