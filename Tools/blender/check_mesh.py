"""Check source meshes before they go to Unreal. Run through `jf meshcheck <files...>` (background Blender).

Per mesh object: non-manifold edges (more than two faces), degenerate faces and zero-length edges, flipped
neighbour normals, inward normals on closed meshes, negative or unapplied scale, missing UVs, ngons, missing
materials, triangle budget, real-world size and pivot placement. UCX_ collision hulls are skipped.
Exit code 1 when any object FAILs; warnings do not fail. Extend the checks here when a new defect class
shows up in the game.
"""
import argparse
import sys
from pathlib import Path

import bmesh
import bpy
from mathutils import Vector

AREA_EPS = 1e-10        # m^2
LEN_EPS = 1e-6          # m
SIZE_MIN_M, SIZE_MAX_M = 0.01, 200.0


def parse_args():
    argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    p = argparse.ArgumentParser(prog="check_mesh.py")
    p.add_argument("--max-tris", type=int, default=60000)
    p.add_argument("files", nargs="+")
    return p.parse_args(argv)


def load(path: Path) -> None:
    ext = path.suffix.lower()
    if ext == ".blend":
        bpy.ops.wm.open_mainfile(filepath=str(path))
        return
    bpy.ops.wm.read_factory_settings(use_empty=True)
    if ext == ".fbx":
        bpy.ops.import_scene.fbx(filepath=str(path))
    elif ext in (".glb", ".gltf"):
        bpy.ops.import_scene.gltf(filepath=str(path))
    elif ext == ".obj":
        bpy.ops.wm.obj_import(filepath=str(path))
    else:
        raise ValueError(f"unsupported mesh format {ext} (fbx, glb, gltf, obj, blend)")


def check_object(obj, max_tris: int) -> tuple[list[str], list[str], str]:
    errors, warnings = [], []
    me = obj.data
    bm = bmesh.new()
    bm.from_mesh(me)
    bm.edges.ensure_lookup_table()
    tris = sum(len(f.verts) - 2 for f in bm.faces)

    multi = sum(1 for e in bm.edges if len(e.link_faces) > 2)
    if multi:
        errors.append(f"{multi} non-manifold edges shared by more than two faces")
    wire = sum(1 for e in bm.edges if not e.link_faces)
    if wire:
        warnings.append(f"{wire} loose edges")
    loose = sum(1 for v in bm.verts if not v.link_edges)
    if loose:
        warnings.append(f"{loose} loose vertices")
    degen = sum(1 for f in bm.faces if f.calc_area() < AREA_EPS)
    if degen:
        errors.append(f"{degen} zero-area faces")
    short = sum(1 for e in bm.edges if e.calc_length() < LEN_EPS)
    if short:
        errors.append(f"{short} zero-length edges")
    flipped = 0
    for e in bm.edges:
        if len(e.link_loops) == 2 and e.link_loops[0].vert == e.link_loops[1].vert:
            flipped += 1
    if flipped:
        errors.append(f"{flipped} edges between faces with opposite winding (flipped normals)")
    closed = bool(bm.edges) and all(len(e.link_faces) == 2 for e in bm.edges)
    if closed and not flipped and not multi and bm.calc_volume(signed=True) < 0:
        errors.append("closed mesh with inward-facing normals")
    ngons = sum(1 for f in bm.faces if len(f.verts) > 4)
    if ngons:
        warnings.append(f"{ngons} ngons (Unreal triangulates them unpredictably)")
    bm.free()

    if any(s < 0 for s in obj.scale):
        errors.append(f"negative scale {tuple(round(s, 3) for s in obj.scale)} (mirrors normals in Unreal)")
    elif any(abs(s - 1.0) > 1e-4 for s in obj.scale):
        warnings.append(f"unapplied scale {tuple(round(s, 3) for s in obj.scale)}")
    if not me.uv_layers:
        errors.append("no UV map")
    if not any(slot.material for slot in obj.material_slots):
        warnings.append("no material assigned")
    if tris > max_tris:
        warnings.append(f"{tris} triangles over the {max_tris} budget")

    corners = [obj.matrix_world @ Vector(c) for c in obj.bound_box]
    lo = Vector((min(c.x for c in corners), min(c.y for c in corners), min(c.z for c in corners)))
    hi = Vector((max(c.x for c in corners), max(c.y for c in corners), max(c.z for c in corners)))
    dims = hi - lo
    size = max(dims)
    if me.polygons and not SIZE_MIN_M <= size <= SIZE_MAX_M:
        warnings.append(f"largest dimension {size:.3f} m (unit or scale mistake?)")
    origin = obj.matrix_world.translation
    height = max(dims.z, 1e-6)
    if me.polygons and abs(origin.z - lo.z) > 0.05 * height:
        warnings.append(f"pivot {origin.z - lo.z:.3f} m above the base (props stand on their pivot)")
    facts = f"tris={tris} size={dims.x:.2f}x{dims.y:.2f}x{dims.z:.2f}m"
    return errors, warnings, facts


def main() -> int:
    a = parse_args()
    failed = 0
    total = 0
    for f in a.files:
        path = Path(f)
        try:
            load(path)
        except Exception as e:  # import errors are a result for this file, not a crash of the batch
            print(f"FAIL {path.name}: cannot load: {e}")
            failed += 1
            continue
        meshes = [o for o in bpy.context.scene.objects if o.type == "MESH" and not o.name.upper().startswith("UCX_")]
        if not meshes:
            print(f"FAIL {path.name}: no mesh objects")
            failed += 1
            continue
        for obj in meshes:
            total += 1
            errors, warnings, facts = check_object(obj, a.max_tris)
            status = "FAIL" if errors else ("WARN" if warnings else "OK")
            notes = "; ".join(errors + warnings)
            print(f"{status} {path.name}:{obj.name} {facts}" + (f" | {notes}" if notes else ""))
            failed += bool(errors)
    print(f"MESHCHECK {total} objects, {failed} failing")
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
