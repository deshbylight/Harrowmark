# Jam rules: 2026 Epic AI MegaJam (itch.io)

Every agent reads this before planning, importing assets or preparing a release. Source pages:
[jam page](https://itch.io/jam/2026-epic-ai-megajam). The Official Rules PDF linked from the jam page
is binding and overrides this summary; the owner reads it (the pipeline could not fetch it).

## Schedule

| Event | PST | EST | UTC |
|---|---|---|---|
| Theme reveal | 2026-11-11 11:00 | 2026-11-11 14:00 | 2026-11-11 19:00 |
| Jam start (game work may begin) | 2026-11-11 21:00 | 2026-11-12 00:00 | 2026-11-12 05:00 |
| Submission deadline (late = disqualified) | 2026-11-15 17:59 | 2026-11-15 20:59 | 2026-11-16 01:59 |

## What may exist before the start

- Allowed: tools, agents, MCP servers, scripts, prompts, pipelines, reusable frameworks and assets.
- Not allowed: building the game itself (its design, levels, gameplay code, game-specific assets).
- Everything made earlier must be declared: `docs/PREMADE.md` lists it and goes into the submission
  page. Nothing game-specific is committed before the start time.
- Custom gameplay must go clearly beyond Epic's starter templates (template code alone is not a game).

## Assets and AI

- Any AI tools are allowed.
- Fab and third-party assets are allowed with a license that permits use in a published game, and each
  one must be listed: every external file has a row in `docs/CREDITS.md` (source, license, author, URL).
  `jf fetch`, `jf lib import-pack` and `jf credit` add rows; an asset without a row is a release blocker.

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
  the Glass Box prize).

## Judging (each 1-5)

Ambition and innovation, Theme, Fun, Gameplay, Visuals, Level design and progression, Audio.
Plan for every criterion: a polished short game beats a broken large one, but each criterion needs
visible work (audio and level progression are scored separately, not as polish).

## Award modifiers

Best Team, Solo Driver, Render Royalty (visuals, Unreal Engine), Pocket Powerhouse (mobile),
Glass Box (dev log), Walkie-Talkie (UEFN only, not applicable here).

## Release checklist (release role, then owner)

1. `jf package --zip` succeeds; the packaged game starts (`jf shot --packaged`), no cook errors.
2. Zip name is `TeamName_GameName.zip`, size under the limit, Development configuration.
3. `docs/CREDITS.md` covers every external asset, each with a named license (no `UNKNOWN` rows); `docs/PREMADE.md` is current.
4. `jf video --seconds 45 --exec "<demo scenario>"` gives a 30-60 s mp4 with game audio.
5. `docs/DEVLOG.md` is current; `docs/SUBMISSION.md` holds the page text, controls and credits.
6. Owner: upload, video link, dev log link, team data, submit before the deadline.
