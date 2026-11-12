"""Headshot 3 Pro from a portrait, then save the .ccAvatar and export an Unreal FBX (with the
CC json for Auto Setup). Runs inside CC through cc_bridge.py; queue it with queue_job.py:

  python Tools/reallusion/queue_job.py headshot CH03 --photo art_src/Generated/Characters/CH03/portrait/CH03_portrait_2.png --body male

Output: art_src/Generated/Characters/<id>/cc/<id>.ccAvatar and <id>.fbx (+ textures, json).
Clothing, hair and hats are dressed afterwards from CC content; re-export with --export-only.
"""
import os

BODY = {"male": "EHSBodyType_Male", "female": "EHSBodyType_Female", "neutral": "EHSBodyType_Neutral"}


CC_ROOT = r"D:\Reallusion\iClone 8\Character Creator 5"
DEFAULT_PROJECT = os.path.join(CC_ROOT, "Program", "Assets", "Creator", "Default.ccProject")


def _avatar(RLPy, log=print, load_default=False):
    av = RLPy.RScene.GetAvatars(RLPy.EAvatarType_All)
    if not av and load_default:
        # Headshot crashes CC (access violation) on an empty scene: load the stock base first.
        log("empty scene, loading", DEFAULT_PROJECT)
        log("load:", RLPy.RFileIO.LoadFile(DEFAULT_PROJECT))
        av = RLPy.RScene.GetAvatars(RLPy.EAvatarType_All)
    if not av:
        raise RuntimeError("no avatar in the scene")
    return av[0]


def export_fbx(RLPy, avatar, path, log):
    opt = (RLPy.EExportFbxOptions_FbxKey | RLPy.EExportFbxOptions_AutoSkinRigidMesh
           | RLPy.EExportFbxOptions_RemoveAllUnused | RLPy.EExportFbxOptions_RemoveHiddenMesh
           | RLPy.EExportFbxOptions_ExportPbrTextureAsImageInFormatDirectory)
    opt2 = (RLPy.EExportFbxOptions2_UnrealPreset | RLPy.EExportFbxOptions2_UnrealEngine4BoneAxis
            | RLPy.EExportFbxOptions2_RenameDuplicateBoneName | RLPy.EExportFbxOptions2_RenameDuplicateMaterialName
            | RLPy.EExportFbxOptions2_RenameTransparencyWithPostFix | RLPy.EExportFbxOptions2_RenameBoneToLowerCase)
    opt3 = RLPy.EExportFbxOptions3_ExportJson
    log("fbx licence:", RLPy.RFileIO.CheckExportFbxHasLicense(avatar))
    st = RLPy.RFileIO.ExportFbxFile(avatar, path, opt, opt2, opt3,
                                    RLPy.EExportTextureSize_Size_4096, RLPy.EExportTextureFormat_Png)
    log("export fbx:", st, path, os.path.exists(path))


def run(RLPy, log, repo, cid, photo=None, body="male", export_only=False):
    out = os.path.join(repo, "art_src", "Generated", "Characters", cid, "cc")
    os.makedirs(out, exist_ok=True)
    if not export_only:
        photo = os.path.join(repo, photo) if not os.path.isabs(photo) else photo
        base = _avatar(RLPy, log, load_default=True)
        log("base avatar:", base.GetName(), "type", base.GetAvatarType() if hasattr(base, "GetAvatarType") else "?")
        RLPy.RScene.SelectObject(base) if hasattr(RLPy.RScene, "SelectObject") else None
        o = RLPy.RHeadshotOption()
        o.eBodyType = getattr(RLPy, BODY[body])
        log("headshot start:", photo)
        st = RLPy.RHeadshot.CreateHeadFromPhoto(photo, RLPy.EHSMode_Pro, o)
        log("headshot:", st)
    av = _avatar(RLPy, log)
    log("avatar:", av.GetName())
    s = RLPy.RSaveFileSetting()
    s.SetSaveType(RLPy.ESaveFileType_Character)
    log("save:", RLPy.RFileIO.SaveFile(av, s, os.path.join(out, cid + ".ccAvatar")))
    export_fbx(RLPy, av, os.path.join(out, cid + ".fbx"), log)
