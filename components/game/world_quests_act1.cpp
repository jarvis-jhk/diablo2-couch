// SPDX-License-Identifier: GPL-3.0-or-later
// Act 1's quests at the World's quest hooks (world.hpp: World::actN_*).
// A new act adds its own file like this one and a call at each hook.
#include "world.hpp"
#include "fight.hpp"
#include "gamedata.hpp"
#include "log.hpp"
#include <d2s_items.hpp>
#include <level_ids.hpp>
#include <monster_ids.hpp>
#include <quests.hpp>
#include <algorithm>
#include <array>
#include <cstdint>
#include <format>
#include <iterator>
#include <string>
#include <vector>

namespace d2d::game {

auto World::act1_log(std::array<std::uint8_t, 15>& log, const d2d::rules::QuestBits& bits) const -> void {
        const int clvl = int(character.stats.get(d2d::d2s::kLevel));
        log = { 0, std::uint8_t(den.log_state(bits)), std::uint8_t(burial.log_state(bits)),
                std::uint8_t(tools.log_state(bits, holding_malus(), clvl)), std::uint8_t(cain.log_state(bits)),
                std::uint8_t(tower.log_state(bits)), std::uint8_t(andy.log_state(bits)) };
    }

auto World::act1_join() -> void {
        den = {};
        den.join(quests());
        andy = {};
        andy.join(quests());
        burial = {};
        burial.join(quests());
        tower = {};
        tower.join(quests());
        tools = {};
        tools.join(quests(), carries("hdm"));
        cain = {};
        cain.join(quests(), carries("bks"), carries("bkd"));
        chain(1);
        cain.camp_spawn();
    }

auto World::act1_enter(const Level& from) -> void {
        andy.enter(quests(), from.id, level->id);
        burial.enter(quests(), from.id, level->id);
        tower.enter(quests(), from.id, level->id);
        tools.enter(quests(), from.id);
        cain.enter(quests(), from.id, level->id);
    }

auto World::act1_talk(std::vector<d2d::rules::QuestMsg>& out, int hc_idx) -> void {
        std::ranges::copy(den.talk(quests(), hc_idx), std::back_inserter(out));
        std::ranges::copy(andy.talk(quests(), hc_idx), std::back_inserter(out));
        std::ranges::copy(burial.talk(quests(), hc_idx), std::back_inserter(out));
        std::ranges::copy(tower.talk(quests(), hc_idx), std::back_inserter(out));
        std::ranges::copy(tools.talk(quests(), hc_idx, holding_malus(), int(character.stats.get(d2d::d2s::kLevel))), std::back_inserter(out));
        std::ranges::copy(cain.talk(quests(), hc_idx, carries("bks")), std::back_inserter(out));
    }

auto World::act1_talk_closed(int hc_idx) -> void {
        andy.talk_closed(hc_idx);
        burial.talk_closed(quests(), hc_idx);
        tools.talk_closed(quests(), hc_idx);
        cain.talk_closed(quests(), hc_idx);
    }

auto World::act1_said(int hc_idx, int message_string) -> void {
        if (tower.said(quests(), hc_idx, message_string)) { d2d::log::info("The Forgotten Tower: done"); chain(5); }   // FUN_00594960
        if (andy.said(quests(), hc_idx, message_string)) {
            d2d::log::info("Sisters to the Slaughter: done, Warriv's caravan goes east");
        }
        const int tools_was = tools.state;
        if (tools.said(quests(), hc_idx, message_string, holding_malus())) {   // FUN_00544160: the malus goes
            if (tools_was == 4 && tools.state == 5) chain(3);   // FUN_00591490: its own +0xf0
            else if (tools.active && tools.carried_in) andy.chain();   // data+0xa1: quest 6's +0xf0
            take_carried("hdm");
            d2d::log::info("Tools of the Trade: the malus is back, Charsi's imbue is due");
        }
        using Said = d2d::rules::CainQuest::Said;
        const auto scroll = std::ranges::find(character.items, std::string_view("bks"), &d2d::d2s::Item::code);
        const bool cain_flagged = cain.rescued_flag;
        const auto cain_said = cain.said(quests(), hc_idx, message_string, scroll != character.items.end());
        if (cain_said == Said::ring && !cain_flagged && d2d::rules::qbit(quests(), 4, 13)) chain(4);   // FUN_00592250: the game's (4, 13) goes on
        if (cain_said == Said::decipher) {        // ponytail: bks becomes bkd where it lies (same size), not a new item (agent A's)
            scroll->code = "bkd";
            d2d::log::info("Search for Cain: Akara deciphered the scroll");
        }
        if (cain_said == Said::ring) {            // FUN_005466b0: into the inventory, else at the feet
            static constexpr int kIlvl[3] = { 7, 30, 60 };
            const int diff = std::clamp(character.header.active_difficulty(), 0, 2);
            loot.put({ .code = "rin", .quality = diff ? 6 : 4 }, player.x, player.y, kIlvl[diff], fight.spawning.game, now);
            loot.take(loot.ground.size() - 1);
            cues.cue("item_ring", now, player.x, player.y);   // FUN_005458e0(4): S->C 0x5d flags 2, the client's item_ring (FUN_004a2cb0)
            d2d::log::info("Search for Cain: done, Akara's ring");
        }
        const int den_was = den.state, burial_was = burial.state;
        if (den.said(quests(), hc_idx, message_string)) {
            ++character.stats.values[d2d::d2s::kSkillPts];
            d2d::log::info("Den of Evil: Akara's reward, a skill point");
            if (den_was != 5 && den.state == 5) chain(1);   // FUN_0058fdd0
        }
        if (burial.said(quests(), hc_idx, message_string)) {
            kashya_merc();
            if (burial_was != 5 && burial.state == 5) chain(2);   // FUN_00590980
        }
    }

auto World::act1_tick() -> void {
        if (andy.tick() && level->id == d2d::rules::level_ids::kCatacombsLevel4) fight.portal_due = true;   // tick 10 of her death (FUN_00596490)
        burial.tick();
        tower.tick();
        tower_treasure(day_at);
    }

auto World::act1_alert(int hc_idx) -> bool {
        return den.alert(quests(), hc_idx)
            || andy.alert(quests(), hc_idx)
            || burial.alert(quests(), hc_idx)
            || tower.alert(quests(), hc_idx)
            || tools.alert(quests(), hc_idx, holding_malus(), int(character.stats.get(d2d::d2s::kLevel)))
            || cain.alert(quests(), hc_idx, carries("bks"));
    }

auto World::act1_kill(const Fight::Kill& kill, std::uint32_t now_ms) -> void {
        if (kill.type == d2d::rules::monster_ids::kAndariel) andariel_died(kill, now_ms);   // a pet's kill too, the player dead
        if (kill.type == d2d::rules::monster_ids::kBloodRaven) blood_raven_died(now_ms);
        if (kill.super == d2d::rules::TowerQuest::kCountess) countess_died(now_ms);
    }

}  // namespace d2d::game
