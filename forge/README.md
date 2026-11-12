# forge: what the team has learned

Learn agents (optimizer, toolsmith) keep this folder; every agent reads it. Everything here reaches
main through the normal merge gate, and every merged change is re-checked by another agent.

- `scripts/`: reusable tools (Python, standard library first). Each script has `--help` and a test
  `scripts/tests/test_<name>.py` that runs without Unreal, Blender or network. The merge gate runs
  `python -m unittest discover -s tests` in `forge/scripts` for every forge change.
- `playbooks/`: step-by-step recipes with exact commands, expected output and known failure modes.
- `roles/`: learned prompt overlays. `roles/common.md` is appended to every agent's instructions,
  `roles/<role>.md` to that role's. Changed only through A/B experiments.
- `experiments/<slug>/`: running A/B tests (`experiment.toml` plus `roles/<role>.md`, the treatment).
- `LEARNING.md`: journal of every promoted, rejected or reverted change with its numbers.

One line per entry below: path, what it does, when to use it, measured saving.
`scripts/tests/test_forge_layout.py` fails when a script or playbook is missing from this index.

## Scripts

<!-- - `scripts/<name>.py`: <what>; use when <situation>. Measured: <N s> vs <M min> by hand. -->

## Playbooks

<!-- - `playbooks/<name>.md`: <recipe>; use when <situation>. -->
