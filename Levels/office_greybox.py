"""Generate the agency office greybox spec (Levels/L_Office.json) for `jf level`.

The hub of every case: the agency office uptown, upstairs above a shop (LEVEL section 9.1, STYLE "Agency
office"). Same frame as the harbour (metres, X east, Y north, Z up, street at 0), own origin. The door to the
landing is the drive (ACaseDriveActor); the player arrives at PlayerStart "Office" and wakes at dawn at
PlayerStart "Dawn" in the darkroom (AHarrowGameMode).
    python Levels/office_greybox.py && python <JamForge>/jf.py level Levels/L_Office.json
"""
from greybox_lib import *  # noqa: F401,F403 (helpers, colours, the shared actors list)

SAFELIGHT = [142, 15, 22, 255]  # STYLE "Safelight red" #8E0F16: the darkroom only
FLOOR = 3.7                     # the office is the first floor; the shop is below

# ---------------------------------------------------------------- the office (8 x 6 m) and the darkroom (3 x 3 m)
room("AgencyOffice", 0, 8, 0, 6, FLOOR, CEIL,
     holes={"N": [window(4.0, 3.0, 0.6, 2.8)], "E": [door(1.5)], "S": [door(7.0)]})
room("Darkroom", 5.5, 8.5, -3.2, -0.2, FLOOR, CEIL, holes={"N": [door(7.0)]})
F = "Interiors/AgencyOffice"
# three desks: the detective's facing the room, two colleagues' under the window (set dressing)
box("Desk_Detective", 3.5, 5.3, 1.5, 2.4, FLOOR, FLOOR + 0.75, folder=F)
box("Desk_ColleagueW", 0.6, 2.4, 3.8, 4.7, FLOOR, FLOOR + 0.75, folder=F)
box("Desk_ColleagueE", 5.6, 7.4, 3.8, 4.7, FLOOR, FLOOR + 0.75, folder=F)
box("Phone", 4.9, 5.2, 2.0, 2.3, FLOOR + 0.75, FLOOR + 0.85, folder=F)
# the corkboard wall (the board UI zooms into it)
box("Board", 0.05, 0.25, 0.8, 5.0, FLOOR + 0.9, FLOOR + 2.5, folder=F)
# the drying line across the room in front of the darkroom door, outside the safelight
box("DryingLine", 5.6, 8.0, 0.88, 0.92, FLOOR + 1.88, FLOOR + 1.9, folder=F)
F2 = "Interiors/Darkroom"
box("Darkroom_Bench", 5.6, 8.4, -3.1, -2.4, FLOOR, FLOOR + 0.9, folder=F2)
box("Darkroom_Enlarger", 7.6, 8.2, -3.0, -2.5, FLOOR + 0.9, FLOOR + 1.9, folder=F2)

# the building: the shop below, the stair landing east of the office door, the roof
B = "Greybox/Building"
box("Shop", -0.2, 8.2, -3.4, 6.2, 0.0, FLOOR - 0.2, mat=GRID2, folder=B)
box("Landing", 8.2, 10.2, 0.6, 2.4, FLOOR - 0.2, FLOOR, folder=B)
box("Building_Upper", -0.2, 8.7, -3.4, 6.2, FLOOR + CEIL + 0.2, FLOOR + CEIL + 1.0, mat=GRID2, folder=B)

# ---------------------------------------------------------------- the uptown street under the window
U = "Greybox/Uptown"
box("Street", -30, 40, -6, 30, -0.2, 0.0, folder=U)
box("Pavement_Near", -30, 40, 6.2, 8.2, 0.0, 0.15, folder=U)
box("TramRail_A", -30, 40, 12.9, 13.0, 0.0, 0.03, folder=U)
box("TramRail_B", -30, 40, 14.3, 14.4, 0.0, 0.03, folder=U)
box("Pavement_Far", -30, 40, 19.0, 21.0, 0.0, 0.15, folder=U)
box("Opposite_Block", -30, 40, 21.0, 28.0, 0.0, 14.0, mat=GRID2, folder=U)
# the cinema across the street: its marquee neon throws the window lettering's shadow on the floor
box("Cinema_Marquee", -1.0, 9.0, 20.2, 21.0, 3.2, 4.4, folder=U)
for i, x in enumerate((-14, 4, 22)):
    post(f"StreetLamp_Post{i}", x, 7.6, 0, 5.0, 0.15, folder=U)
    light(f"StreetLamp{i}", "point", x, 7.6, 4.9, SODIUM, 30, 13)
light("Cinema_Neon", "point", 4.0, 19.8, 3.8, CYAN, 40, 18)

# ---------------------------------------------------------------- lights inside
light("DeskLamp", "point", 4.4, 2.0, FLOOR + 1.1, WARM, 8, 6)
light("Landing_Bulb", "point", 9.2, 1.5, FLOOR + 2.6, WARM, 4, 4)
light("Safelight", "point", 7.0, -1.7, FLOOR + 2.4, SAFELIGHT, 3, 4)

# ---------------------------------------------------------------- case actors
# the office subjects (LEVEL section 6): tripod by the darkroom door, the Meridian print on the line, the drawer
subject("SH_Tripod", 6.2, 0.5, FLOOR + 0.75, (45, 45, 150), (30, 30, 78), lit=True)
subject("SH_Meridian", 7.3, 0.9, FLOOR + 1.72, (25, 1, 30), (16, 8, 18), lit=True)
subject("SH_Drawer", 4.4, 1.35, FLOOR + 0.6, (50, 30, 12), (28, 22, 12), lit=True)
# the door to the landing and the street stair: the drive down to the case
drive("OfficeDoor", 8.1, 1.5, FLOOR, (12, 120, 220))
player_start("Office", 7.0, 1.5, FLOOR, 180)
player_start("Dawn", 7.0, -1.4, FLOOR, 270)

write_spec("/Game/Maps/L_Office", "L_Office.json")
