# Jam rules: 2026 Epic AI MegaJam (itch.io)

Every agent reads this before planning, importing assets or preparing a release. Source pages:
[jam page](https://itch.io/jam/2026-epic-ai-megajam). The Official Rules PDF linked from the jam page
is binding and overrides this summary. The owner read it on 2026-11-12 (version of 2026-11-05); the
section numbers below, written as "OR 7(c)", refer to it.

## Schedule

| Event | PST | EST | UTC |
|---|---|---|---|
| Theme reveal | 2026-11-11 11:00 | 2026-11-11 14:00 | 2026-11-11 19:00 |
| Jam start (game work may begin) | 2026-11-11 21:00 | 2026-11-12 00:00 | 2026-11-12 05:00 |
| Submission deadline (late = disqualified) | 2026-11-15 17:59 | 2026-11-15 20:59 | 2026-11-16 01:59 |

## Theme

**Director's Cut**, announced at the kickoff stream (2026-11-11). Epic's wording (Unreal Engine on X,
the jam page has no theme text):

> Your vision, your specific edits and ultimately: your final cut - your Director's Cut.
> Every developer has that one feature that didn't quite make it, the mechanic scoped out while sleep
> deprived at 2 AM or that idea you've had shelved and never quite found a place for it yet. Well, now's
> your chance to finally build it all in this year's Epic MegaJam.
> If you're utilizing AI agents in your workflows, you're directing those too! Set the vision, delegate
> your tasks and decide what goes in and what doesn't. Aim for ambition and direct your path accordingly!

The theme is about the developer, not the game's content: no genre, setting or film motif is required,
and agents must not add in-game "cut" or film-editing features for it. The judging line is "a fresh,
clever take on the theme"; it is shown through the owner's shelved idea, the ambition of the build and
the dev log of how the agents were directed.

## What may exist before the start

- Allowed: tools, agents, MCP servers, scripts, prompts, pipelines, reusable frameworks and assets.
- Not allowed: building the game itself (its design, levels, gameplay code, game-specific assets).
- Everything made earlier must be declared: `docs/PREMADE.md` lists it and goes into the submission
  page. Nothing game-specific is committed before the start time.
- Custom gameplay must go clearly beyond Epic's starter templates (template code alone is not a game),
  and the game must not be an obvious extension of an official Unreal tutorial project (OR 7(d), 7(e)).
- Epic may ask for the pre-jam materials **or the version control history** to verify that the game was
  made during the jam (OR 7(h)). Commit work regularly with honest timestamps; never squash or rewrite
  history on `main`.

## Assets and AI

- Any AI tools are allowed (OR 7(a)). The extent of AI use is context only and does not raise or lower
  a score on its own (OR 9). AI-generated content is not a sourced asset (OR 15(c)).
- Fab and third-party assets are allowed with a license that permits use in a published game, and each
  one must be listed: every external file has a row in `docs/CREDITS.md` (source, license, author, URL).
  `jf fetch`, `jf lib import-pack` and `jf credit` add rows; an asset without a row is a release blocker.
  Sourced assets are noted at submission and are not scored (OR 15(c)).

## Content restrictions (OR 8, 15(d))

- **No trademarks or logos** anywhere in the game, except the Unreal Engine logo per Epic's branding
  guidelines. For a 1949 setting: no real brands on cameras, film boxes, cigarettes, bottles, cars, ships,
  signs or documents. Invent every name; check generated textures and props for readable real logos.
- **Teen rating (13+, ESRB Teen or equivalent).** A murder is fine; no gore. The body and the wound
  photo stay non-graphic (no open wounds, no blood pools; stains are dark, never red per STYLE.md).
  No nudity, no hate, no real people's likenesses (portraits are invented faces).
- No content that disparages Epic or any person or product.
- **English only:** in-game text, audio, the video and every entry field (OR 7(i)).
- **No additional software to run** (OR 7(f)): the packaged build must start and be playable on a PC
  without an RTX card. Ray-traced features need a working non-RT fallback that the game picks itself.

## Team and engine

- Solo or up to 5 people (18+, each with an Epic Games account); unlimited AI agents.
- Unreal Engine or UEFN, not both. This project is Unreal Engine 5.8.

## Submission (Unreal Engine)

- A packaged Development build (Windows here). Unpackaged projects are not accepted.
- One zip named `TeamName_GameName` (`jam.toml [jam] team_name / game_name`), under 1 GB for the
  browser upload; above that the owner uploads with butler. `jf package --zip` builds it.
- The owner uploads to their own itch.io account and submits it to the jam. Agents never publish.
- Submission form data: team name, member names with their Epic account emails (owner fills these),
  game name, a 30-60 s gameplay video link (`jf video`), dev log link (`docs/DEVLOG.md`, required for
  the Glass Box prize), tools used (needed for partner prizes such as the NVIDIA RTX Award and Hyper3D).
- **Video: 30-60 s of recorded gameplay at 1920x1080 or higher** (OR 1(e), 7(c)). `jf video` records at
  1920x1080 and flags a lower resolution.
- **Dev log:** must be submitted together with the game, before the deadline (OR 10). Anything added
  after the deadline does not count.
- One submission per team. A submission can win at most one main prize and one modifier prize (OR 13).

## Judging (each 1-5)

Ambition and innovation, Theme, Fun, Gameplay, Visuals, Level design and progression, Audio.
Plan for every criterion: a polished short game beats a broken large one, but each criterion needs
visible work (audio and level progression are scored separately, not as polish).
Ties are re-scored with double weight on Fun (OR 9).

## Award modifiers

Best Team, Solo Driver, Render Royalty (visuals, Unreal Engine), Pocket Powerhouse (mobile),
Glass Box (dev log), Walkie-Talkie (UEFN only, not applicable here).

## Release checklist (release role, then owner)

1. `jf package --zip` succeeds; the packaged game starts (`jf shot --packaged`), no cook errors.
2. Zip name is `TeamName_GameName.zip`, size under the limit, Development configuration.
3. `docs/CREDITS.md` covers every external asset, each with a named license (no `UNKNOWN` rows); `docs/PREMADE.md` is current.
4. `jf video --seconds 45 --exec "<demo scenario>"` gives a 30-60 s mp4 at 1920x1080 or higher with game audio.
4a. Content pass: no real trademarks or logos in any texture or prop, nothing beyond a Teen rating, all
    text and audio in English; the game starts and plays without ray tracing.
5. `docs/DEVLOG.md` is current; `docs/SUBMISSION.md` holds the page text, controls and credits.
6. Owner: upload, video link, dev log link, team data, submit before the deadline.
