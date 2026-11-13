"""Procedurally builds the burnt remains a vehicle leaves when it goes up, which vanilla has
no model for: the inflatable boat's hull (its tubes burst, charred and slumped flat, the
floorboards burnt through) and its outboard motor, both sinking to the bottom
(BetterExplosionsDebris.Sink), and a helicopter's main rotor blade snapped off near the hub
(flung off an Expansion UH-1H by BetterExplosions_Expansion).

Run headless (from the project root):
    blender --background --python blender/build_debris.py

Outputs:
    BetterExplosions/data/<name>.p3d  - MLOD P3D each (needs the Arma 3 Object Builder add-on)
    BetterExplosions/data/model.cfg
    models/debris.blend               - source scene
    models/debris_preview.png         - all of them side by side, textured if build/vanilla_png
                                        holds log01burned_co.png

Only vanilla textures, by their in-game paths: charred is the burnt fireplace log
(dz\\gear\\cooking\\data\\log01burned, its no-glow material). These came from the Molotov
mod's ruin builder (blender/build_ruins.py there); each is seeded with its name, so they
come out as they did there.
Axes: Blender X/Y/Z = game X/Z/Y (Z up here, Y up in game).
"""

import math
import os
import random

import addon_utils
import bmesh
import bpy
from mathutils import Euler, Matrix, Vector

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT_DIR = os.path.join(ROOT, "models")
DATA_DIR = os.path.join(ROOT, "BetterExplosions", "data")
PREVIEW_TEX = os.path.join(ROOT, "build", "vanilla_png")

CHAR_TEX = ("dz\\gear\\cooking\\data\\log01burned_co.paa", "dz\\gear\\cooking\\data\\log01burnednoemit.rvmat")
ASH_TEX = ("dz\\gear\\cooking\\data\\stoneground_co.paa", "dz\\gear\\cooking\\data\\stonegroundnoemit.rvmat")
CHAR_U = 0.82          # the bark runs along u 0..0.82 of the log texture; end grain beyond

A3OB_MODULE = "bl_ext.user_default.Arma3ObjectBuilder"
LOD_VISUAL, LOD_GEOMETRY, LOD_FIRE_GEOMETRY = "0", "6", "15"
MASS_KG = 2000

# name: (builder, size, size). The boat's sizes are from its collision box.
DEBRIS = {
    "boat_hull": ("hull", 4.4, 1.9),      # length, width
    "boat_engine": ("engine", 0.45, 1.25),  # head width, overall height
    "rotor_blade": ("blade", 4.6, 0.53),  # snapped-off length, chord (a UH-1H blade is 7.3 m x 0.53 m)
}


class Mesh:
    """Boxes and grids in one bmesh, UV'd per face corner, a material per face. Boxes
    also go to a list of convex collision components when asked."""

    def __init__(self):
        self.bm = bmesh.new()
        self.uv = self.bm.loops.layers.uv.new("UVMap")

    def quad(self, verts, uvs, mat):
        f = self.bm.faces.new(verts)
        f.material_index = mat
        for loop, uv in zip(f.loops, uvs):
            loop[self.uv].uv = uv
        return f

    def box(self, centre, size, rot, mat=0, u0=None):
        """A charred plank/post/beam: size (x, y, z), rot Euler; bark along its longest side"""
        sx, sy, sz = size
        m = Matrix.Translation(centre) @ rot.to_matrix().to_4x4()
        corners = [m @ Vector((x * sx / 2, y * sy / 2, z * sz / 2))
                   for x in (-1, 1) for y in (-1, 1) for z in (-1, 1)]
        v = [self.bm.verts.new(c) for c in corners]
        if u0 is None:
            u0 = random.uniform(0, CHAR_U * 0.5)
        length = max(size)
        u1 = min(CHAR_U, u0 + length * 0.35)
        vv = random.uniform(0, 0.7)
        uvs = ((u0, vv), (u1, vv), (u1, vv + 0.25), (u0, vv + 0.25))
        idx = (0, 1, 2, 3, 4, 5, 6, 7)
        for face in ((0, 1, 3, 2), (4, 6, 7, 5), (0, 4, 5, 1), (2, 3, 7, 6), (0, 2, 6, 4), (1, 5, 7, 3)):
            self.quad([v[i] for i in face], uvs, mat)
        return corners

    def to_object(self, name, materials):
        # P3D faces are triangles or quads only: an end cap with more corners would have
        # the exporter drop the whole visual LOD (the tree stumps came out empty)
        ngons = [f for f in self.bm.faces if len(f.verts) > 4]
        if ngons:
            bmesh.ops.triangulate(self.bm, faces=ngons)
        bmesh.ops.recalc_face_normals(self.bm, faces=self.bm.faces[:])
        # the ash bed is an open sheet: make sure it faces up
        flip = [f for f in self.bm.faces if f.material_index == 1 and f.normal.z < 0]
        if flip:
            bmesh.ops.reverse_faces(self.bm, faces=flip)
        # every closed part must face out: its signed volume is positive
        inward = 0
        for island in islands(self.bm):
            if any(e.is_boundary for f in island for e in f.edges):
                continue
            vol = sum(f.calc_center_median().dot(f.normal) * f.calc_area() for f in island) / 3
            if vol < 0:
                bmesh.ops.reverse_faces(self.bm, faces=island)
                inward += 1
        if inward:
            print(f"[ruins] {name}: turned {inward} inside-out parts the right way out")
        mesh = bpy.data.meshes.new(name)
        self.bm.to_mesh(mesh)
        self.bm.free()
        obj = bpy.data.objects.new(name, mesh)
        for mat in materials:
            mesh.materials.append(mat)
        return obj


def islands(bm):
    """The connected parts of a mesh, as lists of faces"""
    seen = set()
    parts = []
    for f in bm.faces:
        if f.index in seen:
            continue
        stack, part = [f], []
        seen.add(f.index)
        while stack:
            g = stack.pop()
            part.append(g)
            for e in g.edges:
                for h in e.link_faces:
                    if h.index not in seen:
                        seen.add(h.index)
                        stack.append(h)
        parts.append(part)
    return parts


def slumped_tube(mesh, path, r, sag, sides=8):
    """A burst inflatable tube along path: its round section squashed flat and buckled,
    crumpled more the further from where it was still holding air"""
    rings = []
    for i, (p, d) in enumerate(path):
        d = d.normalized()
        side = d.cross(Vector((0, 0, 1))).normalized()
        flat = 0.35 + 0.25 * random.random() * sag[i]   # how much of its height is left
        spread = 1.35 + 0.3 * random.random()           # and it spreads out as it slumps
        ring = []
        for k in range(sides):
            a = 2 * math.pi * k / sides
            wob = 1 + random.uniform(-0.18, 0.18)
            z = r * flat * (1 + math.sin(a)) * wob
            q = p + side * math.cos(a) * r * spread * wob + Vector((0, 0, z))
            ring.append(mesh.bm.verts.new(q))
        rings.append(ring)
    n = len(rings)
    for i in range(n - 1):
        for k in range(sides):
            k2 = (k + 1) % sides
            u0, u1 = CHAR_U * i / n, CHAR_U * (i + 1) / n
            v0, v1 = k / sides, (k + 1) / sides
            mesh.quad((rings[i][k], rings[i][k2], rings[i + 1][k2], rings[i + 1][k]),
                      ((u0, v0), (u0, v1), (u1, v1), (u1, v0)), 0).smooth = True
    ends = [(0.9 + 0.06 * math.cos(2 * math.pi * k / sides), 0.5 + 0.12 * math.sin(2 * math.pi * k / sides)) for k in range(sides)]
    mesh.quad(list(reversed(rings[-1])), ends, 0)
    mesh.quad(list(rings[0]), ends, 0)


def build_boat_hull(length, width):
    """The inflatable burnt out: its U of tubes collapsed flat and buckled, the bow
    slumped, the transom board and a few floorboards charred and askew"""
    vis, geo = Mesh(), []
    r = 0.24                                # the tube, inflated
    half = width / 2 - r
    bow_r = half
    straight = length - r * 2 - bow_r       # stern to where the bow curve starts
    y0 = -length / 2 + r
    path = []
    # starboard tube stern to bow, round the bow, port tube back to the stern
    steps = 9
    for i in range(steps + 1):
        y = y0 + straight * i / steps
        path.append((Vector((half + random.uniform(-0.05, 0.05), y, 0)), Vector((0, 1, 0))))
    for i in range(1, 8):
        a = math.pi * i / 8
        c = Vector((0, y0 + straight, 0))
        path.append((c + Vector((math.cos(a) * bow_r, math.sin(a) * bow_r, 0)), Vector((-math.sin(a), math.cos(a), 0))))
    for i in range(steps + 1):
        y = y0 + straight * (1 - i / steps)
        path.append((Vector((-half + random.uniform(-0.05, 0.05), y, 0)), Vector((0, -1, 0))))
    # where it burst: the tubes held up a little near the stern, flat at the bow
    sag = [max(0.0, 1 - abs(i - len(path) / 2) / (len(path) / 2)) * 0.3 + random.random() * 0.4 for i in range(len(path))]
    slumped_tube(vis, path, r, sag)
    # the transom: a charred board, fallen back
    vis.box(Vector((0, -length / 2 + 0.12, 0.22)), (width - 0.5, 0.06, 0.5), Euler((math.radians(-35), 0, random.uniform(-0.1, 0.1))))
    # floorboards burnt through: a few left, warped
    for k in range(4):
        y = -length / 2 + 0.5 + k * 0.7 + random.uniform(-0.1, 0.1)
        if random.random() < 0.25:
            continue
        vis.box(Vector((random.uniform(-0.08, 0.08), y, 0.05)), (width - 0.8 - random.uniform(0, 0.3), 0.55, 0.03),
                Euler((random.uniform(-0.08, 0.08), random.uniform(-0.12, 0.12), random.uniform(-0.15, 0.15))))
    geo.append((Vector((0, 0, 0.12)), (width, length, 0.24), Euler((0, 0, 0))))
    return vis, geo


def build_boat_engine(head, height):
    """A charred outboard motor: the cowled head, the leg, the gearbox and a bent propeller.
    Its origin is at its middle - it lies on its side in the end, and flies off spinning"""
    vis, geo = Mesh(), []
    z0 = -height / 2
    leg = height * 0.5
    # the propeller and gearbox at the bottom, the leg, the head on top
    vis.box(Vector((0, 0.02, z0 + 0.1)), (0.09, 0.32, 0.12), Euler((0, 0, 0)))                     # gearbox
    vis.box(Vector((0, 0.04, z0 + 0.22)), (0.3, 0.36, 0.02), Euler((0, 0, 0)))                     # cavitation plate
    for k in range(3):                                                                              # blades, one bent
        a = 2 * math.pi * k / 3
        bend = 0.5 if k == 0 else 0
        vis.box(Vector((math.cos(a) * 0.09, -0.16, z0 + 0.1 + math.sin(a) * 0.09)), (0.16, 0.012, 0.07),
                Euler((0, a + bend, math.radians(25))))
    vis.box(Vector((0, 0.02, z0 + 0.2 + leg / 2)), (0.1, 0.16, leg), Euler((0, 0, 0)))              # leg
    hz = z0 + 0.2 + leg
    hh = height - 0.2 - leg
    vis.box(Vector((0, 0.04, hz + hh * 0.3)), (head * 0.85, head * 1.1, hh * 0.6), Euler((0, 0, 0)))   # powerhead
    vis.box(Vector((0, 0.06, hz + hh * 0.75)), (head, head * 1.25, hh * 0.45), Euler((random.uniform(-0.08, 0.08), 0, 0)))  # cowling, cracked loose
    vis.box(Vector((0, 0.34, hz + hh * 0.2)), (0.05, 0.4, 0.05), Euler((0, 0, math.radians(8))))      # tiller stub
    vis.box(Vector((0, -0.12, hz - 0.06)), (0.26, 0.1, 0.22), Euler((0, 0, 0)))                     # transom clamp
    geo.append((Vector((0, 0.04, hz + hh * 0.5)), (head, head * 1.25, hh), Euler((0, 0, 0))))
    geo.append((Vector((0, 0.02, z0 + (0.2 + leg) / 2)), (0.3, 0.36, 0.2 + leg), Euler((0, 0, 0))))
    return vis, geo


def build_rotor_blade(length, chord):
    """A helicopter's main rotor blade snapped off near the hub: a long flat aerofoil, bent up
    and twisted along its length, the broken root end ragged. Charred. Lies flat, its origin
    at its middle (flung off a burning Expansion UH-1H, BetterExplosions_Expansion)"""
    vis, geo = Mesh(), []
    segs = 12
    seg = length / segs
    thick = chord * 0.1
    for i in range(segs):
        t = (i + 0.5) / segs
        y = -length / 2 + seg * (i + 0.5)
        lift = 0.35 * t * t                      # bent up towards the tip
        twist = math.radians(14 * t) + random.uniform(-0.03, 0.03)
        w = chord * (1.0 if i > 0 else 0.7)     # the root end torn narrower
        vis.box(Vector((random.uniform(-0.01, 0.01), y, thick / 2 + lift)), (w, seg * 1.02, thick),
                Euler((math.atan2(0.7 * t / segs, 1), twist, 0)))
    # the spar's stub sticking out of the broken root, and the tip cap
    vis.box(Vector((0.05, -length / 2 - 0.12, thick / 2)), (0.08, 0.3, 0.05), Euler((0, 0, math.radians(10))))
    vis.box(Vector((0, length / 2 - 0.03, thick / 2 + 0.35)), (chord * 0.9, 0.06, thick * 1.4), Euler((0, math.radians(14), 0)))
    geo.append((Vector((0, 0, 0.12)), (chord, length, 0.24 + 0.35), Euler((0, 0, 0))))
    return vis, geo


def geometry_object(name, parts):
    """Convex collision components from (centre, size, rot) boxes"""
    bm = bmesh.new()
    groups = []
    for c, (sx, sy, sz), rot in parts:
        m = Matrix.Translation(c) @ rot.to_matrix().to_4x4() @ Matrix.Diagonal((sx, sy, sz, 1))
        res = bmesh.ops.create_cube(bm, size=1.0, matrix=m)
        groups.append(res["verts"])
    bmesh.ops.triangulate(bm, faces=bm.faces[:])
    bm.verts.index_update()
    idx = [[v.index for v in g] for g in groups]
    mesh = bpy.data.meshes.new(name)
    bm.to_mesh(mesh)
    bm.free()
    obj = bpy.data.objects.new(name, mesh)
    for i, g in enumerate(idx):
        obj.vertex_groups.new(name=f"Component{i + 1:02d}").add(g, 1.0, "REPLACE")
    return obj


def set_lod(obj, lod, resolution=0, properties=()):
    props = obj.a3ob_properties_object
    props.is_a3_lod = True
    props.lod = lod
    props.resolution = resolution
    for name, value in properties:
        prop = props.properties.add()
        prop.name = name
        prop.value = value


def set_uniform_mass(obj, total):
    bm = bmesh.new()
    bm.from_mesh(obj.data)
    layer = bm.verts.layers.float.new("a3ob_mass")
    for v in bm.verts:
        v[layer] = total / len(bm.verts)
    bm.to_mesh(obj.data)
    bm.free()


def make_material(name, color, tex, preview_png):
    mat = bpy.data.materials.new(name)
    mat.use_nodes = True
    nodes = mat.node_tree.nodes
    bsdf = nodes["Principled BSDF"]
    bsdf.inputs["Base Color"].default_value = (*color, 1.0)
    bsdf.inputs["Roughness"].default_value = 0.9
    png = os.path.join(PREVIEW_TEX, preview_png)
    if os.path.exists(png):
        img = nodes.new("ShaderNodeTexImage")
        img.image = bpy.data.images.load(png)
        mat.node_tree.links.new(img.outputs["Color"], bsdf.inputs["Base Color"])
        mat.node_tree.links.new(img.outputs["Alpha"], bsdf.inputs["Alpha"])
        mat.blend_method = "HASHED"
    props = mat.a3ob_properties_material
    props.texture_type = "CUSTOM"
    props.color_raw = tex[0]
    props.material_path = tex[1]
    return mat


def reset_scene():
    for coll in (bpy.data.objects, bpy.data.meshes, bpy.data.materials, bpy.data.images,
                 bpy.data.cameras, bpy.data.lights, bpy.data.collections):
        for block in list(coll):
            coll.remove(block)


def render_preview(objects, path):
    """Every ruin in a row, seen from above at an angle"""
    scene = bpy.context.scene
    scene.render.engine = "BLENDER_EEVEE_NEXT"
    scene.render.resolution_x, scene.render.resolution_y = 1800, 700
    scene.world = bpy.data.worlds.new("sky")
    scene.world.color = (0.55, 0.6, 0.65)
    ground = bpy.data.meshes.new("ground")
    ground.from_pydata([(-200, -200, -0.01), (200, -200, -0.01), (200, 200, -0.01), (-200, 200, -0.01)], [], [(0, 1, 2, 3)])
    g = bpy.data.objects.new("ground", ground)
    gm = bpy.data.materials.new("sand")
    gm.use_nodes = True
    gm.node_tree.nodes["Principled BSDF"].inputs["Base Color"].default_value = (0.55, 0.48, 0.36, 1)
    ground.materials.append(gm)
    scene.collection.objects.link(g)
    x = 0.0
    for o in objects:
        w = o.dimensions.x
        o.location.x = x + w / 2
        x += w + 3
    sun = bpy.data.objects.new("sun", bpy.data.lights.new("sun", "SUN"))
    sun.data.energy = 4
    sun.rotation_euler = (0.7, 0.2, 0.6)
    scene.collection.objects.link(sun)
    cam = bpy.data.objects.new("cam", bpy.data.cameras.new("cam"))
    scene.collection.objects.link(cam)
    scene.camera = cam
    cam.data.type = "ORTHO"
    cam.data.ortho_scale = x + 2
    cam.location = (x / 2, -40, 30)
    cam.rotation_euler = (math.radians(55), 0, 0)
    scene.render.filepath = path
    bpy.ops.render.render(write_still=True)


def main():
    addon_utils.enable(A3OB_MODULE, default_set=False)
    if not hasattr(bpy.ops.a3ob, "export_p3d"):
        raise RuntimeError(f"Arma 3 Object Builder add-on ({A3OB_MODULE}) is not installed")
    reset_scene()
    os.makedirs(OUT_DIR, exist_ok=True)
    os.makedirs(DATA_DIR, exist_ok=True)
    char = make_material("charred", (0.08, 0.07, 0.06), CHAR_TEX, "log01burned_co.png")
    ash = make_material("ash", (0.45, 0.45, 0.45), ASH_TEX, "stoneground_co.png")
    builders = {"hull": build_boat_hull, "engine": build_boat_engine, "blade": build_rotor_blade}

    cfg = ["class CfgSkeletons", "{", "\tclass Default", "\t{", "\t\tisDiscrete = 1;",
           "\t\tskeletonInherit = \"\";", "\t\tskeletonBones[] = {};", "\t};", "};",
           "class CfgModels", "{", "\tclass Default", "\t{", "\t\tsectionsInherit = \"\";",
           "\t\tsections[] = {};", "\t\tskeletonName = \"\";", "\t};"]
    visuals = []
    for name, (kind, w, d) in DEBRIS.items():
        random.seed(name)   # as in Molotov's builder: keeps each as it was
        vis_mesh, parts = builders[kind](w, d)
        coll = bpy.data.collections.new(name)
        bpy.context.scene.collection.children.link(coll)
        visual = vis_mesh.to_object(f"{name}_LOD0", [char, ash])
        set_lod(visual, LOD_VISUAL, resolution=1)
        geometry = geometry_object(f"{name}_Geometry", parts)
        set_uniform_mass(geometry, MASS_KG)
        set_lod(geometry, LOD_GEOMETRY, properties=(("autocenter", "0"),))
        fire = geometry_object(f"{name}_FireGeometry", parts)
        set_lod(fire, LOD_FIRE_GEOMETRY)
        for o in (visual, geometry, fire):
            coll.objects.link(o)
        geometry.display_type = fire.display_type = "WIRE"
        for o in bpy.context.scene.objects:
            o.select_set(o in (visual, geometry, fire))
        bpy.ops.a3ob.export_p3d(
            filepath=os.path.join(DATA_DIR, name + ".p3d"),
            use_selection=True,
            validate_lods=True,
            validate_lods_warning_errors=False,
            lod_collisions="FAIL",
        )
        tris = sum(len(p.vertices) - 2 for p in visual.data.polygons)
        print(f"[debris] {name}: {w} x {d} m, {tris} tris, {len(parts)} collision parts")
        cfg.append(f"\tclass {name}: Default {{}};")
        geometry.hide_render = fire.hide_render = True
        visuals.append(visual)
    cfg.append("};")
    with open(os.path.join(DATA_DIR, "model.cfg"), "w") as f:
        f.write("\n".join(cfg) + "\n")

    bpy.context.preferences.filepaths.save_version = 0
    bpy.ops.wm.save_as_mainfile(filepath=os.path.join(OUT_DIR, "debris.blend"))
    render_preview(visuals, os.path.join(OUT_DIR, "debris_preview.png"))


main()
