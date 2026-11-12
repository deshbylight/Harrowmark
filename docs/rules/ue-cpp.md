# Unreal C++ rules (UE 5.8)

- Confirm unfamiliar APIs in engine headers before use; do not guess signatures.
- `UPROPERTY()` every `UObject*` member that must survive GC (or use `TObjectPtr`). Never cache raw
  `UObject*` in non-UObject structs without `TWeakObjectPtr`.
- No per-frame allocation or component lookup in Tick; cache on BeginPlay.
- New module: add it to the `.uplugin`/`.uproject` and the `Build.cs` dependencies in the same commit.
- Prefer `UE_LOG` with a per-area log category; errors that indicate a bug use `ensureMsgf`.
