# Game Design Document: HarrowMark

Detailed companions: `docs/design/CASE01.md` (the first case), `data/case01.json` (the case as data,
checked by `tools/case_check.py`), `docs/art/STYLE.md` (art and audio direction), `docs/art/ASSETS.md`
(meshes, characters, animation, VFX, audio and UI lists).

## 1. Pitch

A neo-noir detective game in a rain-soaked 1949 port city. A private detective works out of a
three-desk agency uptown and drives down to wherever the case is. His only real tool is a press camera with a flash and twelve
plates a night: anything he does not photograph never happened. He shoots the scene, talks to people,
and pins prints to a corkboard. A black thread ties a theory together (who, with what, why); a red
thread ties two things that cannot both be true. At dawn he develops the film, and a loupe finds what
the eye missed. Talk costs minutes; proof costs a plate. After three nights the report goes to the client,
who is lying too. So is the narrator, by omission: the man holding the camera killed the man in the water,
and the only way to put him on the board is to photograph his reflection.

- Genre: third-person investigation, deduction board, short narrative case.
- Session: one case over up to three nights: about 25 minutes for a sharp player, 45-70 minutes first
  time. Night 1 alone can close the case with an A.
- Platform: Windows, UE 5.8 (NvRTX branch), keyboard and mouse; gamepad is a Could.
- Language: English (player-facing text and voice).

## 2. USP

**The agency sells photographs, not opinions. Three nights, limited film, one report to the client.**

What makes it different from other deduction games:

1. **Evidence is a photograph the player chose to take.** Inspecting and reading are free; keeping
   anything costs a plate and ten minutes. Twelve plates a night, and the body, the notebook and the
   harbour board are gone by morning. The board holds only what the player decided was worth a frame.
2. **The darkroom is a second look.** At dawn the night's frames are developed and enlarged; a loupe
   over the print finds details the eye missed at the scene (a margin note, a smudged name, the angle of
   a blow). The same photo pays twice.
3. **The theory can be wrong in public.** A pinned theory makes claims; a single photo can snap it. The
   board is a contradiction engine, not a checklist: red threads stamp lies, mark honest mistakes, break
   theories, or cost an hour.
4. **The client is a suspect.** The man paying for the report lied to his own detective. The ending is a
   choice of what goes in the envelope, not just who did it.
5. **The camera points both ways.** None of the four suspects did it. The detective did, and his
   voice-over never lies, it only leaves things out. The proof is on his own film: a plate he fired before
   the game began, and his reflection in the bar mirror if the player thinks to shoot it. Most players
   hang an innocent; the ending shows every player the mirror.

## 3. MVP

The smallest build that proves the USP is one full case, playable start to finish, in greybox:

- Camera: raise, frame, shoot; subject validity with viewfinder brackets; film counter; contact print
  card on the board; flash light pulse.
- Board: cards, black threads to WHO / WITH WHAT / WHY, red threads with all four results, close the
  case with the attach choice.
- Darkroom: develop at the office mid-night (a drive there and back plus 30 min) or at dawn; prints on a line, a loupe over each
  print, six authored detail hotspots; night 1 starts with one plate already exposed.
- Clock: three nights, costs, expiring shots, two timed scenes (nights 2 and 3), forced report after night 3.
- Talk: topic list with subtitles and conditions.
- Dawn monologue as a subtitle line.
- Five endings as text screens with the rating card.
- Greybox of every area the case uses (section 10), all clue actors and characters placed.

Everything visual or audio beyond that is Should (section 12).

## 4. Pillars

1. **Film is the budget.** Every frame is a decision; time and plates are the only currencies.
2. **Photographs never lie.** Anchors and details are always true; words and theories may not be. Every
   contradiction the game accepts has a photograph on one side.
3. **Everyone is guilty of something.** Each suspect hides a real sin; none of them hides the murder;
   the client hides the biggest sin on the board, and the narrator hides the one off it. Exposing the
   wrong sins clears the board until only the man with the camera is left.
4. **A beautiful wet night.** Every scene is built for ray-traced reflections, volumetric light and rain,
   and every flash lights it up for a split second.

## 5. Judging map

| Criterion | What earns it |
|---|---|
| Ambition and innovation | Photography as the only way to keep evidence; darkroom enlargement as a second pass on the same photo; a theory board that can break itself; the client as a suspect |
| Theme | No theme for this jam (owner confirmed); the criterion is covered by the strength of the core idea |
| Fun | Aha moments: the loupe resolving a name, the LIE stamp, the snap of a broken theory, the envelope choice |
| Gameplay | Clear verbs (look, shoot, talk, pin, tie, enlarge, report), two currencies (time, film), five endings |
| Visuals (and the NVIDIA prize) | Night port in rain: Lumen hardware ray tracing, the flash lighting wet surfaces and rain in one frame, the red-lit darkroom; DLSS with frame generation; path-traced promo stills. Headline NvRTX feature (confirmed day 1, 2026-11-12): **RTXDI** (NvRTX Sampled Lighting Direct, `r.SampledLighting.Direct`) for every shadowed practical light, denoised by **DLSS Ray Reconstruction**. Measured on /Game/Dev/L_RTXDITest (36 shadowed point lights, 1280x720 DLAA, RTX 5070 Ti, fixed camera): RTXDI on 6.3 ms GPU, p95 8.0 ms, no hitches; off (ray-traced/virtual shadow maps) 5.9 ms GPU, p95 11.7 ms, 16 hitches over 50 ms; on gives soft area-light penumbrae, off gives hard shadows. RTX Mega Geometry and Audio2Face are not claimed (untested) |
| Level design and progression | One dense harbour map, no loading inside it, plus the agency office uptown (a small hub map shared by every case, reached by a drive): the bar, pier, the catwalk under it, port offices, the ship; gates opened by exposed lies; evidence that expires at dawn; two timed scenes on nights 2 and 3 |
| Audio | Noir jazz score with a darkroom variant, full voice-over with the detective's narration, flashbulb pop and rain |

## 6. Fiction

Harrowmark is a port city at the end of the 1940s. The agency is three desks, a corkboard, a phone and a
darkroom in a back room, rented above a shop on an uptown street. The colleagues are out on other jobs;
their desks are set dressing. Every case starts and ends at the office; the detective's car takes him to
the scene and back. Case 1 happens in the harbour, a drive away.

The detective narrates in voice-over: short, dry, ironic. He never explains the rules and never tells
the player what to do; a hint lives only in the subtext. In Case 1 he is the killer and knows it. Every
line he speaks is literally true; he leaves out the hour between 23:05 and 00:05 (CASE01 sections 1, 11).

## 7. Core loop

```
NIGHT (clock + 12 plates)                          DAWN (office, no clock)
  look, read, inspect      -> free                     develop the night's frames
  shoot a subject          -> anchor cards (1 plate,   loupe over prints -> detail cards
                              10 min)                  board: the detective's monologue
  talk to people           -> statement cards (15 min) -> next night
  drive harbour <-> office -> 10 min each way (develop, loupe and the office subjects are at the office)
  board (Tab, anywhere):
    red thread             -> LIE / WRONG stamp, theory breaks, or -60 min
    black thread           -> pin WHO / WITH WHAT / WHY
  report to the client at any time when the theory is complete -> ending
```

Minute to minute: walk a wet harbour area with the flashlight, read a watch book for free, decide it is
worth a plate, raise the camera, wait for the brackets to close, fire the flash, hear the plate slide.
Open the board, tie two prints that cannot both be true, watch the stamp come down.

## 8. Player verbs and controls

| Verb | Key (KB+M) | Notes |
|---|---|---|
| Move, look | WASD, mouse | Third-person over the shoulder; jog on Shift |
| Interact, inspect, talk | E | Inspect view: the object fills the screen, mouse rotates it, documents are readable. Free |
| Flashlight | F | Ray-traced shadows; makes small text readable |
| Raise camera | Hold RMB | First-person viewfinder; brackets close on a valid subject |
| Shoot | LMB while raised | Flash fires; one plate and 10 minutes; the print lands in the board tray |
| Board | Tab | Full-screen board, same board as the office wall; free (the clock moves only on actions, 9.5) |
| Pause menu | Esc | Settings (graphics, RT, DLSS, frame generation, audio, subtitles) |

## 9. Systems

### 9.1 Camera and film

- **Subjects.** Every photographable thing carries a `UPhotoSubjectComponent` with a shot id (CASE01
  section 4.1). A subject is valid when its bounds are inside the viewfinder frustum, its projected size
  is at least 15% of the frame height, at least 60% of its sample points are unoccluded (line traces), and
  it is within flash range (6 m) or lit by a scene light. The viewfinder brackets close around the best
  valid subject; the shutter fires only when one is valid, so a plate is never wasted by accident.
  Some things look like evidence and are not subjects (a bollard, a coil of rope): the brackets stay
  open, and the player learns that without spending a plate.
- **Sharp or grainy.** A subject filling at least 35% of the frame height, within flash range or lit, makes
  a sharp print. Between 15% and 35% the print is grainy: it gives its anchor cards but no darkroom
  detail, and that subject may be shot again. Getting close is the skill; sniping from 20 m is not.
  The brackets tell the two apart before the shot: white for sharp, grey for grainy. The forced wound
  shot is always sharp.
- **Several subjects.** One frame can hold more than one valid subject; it gives all their anchors. The
  brackets mark each one.
- **Film.** 12 plates a night, refilled at dawn. A counter sits in the viewfinder corner. The last plate
  plays a VO line. Night 1 starts at 11: the first holder in the box was fired before the game began
  (frame 1, CASE01 4.1). It cannot be shot; it develops with the others and gives its card and detail
  like any print. Shooting the same subject twice is refused ("Already on film") unless the first
  print was grainy.
- **Prints.** A shot drops a contact print into the board tray at once, carrying its anchor cards. The
  image is a `SceneCaptureComponent2D` frame taken at the moment of the shot into a render target and
  copied to a texture, graded as a black-and-white press print. Showing a print to a
  person in talk is free; a topic it opens costs 15 minutes like any other.
- **Expiry.** Some subjects are gone after night 1: the body and its notebook (coroner's van at 06:00)
  and the gate booth movements board (wiped at shift change). The tutorial forces the wound photo.
- **Timed scenes.** Two subjects exist on one night each, from night start until 01:30 on the clock:
  Varga fishing under the pier (night 2) and the hand-off at the gangway foot (night 3). Both are in
  place when the night starts (set during the dawn cut), since the clock moves only on actions, and clear
  off screen after 01:30; talking to Varga on the landing (S18) ends the fishing early. Both are framed
  by the arrival shot at night start and lured by sound (LEVEL section 15); neither is required.

### 9.2 Cards

Every piece of information is a card. Cards carry **assertions** that the contradiction engine reads.

| Kind | Source | Look | Can be false? | On a successful red thread |
|---|---|---|---|---|
| Anchor | A photograph (a shot can give several) | B/W print with a typed caption | Never | Stays |
| Detail | A darkroom enlargement | Round loupe crop print with a typed caption | Never | Stays |
| Statement | What people say | Yellow lined paper with portrait | Yes | LIE stamp (a lie) or WRONG stamp (an honest mistake); new topics; gates may open |
| Person | Talking to someone | Portrait card | No assertions | Used for WHO |

Assertions have two forms:

- `AT(entity, place, from, to)`: an entity (person or object) is at a place during a time window.
- `PROP(key, value)`: a property of the case, for example `death.cause = blow`, `killer.height = tall`.

Two assertions conflict when `AT` puts the same entity in two incompatible places in overlapping windows
(places form a hierarchy; a place does not conflict with its parent), or when `PROP` gives the same key
two different values.

### 9.3 Board

- Cork board, full screen. New prints land in a tray at the bottom; the player drags them anywhere. The
  layout persists and is the same board that hangs on the office wall.
- Centre: a blank report sheet with three slots: **WHO**, **WITH WHAT**, **WHY**.
- **Black thread** (left-drag from a card's pin to a slot) pins the theory. WHO takes a suspect (one of
  the four with something to hide, or the detective himself once his reflection is on the board; not the
  police, not the dead), WITH WHAT an object card, WHY a motive card. A filled slot makes claims (CASE01
  section 4.5). The detective is never offered: the mirror print simply fits the slot like a portrait.
- Under the slots a typed line reads "Time of death: ____". It fills in (23:41) the moment the player
  holds the watch or the crane log; from then on a pinned WHO is checked against alibis, before that
  only against height. The rule is never explained; the blank line is the hint.
- **Red thread** (right-drag between two cards, or a card and a filled slot) claims a contradiction. It
  is checked at once:
  - an anchor or detail against a statement that conflicts: LIE stamp (or WRONG for an honest
    mistake); the speaker gets a new topic; a gate may open;
  - an anchor or detail against a filled slot that conflicts: the theory breaks, the thread snaps and the
    slot empties. No cost: being wrong in private is free;
  - a statement against a filled slot: "Word against word." Nothing happens, no cost;
  - anything else: "No contradiction." The thread drops and the night loses 60 minutes.
- **Send the report** appears when all three slots are filled. After night 3 the report goes anyway,
  with whatever is pinned; an empty WHO is the misadventure ending. If the player holds the crates photo
  (A15), a second choice appears: attach it or keep it. A confirmation shows the report as one typed
  sentence.

### 9.4 Darkroom, dawn and monologue

**The drive.** The car at the curb outside the Lighthouse takes the detective to the office and back:
a fade and a short authored car shot, 10 minutes of case time each way. Talks and harbour photographs
need the harbour; developing, the loupe and the office subjects need the office. Night 1 starts at the
rope on the pier (the prologue drive is a cutscene); nights 2 and 3 start in the office, and the drive
down ends on an arrival shot that frames the night's timed scene. Dawn always cuts to the darkroom.

**Developing mid-night.** At the office the player can develop the roll so far at any time: 30 minutes,
the red "IN USE" lamp lights, then the loupe (step 2) runs on those prints. Details found this way work
at once, which is how a fast player has the case ready by the first dawn.

**Dawn** is triggered at 06:00 or by "End the night" on the board. Time does not run at dawn.

1. The detective walks into the darkroom (cut). The rest of the night's frames hang on a line under the
   red safelight, developing over a few seconds.
2. **Loupe.** The player moves a loupe over any print. Prints of shots with a detail have one authored
   hotspot; when the loupe sits on it, the crop resolves into a hand-made close-up texture and a Detail
   card drops into the tray. Other prints show plain grain.
3. **Monologue.** At the board, one VO line comments on the pinned theory: the first matching row of the
   case's monologue table (CASE01 section 7). Subtext only; it never names a clue.
4. Night 2 or 3 starts at 00:00 in the office, with 12 fresh plates; the night's timed scene is
   already in place, and the drive down shows it in the arrival shot.

### 9.5 Time

| Action | Cost |
|---|---|
| A photograph | 1 plate, 10 min |
| A new talk topic | 15 min |
| Wrong red thread | 60 min |
| Developing at the office mid-night | 30 min |
| The drive, harbour to office or back | 10 min |
| Walking, reading, inspecting, showing a print, board, loupe | free |

The clock moves only on these actions, never in real time: the player can stand in the rain as long as
they like.

Night 1 runs 02:00-06:00 (the body was found at 01:10); nights 2 and 3 run 00:00-06:00. The police
hold the Agatha as a witness ship until the inquest on Monday at noon, and her charter needs the Monday
morning tide: the report must go by dawn after night 3, or it goes with whatever is pinned.

### 9.6 Probable cause gates

A suspect's private space opens only after one of their statements is stamped LIE:

- Captain Kordes's cabin: after his "aboard all evening" lie, he shows his clearance certificate there
  to save face, and the crates are in plain view.
- Leo Varga's locker in the customs office: after his "out on CL-2 from half past eleven" lie,
  Sgt. Brannock opens it.

### 9.7 Redundancy

Every key fact is provable two ways (CASE01 section 5), at least one of them still available after the
first night. A player who misses a frame on night 1 is slower, never stuck. `case_check.py` and the
`Case01Solvable` test enforce this.

### 9.8 Endings and rating

| Ending | Condition |
|---|---|
| Whole truth | Proven (WHO = the detective, WITH WHAT = the tripod, WHY = the Meridian print or Kordes's envelope in the drawer, and frame 1 or the mirror enlarged) and the crates photo in Brannock's carbon: the detective and Kordes both go down, the Agatha misses her tide; no fee |
| Confession | Proven, crates withheld: the detective gives himself up, the Agatha sails on the Monday tide |
| Not on film | WHO = the detective, not proven. Brannock: "You want to hang yourself, detective, bring me more than a mirror." Misadventure |
| Misadventure | WHO empty when the report goes: the Agatha sails, the inquest records misadventure |
| The wrong man | WHO is anyone else (usually Varga): an innocent is accused; Kordes pays in full ("I needed a name before the tide. I didn't care whose."); the detective counts the fifties at the bar and looks up into the mirror |

After any ending: case card with a rating (A: proven, closed by night 2, at most one wrong red thread;
B: proven; C: not on film; D: wrong man or misadventure) and "Reopen the case" (restart from night 1).
Before the ending text the reconstruction plays (Must, CASE01 8): the player's own prints of the murder
in night order, gaps marked "not on film", always ending on the back-bar mirror. Every player leaves
knowing who did it.

## 10. Onboarding (first five minutes, no text tutorials)

1. 01:40, the darkroom, under the safelight: a print from another job comes up in the tray (a man in a
   customs greatcoat at the door of room 9, Hotel Meridian) and goes up on the drying line. VO: "The
   Meridian job was coming up in the tray when the phone rang. It always does." The phone: Kordes. "A
   customs man is dead under my ship. You'll have heard. The police are sitting on my cargo. I need a
   name before Monday. Quietly. Fifty a day, two hundred if it's over before I sail." VO: "I said yes
   before he named the fee." He slides a plate holder into the box, dark slide reversed, and takes the
   camera from the desk. The counter reads 11. VO: "Twelve plates a night. Anything I don't shoot never
   happened."
2. Down the stairs, through the bar (Nora nods), out to Pier 5. VO: "Harrowmark rains like it's paid by
   the hour. Somebody always drowns in it."
3. Sgt. Brannock at the rope (S01): "Slipped and drowned, if you ask me."
4. At the body, the camera prompt (RMB, LMB) appears; the only valid subject is the wound. The flash
   fires; the print drops into the tray.
5. The board opens with S01 and the wound print highlighted; prompt for a right-drag. WRONG stamp.
   Brannock: "Then it's murder. The van takes him at six." The report slots light up.
6. From here the player is free. Prompt chips appear once per verb (F, Tab, talk) and never again.

## 11. Level: Harbour of Harrowmark, Pier 5

One harbour map, no loading inside the case, and the agency office as its own small map (`L_Office`),
reached by the drive. Layout, metrics, sightlines, photo nooks and night states:
`docs/design/LEVEL.md`.

| Area | Content | Opens |
|---|---|---|
| Agency office (uptown, `L_Office`) | Three desks, phone, corkboard wall, window over a wet uptown street with a cinema's neon, darkroom in the back room with the drying line (Meridian print), the tripod by the darkroom door, the desk drawer (Kordes's envelope) | Start (hub, dawn; reached by the drive) |
| The Lighthouse bar | Nora, cash register, jukebox, back-bar mirror (the detective's reflection), rain windows; the yard behind it where the killer left his car; the detective's car at the curb | Start |
| Street and gate | Fence gate, gate booth (hatch and window only): visitors' book, movements chalkboard; coroner's van (night 1) | Start |
| Port office | Customs office (enterable): Morrow's desk, duty roster, store book, Varga's locker | Start (locker gated) |
| Pier 5 | Body and the dark, gouged lamp at the edge; lower landing; the catwalk under the deck (the killer's route, the lost boathook); fish crates with a bale hook; crane with cab | Start |
| Customs berth | Pontoon on the west side of Pier 5 under the crane's swing, CL-2, Varga | Start |
| SS Saint Agatha | Gangway with the watch book at its head, No. 2 hatch with the tally, captain's cabin in the aft deckhouse; gangway foot on the quay (night-3 scene) | Start (cabin gated) |

Progression comes from information, not keys: the first night shows four plausible suspects; red threads
clear them all, and a fifth face is waiting in the bar mirror; lies open private spaces; the darkroom turns old prints into new evidence. The map itself is
evidence: wherever a document says someone saw something, the player can stand there and see it.

## 12. Architecture (UE 5.8, C++ first)

Plugin `Plugins/Features/Investigation/` (runtime) and `Plugins/Features/InvestigationUI/` (UMG):

| Class | Role |
|---|---|
| `UCaseDefinition` (PrimaryDataAsset) | Places hierarchy, people, shots, cards, details, theory templates, gates, monologue table, endings, rating; one asset per case |
| `FCaseAssertion` | `AT` / `PROP` assertion struct; `Conflicts(A, B, Places)` pure function |
| `UInvestigationSubsystem` (GameInstance subsystem) | Held cards, taken shots, film per night, board state, theory slots, red thread resolution, clock, expiry, gates, dawn processing, monologue pick, ending and rating |
| `UPhotoCameraComponent` | On the detective: viewfinder mode, subject validity (frustum, projected size, occlusion traces, flash range or scene light), shutter, flash light pulse, `SceneCaptureComponent2D` to a render target, print texture |
| `UPhotoSubjectComponent` | On photographable actors: shot id, validity bounds and sample points, night it disappears |
| `UInspectComponent` | On readable and inspectable actors: inspect view, free |
| `UTalkComponent` | On people: topics with conditions (cards held, statement refuted), statements granted |
| `UBoardWidget` | Cork board, card drag, threads drawn in `NativePaint`, red thread checks, report flow with the attach choice |
| `UDarkroomWidget` | Prints on a line, loupe with zoom, authored hotspots per detail with their close-up textures |

Data first: cards, assertions, shots and the monologue live in the case asset (authored from
`CASE01.md` and `data/case01.json`), so the case can be tuned without code. Automation tests:

- `Project.Investigation.Conflicts`: assertion conflict rules (places hierarchy, time overlap, PROP).
- `Project.Investigation.Case01Solvable`: the C++ port of `case_check.py`: anchors and details
  consistent with each other and the truth; the true theory raises no conflict; every false WHO and WITH
  WHAT and every lie has at least two refuters, one of them available after night 1; every gate opens on
  some path; the reference paths replay with the expected stamps, monologue lines, endings and ratings.
- `Project.Investigation.Darkroom`: details appear only for developed shots with a hotspot; expiry; film
  refill at dawn.
- `Project.Level.PhotoNooks`, `Sightlines` (Should) and `Routes`, `NightStates` (Could): the map keeps
  the promises of `docs/design/LEVEL.md` (section 24). In the Must phase the greybox is checked by hand: a
  stopwatch walk of the LEVEL routes and a viewfinder check of every photo nook.

## 13. Scope tiers

**Must** (the MVP, phase 1)
- Board with cards, black and red threads, LIE and WRONG stamps, theory breaks, report with the attach
  choice.
- Contradiction engine and case asset with all of Case 1's shots and cards.
- Camera: viewfinder, validity brackets, shutter, flash light pulse, film counter, print texture.
- Darkroom: mid-night develop and dawn, prints, loupe, six hotspots with placeholder close-ups; the
  pre-exposed frame 1.
- Greybox of all areas: office with darkroom (`L_Office`) and the drive, bar with its yard, Pier 5 with the catwalk and landing, gate booth,
  customs office, Saint Agatha with deck and cabin; all subject and clue actors and characters placed.
- Lighting of the hero areas (edge, catwalk, crane over the berth, street, darkroom) at final quality by
  the end of day 2: the visuals case and the NVIDIA case rest on them.
- Clock, three nights, expiry, the two timed scenes (posed mannequins acceptable), forced report, five
  endings (text screens acceptable), rating card.
- Talk topics as a simple list with subtitles; dawn monologue as subtitles.
- Ending reconstruction: the player's own prints as a slideshow, gaps marked "not on film", ending on the
  mirror (CASE01 8). It delivers the twist to the players who hang Varga; still images are enough.

**Should** (phases 2-3)
- Final art for all areas; rain, wetness, the lighthouse beam, the darkroom safelight.
- Characters from Reallusion with idles and talk loops.
- Full voice-over (TTS) including the narration, and the noir score with the darkroom variant.
- Flash FX: rain droplets lit mid-air, afterimage; print develop effect.
- Inspect view with 3D hero props; stamp and thread-snap effects.
- Settings menu with RT and DLSS options and a non-RT fallback.
- One small acting tell on each lie line (CASE01 4.4).

**Could**
- Lip-sync (Reallusion AccuLips or NVIDIA Audio2Face, whichever imports cleanly into 5.8).
- Gamepad, Russian localization, photo mode, office board scene for the next case.

**Cut order** if a day ends behind plan, first cut first: (1) the tug and funnel smoke; (2) the crane cab
interior, replaced by a fixed inspect view of the log; (3) the night-3 hand-off (a decoy beat; Varga's
WHY keeps A16, A17, D03); (4) showing prints in talk; (5) mid-night developing (dawn only; the A rating still holds on
night 2). Never cut: sharp or grainy prints, the board's red thread, the time-of-death line, frame 1,
the mirror, the reconstruction.

## 14. Risks and answers

| Risk | Answer |
|---|---|
| Case authoring is wrong or unsolvable | Case is data; `case_check.py` now and `Case01Solvable` in engine gate every change |
| Photo validity feels fiddly | Generous thresholds, brackets show validity before the shot, the shutter refuses invalid frames |
| Player runs out of film or misses an expiring shot | 11 plates on night 1 against 5 for the fastest proof and 11 for the careful one; two refuters for every key fact, one after night 1 |
| Brute-forcing red threads | 60-minute cost; statements against theories never resolve |
| The darkroom is a pixel hunt | Only six hotspots, each on a print of an obviously important shot; the loupe snaps when close |
| The twist feels like a cheat | The VO never lies (CASE01 11 lists the writing rule); frame 1 develops for every player; "Meet H.M.", Nora's "your car was in my yard", Holt's "nobody after him", the ash splinters and the J.K. envelopes all point at the office; the reconstruction shows the mirror to everyone |
| The twist is found by accident | The detective cannot be pinned until his reflection is on the board, and the proof needs a developed detail (D05 or D06), so a lucky guess ends as "Not on film" (C) |
| Mid-night develop makes the darkroom a chore | It costs 30 minutes and is optional; dawn develops everything anyway |
| Players miss the timed scenes | Neither is required (each is one of several ways to its card's job); both are framed by the arrival shot at night start and lured by sound; both are in place from night start until 01:30 on the clock |
| Judges play for 20 minutes and stop | Night 1 alone can close the case with an A; Send appears the moment all three slots are filled; the prologue runs under a minute |
| The NvRTX 5.8 branch is late or broken | Decided day 1: NvRTX 5.8 branch (built, RTXDI + DLSS RR verified). RTXDI is the only NvRTX-only dependency and is one cvar: `r.SampledLighting.Direct 0` falls back to standard ray-traced/virtual shadow maps with the same lights, so stock 5.8 still runs the game |
| Players solve it in five minutes | A player who doesn't suspect the narrator can't: he has to think of the mirror. One who does can close in 80 minutes of game time, by design: that is the A run for a replay |
| Reallusion export or retarget eats a day | Characters are in phase 2; greybox uses UE mannequins |
| RT performance on judges' PCs | Scalability presets, DLSS/FSR/TSR, frame generation toggle, software Lumen fallback tested |
| AI-generated meshes look inconsistent | One master material with shared wetness; strict grade; hero props cleaned in Blender |
| Package over 1 GB | 2K textures by default, 4K only for hero props; Nanite; audio in OGG |

## 15. Open questions for the owner

1. Jam theme: none (owner confirmed). Resolved.
2. Detective: male (decided), name still open.
3. Game title: **HarrowMark** (owner, 2026-11-12), after the port city. In-world text spells the city "Harrowmark"; the title logo keeps the capital M. Retired: "Night Exposure", "Exposure", and "HarrowMouth" (already used in other detective fiction). A web check on 2026-11-12 found only one fan-made "Harrowmark Company" in a fantasy setting.
4. SpaceXSI: credits for Grok and Cursor (coding assistants, not an asset source). Resolved.
5. Monologue and ending lines are drafts, to be polished against the reference books.
6. The twist (owner, 2026-11-12): the detective is the killer, knows it, and the VO only omits. Resolved.
