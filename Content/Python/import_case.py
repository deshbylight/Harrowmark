"""Copy data/<case>.json into the case asset and point the Investigation settings at it.

Run after editing the case data (tools/case_check.py first):
    python <JamForge>/jf.py uepy Content/Python/import_case.py [case01]
Creates or updates /Game/Case/DA_<Case> (UCaseDefinition, the JSON is stored in SourceJson and parsed on
load) and writes Case=... to Config/DefaultGame.ini.
"""
import sys
from pathlib import Path

import unreal

name = (sys.argv[1] if len(sys.argv) > 1 else "case01").lower()
project = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
source = project / "data" / f"{name}.json"
asset_name = "DA_" + name[0].upper() + name[1:]
folder = "/Game/Case"
path = f"{folder}/{asset_name}"

text = source.read_text(encoding="utf-8")
case = unreal.load_asset(path)
if case is None:
    tools = unreal.AssetToolsHelpers.get_asset_tools()
    case = tools.create_asset(asset_name, folder, unreal.CaseDefinition, None)
    if case is None:
        raise RuntimeError(f"could not create {path}")
case.set_editor_property("source_json", text)
# UE Python folds a bool return into the out parameters: Reload(OutError) gives the error string on
# success (empty) and None on failure (the parse errors are in the log as LogInvestigation / LogJson).
if case.reload() is None:
    raise RuntimeError(f"{source} did not parse; see the log for the errors")
unreal.EditorAssetLibrary.save_loaded_asset(case, only_if_is_dirty=False)

# UE Python cannot save a config object, so the settings line is written to Config/DefaultGame.ini directly.
ini = project / "Config" / "DefaultGame.ini"
section = "[/Script/Investigation.InvestigationSettings]"
entry = f"Case={path}.{asset_name}"
lines = ini.read_text(encoding="utf-8").splitlines() if ini.exists() else []
if section in lines:
    start = lines.index(section) + 1
    end = next((i for i in range(start, len(lines)) if lines[i].startswith("[")), len(lines))
    lines[start:end] = [x for x in lines[start:end] if not x.startswith("Case=")]
    lines.insert(start, entry)
else:
    lines += ["", section, entry]
ini.write_text("\n".join(lines).strip() + "\n", encoding="utf-8")
print(f"JF_CASE {path} <- {source.name} ({len(text)} chars)")
