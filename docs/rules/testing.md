# Testing rules

- Logic: `IMPLEMENT_SIMPLE_AUTOMATION_TEST(F<Area><Name>Test, "Project.<Area>.<Name>",
  EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)`; run with
  `jf test --filter Project.<Area>`.
- Visual/gameplay: `jf shot`, then Read the PNGs. A change is not verified until you saw it.
- "Built" is not "works". The `jf done` summary states which checks ran and what they showed.
- Never disable or weaken a test to get green. Fix the code, or file a task with the evidence.
