"""Shared helpers for the greybox generators (pier5_greybox.py -> L_Pier5, office_greybox.py -> L_Office).

Coordinates are metres in the LEVEL.md frame: X east, Y north, Z up. write_spec() flips Y on output (UE is
left-handed; in UE north is -Y), so the editor's top view matches the LEVEL.md maps. Each generator adds to
`actors` and calls write_spec() once.
"""
import json
import math
from pathlib import Path

CUBE = "/Engine/BasicShapes/Cube"
CYL = "/Engine/BasicShapes/Cylinder"
GRID = "/Game/LevelPrototyping/Materials/MI_PrototypeGrid_Gray"
GRID2 = "/Game/LevelPrototyping/Materials/MI_PrototypeGrid_Gray_02"
DARK = "/Game/LevelPrototyping/Materials/MI_PrototypeGrid_TopDark"

WATER_Z = -3.0      # high water
SEABED_Z = -8.0
WALL = 0.2          # wall thickness
DOOR_W, DOOR_H = 1.2, 2.2
CEIL = 3.0

actors = []


def box(label, x0, x1, y0, y1, z0, z1, mat=GRID, folder="Greybox", rot=None):
    """Axis-aligned block from min/max corners in metres."""
    a = {"mesh": CUBE, "label": label, "folder": folder, "materials": [mat],
         "loc": [round((x0 + x1) * 50, 1), round((y0 + y1) * 50, 1), round((z0 + z1) * 50, 1)],
         "scale": [round(abs(x1 - x0), 3), round(abs(y1 - y0), 3), round(abs(z1 - z0), 3)]}
    if rot:
        a["rot"] = rot
    actors.append(a)


def ramp(label, x, y0, y1, z0, z1, width, folder="Greybox", thick=0.2):
    """A plank running along +Y from (y0, z0) to (y1, z1), centred on x."""
    run, rise = y1 - y0, z1 - z0
    length = math.hypot(run, rise)
    # UE FRotator: the local Y axis becomes (0, cos R, -sin R), so positive roll LOWERS +Y; negate to lift it
    roll = -math.degrees(math.atan2(rise, run))
    actors.append({"mesh": CUBE, "label": label, "folder": folder, "materials": [GRID2],
                   "loc": [x * 100, (y0 + y1) * 50, (z0 + z1) * 50 - thick * 50],
                   "rot": [0, 0, round(roll, 2)], "scale": [width, round(length, 3), thick]})


def post(label, x, y, z0, z1, d, folder="Greybox", mat=GRID2):
    actors.append({"mesh": CYL, "label": label, "folder": folder, "materials": [mat],
                   "loc": [x * 100, y * 100, (z0 + z1) * 50], "scale": [d, d, round(z1 - z0, 3)]})


def wall_x(label, x0, x1, y, z0, h, holes, folder):
    """Wall along X at y (thickness centred), holes = [(centre_x, width, bottom, top)]."""
    _wall(label, x0, x1, z0, h, holes, folder, lambda a, b, c, d: box(label, a, b, y - WALL / 2, y + WALL / 2, c, d, folder=folder))


def wall_y(label, y0, y1, x, z0, h, holes, folder):
    _wall(label, y0, y1, z0, h, holes, folder, lambda a, b, c, d: box(label, x - WALL / 2, x + WALL / 2, a, b, c, d, folder=folder))


def _wall(label, s0, s1, z0, h, holes, folder, emit):
    cur = s0
    for c, w, bot, top in sorted(holes):
        a, b = c - w / 2, c + w / 2
        if a > cur:
            emit(cur, a, z0, z0 + h)
        if bot > 0:
            emit(a, b, z0, z0 + bot)          # sill
        if top < h:
            emit(a, b, z0 + top, z0 + h)      # lintel
        cur = b
    if cur < s1:
        emit(cur, s1, z0, z0 + h)


def room(name, x0, x1, y0, y1, z0, h=CEIL, holes=None, floor=True, ceiling=True):
    """Enterable room, inner bounds. holes: {'N'|'S'|'E'|'W': [(centre, width, bottom, top)]}.
    N is the +Y wall. A door is (centre, DOOR_W, 0, DOOR_H)."""
    holes = holes or {}
    f = f"Interiors/{name}"
    if floor:
        box(f"{name}_Floor", x0 - WALL, x1 + WALL, y0 - WALL, y1 + WALL, z0 - 0.2, z0, folder=f)
    if ceiling:
        box(f"{name}_Ceiling", x0 - WALL, x1 + WALL, y0 - WALL, y1 + WALL, z0 + h, z0 + h + 0.2, folder=f)
    wall_x(f"{name}_WallS", x0 - WALL, x1 + WALL, y0 - WALL / 2, z0, h, holes.get("S", []), f)
    wall_x(f"{name}_WallN", x0 - WALL, x1 + WALL, y1 + WALL / 2, z0, h, holes.get("N", []), f)
    wall_y(f"{name}_WallW", y0, y1, x0 - WALL / 2, z0, h, holes.get("W", []), f)
    wall_y(f"{name}_WallE", y0, y1, x1 + WALL / 2, z0, h, holes.get("E", []), f)


def door(c):
    return (c, DOOR_W, 0.0, DOOR_H)


def window(c, w, sill=1.0, top=2.2):
    return (c, w, sill, top)


def marker(label, x, y, z, tags, folder):
    actors.append({"class": "/Script/Engine.TargetPoint", "label": label, "folder": folder,
                   "loc": [x * 100, y * 100, z * 100], "tags": tags})


def light(label, kind, x, y, z, color, intensity, radius, rot=None, folder="Lights", extra=None):
    comp = {"point": "point_light_component", "spot": "spot_light_component"}[kind]
    cls = {"point": "/Script/Engine.PointLight", "spot": "/Script/Engine.SpotLight"}[kind]
    props = {f"{comp}.intensity": intensity, f"{comp}.attenuation_radius": radius * 100,
             f"{comp}.light_color": color}
    for k, v in (extra or {}).items():
        props[f"{comp}.{k}"] = v
    a = {"class": cls, "label": label, "folder": folder, "loc": [x * 100, y * 100, z * 100], "props": props}
    if rot:
        a["rot"] = rot
    actors.append(a)


SODIUM = [255, 175, 95, 255]
WARM = [255, 205, 150, 255]
COLD = [170, 200, 255, 255]
CYAN = [60, 230, 255, 255]


# ---------------------------------------------------------------- case actors (LEVEL sections 6 and 8)
# C++ classes in the game module (Source/HarrowMark/CaseActors.h). Positions in metres (LEVEL coordinates),
# sizes and photo boxes in cm. A photo box only has to hold what the frame must hold: small things get a box
# a little larger than the prop so they read sharp from a crouch at about a metre (SharpSize 0.35 of the frame).
SUBJECT = "/Script/HarrowMark.CaseSubjectActor"
PERSON = "/Script/HarrowMark.CasePersonActor"
GATE = "/Script/HarrowMark.CaseGateActor"
DRIVE = "/Script/HarrowMark.CaseDriveActor"


def subject(shot, x, y, z, mesh, extent, lit=False, rot=None):
    """mesh = block size in cm, or None when the set already has the thing (the box then sits on the set)."""
    props = {"mesh_size": list(mesh or (0, 0, 0)), "subject.shot_id": shot, "subject.box_extent": list(extent)}
    if lit:
        props["subject.lit_by_scene"] = True
    a = {"class": SUBJECT, "label": shot, "folder": "Case/Subjects", "loc": [x * 100, y * 100, z * 100],
         "tags": ["SUBJ", shot], "props": props}
    if rot:
        a["rot"] = rot
    actors.append(a)


def person(label, who, x, y, ground, cm=0, nights=None, shot=None, extent=None, shown_with=None,
           hidden_with=None, yaw=0):
    props = {"talk.speaker": who, "height_cm": cm}
    if nights:
        props["nights"] = nights
    if shot:
        props["subject.shot_id"] = shot
        props["subject.box_extent"] = list(extent or (30, 30, 90))
    if shown_with:
        props["shown_with_shot"] = shown_with
    if hidden_with:
        props["hidden_with_shots"] = hidden_with
    actors.append({"class": PERSON, "label": label, "folder": "Case/People", "tags": ["NPC", who],
                   "loc": [x * 100, y * 100, (ground + 0.92) * 100], "rot": [0, yaw, 0], "props": props})


def gate(label, gate_id, x, y, z, size):
    actors.append({"class": GATE, "label": label, "folder": "Case/Gates", "loc": [x * 100, y * 100, z * 100],
                   "props": {"mesh_size": list(size), "gate.gate": gate_id}})


def player_start(tag, x, y, z, yaw):
    """A PlayerStart the game mode picks by tag (AHarrowGameMode: Start, Car, Office, Dawn). z is the floor;
    yaw in the LEVEL frame (90 faces north)."""
    actors.append({"class": "/Script/Engine.PlayerStart", "label": f"PlayerStart_{tag}", "folder": "Markers",
                   "loc": [x * 100, y * 100, (z + 1.0) * 100], "rot": [0, yaw, 0],
                   "props": {"player_start_tag": tag}})


def drive(label, x, y, z, size, yaw=0):
    """The drive (ACaseDriveActor): the car at the curb, the office door. z is the floor; size in cm."""
    actors.append({"class": DRIVE, "label": label, "folder": "Case/Drive",
                   "loc": [x * 100, y * 100, z * 100 + size[2] / 2], "rot": [0, yaw, 0],
                   "props": {"mesh_size": list(size)}})


def to_ue(a):
    """Mirror one actor entry from LEVEL coordinates (Y north) to UE (north = -Y): y -> -y, yaw -> -yaw, roll -> -roll."""
    a = dict(a)
    if "loc" in a:
        a["loc"] = [a["loc"][0], -a["loc"][1], a["loc"][2]]
    if "rot" in a:
        p_, y_, r_ = a["rot"]
        a["rot"] = [p_, -y_ + 0.0, -r_ + 0.0]
    return a


NIGHT = {"sun": {"rot": [-20, -150, 0], "intensity": 0.08, "color": [0.55, 0.65, 1.0]},
         "sky": True, "fog": {"density": 0.04}}


def write_spec(map_path, out_name, environment=NIGHT):
    spec = {"map": map_path, "partitioned": False, "environment": environment,
            "actors": [to_ue(a) for a in actors]}
    out = Path(__file__).with_name(out_name)
    out.write_text(json.dumps(spec, indent=1) + chr(10), encoding="utf-8")
    print(f"{out}: {len(actors)} actors")
