#import "@preview/cetz:0.3.4": *
#set page(width: 1cm, height: 1cm, fill: none)
#set align(center + horizon)

#canvas(length: 1cm, {
  import draw: *
  set-style(stroke: (thickness: 0.75pt, cap: "round"))
  let a = 45deg
  let d = 90deg - a
  let r1 = 3.5pt
  let dr = 2.5pt

  circle((0,0), radius: 0.5cm, fill: gradient.linear(purple.darken(40%), blue.darken(20%), angle: 55deg), stroke: none)

  rotate(-20deg)

  circle((0,0), radius: 1pt)

  for i in range(4) {
    let r = r1 + i * dr
    rotate(15deg)
    arc((a, r), radius: r, start: a, delta: d)
    rotate(90deg)
    arc((a, r), radius: r, start: a, delta: d)
    rotate(90deg)
    arc((a, r), radius: r, start: a, delta: d)
    rotate(90deg)
    arc((a, r), radius: r, start: a, delta: d)
  }
  
})