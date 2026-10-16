# Changelog

Diablo 2 Couch releases (prereleases while the couch co-op is a preview). The launcher
shows a version's section once, on the first start after an update.

## v0.0.5 (2026-10-17)

- Linux: the tarball now runs on any Linux from the last years (glibc 2.28 or newer:
  Linux Mint, Ubuntu, Debian, Fedora, Arch, SteamOS) instead of only on the newest
  distributions. Unpack it and start `d2d-launcher`.

## v0.0.4 (2026-10-16)

- Act III together: quest items a guest picks up (Khalim's eye, heart and brain, the jade
  figurine, the Gidbinn, Lam Esen's tome, and in earlier acts the Cube and the staff
  pieces) go into player 1's pack, who talks to the NPCs and uses the Cube — so no quest
  gets stuck in a guest's pockets.
- Quest objects a guest uses (Khalim's chests, Lam Esen's stand, the sewer lever, the
  Gidbinn's altar, the compelling orb, the Arcane Sanctuary's portal) work as player 1's.
- Mephisto's death and the act reached count for every guest's save, like Andariel and
  Duriel.

## v0.0.3 (2026-10-16)

- Act II together: the Arcane Sanctuary's teleport pads take the whole party along
  instead of the camera pulling the player back.
- Guests keep what the party achieved in their own saves: Andariel and Duriel beaten,
  Acts II and III opened, the waypoints found together, and the act the party is in —
  so next time their hero starts in Lut Gholein, alone or as player 1.

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
