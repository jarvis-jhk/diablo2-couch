# Changelog

Diablo 2 Couch releases (prereleases while the couch co-op is a preview). The launcher
shows a version's section once, on the first start after an update.

## v0.0.2 (2026-10-16)

- Up to four heroes on one screen: the keyboard player and three gamepads, or four
  gamepads with `pad_player1`.
- The launcher tells you when a newer Diablo 2 Couch release is out (not in installs
  that Warpzone manages) and shows this list once after an update.
- `d2d --data <dir>` is remembered, so later starts find the game data without it.
- Installed through the Warpzone launcher, the game uses the Diablo II folder the
  launcher found (`WARPZONE_DATA_D2LOD`) without asking.
- Anonymous crash reports: if the game did not close normally, the next start sends
  one line (version, platform, "crash") so the bug can be fixed. Nothing else is sent.
  Turn it off with `fault_reports = 0` in d2d.cfg or `D2COUCH_NO_REPORT=1`.

## v0.0.1 (2026-10-16)

- First couch co-op preview: a second gamepad joins the game with its own saved hero,
  one shared camera keeps everyone on screen, Act I with upstream's Acts II and III.
