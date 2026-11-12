# Plan

Jam window (rules in `docs/JAM.md`): start 2026-11-12 05:00 UTC, deadline 2026-11-16 01:59 UTC.
No game work exists before the start; late submissions are disqualified.

| Phase | Ends (UTC) | Milestone |
|---|---|---|
| 1 Slice | 2026-11-13 05:00 | Playable vertical slice: core loop end to end, placeholder art, menu, build runs, packaged build smoke-tested |
| 2 Breadth | 2026-11-14 05:00 | Core mechanics and content breadth, first real art pass per STYLE.md, level progression |
| 3 Juice | 2026-11-15 14:00 | Content, feedback, audio and music, VFX, balance, onboarding |
| 4 Freeze | 2026-11-16 01:59 | Feature freeze: bugs, performance, packaging, zip, gameplay video, dev log, page text; release candidate handed to the owner by 2026-11-15 20:00 |

## Current focus

(maintained by the planner; owner plan written 2026-11-12 09:40 UTC while JamForge is stopped. When the planner restarts it takes this list as its starting point.)

Source of truth: this repo only. Design docs (GDD, CASE01, LEVEL, STYLE, ASSETS), `data/case01.json` and the textures were merged from the design session on 2026-11-12; the pre-merge copies are in `Saved/premerge_20261112/`. `python Tools/case_check.py data/case01.json` passes. Raw generations live in `art_src/Generated/` (not in git). Nothing is written to temp or scratch folders.

### Decisions in force

- Camera: first person with a full-body detective (owner directive in `INBOX.md`). Noir framing comes from authored shots.
- Case 1 twist: the detective is the killer. The only WHO proof is the photo of his reflection in the bar mirror (A27, D05/D06). The RT mirror with the player's own body is therefore core gameplay, not decoration.
- Detective CH01: `art_src/Generated/Characters/_review/2_meshy71_ultra_TRIPO_RIG_78j.glb` (Meshy 7.1 Ultra, 361k tris, 4K PBR, Tripo 78-joint rig with fingers). Backup: `3_tripo_h31_RIG_95k.glb` (95k tris, same rig).
- No red on any prop or costume. Red is the investigation signal.

### Phase 1 (to 2026-11-13 05:00): slice

1. First-person controller: walk 3 m/s, FOV 34 deg verified on the `L_Pier5` greybox. Full-body mesh with the head hidden for the owner camera but casting shadow and showing in reflections.
2. Import CH01 into UE 5.8: skeleton, retarget test against the UE mannequin (locomotion, idle) and check the finger bones. If the import or the retarget fails, switch to the backup `3_tripo_h31_RIG_95k.glb`.
3. Bar mirror test, done first: an RT reflection that shows the first-person body, plus a photo taken into the mirror that registers A27. This is the NVIDIA headline check. Decide RTXDI / RT reflections on NvRTX on day 1.
4. Camera loop end to end: raise to the eye (a seamless move, no view switch), shoot (plate count 11 on night 1), develop, loupe, card on the board.
5. Hand animations first: camera raise, flashlight. Then authored shots (prologue, the two timed scenes, endings).
6. Update the docs per the INBOX directive: GDD section 8, STYLE gameplay camera, LEVEL section 18, ASSETS animations.

### Phase 2 (to 2026-11-14 05:00): breadth and the first art pass

7. Clothing clues on CH01 (D06 support, visible in the mirror and in close shots):
   - a dry salt rime on the trouser cuffs and shoes, shin-high;
   - the right glove torn across the palm with a pale ash splinter in it, which matches the palm cut;
   - pale ash sawdust on the right coat cuff and knee, from sawing the tripod leg (A28).
   Stains are dark brown only, never red. No new cards: these strengthen the existing D06. Done as a texture or decal edit on the CH01 albedo plus a small splinter mesh. Update CASE01 D06 and ASSETS CH01 text to match.
8. NPCs (4 suspects + extras):
   - Try Reallusion first (sponsor): Character Creator 5 + Headshot 3 from a fal-generated portrait (original faces, no real people), AccuRIG and CC to UE Auto Setup for facial morphs and lip-sync. This needs CC5 installed on the owner's machine and the sponsor key.
   - Fallback per NPC: Meshy 7.1 Ultra (about 35 credits) + Tripo rig (25 credits).
   - Extras: Meshy auto-rig (24 joints) is enough.
9. Props: the Meshy batch is already in `art_src/Generated/Batch1/`. Import with collision and LODs. Generate the missing props per ASSETS.md, one object per image.
10. Blender cleanup (quad remesh, bake, UV) only if UE performance or deformation needs it. Needs the Blender install path from the owner.

### Phase 3-4

Per the table above: audio and VO pass (adult hard-boiled, subtext only), reconstruction slideshow ending on the mirror, packaging, gameplay video.

### Housekeeping

- Translate `docs/HANDOFF.md` to English (repo rule).
- Add a CREDITS.md row for every new generated asset as it lands.
- Budgets: Tripo 740 credits, Meshy about 650 credits.
