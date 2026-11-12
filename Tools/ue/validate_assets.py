"""Check assets under a content path against the jam art budgets (run by `jf validate [path]`).

Budgets come from Config/JamBudgets.json when present (agents tune it), else the defaults below.
Prints one `JF_VALIDATE {json}` line with per-asset facts and violations; raises when any violation
exists so the command fails.
"""
import json
import os
import re

import unreal

DEFAULTS = {
    "static_mesh_max_tris": 60000,         # LOD0, non-Nanite
    "static_mesh_max_size_cm": 5000,       # longest bounds edge; catches 100x scale imports
    "static_mesh_min_size_cm": 1,          # catches 0.01x scale imports
    "texture_max_size": 2048,
    "sound_max_seconds": 240,
    "require_collision": True,
    "name_prefixes": {"StaticMesh": "SM_", "SkeletalMesh": "SK_", "Texture2D": "T_", "Material": "M_",
                      "MaterialInstanceConstant": "MI_", "SoundWave": "S_", "SoundCue": "SC_",
                      "Blueprint": "BP_", "WidgetBlueprint": "WBP_", "NiagaraSystem": "NS_"},
    "enforce_names": False,                # imports keep source names; turn on when the art lead decides
}


def budgets():
    path = os.path.join(unreal.Paths.project_config_dir(), "JamBudgets.json")
    data = dict(DEFAULTS)
    if os.path.exists(path):
        with open(path, encoding="utf-8") as fh:
            data.update(json.load(fh))
    return data


def collision_count(mesh):
    body = mesh.get_editor_property("body_setup")
    if body is None:
        return 0
    geom = body.get_editor_property("agg_geom")
    return sum(len(geom.get_editor_property(k)) for k in
               ("box_elems", "sphere_elems", "sphyl_elems", "convex_elems", "tapered_capsule_elems"))


def check_static_mesh(mesh, b, facts, issues):
    tris = mesh.get_num_triangles(0)
    nanite = mesh.get_editor_property("nanite_settings").get_editor_property("enabled")
    ext = mesh.get_bounds().box_extent
    size = 2 * max(ext.x, ext.y, ext.z)
    cols = collision_count(mesh)
    facts.update(tris=tris, nanite=nanite, size_cm=round(size, 1), collision=cols,
                 lods=mesh.get_num_lods(), sections=mesh.get_num_sections(0))
    if not nanite and tris > b["static_mesh_max_tris"]:
        issues.append(f"{tris} tris > {b['static_mesh_max_tris']} (enable Nanite or decimate)")
    if size > b["static_mesh_max_size_cm"]:
        issues.append(f"size {size:.0f} cm > {b['static_mesh_max_size_cm']} (wrong import scale?)")
    if size < b["static_mesh_min_size_cm"]:
        issues.append(f"size {size:.2f} cm < {b['static_mesh_min_size_cm']} (wrong import scale?)")
    if b["require_collision"] and cols == 0:
        issues.append("no simple collision")
    for i, m in enumerate(mesh.get_editor_property("static_materials")):
        if m.get_editor_property("material_interface") is None:
            issues.append(f"material slot {i} is empty")


LINEAR = re.compile(r"(_|-)(n|nor|norm|normal|nrm|nor_gl|nor_dx|r|rough|roughness|ao|occlusion|arm|orm|"
                    r"mask|metal|metallic|metalness|height|disp|displacement|opacity)(_|-|\.|$)", re.I)
NORMAL = re.compile(r"(_|-)(n|nor|norm|normal|nrm|nor_gl|nor_dx)(_|-|\.|$)", re.I)


def check_texture(tex, b, facts, issues):
    w, h = tex.blueprint_get_size_x(), tex.blueprint_get_size_y()
    srgb = tex.get_editor_property("srgb")
    comp = tex.get_editor_property("compression_settings").name
    facts.update(size=f"{w}x{h}", srgb=srgb, compression=comp)
    name = tex.get_name()
    if NORMAL.search(name) and comp != "TC_NORMALMAP":
        issues.append(f"normal map with {comp} compression (want TC_NORMALMAP)")
    elif LINEAR.search(name) and srgb:
        issues.append("linear data map (roughness/AO/metal/mask) has sRGB on")
    if max(w, h) > b["texture_max_size"]:
        issues.append(f"{w}x{h} > {b['texture_max_size']} (set max texture size or downscale)")
    if w & (w - 1) or h & (h - 1):
        issues.append(f"{w}x{h} is not power of two (no mips/streaming)")


def check_sound(snd, b, facts, issues):
    dur = snd.get_editor_property("duration")
    facts.update(seconds=round(dur, 2))
    if dur > b["sound_max_seconds"]:
        issues.append(f"{dur:.0f} s > {b['sound_max_seconds']} s")


def main():
    args = json.loads(os.environ.get("JF_PY_ARGS", "[]"))
    root = (args[0] if args else "/Game").rstrip("/") or "/Game"
    b = budgets()
    out, bad = [], 0
    for path in unreal.EditorAssetLibrary.list_assets(root, recursive=True, include_folder=False):
        data = unreal.EditorAssetLibrary.find_asset_data(path)
        cls = str(data.asset_class_path.asset_name)
        if cls not in ("StaticMesh", "Texture2D", "SoundWave") and not b["enforce_names"]:
            continue
        name = str(data.asset_name)
        facts, issues = {"asset": str(data.package_name), "class": cls}, []
        if cls in ("StaticMesh", "Texture2D", "SoundWave"):
            obj = unreal.load_asset(str(data.package_name))
            {"StaticMesh": check_static_mesh, "Texture2D": check_texture, "SoundWave": check_sound}[cls](
                obj, b, facts, issues)
        prefix = b["name_prefixes"].get(cls)
        if b["enforce_names"] and prefix and not name.startswith(prefix):
            issues.append(f"name should start with {prefix}")
        if re.search(r"\s", name):
            issues.append("whitespace in asset name")
        facts["issues"] = issues
        bad += bool(issues)
        out.append(facts)
    print("JF_VALIDATE " + json.dumps({"root": root, "checked": len(out), "with_issues": bad, "assets": out}))
    for a in out:
        for i in a["issues"]:
            print(f"JF_ISSUE {a['asset']}: {i}")
    if bad:
        raise RuntimeError(f"{bad} of {len(out)} assets under {root} violate budgets (JF_ISSUE lines above)")


main()
