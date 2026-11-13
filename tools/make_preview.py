#!/usr/bin/env python3
"""Draws Better Explosions' Workshop preview: a cartoon explosion with bits of car, a skull
and a potted plant riding its shockwave. Writes an SVG; render it with the Bangers font
(Google Fonts, OFL) available:  rsvg-convert -w 1024 -h 1024 preview.svg -o BetterExplosions/preview.png

    make_preview.py out.svg              Better Explosions
    make_preview.py out.svg expansion    the Expansion Vehicles add-on: a Huey rotor blade, a
                                         tractor tyre and a UAZ door, and the add-on's banner"""
import math
import random
import sys

W = H = 1024
EXP = len(sys.argv) > 2 and sys.argv[2] == "expansion"
CX, CY = 512, 462          # the blast
RING = 290                 # the shockwave's radius
if EXP:                    # room under it for the add-on's banner
    CY, RING = 432, 274
INK = "#1a0d08"            # outline colour
OUT = 7                    # outline width

rnd = random.Random(7)
parts = []


def add(s):
    parts.append(s)


def cloud(puffs, fill, outline=True):
    """a cartoon cloud: circles, outlined as one shape (all the strokes, then all the fills)"""
    if outline:
        add('<g fill="%s" stroke="%s" stroke-width="%d">' % (INK, INK, OUT * 2))
        for x, y, r in puffs:
            add('<circle cx="%.1f" cy="%.1f" r="%.1f"/>' % (x, y, r))
        add('</g>')
    add('<g fill="%s">' % fill)
    for x, y, r in puffs:
        add('<circle cx="%.1f" cy="%.1f" r="%.1f"/>' % (x, y, r))
    add('</g>')


def blob(cx, cy, rx, ry, n, rmin, rmax, core):
    """puffs round an ellipse plus a body filling the middle"""
    puffs = [(cx, cy, core)]
    for i in range(n):
        a = 2 * math.pi * i / n + rnd.uniform(-0.12, 0.12)
        r = rnd.uniform(rmin, rmax)
        puffs.append((cx + math.cos(a) * rx, cy + math.sin(a) * ry, r))
    return puffs


def on_ring(deg, extra=0):
    a = math.radians(deg)
    return CX + math.cos(a) * (RING + extra), CY + math.sin(a) * (RING + extra)


def speed_lines(deg, length=62, spread=24, gap=40):
    """streaks trailing from the blast to whatever is riding the wave at deg"""
    a = math.radians(deg)
    ux, uy = math.cos(a), math.sin(a)
    px, py = -uy, ux
    for k, off in enumerate((-spread, 0, spread)):
        r1 = RING - gap - (k % 2) * 18
        r0 = r1 - length + (k % 2) * 30
        add('<line x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f" stroke="#fff4c8" stroke-width="9" '
            'stroke-linecap="round" opacity="0.9"/>' % (
                CX + ux * r0 + px * off, CY + uy * r0 + py * off,
                CX + ux * r1 + px * off, CY + uy * r1 + py * off))


def item(deg, rot, body, scale=1.0, extra=0):
    x, y = on_ring(deg, extra)
    add('<g transform="translate(%.1f %.1f) rotate(%.1f) scale(%.2f)" stroke="%s" '
        'stroke-width="%d" stroke-linejoin="round" stroke-linecap="round">' % (x, y, rot, scale, INK, OUT))
    add(body)
    add('</g>')


# --- background: comic sunburst from the blast, darker towards the edges -----------------
add('<defs>'
    '<radialGradient id="vig" cx="%d" cy="%d" r="760" gradientUnits="userSpaceOnUse">'
    '<stop offset="0" stop-color="#000" stop-opacity="0"/>'
    '<stop offset="0.55" stop-color="#000" stop-opacity="0.05"/>'
    '<stop offset="1" stop-color="#000" stop-opacity="0.75"/></radialGradient>'
    '<radialGradient id="wave" cx="%d" cy="%d" r="%d" gradientUnits="userSpaceOnUse">'
    '<stop offset="0.80" stop-color="#ffe9a0" stop-opacity="0"/>'
    '<stop offset="0.96" stop-color="#ffe9a0" stop-opacity="0.55"/>'
    '<stop offset="1" stop-color="#fffbe6" stop-opacity="0.95"/></radialGradient>'
    '</defs>' % (CX, CY, CX, CY, RING + 14))
add('<rect width="%d" height="%d" fill="#8e1c0c"/>' % (W, H))
rays = 28
for i in range(rays):
    if i % 2:
        continue
    a0 = 2 * math.pi * i / rays
    a1 = 2 * math.pi * (i + 1) / rays
    R = 1600
    add('<polygon points="%d,%d %.1f,%.1f %.1f,%.1f" fill="#b8300f"/>' % (
        CX, CY, CX + math.cos(a0) * R, CY + math.sin(a0) * R, CX + math.cos(a1) * R, CY + math.sin(a1) * R))
add('<rect width="%d" height="%d" fill="url(#vig)"/>' % (W, H))

# --- the shockwave: a glowing band with a hard white front -------------------------------
add('<circle cx="%d" cy="%d" r="%d" fill="url(#wave)"/>' % (CX, CY, RING + 14))
add('<circle cx="%d" cy="%d" r="%d" fill="none" stroke="%s" stroke-width="20"/>' % (CX, CY, RING + 6, INK))
add('<circle cx="%d" cy="%d" r="%d" fill="none" stroke="#fffbe6" stroke-width="10"/>' % (CX, CY, RING + 6))

# --- the explosion: smoke on top, then red, orange, yellow, a white-hot core -------------
cloud(blob(CX - 8, CY - 128, 100, 44, 9, 38, 52, 70), "#5b4a44")
cloud(blob(CX, CY - 4, 132, 116, 14, 46, 62, 135), "#e2401a")
cloud(blob(CX + 6, CY + 6, 88, 74, 12, 34, 46, 92), "#ff8a1c")
cloud(blob(CX + 2, CY + 14, 48, 40, 9, 24, 32, 54), "#ffd43a")
cloud(blob(CX + 4, CY + 18, 18, 14, 7, 14, 18, 26), "#fff8cf", outline=False)
# smoke highlights
for x, y, r in ((CX - 56, CY - 158, 17), (CX + 34, CY - 168, 14), (CX + 96, CY - 134, 11)):
    add('<circle cx="%d" cy="%d" r="%d" fill="#7a6a62"/>' % (x, y, r))

# streaks between the cloud and whatever is riding the wave
for d in (-150, -90, -35, 150, 25):
    speed_lines(d)

# --- what the wave is carrying -----------------------------------------------------------
# a wheel: tyre with tread, steel rim, hub and nuts
wheel = ['<circle r="62" fill="#26211f"/>']
if EXP:
    # a tractor's back tyre: chevron lugs, red rim
    for i in range(14):
        a = math.degrees(2 * math.pi * i / 14)
        wheel.append('<path d="M -14 -60 L 0 -74 L 14 -60 L 6 -60 L 0 -66 L -6 -60 Z" fill="#26211f" '
                     'stroke-width="4" transform="rotate(%.1f)"/>' % a)
    wheel.append('<circle r="38" fill="#c8352a"/>')
    wheel.append('<circle r="26" fill="#9a261d" stroke-width="4"/>')
else:
    for i in range(16):
        a = 2 * math.pi * i / 16
        wheel.append('<rect x="-7" y="-70" width="14" height="14" rx="3" fill="#26211f" '
                     'transform="rotate(%.1f)"/>' % math.degrees(a))
    wheel.append('<circle r="36" fill="#a9b0b4"/>')
    wheel.append('<circle r="25" fill="#7d868b" stroke-width="4"/>')
for i in range(5):
    a = 2 * math.pi * i / 5
    wheel.append('<circle cx="%.1f" cy="%.1f" r="4" fill="#3a3f42" stroke-width="2"/>' % (
        math.cos(a) * 15, math.sin(a) * 15))
wheel.append('<circle r="7" fill="#d6dadc" stroke-width="3"/>')
wheel.append('<path d="M -40 -30 A 52 52 0 0 1 -10 -50" fill="none" stroke="#5a5250" stroke-width="6"/>')
item(-150, 18, "".join(wheel), 0.95)

# a potted plant: terracotta pot, soil, leaves, one flower, dirt flying off
plant = (
    '<path d="M -10 20 C -60 -10 -70 -70 -40 -95 C -25 -60 -15 -30 -10 20 Z" fill="#3e9a3a"/>'
    '<path d="M 6 20 C 50 -15 75 -60 55 -100 C 35 -65 15 -30 6 20 Z" fill="#4fb043"/>'
    '<path d="M -2 20 C -12 -40 -6 -90 10 -125 C 20 -85 16 -40 -2 20 Z" fill="#5cc44d"/>'
    '<path d="M -40 -95 C -30 -60 -18 -30 -10 18" fill="none" stroke-width="3"/>'
    '<path d="M 55 -100 C 35 -60 16 -28 6 18" fill="none" stroke-width="3"/>'
    '<circle cx="10" cy="-128" r="13" fill="#ff5fa2"/><circle cx="10" cy="-128" r="5" fill="#ffe14a" stroke-width="3"/>'
    '<ellipse cx="0" cy="22" rx="52" ry="11" fill="#4a2c1a"/>'
    '<path d="M -50 22 L 50 22 L 38 100 L -38 100 Z" fill="#c8643b"/>'
    '<path d="M -58 10 L 58 10 L 58 34 L -58 34 Z" fill="#d97847"/>'
    '<path d="M -30 50 L -24 92" fill="none" stroke="#a24d2b" stroke-width="5"/>'
    '<circle cx="-70" cy="-10" r="6" fill="#4a2c1a" stroke-width="3"/>'
    '<circle cx="68" cy="-30" r="5" fill="#4a2c1a" stroke-width="3"/>'
    '<circle cx="-84" cy="18" r="4" fill="#4a2c1a" stroke-width="2"/>'
)
item(-90, -10, plant, 0.9, 10)

# a skull, grinning
skull = (
    '<path d="M -48 6 C -62 -48 -30 -78 0 -78 C 30 -78 62 -48 48 6 C 44 18 34 22 30 30 '
    'L 30 46 L -30 46 L -30 30 C -34 22 -44 18 -48 6 Z" fill="#f3ecd8"/>'
    '<ellipse cx="-20" cy="-14" rx="15" ry="17" fill="#1a0d08" stroke-width="2"/>'
    '<ellipse cx="20" cy="-14" rx="15" ry="17" fill="#1a0d08" stroke-width="2"/>'
    '<circle cx="-16" cy="-19" r="4" fill="#ff7a2a" stroke="none"/>'
    '<circle cx="24" cy="-19" r="4" fill="#ff7a2a" stroke="none"/>'
    '<path d="M 0 6 L -8 20 L 8 20 Z" fill="#1a0d08" stroke-width="3"/>'
    '<path d="M -18 30 L -18 46 M -6 30 L -6 46 M 6 30 L 6 46 M 18 30 L 18 46" fill="none" stroke-width="4"/>'
    '<path d="M -26 30 L 26 30" fill="none" stroke-width="4"/>'
    '<path d="M 30 -60 L 20 -44 L 30 -34" fill="none" stroke-width="4"/>'
)
item(-35, 22, skull, 1.1)

# a car door: green paint, cracked glass, the handle, a scorched edge
DOOR = "#5d6b35" if EXP else "#4c8a3d"   # UAZ olive / hatchback green
door = (
    '<path d="M -70 -80 L 40 -80 L 78 -20 L 78 90 L -70 90 Z" fill="%s"/>' % DOOR +
    '<path d="M -58 -68 L 34 -68 L 64 -24 L -58 -24 Z" fill="#9fd4e8"/>'
    '<path d="M -10 -68 L 0 -46 L -18 -36 L -6 -24 M 0 -46 L 22 -40" fill="none" stroke-width="4"/>'
    '<path d="M -70 -10 L 78 -10" fill="none" stroke-width="4"/>'
    '<rect x="30" y="4" width="34" height="11" rx="5" fill="#c9ced1" stroke-width="5"/>'
    '<path d="M -70 60 C -50 50 -40 75 -20 64 C 0 54 10 80 30 68 C 50 58 60 82 78 70 L 78 90 L -70 90 Z" '
    'fill="#2a1c14" stroke-width="5"/>'
)
item(150, -24, door, 0.95)

# a bonnet: red, dented, a crease down the middle
hood = (
    '<path d="M -90 -50 C -40 -66 40 -66 90 -50 L 76 60 C 30 70 -30 70 -76 60 Z" fill="#c8352a"/>'
    '<path d="M 0 -60 C 6 -20 -6 20 0 66" fill="none" stroke="#8e2219" stroke-width="5"/>'
    '<path d="M -40 -10 C -26 -22 -14 -6 -24 6" fill="none" stroke="#8e2219" stroke-width="5"/>'
    '<path d="M -76 60 C -30 70 30 70 76 60 L 70 46 C 30 54 -30 54 -70 46 Z" fill="#2a1c14" stroke-width="5"/>'
    '<path d="M -60 -40 C -30 -50 10 -50 30 -48" fill="none" stroke="#f08a7e" stroke-width="6"/>'
)
if EXP:
    # a Huey rotor blade, broken off at the root, yellow tip
    blade = (
        '<path d="M -150 -16 L 140 -16 C 158 -16 158 16 140 16 L -150 16 L -140 6 L -156 -2 L -142 -10 Z" fill="#4b5523"/>'
        '<path d="M 112 -16 L 132 -16 L 132 16 L 112 16 Z" fill="#f2c230" stroke-width="5"/>'
        '<path d="M -120 -6 L 100 -6" fill="none" stroke="#6f7c3a" stroke-width="5"/>'
        '<circle cx="-118" cy="0" r="5" fill="#2b2f1a" stroke-width="3"/>'
    )
    item(25, 70, blade, 0.95)
else:
    item(25, 28, hood, 0.95)

# bits and bolts in the wave
for d, s in ((-120, 1), (-62, 0.8), (-6, 1), (62, 0.9), (100, 1.1), (122, 0.8), (186, 0.9)):
    x, y = on_ring(d, rnd.uniform(-12, 18))
    add('<polygon points="%s" fill="#3b3330" stroke="%s" stroke-width="4" stroke-linejoin="round"/>' % (
        " ".join("%.1f,%.1f" % (x + math.cos(a) * r * s, y + math.sin(a) * r * s)
                 for a, r in ((0.2, 16), (1.5, 11), (2.6, 18), (3.9, 9), (5.0, 14))), INK))

# --- the name -----------------------------------------------------------------------------
def title(text, y, size, fill, rot):
    add('<text x="%d" y="%d" transform="rotate(%.1f %d %d)" text-anchor="middle" '
        'font-family="Bangers" font-size="%d" letter-spacing="4" fill="%s" stroke="%s" '
        'stroke-width="%d" stroke-linejoin="round" paint-order="stroke">%s</text>' % (
            W // 2, y, rot, W // 2, y, size, fill, INK, size // 6, text))
    # a drop shadow under the outline
if EXP:
    title("BETTER", 784, 100, "#ffe14a", -4)
    title("EXPLOSIONS", 894, 132, "#ff9a1f", -4)
    # the add-on's banner: a ribbon under the name
    add('<g transform="rotate(-4 512 944)">'
        '<path d="M 150 906 L 874 906 L 900 944 L 874 982 L 150 982 L 124 944 Z" fill="#4b5523" '
        'stroke="%s" stroke-width="8" stroke-linejoin="round"/>'
        '<text x="512" y="966" text-anchor="middle" font-family="Bangers" font-size="62" '
        'letter-spacing="5" fill="#f3ecd8" stroke="%s" stroke-width="9" stroke-linejoin="round" '
        'paint-order="stroke">+ EXPANSION VEHICLES</text></g>' % (INK, INK))
else:
    title("BETTER", 858, 120, "#ffe14a", -4)
    title("EXPLOSIONS", 992, 160, "#ff9a1f", -4)

svg = ('<svg xmlns="http://www.w3.org/2000/svg" width="%d" height="%d" viewBox="0 0 %d %d">' % (W, H, W, H)
       + "".join(parts) + "</svg>")
open(sys.argv[1], "w").write(svg)
