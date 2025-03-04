/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */


#include "Management/ItemInterface.h"
#include "Management/Arenas.h"
#include "Management/ArenaTeam.h"
#include "Management/WorldStates.h"
#include "Map/Management/MapMgr.hpp"
#include "Spell/SpellAuras.h"
#include "Objects/GameObject.h"
#include "Management/ObjectMgr.h"
#include "Server/Script/ScriptMgr.h"
#include "Spell/SpellMgr.hpp"

const uint32 ARENA_PREPARATION = 32727;

const uint32 GREEN_TEAM = 0;
const uint32 GOLD_TEAM = 1;

Arena::Arena(WorldMap* mgr, uint32 id, uint32 lgroup, uint32 t, uint32 players_per_side) : CBattleground(mgr, id, lgroup, t)
{

    for (uint8 i = 0; i < 2; i++)
    {
        m_players[i].clear();
        m_pendPlayers[i].clear();
    }

    m_pvpData.clear();
    m_resurrectMap.clear();

    m_started = false;
    m_playerCountPerTeam = players_per_side;
    m_buffs[0] = m_buffs[1] = NULL;
    m_playersCount[0] = m_playersCount[1] = 0;
    m_teams[0] = m_teams[1] = NULL;

    switch (t)
    {
        case BATTLEGROUND_ARENA_5V5:
            m_arenateamtype = 2;
            break;
        case BATTLEGROUND_ARENA_3V3:
            m_arenateamtype = 1;
            break;
        case BATTLEGROUND_ARENA_2V2:
            m_arenateamtype = 0;
            break;
        default:
            m_arenateamtype = 0;
            break;
    }
    rated_match = false;

    switch (m_mapMgr->getBaseMap()->getMapId())
    {
        case 559:
            m_zoneid = 3698;
            break;
        case 562:
            m_zoneid = 3702;
            break;
        case 572:
            m_zoneid = 3968;
            break;
        case 617:
            m_zoneid = 4378;
            break;
        case 618:
            m_zoneid = 4408;
            break;
        default:
            break;

    }
}

Arena::~Arena()
{
    for (uint8 i = 0; i < 2; ++i)
    {
        // buffs may not be spawned, so delete them if they're not
        if (m_buffs[i] && m_buffs[i]->IsInWorld() == false)
            delete m_buffs[i];
    }

    for (std::set<GameObject*>::iterator itr = m_gates.begin(); itr != m_gates.end(); ++itr)
    {
        if ((*itr) != NULL)
        {
            if (!(*itr)->IsInWorld())
                delete(*itr);
        }
    }

}

// \todo Rewrite this function entirely
bool Arena::HandleFinishBattlegroundRewardCalculation(PlayerTeam winningTeam)
{
    // update arena team stats
    if (rated_match)
    {
        m_deltaRating[0] = m_deltaRating[1] = 0;
        for (uint8 i = 0; i < 2; ++i)
        {
            uint8 j = i ? 0 : 1; // opposing side
            bool outcome;

            if (m_teams[i] == NULL || m_teams[j] == NULL)
                continue;

            outcome = (i == winningTeam);
            if (outcome)
            {
                m_teams[i]->m_stats.won_season++;
                m_teams[i]->m_stats.won_week++;
            }

            m_deltaRating[i] = CalcDeltaRating(m_teams[i]->m_stats.rating, m_teams[j]->m_stats.rating, outcome);
            m_teams[i]->m_stats.rating += m_deltaRating[i];
            if (static_cast<int32>(m_teams[i]->m_stats.rating) < 0) m_teams[i]->m_stats.rating = 0;

            for (std::set<uint32>::iterator itr = m_players2[i].begin(); itr != m_players2[i].end(); ++itr)
            {
                CachedCharacterInfo* info = sObjectMgr.GetPlayerInfo(*itr);
                if (info)
                {
                    ArenaTeamMember* tp = m_teams[i]->GetMember(info);

                    if (tp != NULL)
                    {
                        tp->PersonalRating += CalcDeltaRating(tp->PersonalRating, m_teams[j]->m_stats.rating, outcome);
                        if (static_cast<int32>(tp->PersonalRating) < 0)
                            tp->PersonalRating = 0;

                        if (outcome)
                        {
                            ++(tp->Won_ThisWeek);
                            ++(tp->Won_ThisSeason);
                        }
                    }
                }
            }

            m_teams[i]->SaveToDB();
        }
    }

    sObjectMgr.UpdateArenaTeamRankings();

    m_nextPvPUpdateTime = 0;
    UpdatePvPData();
    PlaySoundToAll(winningTeam ? SOUND_ALLIANCEWINS : SOUND_HORDEWINS);

    sEventMgr.RemoveEvents(this, EVENT_BATTLEGROUND_CLOSE);
    sEventMgr.AddEvent(static_cast< CBattleground* >(this), &CBattleground::Close, EVENT_BATTLEGROUND_CLOSE, 120000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);

    for (uint8 i = 0; i < 2; i++)
    {
        bool victorious = (i == winningTeam);
        std::set<Player*>::iterator itr = m_players[i].begin();
        for (; itr != m_players[i].end(); ++itr)
        {
            Player* plr = *itr;
            if (plr != NULL)
            {
                sHookInterface.OnArenaFinish(plr, plr->getArenaTeam(m_arenateamtype), victorious, rated_match);
                plr->resetAllCooldowns();
            }
        }
    }

    /* Prevent honor being given to people in arena */
    return false;
}

void Arena::OnAddPlayer(Player* plr)
{
    if (plr == NULL)
        return;

    plr->m_deathVision = true;

    // remove all buffs (exclude talents, include flasks)
    for (uint32 x = AuraSlots::REMOVABLE_SLOT_START; x < AuraSlots::REMOVABLE_SLOT_END; x++)
    {
        if (auto* const aur = plr->getAuraWithAuraSlot(x))
        {
            if (!aur->getSpellInfo()->getDurationIndex() && aur->getSpellInfo()->getAttributesExC() & ATTRIBUTESEXC_CAN_PERSIST_AND_CASTED_WHILE_DEAD)
                continue;

            aur->removeAura();
        }
    }
    // On arena start all conjured items are removed
    plr->getItemInterface()->RemoveAllConjured();
    // On arena start remove all temp enchants
    plr->removeTempItemEnchantsOnArena();

    // Before the arena starts all your cooldowns are reset
    if (!m_started  && plr->IsInWorld())
        plr->resetAllCooldowns();

    // if (plr->m_isGmInvisible == false)
    // Make sure the player isn't a GM an isn't invisible (monitoring?)
    if (!plr->m_isGmInvisible)
    {
        if (!m_started  && plr->IsInWorld())
            plr->castSpell(plr, ARENA_PREPARATION, true);

        m_playersCount[plr->getTeam()]++;
        UpdatePlayerCounts();
    }
    // If they're still queued for the arena, remove them from the queue
    if (plr->isQueuedForBg())
        plr->setIsQueuedForBg(false);

    // Add the green/gold team flag
    Aura* aura = sSpellMgr.newAura(sSpellMgr.getSpellInfo((plr->getInitialTeam()) ? 35775 - plr->getBgTeam() : 32725 - plr->getBgTeam()), -1, plr, plr, true);
    plr->addAura(aura);

    plr->setFfaPvpFlag();

    m_playersAlive.insert(plr->getGuidLow());
}

void Arena::OnRemovePlayer(Player* plr)
{
    // remove arena readiness buff
    plr->m_deathVision = false;

    plr->removeAllAuras();

    // Player has left arena, call HookOnPlayerDeath as if he died
    HookOnPlayerDeath(plr);

    plr->removeAllAurasById(plr->getInitialTeam() ? 35775 - plr->getBgTeam() : 32725 - plr->getBgTeam());
    plr->removeFfaPvpFlag();

    // Reset all their cooldowns and restore their HP/Mana/Energy to max
    plr->resetAllCooldowns();
    plr->setFullHealthMana();
}

void Arena::HookOnPlayerKill(Player* plr, Player* pVictim)
{
#ifdef ANTI_CHEAT
    if (!m_started)
    {
        plr->kill(); //cheater.
        return;
    }
#endif
    if (pVictim->isPlayer())
    {
        plr->m_bgScore.KillingBlows++;
    }
}

void Arena::HookOnHK(Player* plr)
{
    plr->m_bgScore.HonorableKills++;
}

void Arena::HookOnPlayerDeath(Player* plr)
{
    if (plr)
    {
        if (plr->m_isGmInvisible == true)
            return;

        if (m_playersAlive.find(plr->getGuidLow()) != m_playersAlive.end())
        {
            m_playersCount[plr->getTeam()]--;
            UpdatePlayerCounts();
            m_playersAlive.erase(plr->getGuidLow());
        }
    }
    else
    {
        sLogger.failure("Tried to call Arena::HookOnPlayerDeath with nullptr player pointer");
    }
}

void Arena::OnCreate()
{
    // push gates into world
    for (std::set<GameObject*>::iterator itr = m_gates.begin(); itr != m_gates.end(); ++itr)
        (*itr)->PushToWorld(m_mapMgr);
}

void Arena::HookOnShadowSight()
{}

void Arena::OnStart()
{
    // remove arena readiness buff
    for (uint8 i = 0; i < 2; ++i)
    {
        for (std::set<Player*>::iterator itr = m_players[i].begin(); itr != m_players[i].end(); ++itr)
        {
            Player* plr = *itr;
            plr->removeAllAurasById(ARENA_PREPARATION);
            m_players2[i].insert(plr->getGuidLow());

            // update arena team stats
            if (rated_match && plr->isInArenaTeam(m_arenateamtype))
            {
                m_teams[i] = plr->getArenaTeam(m_arenateamtype);
                ArenaTeamMember* tp = m_teams[i]->GetMember(plr->getPlayerInfo());
                if (tp != NULL)
                {
                    tp->Played_ThisWeek++;
                    tp->Played_ThisSeason++;
                }
            }
        }
    }

    for (uint8 i = 0; i < 2; i++)
    {
        if (m_teams[i] == NULL)
            continue;

        m_teams[i]->m_stats.played_season++;
        m_teams[i]->m_stats.played_week++;
        m_teams[i]->SaveToDB();
    }

    // open gates
    for (std::set<GameObject*>::iterator itr = m_gates.begin(); itr != m_gates.end(); ++itr)
    {
        (*itr)->setFlags(GO_FLAG_TRIGGERED);
        (*itr)->setState(GO_STATE_CLOSED);
    }

    m_started = true;

    // Incase all players left
    UpdatePlayerCounts();

    // WHEEEE
    PlaySoundToAll(SOUND_BATTLEGROUND_BEGIN);

    sEventMgr.RemoveEvents(this, EVENT_ARENA_SHADOW_SIGHT);
    sEventMgr.AddEvent(static_cast< CBattleground* >(this), &CBattleground::HookOnShadowSight, EVENT_ARENA_SHADOW_SIGHT, 90000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
}

void Arena::UpdatePlayerCounts()
{
    if (this->HasEnded())
        return;

    SetWorldState(WORLDSTATE_ARENA__GREEN_PLAYER_COUNT, m_playersCount[0]);
    SetWorldState(WORLDSTATE_ARENA__GOLD_PLAYER_COUNT, m_playersCount[1]);

    if (!m_started)
        return;

    //    return;

    if (m_playersCount[TEAM_HORDE] == 0)
        this->EndBattleground(TEAM_ALLIANCE);
    else if (m_playersCount[TEAM_ALLIANCE] == 0)
        this->EndBattleground(TEAM_HORDE);
}

uint32 Arena::CalcDeltaRating(uint32 oldRating, uint32 opponentRating, bool outcome)
{
    double power = (int)(opponentRating - oldRating) / 400.0f;
    double divisor = pow(10.0, power);
    divisor += 1.0;

    double winChance = 1.0 / divisor;

    // New Rating Calculation via Elo
    // New Rating = Old Rating + K * (outcome - Expected Win Chance)
    // outcome = 1 for a win and 0 for a loss (0.5 for a draw ... but we cant have that)
    // K is the maximum possible change
    // Through investigation, K was estimated to be 32 (same as chess)
    double multiplier = (outcome ? 1.0 : 0.0) - winChance;
    return long2int32(32.0 * multiplier);
}

uint32 Arena::GetTeamFaction(uint32 team)
{
    std::set< Player* >::iterator itr = m_players[team].begin();
    Player* p = *itr;
    return p->getTeam();
}

LocationVector Arena::GetStartingCoords(uint32 /*Team*/)
{
    return LocationVector(0, 0, 0, 0);
}

bool Arena::HookHandleRepop(Player* /*plr*/)
{
    return false;
}

void Arena::HookOnAreaTrigger(Player* plr, uint32 id)
{
    int32 buffslot = -1;

    if (plr)
    {
        switch (id)
        {
        case 4536:
        case 4538:
        case 4696:
            buffslot = 0;
            break;
        case 4537:
        case 4539:
        case 4697:
            buffslot = 1;
            break;
        default:
            break;
        }

        if (buffslot >= 0)
        {
            if (m_buffs[buffslot] != NULL && m_buffs[buffslot]->IsInWorld())
            {
                // apply the buff
                SpellInfo const* sp = sSpellMgr.getSpellInfo(m_buffs[buffslot]->GetGameObjectProperties()->raw.parameter_3);
                if (sp == nullptr)
                {
                    sLogger.failure("Arena::HookOnAreaTrigger: tried to use invalid spell %u for gameobject %u", m_buffs[buffslot]->GetGameObjectProperties()->raw.parameter_3, m_buffs[buffslot]->GetGameObjectProperties()->entry);
                }

                Spell* s = sSpellMgr.newSpell(plr, sp, true, 0);
                SpellCastTargets targets(plr->getGuid());
                s->prepare(&targets);

                // despawn the gameobject (not delete!)
                m_buffs[buffslot]->despawn(0, 30 /*BUFF_RESPAWN_TIME*/);
            }
        }
    }
    else
    {
        sLogger.failure("Tried to call Arena::HookOnAreaTrigger with nullptr player pointer");
    }
}

void Arena::HookGenerateLoot(Player* /*plr*/, Object* /*pCorpse*/)    // Not Used
{}

void Arena::HookOnUnitKill(Player* /*plr*/, Unit* /*pVictim*/)
{}

void Arena::HookOnFlagDrop(Player* /*plr*/)
{}
