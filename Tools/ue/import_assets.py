"""Import source files into /Game (run by `jf import`; arguments arrive as JSON in JF_PY_ARGS).

Args: --dest /Game/Path [--kind mesh|skeletal|texture|audio|font] file...
Meshes (fbx/glb/gltf/obj) go through the engine's default import pipeline; textures get compression
and sRGB from their name suffix (normal / roughness / AO / ARM / mask maps are linear). Static mesh
collision comes from the import pipeline (UCX_ meshes, else a generated convex hull). Prints one `JF_IMPORT {json}` line; raises on any failed file.
"""
import json
import os
import re

import unreal

LINEAR = re.compile(r"(_|-)(n|nor|norm|normal|nrm|nor_gl|nor_dx|r|rough|roughness|ao|occlusion|arm|orm|"
                    r"mask|metal|metallic|metalness|height|disp|displacement|opacity)(_|-|\.|$)", re.I)
NORMAL = re.compile(r"(_|-)(n|nor|norm|normal|nrm|nor_gl|nor_dx)(_|-|\.|$)", re.I)


def parse(argv):
    dest, kind, files, i = "", "", [], 0
    while i < len(argv):
        if argv[i] == "--dest":
            dest, i = argv[i + 1], i + 2
        elif argv[i] == "--kind":
            kind, i = argv[i + 1], i + 2
        else:
            files.append(argv[i])
            i += 1
    if not dest.startswith("/Game"):
        raise RuntimeError(f"--dest must start with /Game, got {dest!r}")
    if not files:
        raise RuntimeError("no files to import")
    return dest.rstrip("/"), kind, files


def tune_texture(tex, source_name):
    """Linear maps by name suffix; base color keeps sRGB + default compression."""
    if NORMAL.search(source_name):
        tex.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_NORMALMAP)
        tex.set_editor_property("srgb", False)
    elif LINEAR.search(source_name):
        tex.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_MASKS)
        tex.set_editor_property("srgb", False)


def main():
    dest, kind, files = parse(json.loads(os.environ.get("JF_PY_ARGS", "[]")))
    tasks = []
    for f in files:
        t = unreal.AssetImportTask()
        t.set_editor_property("filename", f)
        t.set_editor_property("destination_path", dest)
        t.set_editor_property("automated", True)
        t.set_editor_property("replace_existing", True)
        t.set_editor_property("save", False)
        tasks.append(t)
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)

    report, failed = [], []
    for f, t in zip(files, tasks):
        paths = [str(p) for p in t.get_editor_property("imported_object_paths")]
        if not paths:
            failed.append(f)
            continue
        for p in paths:
            obj = unreal.load_asset(p)
            if obj is None:
                failed.append(f"{f} -> {p} (not loadable)")
                continue
            if isinstance(obj, unreal.Texture2D):
                tune_texture(obj, os.path.basename(f))
                continue  # saved with the folder sweep below
            elif isinstance(obj, unreal.StaticMesh) and kind == "skeletal":
                failed.append(f"{f} imported as StaticMesh but --kind skeletal (no skin/skeleton in file?)")
            unreal.EditorAssetLibrary.save_loaded_asset(obj, False)
            report.append({"file": f, "asset": p.split(".")[0], "class": obj.get_class().get_name()})
    # Mesh imports bring their own textures (not listed in imported_object_paths): tune every texture
    # in the destination by asset name so linear maps never stay sRGB.
    for path in unreal.EditorAssetLibrary.list_assets(dest, recursive=True, include_folder=False):
        data = unreal.EditorAssetLibrary.find_asset_data(path)
        if str(data.asset_class_path.asset_name) != "Texture2D":
            continue
        tex = unreal.load_asset(str(data.package_name))
        tune_texture(tex, str(data.asset_name))
        unreal.EditorAssetLibrary.save_loaded_asset(tex, False)
        if not any(r["asset"] == str(data.package_name) for r in report):
            report.append({"file": "(from mesh)", "asset": str(data.package_name), "class": "Texture2D"})
    print("JF_IMPORT " + json.dumps({"dest": dest, "assets": report, "failed": failed}))
    if failed:
        raise RuntimeError("import failed for: " + "; ".join(failed))


main()
