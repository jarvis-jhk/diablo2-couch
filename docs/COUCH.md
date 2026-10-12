# Couch co-op: where d2d stands

Survey for ROADMAP M0, written 2026-10-12 at upstream 625fd8f (fork head 7f2a9db).
Line numbers drift with upstream merges; the names don't.

Goal: one to four local players, each with their own controller and hero, on ONE
shared screen in one process, against one game. Panels are per player and never
pause the world.

## Client and server today

d2d already has D2's shape: single player is a client talking to an
authoritative server in the same process (docs/design/multiplayer.md, "Decision").

- `World` (components/game/world.hpp) is the server: 25 Hz tick
  (`kTickMs`), `World::tick(commands, ...)`.
- Clients send `Command`s (protocol.hpp), D2-shaped intents with a byte codec,
  through a `LocalTransport` (protocol.hpp) — one queue in, one buffer out, so
  one client per transport.
- The server answers with a one-player `View` (world.hpp), sent as deltas by a
  per-client `ViewEncoder` (replication.hpp). The client draws from the View only.
- But the server lives inside the client: `Town` (apps/d2d/town.hpp) owns
  `World world`, `LocalTransport net`, `View view`, `ViewEncoder view_enc`,
  and `Town::walk` steps the world (`world.tick(net.receive(), …)`) and
  publishes the view. `main.cpp` has exactly one `Town`.
- `--join` (netgame.*, components/net) is a second client path that talks
  D2GS to a game.exe host. d2d cannot host yet.

## What is single-player

**Server.** `World` holds one `character`, `level`, `player`, merc, target,
`held` item, `store`, `talking`, portals and corpses; commands carry no player
id; `view()` culls around the one player. `Fight` keeps references to that one
player and holds its combat state (curses, chill, stun, pmode, …). Monster
targeting (`Foe`, nearest live foe) is already N-ready. No `PlayerSession`,
`GameSession` or `max_players` exists yet.

Upstream has planned exactly this split: docs/design/net-host.md §1 lists every
blocker, §3 stage 1 is "N-player World, still in process … two local players
driven by devctl, rendering player 1 only", §5 item 1 sizes it at 1.5–2 weeks
(it touches most of world.cpp and fight.cpp). PLAN.md defers it until after
Act 2. That stage is also our M1 server.

**Client.** Everything per player is a plain member of the one `Town` — no
singletons:

| State | Where |
|---|---|
| Mouse | one `Mouse` (ui.hpp) filled by `handle_sdl_events` (window.cpp) |
| Keys | `keys_this_frame` (main.cpp) into `Town::update`; hotkeys I/T/R/C/Q/Tab/`/1-4/Esc |
| Modifiers | direct `SDL_GetModState()` in town.cpp (belt Shift, Alt) and skillbar.hpp |
| Held item | `Town::held` (+ identify/imbue cursors); server `World::held` |
| Panels | `Town` bools: stash, skill tree, quest log, belt, cube, inventory, character |
| NPC UI | `npc_menu`, `store`, `waypoint`, `speech`, `hire_offers` |
| Menus | `game_menu`, `mini` |
| Skills / F-keys | `SkillBar skillbar` |
| Automap | `automap`, `other_automaps`, saved per character |
| Camera | `cam_x/y`, `prev_x/y`, set once in `Town::draw` from the player |
| Pause | `if (!net_game && game_menu.open) world_ms = frame_ms;` in `Town::walk` |

Process-wide: `Audio`, the screen state machine, `scene->palette_act` (swapped
to the player's act), a few toggles (`g_autoloot`, `g_roof_cutout`, …), the map
seed (fine: one game per process).

**Rendering.** The camera is the player: `render_ingame` draws the local hero at
the camera, NPCs are culled around it, the light grid and lamp and the roof
cut-out centre on it. The world-to-screen projection (`render_world`,
world_view.cpp) is cleanly parameterised at a fixed 800×600; it keeps a
function-local static mask (fine for one shared view). Two more hidden
function-local statics carry state between frames (found with REA, see
docs/research/rea.md): `fades` in `render_world` (wall see-through, centred on ONE
player's cell, keyed by level pointer) and `eases` in `frame_light` (town.cpp, light
radius easing per light). With a shared screen the wall cut-out must consider every
local hero; with any second view in the process both maps would be shared silently. `render_ingame` takes one
player's inventory/HUD and draws panels at fixed screen positions.

**Controllers.** None. `SDL_Init(SDL_INIT_VIDEO)` only; no gamepad code. Upstream
lists controllers under "Cross-platform polish" (PLAN.md step 7), not started.

## The seams, narrowest first

1. **Server: upstream's PlayerSession split, in upstream's shape.** Tagged
   commands (`tick(span<Tagged{player_id, Command}>)`), per-player Fight state,
   `create_game` vs `join(player)`, a View per player. Building this in the fork
   alone would conflict with nearly every upstream commit to world.cpp/fight.cpp,
   so it should go upstream (see "Upstream" below) or wait for them.
2. **Transport: additive.** One `LocalTransport` + `ViewEncoder` per local player.
3. **Client: host + N clients.** Lift `World` and the tick loop out of `Town` into a
   host owned by main.cpp; each local player is a slimmed `Town` keeping its view,
   panels, skillbar, cursor, automap. `Town` is "the client" by design already. The
   devctl verbs and `Town`'s reference aliases into `world` move to the host.
4. **Input: one seam.** `Town::update(const Mouse&, keys)` takes a per-player input
   snapshot instead; a gamepad drives a virtual `Mouse` plus direct-stick movement
   commands. The `SDL_GetModState()` reads become per-player modifiers. SDL3
   gamepad events are routed per device next to `handle_sdl_events`.
5. **Camera: one place.** Replace the camera line in `Town::draw` with a shared
   group camera (centroid + leash); draw every hero at its own position, not at the
   camera; light and roof cut-out per hero.
6. **Panels.** Give panel drawing a screen origin (and later a scale) so each player's
   panels sit in their own corner/quadrant; held items and NPC dialogs per player.
7. **Pause.** The menu pause applies only with one local player.

Different acts for different players fight over `palette_act`; group rules
(M1 "waypoints, portals, area transitions") keep the party in one act, which
sidesteps that.

## Upstream

- psi29a/d2decomped has no CONTRIBUTING, no PR/issue template, no code of conduct,
  issues enabled, no discussions. All 19 PRs and 770 commits are the maintainer's
  own; no outside contribution has ever been made. 3 stars, 1 fork (ours).
- AI stance: no written policy. The maintainer works with Claude himself
  (.claude/skills, docs written for "one focused developer (or agent)") and
  CodeRabbit reviews, so AI-written code is not foreign there — but that is not
  the same as inviting it. Treat it as unknown.
- Contact plan: before any PR, one short issue introducing the fork (couch co-op,
  GPL, attribution kept), stating plainly that our work is AI-written, and asking
  (a) whether outside PRs are welcome at all and (b) whether he would take the
  PlayerSession split (net-host.md item 1) and SDL3 gamepad input from us, or
  prefers to build them himself. Open it when we have something concrete to offer,
  not before. A "no" means we keep merging upstream and carry our own seams.

## Testing without hardware

`--headless` plus the devctl control channel (devctl_verbs.hpp) already drive the
game in scripted tests; tests that need MPQs print SKIP on CI. Multi-player tests
can drive several players through devctl. None of this certifies a physical pad
or the Deck.
