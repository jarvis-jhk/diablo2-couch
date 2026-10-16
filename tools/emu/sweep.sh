#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-or-later
# Every level of an act ($ACT: 1, the default, to 5; towns left out) against
# game.exe, a job per core but two ($JOBS): sweep.sh [seeds] [kind...]
# e.g. ACT=2 sweep.sh 1-50 monsters objgroups   (kinds: monsters, units, tiles, seeds, objgroups, drops, collision)
# Our side runs from a Release build (build-release, configured and brought
# up to date here; same output as Debug, ~2x faster). Progress goes to
# stderr as each check finishes; the sorted results to stdout at the end.
cd "$(dirname "$0")"
root=../..
[ -f "$root/build-release/build.ninja" ] || cmake -S "$root" -B "$root/build-release" -G Ninja -DCMAKE_BUILD_TYPE=Release \
    -DD2_PATCH_INSTALLER="${D2_PATCH_INSTALLER:-$HOME/Downloads/Diablo II + LoD/patch/LODPatch_114d.exe}" >/dev/null || exit 1
cmake --build "$root/build-release" --target drlg-dump mpq-cat >/dev/null || { echo "build-release failed" >&2; exit 1; }
export D2_BUILD=build-release
uv sync -q || exit 1   # once, before the jobs: parallel first-time syncs race
seeds=${1:-1-50}; shift; kinds=${*:-monsters objgroups}
act=${ACT:-1}; case $act in [1-5]) ;; *) echo "ACT is 1 to 5" >&2; exit 1 ;; esac
towns="1 40 75 103 109 137"   # each act's town (drlg.TOWNS), then one past Act 5's last level
first=$(( $(echo $towns | cut -d' ' -f"$act") + 1 )); last=$(( $(echo $towns | cut -d' ' -f$((act + 1))) - 1 ))
total=$(( $(echo $kinds | wc -w) * (last - first + 1) )); count=$(echo "$seeds" | awk -F- '{print $2 - $1 + 1}')
start=$(date +%s)
# A job per level, all kinds in it: game.exe loads once per level; the
# later levels first (the slow ones), so none starts last.
seq $last -1 $first |
    xargs -P "${JOBS:-$(( $(sysctl -n hw.ncpu 2>/dev/null || nproc) - 2 ))}" -n 1 sh -c 'uv run python diff_drlg.py '"$seeds"' $0 '"$kinds"' 2>&1 | grep "seeds match"' |
    while read -r line; do
        finished=$((${finished:-0} + 1)); now=$(date +%s); spent=$((now - start))
        case $line in *" $count/$count seeds match"*) mark=ok ;; *) mark=FAIL ;; esac
        printf '[%d/%d %dm%02ds eta %dm%02ds] %s %s\n' "$finished" "$total" $((spent / 60)) $((spent % 60)) \
            $(( spent * (total - finished) / finished / 60 )) $(( spent * (total - finished) / finished % 60 )) "$mark" "$line" >&2
        echo "$line"
    done | sort -k3 -n
