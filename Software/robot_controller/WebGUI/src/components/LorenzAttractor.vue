<script setup lang="ts">
import { onMounted, onBeforeUnmount } from 'vue'
import { turboColormap } from '../utils/colormaps'
import * as THREE from 'three'

let renderer: THREE.WebGLRenderer | null = null
let scene: THREE.Scene
let camera: THREE.PerspectiveCamera
let line: THREE.Line
let animationId: number

var N = 25000
const SEGMENT = 10000 // Number of points in the visible "tail"

let points: [number, number, number, number][] = []
let vMax = 0.01

onMounted(() => {
    // Lorenz parameters
    const sigma = 10, rho = 30, beta = 8 / 3
    let x = 0.1, y = 0, z = 0
    const dt = 0.002

    points = []
    vMax = 0.01
    for (let i = 0; i < N; i++) {
        let dx = sigma * (y - x)
        let dy = x * (rho - z) - y
        let dz = x * y - beta * z
        let v = Math.sqrt(dx * dx + dy * dy + dz * dz)
        vMax = Math.max(v, vMax)
        x += dx * dt
        y += dy * dt
        z += dz * dt
        points.push([x, y, z, v])
    }
    console.log(`Lorenz attractor points: ${N}`)

    // Move the centroid of the points to the origin
    const centroid = points.reduce((acc, [x, y, z]) => {
        acc[0] += x
        acc[1] += y
        acc[2] += z
        return acc
    }, [0, 0, 0])
    centroid[0] /= N
    centroid[1] /= N
    centroid[2] /= N
    console.log(`Centroid: ${centroid}`)
    points = points.map(([x, y, z, v]) => [x - centroid[0], y - centroid[1], z - centroid[2], v])

    // After shifting points to centroid...

    // Get start and end points and tangents
    const p0 = points[0]
    const p1 = points[1]
    const pn = points[points.length - 1]
    const pn_1 = points[points.length - 2]

    // Tangent at start (p0 to p1) and end (pn_1 to pn)
    const startTangent = [
        p1[0] - p0[0],
        p1[1] - p0[1],
        p1[2] - p0[2]
    ]
    const endTangent = [
        pn[0] - pn_1[0],
        pn[1] - pn_1[1],
        pn[2] - pn_1[2]
    ]

    const tangentScale = 100; // Increase for a smoother, wider curve

    const bezierP0 = [pn[0], pn[1], pn[2]]
    const bezierP1 = [
        pn[0] + endTangent[0] * tangentScale,
        pn[1] + endTangent[1] * tangentScale,
        pn[2] + endTangent[2] * tangentScale
    ]
    const bezierP2 = [
        p0[0] - startTangent[0] * tangentScale,
        p0[1] - startTangent[1] * tangentScale,
        p0[2] - startTangent[2] * tangentScale
    ]
    const bezierP3 = [p0[0], p0[1], p0[2]]

    // Sample points along the Bézier curve
    const BEZIER_SAMPLES = 100
    for (let t = 0; t <= 1; t += 1 / BEZIER_SAMPLES) {
        const u = 1 - t
        // Cubic Bézier formula
        const x =
            u ** 3 * bezierP0[0] +
            3 * u ** 2 * t * bezierP1[0] +
            3 * u * t ** 2 * bezierP2[0] +
            t ** 3 * bezierP3[0]
        const y =
            u ** 3 * bezierP0[1] +
            3 * u ** 2 * t * bezierP1[1] +
            3 * u * t ** 2 * bezierP2[1] +
            t ** 3 * bezierP3[1]
        const z =
            u ** 3 * bezierP0[2] +
            3 * u ** 2 * t * bezierP1[2] +
            3 * u * t ** 2 * bezierP2[2] +
            t ** 3 * bezierP3[2]
        // For velocity, you can interpolate or just use the start/end value
        const v = pn[3] * u + p0[3] * t
        points.push([x, y, z, v])
    }

    // Three.js setup
    scene = new THREE.Scene()
    camera = new THREE.PerspectiveCamera(75, window.innerWidth / window.innerHeight, 0.1, 1000)
    camera.position.z = 60

    renderer = new THREE.WebGLRenderer({ alpha: true, antialias: true })
    renderer.setSize(window.innerWidth, window.innerHeight)
    renderer.setClearColor(0x000000, 0)
    renderer.domElement.style.position = 'fixed'
    renderer.domElement.style.top = '0'
    renderer.domElement.style.left = '0'
    renderer.domElement.style.zIndex = '0'
    renderer.domElement.style.pointerEvents = 'none'
    document.body.appendChild(renderer.domElement)

    // Geometry and color (dynamic, only SEGMENT points)
    const positions = new Float32Array(SEGMENT * 3)
    const colors = new Float32Array(SEGMENT * 4) // RGBA

    const geometry = new THREE.BufferGeometry()
    geometry.setAttribute('position', new THREE.BufferAttribute(positions, 3))
    geometry.setAttribute('color', new THREE.BufferAttribute(colors, 4))

    const material = new THREE.LineBasicMaterial({ vertexColors: true, transparent: true })
    line = new THREE.Line(geometry, material)
    scene.add(line)

    let head = 0

    function animate() {
        // Animate head position
        head = (head + 2) % N
        const tail = (head - SEGMENT + N) % N

        // Fill positions/colors for the visible segment
        for (let i = 0; i < SEGMENT; i++) {
            // Circular buffer logic
            const idx = (tail + i) % N
            const [x, y, z, v] = points[idx]
            positions[i * 3 + 0] = x
            positions[i * 3 + 1] = y
            positions[i * 3 + 2] = z

            // Fade tail: alpha goes from 0 (oldest) to 1 (head)
            const alpha = i / SEGMENT
            const t = Math.min(1, v / vMax)
            const [r, g, b] = turboColormap(t)
            colors[i * 4 + 0] = r / 255
            colors[i * 4 + 1] = g / 255
            colors[i * 4 + 2] = b / 255
            colors[i * 4 + 3] = alpha // fade tail
        }
        geometry.attributes.position.needsUpdate = true
        geometry.attributes.color.needsUpdate = true

        line.rotation.y += 0.0002
        renderer!.render(scene, camera)
        animationId = requestAnimationFrame(animate)
    }
    animate()

    window.addEventListener('resize', onResize)
})

onBeforeUnmount(() => {
    if (renderer) {
        renderer.dispose()
        if (renderer.domElement.parentNode) {
            renderer.domElement.parentNode.removeChild(renderer.domElement)
        }
    }
    window.removeEventListener('resize', onResize)
    cancelAnimationFrame(animationId)
})

function onResize() {
    if (!renderer || !camera) return
    camera.aspect = window.innerWidth / window.innerHeight
    camera.updateProjectionMatrix()
    renderer.setSize(window.innerWidth, window.innerHeight)
}
</script>

<template>
    <!-- No visible DOM, background handled by three.js -->
</template>