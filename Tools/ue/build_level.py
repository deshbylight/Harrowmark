"""Build or rebuild a level from a JSON spec (run by `jf level <spec.json>`).

Idempotent: actors this script spawns carry the tag JF_Gen; a rebuild deletes them and spawns the
spec again, while hand-placed actors (no tag) stay. Keep specs in the repo (e.g. Levels/<name>.json).

Spec (every key except "map" optional; rotations are [pitch, yaw, roll] in degrees, units in cm):
{
  "map": "/Game/Maps/L_Arena",
  "partitioned": false,
  "environment": {"sun": {"rot": [-45, 30, 0], "intensity": 8, "color": [1, 0.95, 0.9]},
                  "sky": true, "fog": {"density": 0.02}, "clouds": false, "exposure": 0.0},
  (exposure = auto-exposure bias in stops; colors: FColor props take 0-255, LinearColor props 0-1)
  "game_mode": "/Script/MyGame.MyGameMode",
  "player_start": {"loc": [0, 0, 120], "rot": [0, 0, 0]},
  "actors": [
    {"mesh": "/Engine/BasicShapes/Cube", "loc": [0, 0, -50], "scale": [40, 40, 1],
     "materials": ["/Game/Art/Materials/MI_Floor"], "label": "Floor"},
    {"mesh": "/Game/Art/Props/SM_Barrel", "loc": [300, 0, 0], "rot": [0, 45, 0],
     "grid": {"count": [3, 2], "step": [150, 150]}, "random_yaw": true},
    {"class": "/Script/Engine.PointLight", "loc": [0, 0, 300],
     "props": {"point_light_component.intensity": 5000, "point_light_component.light_color": [255, 200, 150, 255]}},
    {"class": "/Game/Blueprints/BP_Enemy.BP_Enemy_C", "loc": [800, 0, 100], "tags": ["Enemy"]}
  ]
}
"""
import json
import os
import random

import unreal

GEN_TAG = "JF_Gen"


def vec(v, default=(0, 0, 0)):
    v = v if v is not None else default
    if isinstance(v, (int, float)):
        v = (v, v, v)
    return unreal.Vector(float(v[0]), float(v[1]), float(v[2]))


def rot(r):
    r = r or (0, 0, 0)
    return unreal.Rotator(pitch=float(r[0]), yaw=float(r[1]), roll=float(r[2]))


def convert(current, value):
    """Shape a JSON value like the property's current value (vectors, colors, rotators, assets)."""
    if isinstance(value, list):
        if isinstance(current, unreal.LinearColor):
            r, g, b, a = [float(x) for x in (value + [1.0])[:4]]
            return unreal.LinearColor(r=r, g=g, b=b, a=a)
        if isinstance(current, unreal.Color):
            # keywords: FColor's field order is B, G, R, A, so positional arguments swap red and blue
            r, g, b, a = [int(x) for x in (value + [255])[:4]]
            return unreal.Color(r=r, g=g, b=b, a=a)
        if isinstance(current, unreal.Rotator):
            return rot(value)
        if isinstance(current, unreal.Vector):
            return vec(value)
    if isinstance(value, str) and isinstance(current, unreal.EnumBase):
        return getattr(type(current), value.upper())
    if isinstance(value, str) and value.startswith("/") and not isinstance(current, str):
        obj = unreal.load_object(None, value)
        if obj is None:
            raise RuntimeError(f"asset {value} not found")
        return obj
    return value


def set_props(actor, props):
    for key, value in (props or {}).items():
        target, name = actor, key
        if "." in key:
            comp, name = key.split(".", 1)
            target = actor.get_editor_property(comp)
        target.set_editor_property(name, convert(target.get_editor_property(name), value))


def load_class(path):
    cls = unreal.load_class(None, path)
    if cls is None:
        raise RuntimeError(f"class {path} not found (Blueprints need the _C suffix: /Game/X/BP_A.BP_A_C)")
    return cls


def tag(actor, extra=()):
    actor.set_editor_property("tags", [unreal.Name(t) for t in (GEN_TAG, *extra)])


def spawn_one(actors, spec, loc, yaw_jitter, label):
    r = list(spec.get("rot") or (0, 0, 0))
    r[1] += yaw_jitter
    if "mesh" in spec:
        mesh = unreal.load_asset(spec["mesh"])
        if not isinstance(mesh, unreal.StaticMesh):
            raise RuntimeError(f"{spec['mesh']} is not a StaticMesh")
        # spawn_actor_from_object goes through viewport placement and spawns nothing in a commandlet
        actor = actors.spawn_actor_from_class(unreal.StaticMeshActor, loc, rot(r))
        comp = actor.get_editor_property("static_mesh_component")
        comp.set_editor_property("static_mesh", mesh)
        for i, mpath in enumerate(spec.get("materials") or []):
            if mpath:
                mat = unreal.load_asset(mpath)
                if mat is None:
                    raise RuntimeError(f"material {mpath} not found")
                comp.set_material(i, mat)
    elif "class" in spec:
        actor = actors.spawn_actor_from_class(load_class(spec["class"]), loc, rot(r))
    else:
        raise RuntimeError(f"actor entry needs 'mesh' or 'class': {spec}")
    if actor is None:
        raise RuntimeError(f"spawn failed for {spec}")
    if "scale" in spec:
        actor.set_actor_scale3d(vec(spec["scale"]))
    if label:
        actor.set_actor_label(label)
    if spec.get("folder"):
        actor.set_folder_path(spec["folder"])
    tag(actor, spec.get("tags") or ())
    set_props(actor, spec.get("props"))
    return actor


def environment(actors, env):
    made = []
    if env.get("sun", True) is not False:
        sun = env.get("sun") or {}
        a = actors.spawn_actor_from_class(unreal.DirectionalLight, vec((0, 0, 1000)), rot(sun.get("rot", (-45, 30, 0))))
        c = a.get_editor_property("directional_light_component")
        c.set_editor_property("atmosphere_sun_light", True)
        c.set_editor_property("intensity", float(sun.get("intensity", 8)))
        c.set_editor_property("light_color", convert(c.get_editor_property("light_color"),
                                                     [int(255 * x) for x in sun.get("color", (1, 0.96, 0.9))]))
        c.set_editor_property("mobility", unreal.ComponentMobility.MOVABLE)
        made.append(("Sun", a))
    if env.get("sky", True):
        made.append(("SkyAtmosphere", actors.spawn_actor_from_class(unreal.SkyAtmosphere, vec(None), rot(None))))
        sky = actors.spawn_actor_from_class(unreal.SkyLight, vec((0, 0, 500)), rot(None))
        sc = sky.get_editor_property("light_component")
        sc.set_editor_property("real_time_capture", True)
        sc.set_editor_property("mobility", unreal.ComponentMobility.MOVABLE)
        made.append(("SkyLight", sky))
    if env.get("fog", True) is not False:
        fog = actors.spawn_actor_from_class(unreal.ExponentialHeightFog, vec(None), rot(None))
        fc = fog.get_editor_property("component")
        fc.set_editor_property("fog_density", float((env.get("fog") or {}).get("density", 0.02)))
        made.append(("HeightFog", fog))
    if env.get("clouds"):
        made.append(("Clouds", actors.spawn_actor_from_class(unreal.VolumetricCloud, vec(None), rot(None))))
    if "exposure" in env:
        ppv = actors.spawn_actor_from_class(unreal.PostProcessVolume, vec(None), rot(None))
        ppv.set_editor_property("unbound", True)
        s = ppv.get_editor_property("settings")
        # Auto exposure stays on (manual/physical exposure turns a default-intensity sun black);
        # the bias shifts the whole image brighter (+) or darker (-) in stops.
        s.set_editor_property("override_auto_exposure_bias", True)
        s.set_editor_property("auto_exposure_bias", float(env["exposure"]))
        ppv.set_editor_property("settings", s)
        made.append(("PostProcess", ppv))
    for label, a in made:
        a.set_actor_label(label)
        a.set_folder_path("Environment")
        tag(a)
    return len(made)


def main():
    args = json.loads(os.environ.get("JF_PY_ARGS", "[]"))
    if not args:
        raise RuntimeError("usage: jf level <spec.json>")
    with open(args[0], encoding="utf-8") as fh:
        spec = json.load(fh)
    path = spec["map"]
    if not path.startswith("/Game/"):
        raise RuntimeError(f"map must be under /Game, got {path}")
    levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    editor = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
    existed = unreal.EditorAssetLibrary.does_asset_exist(path)
    ok = levels.load_level(path) if existed else levels.new_level(path, bool(spec.get("partitioned", False)))
    if not ok:
        raise RuntimeError(f"could not {'load' if existed else 'create'} {path}")

    removed = 0
    for a in actors.get_all_level_actors():
        if unreal.Name(GEN_TAG) in a.get_editor_property("tags"):
            actors.destroy_actor(a)
            removed += 1

    env_count = environment(actors, spec.get("environment") or {}) if spec.get("environment", {}) is not None else 0
    ps = spec.get("player_start")
    if ps is not None:
        a = actors.spawn_actor_from_class(unreal.PlayerStart, vec(ps.get("loc", (0, 0, 120))), rot(ps.get("rot")))
        a.set_actor_label("PlayerStart")
        tag(a)
    if spec.get("game_mode"):
        ws = editor.get_editor_world().get_world_settings()
        ws.set_editor_property("default_game_mode", load_class(spec["game_mode"]))

    rng = random.Random(spec.get("seed", 1))
    spawned = 0
    for n, entry in enumerate(spec.get("actors") or []):
        grid = entry.get("grid") or {}
        cx, cy = (grid.get("count") or (1, 1))[:2]
        sx, sy = (grid.get("step") or (100, 100))[:2]
        base = vec(entry.get("loc"))
        for ix in range(int(cx)):
            for iy in range(int(cy)):
                loc = unreal.Vector(base.x + ix * sx, base.y + iy * sy, base.z)
                yaw = rng.uniform(0, 360) if entry.get("random_yaw") else 0.0
                label = entry.get("label") or ""
                if label and cx * cy > 1:
                    label = f"{label}_{ix}_{iy}"
                spawn_one(actors, entry, loc, yaw, label)
                spawned += 1

    if not levels.save_current_level():
        raise RuntimeError(f"save failed for {path}")
    print("JF_LEVEL " + json.dumps({"map": path, "created": not existed, "removed_generated": removed,
                                    "environment": env_count, "spawned": spawned,
                                    "player_start": ps is not None}))


main()
