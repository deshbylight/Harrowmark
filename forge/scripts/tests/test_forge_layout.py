"""Layout contract of forge/ (run by the merge gate: `python -m unittest discover -s tests` in forge/scripts).

Every script is indexed, tested and answers --help; every playbook is indexed; overlays stay small.
"""
import subprocess
import sys
import unittest
from pathlib import Path

FORGE = Path(__file__).resolve().parents[2]
SCRIPTS = FORGE / "scripts"
OVERLAY_MAX_WORDS = 2500


def _readme() -> str:
    p = FORGE / "README.md"
    return p.read_text(encoding="utf-8") if p.is_file() else ""


def _scripts() -> list[Path]:
    return sorted(p for p in SCRIPTS.glob("*.py") if not p.name.startswith("_"))


class ForgeLayout(unittest.TestCase):
    def test_scripts_indexed_and_tested(self):
        readme = _readme()
        for s in _scripts():
            with self.subTest(script=s.name):
                self.assertIn(f"scripts/{s.name}", readme, "add a line for it to forge/README.md")
                self.assertTrue((SCRIPTS / "tests" / f"test_{s.stem}.py").is_file(),
                                f"missing forge/scripts/tests/test_{s.stem}.py")

    def test_scripts_answer_help(self):
        for s in _scripts():
            with self.subTest(script=s.name):
                p = subprocess.run([sys.executable, str(s), "--help"], capture_output=True, text=True,
                                   encoding="utf-8", errors="replace", timeout=30, stdin=subprocess.DEVNULL)
                self.assertEqual(p.returncode, 0, (p.stdout + p.stderr)[-1500:])

    def test_playbooks_indexed(self):
        readme = _readme()
        for p in sorted((FORGE / "playbooks").glob("*.md")):
            if p.name == "README.md":
                continue
            with self.subTest(playbook=p.name):
                self.assertIn(f"playbooks/{p.name}", readme, "add a line for it to forge/README.md")

    def test_overlays_within_budget(self):
        files = [p for p in (FORGE / "roles").glob("*.md") if p.name != "README.md"]
        files += list((FORGE / "experiments").glob("*/roles/*.md"))
        for p in sorted(files):
            with self.subTest(overlay=p.relative_to(FORGE).as_posix()):
                words = len(p.read_text(encoding="utf-8", errors="replace").split())
                self.assertLessEqual(words, OVERLAY_MAX_WORDS)


if __name__ == "__main__":
    unittest.main()
