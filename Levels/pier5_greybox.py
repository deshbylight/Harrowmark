"""Generate the Pier 5 greybox spec (Levels/L_Pier5.json) for `jf level`.

Source of truth for every number here: docs/design/LEVEL.md (sections 2, 3, 9). Coordinates are metres,
X east (-50..50), Y seaward (-55..65), Z up with the quay and the pier deck at 0 and high water at -3.0.
The agency office is its own map (office_greybox.py -> L_Office). Helpers are in greybox_lib.py.
Edit this script, not the JSON, then:
    python Levels/pier5_greybox.py && python <JamForge>/jf.py level Levels/L_Pier5.json

Case actors (photo subjects, people, gates; tags SUBJ / NPC) are the C++ classes in Source/HarrowMark/CaseActors.h;
subject labels match the shot ids in LEVEL.md section 6 and data/case01.json.

UE is left-handed: facing +Y, +X is on the LEFT, so LEVEL's map (X east, Y north) would come out mirrored
(west on the right when you face the sea). to_ue() flips Y on output: in UE north is -Y, east is +X, and the
editor's top view matches the LEVEL.md map. Everything in this script stays in LEVEL coordinates.
"""
from greybox_lib import *  # noqa: F401,F403 (helpers, colours, the shared actors list)

# ---------------------------------------------------------------- water, land, quay, street
box("Water", -200, 200, -200, 700, WATER_Z - 0.1, WATER_Z, mat=DARK, folder="Greybox/Ground")
box("Seabed", -200, 200, -200, 700, SEABED_Z - 0.2, SEABED_Z, mat=DARK, folder="Greybox/Ground")
# land south of the quay edge (Y < 0); the quay wall drops into the water at Y 0
box("Land_Quay", -50, 50, -60, 0, SEABED_Z, 0.0, folder="Greybox/Ground")

# fence at Y -30 with the gate at X 0..6
box("Fence_W", -50, 0, -30.1, -29.9, 0, 2.4, folder="Greybox/Street")
# the harbour-works wicket at X 28 behind Warehouse 5 (the killer's way in and out, LEVEL section 3): out of
# the booth's sight past the warehouse corner, in the dark gap between the street lamps
box("Fence_E", 6, 27.5, -30.1, -29.9, 0, 2.4, folder="Greybox/Street")
box("Fence_E2", 28.5, 50, -30.1, -29.9, 0, 2.4, folder="Greybox/Street")
box("Wicket_Lintel", 27.5, 28.5, -30.1, -29.9, 2.0, 2.4, folder="Greybox/Street")
box("GateBarrier", 0.3, 5.7, -30.05, -29.95, 0.95, 1.05, folder="Greybox/Street")
post("GateBarrierPost", 0.15, -30, 0, 1.1, 0.3, folder="Greybox/Street")
# police car blocking the east end of the town street (Y -30..-44)
box("PoliceCar", 40, 44.5, -40, -38, 0, 1.5, folder="Greybox/Street")

# ---------------------------------------------------------------- Pier 5 (X -6..6, Y 0..60)
P = "Greybox/Pier5"
box("Pier_Deck", -6, 6, 0, 60, -0.3, 0.0, folder=P)
for y in range(2, 60, 4):
    for x in (-5.8, -2.0, 2.0, 5.8):
        post("Piling", x, y, SEABED_Z, -0.3, 0.4, folder=P)
for y in range(6, 60, 8):
    for x in (-5.7, 5.7):
        if x > 0 and y < 16:
            continue  # gangway foot
        post("Bollard", x, y, 0, 0.6, 0.35, folder=P)
# crane rails along the deck
for x in (-3.0, 3.0):
    box("CraneRail", x - 0.05, x + 0.05, 0, 60, 0, 0.05, folder=P)
# fish crates and nets at Y 30-38 break the sightline; bale hook on the top crate (A08)
box("FishCrates_A", -2.5, 0.5, 33, 36, 0, 1.6, folder=P)
box("FishCrates_B", 0.5, 2.0, 34, 35.5, 0, 0.8, folder=P)
box("Nets", -4.5, -2.5, 30, 32, 0, 0.6, folder=P)
# edge: last 5 m. Lamp post on the west corner (head at 2.4 m), gouge at 2.1 m on the deck side
post("EdgeLamp_Post", -5.4, 59.4, 0, 2.4, 0.15, folder=P)
box("EdgeLamp_Head", -5.6, -5.2, 59.2, 59.6, 2.3, 2.5, folder=P)
box("Curb_W", -6, -5.85, 0, 60, 0, 0.15, folder=P)
box("Curb_E", 5.85, 6, 0, 60, 0, 0.15, folder=P)
box("Curb_N", -6, 6, 59.85, 60, 0, 0.15, folder=P)
# ladder down the west side to the lower landing (5 m from the body)
box("Ladder_Landing", -6.4, -6.0, 53.5, 54.3, -2.4, 0.9, folder=P)

# lower landing: floating stage beside the pier, open to the sky, top 0.8 m above high water
box("LowerLanding", -11, -6.1, 50, 56, -2.4, -2.2, folder=P)
box("Catwalk_LandingStep", -6.1, -5.6, 51, 52, -2.2, -2.1, folder=P)
# catwalk under the west overhang, Y 24-52, 1.0 m wide, floor 1.0 m above high water, 1.7 m headroom
box("Catwalk", -5.6, -4.6, 24, 52, -2.1, -2.0, mat=GRID2, folder=P)
for y in range(26, 52, 4):
    box("CatwalkStringer", -5.6, -4.6, y - 0.1, y + 0.1, -2.4, -2.1, folder=P)
# the boathook wedged in the pilings 4 m in from the north end of the catwalk (A06), seen only from there
box("Boathook", -4.2, -1.8, 47.8, 48.0, -1.3, -1.2, folder=P, rot=[0, 20, 35])

# customs berth: pontoon on the west side, Y 8-24, reached by a ramp from the pier root
box("Pontoon", -14, -6.2, 8, 24.5, -2.4, -2.2, folder=P)
box("CatwalkStep", -6.2, -5.6, 23.8, 24.6, -2.2, -2.1, folder=P)
ramp("Ramp_Berth", -8.0, 0.0, 8.0, 0.0, -2.2, 2.0, folder=P)
box("Ramp_Wall_W", -9.1, -9.0, 0, 8, -2.2, 0.9, folder=P)
post("CL2_MooringPost", -6.6, 18, -2.2, -1.2, 0.3, folder=P)
box("CL2_Launch", -13.5, -10.5, 10, 19, -3.0, -1.6, folder=P)
post("CL2_Mast", -12, 15, -1.6, 3.5, 0.12, folder=P)
box("NoAdmittanceBoard", -7.4, -6.3, 24.5, 24.6, -2.2, -1.0, folder=P)

# ---------------------------------------------------------------- crane on rails at Y 20, cab 12 m up
C = "Greybox/Crane"
for x in (-3.0, 3.0):
    for y in (18.5, 21.5):
        box("CraneLeg", x - 0.3, x + 0.3, y - 0.3, y + 0.3, 0, 11.0, folder=C)
box("CranePortal", -3.3, 3.3, 18.2, 21.8, 11.0, 11.8, folder=C)
room("CraneCab", -1.0, 1.0, 19.0, 21.0, 12.0, 2.2,
     holes={"N": [window(0, 1.6, 0.9, 2.0)], "S": [window(0, 1.6, 0.9, 2.0)],
            "E": [window(20, 1.6, 0.9, 2.0)], "W": [window(20, 1.6, 0.9, 2.0)]})
# parked jib over the Agatha's No. 2 hatch (discharge stopped at 01:10); reach 18 m
box("CraneJib", 1.2, 19.0, 19.6, 20.4, 13.6, 14.2, folder=C)
box("CraneLadder", 3.3, 3.5, 21.6, 22.2, 0, 11.0, folder=C)
box("CraneAwning", 3.4, 6.0, 17.0, 19.0, 2.4, 2.5, folder=C)

# ---------------------------------------------------------------- SS Saint Agatha (hull X +7..+15)
A = "Greybox/Agatha"
box("Agatha_Hull", 7, 15, 4, 64, -6.0, 4.0, mat=GRID2, folder=A)
box("Agatha_Bow", 8, 14, 64, 68, -4.0, 4.6, mat=GRID2, folder=A)
box("Agatha_Rail_W", 7, 7.1, 15, 64, 4.0, 5.0, folder=A)
# gangway at Y 14: 9 m, foot on the pier near the quay, head at the ship's rail
ramp("Agatha_Gangway", 5.4, 5.85, 14.0, 0.0, 4.0, 1.0, folder=A)
box("Agatha_GangwayHead", 4.9, 7.0, 14.0, 15.0, 3.8, 4.0, folder=A)
box("WatchStand", 7.6, 8.1, 15.2, 15.7, 4.0, 5.2, folder=A)
box("WatchStand_Hood", 7.5, 8.2, 15.1, 15.8, 5.6, 5.7, folder=A)
# No. 2 hatch under the jib
box("Hatch2_Coaming", 9, 13, 17, 23, 4.0, 4.8, folder=A)
# aft deckhouse with the captain's cabin (5 x 4 m), MASTER door on the north side
room("CaptainCabin", 8.5, 13.5, 6.0, 10.0, 4.0, 2.6, holes={"N": [door(9.6)], "W": [window(8.0, 1.0, 1.0, 1.8)]})
box("Funnel", 10, 12, 7, 9, 6.8, 11.0, folder=A)

# ---------------------------------------------------------------- port office (X -40..-15), customs office
O = "Greybox/PortOffice"
# customs office: 7 x 5 m ground floor, door on the east wall facing the gate booth
room("CustomsOffice", -22, -15.2, -28, -23, 0.0, CEIL,
     holes={"E": [door(-26.6)], "S": [window(-18.5, 1.6)]})
box("PortOffice_West", -40, -22.2, -28.2, -16, 0, 7.0, mat=GRID2, folder=O)
box("PortOffice_North", -22.2, -15, -22.8, -16, 0, 7.0, mat=GRID2, folder=O)
box("PortOffice_Upper", -22.2, -15, -28.2, -22.8, 3.2, 7.0, mat=GRID2, folder=O)
# gate booth (2.5 x 2 m), not enterable: open hatch and north window, west window toward the office door
room("GateBooth", -6.25, -3.75, -29.0, -27.0, 0.0, 2.6,
     holes={"N": [window(-5.0, 1.4, 1.0, 2.0)], "W": [window(-28.0, 1.0, 1.0, 2.0)],
            "E": [window(-28.0, 0.8, 1.0, 1.8)]})
# warehouse 5, exterior only, loading dock facing the quay
box("Warehouse5", 15, 45, -29, -16, 0, 9.0, mat=GRID2, folder="Greybox/Quay")
box("Warehouse5_Dock", 22, 38, -16, -13, 0, 1.2, folder="Greybox/Quay")
box("Warehouse5_Canopy", 22, 38, -16, -12.5, 4.5, 4.7, folder="Greybox/Quay")
box("OilDrums", -12, -10, -6, -4, 0, 0.9, folder="Greybox/Quay")
box("Pallets", 9, 12, -8, -6, 0, 0.5, folder="Greybox/Quay")
# coroner's van at the gate (night 1 layer, folder Night1)
box("CoronerVan", 8, 13, -27, -25, 0, 2.2, folder="Night1")

# ---------------------------------------------------------------- the Lighthouse (X 5..25, Y -55..-44)
L = "Greybox/Lighthouse"
# bar: 12 x 9 m ground floor, door on the NW corner facing the gate, counter along the east wall, back door
# to the yard; upstairs are Nora's rooms (not enterable). The agency is uptown: office_greybox.py -> L_Office.
room("Bar", 6, 18, -53, -44, 0.0, 3.5,
     holes={"N": [door(7.2), window(11.5, 2.0, 0.9, 2.4), window(15.5, 2.0, 0.9, 2.4)],
            "E": [door(-52.0)]})
box("Bar_Counter", 15.6, 16.4, -52, -45.5, 0, 1.1, folder="Interiors/Bar")
box("Bar_BackBar_Mirror", 17.6, 17.8, -50, -46.5, 1.0, 2.6, folder="Interiors/Bar")
box("Bar_Register", 15.7, 16.3, -46.0, -45.5, 1.1, 1.4, folder="Interiors/Bar")
for i, (x, y) in enumerate(((8.5, -47), (11.5, -47), (8.5, -50.5), (11.5, -50.5))):
    post(f"Bar_Table{i}", x, y, 0, 0.75, 0.9, folder="Interiors/Bar")
box("Lighthouse_Shell_W", 5, 5.8, -55, -43.8, 0, 7.2, mat=GRID2, folder=L)
box("Lighthouse_Shell_S", 5, 25.1, -55, -53.2, 0, 7.2, mat=GRID2, folder=L)
box("Lighthouse_Upper", 5.8, 18.2, -53.2, -43.8, 3.7, 7.2, mat=GRID2, folder=L)
box("Lighthouse_Roof", 5, 18.2, -55, -43.8, 7.2, 7.5, mat=GRID2, folder=L)
box("Neon_Sign", 9, 17, -43.75, -43.6, 3.8, 4.6, folder=L)
# the yard behind the bar: walls 2.6 m, one bulb over the back door, a car-wide gate onto the street
Y = "Greybox/LighthouseYard"
wall_x("Yard_WallN", 18.2, 25.1, -43.9, 0.0, 2.6, [(23.0, 2.8, 0.0, 2.6)], Y)
wall_y("Yard_WallE", -53.2, -43.8, 25.0, 0.0, 2.6, [], Y)
box("Yard_Empties", 18.4, 19.6, -53.1, -52.6, 0, 0.9, folder=Y)
box("Yard_Bins", 23.8, 24.8, -53.1, -51.9, 0, 1.1, folder=Y)

# the detective's car at the curb west of the Lighthouse: E is the drive to the office (GDD 9.4)
drive("DetectiveCar", -0.6, -42.7, 0.0, (480, 180, 145))
player_start("Car", -0.6, -40.6, 0.0, 60)
# night 1 starts at the rope on Pier 5 (the prologue drive is a cutscene)
player_start("Start", 1.5, 52.5, 0.0, 120)

# ---------------------------------------------------------------- landmark and practical lights
# crane floods: fixed; one down the deck from the root to the last 5 m, one onto the pontoon
light("CraneFlood_Deck", "spot", 0.5, 21.0, 11.5, COLD, 400, 60, rot=[-40, 90, 0],
      extra={"outer_cone_angle": 21.0, "inner_cone_angle": 14.0})
light("CraneFlood_Berth", "spot", -2.5, 19.0, 11.5, COLD, 2500, 30, rot=[-60, 202, 0],
      extra={"outer_cone_angle": 25.0, "inner_cone_angle": 15.0})
# pier lamps every ~13 m, the edge lamp is dead (A09: the missing light is the main landmark)
for i, (x, y) in enumerate(((-5.4, 8), (5.4, 20), (-5.4, 33), (5.4, 46))):
    post(f"PierLamp_Post{i}", x, y, 0, 4.0, 0.15, folder=P)
    light(f"PierLamp{i}", "point", x, y, 3.9, SODIUM, 25, 12)
# quay and street sodium lamps, 12-15 m apart with dark gaps
for i, x in enumerate((-42, -28, -14, 14, 28, 42)):
    post(f"QuayLamp_Post{i}", x, -2.0, 0, 5.0, 0.15, folder="Greybox/Quay")
    light(f"QuayLamp{i}", "point", x, -2.0, 4.9, SODIUM, 30, 13)
for i, x in enumerate((-35, -21, -7, 36, 46)):
    post(f"StreetLamp_Post{i}", x, -31.0, 0, 5.0, 0.15, folder="Greybox/Street")
    light(f"StreetLamp{i}", "point", x, -31.0, 4.9, SODIUM, 30, 13)
light("Neon_Lighthouse", "point", 13, -43.4, 4.2, CYAN, 20, 14)
light("Agatha_WorkLight", "point", 9.5, 20.0, 7.5, WARM, 40, 16)
light("Agatha_DeckWorkLight_Spill", "point", 8.0, 12.0, 6.0, WARM, 15, 10)
light("Booth_Bulb", "point", -5.0, -28.0, 2.3, WARM, 8, 5)
light("CustomsOffice_DeskLamp", "point", -18.5, -25.5, 2.0, WARM, 10, 6)
light("CraneCab_Bulb", "point", 0, 20, 14.0, WARM, 5, 4)
light("WatchStand_Bulb", "point", 7.85, 15.45, 5.5, WARM, 6, 5)
light("Yard_Bulb", "point", 18.7, -52.0, 2.6, WARM, 6, 6)
light("Nora_Window", "point", 12.0, -43.4, 5.4, WARM, 3, 4)
for i, (x, y) in enumerate(((9, -48), (13, -48), (9, -51), (13, -51))):
    light(f"Bar_Practical{i}", "point", x, y, 3.0, WARM, 10, 6)
# warm lights over usable doors
for i, (x, y, z) in enumerate(((7.2, -43.6, 2.6), (-14.8, -26.6, 2.6), (9.6, 10.4, 6.4))):
    light(f"DoorLight{i}", "point", x, y, z, WARM, 6, 4)
# hurricane lantern at the rope post (night 1)
light("Lantern_Rope", "point", -1.5, 55.5, 1.0, WARM, 4, 5, folder="Night1")
# breakwater lighthouse 600 m north
post("Breakwater_Lighthouse", -60, 600, WATER_Z, 25, 4.0, folder="Greybox/Ground")
light("Breakwater_Beam", "point", -60, 600, 24, COLD, 2000, 120)

# ---------------------------------------------------------------- case actors (LEVEL sections 6 and 8)
# C++ classes in the game module (Source/HarrowMark/CaseActors.h); helpers in greybox_lib.py.
# set dressing the subjects rest on
box("Notebook_Crate", -3.85, -3.35, 55.75, 56.25, 0, 0.58, folder="Night1")
box("CraneCab_Shelf", 0.3, 0.9, 20.35, 20.85, 12.0, 12.98, folder="Greybox/Crane")
box("Cabin_Desk", 9.4, 10.6, 8.5, 9.5, 4.0, 4.75, folder="Interiors/CaptainCabin")
box("Customs_Desk", -19.3, -17.7, -26.5, -25.5, 0, 0.75, folder="Interiors/CustomsOffice")
box("Customs_LockerBody", -22.0, -21.45, -24.3, -23.7, 0, 1.9, folder="Interiors/CustomsOffice")

# pier edge (night 1: the body under the tarp; expires after night 1 and hides itself)
subject("SH_Wound", -2.5, 57.5, 0.13, (170, 50, 25), (30, 25, 15))
subject("SH_Watch", -1.9, 57.0, 0.02, (5, 5, 2), (14, 14, 6))
subject("SH_Notebook", -3.6, 56.0, 0.59, (15, 21, 2), (18, 22, 6))
subject("SH_Lamp", -5.4, 59.4, 2.4, None, (25, 25, 20))
subject("SH_Gouge", -5.31, 59.4, 2.1, None, (6, 10, 15))
subject("SH_Bottle", 1.0, 54.0, 0.04, (25, 8, 8), (18, 18, 10))
# under the pier, seen only from the catwalk
subject("SH_Boathook", -2.9, 47.9, -1.25, (6, 170, 6), (15, 70, 15))
subject("SH_BaleHook", -1.0, 34.5, 1.68, (25, 6, 25), (18, 18, 20))
subject("SH_CraneLog", 0.6, 20.6, 13.0, (21, 30, 3), (22, 25, 8))
# gate booth: book on the east sill, chalkboard on the back wall seen through the north window
subject("SH_GateBook", -4.0, -28.0, 1.03, (30, 22, 4), (22, 22, 8))
subject("SH_Movements", -5.0, -28.85, 1.5, (100, 4, 70), (55, 8, 40))
# bar
subject("SH_CashTape", 16.0, -45.75, 1.41, (8, 20, 1), (12, 18, 6), lit=True)
subject("SH_Slate", 17.95, -45.95, 1.8, (3, 45, 35), (8, 24, 20), lit=True)
# the Agatha
subject("SH_GangwayBook", 7.85, 15.45, 5.22, (30, 22, 3), (20, 20, 8), lit=True)
subject("SH_Tally", 8.98, 20.0, 4.45, (2, 25, 35), (6, 18, 22))
subject("SH_Crates", 12.5, 6.8, 4.45, (120, 80, 90), (65, 45, 50))
subject("SH_CabinDesk", 10.0, 8.8, 4.76, (30, 21, 2), (22, 20, 6))
# customs office (lit, unlocked)
subject("SH_Locker", -21.7, -24.0, 0.95, None, (25, 30, 90), lit=True)
subject("SH_Roster", -15.25, -25.0, 1.6, (2, 30, 42), (6, 18, 24), lit=True)
subject("SH_Draft", -18.5, -26.0, 0.76, (21, 30, 1), (20, 20, 6), lit=True)

gate("Gate_CabinDoor", "G_Cabin", 9.6, 10.1, 5.1, (120, 12, 220))
gate("Gate_LockerDoor", "G_Locker", -21.43, -24.0, 0.95, (3, 58, 186))

# people (case data heights). Usual spots every night unless a tableau takes them away.
person("Holt", "P_Holt", 4.7, 18.0, 0.0, cm=190, yaw=-90)
person("Kordes", "P_Kordes", 5.8, 14.5, 4.0, cm=182, hidden_with=["SH_Handoff"], yaw=180)
person("Varga", "P_Varga", -9.0, 21.0, -2.2, cm=186, hidden_with=["SH_Fishing", "SH_Handoff"])
person("Nora", "P_Nora", 16.8, -48.0, 0.0, cm=160, yaw=180)
person("Brannock_Rope", "P_Brannock", -0.5, 55.5, 0.0, nights=[1], yaw=-90)
person("Brannock_Booth", "P_Brannock", -5.0, -28.0, 0.0, nights=[2, 3], yaw=90)
# timed tableaux: night 2 Varga fishing on the lower landing, night 3 the envelope under the gangway
person("Varga_Fishing", "P_Varga", -8.5, 53.0, -2.2, cm=186, shot="SH_Fishing", extent=(40, 40, 95))
person("Kordes_Handoff", "P_Kordes", 4.2, 8.4, 0.0, cm=182, shot="SH_Handoff", extent=(45, 90, 95), yaw=-90)
person("Varga_Handoff", "P_Varga", 4.2, 7.6, 0.0, cm=186, shown_with="SH_Handoff", yaw=90)

write_spec("/Game/Maps/L_Pier5", "L_Pier5.json")
