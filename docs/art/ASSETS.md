# Asset list: Case 1, Pier 5

Every external or generated asset gets a row in `docs/CREDITS.md` (source, author or tool, licence, date)
before it is committed. Generated binary assets are imported by scripts in `Content/Python/`.
Priority: **M** must (the slice and the case cannot work without it), **S** should, **C** could.
Phase is when it must be in the build (see `docs/PLAN.md`): 1 Slice, 2 Breadth, 3 Juice.

## 0. Sources and rules

| Source | Use for | Notes |
|---|---|---|
| Fab (marketplace) | Architecture kits: harbour, warehouse, brick buildings, 1940s props | Only listings whose licence covers a published game. UE5 Lumen-ready kits only, no baked-lighting kits. "Allows usage with AI: No" (NoAI) does not block using the pack in this game; it only forbids feeding the content to generative AI (training, datasets, inputs): never use a NoAI asset or its renders as an input to Tripo, Meshy, Hyper3D, PBRgen or fal |
| Quixel Megascans (via Fab) | Ground, timber, rope, rock, debris scans | Check the licence of each pack at download time |
| Tripo, Meshy, Hyper3D (Rodin) | Hero props and clue props from text or a reference image | Retopologise or decimate, keep under 50k tris before Nanite; bake to 2K; sponsor credits cover use in the jam game |
| PBRgen | Tileable materials (planks, steel, brick, canvas, tar) | 2K, ORM packed |
| fal | Images (posters, signs, labels, card art, sky plate), music, SFX, TTS voice | Sponsor credits cover use in the jam game; record the model name per output for `docs/CREDITS.md` |
| Reallusion (Character Creator, Headshot, ActorCore) | All characters and some animations | Sponsor credits cover use in the jam game |
| Rokoko (Vision / Studio) | Custom animations from video | Retarget to the UE5 mannequin skeleton, then to the CC skeleton |
| Epic: Game Animation Sample, UE5 mannequin | Third-person locomotion (Motion Matching) | Epic content, free for UE projects |
| Diversion | Version control for binary assets, if used | - |
| SpaceXSI | Grok and Cursor credits (code and text help), not an asset source | - |

Budgets (packaged zip under 1 GB): textures 2K default (4K only where listed), shared trim sheets, no
unique 4K per prop. Static meshes Nanite-enabled; foliage none. Characters 2K textures, one LOD chain.
Audio: OGG/Vorbis at quality 0.6, music streamed.

Naming: `SM_` static mesh, `SK_` skeletal mesh, `M_`/`MI_` materials, `T_` textures, `NS_` Niagara,
`A_`/`AM_` animation, `S_` sound wave, `MS_` MetaSound. Folders: `/Game/Harbour/` (world),
`/Game/Characters/<Name>/`, `/Game/Case01/` (clue props, data), `/Game/UI/`, `/Game/Audio/`.

## 1. Location map and areas

Layout, metrics and night states: `docs/design/LEVEL.md` (sections 3 and 8). Playable area about
100 x 120 m; enterable interiors (5): agency office + darkroom, bar, customs office, captain's cabin, crane cab.
The catwalk under the pier is an exterior. The gate booth and the harbour master's office are seen through
a window only.

## 2. Environment meshes

### 2.1 Pier 5 and water

| Id | Asset | Source | Pri | Ph |
|---|---|---|---|---|
| E01 | Pier deck modules (plank sections 4 m, end section, damaged section) | Fab harbour kit or modelled + PBRgen planks (4K) | M | 1 |
| E02 | Pilings, cross braces, algae line (instanced) | Fab kit / Megascans | M | 1 |
| E03 | Pier edge with bollards (3 variants) and chain | Tripo/Meshy | M | 1 |
| E04 | Lamp post, 1940s sodium head, with a separate unscrewed bulb (clue A09) and a fresh gouge at 2.1 m (clue A24, decal + small mesh splinters) | Tripo/Meshy + decal | M | 1 |
| E05 | Lower landing: ladder, floating stage, mooring rings | Fab kit / modelled | M | 1 |
| E05b | Catwalk under the west overhang: plank walkway on pile caps (28 m, 1 m wide, 1.7 m headroom), "HARBOUR WORKS - NO ADMITTANCE" board at the berth end | Modelled from E01/E02 parts | M | 1 |
| E06 | Fish crates, nets, buckets, coiled rope, tarps (6-8 props); the crates carry the bale hook (C05) | Megascans / Tripo | M | 1 |
| E07 | Water: UE Water plugin ocean body or custom plane with wave normals, rain ripples, foam at pilings | Engine + material | M | 1 |
| E08 | Breakwater with lighthouse tower (silhouette, far) | Modelled low detail | S | 2 |
| E09 | Tarp over the body (cloth-like static mesh), police rope on posts | Tripo | M | 1 |

### 2.2 Crane

| Id | Asset | Source | Pri | Ph |
|---|---|---|---|---|
| E10 | Portal crane on rails (riveted steel, rotating top, jib, hook, cable) | Fab or Hyper3D, split into parts for animation | M | 1 |
| E11 | Crane cab interior (seat, levers, log clipboard A10) | Modelled + Tripo props | S | 2 |
| E12 | Cargo net with crates on the hook (slow lift loop) | Tripo | S | 2 |
| E13 | Crane access ladder and platform | Fab kit | M | 1 |

### 2.3 SS Saint Agatha

| Id | Asset | Source | Pri | Ph |
|---|---|---|---|---|
| E20 | Hull side (east of the pier), 70 m, only the visible side detailed; name painted on the bow | Fab cargo ship or Hyper3D blockout + trim sheet | M | 1 |
| E21 | Gangway with rope rails; watch stand at the head (lectern, tin hood, bulb) holding the gangway watch book (C10) | Tripo | M | 1 |
| E22 | Deck section: hatches, winches, ventilators, work lights, tally clipboard by the hatch (A21) | Fab kit / Megascans | M | 2 |
| E24 | Captain's cabin (deck level, door with the E53 brass plate): bunk, desk, oil lamp, contraband crates (A15), manifest copy (A16), clearance certificate (A22) | Fab interior kit + Tripo | M | 2 |

### 2.4 Quay road, customs berth, gate booth

| Id | Asset | Source | Pri | Ph |
|---|---|---|---|---|
| E30 | Cobblestone quay with embedded rails (PBRgen tileable + puddle mask) | PBRgen / Megascans | M | 1 |
| E31 | Customs berth: floating pontoon on the west side of Pier 5 under the crane swing, ramp to the pier root, CL-2 mooring post with stencil "CL-2" | Modelled + decal | M | 1 |
| E32 | Customs launch CL-2 (1940s motor launch, 9 m, cabin, boathook rack empty) | Hyper3D or Tripo, cleaned | M | 1 |
| E33 | Gate booth (small wooden kiosk, not enterable: open hatch and north/west windows, desk, bare bulb, visitors' book A11, harbour movements chalkboard A20 on the wall) | Fab kit / Tripo | M | 2 |
| E34 | 1940s cars: the detective's black sedan at the Lighthouse curb (also the drive shot: windscreen, wipers, dashboard), a parked car by the warehouse, police car by the gate | Tripo/Meshy | S | 2 |
| E35 | Fences, barriers, oil drums (rust brown, never red), pallets | Megascans / Tripo | S | 2 |
| E36 | Telephone pole and sagging wires across the road | Fab / modelled | C | 3 |
| E37 | Coroner's van (1940s panel van, dark grey, "COUNTY CORONER" in off-white), by the gate on night 1 only, drives off at 06:00 | Tripo/Meshy | M | 2 |

### 2.5 Buildings

| Id | Asset | Source | Pri | Ph |
|---|---|---|---|---|
| E40 | Warehouse 5 exterior (brick, big sliding door with "5", loading dock) | Fab modular brick kit | M | 1 |
| E41 | Port office exterior (two storeys, brick, lit windows with blinds, sign "HARBOUR MASTER / CUSTOMS") | Fab modular kit | M | 1 |
| E42 | Customs office interior: two desks, filing cabinets, lockers (Varga's opens), duty board (A18), draft report (A19), venetian blinds | Fab office kit + Tripo | M | 2 |
| E43 | Harbour master's office: window dressing only (seen from the street) | Fab | S | 2 |
| E44 | The Lighthouse bar exterior (corner building, cyan neon sign, amber windows, lit upstairs window of Nora's rooms; a walled yard on the east side under one bulb, its gate onto the street, LEVEL 3) | Fab kit + custom sign mesh | M | 1 |
| E45 | Bar interior: counter, back-bar bottles, stools, 4 tables, jukebox, cash register (A13), coat rack, back door to the yard; the back-bar mirror is a full-height pier glass at the register end, between the gin and the rye (SH_Mirror: RT reflection, the first-person body kept visible in reflections, LEVEL 6) | Fab bar kit + Tripo (jukebox, register) | M | 2 |
| E46 | City skyline backdrop in fog (cards or low-poly blocks with lit windows) | Modelled + fal window texture | S | 2 |
| E47 | Agency office (uptown, `L_Office`, shared by every case): three desks (the detective's lit by a green-shade lamp, its drawers open on inspect), filing cabinets, coat stand, corkboard wall (the board), desk phone, tall window over an uptown street (a cinema marquee glows from below), door to the darkroom, a door to the landing and the street stair; a drying line with pegs and hanging prints across the room in front of the darkroom door (outside the red light) | Fab office kit + Tripo | M | 1 |
| E48 | Darkroom: bench, enlarger, three developing trays, safelight lamp, sink | Fab kit + Tripo | M | 2 |

### 2.6 Level dressing, lures and night layers (LEVEL.md sections 8, 15, 17)

| Id | Asset | Source | Pri | Ph |
|---|---|---|---|---|
| E50 | Hurricane lantern on the rope post (night 1), police rope and posts | Tripo | M | 1 |
| E51 | White lifebuoy on the rail at the ladder top (lure to the lower landing) | Tripo | M | 1 |
| E52 | Chalk outline decal, half washed (nights 2-3); tyre-track puddle decals at the gate | Decal (fal texture) | S | 2 |
| E53 | Brass "MASTER" door plate, padlock and name card "L. VARGA" for the locker row; padlocked harbour-works wicket in the dock fence behind Warehouse 5 (LEVEL 3) | Tripo + decal | M | 2 |
| E54 | Police seal on No. 2 hatch, police car at the east end of the street, police bicycle | Tripo/Meshy | S | 2 |
| E55 | Flavour props: Morrow's umbrella, rent reminder, framed press print, covered typewriter, thermos, crew laundry, Morrow's coat, ringed calendar, cigarette butts | Tripo / Megascans | S | 2 |
| E56 | Varga's lantern (pontoon; on the landing on night 2) and a pike pole (night 2 tableau); empty boathook rack on CL-2 (part of E32, readable in inspect) | Tripo | M | 2 |
| E57 | No-rain volumes under awnings (crane, warehouse dock, booth eave, deckhouse, watch-stand hood, the catwalk) | Engine | M | 1 |
| E58 | Tug MARGARET (1940s harbour tug, smoke, steam whistle), alongside the Agatha on night 3 only | Hyper3D / Tripo | S | 2 |
| E59 | Agatha departure dressing: funnel smoke (Niagara), singled-up mooring lines (night 2-3) | Niagara + mesh variant | S | 2 |
| E60 | Agency window gold-leaf lettering (decal, mirrored inside) + light function for its floor shadow: "HARROWMARK / PRIVATE INQUIRIES" | Decal | S | 2 |
| E61 | Drift under the pier: foam lines and floating debris that run from the landing under the deck and collect at the boathook pilings | Niagara / water material | S | 2 |
| E62 | Movements chalkboard variants for nights 2-3 ("SAT 12 NOV", "SUN 13 NOV", "CL-2 HELD - POLICE") | render_docs.py texture | M | 2 |

## 3. Clue props (Case 1)

Each clue is a hero prop: 2K textures, close-up quality through the aim view. Its board card is the
player's own photograph, captured in engine at the shot.

| Id | Clue | Asset | Source | Pri | Ph |
|---|---|---|---|---|---|
| C01 | A02 | Wristwatch, stopped hands (hands as separate meshes set to 23:41), water inside | Tripo + manual | M | 1 |
| C02 | A03-A05 | Notebook, open, pages with handwriting (script-rendered), smudged signature | Tripo + generated pages | M | 1 |
| C03 | A06 | Boathook, ash pole 2.4 m with brass hook, tar on the grip over a stencil "C.L. 2" (mask `T_C03_HookStencil_M`; the stencil reads only in the D04 close-up) | Tripo + script mask | M | 1 |
| C04 | A07 | Broken whisky bottle with Lighthouse label | Tripo + fal label | M | 1 |
| C05 | A08 | Docker's bale hook, short T-handle, wet | Tripo | M | 1 |
| C06 | A09 | Loose bulb in the lamp head (part of E04) | E04 | M | 1 |
| C07 | A10 | Crane log clipboard | Tripo + generated page | M | 2 |
| C08 | A11 | Gate visitors' book | Tripo + generated page | M | 2 |
| C09 | A13 | Cash register with paper tape | Tripo | M | 2 |
| C10 | A14 | Gangway watch book on the watch stand (page `T_C10_GangwayBook_D`) | Tripo + generated page | M | 2 |
| C11 | A15 | Opened crate stencilled "MACHINE PARTS" with cigarette cartons and bottles | Tripo + fal stencil | M | 2 |
| C12 | A16 | Manifest copy with stamp and signature | Generated page on paper mesh | M | 2 |
| C13 | A17 | Plain envelope with $500 in fifties, "J.K." on the flap (Varga's locker) | Tripo | M | 2 |
| C14 | A18 | Duty roster board with chalk names | Tripo + generated texture | S | 2 |
| C15 | A19 | Typed draft report | Generated page on paper mesh | S | 2 |
| C16 | A01 | Wound on the back of the head (decal on Morrow's body mesh) | Decal | M | 1 |
| C17 | A20 | Harbour movements chalkboard at the gate booth (chalk text per CASE01; wiped and rewritten after night 1) | Generated texture on board mesh | M | 2 |
| C18 | A21 | Agatha tally sheet on a clipboard, tallyman P. Doyle: hoists 23:00-00:30, crane S.H. on every line | Tripo + generated page | M | 2 |
| C19 | A22 | Clearance certificate: harbour master's stamp, countersigned by Kordes; "for customs, L. Varga" | Generated page on paper mesh | M | 2 |
| C20 | A12 | Credit slate on the back bar: "No credit." and the old "E.M. $40" (`T_C23_BarSlate_D`) | Generated texture on slate mesh | M | 2 |
| C21 | A24 | Lamp-post gouge (part of E04) | E04 | M | 1 |
| C22 | A25 | Plain brown envelope for the night-3 hand-off (Kordes's hand to Varga's) | Tripo | S | 2 |
| C23 | A26 | Customs store book open on the front counter (page `T_C24_StoreBook_D`: the boathook line at 16:30, the clearance stamp out 23:28 and back 23:52) | Tripo + generated page | M | 2 |
| C24 | A28 | Camera tripod, 1940s: three ash legs with brass feet; one leg sawn short, the cut pale, its foot gone, so it stands a little crooked | Tripo + manual cut | M | 2 |
| C25 | A29 | The Meridian print on the drying line: B/W, a man in a customs greatcoat at the door of room 9, a woman's gloved hand on his sleeve | Scene capture of CH02 at a door flat with a brass "9", graded with UI12 | M | 2 |
| C26 | A30 | Hotel Meridian envelope, $300 in fifties, "J.K." on the flap, dated Thursday (in the detective's desk drawer) | Tripo + fal letterhead | M | 2 |
| C27 | A31 | Frame 1 print: almost black; rain in a flash, the pier rail. Authored, not captured (it was shot before the game) | Scene capture at the edge, crushed and graded with UI12 | M | 2 |

Detail close-ups for the darkroom loupe: authored B/W textures, grainy, matching the print framing, swapped
in under the lens when it finds the hotspot. CD06 sits on a print the player frames himself; the
hotspot is the reflected cuffs and right hand, and the close-up is the same at any framing that passes
the sharp check.

| Id | Detail | Close-up | Source | Pri | Ph |
|---|---|---|---|---|---|
| CD01 | D01 | Wound enlarged: ash splinters in the hair; a long wooden shaft, swung hard | Graded render of C16 + paint-over | M | 2 |
| CD02 | D02 | Crane log margin, pencil: "slewed short 23:00-23:50 - CL-2 under the swing." | Script-rendered on the C07 page | M | 2 |
| CD03 | D03 | Notebook smudge enlarged: "Cleared by L. Varga" | Script-rendered on the C02 page | M | 2 |
| CD04 | D04 | Boathook grip enlarged: the stencil "C.L. 2" under the tar (`T_D04_HookStencil_D`) | Script-rendered | M | 2 |
| CD05 | D05 | Frame 1 enlarged: the dark end lamp of Pier 5; below it a man's hand thrown up against the flash, a customs button on the cuff | Scene capture of CH02's arm + paint-over | M | 2 |
| CD06 | D06 | Mirror print enlarged: a dry salt line on the trouser cuffs, shin-high; the right glove torn across the palm, a fresh splinter cut under it | Scene capture of CH01's variant + paint-over | M | 2 |

Generated pages (notebook, logs, watch book, manifest, report, chalkboards, slate, tally sheet, certificate, detail close-ups)
are rendered from text by a script so the text matches `docs/design/CASE01.md` exactly; fal is used only
for paper texture and stains.

Camera props:

| Id | Asset | Source | Pri | Ph |
|---|---|---|---|---|
| CA01 | Press camera, 1940s 4x5 style (folding bed, bellows, leatherette, chrome), side flash gun with round reflector, film holders; generic, no brand or logo | Tripo/Meshy, cleaned | M | 1 |
| CA02 | Spent flashbulb (ejected after each shot, small physics prop, despawns) | Tripo | C | 3 |

## 4. Characters

All characters from Reallusion Character Creator (realistic base), exported to UE with the CC skeleton,
retargeted animation from the UE mannequin. 1949 clothing: wool coats, fedoras, flat caps, oilskins,
uniforms. No red clothing on anyone.

| Id | Character | Look | Variants | Pri | Ph |
|---|---|---|---|---|---|
| CH01 | Detective (player, male, name TBD) | Late 30s, 185 cm, dark trench coat, fedora, scarf, dark brown leather gloves, press camera on a strap, flashlight. From the first frame (painted into the base colour by `Tools/art/ch01_clues.py`, never mentioned, D06): a dry salt tide line on both trouser legs shin-high with salt bloom below and on the shoes; the right glove torn across the palm, the cut showing, a pale ash splinter in the tear; pale ash sawdust on the right coat cuff and right knee (A28). Model: Meshy 7.1 Ultra + Tripo 78-joint rig, `art_src/Generated/Characters/CH01/CH01_Detective_D06.glb` | Wet shader | M | 1 |
| CH02 | Elias Morrow (victim) | 40s, customs uniform (navy wool, cap), wedding ring | Body only (lying face down, tarp pulled back); gone with the van after night 1 | M | 1 |
| CH03 | Leo Varga | 40s, tall, customs uniform (same outfit as Morrow, different face, moustache), rain-wet | Reuses CH02 clothing | M | 2 |
| CH04 | Nora Vale | Late 30s, 160 cm, dark green dress, cardigan, apron, hair up | - | M | 2 |
| CH05 | Sam Holt | 50s, 190 cm, heavy, oilskin jacket, flat cap, gloves | - | M | 2 |
| CH06 | Jan Kordes (client) | 50s, peacoat, captain's cap, beard | - | M | 2 |
| CH07 | Sgt. Brannock | 50s, police raincoat and helmet-style cap, cape | At the rope by the body (night 1), gate booth (nights 2-3), customs office by the open locker once S08 is stamped LIE. Lines include S01 and S15 "Then it's murder. The van takes him at six." | M | 1 |
| CH08 | Extras: 2 dockers, 1 sailor, 2 bar patrons, 2 coroner's men (night 1) | CC morphs and clothing swaps from CH05/CH06 | - | C | 3 |

Faces: Headshot from generated portraits (fal) to keep faces original; no real people. Portraits: `Tools/art/gen_portraits.py` (picks: CH02_2, CH03_2, CH04_2, CH05_1, CH06_1, CH07_2). CC build: `Tools/reallusion/` (bridge loaded in CC via Script > Load Python, jobs queued by `queue_job.py`: Headshot Pro, save .ccAvatar, Unreal FBX + json for Auto Setup).

## 5. Animations

| Id | Animation | Source | Used by | Pri | Ph |
|---|---|---|---|---|---|
| AN01 | Locomotion: idle, walk, jog, turns, starts and stops | Game Animation Sample (Motion Matching) | Detective | M | 1 |
| AN02 | Flashlight hold (upper-body layer) | Rokoko Vision | Detective | S | 2 |
| AN03 | Crouch inspect (enter, loop, exit), stand inspect (reach) | Rokoko / ActorCore | Detective | M | 1 |
| AN04 | Talk idles: arms crossed, leaning on rail, smoking, wiping glass (Nora), hands in pockets | ActorCore / Rokoko | NPCs | M | 2 |
| AN05 | Talk gestures (3-4 additive) | ActorCore | NPCs | S | 2 |
| AN06 | Camera (upper-body layer): raise to eye, aim idle, shoot (flash kick), flashbulb swap, film-holder change, lower | Rokoko Vision | Detective | M | 1 |
| AN07 | Phone: pick up, listen, hang up (office opening) | ActorCore / Rokoko | Detective | S | 2 |
| AN08 | Body lying face down (pose) | Pose | Morrow | M | 1 |
| AN09 | Reaction: "I want a lawyer" (step back, arms crossed), arrest (hands behind back), holding out both wrists (E1, E2) | Rokoko | Varga, Kordes, Detective | S | 3 |
| AN10 | Darkroom hands: rock tray, lift print, peg it on the line (first-person hands or close camera) | Rokoko | Detective | C | 3 |
| AN11 | Lip-sync for voiced lines | Reallusion AccuLips or NVIDIA Audio2Face, whichever runs on 5.8; fallback: jaw-only from audio amplitude | All speakers | C | 3 |
| AN12 | Tableau poses: Varga crouched on the stage probing with the pike pole (loop, lantern at his feet); Kordes and Varga at the gangway foot, envelope passed and pocketed (loop with a 20 s cycle) | Rokoko Vision | Varga, Kordes | M | 2 |

## 6. VFX (Niagara and materials)

| Id | Effect | Pri | Ph |
|---|---|---|---|
| FX01 | Rain (camera volume); drops inside the flash cone lit by `MPC_World.Flash` | M | 1 |
| FX02 | Splashes and ripples on surfaces and water | M | 1 |
| FX03 | Roof drips and gutter streams | S | 2 |
| FX04 | Lighthouse beam (volumetric cone mesh with material, rotating) | S | 2 |
| FX05 | Steam from vents, cigarette smoke (Holt), breath vapour | C | 3 |
| FX06 | Flashbulb pop: spot light pulse (5600 K, 2 frames full, 0.25 s decay) + `MPC_World.Flash` curve + Niagara spark and smoke puff at the reflector | M | 1 |
| FX07 | Flash afterimage: post-process negative of the frame, 0.4 s fade | M | 1 |
| FX08 | Aim view: ground-glass frame overlay, vignette, slight barrel (post-process material) | M | 1 |
| FX09 | Print developing in the tray (material fade-up over 3 s with liquid ripple) | S | 2 |
| FX10 | Safelight haze in the darkroom (light volume) and enlarger beam | S | 2 |
| FX11 | Loupe lens (UI material: magnify, edge distortion, grain scales with zoom) | M | 2 |
| FX12 | Thread snap on a broken theory (UI) and stamp dust on LIE/WRONG | S | 2 |
| FX13 | Dawn sky transition at the end of night 3 (sky plate and fog colour blend) | C | 3 |

## 7. UI assets

| Id | Asset | Source | Pri | Ph |
|---|---|---|---|---|
| UI01 | Cork board texture (2K tileable) and wooden frame | PBRgen / Megascans | M | 1 |
| UI02 | Card paper texture with stains, 4 kind strips (Anchor print, Detail crop, Statement, Person) | fal + script | M | 1 |
| UI03 | Brass pin, thread textures (black cord, red cord) | fal / script | M | 1 |
| UI04 | LIE and WRONG rubber stamps, "WORD AGAINST WORD" and "NO CONTRADICTION" stamps | Script-rendered with Special Elite | M | 1 |
| UI05 | Person photos (black-and-white, captured in engine from each character) | Scene capture | M | 2 |
| UI06 | Report sheet with WHO / WITH WHAT / WHY slots | Script + fal paper | M | 1 |
| UI07 | Fonts: Courier Prime, Special Elite, Playfair Display | Google Fonts (OFL / Apache) | M | 1 |
| UI08 | Title screen: rain on the pier, title in Playfair Display | In-engine | S | 3 |
| UI09 | Case card with rating stamp (A-D) | Script | S | 3 |
| UI10 | Posters and signage in the world (shipping line posters, "NO SMOKING", tide table) | fal | C | 3 |
| UI11 | Film counter "PLATES n/12" under the clock; last-plate blink. Night 1 opens at 11/12 (frame 1) | Script + Special Elite | M | 1 |
| UI12 | Print template: white border, grain overlay, date-time stamp, B/W grade (applied to every captured photo) | Material + script | M | 1 |
| UI13 | Darkroom widget: contact strip of the night's prints, enlarge view, loupe cursor, hotspot pulse | UMG + FX11 | M | 2 |
| UI14 | Detail crop card with a grease-pencil circle | Script | M | 2 |
| UI15 | Reconstruction: the player's prints in night order with an empty "NOT ON FILM" frame for each missing beat, ending on the mirror (the player's print or the live view at the register) (CASE01 8) | UMG + UI12 | M | 3 |

## 8. Audio

| Id | Asset | Source | Pri | Ph |
|---|---|---|---|---|
| AU01 | Music: investigation loop (2-3 min, seamless) | fal music model | M | 2 |
| AU02 | Music: darkroom/dawn variant (bass and solo trumpet, low-pass, long reverb) | Processed from AU01 | S | 2 |
| AU03 | Music: office/phone cue, 5 ending stingers (E1-E5), a reconstruction bed | fal | S | 3 |
| AU04 | Rain beds: roof, water, canvas, street; wind | fal / CC0 libraries | M | 1 |
| AU05 | Harbour: foghorn, ship bell, creaking pilings, rope, gulls (sparse), crane cable creak, van engine idle, pike pole knocking on wood, steam whistle | fal / CC0 | S | 2 |
| AU06 | Darkroom: drip, tray slosh, safelight hum, peg click; office: rain on the window, neon buzz, bar through the floor | fal / CC0 | M | 1 |
| AU07 | UI: typewriter, pin, cord pull, stamp thud, thread snap, clock tick, click, hover (`jf sfx click`, `jf sfx hover`) | fal / jf sfx | M | 1 |
| AU08 | Footsteps: wet wood, wet stone, metal, interior wood (4 sets x 6) | CC0 / fal | M | 1 |
| AU09 | Bar: jukebox record (period-style instrumental, original), glasses, murmur | fal | S | 2 |
| AU10 | Voice: all lines in `docs/design/CASE01.md` (6 speakers: detective, Brannock, Nora, Holt, Kordes, Varga), including Kordes's opening phone call (phone-filtered), Brannock's S15, and the detective's narration (arrival, film rule, stamps, wasted red, last plate, darkroom, dawn monologue, endings) | fal TTS, distinct voice per speaker | S | 3 |
| AU11 | Camera: flashbulb pop, shutter, film-holder slide and advance, empty-camera click | fal / CC0 | M | 1 |
| AU12 | Office phone ring (1940s bell) | fal / CC0 | M | 2 |

## 9. Production order

1. Phase 1 (slice): greybox the whole map with final dimensions first (detective, bar with its yard, the office as `L_Office`,
   pier, quay, crane, CL-2, gate booth, port office, Agatha, all as blocks), then E01-E07, E09, E30, E47,
   E05b, CH01, CH02, CH07, CA01, C01-C06, C16, C21, FX01-FX02, FX06-FX08, UI01-UI04, UI06-UI07, UI11-UI12, AU04,
   AU06-AU08, AU11, AN06. Goal: the tutorial on the pier (photograph the wound, red thread S01 vs A01,
   WRONG stamp) playable in the final look, with camera, flash and board.
2. Phase 2 (breadth): all remaining M assets: suspects, interiors, darkroom and loupe, all clue props and
   detail close-ups, coroner's van, music loop.
3. Phase 3 (juice): S and C assets in order of screen time: lighthouse beam, bar interior detail, voice,
   endings, dawn, title screen.

Screenshot and video shots to protect (Render Royalty): the flashbulb pop on the pier edge lighting the
rain mid-air and flaring in ray-traced puddles; the darkroom under the safelight with a print developing;
the loupe finding a detail; the board with red thread and a LIE stamp; the bar neon in a puddle and seen
through the office window with rain on the glass; the detective in the back-bar mirror, camera raised.
