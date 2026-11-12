# Architecture rules

- One plugin per feature area: `Plugins/Features/<Area>/Source/<Area>/{Public,Private}`. The area
  name equals the task area on the board.
- `Plugins/Core/JamCore`: interfaces (`UINTERFACE`), gameplay tags, shared structs, subsystems that
  route between features. Additive changes only: add a file instead of editing a shared one.
- Feature-to-feature calls go through JamCore interfaces, `UGameInstanceSubsystem`/`UWorldSubsystem`
  lookups or multicast delegates. Never include another feature's Private headers. Never add a
  feature-to-feature module dependency.
- The game module (`Source/HarrowMark`) only wires things together: game mode, default classes, and
  startup.
- Tunables are data (DataAssets, `EditDefaultsOnly` properties, config) so balance changes need no
  code.
