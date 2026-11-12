# Learned prompt overlays

- `common.md`: appended to the instructions of every agent.
- `<role>.md` (for example `builder.md`): appended to the instructions of that role.

Overlays change only through A/B experiments in `forge/experiments/<slug>/`: the optimizer writes the
treatment there, the orchestrator alternates runs between the current overlay (A) and the treatment
(B), and the verdict decides what lands here. Each overlay stays under 2500 words
(`scripts/tests/test_forge_layout.py` checks it).
