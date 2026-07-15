 ---
  Starfall

  A top-down bullet-hell shoot-'em-up built on the axmol engine (v2.11.4), C++20. Waves of enemies converge on the player from every edge; you thin them out
  before they reach you.

  ▎ Status: work in progress. Starfall is a from-scratch gameplay project — the focus is engine-level systems (pooling, collision, event flow) written to be
  ▎ read, not just to run. Expect TODOs at the edges; the core loop is playable.

  Build

  Requires axmol 2.11.4 with AX_ROOT set. From game/Starfall: axmol build -p win32 then axmol run -p win32. Standard CMake underneath; build_x64/ and .vs/
  are git-ignored.

  Architecture (the interesting part)

  - Pooled entities — fixed-capacity, allocated once; acquire() returns null at cap, never grows. No mid-frame allocation.
  - Spatial-grid broadphase — rebuilt per frame; bullets test only enemies in cells along their path.
  - Swept collision — bullets test a segment vs enemy circles, so fast bullets can't tunnel.
  - Deferred removal — cull lists released at frame end; the active set is never mutated mid-iteration.
  - Closed-set event queues — POD events in per-type vectors, deliberately not an observer bus — and it says why (no vcall/alloc per event in a closed
  system).
  - Component-based entities — Unity-style GameObject + components.
  - Trauma-based screen shake — squared-magnitude decay for punchy, tapering feedback.
