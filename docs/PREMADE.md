# Declaration of work made before the jam

The jam allows tools, agents and frameworks prepared in advance and requires them to be declared.
This file is that declaration; its text goes into the submission page. Everything not listed here
(game design, gameplay code, levels, game-specific assets, music) was made during the jam.

## Development pipeline (not shipped in the game)

- JamForge: the multi-agent orchestration pipeline (task board, worktrees, merge gate, role prompts,
  `jf` command line) that ran the AI agents building this game.
- Agent role prompts and team rules (`CLAUDE.md`, `docs/rules/`), written before the jam as generic
  Unreal Engine guidance.
- Asset tooling: editor Python scripts in `Tools/ue/` (asset import, validation, level assembly from
  JSON), a procedural sound-effect synthesizer (`jf sfx`), free-asset fetchers for CC0 libraries
  (`jf fetch`), a local asset library index (`jf lib`), Blender batch runner (`jf blender`), sound
  file checks (`jf audio-check`: loudness, true peak, DC offset, silence) and a mesh checker
  (`jf meshcheck`, `Tools/blender/check_mesh.py`: topology, normals, UVs, scale, pivot).
- Knowledge library: digests in `docs/kb/` marked `Origin: pre-jam library`, researched before the jam
  by AI agents from openly accessible sources (each digest lists its sources and their terms). They hold
  generic engineering, art, audio and production knowledge, no design or content of this game. Digests
  marked `Origin: jam` were written during the jam.
- Self-improvement tooling: the `forge/` folder skeleton (layout test, journal format) and any
  generic scripts, playbooks and agent prompt overlays the learn agents built in the pre-jam knowledge
  workspace (copied by `jf init --kb-from`). They improve how agents work, not what the game contains.
  `forge/LEARNING.md` rows dated on or after the jam start were written during the jam.
- Media tooling: the media outbox and publisher (`jf media`, `jf publish`): package checks for
  social posts and owner-run posting through free platform APIs. Clips, posts and their text were
  made during the jam from the game itself.
- Test tooling: the JamProbe plugin (headless screenshots, scripted console commands, performance
  capture, gameplay video capture). It only acts when started with its command-line switches.

## Reusable frameworks shipped in the game (generic, no game content)

- JamCore plugin: empty shared-contract module and log category.
- JamUI plugin: generic code-built UI layer (screen stack, themed menu screen, button focus and
  navigation, theme settings). Game screens built on it were made during the jam.
- Project template: engine config defaults, empty game module, build targets.

## Third-party assets

Listed with source and license in `docs/CREDITS.md` (Fab, CC0 libraries and others). Assets from the
owner's own library are listed there too.

<!-- Release role: add any later pre-jam item the owner reports; keep this list factual. -->
