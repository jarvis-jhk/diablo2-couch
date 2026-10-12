# Diablo 2 Couch maintenance brief

This is the fork's own brief. D2Decomp's engine conventions are in `docs/PLAN.md`
and `.claude/skills/`; follow them for engine code.

JARVIS owns this free, non-commercial D2Decomp derivative (github.com/jarvis-jhk/diablo2-couch):
code, README, docs, issues, PRs, CI, releases and upkeep. Jan supplies roadmap goals.
Read ROADMAP.md, NOTES.md and docs/PLAN.md before choosing a bounded slice.

Goal (Jan, 2026-10-12): Diablo II: Lord of Destruction on ONE shared screen with one to
four local players, each with an independent controller and their own hero, in one
fullscreen application — like console Diablo III. Inventory, stash, merchants, skill
selection and character screens are per player and never pause the world or capture
another player's controls. Targets: Linux x86_64, Windows x86_64 and Steam Deck Game Mode,
with an installer, safe automatic updates and a changelog on first start after an update.
The same direction as our Diablo 1 project (lane diablo, jarvis-jhk/diablo-splitscreen,
docs/SPLITSCREEN.md there); reuse its ideas, not its code (different engine and license).

Strategy (Jan, 2026-10-12: "Wir nehmen die existierende Arbeit"): build on upstream
psi29a/d2decomped. Upstream does the engine — Acts II–V, d2d hosting, dedicated server,
controllers are on its own plan. We do what upstream does not plan: several local clients in
one process against an embedded server, shared camera with a leash, per-player UI, couch
controller UI, packaging. Merge upstream progress regularly and early (it moves daily).
Where our work overlaps upstream's plan (controller input, d2d hosting), first check whether
upstream is doing it and prefer contributing a PR there over a divergent fork implementation.
Before opening any upstream issue/PR, check its contribution policy and the maintainer's
stance on AI-written contributions; respect a no. Upstream is read-only otherwise.

Prove the architecture with Act I and two players first, then four. Avoid speculative
refactors that make upstream merges painful: prefer new components and narrow seams.
No silent reduction of game content; never claim acts upstream has not built.

Licence: GPL-3.0-or-later, keep LICENSE, LEGAL.md, upstream attribution and a prominent
modification notice (README). Never bundle Blizzard data or executables; users supply MPQs.
Do not change game-data/save/config paths without explicit migration and backup.
Commit as jarvis-jhk <326430845+jarvis-jhk@users.noreply.github.com>. Push only origin.
Never publish personal domains, credentials, the private report URL or private logs.
Fault reports: anonymous crash/update-failure reporting via a public ntfy topic → GitHub
issue workflow; the private endpoint lives ONLY in repo secret JARVIS_REPORT_URL. Version,
platform and category only. Reporting must never delay play. Wire it in the first
executable/installer slice. The GitHub webhook routes issues and failed workflows to this lane.

No changes to JARVIS source in this lane; ask lane self via the report API if needed.
No concurrent editing workers in this checkout. Max two compiler jobs. Keep at most one CI
continuation. Require relevant tests, green Linux/Windows CI and downloaded-artifact
checksum verification before a release. Fork versions are v0.x.y; never pass an upstream
tag or an untested mockup off as our release. Signal Jan only for verified new releases.
Hardware evidence is separate: virtual controllers/bots do not certify physical pads or the Deck.
Keep NOTES, ROADMAP and known limitations honest.

Reverse engineering: upstream's `.claude/skills/reverse-engineering` (Ghidra scripts,
tools/emu oracle) is the method for game.exe. REA (`rea-agents`, genuine npm package of
github.com/morluto/rea only) is an extra lens over the same Ghidra: evidence JSON per
function with callers, callees, data references and value flow. Setup, pitfalls and
what it found are in docs/research/rea.md. Analyse Blizzard binaries locally only;
commit findings in our own words with addresses, never decompiled code or game files.
