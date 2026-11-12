# Credits and third-party assets

Every external file in the game is listed here. `jf fetch`, `jf lib import-pack` and `jf credit` add rows.

| Asset | Source | License | Author | URL |
|---|---|---|---|---|
| Special Elite (font) | Google Fonts | Apache 2.0 | Astigmatic | https://github.com/google/fonts/tree/main/apache/specialelite |
| Homemade Apple (font) | Google Fonts | Apache 2.0 | Font Diner | https://github.com/google/fonts/tree/main/apache/homemadeapple |
| Caveat (font) | Google Fonts | SIL OFL 1.1 | Impallari Type | https://github.com/google/fonts/tree/main/ofl/caveat |
| Courier Prime (font) | Google Fonts | SIL OFL 1.1 | Alan Dague-Greene (Quote-Unquote Apps) | https://github.com/google/fonts/tree/main/ofl/courierprime |
| Clue document textures T_C02-T_C24, darkroom close-ups T_D02-T_D04 | Rendered by `Tools/render_docs.py` (project code) | Project | Project |  |
| Hero/clue props: boathook C03, broken bottle C04, flashbulb CA02 | Meshy (gpt-image-2 reference, meshy-t2.5 + meshy-7 PBR), generated 2026-11-12 | Meshy output, sponsor credits | Generated with Meshy | https://www.meshy.ai |
| Set props: police tarp, rope post E09, hatch cover E22, chart table + drawer E23, developing tray E48, door plate E53, typewriter cover, wall chalkboard E55 | Meshy (as above), generated 2026-11-12 | Meshy output, sponsor credits | Generated with Meshy | https://www.meshy.ai |
| Detective CH01 (mesh and textures) | Meshy 7.1 Ultra + meshy-7 PBR 4K, generated 2026-11-12 | Meshy output, sponsor credits | Generated with Meshy | https://www.meshy.ai |
| Detective CH01 skeleton (78 joints) | Tripo animate_rig v3.0, 2026-11-12 | Tripo output, sponsor credits | Generated with Tripo | https://www.tripo3d.ai |
| NPC face portraits CH02-CH07 (Headshot input, invented faces) | fal flux-pro v1.1 ultra via `Tools/art/gen_portraits.py`, 2026-11-12 | fal output, sponsor credits | Generated with fal | https://fal.ai |

## Where each asset is used

- Special Elite (font): Card text; typed entries on clue documents (T_C12, T_C15, T_C22)
- Homemade Apple (font): Morrow's notebook (T_C02) and its darkroom enlargement (T_D03), Varga's signature (T_C12)
- Caveat (font): Handwritten logs, chalk boards, tally entries and signatures (T_C02, T_C07, T_C08, T_C10, T_C14, T_C20-T_C23, T_D02)
- Courier Prime (font): UI; printed forms, register tape, clearance stamp, the stencil on the boathook grip (T_C03, T_C07-T_C12, T_C21, T_C22, T_D04)
- Clue document textures T_C02-T_C24, darkroom close-ups T_D02-T_D04: Clue props C02, C03, C07-C10, C12, C14, C15, C17-C20 (A20-A22, A12 slate); close-ups CD02-CD04
- Meshy props: clue props A06, A07, the camera flash; pier scene, ship deck, darkroom, captain's cabin door, office, gate booth (C17 board)
- NPC portraits: source photos for Character Creator Headshot (CH02-CH07); raw files in `art_src/Generated/Characters/<id>/portrait/`, the CC build is scripted by `Tools/reallusion/`
- Detective CH01: the player body (first person, mirror reflection) and authored shots. Raw sources in `art_src/Generated/` (not in git); the D06 clothing clues are painted by `Tools/art/ch01_clues.py` (project code)

Licence texts: ship `apache_*_LICENSE.txt`, `ofl_*_OFL.txt` from the font download folder in the game's
`Content/UI/Fonts/` licences folder (or `docs/licenses/`).
