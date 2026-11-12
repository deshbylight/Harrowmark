# Art style: HarrowMark

Colour neo-noir. Harrowmark, November 1949, a port at night in steady rain. Everything is wet, dark and
lit by a few warm practical lights; the picture is built from reflections. The detective's camera cuts
into that darkness: every photograph is a hard white flash that freezes the rain for a split second, and
the evidence the player keeps is black-and-white press prints with grain.

One rule above all: **red means the investigation.** Red thread, LIE and WRONG stamps, the darkroom
safelight. No other red in the world (no red signs, red cars, red clothes, red barrels). If a prop must be
red in life, make it rust brown or oxblood (#4A1C1C or darker) so it never reads as signal red.

Visual references for mood only (do not copy shots or designs): rain-soaked harbour night photography,
1940s press flash photography (hard on-camera flash, black falloff behind the subject), 1940s film noir
lighting (hard single sources, venetian blind shadows), Edward Hopper's night interiors.

## Palette

| Name | Hex | Use |
|---|---|---|
| Night | #0B1018 | Sky, deep shadow, UI panels |
| Fog teal | #2E4A55 | Fog colour, distant silhouettes, ambient fill, UI hover |
| Sodium amber | #FF9A3C | Street lamps, crane lights, bar windows, UI accent |
| Neon cyan | #3FE0FF | Two signs only: the bar's "THE LIGHTHOUSE" on the harbour map and the cinema marquee under the office window uptown; reflected in puddles and on the office ceiling |
| Rust brick | #6B3A2A | Warehouse, port office and bar building walls |
| Wet wood | #3A2E26 | Pier planks, pilings, crates, office floor |
| Steel | #4B5359 | Crane, ship hull, rails, bollards, camera body |
| Paper | #E8DCC2 | Cards, notebook, documents, UI text |
| Print white | #F2F0EA | Photo borders, flash core |
| Print grey | #8A8A86 | Mid-tone of the B/W prints |
| Cork | #B08A5B | Evidence board |
| Thread red | #D7192A | Red thread, LIE and WRONG stamps. Exclusive |
| Safelight red | #8E0F16 | Darkroom safelight and its spill only. Exclusive |

Value structure: 80% of every frame is below 15% luminance. Highlights come only from light sources and
their reflections. No pure black (floor #05080C) and no pure white outside light cores and the flash pop.

## Lighting and atmosphere

Exterior (night 02:00-06:00, then the sky lifts toward blue hour from 05:00):

- Key: sodium street lamps along the quay and pier (warm, 2200 K, point lights with IES profile, light
  functions off). One per 12-15 m, leaving dark gaps between pools of light; the player walks from pool to
  pool. The edge lamp of Pier 5 is dark (clue A09).
- Crane floodlights (two spots on the crane, 3500 K), fixed: the crane is parked since the police
  stopped the discharge at 01:10. One cone lights the No. 2 hatch, the other the customs berth and CL-2;
  the pier edge stays outside both (LEVEL.md section 5). The only moving light is the lighthouse beam.
- Lighthouse on the breakwater: rotating beam, visible volumetric shaft, period 12 s. Sweeps the sky and
  the water, never the pier floor (avoid flicker on gameplay space).
- The Lighthouse bar: amber windows plus the cyan neon sign; inside, warm practicals, a back-bar mirror
  and a lit jukebox.
- Port office: venetian blinds, one desk lamp per room, blind-stripe shadows on walls and on the detective.
- Gate booth: one bare bulb over the movements chalkboard (A20) and the visitors' book.
- SS Saint Agatha: deck work lights, oil lamp in the captain's cabin.
- Night 1 only: the coroner's van by the gate with its headlamps on the rope line; it leaves at 06:00.
- Sky: overcast, no moon, faint city glow on the horizon in fog teal.
- Fog: Exponential Height Fog with volumetric fog on; light sources show halos in rain. Density higher over
  the water, lower on the quay.
- Rain: Niagara GPU rain (camera-attached volume), splash particles on surfaces, ripple decals in puddles,
  drips from roof edges, streaks on windows (material).
- Rendering (NvRTX 5.8): Lumen GI and reflections with hardware ray tracing, RTXDI (Sampled Lighting
  Direct) for the many shadowed practical lights with DLSS Ray Reconstruction as its denoiser, Nanite on all
  static geometry, DLSS 4.5 (Super Resolution + Frame Generation where available), Reflex. Give every
  practical a real source radius: RTXDI turns it into a soft area shadow, which is the look we sell. Scalability presets must still give a readable picture with ray tracing off.

Camera and flashbulb (the signature moment):

- Aim view: the detective raises the press camera; the screen narrows to the ground-glass frame (thin
  white frame lines, slight barrel vignette, no HUD other than the frame counter).
- Flash pop: one shadow-casting spot light at the camera, 5600 K, very high intensity for 2 frames, then
  a 0.25 s exponential decay. Short attenuation radius (8-10 m) so the background stays black: the classic
  press-flash look, subject blown bright, world behind it gone.
- Rain droplets inside the flash cone are lit mid-air as white streak-free dots for the pop frames (the
  rain material reads the flash intensity from `MPC_World.Flash`).
- Afterimage: a 0.4 s faded negative of the frame on screen (inverted luminance, low opacity), then
  the view returns. Wet surfaces keep the specular glint of the flash for the pop frames only.
- Ray-traced reflections must catch the flash in puddles and windows: it is the NVIDIA shot.

The agency office (uptown, the hub of every case, its own map `L_Office`):

- Three desks (only the detective's is lit), filing cabinets, coat stand, the corkboard wall, the phone on
  the detective's desk, a door to the darkroom.
- One tall window: rain streaks, the cyan cinema marquee in the street below pulses faintly on the
  ceiling and the board. Desk lamp in sodium amber. The two empty desks sit in the dark.
- A door to the street stair. The harbour is a drive away: the car shot (fade, wipers, the harbour lights
  coming up through the windscreen) carries the player between the two maps.

Darkroom (dawn, end of each night):

- Lit only by the safelight (Safelight red, soft, one source) plus the white enlarger beam when the
  player enlarges a print.
- Three developing trays, the enlarger on the bench, prints hanging on a drying line with clothes pegs.
- The developing image fades up in the tray (material parameter 0-1 over 3 s).
- Loupe view: full-screen enlarged print, a round magnified lens following the cursor with a slight
  edge distortion and chromatic fringe inside the lens only; grain grows with magnification. Authored
  hotspots resolve to a sharper close-up texture when the lens rests on them.

Camera (gameplay): third-person, 35-40 mm equivalent, slight handheld sway, depth of field only in the
aim view, inspect mode and the board. Film grain light, vignette light, chromatic aberration off (ugly on
DLSS edges). Bloom moderate, lens flare off except the lighthouse beam and the flash pop.

## Photographs

- Every photo is captured in engine at the moment of the shot (scene capture of the actual frame, B/W
  grade with a slight warm paper tone), so the print always matches what the player framed.
- Print look: strong grain, crushed blacks, white border 6%, slight vignette, small date-time stamp in
  the border ("12 NOV 49 03:20"). Flash falloff baked into the print: bright subject, black surround.
- Cards on the board are these prints pinned with a typed label strip under them.
- Detail cards (from the loupe) are crops of the print with a hand-drawn grease-pencil circle in Paper.

## Materials and shapes

- One master material `M_Wet` for all opaque world surfaces: base colour, ORM, normal, plus a puddle mask
  (vertex colour or world-aligned noise) and global wetness from MPC `MPC_World` (Wetness 0-1, RainOn
  0/1, Flash 0-1). Wet surfaces: darken base colour 30-40%, roughness toward 0.1-0.2, puddles flat
  normal with rain ripple normal (flipbook).
- Materials are physically plausible, 2K textures by default, 4K only for the pier planks and the hero
  ship hull. Tileables and trim sheets over unique textures (packaged game must stay under 1 GB).
- Shapes: heavy, utilitarian 1940s industry. Riveted steel, creosoted timber, brick, wet canvas, rope.
  Rounded 1940s cars and lamp heads, art-deco signage on the bar only.
- Wear: rust streaks under every metal fixing, algae line on pilings at the waterline, wet stains under
  windows.
- The press camera: black leatherette and worn chrome, folding bed, bellows, a round flash reflector on a
  side gun. Generic design, no brand or model name.
- Evidence board: real cork, black-and-white prints with typed label strips, brass pins. The same board
  hangs on the office wall and opens full-screen from anywhere.

## UI theme

Diegetic where cheap: the evidence board is a physical cork board filling the screen; cards are prints
and paper. Menus and HUD are minimal typewritten text on dark panels.

Fonts (all licences allow embedding in a published game; add rows to `docs/CREDITS.md`):

- Body and buttons: Courier Prime (SIL OFL 1.1), Regular and Bold.
- Card text, typed forms and reports: Special Elite (Apache 2.0).
- Game title and case titles only: Playfair Display Black Italic (SIL OFL 1.1).

`Config/DefaultGame.ini` (colours are linear, converted from the palette above):

```ini
[/Script/JamUI.JamUISettings]
Backdrop=(R=0.0,G=0.0,B=0.0,A=0.65)
Panel=(R=0.0033,G=0.0052,B=0.0091,A=0.94)
Accent=(R=1.0,G=0.323,B=0.045,A=1.0)
ButtonNormal=(R=0.008,G=0.013,B=0.021,A=1.0)
ButtonHovered=(R=0.027,G=0.069,B=0.091,A=1.0)
ButtonPressed=(R=0.002,G=0.003,B=0.005,A=1.0)
Text=(R=0.807,G=0.716,B=0.540,A=1.0)
TextDim=(R=0.262,G=0.235,B=0.181,A=1.0)
Danger=(R=0.680,G=0.010,B=0.023,A=1.0)
FontFamily=/Game/UI/Fonts/F_CourierPrime.F_CourierPrime
TitleSize=44
HeadingSize=28
BodySize=20
CaptionSize=14
ButtonSize=22
TitleTypeface=Bold
BodyTypeface=Regular
CornerRadius=2
Padding=24
Spacing=12
PanelWidth=560
ButtonHeight=52
```

Square-ish corners (radius 2): paper and steel, not app UI. Danger buttons (Close the case, send the
report) use the thread red. HUD: top-left the clock ("NIGHT 1 - 03:20") in Special Elite on no panel;
under it the frame counter as a film-holder tally ("PLATES 9/12"); bottom-centre one interaction prompt
("E  Inspect", "RMB  Raise camera"); nothing else on screen in the world. Time cost appears as a small
"-10 min" that drifts up and fades next to the clock; the last plate makes the counter blink once.

Board specifics: cards 1.4:1 portrait with a coloured top strip by kind (Anchor print: steel grey #4B5359,
Detail crop: print white with the grease-pencil circle, Statement: sodium amber, Person: paper with photo).
The report sheet in the centre holds WHO / WITH WHAT / WHY. Black thread is waxed cord with slight sag
(spline), red thread is taut. LIE and WRONG stamps are rubber-stamp textures rotated -8 degrees. A broken
theory: the black thread snaps and the card drops back into the tray.

## Audio mood

- Music: slow noir jazz, 60-70 BPM: muted trumpet, upright bass, brushed drums, piano. Three cues:
  investigation loop, darkroom/dawn variant of the same loop (bass and solo trumpet only, low-pass, long
  reverb, under the dawn voice-over), and the office/phone cue. Board screen: the investigation loop
  ducked -8 dB with a filtered radio feel. Ending stingers: E1 "Whole truth" bitter resolution, E2 "Paid
  in full" warm but hollow, E3 "He walks" unresolved, E4 "The wrong man" dissonant.
- Ambience (exterior): constant rain layers (roof, water, canvas, puddles by proximity), foghorn every
  90-120 s, distant ship bells, creaking pilings and ropes, crane cable creak in the wind (the crane is parked), muffled
  jukebox from the bar that gets clear inside and dull from the yard.
- Ambience (office): rain on the window, neon buzz, a tram far off in the street below, wall clock.
- The drive: engine, wipers, tyres on wet setts; under 5 s per leg.
- Ambience (darkroom): slow drip, tray slosh when a print is rocked, safelight hum, the clothes-peg
  click of a print hung to dry.
- Camera: flashbulb pop (sharp, slightly glassy), shutter, film-holder slide and advance click, a dull
  click on an empty camera. The rain does not stop; the pop sits on top of it.
- Interaction: typewriter key for card creation, pin push, thread pull (cord creak), rubber stamp thud
  for LIE and WRONG, thread snap for a broken theory, clock tick for each time cost, phone ring (office).
- Voice: English, short lines only (see `docs/design/CASE01.md` for tone). Detective (male): low, dry,
  tired; he also narrates (noir voice-over at arrival, on key stamps and at each dawn at the board),
  close-miked, slightly warmer than his in-world lines. Subtitles always on.
- Mix: dialogue and narration first, then interaction cues, then music, then ambience. Rain never masks a
  voice line (sidechain duck 4 dB).
