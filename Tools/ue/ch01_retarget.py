"""Retarget the UE mannequin animations onto the detective CH01 (run: jf uepy Tools/ue/ch01_retarget.py).

Builds, under /Game/Characters/Detective:
  Retarget/IK_Manny            IK rig of the mannequin (auto-generated retarget definition)
  Retarget/IK_CH01             IK rig of CH01: the same chain names, mapped onto the Tripo 78-joint rig
  Retarget/RTG_Manny_to_CH01   retargeter, target pose auto-aligned (CH01 is bound in a T pose, Manny in an A pose)
  Anims/                       ABP_Unarmed and its animations duplicated and retargeted for CH01

CH01 notes: the skeleton root `BoneRoot` sits at hip height, not on the ground, and stays still;
the retarget root is `Hip`. The head bone is `Head` (FName lookups ignore case, so "head" also finds it).
Prints JF_RETARGET lines; raises on any failure. Idempotent: existing assets are replaced.
"""
import unreal

MANNY_MESH = "/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple"
SOURCE_ABP = "/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed"
CH01_MESH = "/Game/Characters/Detective/SK_CH01_Detective/SkeletalMeshes/SK_CH01_Detective"
RIG_DIR = "/Game/Characters/Detective/Retarget"
ANIM_DIR = "/Game/Characters/Detective/Anims"

# Mannequin bone -> CH01 bone. Metacarpals map to the first finger joint (CH01 has none).
BONES = {
    "pelvis": "Hip",
    "spine_01": "Waist", "spine_02": "Waist", "spine_03": "Spine01", "spine_04": "Spine02", "spine_05": "Spine02",
    "neck_01": "NeckTwist01", "neck_02": "NeckTwist02", "head": "Head",
}
for s, c in (("l", "L"), ("r", "R")):
    BONES.update({
        f"clavicle_{s}": f"{c}_Clavicle", f"upperarm_{s}": f"{c}_Upperarm", f"lowerarm_{s}": f"{c}_Forearm",
        f"hand_{s}": f"{c}_Hand", f"thigh_{s}": f"{c}_Thigh", f"calf_{s}": f"{c}_Calf", f"foot_{s}": f"{c}_Foot",
        f"ball_{s}": f"{c}_ToeBase",
        f"upperarm_twist_01_{s}": f"{c}_UpperarmTwist01", f"upperarm_twist_02_{s}": f"{c}_UpperarmTwist02",
        f"lowerarm_twist_01_{s}": f"{c}_ForearmTwist01", f"lowerarm_twist_02_{s}": f"{c}_ForearmTwist02",
        f"thigh_twist_01_{s}": f"{c}_ThighTwist01", f"thigh_twist_02_{s}": f"{c}_ThighTwist02",
        f"calf_twist_01_{s}": f"{c}_CalfTwist01", f"calf_twist_02_{s}": f"{c}_CalfTwist02",
    })
    for mf, cf in (("thumb", "Thumb"), ("index", "Index"), ("middle", "Mid"), ("ring", "Ring"), ("pinky", "Pinky")):
        BONES[f"{mf}_metacarpal_{s}"] = f"{c}_{cf}1"
        for k in (1, 2, 3):
            BONES[f"{mf}_0{k}_{s}"] = f"{c}_{cf}{k}"


def log(msg):
    print("JF_RETARGET " + msg)


def fresh(name, path, cls, factory):
    full = f"{path}/{name}"
    if unreal.EditorAssetLibrary.does_asset_exist(full):
        unreal.EditorAssetLibrary.delete_asset(full)
    asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(name, path, cls, factory)
    if asset is None:
        raise RuntimeError(f"could not create {full}")
    return asset


def chains(ctl):
    out = []
    for ch in ctl.get_retarget_chains():
        bone = lambda key: str(ch.get_editor_property(key).get_editor_property("bone_name"))
        out.append((str(ch.get_editor_property("chain_name")), bone("start_bone"), bone("end_bone"),
                    str(ch.get_editor_property("ik_goal_name"))))
    return out


def main():
    manny = unreal.load_asset(MANNY_MESH)
    ch01 = unreal.load_asset(CH01_MESH)
    if not manny or not ch01:
        raise RuntimeError("source or target mesh missing")
    for d in (ANIM_DIR, RIG_DIR):  # outputs reference each other: drop both folders, then rebuild
        if unreal.EditorAssetLibrary.does_directory_exist(d):
            unreal.EditorAssetLibrary.delete_directory(d)
    unreal.EditorAssetLibrary.make_directory(RIG_DIR)
    unreal.EditorAssetLibrary.make_directory(ANIM_DIR)

    ik_manny = fresh("IK_Manny", RIG_DIR, unreal.IKRigDefinition, unreal.IKRigDefinitionFactory())
    cm = unreal.IKRigController.get_controller(ik_manny)
    cm.set_skeletal_mesh(manny)
    if not cm.apply_auto_generated_retarget_definition():
        raise RuntimeError("mannequin not recognised by the auto retarget definition")
    cm.apply_auto_fbik()
    src = chains(cm)
    log(f"IK_Manny root={cm.get_retarget_root()} chains={src}")

    ik_ch = fresh("IK_CH01", RIG_DIR, unreal.IKRigDefinition, unreal.IKRigDefinitionFactory())
    cc = unreal.IKRigController.get_controller(ik_ch)
    cc.set_skeletal_mesh(ch01)
    cc.set_retarget_root("Hip")
    skipped = []
    for name, start, end, _goal in src:
        a, b = BONES.get(start), BONES.get(end)
        if not a or not b:
            skipped.append(name)
            continue
        cc.add_retarget_chain(name, a, b, "None")
    cc.apply_auto_fbik()
    log(f"IK_CH01 root={cc.get_retarget_root()} chains={chains(cc)} skipped={skipped}")

    rtg = fresh("RTG_Manny_to_CH01", RIG_DIR, unreal.IKRetargeter, unreal.IKRetargetFactory())
    rc = unreal.IKRetargeterController.get_controller(rtg)
    S, T = unreal.RetargetSourceOrTarget.SOURCE, unreal.RetargetSourceOrTarget.TARGET
    rc.set_ik_rig(S, ik_manny)
    rc.set_ik_rig(T, ik_ch)
    rc.set_preview_mesh(S, manny)
    rc.set_preview_mesh(T, ch01)
    rc.remove_all_ops()
    rc.add_default_ops()
    rc.assign_ik_rig_to_all_ops(S, ik_manny)
    rc.assign_ik_rig_to_all_ops(T, ik_ch)
    rc.auto_map_chains(unreal.AutoMapChainType.EXACT, True)
    rc.auto_align_all_bones(T, unreal.RetargetAutoAlignMethod.CHAIN_TO_CHAIN)
    rc.snap_bone_to_ground("L_ToeBase", T)
    ops = [str(rc.get_op_name(i)) for i in range(rc.get_num_retarget_ops())]
    log(f"RTG ops={ops} pose={rc.get_current_retarget_pose_name(T)}")
    for p in (ik_manny, ik_ch, rtg):
        unreal.EditorAssetLibrary.save_loaded_asset(p, False)

    abp = unreal.EditorAssetLibrary.find_asset_data(SOURCE_ABP)
    out = unreal.IKRetargetBatchOperation.duplicate_and_retarget(
        [abp], manny, ch01, rtg, "", "", "", "_CH01", ANIM_DIR, False, True, True)
    made = [str(d.package_name) for d in out]
    log(f"retargeted {len(made)} assets: {made}")
    if not made:
        raise RuntimeError("batch retarget produced nothing")
    # The batch compiles the duplicated AnimBP but neither returns nor saves it.
    abp_out = f"{ANIM_DIR}/ABP_Unarmed_CH01"
    if not unreal.EditorAssetLibrary.does_asset_exist(abp_out):
        raise RuntimeError(f"{abp_out} was not created")
    made.append(abp_out)
    for p in made:
        a = unreal.load_asset(p)
        if isinstance(a, unreal.AnimBlueprint):
            unreal.BlueprintEditorLibrary.compile_blueprint(a)
            log(f"ABP {p} skeleton={a.get_editor_property('target_skeleton').get_path_name()}")
        unreal.EditorAssetLibrary.save_loaded_asset(a, False)


main()
