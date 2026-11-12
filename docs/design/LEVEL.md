# Level design: Harrowmark, Pier 5 (Case 1)

One harbour map, no loading inside it; the agency office is its own small map (`L_Office`, section 9.1),
reached by a drive. This file owns the layout, metrics, sightlines and night states. Asset ids are in
`docs/art/ASSETS.md`, light and mood in `docs/art/STYLE.md`, the case logic in `CASE01.md`.

## 1. Goals

1. **Short walks.** Walking is free in game time, so real time is the cost. Any two story points are at
   most 45 s apart at walking pace. The car-to-body route is about 37 s. The office is uptown, a drive
   away (a fade and a car shot, 10 minutes of case time each way; CASE01 section 3).
2. **Every record can be checked by standing there.** If a document says someone saw something, the
   player can stand in that spot and see the same thing (section 5). The map is evidence, not
   just a backdrop.
3. **Photo nooks.** Every subject has a clear place to stand within its maximum shooting distance
   (section 6), with nothing in the way and a dark background for the flash.
4. **Light leads the player.** No HUD markers. The player walks from one pool of light to the next and
   steers by four landmarks (section 7). The one missing light, the dark pier edge, is the main
   landmark.
5. **Three nights that read at a glance.** Each night changes a few cheap things (section 8), not the
   geometry.
6. **Fits four days.** Five enterable interiors, all small. Everything else is exterior or seen
   through a window (section 10).
7. **The killer's route is walkable.** The Lighthouse yard, the wicket and the catwalk under the pier are
   how the detective reached the edge unseen (CASE01 1); the player uses the same route as a shortcut every
   night, and the catwalk is the only way to see Varga's lost boathook, the decoy weapon.

## 2. Metrics

| Item | Value |
|---|---|
| Walk / jog speed | 3.0 m/s / 5.0 m/s |
| Playable area | 100 x 120 m (X -50..50, Y -55..65), origin at the root of Pier 5, +Y seaward |
| Pier 5 | 60 x 12 m timber deck, 3.0 m above high water (catwalk 1.0 m + 1.7 m headroom + beams); the edge is the last 5 m |
| Crane cab | 12 m up, jib reach 18 m, swing covers the Agatha's No. 2 hatch and the customs berth |
| Saint Agatha deck | +4 m above the pier; gangway 9 m long, 25 degrees |
| Street lamp spacing | 12-15 m, so there are dark gaps between pools of light |
| Flash range | 6 m (GDD 9.1) |
| Viewfinder | 34 degrees vertical FOV (40 mm on 35 mm film); a subject must fill 15% of the frame height to count, 35% for a sharp print (GDD 9.1) |
| Catwalk | 1.0 m wide, 1.7 m headroom under the deck beams, 1.0 m above the water at high tide; west side, Y 24-52 |
| Doors | 1.2 m; interior ceilings 3.0 m; stairs 18 cm risers |

## 3. Layout

```
 Y
 65 ~~~~~~~~~~~~~~~~~~~~~~~~ open water, breakwater + lighthouse 600 m north ~~~~~~~~~~~~~~~~~~~~~~
                              ________________________
 60                          | EDGE  o lamp (dark,    |   bow "SAINT AGATHA" (name faces the edge)
                             |   x    gouged post)    |   |
 52     [LOWER LANDING]======|ladder  body, tarp, rope|   |
        floating stage       :c   boathook wedged in  |   |  SS SAINT AGATHA
 46                          :a   the pilings 4 m in  |   |  hull X +7..+15
                             :t   from the catwalk    |   |
 35                          :w                       |   |
                             :a  fish crates, nets,   |   |
 24   [CUSTOMS BERTH]========:l  bale hook            |   |  No. 2 hatch + tally clipboard
      pontoon, CL-2 (lit     :k [CRANE] cab 12 m      |===|  gangway at Y 14; watch stand + book at the head
 10    by crane floods)      |  rails along the pier  |   |  aft deckhouse: captain's cabin (deck level)
                             |________________________|   |  gangway foot (quay, under the gangway)
  0  ===================== root of Pier 5 ==================================================
     QUAY ROAD: cobbles, embedded rails, puddles, sodium lamps
-16  ==============================================================  [WAREHOUSE 5] X +15..+45
     [PORT OFFICE] X -40..-15        gate yard
-24   customs office (ground floor)  door faces the booth -> [GATE BOOTH] (-5,-28) windows N and W
-30  ---- dock fence ------------------------------ GATE (barrier) ------- wicket ----- fence ---
     town street (wet, fog closes it at X +-50; police car blocks the east end)
-36            (the car at the curb) ->  [THE LIGHTHOUSE] X +5..+25, door on the NW corner
-52                                       bar (ground), Nora's rooms upstairs (not enterable),
                                           walled YARD on the east side, its gate onto the street
      X  -50            -15        -5  0  +6           +25                                 +50
```

Changes from the earlier map (ASSETS.md section 1 now points here):
- **The customs berth moves next to the pier.** It is now on the west side of Pier 5, under the crane's
  swing. D02 ("CL-2 under the swing") requires this.
- **The port office is inside the fence**, with its door opening onto the gate yard in front of the
  booth window. This is why the gate keeper logged Kordes (A11).
- **The pier is shorter, 60 m instead of 90 m**, for walking time.
- **The agency is not on this map.** It is uptown, on its own small map shared by every case
  (section 9.1). The detective's car stands at the curb outside the Lighthouse: using it is the drive.
- **A catwalk runs under the west overhang** from the customs berth to the lower landing (the killer's
  route, section 22). The Agatha's wheelhouse is cut: the gangway watch book stands at the gangway head.
- **The Lighthouse yard and a harbour-works wicket.** Behind the bar, a walled yard with one bulb over
  the back door and a gate onto the dark east end of the street. Across the street, a padlocked wicket in
  the dock fence opens behind Warehouse 5, out of both booth windows; the agency's dock pass (insurance
  work) carries the key. From there the warehouse's shadow leads to the quay. This is the killer's way in
  and out (his car stood in the yard, S02), and the player's shortcut to the pier through the bar's back
  door.

## 4. Routes

| From | To | Path | Walk |
|---|---|---|---|
| The car (curb outside the Lighthouse) | Body at the edge | street, gate, quay, pier | ~110 m, 37 s |
| Lighthouse yard | Catwalk mouth | yard gate, the dark east end of the street, wicket, behind Warehouse 5, the dark gap at the pier root, ramp, pontoon | ~80 m, 27 s |
| Customs berth | Lighthouse yard | the same way back (the killer's 23:46-23:54) | ~80 m, 27 s |
| The car | Agency office | the drive: a fade and an authored car shot | 10 min of case time each way |
| Body | Lower landing | ladder on the west side, 5 m away | 6 s |
| Customs berth | Lower landing | catwalk under the pier, 28 m, crouched | 14 s |
| Body | Crane cab | 35 m along the pier + ladder (an interaction, fade) | 15 s |
| Crane | Customs berth / CL-2 | ramp down from the pier root | 10 s |
| Pier | Agatha deck | gangway at Y 14 | 8 s |
| Pier root | Gate booth / port office | across the quay | 10 s |
| Gate | Lighthouse bar | across the street | 6 s |

Night start (00:00, nights 2-3): the detective starts in the office; the drive down ends on an authored
arrival shot that frames the night's tableau (section 15). Dawn (06:00) cuts straight to the darkroom;
there is no drive home.

## 5. Sightlines (the map as evidence)

Each line is something a witness or document claims. The player can stand in the witness's place and
see the same thing.

| Claim | Stand at | What the player sees |
|---|---|---|
| Holt: "Nobody after him; I'd have had him in my floods" (S04) | Crane cab | The deck from the root to the last 5 m is under the floods; the edge is the only unlit patch. Holt could not see the murder, but nobody can walk the deck to it unseen: the killer came another way. |
| A10: "splash 23:41 W side?" | Crane cab | The west corner of the edge, 35 m off, black. A splash there is heard, not seen. |
| Nobody saw the killer go to the edge | Crane cab, gate booth | The catwalk is under the deck overhang: invisible from the cab and from the quay. Its mouth at the berth end is a black slot under the deck edge, behind the NO ADMITTANCE board; the crane flood lights the pontoon, not the slot. At 23:05-23:15 the pontoon is empty (Varga at the customs office, Morrow at the bar) and Holt's eyes are on the sling over No. 2 hatch, east of the pier. |
| Nobody logged the detective going out | Gate booth, both windows | The wicket behind Warehouse 5 is east of the booth, outside its north and west windows. The agency's pass keeps it out of the visitors' book anyway (A11 rule). |
| S02: "Your car was in my yard tonight. I thought you'd come in for one." | Bar, back door by the empties | The yard under its one bulb: a car parked there is plain to anyone carrying out the empties. The yard gate opens onto the dark east end of the street, 25 m from the wicket. Nobody sees a man go from the yard to the wicket; the bar's windows face the gate, not the yard. |
| D02: "CL-2 under the swing" | Crane cab | The customs berth lies straight below the jib arc, and CL-2's mooring post with its "CL-2" stencil is in the floodlight. A jib swung full would hit the launch's mast. |
| A09: "eight feet up, nothing to stand on" | Edge | The lamp head at 2.4 m; the nearest bollard 3 m away, the fish crates 20 m back; the curb is 15 cm. |
| A24: gouge "higher than a man's head" | Edge | A fresh pale bite in the post at 2.1 m, on the side facing the deck, where a long swing would land. |
| A20: the gate keeper chalks the launches | Gate booth, north window | The customs berth pontoon across the quay, 35 m away under the crane floods. He chalks boats, not men; the catwalk mouth beside the pontoon is in the deck's shadow. |
| A11: Kordes logged in and out | Gate booth, west window | The port office door, 10 m away. Rule: staff with passes (customs, dockers, the crane gang) are not entered in the visitors' book. Masters and outsiders are. |
| A14: "Master ashore 23:20" | Watch stand at the gangway head | The gangway, its foot and the quay down to the gate. |
| S02: Morrow left the bar for the pier | Bar window | The gate and the quay. The edge itself is hidden behind the Agatha's bow. |
| A05: "Meet H.M. 23:30, P5" | Pier edge | A dark spot with no witness: the Agatha's bow blocks the deck, the lamp is out, and the crane cab looks the other way during lifts. This is why the killer chose it. |
| A06: boathook under the pier | Catwalk, north end | Seen only from the catwalk, wedged in the pilings 4 m in, in total dark. The flashlight shows the brass glint; the stencil under the tar only shows on the enlargement (D04). |
| A23: Varga fishing (night 2) | Edge, looking down over the curb | The lantern on the floating stage, the pike pole probing under the deck. Also a moving glow under the pier in the night-2 arrival shot. |
| A25: the hand-off (night 3) | Quay by the pier root | The shadow under the gangway at its foot, at the edge of the deck work light's spill: two men, one envelope, a match struck for Kordes's cigarette. The night-3 arrival shot, from the car at the gate, shows only the match. |

Rule for later changes: if a sightline in this table breaks (a mesh moves, a wall goes up), the claim
breaks too. Keep the table up to date.

## 6. Photo nooks

The maximum distance comes from the 15% frame-height rule at 34 degrees: d_max = 11 x subject height,
capped at the 6 m flash range unless the subject is lit. A sharp print needs 35%: d_sharp = 4.7 x subject
height (same cap). Beyond d_sharp the print is grainy: it gives its anchors but no detail, and the shot can
be retaken for a plate (GDD 9.1). The subject bounds are what `UPhotoSubjectComponent` measures, so they
must enclose the whole framed thing (the hand with the watch, not just the watch). Every nook leaves room
to stand inside d_sharp.

| Shot | Subject bounds | Lit by | Stand within | Sharp within | Nook |
|---|---|---|---|---|---|
| SH_Frame1 | - | - | - | - | No nook: pre-exposed before the game (CASE01 4.1); taken from the deck at about 2 m with the flash, always sharp |
| SH_Wound | Head and shoulders, 0.45 m | Flash only | 4.9 m | 2.1 m | Kneel at the tarp; Brannock lifts the corner on the forced shot |
| SH_Watch | Forearm and hand, 0.35 m | Flash | 3.8 m | 1.6 m | Same spot, the arm is out from under the tarp |
| SH_Notebook | Open notebook, 0.25 m | Flash | 2.7 m | 1.2 m | Laid out by Brannock on a crate lid beside the body, under his lantern and a tarp lean-to |
| SH_Lamp | Lamp head with the loose bulb, 0.6 m | Flash | 6 m | 2.8 m | From the bollard at the edge, looking up |
| SH_Gouge | Gouge in the post, 0.3 m | Flash | 3.3 m | 1.4 m | At the foot of the lamp post, looking up |
| SH_Boathook | Boathook in the pilings, 2.4 m | Flash | 6 m | 6 m | North end of the catwalk, the only place it can be seen from |
| SH_Bottle | Broken bottle, 0.3 m | Flash | 3.3 m | 1.4 m | On the planks 4 m from the body, in a puddle (the flash reflects in it) |
| SH_BaleHook | Hook on the top crate, 0.5 m | Pier lamp | 5.5 m | 2.4 m | Fish crates at Y 35 |
| SH_CraneLog | Clipboard, 0.35 m | Cab bulb | 3.8 m | 1.6 m | Cab interior, 2 m deep |
| SH_GateBook | Book on the counter, 0.4 m | Booth bulb | 4.4 m | 1.9 m | Through the open hatch of the booth |
| SH_Movements | Chalkboard, 1.2 m | Booth bulb | 13 m | 5.6 m | Through the booth's north window or the hatch |
| SH_CashTape | Register with the tape, 0.4 m | Bar practicals | 4.4 m | 1.9 m | Behind the bar's end, Nora steps aside |
| SH_Slate | Credit slate on the back bar, 0.6 m | Bar practicals | 6.6 m | 2.8 m | Across the counter, next to the mirror |
| SH_Mirror | The detective's reflection, hat to shoes, 1.85 m | Bar practicals | 20 m reflected path | 8.7 m reflected path | At the register end, facing the glass; the reflected path is twice the distance to the glass, so stand within 4.3 m of it |
| SH_GangwayBook | Book on the watch stand, 0.4 m | Stand bulb under a tin hood | 4.4 m | 1.9 m | Gangway head |
| SH_Tally | Clipboard on the hatch coaming, 0.35 m | Deck work light | 3.8 m | 1.6 m | No. 2 hatch |
| SH_Crates | Crate stack under a tarp, 1.0 m | Oil lamp | 11 m | 4.7 m | Cabin (gated) |
| SH_CabinDesk | Desk top with both papers, 0.8 m | Oil lamp | 8.8 m | 3.8 m | Cabin (gated) |
| SH_Locker | Open locker, 1.8 m | Desk lamp | 19.8 m | 8.5 m | Customs office (gated) |
| SH_Roster | Duty board, 1.0 m | Desk lamp | 11 m | 4.7 m | Customs office wall |
| SH_Draft | Report on Morrow's desk, 0.3 m | Desk lamp | 3.3 m | 1.4 m | Customs office |
| SH_StoreBook | Book open on the front counter, 0.4 m | Desk lamp | 4.4 m | 1.9 m | Customs office, the counter inside the door |
| SH_Tripod | Tripod standing, 1.5 m | Desk lamp | 16.5 m | 7 m | Agency office, by the darkroom door; anywhere in the room frames it |
| SH_Meridian | Print on the drying line, 0.3 m | Desk lamp | 3.3 m | 1.4 m | Agency office: the line is strung across the room in front of the darkroom door, outside the red light |
| SH_Drawer | Open drawer with the envelope, 0.5 m | Desk lamp | 5.5 m | 2.4 m | The detective's desk; the drawer opens on inspect like every drawer in the office |
| SH_Fishing | Varga on the stage, 2.0 m | His lantern | 22 m | 9.4 m | The edge, looking down; no flash at that range |
| SH_Handoff | Two men at the gangway foot, 2.0 m | Spill of the deck work light, a struck match | 22 m | 9.4 m | Shadow of the crane leg at the pier root, 9 m off; no flash at that range |

Each nook has a dark or plain background behind the subject so the flash falloff reads on the print. No
subject sits under a red light (red stays the investigation signal; the darkroom is the only red room).

**The mirror shot.** The subject of SH_Mirror is a proxy volume that mirrors the player's capsule
through the glass plane, so the validity check runs as for any subject. The print is the rendered view:
the first-person body is hidden in the main view and kept visible in reflections, so the RT reflection
shows the detective, camera raised. The proxy only exists while the player stands on the bar side of the
glass; there is no other mirror on the map.

## 7. Navigation landmarks

1. **The crane.** The tallest thing on the dock, with two fixed floodlights. You can see it from
   everywhere on the map.
2. **The cyan neon "THE LIGHTHOUSE"** marks the bar and the car at its curb: the way out. It reflects in
   every puddle on the street.
3. **The lighthouse beam** on the breakwater shows where the sea is (north).
4. **The dark edge.** Pier 5 is lit along its length and black at the end. The player is drawn to the
   gap, and it is the first clue (A09).

There are no markers, compass or minimap. Doors that can be used have a warm light over them. Doors
that cannot are dark and unlit.

## 8. Night states

| | Night 1 (02:00-06:00) | Nights 2-3 (00:00-06:00) |
|---|---|---|
| Weather | Heavy rain, low fog | Night 2: drizzle and thick fog. Night 3: rain and wind, waves at the pilings |
| Pier edge | Body under a tarp, police rope, a hurricane lantern on the rope post (low light) | Rope only, the chalk outline half washed away, the lantern gone |
| Coroner's van | At the gate, headlamps on the quay; leaves at 06:00 | Gone, with tyre tracks in the puddles |
| Gate booth | Keeper away at the van, hatch open; chalkboard shows Friday's movements | Brannock inside; board wiped and re-chalked for the night ("SAT 12 NOV", "SUN 13 NOV"): "CL-2 HELD - POLICE" |
| Brannock | At the rope | In the gate booth |
| Crane | Parked since the police stopped the discharge at 01:10; floods fixed, not swinging | Same |
| Agatha | Police seal on No. 2 hatch; Kordes at the top of the gangway; funnel cold | Night 2: smoke from the funnel, mooring lines singled up. Night 3: the tug MARGARET alongside, steam at the whistle and one blast of the whistle at each half hour. Cabin open after S07 |
| Varga | Beside CL-2 on the customs berth | Same (CL-2 held by the police), except: night 2, from night start to 01:30 on the clock (or until S18), on the lower landing with a lantern and a pike pole; night 3, from night start to 01:30, under the gangway with Kordes |
| Kordes | Top of the gangway | Same; night 3, from night start to 01:30, under the gangway |
| Holt | Under the crane awning, smoking | Same |
| Nora | Behind the bar | Same |
| Customs office | Lit, unlocked, empty | Same; locker open once S08 is stamped LIE |

The ship is the clock: the player sees the Agatha getting ready to sail without a single timer on
screen.

Implementation: one persistent level without World Partition (the map is small) and streamed sublevels
for the night states: `L_Pier5_N1` (body, tarp, lantern, van, keeper prop, Friday chalkboard),
`L_Pier5_N23` (outline decal, wiped board, Brannock at the booth), `L_Pier5_N2` and `L_Pier5_N3` (the
Agatha's departure dressing). Story-state changes (cabin, locker, NPC moves, the two timed tableaux) are
actor states driven by the investigation subsystem and the clock, not sublevels. The weather is set per night by parameters in `MPC_World`.

## 9. Areas

### 9.1 Agency office and darkroom (hub, uptown, its own map `L_Office`)
- **Layout:** 8 x 6 m. Three desks (only the detective's is lit), the corkboard wall facing the
  desk, a tall north window, the phone. The darkroom (3 x 3 m) is behind a door with a red "IN USE" lamp,
  the only red source, lit only while film develops (at dawn, or when the player develops mid-night).
  A door to the landing and the street stair; the car waits at the curb below (the door is the drive).
  The same set serves every case: only the board, the drawer and the drying line change.
- **The killer's things, in plain sight:** the tripod by the darkroom door, standing a little crooked
  (one leg sawn short, the cut pale, no brass foot); the drying line across the room in front of the
  darkroom door with the Meridian print on it; the desk drawer with Kordes's envelope. None of them is
  flagged: no glint, no prompt beyond the ordinary viewfinder brackets (CASE01 4.1).
- **Window:** looks down on an uptown street: wet tram rails and a cinema's neon below, which throws the
  gold-leaf lettering's shadow across the floor and the board (section 23). The opening shot of the game
  frames it during the phone call. The harbour is not in view: Harrowmark's docks are a drive away.
- **The board:** the wall is the same board as on Tab. Standing at it opens it.

### 9.2 The Lighthouse bar
- **Layout:** 12 x 9 m, the counter along the east wall, a back-bar mirror, 4 tables, the jukebox, the
  register at the counter's end. The door is on the northwest corner, facing the gate: a dockers' bar
  at the dock gate. The back door by the coat rack opens onto the yard; the stair to Nora's rooms is
  private.
- **The mirror:** an old full-height pier glass the back bar was built around, at the register end,
  between the gin and the rye. It faces the street door, so the player meets his own reflection every
  time he comes in. It shows him hat to shoes (D06 needs the trouser cuffs).
- **Light:** amber practicals plus the cyan neon through the windows. The mirror reflects the room
  and the detective (RT reflection showcase, section 6).
- **Slate:** the credit slate hangs on the back bar beside the mirror: "No credit." in fresh chalk, and
  under it, old and never rubbed out, "E.M. $40" (A12).

### 9.3 Street and gate
- A 14 m wide wet street. The fence has a single gate with a barrier. The town is closed off by fog at
  both ends, and a police car blocks the east end.
- **Gate booth:** a wooden kiosk, 2.5 x 2 m. It cannot be entered: the player uses it through an open
  hatch (book) and sees the chalkboard through the north window.

### 9.4 Port office (customs office inside)
- A two-storey brick building. Only the ground-floor customs office (7 x 5 m) can be entered: two
  desks, filing cabinets, a row of lockers (Varga's has a padlock), the duty board. The harbour master's
  office upstairs is a lit window with blinds.

### 9.5 Quay and Warehouse 5
- Cobbles with rails, puddles, oil drums (rust brown), pallets. Warehouse 5 is exterior only. Its loading
  dock is a sheltered spot for the player to raise the camera out of the rain (useful for the trailer).

### 9.6 Pier 5, edge and lower landing
- **Deck:** planks on pilings with bollards along both sides and rails for the crane. The fish crates
  and nets at Y 30-38 break the long sightline; a docker's bale hook lies on the top crate (A08).
- **Edge:** the last 5 m. The lamp post (A09) stands on the west corner, with a fresh gouge at 2.1 m
  (A24). The Agatha's bow overhangs the east side.
- **Lower landing:** a ladder down the west side to a floating stage beside the pier, open to the sky.
- **Catwalk:** a plank walkway on the pile caps under the west overhang, from the berth pontoon (Y 24) to
  the landing (Y 52), for the harbour works. Pitch black, crouched, first person. The boathook is wedged
  in the pilings 4 m in from its north end. The flashlight shows the pilings; the brass hook glints.
  The tripod's brass foot is on the bottom off the edge and is never found.

### 9.7 Crane
- A portal crane on rails at Y 20. A ladder to the cab is an interaction with a short fade. The cab
  (2 x 2 m) has the seat, levers and the log clipboard, with windows on four sides. The player can turn
  inside it and look out (the main sightlines in section 5).

### 9.8 Customs berth and CL-2
- A pontoon on the west side at Y 8-24, reached by a ramp from the pier root. CL-2 (9 m) is moored to a
  post stencilled "CL-2". Its boathook rack is empty and readable in inspect. Varga stands on the
  pontoon. The crane floods light it. The catwalk starts at the pontoon's north end, behind a
  "HARBOUR WORKS - NO ADMITTANCE" board.

### 9.9 SS Saint Agatha
- Only the pier side of the hull is detailed. The gangway at Y 14 leads to the aft deck.
- **Gangway head:** the watch stand, a lectern under a tin hood with a bulb and the gangway watch book
  (A14). The watch, A. Petrie, is "below for cocoa" and never seen.
- **No. 2 hatch:** under the crane jib, with a police seal and the tally clipboard.
- **Aft deckhouse:** one block at deck level with the captain's cabin (5 x 4 m) behind a door with a
  brass "MASTER" plate, locked until S07.
- **Gangway foot:** on the quay, in the shadow under the gangway at the edge of the deck work light's
  spill; the night-3 tableau spot.
- The bow and the painted name face the pier edge, so the ship that holds the secret looms over the
  murder spot.

## 10. Scope and build order

**Enterable interiors (5):** agency office + darkroom (`L_Office`, shared by every case), bar, customs office, captain's cabin, crane cab.
The catwalk is an exterior. The gate booth and the harbour master's office are seen through a window
only.

**Day 1 greybox (Must):** BSP or modelling-tool blocks at final metrics; all subject actors and NPC
spots placed; both night layers; the four landmark lights; walk all routes in section 4 with a stopwatch.

**Day 2 lighting (Must):** the hero areas lit at final quality by the end of day 2 (edge, catwalk, crane
over the berth, street, darkroom): the NVIDIA case rests on them, so they are not left to day 4.

**Cut order if late:** the crane cab becomes a fixed camera view (no walking inside); the catwalk
becomes a straight plank run with no props; warehouse 5 becomes a facade; night-3 weather becomes the
same as night 1.

## 11. NVIDIA hero shots

Locked camera positions for screenshots, the trailer and the judges' first minute:

1. **Flash at the edge.** Flash pop over the body in rain: lit droplets, the RT reflection of the flash
   in the wet planks and in the Agatha's hull paint.
2. **Neon street.** The Lighthouse from the gate: cyan neon and amber windows doubled in the cobble
   puddles, Nora's window lit above.
3. **Crane over CL-2.** From the cab: floodlight cones in volumetric fog, the launch below, the
   dark pier end beyond.
4. **Under the pier.** From the catwalk: the flashlight beam between the pilings, light off the water
   rippling on the underside of the deck, the boathook glint.
5. **Darkroom.** Safelight red, the print fading up in the tray, the enlarger beam.
6. **Office window.** The detective's silhouette against the rain-streaked glass, the neon pulsing
   on the board.

Shots 1, 2 and 6 fall inside the first five minutes (office window during the call, the street on
arrival, the forced wound photo). A judge who plays ten minutes sees half the showcase.

## 12. The first walk: a line-up of suspects

The route from the car to the body (section 4) passes every suspect, each in their own pool of light,
before the player reaches the body. The player meets the cast on the way in without a word of
exposition:

| Order | Where on the route | Who | How the player sees them |
|---|---|---|---|
| 1 | Street, getting out of the car | Nora | Through the bar window: behind the counter, she nods; the jukebox plays to an empty room |
| 2 | Gate | (the keeper is away at the van) | The booth hatch is open, the bulb is on and the chalkboard is readable from the path |
| 3 | Quay, looking left from the pier root | Varga | On the pontoon under the crane flood, by CL-2, a lantern at his feet |
| 4 | Quay, looking right | Kordes | At the top of the gangway under the deck work light, watching the rope |
| 5 | Pier, Y 20 | Holt | Under the crane awning, a cigarette glowing; he turns his head as the player passes |
| 6 | The edge | Brannock | At the rope by the lantern, the tarp at his feet |

The camera stays slung until the rope. The first plate the player fires is always the wound, so the
tutorial is never skipped. On the way back every subject on the route is already familiar.

The first face on the route is not in the table: through the bar window the back-bar mirror holds a
man in a hat on the wet street. Nothing points at it.

## 13. Night 1 on the map

The careful reference path (CASE01 section 9), reordered by space. The data order is logical; a player
walks in loops:

| Loop | Places | Steps (careful path, night 1) |
|---|---|---|
| 1 | Edge, ladder, catwalk | S01, SH_Wound, WRONG, S15, SH_Watch, SH_Notebook, down the ladder and along the catwalk: SH_Boathook |
| 2 | Crane (the catwalk comes out at the berth below it) | S04, SH_CraneLog |
| 3 | Agatha: gangway head, cabin | S06, SH_GangwayBook, LIE, S07, SH_CabinDesk, SH_Crates |
| 4 | Customs berth | S08 |
| 5 | Gate booth | SH_Movements, LIE (S08) |
| 6 | The board (Tab), on the way back by the wicket | Pin Varga / A06 / A16; red A22-WHO: BREAK, red A20-WITH: BREAK |
| 7 | Bar, then the drive | SH_Mirror at the register; drive up to the office: SH_Tripod, pin the detective / A28 |

About 550 m of walking, 3 minutes. A rough real-time estimate for night 1: 3 min walking, 3 min talking,
2 min shooting, 6 min reading, 6 min at the board, about 20 minutes. A first-time player needs two or
three nights, 30-45 minutes for the whole case, and most of them stop after loop 6 with Varga pinned.

The fast path (the knowing player, a replay) is three stops: the edge, the bar mirror, the office. About
250 m and 8 minutes for night 1.

The last loop goes from the board down to the mirror and back up: once the board is empty of suspects,
the way home passes the only face left.

## 14. NPC staging

Each NPC stands under their own light, sheltered from the rain (so talk shots do not need wet hair), and
faces the route so their face catches the light as the player approaches. Talk starts within 2.5 m.

| NPC | Spot | Light | Idle | Changes |
|---|---|---|---|---|
| Brannock | Rope post at the edge (N1); booth (N23) | Hurricane lantern; booth bulb | Hands in the cape, stamps his feet | Once S08 is stamped LIE: in the customs office by the open locker |
| Nora | Behind the counter, at the register end | Back-bar practicals | Polishing a glass, glancing at the door | - |
| Holt | Crane awning, Y 20 | Cigarette glow + the flood spill | Smoking, looking up at the stopped jib | - |
| Kordes | Top of the gangway | Deck work light | Watches the rope, then the player | After S07: in his cabin, by the desk |
| Varga | Pontoon by CL-2 | Crane flood + his lantern | Coiling a rope that is already coiled | Tableaux (section 8); otherwise stays until the ending (he is not arrested before the report) |

The two tableaux (Varga fishing, the hand-off) are posed scenes. The clock moves only on actions (GDD
9.5), so neither scene starts mid-night: both are already in place when the night starts (set during the
dawn cut) and clear off screen once the clock passes 01:30, like any other NPC move. The men in the
hand-off do not react to the player. Varga on the landing turns his back until A23 is taken; after that
he answers S18, and S18 ends the scene: he walks back to the berth (off screen).

All NPC moves happen off screen: the new spot is filled when the player is out of sight of both the old
and the new spot, or at dawn.

## 15. Affordances and lures

Things the player must find, and what draws them there without markers or instructions:

| Target | Lure |
|---|---|
| Lower landing | A white lifebuoy on the rail at the ladder top, the only white object at the edge; the ladder rungs are polished wet by the lantern |
| Catwalk | Its mouth behind the "NO ADMITTANCE" board at the berth end, the one gap in the pontoon rail, a black slot under the deck edge that the flood does not reach; from the landing, its far end is another black slot; Holt's "nobody after him" (S04) asks how the killer got out there |
| Boathook under the pier | The empty boathook rack on CL-2 (readable in inspect); the gouge and D01 say "long"; foam and debris drift under the deck (section 22); from the catwalk, the flashlight glint |
| Crane cab | A lit cab window 12 m up, the only lit window on the pier; Holt's "check my log" (S05) |
| Gangway watch book | The hooded bulb on the stand at the gangway head, the warmest light on the ship's side |
| Captain's cabin | A brass plate "MASTER" on a locked door beside the gangway head |
| Varga fishing (night 2) | A lantern moving at water level under the pier, framed by the night-2 arrival shot; the pike pole knocking on the pilings, heard from the edge and the crane |
| The hand-off (night 3) | At night start Kordes is gone from his post at the gangway head; in the night-3 arrival shot a match flares under the gangway and shows two faces |
| Varga's locker | The one padlocked locker in the row, with a name card "L. VARGA" |
| Movements chalkboard | It faces the path through the booth window under the brightest bulb at the gate |
| Back-bar mirror | Nothing in the world points at it; it faces the street door, so the player meets it every time he comes in. The case points at it: an empty WHO after the red threads, "Meet H.M.", Nora's "your car was in my yard", frame 1 on the board |
| Tripod, drying line, drawer | The hub the player crosses every dawn; the tripod stands crooked, the print hangs at eye height, the drawer opens like every other drawer. The ash splinters (D01) and "J.K." on the envelopes lead back to them |

A locked door looks locked: dark, a padlock or a brass plate, no warm light. It never just fails
an E press.

## 16. Expiry cues

The player is never told what expires, but the world shows it:

- **Body, watch, notebook:** the coroner's van with its engine running at the gate; Brannock: "The van takes
  him at six" (S15).
- **Movements board:** chalked header "SHIFT 18:00-06:00"; a wet rag hangs on the board's frame.

Missing the board is survivable: S08 is also refuted by D02 (the redundancy table in CASE01).

The tableaux are not announced either. They last 90 minutes of game time each and are lured (section 15);
neither is needed to close the case, and each is one of several ways to its card's job (A23 points to the
weapon, A25 is one of four motives).

## 17. Flavour props (inspect only, no cards)

They give the place a history. Rule: a flavour prop never states a time, a place or a person's
movement. Anything like that is a card and belongs in CASE01, or it is cut. Inspect text is one dry
line from the detective.

| Area | Props |
|---|---|
| Agency office | The landlord's rent reminder on the desk ("Second notice."), a framed press print from an old case, an empty desk with a typewriter under a cover, coffee rings on the blotter |
| Bar | Morrow's umbrella on the coat rack (he left without it) |
| Quay | A police bicycle against the fence, rain-swollen newspapers in a drum |
| Pier | Cigarette butts piled under the crane awning, a coil of mooring line, a gull on a bollard |
| CL-2 | A thermos, a wet coil of rope, a logbook locked in the cabin (not readable) |
| Agatha | Crew laundry under the deckhouse eave, a police seal on No. 2 hatch dated 12 NOV |
| Customs office | Morrow's coat on its hook, a cold cup on his desk, a calendar with Monday ringed ("Holt - report") |

## 18. Movement and camera rules in the space

- **No falling in.** Pier edges have a 15 cm timber curb and a blocking volume. Water is scenery.
- **Ladders and the crane** are interactions with a 0.5 s fade, not climbing animations.
- **Tight spaces** (crane cab, booth hatch, captain's cabin, catwalk, darkroom) switch the gameplay
  camera to first person. The third-person camera has collision against the Agatha hull and crane legs, and pulls in
  under awnings.
- **Rain shelter.** The awnings (crane, warehouse dock, booth eave, deckhouse) have a no-rain volume. The
  rain sound drops and the camera lens stays dry under them.
- **Camera raise anywhere.** Every nook in section 6 leaves 1.5 m of free space behind the stand point,
  so the first-person viewfinder does not start inside a wall.

## 19. Audio map

| Source | Where | Job |
|---|---|---|
| Foghorn from the breakwater, every 90-120 s (STYLE.md) | North | Direction to the sea |
| Jukebox, muffled through the walls | Bar, audible on the street | Direction to the car |
| Rain layers: tarp, planks, steel, cobbles, glass | By surface | The world sounds wet under the feet |
| Crane: cable creak in the wind | Pier, Y 20 | Landmark |
| Water slapping the pilings, nothing else | The edge | The quietest place on the map: the murder spot is silent |
| Coroner's van engine idling | Gate, night 1 | Expiry cue (section 16) |
| Pike pole knocking on wood, a man swearing under his breath | Under the pier, night 2, 00:00-01:30 | Tableau lure (section 15) |
| Ship's whistle, one blast at each half hour | The Agatha, night 3 | The clock: the player hears the half hours pass (the clock moves only on actions) |
| Phone ringing | Office | Prologue |

## 20. Dawn and endings in the existing spaces

- **Dawn.** 06:00 cuts to the darkroom. After the loupe, the detective stands at the board while the
  office window behind him goes from black to blue hour (the sky lifts from 05:00, STYLE.md), and the
  monologue plays. The next night starts in the office; the drive down ends on the arrival shot.
- **Reconstruction** (CASE01 8) comes first in every ending: the player's own prints as a slideshow,
  no set. Its last frame is the back-bar mirror, the player's print or, if he never took it, the live
  view over the detective's shoulder at the register.
- **Endings** are staged as 2-3 locked shots in places that already exist, using the night layers, with
  no new sets:

| Ending | Shots |
|---|---|
| E1 Whole truth | Booth: Brannock reads the carbon twice; the detective holds out his wrists. Gangway: Kordes walks down in cuffs a few minutes behind him; the tug casts off without her. Bar: the mirror, an empty stool |
| E2 Confession | Booth: the carbon, the wrists. Quay: the Agatha's lights move away on the tide. Office: the drawer, the envelope still in it |
| E3 Not on film | Booth: Brannock hands the report back. Office: the tripod a little crooked by the darkroom door. Quay: the Agatha's lights move away |
| E5 Misadventure | Quay: the Agatha's lights move away. Office: fifty a day in an envelope on the desk; the phone doesn't ring |
| E4 The wrong man | Quay: the accused (usually Varga) in the police car. Bar: the detective counts the fifties and looks up into the mirror |

## 21. Calendar

The murder is on the night of Friday 11 November 1949; 11 Nov 1949 was a Friday, as the stamp and the
logs say.

| Night | Dates | Board header (N23) |
|---|---|---|
| 1 | Fri 11 / Sat 12 Nov, 02:00-06:00 | Friday's board, still unwiped |
| 2 | Sat 12 / Sun 13 Nov, 00:00-06:00 | "SAT 12 NOV" |
| 3 | Sun 13 / Mon 14 Nov, 00:00-06:00 | "SUN 13 NOV" |

The Agatha is held as a witness ship until the inquest sits on Monday at noon; her charter needs the
Monday morning tide. A named killer before then gets her released, which is why the report must go by
dawn after night 3. Morrow's "Report Monday" (A03) lands on the day the ship sails, a free echo.

## 22. The killer's route and the boathook

**The killer's route.** At 22:55 the detective left his car in the Lighthouse yard and did not go into
the bar. At 23:05 he went out by the yard gate, across the dark east end of the street, through the wicket, behind Warehouse 5 to the quay, over
the dark gap at the pier root, down the ramp and along the empty pontoon into the catwalk mouth, with the
camera on its tripod. Up the landing ladder to the edge; at 23:15 he unscrewed the lamp bulb from the
deck. After 23:41 he went down the ladder to the lower landing (the wash to his shins, D06), back along
the catwalk and the same way to the car at 23:54, and was in the office uptown by 00:10. Every leg of it is a route the player walks
(section 4), and the board never asks about it: no witness, no record, no card. The route is the
negative space of the case.

**The boathook, the decoy.** At 16:30 Varga lost CL-2's hook over the side at the west face of Pier 5
(A20, A26). The ash shaft floated, the onshore wind and the current carried it in under the deck, and it
wedged in the pilings 4 m in from the catwalk's north end. The level shows the drift: foam lines and
floating debris (a crate slat, weed) run in under the deck and collect at the same pilings. That is a lure
(section 15), and it explains why a CL-2 hook sits under a dead man, and why Varga is back on night 2 with
a pike pole: he knows how it looks. The pilings hold it: he never gets it out.

## 23. Agency signage

The office window carries gold-leaf lettering, read from the street and mirrored from inside. At night
the neon from below throws the lettering's shadow across the office floor and the board, which is the
classic noir shot and costs one decal plus a light function. The gate book (T_C08) already names the agency:
at 02:00, with the keeper away at the van, the detective signed himself in as "HarrowMark agency",
visiting Sgt. Brannock. His pass doesn't require it. It is the only agency line in the book, and it was
written after the murder. The window carries the same name: "HARROWMARK / PRIVATE INQUIRIES".
The agency is named HarrowMark (owner, 2026-11-12; replaced the working name "Night Shift Bureau"), so the window, the game logo and the gate book read alike. If it changes, re-render T_C08 and swap the lettering decal
(E60); nothing else references it.

## 24. Making the level testable

The claims in this file become actors and automation tests, so moving a mesh cannot silently break the
case:

| Marker actor | Placed at | Test |
|---|---|---|
| `APhotoNookMarker` (one per shot id except SH_Frame1) | Each stand point in section 6 | `Level.PhotoNooks`: from every marker, the subject passes the full validity check (frustum, 15%, 60% unoccluded, flash range or lit); SH_Mirror runs with the player capsule placed at its marker, against the mirrored proxy |
| `ASightlineMarker` pair (one per row in section 5) | Witness spot, target | `Level.Sightlines`: line traces from the eye to the target sample points are clear |
| `ARouteMarker` (car, yard, edge, landing, catwalk ends, crane, gangway, berth, booth, bar) | Route ends in section 4 | `Level.Routes` (Could): the nav path length between pairs stays under the walk budget (45 s at 3 m/s = 135 m) |
| Night sublevels and tableau actors | - | `Level.NightStates` (Could): each night loads the right sublevels; every expiring subject exists on night 1 and is absent on nights 2-3; each tableau subject exists only inside its window |

`PhotoNooks` and `Sightlines` are Must and run with the `Case01Solvable` test in the GDD section 12
suite; the greybox counts as done when they pass. `Routes` and `NightStates` are Could: a stopwatch walk
and a playthrough cover them if time is short.

