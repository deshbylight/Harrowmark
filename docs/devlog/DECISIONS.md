# Decision journal

**Internal. Contains the Case 1 solution. Never copy a spoiler row into `docs/DEVLOG.md` before the
owner publishes the post-jam entry.**

Raw material for the dev log: every design or pipeline decision, what it replaced and why. The docs role
appends a row whenever an owner directive, a planner decision or a pipeline fix changes direction
(newest last). Rows marked **S** are spoilers.

| Date (UTC) | Decision | Replaced / rejected | Why | By | S |
|---|---|---|---|---|---|
| 2026-11-12 | Core: a press camera, 12 plates a night. Evidence exists only if photographed; talk costs minutes, proof costs a plate; the clock moves only on actions | - | One verb carries the whole genre: choosing what to photograph is the deduction | owner + Claude | |
| 2026-11-12 | No supernatural layer | Ghosts / a "dead" layer | The camera is the only special power; a plain recording machine fits noir | owner | |
| 2026-11-12 | Agency, city and game are all named HarrowMark | "Night Shift Bureau" (agency); "Night Exposure", "Exposure", "HarrowMouth" (titles) | "HarrowMouth" already exists in other detective fiction | owner | |
| 2026-11-12 | Colour neo-noir, not black and white | B/W | Aimed at the NVIDIA RTX award: wet RT reflections and coloured practical light | owner | |
| 2026-11-12 | Red is reserved for the investigation signal; no red props or costumes | - | The red thread must be the only red thing on screen | owner | |
| 2026-11-12 | Case 1 twist: the detective (the player) killed Morrow. The voice-over never lies, it only omits 23:05-00:05 | Four suspects as the only candidates | "Hero knows" variant chosen by the owner; Pillar 3 ends with the man holding the camera | owner | S |
| 2026-11-12 | The only WHO proof against the detective is a photo of his reflection in the bar mirror (A27), backed by D05 (frame 1, exposed before the game) or D06 (salt line and palm cut) | A confession scene | The camera points both ways; the RT mirror becomes core gameplay, not decoration | owner + Claude | S |
| 2026-11-12 | Varga is the decoy: his lie is real, but the store book (A26) and the clearance (A22) clear him | - | Most players hang the wrong man (E4); the reconstruction then shows them the mirror | owner + Claude | S |
| 2026-11-12 | Soft spot answered: frame 1 wasn't fogged because the phone rang (prologue) | - | Closes the obvious plot hole | owner + Claude | S |
| 2026-11-12 | No hint button and no helper character; a dawn voice-over in subtext plus two-way redundancy, enforced by `Tools/case_check.py` | On-demand hints, a hint NPC | Hints break noir tone; redundancy keeps it fair without breaking it | owner | |
| 2026-11-12 | Voice-over: adult hard-boiled, short, ironic, never an instruction | Explanatory lines | Owner: "a five-year-old's monologue" | owner | |
| 2026-11-12 | First person with a full-body detective; noir framing from authored shots | Third person (inertia from the first GDD draft) | "Third person loses all the cinematic feel"; your own body in the mirror is the twist | owner (INBOX) | |
| 2026-11-12 | Clue documents are rendered from case data by `Tools/render_docs.py` with open fonts | AI-generated document images | Text under the loupe must be exact and match the case data | Claude | |
| 2026-11-12 | Prop reference images: gpt-image-2, one object per image, grey background, no background removal | nano-banana-pro (lost the A/B on the developing trays) | Cleaner single-object references | Claude | |
| 2026-11-12 | Props: Meshy t2.5 smart topology + meshy-7 PBR (about 15 credits) | meshy-7.1 high-poly then remesh | Game-ready topology directly | Claude | |
| 2026-11-12 | Detective CH01: Meshy 7.1 Ultra mesh (361k, 4K PBR) on a Tripo 78-joint rig; fallback Tripo H3.1 (95k) | Meshy auto-rig (24 joints, no fingers), Meshy t2.5, Rodin HD | First person needs fingers for camera handling | owner + Claude | |
| 2026-11-12 | Pipeline rule: check the Meshy preview before rigging; image-to-3D from an uploaded file, not a chained task id with a pose option | - | The chained call returned an unrelated elf (credits lost) | Claude | |
| 2026-11-12 | NPC faces: fal flux-pro portraits as input for Reallusion Headshot (invented faces) | - | Sponsor pipeline with facial morphs and lip-sync | owner + Claude | |
| 2026-11-12 | Engine: NvRTX 5.8 source build; claim no RTX feature until it works in game | Stock 5.8 | NVIDIA award, without overclaiming | owner | |
| 2026-11-12 | Official Rules folded into docs/JAM.md: no real trademarks, Teen rating, English only, no extra software (non-RT fallback is a Must), video 1920x1080+, dev log submitted with the game, VCS history may be requested | Summary from the jam page only | The rules PDF is binding; 720p video and an RTX-only build would each break eligibility | owner + Claude | |
| 2026-11-12 | `jf video` records at 1920x1080 and flags lower resolutions | 1280x720 capture | Official Rules 7(c) | Claude | |
| 2026-11-12 | Theme "Director's Cut" read as Epic framed it: build the shelved idea, direct the agents, decide the final cut. No in-game film or "cut" features for it | A literal reading (narrator as the theatrical cut, a director's-cut ending), dropped within the hour | Epic's text is about the developer, not the content | owner | |
| 2026-11-12 | The agency office moves uptown to its own small map (`L_Office`, the hub of every case); harbour and office are joined by a drive (fade + authored car shot, 10 min of case time each way); night 1 starts at the rope, dawn cuts to the darkroom | Office upstairs from the Lighthouse bar on the one harbour map | Owner: the office is the series hub, not part of one case; the arrival shot frames each night's tableau | owner + Claude | |
| 2026-11-12 | First-person body: the world body is hidden from the owner but casts shadows and shows in RT reflections; a second, owner-only copy with the head bone hidden is what the player sees | UE 5.8 first-person primitive types (WorldSpaceRepresentation) | NvRTX RTXDI shadow rays skip the first-person world-space mask, so that body cast no shadow; the hidden-to-owner mesh stays in the ray tracing scene | Claude | |
| 2026-11-12 | Lamp lights sit above their posts and below the crane beam, never inside a mesh | Lights placed inside the greybox post cylinders | Under RT shadows (RTXDI) every enclosed light was fully occluded: the harbour was lit by GI alone and nothing cast a shadow | Claude | |
