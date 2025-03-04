/*
Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/


#include "AchievementMgr.h"
#include "Objects/Item.h"
#include "Objects/Units/Stats.h"
#include "Server/WorldSocket.h"
#include "Storage/MySQLDataStore.hpp"
#include "Server/MainServerDefines.h"
#include "Map/Maps/InstanceDefines.hpp"
#include "Map/Management/MapMgr.hpp"
#include "Management/Faction.h"
#include "Spell/Definitions/SpellMechanics.hpp"
#include "Spell/SpellMgr.hpp"
#include "Spell/Definitions/SpellEffects.hpp"
#include "Guild/Guild.hpp"
#include "Server/Packets/SmsgServerFirstAchievement.h"
#include "Server/Packets/SmsgAchievementDeleted.h"
#include "Server/Packets/SmsgCriteriaDeleted.h"
#include "Server/Packets/SmsgCriteriaUpdate.h"
#include "Server/Packets/SmsgMessageChat.h"
#include "Server/Script/ScriptMgr.h"
#include "Macros/ScriptMacros.hpp"

using namespace AscEmu::Packets;

#if VERSION_STRING > TBC
// APGL End
// MIT Start
bool AchievementMgr::canCompleteCriteria(DBC::Structures::AchievementCriteriaEntry const* achievementCriteria, AchievementCriteriaTypes type, Player* player) const
{
    switch (type)
    {
        case ACHIEVEMENT_CRITERIA_TYPE_REACH_LEVEL:
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST_COUNT:
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUESTS_IN_ZONE:
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST:
        case ACHIEVEMENT_CRITERIA_TYPE_GAIN_REPUTATION:
        case ACHIEVEMENT_CRITERIA_TYPE_GAIN_EXALTED_REPUTATION:
        case ACHIEVEMENT_CRITERIA_TYPE_NUMBER_OF_MOUNTS:
        case ACHIEVEMENT_CRITERIA_TYPE_REACH_SKILL_LEVEL:
        case ACHIEVEMENT_CRITERIA_TYPE_LEARN_SKILL_LEVEL:
        case ACHIEVEMENT_CRITERIA_TYPE_BUY_BANK_SLOT:
        case ACHIEVEMENT_CRITERIA_TYPE_WIN_RATED_ARENA:
        case ACHIEVEMENT_CRITERIA_TYPE_ROLL_NEED_ON_LOOT:
        case ACHIEVEMENT_CRITERIA_TYPE_ROLL_GREED_ON_LOOT:
            return true;
        case ACHIEVEMENT_CRITERIA_TYPE_EXPLORE_AREA:
            return player->hasOverlayUncovered(achievementCriteria->explore_area.areaReference);
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_ACHIEVEMENT:
            return m_completedAchievements.find(achievementCriteria->complete_achievement.linkedAchievement) != m_completedAchievements.end();
        case ACHIEVEMENT_CRITERIA_TYPE_LEARN_SPELL:
            return player->HasSpell(achievementCriteria->learn_spell.spellID);
        default:
            break;
    }

    return false;
}

bool AchievementMgr::canCompleteCriteria(DBC::Structures::AchievementCriteriaEntry const* achievementCriteria, AchievementCriteriaTypes type, int32_t miscValue1, int32_t /*miscValue2*/, Player* player) const
{
    switch (type)
    {
        case ACHIEVEMENT_CRITERIA_TYPE_REACH_LEVEL:
        case ACHIEVEMENT_CRITERIA_TYPE_KILLING_BLOW:
        case ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE_TYPE:
        case ACHIEVEMENT_CRITERIA_TYPE_FALL_WITHOUT_DYING:
        case ACHIEVEMENT_CRITERIA_TYPE_QUEST_REWARD_GOLD:
        case ACHIEVEMENT_CRITERIA_TYPE_LOOT_MONEY:
        case ACHIEVEMENT_CRITERIA_TYPE_GAIN_EXALTED_REPUTATION:
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST_COUNT:
        case ACHIEVEMENT_CRITERIA_TYPE_BUY_BANK_SLOT:
        case ACHIEVEMENT_CRITERIA_TYPE_VISIT_BARBER_SHOP:
        case ACHIEVEMENT_CRITERIA_TYPE_GOLD_SPENT_AT_BARBER:
        case ACHIEVEMENT_CRITERIA_TYPE_EARN_HONORABLE_KILL:
        case ACHIEVEMENT_CRITERIA_TYPE_KILLED_BY_PLAYER:
        case ACHIEVEMENT_CRITERIA_TYPE_KILLED_BY_CREATURE:
        case ACHIEVEMENT_CRITERIA_TYPE_DEATH:
        case ACHIEVEMENT_CRITERIA_TYPE_WIN_RATED_ARENA:
        case ACHIEVEMENT_CRITERIA_TYPE_ROLL_NEED_ON_LOOT:
        case ACHIEVEMENT_CRITERIA_TYPE_ROLL_GREED_ON_LOOT:
            return true;
        case ACHIEVEMENT_CRITERIA_TYPE_LOOT_ITEM:
        case ACHIEVEMENT_CRITERIA_TYPE_OWN_ITEM:
            return achievementCriteria->loot_item.itemID == static_cast<uint32_t>(miscValue1);
        case ACHIEVEMENT_CRITERIA_TYPE_EXPLORE_AREA:
            return player->hasOverlayUncovered(achievementCriteria->explore_area.areaReference);
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUESTS_IN_ZONE:
            return achievementCriteria->complete_quests_in_zone.zoneID == static_cast<uint32_t>(miscValue1);
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST:
            return achievementCriteria->complete_quest.questID == static_cast<uint32_t>(miscValue1);
        case ACHIEVEMENT_CRITERIA_TYPE_GAIN_REPUTATION:
            return achievementCriteria->gain_reputation.factionID == static_cast<uint32_t>(miscValue1);
        case ACHIEVEMENT_CRITERIA_TYPE_LEARN_SPELL:
            return achievementCriteria->learn_spell.spellID == static_cast<uint32_t>(miscValue1);
        case ACHIEVEMENT_CRITERIA_TYPE_NUMBER_OF_MOUNTS:
            return achievementCriteria->number_of_mounts.unknown == static_cast<uint32_t>(miscValue1);
        case ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET:
        case ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET2:
            return achievementCriteria->be_spell_target.spellID == static_cast<uint32_t>(miscValue1);
        case ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE:
            return achievementCriteria->kill_creature.creatureID == static_cast<uint32_t>(miscValue1);
        case ACHIEVEMENT_CRITERIA_TYPE_REACH_SKILL_LEVEL:
            return achievementCriteria->reach_skill_level.skillID == static_cast<uint32_t>(miscValue1);
        case ACHIEVEMENT_CRITERIA_TYPE_LEARN_SKILL_LEVEL:
            return achievementCriteria->learn_skill_level.skillID == static_cast<uint32_t>(miscValue1);
        case ACHIEVEMENT_CRITERIA_TYPE_EQUIP_ITEM:
            return achievementCriteria->equip_item.itemID == static_cast<uint32_t>(miscValue1);
        case ACHIEVEMENT_CRITERIA_TYPE_EQUIP_EPIC_ITEM:
            return achievementCriteria->equip_epic_item.itemSlot == static_cast<uint32_t>(miscValue1);
        case ACHIEVEMENT_CRITERIA_TYPE_DO_EMOTE:
            return achievementCriteria->do_emote.emoteID == static_cast<uint32_t>(miscValue1);
        case ACHIEVEMENT_CRITERIA_TYPE_USE_ITEM:
            return achievementCriteria->use_item.itemID == static_cast<uint32_t>(miscValue1);
        case ACHIEVEMENT_CRITERIA_TYPE_USE_GAMEOBJECT:
            return achievementCriteria->use_gameobject.goEntry == static_cast<uint32_t>(miscValue1);
        case ACHIEVEMENT_CRITERIA_TYPE_HONORABLE_KILL_AT_AREA:
            return achievementCriteria->honorable_kill_at_area.areaID == static_cast<uint32_t>(miscValue1);
        case ACHIEVEMENT_CRITERIA_TYPE_HK_CLASS:
            return achievementCriteria->hk_class.classID == static_cast<uint32_t>(miscValue1);
        case ACHIEVEMENT_CRITERIA_TYPE_HK_RACE:
            return achievementCriteria->hk_race.raceID == static_cast<uint32_t>(miscValue1);
        case ACHIEVEMENT_CRITERIA_TYPE_DEATH_AT_MAP:
            return achievementCriteria->death_at_map.mapID == static_cast<uint32_t>(miscValue1);
        default:
            break;
    }

    return false;
}

// MIT End
// APGL Start
//////////////////////////////////////////////////////////////////////////////////////////
/// Takes achievementlink c-string
/// \return c-string ID value
uint32_t GetAchievementIDFromLink(const char* achievementlink)
{
    if (achievementlink == nullptr)
        return 0;

    const char* ptr = strstr(achievementlink, "|Hachievement:");
    if (ptr == nullptr)
        return 0;

    // achievement id is just past "|Hachievement:" (14 bytes)
    return atol(ptr + 14);
}

//////////////////////////////////////////////////////////////////////////////////////////
/// True if CriteriaProgress should be sent to Player; False if CriteriaProgress should not be sent.
///    If the CriteriaProgress specified should not be sent to the Player, it returns false, otherwise it returns true.
///    Examples of CriteriaProgress that should not be sent to the Player are:
///    1. When counter is zero or negative, which would indicate the achievement hasn't been started yet.
///    2. Reputation type achievements, where the progress is not shown in the client.
///    3. Reach-Level type achievements, where the progress is not shown in the client.
bool SendAchievementProgress(const CriteriaProgress* c)
{
    if (c == nullptr || c->counter <= 0)
        // achievement not started yet, don't send progress
        return false;

    auto acEntry = sAchievementCriteriaStore.LookupEntry(c->id);
    if (!acEntry)
        return false;

    if (acEntry->requiredType == ACHIEVEMENT_CRITERIA_TYPE_GAIN_REPUTATION)
        // Exalted with X faction (don't send 12323/42000 progress, it's not shown anyway)
        return false;

    if (acEntry->requiredType == ACHIEVEMENT_CRITERIA_TYPE_REACH_LEVEL)
        // Reach level (don't send 7/80 progress, it's not shown anyway)
        return false;

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
///  True if CriteriaProgress should be saved to database.  False if CriteriaProgress should not be saved to database.
///    Not all achievement progresses get saved to progress database, since some are saved in the character database,
///    or are easily computable when the player logs in.
bool SaveAchievementProgressToDB(const CriteriaProgress* c)
{
    if (c->counter <= 0)
    {
        // don't save it if it's not started yet
        return false;
    }

    auto achievement = sAchievementCriteriaStore.LookupEntry(c->id);
    if (achievement == nullptr)
        return false;

    switch (achievement->requiredType)
    {
        // these get updated when character logs on, don't save to character progress db
        case ACHIEVEMENT_CRITERIA_TYPE_REACH_LEVEL:
        case ACHIEVEMENT_CRITERIA_TYPE_EXPLORE_AREA:
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_ACHIEVEMENT:
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST_COUNT:
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUESTS_IN_ZONE:
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST:
        case ACHIEVEMENT_CRITERIA_TYPE_GAIN_REPUTATION:
        case ACHIEVEMENT_CRITERIA_TYPE_GAIN_EXALTED_REPUTATION:
        case ACHIEVEMENT_CRITERIA_TYPE_LEARN_SPELL:
        case ACHIEVEMENT_CRITERIA_TYPE_NUMBER_OF_MOUNTS:
        case ACHIEVEMENT_CRITERIA_TYPE_REACH_SKILL_LEVEL:
        case ACHIEVEMENT_CRITERIA_TYPE_LEARN_SKILL_LEVEL:
        case ACHIEVEMENT_CRITERIA_TYPE_BUY_BANK_SLOT:
            return false;
        default:
            break;
    }
    return true;
}

/// true if the achievement should be shown; false otherwise
bool ShowCompletedAchievement(uint32_t achievementID, const Player* plr)
{
    switch (achievementID)
    {
        case  457: // Realm First! Level 80
        case  467: // Realm First! Level 80 Shaman
        case  466: // Realm First! Level 80 Druid
        case  465: // Realm First! Level 80 Paladin
        case  464: // Realm First! Level 80 Priest
        case  463: // Realm First! Level 80 Warlock
        case  462: // Realm First! Level 80 Hunter
        case  461: // Realm First! Level 80 Death Knight
        case  460: // Realm First! Level 80 Mage
        case  459: // Realm First! Level 80 Warrior
        case  458: // Realm First! Level 80 Rogue
        case 1404: // Realm First! Level 80 Gnome
        case 1405: // Realm First! Level 80 Blood Elf
        case 1406: // Realm First! Level 80 Draenei
        case 1407: // Realm First! Level 80 Dwarf
        case 1408: // Realm First! Level 80 Human
        case 1409: // Realm First! Level 80 Night Elf
        case 1410: // Realm First! Level 80 Orc
        case 1411: // Realm First! Level 80 Tauren
        case 1412: // Realm First! Level 80 Troll
        case 1413: // Realm First! Level 80 Forsaken
        case 1415: // Realm First! Grand Master Alchemist
        case 1414: // Realm First! Grand Master Blacksmith
        case 1416: // Realm First! Cooking Grand Master
        case 1417: // Realm First! Grand Master Enchanter
        case 1418: // Realm First! Grand Master Engineer
        case 1419: // Realm First! First Aid Grand Master
        case 1420: // Realm First! Grand Master Angler
        case 1421: // Realm First! Grand Master Herbalist
        case 1422: // Realm First! Grand Master Scribe
        case 1423: // Realm First! Grand Master Jewelcrafter
        case 1424: // Realm First! Grand Master Leatherworker
        case 1425: // Realm First! Grand Master Miner
        case 1426: // Realm First! Grand Master Skinner
        case 1427: // Realm First! Grand Master Tailor
        case 1463: // Realm First! Northrend Vanguard: First player on the realm to gain exalted reputation with the Argent Crusade, Wyrmrest Accord, Kirin Tor and Knights of the Ebon Blade.
        {
            QueryResult* achievementResult = CharacterDatabase.Query("SELECT guid FROM character_achievement WHERE achievement=%u ORDER BY date LIMIT 1", achievementID);
            if (achievementResult != nullptr)
            {
                Field* field = achievementResult->Fetch();
                if (field != nullptr)
                {
                    // somebody has this Realm First achievement... is it this player?
                    uint64_t firstguid = field->GetUInt32();
                    if (firstguid != (uint32_t)plr->getGuid())
                    {
                        // nope, somebody else was first.
                        delete achievementResult;
                        return false;
                    }
                }
                delete achievementResult;
            }
        }
        break;
        /* All raid members should receive these last 3 Realm First achievements when they first occur.
           (not implemented yet)
               case 1400: // Realm First! Magic Seeker: Participated in the realm first defeat of Malygos on Heroic Difficulty.
               case  456: // Realm First! Obsidian Slayer: Participated in the realm first defeat of Sartharion the Onyx Guardian on Heroic Difficulty.
               case 1402: // Realm First! Conqueror of Naxxramas: Participated in the realm first defeat of Kel'Thuzad on Heroic Difficulty in Naxxramas. */
        default:
            break;
    }
    return true;
}

/// AchievementMgr constructor
AchievementMgr::AchievementMgr(Player* player) : m_player(player), isCharacterLoading(true)
{
    
}

/// AchievementMgr destructor
AchievementMgr::~AchievementMgr()
{
    for (auto iterCriteriaProgress : m_criteriaProgress)
        delete iterCriteriaProgress.second;

    m_criteriaProgress.clear();
    m_completedAchievements.clear();
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Save Achievement data to database
/// \brief Saves all completed achievements to database. Saves all achievement
/// progresses that have been started, and that aren't calculated on login, to database.
void AchievementMgr::SaveToDB(QueryBuffer* buf)
{
    if (!m_completedAchievements.empty())
    {
        std::ostringstream ss;

        ss << "DELETE FROM character_achievement WHERE guid = ";
        ss << m_player->getGuidLow();
        ss << ";";

        if (buf == nullptr)
        {
            CharacterDatabase.ExecuteNA(ss.str().c_str());
        }
        else
        {
            buf->AddQueryNA(ss.str().c_str());
        }

        ss.rdbuf()->str("");

        ss << "INSERT INTO character_achievement VALUES ";
        bool first = true;
        for (auto iterCompletedAchievements : m_completedAchievements)
        {
            if (ss.str().length() >= 16000)
            {
                // SQL query length is limited to 16384 characters
                if (buf == nullptr)
                    CharacterDatabase.ExecuteNA(ss.str().c_str());
                else
                    buf->AddQueryNA(ss.str().c_str());
                ss.str("");
                ss << "INSERT INTO character_achievement VALUES ";
                first = true;
            }

            if (!first)
            {
                ss << ", ";
            }
            else
            {
                first = false;
            }
            ss << "(" << m_player->getGuidLow() << ", " << iterCompletedAchievements.first << ", " << iterCompletedAchievements.second << ")";
        }
        if (buf == nullptr)
            CharacterDatabase.ExecuteNA(ss.str().c_str());
        else
            buf->AddQueryNA(ss.str().c_str());
    }

    if (!m_criteriaProgress.empty())
    {
        std::ostringstream ss;

        ss << "DELETE FROM character_achievement_progress WHERE guid = ";
        ss << m_player->getGuidLow();
        ss << ";";

        if (buf == nullptr)
            CharacterDatabase.ExecuteNA(ss.str().c_str());
        else
            buf->AddQueryNA(ss.str().c_str());

        ss.rdbuf()->str("");

        ss << "INSERT INTO character_achievement_progress VALUES ";
        
        bool first = true;
        for (auto iterCriteriaProgress : m_criteriaProgress)
        {
            if (SaveAchievementProgressToDB(iterCriteriaProgress.second))
            {
                // only save some progresses, others will be updated when character logs in
                if (ss.str().length() >= 16000)
                {
                    // SQL query length is limited to 16384 characters
                    if (buf == nullptr)
                        CharacterDatabase.ExecuteNA(ss.str().c_str());
                    else
                        buf->AddQueryNA(ss.str().c_str());
                    ss.str("");
                    ss << "INSERT INTO character_achievement_progress VALUES ";
                    first = true;
                }
                if (!first)
                {
                    ss << ", ";
                }
                else
                {
                    first = false;
                }
                ss << "(" << m_player->getGuidLow() << ", " << iterCriteriaProgress.first << ", " << iterCriteriaProgress.second->counter << ", " << iterCriteriaProgress.second->date << ")";
            }
        }
        if (!first)
        {
            // don't execute query if there's no entries to save
            if (buf == nullptr)
                CharacterDatabase.ExecuteNA(ss.str().c_str());
            else
                buf->AddQueryNA(ss.str().c_str());
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Load achievements from database.
/// \brief Loads completed achievements and achievement progresses from the database
void AchievementMgr::LoadFromDB(QueryResult* achievementResult, QueryResult* criteriaResult)
{
    if (achievementResult)
    {
        do
        {
            Field* fields = achievementResult->Fetch();
            uint32_t id = fields[0].GetUInt32();
            if (m_completedAchievements[id] == 0)
                m_completedAchievements[id] = fields[1].GetUInt32();
            else
                sLogger.failure("Duplicate completed achievement %u for player %u, skipping", id, (uint32_t)m_player->getGuid());
        }
        while (achievementResult->NextRow());
    }

    if (criteriaResult)
    {
        do
        {
            Field* fields = criteriaResult->Fetch();
            uint32_t progress_id = fields[0].GetUInt32();
            if (m_criteriaProgress[progress_id] == nullptr)
            {
                CriteriaProgress* progress = new CriteriaProgress(progress_id, fields[1].GetUInt32(), static_cast<time_t>(fields[2].GetUInt64()));
                m_criteriaProgress[progress_id] = progress;
            }
            else
                sLogger.failure("Duplicate criteria progress %u for player %u, skipping", progress_id, (uint32_t)m_player->getGuid());

        }
        while (criteriaResult->NextRow());
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Sends message to player(s) that the achievement has been completed.
/// Realm first! achievements get sent to all players currently online.
/// All other achievements get sent to all of the achieving player's guild members,
/// group members, and other in-range players
void AchievementMgr::SendAchievementEarned(DBC::Structures::AchievementEntry const* achievement)
{
    if (achievement == nullptr || isCharacterLoading)
    {
        return;
    }

    const char* msg = "|Hplayer:$N|h[$N]|h has earned the achievement $a!";
    uint32_t* guidList = nullptr;
    uint32_t guidCount = 0;
    uint32_t guidIndex = 0;

    // Send Achievement message to everyone currently on the server
    if (achievement->flags & (ACHIEVEMENT_FLAG_REALM_FIRST_KILL | ACHIEVEMENT_FLAG_REALM_FIRST_REACH))
    {
        std::string playerName = GetPlayer()->getName();
        uint64_t guid = GetPlayer()->getGuid();

        // own team = clickable name
        sWorld.sendGlobalMessage(SmsgServerFirstAchievement(playerName, guid, achievement->ID, 1).serialise().get(), nullptr, GetPlayer()->GetTeam());
        
        sWorld.sendGlobalMessage(SmsgServerFirstAchievement(playerName, guid, achievement->ID, 0).serialise().get(), nullptr, GetPlayer()->GetTeam() == TEAM_ALLIANCE ? TEAM_HORDE : TEAM_ALLIANCE);
    }
    else
    {
        // allocate enough space
        guidList = new uint32_t[sWorld.getSessionCount() + 256];

        bool alreadySent;

        // Send Achievement message to group members
        if (Group* grp = GetPlayer()->getGroup())
        {
            // grp->SendPacketToAll(&cdata);
            uint8_t i = 0;
            grp->Lock();
            for (GroupMembersSet::iterator groupItr; i < grp->GetSubGroupCount(); ++i)
            {
                SubGroup* sg = grp->GetSubGroup(i);
                if (sg == nullptr)
                    continue;

                groupItr = sg->GetGroupMembersBegin();
                GroupMembersSet::iterator groupItrLast = sg->GetGroupMembersEnd();
                for (; groupItr != groupItrLast; ++groupItr)
                {
                    if (Player* loggedInPlayer = sObjectMgr.GetPlayer((*groupItr)->guid))
                    {
                        if (loggedInPlayer->getSession())
                        {
                            // check if achievement message has already been sent to this player (if they received a guild achievement message already)
                            alreadySent = false;
                            for (guidIndex = 0; guidIndex < guidCount; ++guidIndex)
                            {
                                if (guidList[guidIndex] == (*groupItr)->guid)
                                {
                                    alreadySent = true;
                                    guidIndex = guidCount;
                                }
                            }

                            if (!alreadySent)
                            {
                                loggedInPlayer->getSession()->SendPacket(SmsgMessageChat(CHAT_MSG_ACHIEVEMENT, LANG_UNIVERSAL, 0, msg, GetPlayer()->getGuid(), "", GetPlayer()->getGuid(), "", achievement->ID).serialise().get());
                                guidList[guidCount++] = (*groupItr)->guid;
                            }
                        }
                    }
                }
            }
            grp->Unlock();
        }

        // Send Achievement message to nearby players
        for (const auto& inRangeItr : GetPlayer()->getInRangePlayersSet())
        {
            Player* p = static_cast<Player*>(inRangeItr);
            if (p && p->getSession() && !p->isIgnored(GetPlayer()->getGuidLow()))
            {
                // check if achievement message has already been sent to this player (in guild or group)
                alreadySent = false;
                for (guidIndex = 0; guidIndex < guidCount; ++guidIndex)
                {
                    if (guidList[guidIndex] == p->getGuidLow())
                    {
                        alreadySent = true;
                        guidIndex = guidCount;
                    }
                }
                if (!alreadySent)
                {
                    p->getSession()->SendPacket(SmsgMessageChat(CHAT_MSG_ACHIEVEMENT, LANG_UNIVERSAL, 0, msg, GetPlayer()->getGuid(), "", GetPlayer()->getGuid(), "", achievement->ID).serialise().get());
                    guidList[guidCount++] = p->getGuidLow();
                }
            }
        }
        // Have we sent the message to the achieving player yet?
        alreadySent = false;
        for (guidIndex = 0; guidIndex < guidCount; ++guidIndex)
        {
            if (guidList[guidIndex] == GetPlayer()->getGuidLow())
            {
                alreadySent = true;
                guidIndex = guidCount;
            }
            if (!alreadySent)
            {
                GetPlayer()->getSession()->SendPacket(SmsgMessageChat(CHAT_MSG_ACHIEVEMENT, LANG_UNIVERSAL, 0, msg, GetPlayer()->getGuid(), "", GetPlayer()->getGuid(), "", achievement->ID).serialise().get());
            }
        }
    }
    //    GetPlayer()->sendMessageToSet(&cdata, true);

    WorldPacket data(SMSG_ACHIEVEMENT_EARNED, 30);
    data << GetPlayer()->GetNewGUID();
    data << uint32_t(achievement->ID);
    data << uint32_t(secsToTimeBitFields(UNIXTIME));
    data << uint32_t(0);
    GetPlayer()->getSession()->SendPacket(&data);
    if (guidList)
    {
        delete[] guidList;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
/// \brief Sends update to achievement criteria to the player.
void AchievementMgr::SendCriteriaUpdate(CriteriaProgress* progress)
{
    if (progress == nullptr || isCharacterLoading)
        return;

    GetPlayer()->sendPacket(SmsgCriteriaUpdate(progress->id, progress->counter, GetPlayer()->GetNewGUID(), secsToTimeBitFields(progress->date)).serialise().get());
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Updates ALL achievement criteria
/// \brief This is called during player login to update some criteria which aren't
/// saved in achievement progress DB, since they are saved in the character DB or
/// can easily be computed.
void AchievementMgr::CheckAllAchievementCriteria()
{
    for (uint8_t i = 0; i < ACHIEVEMENT_CRITERIA_TYPE_TOTAL; i++)
        UpdateAchievementCriteria(AchievementCriteriaTypes(i));
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Updates achievement criteria of the specified type
/// \brief This is what should be called from other places in the code (upon killing a
/// monster, or looting an object, or completing a quest, etc.). miscvalue1, miscvalue2
/// depend on the achievement type. Generally, miscvalue1 is an ID of some type (quest ID,
/// item ID, faction ID, etc.), and miscvalue2 is the amount to increase the progress.
void AchievementMgr::UpdateAchievementCriteria(AchievementCriteriaTypes type, int32_t miscvalue1, int32_t miscvalue2, uint32_t /*time*/, Object* reference /*nullptr*/)
{
    if (m_player->getSession()->HasGMPermissions() && worldConfig.gm.disableAchievements)
        return;

    uint64_t selectedGUID = 0;
    if (type == ACHIEVEMENT_CRITERIA_TYPE_DO_EMOTE)
    {
        selectedGUID = GetPlayer()->getTargetGuid();
    }
    AchievementCriteriaEntryList const & achievementCriteriaList = sObjectMgr.GetAchievementCriteriaByType(type);
    for (AchievementCriteriaEntryList::const_iterator i = achievementCriteriaList.begin(); i != achievementCriteriaList.end(); ++i)
    {
        DBC::Structures::AchievementCriteriaEntry const* achievementCriteria = (*i);

        if (IsCompletedCriteria(achievementCriteria))
        {
            // don't bother updating it, if it has already been completed
            continue;
        }

        if ((achievementCriteria->groupFlag & ACHIEVEMENT_CRITERIA_GROUP_NOT_IN_GROUP) && GetPlayer()->getGroup())
        {
            // criteria requires that the player not be in a group, but they are in a group, so don't update it
            continue;
        }

        auto achievement = sAchievementStore.LookupEntry(achievementCriteria->referredAchievement);
        if (!achievement)
        {
            // referred achievement not found (shouldn't normally happen)
            continue;
        }

        if ((achievement->factionFlag == ACHIEVEMENT_FACTION_FLAG_HORDE && GetPlayer()->isTeamHorde() == false) ||
            (achievement->factionFlag == ACHIEVEMENT_FACTION_FLAG_ALLIANCE && GetPlayer()->isTeamAlliance() == false))
        {
            // achievement requires a faction of which the player is not a member
            continue;
        }

        if (!canCompleteCriteria(achievementCriteria, type, miscvalue1, miscvalue2, GetPlayer()))
            continue;

        // Check scripted criteria checks
        const auto scriptResult = sScriptMgr.callScriptedAchievementCriteriaCanComplete(achievementCriteria->ID, GetPlayer(), reference);
        if (!scriptResult)
            continue;

        switch (type)
        {
            //Start of Achievement List
            case ACHIEVEMENT_CRITERIA_TYPE_REACH_LEVEL:
                SetCriteriaProgress(achievementCriteria, GetPlayer()->getLevel());
                break;
            case ACHIEVEMENT_CRITERIA_TYPE_LOOT_ITEM:
            case ACHIEVEMENT_CRITERIA_TYPE_OWN_ITEM:
                UpdateCriteriaProgress(achievementCriteria, miscvalue2);
                break;
            case ACHIEVEMENT_CRITERIA_TYPE_EXPLORE_AREA:
            case ACHIEVEMENT_CRITERIA_TYPE_EQUIP_ITEM:
                SetCriteriaProgress(achievementCriteria, 1);
                break;
            case ACHIEVEMENT_CRITERIA_TYPE_HONORABLE_KILL_AT_AREA:
            case ACHIEVEMENT_CRITERIA_TYPE_HK_CLASS:
            case ACHIEVEMENT_CRITERIA_TYPE_HK_RACE:
            case ACHIEVEMENT_CRITERIA_TYPE_DEATH_AT_MAP:
            case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUESTS_IN_ZONE:
            case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST:
            case ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET:
            case ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET2:
            case ACHIEVEMENT_CRITERIA_TYPE_USE_GAMEOBJECT:
                UpdateCriteriaProgress(achievementCriteria, 1);
                break;
            case ACHIEVEMENT_CRITERIA_TYPE_NUMBER_OF_MOUNTS:
                // Vanity pets owned - miscvalue1==778
                // Number of mounts  - miscvalue1==777
                UpdateCriteriaProgress(achievementCriteria, 1);
                break;
            case ACHIEVEMENT_CRITERIA_TYPE_GAIN_REPUTATION:
            case ACHIEVEMENT_CRITERIA_TYPE_LEARN_SPELL:
            case ACHIEVEMENT_CRITERIA_TYPE_REACH_SKILL_LEVEL:
            case ACHIEVEMENT_CRITERIA_TYPE_LEARN_SKILL_LEVEL:
                SetCriteriaProgress(achievementCriteria, miscvalue2);
                break;
            case ACHIEVEMENT_CRITERIA_TYPE_QUEST_REWARD_GOLD:
            case ACHIEVEMENT_CRITERIA_TYPE_LOOT_MONEY:
            case ACHIEVEMENT_CRITERIA_TYPE_GAIN_EXALTED_REPUTATION:
            case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST_COUNT:
            case ACHIEVEMENT_CRITERIA_TYPE_BUY_BANK_SLOT:
            case ACHIEVEMENT_CRITERIA_TYPE_VISIT_BARBER_SHOP:
            case ACHIEVEMENT_CRITERIA_TYPE_GOLD_SPENT_AT_BARBER:
            case ACHIEVEMENT_CRITERIA_TYPE_EARN_HONORABLE_KILL:
            case ACHIEVEMENT_CRITERIA_TYPE_KILLED_BY_PLAYER:
            case ACHIEVEMENT_CRITERIA_TYPE_KILLED_BY_CREATURE:
            case ACHIEVEMENT_CRITERIA_TYPE_DEATH:
                UpdateCriteriaProgress(achievementCriteria, miscvalue1);
                break;

            // TODO: add achievement scripts for following cases
            case ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE:
                switch (achievement->ID)
                {
                    case 2556: // Pest Control
                        if ((miscvalue1 == 3300 && achievementCriteria->index == 1)       // Adder
                            || (miscvalue1 == 32261 && achievementCriteria->index == 2)   // Crystal Spider
                            || (miscvalue1 == 24270 && achievementCriteria->index == 3)   // Devouring Maggot
                            || (miscvalue1 == 9699 && achievementCriteria->index == 4)    // Fire Beetle
                            || (miscvalue1 == 24174 && achievementCriteria->index == 5)   // Fjord Rat
                            || (miscvalue1 == 32258 && achievementCriteria->index == 6)   // Gold Beetle
                            || (miscvalue1 == 16068 && achievementCriteria->index == 7)   // Larva
                            || (miscvalue1 == 16030 && achievementCriteria->index == 8)   // Maggot
                            || (miscvalue1 == 4953 && achievementCriteria->index == 9)    // Moccasin
                            || (miscvalue1 == 6271 && achievementCriteria->index == 10)   // Mouse
                            || (miscvalue1 == 4075 && achievementCriteria->index == 11)   // Rat
                            || (miscvalue1 == 4076 && achievementCriteria->index == 12)   // Roach
                            || (miscvalue1 == 15476 && achievementCriteria->index == 13)  // Scorpion
                            || (miscvalue1 == 2914 && achievementCriteria->index == 14)   // Snake
                            || (miscvalue1 == 14881 && achievementCriteria->index == 15)  // Spider
                            || (miscvalue1 == 32428 && achievementCriteria->index == 16)  // Underbelly Rat
                            || (miscvalue1 == 28202 && achievementCriteria->index == 17)) // Zul'Drak Rat
                        {
                            SetCriteriaProgress(achievementCriteria, 1);
                        }
                        break;
                        // Kill creature X in Heroic dungeon
                    case 489: // Heroic: Utgarde Keep
                    case 490: // Heroic: The Nexus
                    case 491: // Heroic: Azjol-Nerub
                    case 492: // Heroic: Ahn'kahet: The Old Kingdom
                    case 493: // Heroic: Drak'Tharon Keep
                    case 494: // Heroic: The Violet Hold
                    case 495: // Heroic: Gundrak
                    case 496: // Heroic: Halls of Stone
                    case 497: // Heroic: Halls of Lightning
                    case 498: // Heroic: The Oculus
                    case 499: // Heroic: Utgarde Pinnacle
                    case 500: // Heroic: The Culling of Stratholme
                    case 563: // Heroic: The Arachnid Quarter
                    case 565: // Heroic: The Construct Quarter
                    case 567: // Heroic: The Plague Quarter
                    case 569: // Heroic: The Military Quarter
                    case 573: // Heroic: Sapphiron's Demise
                    case 575: // Heroic: Kel'Thuzad's Defeat
                    case 577: // Heroic: The Fall of Naxxramas
                    case 623: // Heroic: The Spellweaver's Downfall
                    case 625: // Heroic: Besting the Black Dragonflight
                    case 667: // Heroic: Hellfire Ramparts
                    case 668: // Heroic: The Blood Furnace
                    case 669: // Heroic: The Slave Pens
                    case 670: // Heroic: Underbog
                    case 671: // Heroic: Mana-Tombs
                    case 672: // Heroic: Auchenai Crypts
                    case 673: // Heroic: The Escape From Durnholde
                    case 674: // Heroic: Sethekk Halls
                    case 675: // Heroic: Shadow Labyrinth
                    case 676: // Heroic: Opening of the Dark Portal
                    case 677: // Heroic: The Steamvault
                    case 678: // Heroic: The Shattered Halls
                    case 679: // Heroic: The Mechanar
                    case 680: // Heroic: The Botanica
                    case 681: // Heroic: The Arcatraz
                    case 682: // Heroic: Magister's Terrace
                    case 1312: // Utgarde Keep bosses on Heroic Difficulty.
                    case 1504: // Ingvar the Plunderer kills (Heroic Utgarde Keep)
                    case 1505: // Keristrasza kills (Heroic Nexus)
                    case 1506: // Anub'arak kills (Heroic Azjol-Nerub)
                    case 1507: // Herald Volazj kills (Heroic Ahn'kahet)
                    case 1508: // The Prophet Tharon'ja kills (Heroic Drak'Tharon Keep)
                    case 1509: // Cyanigosa kills (Heroic Violet Hold)
                    case 1510: // Gal'darah kills (Heroic Gundrak)
                    case 1511: // Sjonnir the Ironshaper kills (Heroic Halls of Stone)
                    case 1512: // Loken kills (Heroic Halls of Lightning)
                    case 1513: // Ley-Guardian Eregos kills (Heroic Oculus)
                    case 1514: // King Ymiron kills (Heroic Utgarde Pinnacle)
                    case 1515: // Mal'Ganis defeated (Heroic CoT: Stratholme)
                    case 1721: // Heroic: Archavon the Stone Watcher
                    case 1817: // The Culling of Time
                    case 1865: // Lockdown!
                        if (GetPlayer()->getDungeonDifficulty() >= InstanceDifficulty::DUNGEON_HEROIC)
                        {
                            UpdateCriteriaProgress(achievementCriteria, 1);
                        }
                        break;
                        ///\todo More complicated achievements: time limits, group size limits, other criteria...
                    case 1870: // Heroic: A Poke In The Eye
                        // Defeat Malygos on Heroic Difficulty with fewer than 21.
                    case 2056: // Volunteer Work
                        // Defeat Jedoga Shadowseeker in Ahn'kahet on Heroic Difficulty without killing any Twilight Volunteers.
                    case 1875: // Heroic: You Don't Have An Eternity
                        // Defeat Malygos in 6 minutes or less on Heroic Difficulty.
                    case 2185: // Heroic: Just Can't Get Enough
                        // Defeat Kel'Thuzad on Heroic Difficulty in Naxxramas while killing at least 18 abominations in his chamber.
                    case 1862: // Volazj's Quick Demise
                        // Defeat Herald Volazj in Ahn'kahet on Heroic Difficulty in 2 minutes or less.
                    case 2186: // The Immortal
                        // Within one raid lockout period, defeat every boss in Naxxramas on Heroic Difficulty without allowing any raid member to die during any of the boss encounters.
                    case 2038: // Respect Your Elders
                        // Defeat Elder Nadox in Ahn'kahet on Heroic Difficulty without killing any Ahn'kahar Guardians.
                    case 2183: // Heroic: Spore Loser
                        // Defeat Loatheb in Naxxramas on Heroic Difficulty without killing any spores.
                    case 1297: // Hadronox Denied
                        // Defeat Hadronox in Azjol-Nerub on Heroic Difficulty before he webs the top doors and prevents more creatures from spawning.
                    case 2177: // Heroic: And They Would All Go Down Together
                        // Defeat the 4 Horsemen in Naxxramas on Heroic Difficulty, ensuring that they all die within 15 seconds of each other.
                    case 1860: // Gotta Go!
                        // Defeat Anub'arak in Azjol-Nerub on Heroic Difficulty in 2 minutes or less.
                    case 2147: // Heroic: The Hundred Club
                        // Defeat Sapphiron on Heroic Difficulty in Naxxramas without any member of the raid having a frost resist value over 100.
                    case 1861: // The Party's Over
                        // Defeat Prince Taldaram in Ahn'kahet on Heroic Difficulty with less than 5 people.
                    case 2181: // Heroic: Subtraction
                        // Defeat Thaddius in Naxxramas on Heroic Difficulty with less than 21 people.
                    case 579: // Heroic: The Dedicated Few
                        // Defeat the bosses of Naxxramas with less than 21 people in the zone on Heroic Difficulty.
                    case 1296: // Watch Him Die
                        // Defeat Krik'thir the Gatewatcher in Azjol-Nerub on Heroic Difficulty while Watcher Gashra, Watcher Narjil and Watcher Silthik are still alive.
                    case 1589: // Heroic: Arachnophobia
                        // Kill Maexxna in Naxxramas within 20 minutes of Anub'Rekhan's death on Heroic Difficulty.
                    case 1857: // Heroic: Make Quick Werk Of Him
                        // Kill Patchwerk in Naxxramas in 3 minutes or less on Heroic Difficulty.
                    case 1877: // Heroic: Less Is More
                        // Defeat Sartharion the Onyx Guardian and the Twilight Drakes on Heroic Difficulty with fewer than 21.
                    case 1919: // On The Rocks
                        // Defeat Prince Keleseth in Utgarde Keep on Heroic Difficulty without shattering any Frost Tombs.
                    case 2036: // Intense Cold
                        // Defeat Keristrasza in The Nexus on Heroic Difficulty without allowing Intense Cold to reach more than two stacks.
                    case 2139: // Heroic: The Safety Dance
                        // Defeat Heigan the Unclean in Naxxramas on Heroic Difficulty without anyone in the raid dying.
                    case 2140: // Heroic: Momma Said Knock You Out
                        // Defeat Grand Widow Faerlina in Naxxramas on Heroic Difficulty without dispelling frenzy.
                    case 2150: // Split Personality
                        // Defeat Grand Magus Telestra in The Nexus on Heroic Difficulty after having killed her images within 5 seconds of each other during both splits.
                    case 2151: // Consumption Junction
                        // Defeat Trollgore in Drak'Tharon Keep on Heroic Difficulty before Consume reaches ten stacks.
                    case 2179: // Heroic: Shocking!
                        // Defeat Thaddius in Naxxramas on Heroic Difficulty without anyone in the raid crossing the negative and positive charges.
                    case 2037: // Chaos Theory
                        // Defeat Anomalus in The Nexus on Heroic Difficulty without destroying any Chaotic Rifts.
                    case 2039: // Better Off Dred
                        // Engage King Dred in Drak'Tharon Keep on Heroic Difficulty and slay 6 Drakkari Gutrippers or Drakkari Scytheclaw during his defeat.
                    case 2048: // Heroic: Gonna Go When the Volcano Blows
                        // Defeat Sartharion the Onyx Guardian on Heroic Difficulty without getting hit by Lava Strike.
                    case 2057: // Oh Novos!
                        // Defeat Novos the Summoner in Drak'Tharon Keep on Heroic Difficulty without allowing any undead minions to reach the floor.
                    case 1816: // Defenseless
                        // Defeat Cyanigosa in The Violet Hold without using Defense Control Crystals and with Prison Seal Integrity at 100% while in Heroic Difficulty.
                    case 2052: // Heroic: Twilight Assist
                        // With at least one Twilight Drake still alive, engage and defeat Sartharion the Onyx Guardian on Heroic Difficulty.
                    case 2053: // Heroic: Twilight Duo
                        // With at least two Twilight Drakes still alive, engage and defeat Sartharion the Onyx Guardian on Heroic Difficulty.
                    case 2041: // Dehydration
                        // Defeat Ichoron in the Violet Hold on Heroic Difficulty without allowing any Ichor Globules to merge.
                    case 2054: // Heroic: The Twilight Zone
                        // With all three Twilight Drakes still alive, engage and defeat Sartharion the Onyx Guardian on Heroic Difficulty.
                    case 1864: // What the Eck?
                        // Defeat Gal'darah in Gundrak on Heroic Difficulty while under the effects of Eck Residue.
                    case 2152: // Share The Love
                        // Defeat Gal'darah in Gundrak on Heroic Difficulty and have 5 unique party members get impaled throughout the fight.
                    case 2040: // Less-rabi
                        // Defeat Moorabi in Gundrak on Heroic Difficulty while preventing him from transforming into a mammoth at any point during the encounter.
                    case 2058: // Snakes. Why'd It Have To Be Snakes?
                        // Defeat Slad'ran in Gundrak on Heroic Difficulty without getting snake wrapped.
                    case 1866: // Good Grief
                        // Defeat the Maiden of Grief in the Halls of Stone on Heroic Difficulty in 1 minute or less.
                    case 2155: // Abuse the Ooze
                        // Defeat Sjonnir the Ironshaper in the Halls of Stone on Heroic Difficulty and kill 5 Iron Sludges during the encounter.
                    case 2154: // Brann Spankin' New
                        // Defeat the Tribunal of Ages encounter in the Halls of Stone on Heroic Difficulty without allowing Brann Bronzebeard to take any damage.
                    case 1867: // Timely Death
                        // Defeat Loken in the Halls of Lightning on Heroic Difficulty in 2 minutes or less.
                    case 1834: //Lightning Struck
                        // Defeat General Bjarngrim in the Halls of Lightning on Heroic Difficulty while he has a Temporary Electrical Charge.
                    case 2042: // Shatter Resistant
                        // Defeat Volkhan in the Halls of Lightning on Heroic Difficulty without allowing him to shatter more than 4 Brittle Golems.
                    case 1872: // Zombiefest!
                        // Kill 100 Risen Zombies in 1 minute in The Culling of Stratholme on Heroic Difficulty.
                    case 2043: // The Incredible Hulk
                        // Force Svala Sorrowgrave to kill a Scourge Hulk on Heroic Difficulty in Utgarde Pinnacle.
                    case 1873: // Lodi Dodi We Loves the Skadi
                        // Defeat Skadi the Ruthless in Utgarde Pinnacle on Heroic Difficulty within 3 minutes of starting the gauntlet event.
                    case 2156: // My Girl Loves to Skadi All the Time
                        // Defeat Skadi the Ruthless in Utgarde Pinnacle on Heroic Difficulty after having killed Grauf from 100% to dead in a single pass.
                    case 2157: // King's Bane
                        // Defeat King Ymiron in Utgarde Pinnacle on Heroic Difficulty without anyone in the party triggering Bane.
                    case 1871: // Experienced Drake Rider
                        // On three different visits to The Oculus, get credit for defeating Ley-Guardian Eregos while riding an Amber, Emerald, and Ruby drake on Heroic Difficulty.
                    case 1868: // Make It Count
                        // Defeat Ley-Guardian Eregos in The Oculus on Heroic Difficulty within 20 minutes of Drakos the Interrogator's death.
                    case 2044: // Ruby Void
                        // Defeat Ley-Guardian Eregos in The Oculus on Heroic Difficulty without anyone in your party using a Ruby Drake.
                    case 2045: // Emerald Void
                        // Defeat Ley-Guardian Eregos in The Oculus on Heroic Difficulty without anyone in your party using an Emerald Drake.
                    case 2046: // Amber Void
                        // Defeat Ley-Guardian Eregos in The Oculus on Heroic Difficulty without anyone in your party using an Amber Drake.
                        break;
                    default:
                        if (!IS_INSTANCE(GetPlayer()->GetMapId()) || (GetPlayer()->getDungeonDifficulty() == InstanceDifficulty::DUNGEON_NORMAL))
                        {
                            // already tested heroic achievements above, the rest should be normal or non-dungeon
                            UpdateCriteriaProgress(achievementCriteria, 1);
                        }
                        break;
                }
                break;
            case ACHIEVEMENT_CRITERIA_TYPE_EQUIP_EPIC_ITEM:
                // Achievement ID:556 description Equip an epic item in every slot with a minimum item level of 213.
                // "213" value not found in achievement or criteria entries (dbc), have to hard-code it here? :(
                // Achievement ID:557 description Equip a superior item in every slot with a minimum item level of 187.
                // "187" value not found in achievement or criteria entries (dbc), have to hard-code it here? :(
                // AchievementType for both Achievement ID:556 (Equip epic items) and ID:557 (Equip superior items)
                //    is the same (47) ACHIEVEMENT_CRITERIA_TYPE_EQUIP_EPIC_ITEM
                // Going to send item slot in miscvalue1 and item quality in miscvalue2 when calling UpdateAchievementCriteria.
                if ((achievementCriteria->referredAchievement == 556) && (miscvalue2 == ITEM_QUALITY_EPIC_PURPLE))
                {
                    SetCriteriaProgress(achievementCriteria, 1);
                }
                else if ((achievementCriteria->referredAchievement == 557) && (miscvalue2 == ITEM_QUALITY_RARE_BLUE))
                {
                    SetCriteriaProgress(achievementCriteria, 1);
                }
                break;
            case ACHIEVEMENT_CRITERIA_TYPE_DO_EMOTE:
                // emote matches, check the achievement target ... (if required)
                switch (achievement->ID)
                {
                    case 1206: // To All The Squirrels I've Loved Before
                    {
                        // requires a target
                        if (Unit* pUnit = GetPlayer()->getWorldMap()->getUnit(selectedGUID))
                        {
                            uint32_t ent = pUnit->getEntry();
                            if ((ent == 1412 && achievementCriteria->index == 1)      // Squirrel
                                || (ent == 25679 && achievementCriteria->index == 2)  // Steam Frog
                                || (ent == 25677 && achievementCriteria->index == 3)  // Borean Frog
                                || (ent == 6368 && achievementCriteria->index == 4)   // Cat
                                || (ent == 620 && achievementCriteria->index == 5)    // Chicken
                                || (ent == 2442 && achievementCriteria->index == 6)   // Cow
                                || (ent == 6827 && achievementCriteria->index == 7)   // Crab
                                || (ent == 883 && achievementCriteria->index == 8)    // Deer
                                || (ent == 19665 && achievementCriteria->index == 9)  // Ewe
                                || (ent == 890 && achievementCriteria->index == 10)   // Fawn
                                || (ent == 13321 && achievementCriteria->index == 11) // Frog
                                || (ent == 4166 && achievementCriteria->index == 12)  // Gazelle
                                || (ent == 5951 && achievementCriteria->index == 13)  // Hare
                                || (ent == 9600 && achievementCriteria->index == 14)  // Parrot
                                || (ent == 721 && achievementCriteria->index == 15)   // Rabbit
                                || (ent == 2098 && achievementCriteria->index == 16)  // Ram
                                || (ent == 1933 && achievementCriteria->index == 17)  // Sheep
                                || (ent == 17467 && achievementCriteria->index == 18) // Skunk
                                || (ent == 10685 && achievementCriteria->index == 19) // Swine
                                || (ent == 1420 && achievementCriteria->index == 20)  // Toad
                                || (ent == 2620 && achievementCriteria->index == 21)) // Prairie Dog
                            {
                                SetCriteriaProgress(achievementCriteria, 1);
                            }
                        }
                    } break;
                    case 2557: // To All The Squirrels Who Shared My Life
                    {
                        // requires a target
                        if (Unit* pUnit = GetPlayer()->getWorldMap()->getUnit(selectedGUID))
                        {
                            uint32_t ent = pUnit->getEntry();
                            if ((ent == 29328 && achievementCriteria->index == 1)      // Arctic Hare
                                || (ent == 31685 && achievementCriteria->index == 2)   // Borean Marmot
                                || (ent == 28407 && achievementCriteria->index == 3)   // Fjord Penguin
                                || (ent == 24746 && achievementCriteria->index == 4)   // Fjord Turkey
                                || (ent == 32498 && achievementCriteria->index == 5)   // Glacier Penguin (not in db?)
                                || (ent == 31889 && achievementCriteria->index == 6)   // Grizzly Squirrel
                                || (ent == 6653 && achievementCriteria->index == 7)    // Huge Toad
                                || (ent == 9700 && achievementCriteria->index == 8)    // Lava Crab
                                || (ent == 31890 && achievementCriteria->index == 9)   // Mountain Skunk
                                || (ent == 26503 && achievementCriteria->index == 10)  // Scalawag Frog
                                || (ent == 28093 && achievementCriteria->index == 11)  // Sholazar Tickbird
                                || (ent == 28440 && achievementCriteria->index == 12)) // Tundra Penguin
                            {
                                SetCriteriaProgress(achievementCriteria, 1);
                            }
                        }
                    } break;
                    case 247: // Make Love, Not Warcraft
                    {
                        Player* pTarget = sObjectMgr.GetPlayer((uint32_t)selectedGUID);
                        if (pTarget && pTarget->isDead() && isHostile(pTarget, GetPlayer()))
                        {
                            UpdateCriteriaProgress(achievementCriteria, 1);
                        }
                    }
                    break;
                    case 293: ///\todo Disturbing the Peace
                        // While wearing 3 pieces of Brewfest clothing, get completely smashed and dance in Dalaran.
                        break;
                    case 1280: ///\todo Flirt With Disaster
                        // Get completely smashed, put on your best perfume, throw a handful of rose petals on Jeremiah Payson and then kiss him. You'll regret it in the morning.
                        break;
                    case 1279: ///\todo Flirt With Disaster
                        // Get completely smashed, put on your best perfume, throw a handful of rose petals on Sraaz and then kiss him. You'll regret it in the morning.
                        break;
                    case 1690: ///\todo A Frosty Shake
                        // During the Feast of Winter Veil, use your Winter Veil Disguise kit to become a snowman and then dance with another snowman in Dalaran.
                        break;
                    case 1704: ///\todo I Pitied The Fool
                        // Pity the Love Fool in the locations specified below.
                        // Wintergrasp (achievementCriteria->index==1)
                        // Battle Ring of Gurubashi Arena (achievementCriteria->index==2)
                        // Arathi Basin Blacksmith (achievementCriteria->index==3)
                        // The Culling of Stratholme (achievementCriteria->index==4)
                        // Naxxramas (achievementCriteria->index==5)
                        break;

                        // Statistics for emotes
                    case 1042: // Number of Hugs
                    case 1045: // Total cheers
                    case 1047: // Total facepalms
                    case 1065: // Total waves
                    case 1066: // Total times LOL'd (laugh, guffaw, rofl, giggle, chuckle)
                    case 1067: // Total times playing world's smallest violin
                        UpdateCriteriaProgress(achievementCriteria, 1);
                        break;
                    default:
                        break;
                }
                break;
            case ACHIEVEMENT_CRITERIA_TYPE_KILLING_BLOW:
                // miscvalue1 contain Map ID
                switch (achievementCriteria->referredAchievement)
                {
                    case 231:
                        if (((miscvalue1 == 30) && (achievementCriteria->index == 1))       // Alterac Valley
                            || ((miscvalue1 == 529) && (achievementCriteria->index == 2))   // Arathi Basin
                            || ((miscvalue1 == 566) && (achievementCriteria->index == 3))   // Eye of the Storm
                            || ((miscvalue1 == 489) && (achievementCriteria->index == 4))   // Warsong Gulch
                            || ((miscvalue1 == 607) && (achievementCriteria->index == 5)))  // Strand of the Ancients
                        {
                            UpdateCriteriaProgress(achievementCriteria, 1);
                        }
                        break;
                    case 233: ///\todo Berserking killing blow
                        break;
                    case 1487: // Total Killing Blows
                        UpdateCriteriaProgress(achievementCriteria, 1);
                        break;
                    case 1488:
                        if (((miscvalue1 == 0) && (achievementCriteria->index == 1))        // Eastern Kingdoms
                            || ((miscvalue1 == 1) && (achievementCriteria->index == 2))     // Kalimdor
                            || ((miscvalue1 == 530) && (achievementCriteria->index == 3))   // Burning Crusade Areas
                            || ((miscvalue1 == 571) && (achievementCriteria->index == 4)))  // Northrend
                        {
                            UpdateCriteriaProgress(achievementCriteria, 1);
                        }
                        break;
                    case 1490:
                        if (((miscvalue1 == 559) && (achievementCriteria->index == 1))      // Nagrand Arena
                            || ((miscvalue1 == 562) && (achievementCriteria->index == 2))   // Blade's Edge Arena
                            || ((miscvalue1 == 572) && (achievementCriteria->index == 3))   // Ruins of Lordaeron
                            || ((miscvalue1 == 617) && (achievementCriteria->index == 4))   // Dalaran Sewers
                            || ((miscvalue1 == 618) && (achievementCriteria->index == 5)))  // Ring of Valor
                        {
                            UpdateCriteriaProgress(achievementCriteria, 1);
                        }
                        break;
                    case 1491:
                        if (((miscvalue1 == 30) && (achievementCriteria->index == 1))       // Alterac Valley
                            || ((miscvalue1 == 529) && (achievementCriteria->index == 2))   // Arathi Basin
                            || ((miscvalue1 == 489) && (achievementCriteria->index == 3))   // Warsong Gulch
                            || ((miscvalue1 == 566) && (achievementCriteria->index == 4))   // Eye of the Storm
                            || ((miscvalue1 == 607) && (achievementCriteria->index == 5)))  // Strand of the Ancients
                        {
                            UpdateCriteriaProgress(achievementCriteria, 1);
                        }
                        break;
                    case 1492: ///\todo 2v2 Arena Killing Blows
                        break;
                    case 1493: ///\todo 3v3 Arena Killing Blows
                        break;
                    case 1494: ///\todo 5v5 Arena Killing Blows
                        break;
                    case 1495: // Alterac Valley Killing Blows
                        if (miscvalue1 == 30)
                        {
                            UpdateCriteriaProgress(achievementCriteria, 1);
                        }
                        break;
                    case 1496: // Arathi Basin Killing Blows
                        if (miscvalue1 == 529)
                        {
                            UpdateCriteriaProgress(achievementCriteria, 1);
                        }
                        break;
                    case 1497: // Warsong Gulch Killing Blows
                        if (miscvalue1 == 489)
                        {
                            UpdateCriteriaProgress(achievementCriteria, 1);
                        }
                        break;
                    case 1498: // Eye of the Storm Killing Blows
                        if (miscvalue1 == 566)
                        {
                            UpdateCriteriaProgress(achievementCriteria, 1);
                        }
                        break;
                    case 1499: // Strand of the Ancients Killing Blows
                        if (miscvalue1 == 607)
                        {
                            UpdateCriteriaProgress(achievementCriteria, 1);
                        }
                        break;
                    case 2148: ///\todo Deliver a killing blow to a Scion of Eternity while riding on a hover disk
                        break;
                    case 2149: ///\todo Deliver a killing blow to a Scion of Eternity while riding on a hover disk
                        break;
                    default:
                        break;
                }
                break;
            case ACHIEVEMENT_CRITERIA_TYPE_USE_ITEM: // itemID in miscvalue1
                switch (achievementCriteria->referredAchievement)
                {
                    case 1281: // Shoot off 10 Red Rocket Clusters in 25 seconds or less
                    case 1552: // Shoot off 10 Festival Firecrackers in 30 seconds or less
                    case 1696: // Shoot off 10 Love Rockets in 20 seconds or less
                    case 1781: // Get 10 critters in 3 minutes
                    case 1791: // Hearthstone with kid out
                        break;
                    default:
                        UpdateCriteriaProgress(achievementCriteria, 1);
                        break;
                }
                break;
            case ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE_TYPE:
                // Total NPC kills, Kill an NPC that yields XP, Beasts, Critters killed, Demons, Dragonkin ...
                // miscvalue1 = killed creature high GUID
                // miscvalue2 = killed creature low GUID
            {
                uint64_t crGUID = miscvalue1;
                crGUID <<= 32; // shift to high 32-bits
                crGUID += miscvalue2;
                Unit* pUnit = GetPlayer()->getWorldMap()->getUnit(crGUID);
                if (pUnit)
                {
                    uint32_t crType = UNIT_TYPE_NONE;
                    bool crTotem = false;
                    bool yieldXP = CalculateXpToGive(pUnit, GetPlayer()) > 0;
                    if (pUnit->isCreature())
                    {
                        crTotem = pUnit->isTotem();
                        crType = static_cast<Creature*>(pUnit)->GetCreatureProperties()->Type;
                        if ((achievementCriteria->ID == 4944)                                            // Total NPC kills              refAch==1197
                            || ((achievementCriteria->ID == 4946) && (yieldXP))                          // Kill an NPC that yields XP   refAch==1198
                            || ((achievementCriteria->ID == 4948) && (crType == UNIT_TYPE_BEAST))        // Beasts                       refAch== 107
                            || ((achievementCriteria->ID == 4958) && (crType == UNIT_TYPE_CRITTER))      // Critters killed              refAch== 107
                            || ((achievementCriteria->ID == 4949) && (crType == UNIT_TYPE_DEMON))        // Demons                       refAch== 107
                            || ((achievementCriteria->ID == 4950) && (crType == UNIT_TYPE_DRAGONKIN))    // Dragonkin                    refAch== 107
                            || ((achievementCriteria->ID == 4951) && (crType == UNIT_TYPE_ELEMENTAL))    // Elemental                    refAch== 107
                            || ((achievementCriteria->ID == 4952) && (crType == UNIT_TYPE_GIANT))        // Giant                        refAch== 107
                            || ((achievementCriteria->ID == 4953) && (crType == UNIT_TYPE_HUMANOID))     // Humanoid                     refAch== 107
                            || ((achievementCriteria->ID == 4954) && (crType == UNIT_TYPE_MECHANICAL))   // Mechanical                   refAch== 107
                            || ((achievementCriteria->ID == 4955) && (crType == UNIT_TYPE_UNDEAD))       // Undead                       refAch== 107
                            || ((achievementCriteria->ID == 4956) && (crType == UNIT_TYPE_NONE))         // Unspecified                  refAch== 107
                            || ((achievementCriteria->ID == 4957) && (crTotem)))                         // Totems                       refAch== 107
                        {
                            UpdateCriteriaProgress(achievementCriteria, 1);
                        }
                    }
                }
            }
            break;
            case ACHIEVEMENT_CRITERIA_TYPE_FALL_WITHOUT_DYING:
                // fall distance (>=65) has been checked before UpdateAchievementCriteria() call, but it is sent in miscvalue1 just in case "they" add more...
                if (achievement->ID == 1260)
                {
                    // Fall 65 yards without dying while completely smashed during the Brewfest Holiday.
                    if (miscvalue2 == DRUNKEN_SMASHED)
                    {
                        // drunken state, "completely smashed"
                        ///\todo Check if it is during the Brewfest Holiday ...
                        UpdateCriteriaProgress(achievementCriteria, 1);
                    }
                }
                else
                {
                    // achievement->ID==964 // Fall 65 yards without dying.
                    UpdateCriteriaProgress(achievementCriteria, 1);
                }
                break;

            //End of Achievement List
            default:
                return;
        }
        CompletedCriteria(achievementCriteria);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Updates all achievement criteria of the specified type.
/// \brief This is only called from CheckAllAchievementCriteria(), during player login
void AchievementMgr::UpdateAchievementCriteria(AchievementCriteriaTypes type)
{
    if (m_player->getSession()->HasGMPermissions() && worldConfig.gm.disableAchievements)
        return;

    AchievementCriteriaEntryList const & achievementCriteriaList = sObjectMgr.GetAchievementCriteriaByType(type);
    for (AchievementCriteriaEntryList::const_iterator i = achievementCriteriaList.begin(); i != achievementCriteriaList.end(); ++i)
    {
        DBC::Structures::AchievementCriteriaEntry const* achievementCriteria = (*i);

        auto achievement = sAchievementStore.LookupEntry(achievementCriteria->referredAchievement);
        if (!achievement  //|| IsCompletedCriteria(achievementCriteria)
            || (achievement->flags & ACHIEVEMENT_FLAG_COUNTER)
            || (achievement->factionFlag == ACHIEVEMENT_FACTION_FLAG_HORDE && !m_player->isTeamHorde())
            || (achievement->factionFlag == ACHIEVEMENT_FACTION_FLAG_ALLIANCE && !m_player->isTeamAlliance()))
        {
            continue;
        }

        if (!canCompleteCriteria(achievementCriteria, type, GetPlayer()))
            continue;

        // Check scripted criteria checks
        const auto scriptResult = sScriptMgr.callScriptedAchievementCriteriaCanComplete(achievementCriteria->ID, GetPlayer(), nullptr);
        if (!scriptResult)
            continue;

        switch (type)
        {
            //Start of Achievement List
            case ACHIEVEMENT_CRITERIA_TYPE_REACH_LEVEL:
                SetCriteriaProgress(achievementCriteria, GetPlayer()->getLevel());
                break;
            case ACHIEVEMENT_CRITERIA_TYPE_EXPLORE_AREA:
            case ACHIEVEMENT_CRITERIA_TYPE_LEARN_SPELL:
                SetCriteriaProgress(achievementCriteria, 1);
                break;
            case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_ACHIEVEMENT:
                SetCriteriaProgress(achievementCriteria, 1, true);
                break;
            case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST_COUNT:
                SetCriteriaProgress(achievementCriteria, (int32_t)GetPlayer()->getFinishedQuests().size());
                break;
            case ACHIEVEMENT_CRITERIA_TYPE_GAIN_REPUTATION:
                SetCriteriaProgress(achievementCriteria, GetPlayer()->getFactionStanding(achievementCriteria->gain_reputation.factionID));
                break;
            case ACHIEVEMENT_CRITERIA_TYPE_GAIN_EXALTED_REPUTATION:
                SetCriteriaProgress(achievementCriteria, GetPlayer()->getExaltedCount());
                break;
            case ACHIEVEMENT_CRITERIA_TYPE_REACH_SKILL_LEVEL:
                SetCriteriaProgress(achievementCriteria, GetPlayer()->getSkillLineCurrent(static_cast<uint16_t>(achievementCriteria->reach_skill_level.skillID), true));
                break;
            case ACHIEVEMENT_CRITERIA_TYPE_LEARN_SKILL_LEVEL:
                SetCriteriaProgress(achievementCriteria, GetPlayer()->getSkillLineMax(static_cast<uint16_t>(achievementCriteria->learn_skill_level.skillID) / 75U));
                break;
            case ACHIEVEMENT_CRITERIA_TYPE_BUY_BANK_SLOT:
                SetCriteriaProgress(achievementCriteria, GetPlayer()->getBankSlots());
                break;
            case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUESTS_IN_ZONE:
            {
                uint32_t qcinzone = 0;
                for (uint32_t finishedQuestId : GetPlayer()->getFinishedQuests())
                {
                    QuestProperties const* qst = sMySQLStore.getQuestProperties(finishedQuestId);
                    if (qst && qst->zone_id == achievementCriteria->complete_quests_in_zone.zoneID)
                        ++qcinzone;
                }
                SetCriteriaProgress(achievementCriteria, qcinzone);
            } break;
            case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST:
            {
                uint32_t completed = 0;
                for (uint32_t finishedQuestId : GetPlayer()->getFinishedQuests())
                {
                    if (QuestProperties const* qst = sMySQLStore.getQuestProperties(finishedQuestId))
                        if (finishedQuestId == achievementCriteria->complete_quest.questID)
                            ++completed;
                }

                SetCriteriaProgress(achievementCriteria, completed);
            } break;
            case ACHIEVEMENT_CRITERIA_TYPE_NUMBER_OF_MOUNTS:
            {
                // achievementCriteria field4 = 777 for mounts, 778 for companion pets
                SpellSet::iterator sl = GetPlayer()->mSpells.begin();
                uint32_t nm = 0;
                while (sl != GetPlayer()->mSpells.end())
                {
                    SpellInfo const* sp = sSpellMgr.getSpellInfo(*sl);
                    if (achievementCriteria->number_of_mounts.unknown == 777 && sp && sp->getMechanicsType() == MECHANIC_MOUNTED)
                    {
                        // mount spell
                        ++nm;
                    }
                    else if (achievementCriteria->number_of_mounts.unknown == 778 && sp && (sp->getEffect(0) == SPELL_EFFECT_SUMMON))
                    {
                        // Companion pet? Make sure it's a companion pet, not some other summon-type spell
                        // temporary solution since spell description is no longer loaded -Appled
                        const auto creatureEntry = sp->getEffectMiscValue(0);
                        auto creatureProperties = sMySQLStore.getCreatureProperties(creatureEntry);
                        if (creatureProperties != nullptr && creatureProperties->Type == UNIT_TYPE_NONCOMBAT_PET)
                            ++nm;
                    }
                    ++sl;
                }
                SetCriteriaProgress(achievementCriteria, nm);
            } break;
            //End of Achievement List
            default:
                break;
        }
        CompletedCriteria(achievementCriteria);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
/// \return True if the criteria has been completed otherwise false (error...)
bool AchievementMgr::IsCompletedCriteria(DBC::Structures::AchievementCriteriaEntry const* achievementCriteria)
{
    if (!achievementCriteria)
    {
        return false;
    }

    auto achievement = sAchievementStore.LookupEntry(achievementCriteria->referredAchievement);
    if (achievement == nullptr)
    {
        return false;
    }

    if (achievement->flags & ACHIEVEMENT_FLAG_COUNTER)
    {
        return false;
    }

    if (achievement->flags & (ACHIEVEMENT_FLAG_REALM_FIRST_REACH | ACHIEVEMENT_FLAG_REALM_FIRST_KILL))
    {
        if (sObjectMgr.allCompletedAchievements.find(achievement->ID) != sObjectMgr.allCompletedAchievements.end())
        {
            return false;
        }
    }

    CriteriaProgressMap::iterator itr = m_criteriaProgress.find(achievementCriteria->ID);
    if (itr == m_criteriaProgress.end())
    {
        return false;
    }

    CriteriaProgress* progress = itr->second;

    // 0 or negative, not completed.
    if (progress->counter < 1)
    {
        return false;
    }

    uint32_t progresscounter = (uint32_t)progress->counter;
    switch (achievementCriteria->requiredType)
    {
        case ACHIEVEMENT_CRITERIA_TYPE_REACH_LEVEL:
            if ((achievement->ID == 467 && GetPlayer()->getClass() != SHAMAN) ||
                (achievement->ID == 466 && GetPlayer()->getClass() != DRUID) ||
                (achievement->ID == 465 && GetPlayer()->getClass() != PALADIN) ||
                (achievement->ID == 464 && GetPlayer()->getClass() != PRIEST) ||
                (achievement->ID == 463 && GetPlayer()->getClass() != WARLOCK) ||
                (achievement->ID == 462 && GetPlayer()->getClass() != HUNTER) ||
                (achievement->ID == 461 && GetPlayer()->getClass() != DEATHKNIGHT) ||
                (achievement->ID == 460 && GetPlayer()->getClass() != MAGE) ||
                (achievement->ID == 459 && GetPlayer()->getClass() != WARRIOR) ||
                (achievement->ID == 458 && GetPlayer()->getClass() != ROGUE) ||
                (achievement->ID == 1404 && GetPlayer()->getRace() != RACE_GNOME) ||
                (achievement->ID == 1405 && GetPlayer()->getRace() != RACE_BLOODELF) ||
                (achievement->ID == 1406 && GetPlayer()->getRace() != RACE_DRAENEI) ||
                (achievement->ID == 1407 && GetPlayer()->getRace() != RACE_DWARF) ||
                (achievement->ID == 1408 && GetPlayer()->getRace() != RACE_HUMAN) ||
                (achievement->ID == 1409 && GetPlayer()->getRace() != RACE_NIGHTELF) ||
                (achievement->ID == 1410 && GetPlayer()->getRace() != RACE_ORC) ||
                (achievement->ID == 1411 && GetPlayer()->getRace() != RACE_TAUREN) ||
                (achievement->ID == 1412 && GetPlayer()->getRace() != RACE_TROLL) ||
                (achievement->ID == 1413 && GetPlayer()->getRace() != RACE_UNDEAD))
            {
                return false;
            }
            return progresscounter >= achievementCriteria->reach_level.level;
            break;
        case ACHIEVEMENT_CRITERIA_TYPE_LOOT_ITEM:
        case ACHIEVEMENT_CRITERIA_TYPE_OWN_ITEM:
            return progresscounter >= achievementCriteria->loot_item.itemCount;
        case ACHIEVEMENT_CRITERIA_TYPE_LOOT_MONEY:
            return progresscounter >= achievementCriteria->loot_money.goldInCopper;
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST_COUNT:
            return progresscounter >= achievementCriteria->complete_quest_count.totalQuestCount;
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUESTS_IN_ZONE:
            return progresscounter >= achievementCriteria->complete_quests_in_zone.questCount;
        case ACHIEVEMENT_CRITERIA_TYPE_QUEST_REWARD_GOLD:
            return progresscounter >= achievementCriteria->quest_reward_money.goldInCopper;
        case ACHIEVEMENT_CRITERIA_TYPE_GAIN_REPUTATION:
            return progresscounter >= achievementCriteria->gain_reputation.reputationAmount;
        case ACHIEVEMENT_CRITERIA_TYPE_GAIN_EXALTED_REPUTATION:
            return progresscounter >= achievementCriteria->gain_exalted_reputation.numberOfExaltedFactions;
        case ACHIEVEMENT_CRITERIA_TYPE_NUMBER_OF_MOUNTS:
            return progresscounter >= achievementCriteria->number_of_mounts.mountCount;
        case ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET:
        case ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET2:
            return progresscounter >= achievementCriteria->be_spell_target.spellCount;
        case ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE:
            return progresscounter >= achievementCriteria->kill_creature.creatureCount;
        case ACHIEVEMENT_CRITERIA_TYPE_REACH_SKILL_LEVEL:
            return progresscounter >= achievementCriteria->reach_skill_level.skillLevel;
        case ACHIEVEMENT_CRITERIA_TYPE_LEARN_SKILL_LEVEL:
            return progresscounter >= achievementCriteria->learn_skill_level.skillLevel;
        case ACHIEVEMENT_CRITERIA_TYPE_USE_ITEM:
            return progresscounter >= achievementCriteria->use_item.itemCount;
        case ACHIEVEMENT_CRITERIA_TYPE_USE_GAMEOBJECT:
            return progresscounter >= achievementCriteria->use_gameobject.useCount;
        case ACHIEVEMENT_CRITERIA_TYPE_BUY_BANK_SLOT:
            return progresscounter >= achievementCriteria->buy_bank_slot.numberOfSlots;
        case ACHIEVEMENT_CRITERIA_TYPE_EARN_HONORABLE_KILL:
            return progresscounter >= achievementCriteria->honorable_kill.killCount;
        case ACHIEVEMENT_CRITERIA_TYPE_HONORABLE_KILL_AT_AREA:
            return progresscounter >= achievementCriteria->honorable_kill_at_area.killCount;
        case ACHIEVEMENT_CRITERIA_TYPE_HK_CLASS:
            return progresscounter >= achievementCriteria->hk_class.count;
        case ACHIEVEMENT_CRITERIA_TYPE_HK_RACE:
            return progresscounter >= achievementCriteria->hk_race.count;
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_ACHIEVEMENT:
            return m_completedAchievements.find(achievementCriteria->complete_achievement.linkedAchievement) != m_completedAchievements.end();

            // These achievements only require counter to be 1 (or higher)
        case ACHIEVEMENT_CRITERIA_TYPE_EXPLORE_AREA:
        case ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST:
        case ACHIEVEMENT_CRITERIA_TYPE_LEARN_SPELL:
        case ACHIEVEMENT_CRITERIA_TYPE_EQUIP_ITEM:
        case ACHIEVEMENT_CRITERIA_TYPE_EQUIP_EPIC_ITEM:
        case ACHIEVEMENT_CRITERIA_TYPE_DO_EMOTE:
        case ACHIEVEMENT_CRITERIA_TYPE_VISIT_BARBER_SHOP:
        case ACHIEVEMENT_CRITERIA_TYPE_FALL_WITHOUT_DYING:
            return progresscounter >= 1;

            // unknown or need to be finished:
        default:
            if (achievementCriteria->raw.field4 > 0)
            {
                return progresscounter >= achievementCriteria->raw.field4;
            }
            break;
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////
/// If achievement criteria has been completed, checks whether to complete the achievement too.
void AchievementMgr::CompletedCriteria(DBC::Structures::AchievementCriteriaEntry const* criteria)
{
    if (!IsCompletedCriteria(criteria))
    {
        return;
    }

    auto achievement = sAchievementStore.LookupEntry(criteria->referredAchievement);
    if (achievement == nullptr)
    {
        return;
    }

    if (criteria->completionFlag & ACHIEVEMENT_CRITERIA_COMPLETE_FLAG_ALL || GetAchievementCompletionState(achievement) == ACHIEVEMENT_COMPLETED_COMPLETED_NOT_STORED)
    {
        CompletedAchievement(achievement);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Returns the completion state of the achievement.
/// \brief ACHIEVEMENT_COMPLETED_COMPLETED_STORED: has been completed and stored already.
/// ACHIVEMENT_COMPLETED_COMPLETED_NOT_STORED: has been completed but not stored yet.
/// ACHIEVEMENT_COMPLETED_NONE: has not been completed yet
AchievementCompletionState AchievementMgr::GetAchievementCompletionState(DBC::Structures::AchievementEntry const* entry)
{
    if (m_completedAchievements.find(entry->ID) != m_completedAchievements.end())
    {
        return ACHIEVEMENT_COMPLETED_COMPLETED_STORED;
    }

    uint32_t completedCount = 0;
    bool foundOutstanding = false;
    for (uint32_t rowId = 0; rowId < sAchievementCriteriaStore.GetNumRows(); ++rowId)
    {
        auto criteria = sAchievementCriteriaStore.LookupEntry(rowId);
        if (criteria == nullptr || criteria->referredAchievement != entry->ID)
        {
            continue;
        }

        if (IsCompletedCriteria(criteria) && criteria->completionFlag & ACHIEVEMENT_CRITERIA_COMPLETE_FLAG_ALL)
        {
            return ACHIEVEMENT_COMPLETED_COMPLETED_NOT_STORED;
        }

        if (!IsCompletedCriteria(criteria))
        {
            foundOutstanding = true;
        }
        else
        {
            ++completedCount;
        }
    }
    if (!foundOutstanding)
    {
        return ACHIEVEMENT_COMPLETED_COMPLETED_NOT_STORED;
    }

    if ((entry->count > 1) && (completedCount >= entry->count))
    {
        return ACHIEVEMENT_COMPLETED_COMPLETED_NOT_STORED;
    }
    return ACHIEVEMENT_COMPLETED_NONE;
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Sets progress of the achievement criteria.
/// \brief If relative argument is true, this behaves the same as UpdateCriteriaProgress
void AchievementMgr::SetCriteriaProgress(DBC::Structures::AchievementCriteriaEntry const* entry, int32_t newValue, bool /*relative*/)
{
    CriteriaProgress* progress = nullptr;

    if (m_criteriaProgress.find(entry->ID) == m_criteriaProgress.end())
    {
        if (newValue < 1)
        {
            return;
        }
        progress = new CriteriaProgress(entry->ID, newValue);
        m_criteriaProgress[entry->ID] = progress;
    }
    else
    {
        progress = m_criteriaProgress[entry->ID];
        if (progress->counter == static_cast<uint32_t>(newValue))
        {
            return;
        }
        progress->counter = newValue;
    }
    if (progress->counter > 0)
    {
        // Send update only if criteria is started (counter > 0)
        SendCriteriaUpdate(progress);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Updates progress of the achievement criteria.
/// \brief updateByValue is added to the current progress counter
void AchievementMgr::UpdateCriteriaProgress(DBC::Structures::AchievementCriteriaEntry const* entry, int32_t updateByValue)
{
    CriteriaProgress* progress = nullptr;

    if (m_criteriaProgress.find(entry->ID) == m_criteriaProgress.end())
    {
        if (updateByValue < 1)
        {
            return;
        }
        progress = new CriteriaProgress(entry->ID, updateByValue);
        m_criteriaProgress[entry->ID] = progress;
    }
    else
    {
        progress = m_criteriaProgress[entry->ID];
        progress->counter += updateByValue;
    }
    if (progress->counter > 0)
    {
        SendCriteriaUpdate(progress);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Completes the achievement for the player.
void AchievementMgr::CompletedAchievement(DBC::Structures::AchievementEntry const* achievement)
{
    if (achievement->flags & ACHIEVEMENT_FLAG_COUNTER || m_completedAchievements.find(achievement->ID) != m_completedAchievements.end())
    {
        return;
    }

    if (ShowCompletedAchievement(achievement->ID, GetPlayer()))
    {
        SendAchievementEarned(achievement);
    }
    m_completedAchievements[achievement->ID] = time(nullptr);

    sObjectMgr.allCompletedAchievements.insert(achievement->ID);
    UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_ACHIEVEMENT);

    // check for reward
    GiveAchievementReward(achievement);
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Sends all achievement data to the player. Also used for achievement inspection.
#if VERSION_STRING >= Cata

struct VisibleAchievementPred
{
    bool operator()(CompletedAchievementMap::value_type const& val)
    {
        auto achievement = sAchievementStore.LookupEntry(val.first);
        return achievement && !(achievement->flags & ACHIEVEMENT_FLAG_HIDDEN);
    }
};

void AchievementMgr::sendAllAchievementData(Player* player)
{
    DBC::Structures::AchievementCriteriaEntry const* acEntry;

    VisibleAchievementPred isVisible;

    size_t numCriteria = m_criteriaProgress.size();
    size_t numAchievements = std::count_if(m_completedAchievements.begin(), m_completedAchievements.end(), isVisible);

    ByteBuffer criteriaData(m_criteriaProgress.size() * (4 + 4 + 4 + 4 + 8 + 8));
    ObjectGuid guid = m_player->getGuid();
    ObjectGuid counter;

    WorldPacket data(SMSG_ALL_ACHIEVEMENT_DATA, 4 + numAchievements * (4 + 4) + 4 + numCriteria * (4 + 4 + 4 + 4 + 8 + 8));
    data.writeBits(numCriteria, 21);

    for (auto progressIter : m_criteriaProgress)
    {
        acEntry = sAchievementCriteriaStore.LookupEntry(progressIter.first);

        if (!acEntry)
            continue;

        if (!sAchievementStore.LookupEntry(acEntry->referredAchievement))
            continue;

        counter = uint64_t(progressIter.second->counter);

        data.writeBit(guid[4]);
        data.writeBit(counter[3]);
        data.writeBit(guid[5]);
        data.writeBit(counter[0]);
        data.writeBit(counter[6]);
        data.writeBit(guid[3]);
        data.writeBit(guid[0]);
        data.writeBit(counter[4]);
        data.writeBit(guid[2]);
        data.writeBit(counter[7]);
        data.writeBit(guid[7]);
        data.writeBits(0u, 2);
        data.writeBit(guid[6]);
        data.writeBit(counter[2]);
        data.writeBit(counter[1]);
        data.writeBit(counter[5]);
        data.writeBit(guid[1]);

        criteriaData.WriteByteSeq(guid[3]);
        criteriaData.WriteByteSeq(counter[5]);
        criteriaData.WriteByteSeq(counter[6]);
        criteriaData.WriteByteSeq(guid[4]);
        criteriaData.WriteByteSeq(guid[6]);
        criteriaData.WriteByteSeq(counter[2]);
        criteriaData << uint32_t(0);    // timer 2
        criteriaData.WriteByteSeq(guid[2]);

        criteriaData << uint32_t(progressIter.first);   // criteria id
        criteriaData.WriteByteSeq(guid[5]);
        criteriaData.WriteByteSeq(counter[0]);
        criteriaData.WriteByteSeq(counter[3]);
        criteriaData.WriteByteSeq(counter[1]);
        criteriaData.WriteByteSeq(counter[4]);
        criteriaData.WriteByteSeq(guid[0]);
        criteriaData.WriteByteSeq(guid[7]);
        criteriaData.WriteByteSeq(counter[7]);
        criteriaData << uint32_t(0); // timer 1
        criteriaData.appendPackedTime(progressIter.second->date);   // criteria date
        criteriaData.WriteByteSeq(guid[1]);
    }

    data.writeBits(m_completedAchievements.size(), 23);
    data.flushBits();
    data.append(criteriaData);

    for (auto completeIter : m_completedAchievements)
    {
        if (!isVisible(completeIter))
            continue;

        data << uint32_t(completeIter.first);
        data.appendPackedTime(completeIter.second);
    }

    player->getSession()->SendPacket(&data);
    
    if (isCharacterLoading && player == m_player)
    {
        // a SMSG_ALL_ACHIEVEMENT_DATA packet has been sent to the player, so the achievement manager can send SMSG_CRITERIA_UPDATE and SMSG_ACHIEVEMENT_EARNED when it gets them
        isCharacterLoading = false;
    }
}

void AchievementMgr::sendRespondInspectAchievements(Player* player)
{
    DBC::Structures::AchievementCriteriaEntry const* acEntry;

    VisibleAchievementPred isVisible;

    ObjectGuid guid = m_player->getGuid();
    ObjectGuid counter;

    size_t numCriteria = m_criteriaProgress.size();
    size_t numAchievements = std::count_if(m_completedAchievements.begin(), m_completedAchievements.end(), isVisible);
    ByteBuffer criteriaData(numCriteria * (0));

    WorldPacket data(SMSG_RESPOND_INSPECT_ACHIEVEMENTS, 1 + 8 + 3 + 3 + numAchievements * (4 + 4) + numCriteria * (0));
    data.writeBit(guid[7]);
    data.writeBit(guid[4]);
    data.writeBit(guid[1]);
    data.writeBits(numAchievements, 23);
    data.writeBit(guid[0]);
    data.writeBit(guid[3]);
    data.writeBits(numCriteria, 21);
    data.writeBit(guid[2]);

    for (auto progressIter : m_criteriaProgress)
    {
        acEntry = sAchievementCriteriaStore.LookupEntry(progressIter.first);

        if (!acEntry)
            continue;

        if (!sAchievementStore.LookupEntry(acEntry->referredAchievement))
            continue;

        counter = uint64_t(progressIter.second->counter);

        data.writeBit(counter[5]);
        data.writeBit(counter[3]);
        data.writeBit(guid[1]);
        data.writeBit(guid[4]);
        data.writeBit(guid[2]);
        data.writeBit(counter[6]);
        data.writeBit(guid[0]);
        data.writeBit(counter[4]);
        data.writeBit(counter[1]);
        data.writeBit(counter[2]);
        data.writeBit(guid[3]);
        data.writeBit(guid[7]);
        data.writeBits(0, 2);   // criteria progress flags
        data.writeBit(counter[0]);
        data.writeBit(guid[5]);
        data.writeBit(guid[6]);
        data.writeBit(counter[7]);

        criteriaData.WriteByteSeq(guid[3]);
        criteriaData.WriteByteSeq(counter[4]);
        criteriaData << uint32_t(0);    // timer 1
        criteriaData.WriteByteSeq(guid[1]);
        criteriaData.appendPackedTime(progressIter.second->date);
        criteriaData.WriteByteSeq(counter[3]);
        criteriaData.WriteByteSeq(counter[7]);
        criteriaData.WriteByteSeq(guid[5]);
        criteriaData.WriteByteSeq(counter[0]);
        criteriaData.WriteByteSeq(guid[4]);
        criteriaData.WriteByteSeq(guid[2]);
        criteriaData.WriteByteSeq(guid[6]);
        criteriaData.WriteByteSeq(guid[7]);
        criteriaData.WriteByteSeq(counter[6]);
        criteriaData << uint32_t(progressIter.first);
        criteriaData << uint32_t(0);    // timer 2
        criteriaData.WriteByteSeq(counter[1]);
        criteriaData.WriteByteSeq(counter[5]);
        criteriaData.WriteByteSeq(guid[0]);
        criteriaData.WriteByteSeq(counter[2]);
    }

    data.writeBit(guid[6]);
    data.writeBit(guid[5]);
    data.flushBits();
    data.append(criteriaData);
    data.WriteByteSeq(guid[1]);
    data.WriteByteSeq(guid[6]);
    data.WriteByteSeq(guid[3]);
    data.WriteByteSeq(guid[0]);
    data.WriteByteSeq(guid[2]);

    for (auto completeIter : m_completedAchievements)
    {
        if (!isVisible(completeIter))
            continue;

        data << uint32_t(completeIter.first);
        data.appendPackedTime(completeIter.second);
    }

    data.WriteByteSeq(guid[7]);
    data.WriteByteSeq(guid[4]);
    data.WriteByteSeq(guid[5]);

    player->getSession()->SendPacket(&data);
}
#endif

#if VERSION_STRING <= WotLK
void AchievementMgr::sendAllAchievementData(Player* player)
{
    // maximum size for the SMSG_ALL_ACHIEVEMENT_DATA packet without causing client problems seems to be 0x7fff
    uint32_t packetSize = 18 + ((uint32_t)m_completedAchievements.size() * 8) + (GetCriteriaProgressCount() * 36);
    bool doneCompleted = false;
    bool doneProgress = false;
    DBC::Structures::AchievementCriteriaEntry const* acEntry;
    DBC::Structures::AchievementEntry const* achievement;

    WorldPacket data;
    if (packetSize < 0x8000)
    {
        data.resize(packetSize);
    }
    else
    {
        data.resize(0x7fff);
    }

    CompletedAchievementMap::iterator completeIter = m_completedAchievements.begin();
    CriteriaProgressMap::iterator progressIter = m_criteriaProgress.begin();
    bool packetFull;

    while (!doneCompleted || !doneProgress)
    {
        data.clear();
        if (player == m_player)
        {
            data.SetOpcode(SMSG_ALL_ACHIEVEMENT_DATA);
        }
        else
        {
            data.SetOpcode(SMSG_RESPOND_INSPECT_ACHIEVEMENTS);
            FastGUIDPack(data, m_player->getGuid());
        }
        packetFull = false;

        // add the completed achievements
        if (!doneCompleted)
        {
            for (; completeIter != m_completedAchievements.end() && !packetFull; ++completeIter)
            {
                if (ShowCompletedAchievement(completeIter->first, m_player))
                {
                    data << uint32_t(completeIter->first);
                    data << uint32_t(secsToTimeBitFields(completeIter->second));
                }
                packetFull = data.size() > 0x7f00;
            }
            if (completeIter == m_completedAchievements.end())
            {
                doneCompleted = true;
            }
        }

        // 0xffffffff separates between completed achievements and ones in progress
        data << int32_t(-1);
        for (; progressIter != m_criteriaProgress.end() && !packetFull; ++progressIter)
        {
            acEntry = sAchievementCriteriaStore.LookupEntry(progressIter->first);
            if (!acEntry)
            {
                continue;
            }
            achievement = sAchievementStore.LookupEntry(acEntry->referredAchievement);
            if (!achievement)
            {
                continue;
            }
            // achievement progress to send to self
            if (player == m_player)
            {
                if (SendAchievementProgress(progressIter->second))
                {
                    data << uint32_t(progressIter->first);
                    data.appendPackGUID(progressIter->second->counter);
                    data << GetPlayer()->GetNewGUID();
                    data << uint32_t(0);
                    data << uint32_t(secsToTimeBitFields(progressIter->second->date));
                    data << uint32_t(0);
                    data << uint32_t(0);
                }
            }
            // achievement progress to send to other players (inspect)
            else
            {
                // only send statistics, no other unfinished achievement progress, since client only displays them as completed or not completed
                if ((progressIter->second->counter > 0) && (achievement->flags & ACHIEVEMENT_FLAG_COUNTER))
                {
                    data << uint32_t(progressIter->first);
                    data.appendPackGUID(progressIter->second->counter);
                    data << GetPlayer()->GetNewGUID();
                    data << uint32_t(0);
                    data << uint32_t(secsToTimeBitFields(progressIter->second->date));
                    data << uint32_t(0);
                    data << uint32_t(0);
                }
            }
            packetFull = data.size() > 0x7f00;
        }
        if (progressIter == m_criteriaProgress.end())
        {
            doneProgress = true;
        }

        // another 0xffffffff denotes end of the packet
        data << int32_t(-1);
        player->getSession()->SendPacket(&data);
    }
    if (isCharacterLoading && player == m_player)
    {
        // a SMSG_ALL_ACHIEVEMENT_DATA packet has been sent to the player, so the achievement manager can send SMSG_CRITERIA_UPDATE and SMSG_ACHIEVEMENT_EARNED when it gets them
        isCharacterLoading = false;
    }
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////
/// Returns the number of achievement progresses that get sent to the player.
uint32_t AchievementMgr::GetCriteriaProgressCount()
{
    uint32_t criteriapc = 0;
    for (auto iterCriteriaProgress : m_criteriaProgress)
    {
        //AchievementEntry const *achievement = dbcAchievementStore.LookupEntry(iterCriteriaProgress.second->id);
        if (SendAchievementProgress(iterCriteriaProgress.second))
        {
            ++criteriapc;
        }
    }
    return criteriapc;
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Gives reward to player for completing the achievement.
void AchievementMgr::GiveAchievementReward(DBC::Structures::AchievementEntry const* entry)
{
    if (entry == nullptr || isCharacterLoading)
    {
        return;
    }
    AchievementReward const * Reward = sObjectMgr.GetAchievementReward(entry->ID, GetPlayer()->getGender());

    if (!Reward)
        return;

    //Reward Titel
    if (GetPlayer()->getTeam() == TEAM_ALLIANCE)
    {
        if (Reward->titel_A)
        {
            auto char_title = sCharTitlesStore.LookupEntry(Reward->titel_A);
            if (char_title)
                GetPlayer()->setKnownPvPTitle(static_cast< RankTitles >(char_title->bit_index), true);
        }
    }
    if (GetPlayer()->getTeam() == TEAM_HORDE)
    {
        if (Reward->titel_H)
        {
            auto char_title = sCharTitlesStore.LookupEntry(Reward->titel_H);
            if (char_title)
                GetPlayer()->setKnownPvPTitle(static_cast< RankTitles >(char_title->bit_index), true);
        }
    }

    //Reward Mail
    if (Reward->sender)
    {
        Creature* pCreature = GetPlayer()->getWorldMap()->createCreature(Reward->sender);
        if (pCreature == nullptr)
        {
            sLogger.failure("can not create sender for achievement %u", entry);
            return;
        }

        uint32_t Sender = Reward->sender;
        uint64_t receiver = GetPlayer()->getGuid();
        std::string messageheader = Reward->subject;
        std::string messagebody = Reward->text;

        //Create Item
        Item* pItem = sObjectMgr.CreateItem(Reward->itemId, GetPlayer());

        if (Reward->itemId == 0)
        {
            //Sending mail
            sMailSystem.SendCreatureGameobjectMail(MAIL_TYPE_CREATURE, Sender, receiver, messageheader, messagebody, 0, 0, 0, 0, MAIL_CHECK_MASK_HAS_BODY, MAIL_DEFAULT_EXPIRATION_TIME);
        }
        else if (pItem != nullptr)
        {
            pItem->SaveToDB(-1, -1, true, nullptr);
            //Sending mail
            sMailSystem.SendCreatureGameobjectMail(MAIL_TYPE_CREATURE, Sender, receiver, messageheader, messagebody, 0, 0, pItem->getGuid(), 0, MAIL_CHECK_MASK_HAS_BODY, MAIL_DEFAULT_EXPIRATION_TIME);

            //removing pItem
            pItem->DeleteMe();
            pItem = nullptr;

            //removing sender
            pCreature->Delete();
        }
        else
        {
            sLogger.failure("Can not create item for message! (nullptr)");
            return;
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Returns the number of completed achievements.
uint32_t AchievementMgr::GetCompletedAchievementsCount() const
{
    return (uint32_t)m_completedAchievements.size();
}

//////////////////////////////////////////////////////////////////////////////////////////
/// \brief GM has used a command to make the specified achievement to be completed.
/// If finishAll is true, all achievements available for the player's faction get
/// marked as completed
/// \return true if able to complete specified achievement successfully, otherwise false
bool AchievementMgr::GMCompleteAchievement(WorldSession* gmSession, uint32_t achievementID, bool finishAll/* = false*/)
{
    if (finishAll)
    {
        uint32_t nr = sAchievementStore.GetNumRows();

        for (uint32_t i = 0; i < nr; ++i)
        {
            auto ach = sAchievementStore.LookupEntry(i);
            if (ach == nullptr)
            {
                m_player->getSession()->SystemMessage("Achievement %u entry not found.", i);
            }
            else
            {
                if (!(ach->flags & ACHIEVEMENT_FLAG_COUNTER))
                {
                    if ((ach->factionFlag == ACHIEVEMENT_FACTION_FLAG_HORDE && !m_player->isTeamHorde()) ||
                        (ach->factionFlag == ACHIEVEMENT_FACTION_FLAG_ALLIANCE && !m_player->isTeamAlliance()))
                    {
                        continue;
                    }
                    CompletedAchievement(ach);
                }
            }
        }
        m_player->getSession()->SystemMessage("All achievements completed.");
        return true;
    }
    if (m_completedAchievements.find(achievementID) != m_completedAchievements.end())
    {
        gmSession->SystemMessage("Player has already completed that achievement.");
        return false;
    }
    auto achievement = sAchievementStore.LookupEntry(achievementID);
    if (!achievement)
    {
        gmSession->SystemMessage("Achievement %d entry not found.", achievementID);
        return false;
    }
    if (achievement->flags & ACHIEVEMENT_FLAG_COUNTER)
    {
        gmSession->SystemMessage("Achievement (%u) |Hachievement:%u:" I64FMT ":0:0:0:-1:0:0:0:0|h[%s]|h is a counter and cannot be completed.",
                                 achievement->ID, achievement->ID, gmSession->GetPlayer()->getGuid(), achievement->name);
        return false;
    }
    CompletedAchievement(achievement);
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
/// \brief GM has used a command to make the specified achievement criteria to be completed.
/// If finishAll is true, all achievement criteria get marked as completed
/// \return true if able to complete the achievement criteria, otherwise false
bool AchievementMgr::GMCompleteCriteria(WorldSession* gmSession, uint32_t criteriaID, bool finishAll/* = false*/)
{
    if (finishAll)
    {
        uint32_t nr = sAchievementCriteriaStore.GetNumRows();
        DBC::Structures::AchievementCriteriaEntry const* crt;
        for (uint32_t i = 0, j = 0; j < nr; ++i)
        {
            crt = sAchievementCriteriaStore.LookupEntry(i);
            if (crt == nullptr)
            {
                sLogger.failure("Achievement Criteria %lu entry not found.", i);
                continue;
            }
            ++j;
            if (crt->raw.field4)
            {
                SetCriteriaProgress(crt, crt->raw.field4);
                CompletedCriteria(crt);
            }
        }
        m_player->getSession()->SystemMessage("All achievement criteria completed.");
        return true;
    }

    auto criteria = sAchievementCriteriaStore.LookupEntry(criteriaID);
    if (!criteria)
    {
        gmSession->SystemMessage("Achievement criteria %d not found.", criteriaID);
        return false;
    }

    if (IsCompletedCriteria(criteria))
    {
        gmSession->SystemMessage("Achievement criteria %d already completed.", criteriaID);
        return false;
    }

    auto achievement = sAchievementStore.LookupEntry(criteria->referredAchievement);
    if (!achievement)
    {
        // achievement not found
        gmSession->SystemMessage("Referred achievement (%u) entry not found.", criteria->referredAchievement);
        return false;
    }

    if (achievement->flags & ACHIEVEMENT_FLAG_COUNTER)
    {
        // can't complete this type of achivement (counter)
        gmSession->SystemMessage("Referred achievement (%u) |Hachievement:%u:" I64FMT ":0:0:0:-1:0:0:0:0|h[%s]|h is a counter and cannot be completed.",
                                 achievement->ID, achievement->ID, gmSession->GetPlayer()->getGuid(), achievement->name);
        return false;
    }

    CriteriaProgressMap::iterator itr = m_criteriaProgress.find(criteriaID);
    CriteriaProgress* progress;
    if (itr == m_criteriaProgress.end())
    {
        // not in progress map
        progress = new CriteriaProgress(criteriaID, 0);
        m_criteriaProgress[criteriaID] = progress;
    }
    else
    {
        progress = itr->second;
    }

    progress->counter = criteria->raw.field4;
    SendCriteriaUpdate(progress);
    CompletedCriteria(criteria);
    return true;
}


bool AchievementMgr::UpdateAchievementCriteria(Player* player, int32_t criteriaID, uint32_t count)
{
    auto criteria = sAchievementCriteriaStore.LookupEntry(criteriaID);
    if (!criteria)
    {
        sLogger.debug("Achievement ID %u is Invalid", criteriaID);
        return false;
    }
    if (IsCompletedCriteria(criteria))
    {
        sLogger.debug("Achievement criteria %lu already completed.", criteriaID);
        return false;
    }
    auto* achievement = sAchievementStore.LookupEntry(criteria->referredAchievement);
    if (!achievement)
    {
        // achievement not found
        sLogger.debug("Referred achievement (%lu) entry not found.", criteria->referredAchievement);
        return false;
    }
    if (achievement->flags & ACHIEVEMENT_FLAG_COUNTER)
    {
        // can't complete this type of achivement (counter)
        sLogger.debug("AchievementMgr Referred achievement (%u) |Hachievement:%u:" I64FMT ":0:0:0:-1:0:0:0:0|h[%s]|h is a counter and cannot be completed.",
            achievement->ID, achievement->ID, player->getGuid(), achievement->name);
        return false;
    }

    CriteriaProgressMap::iterator itr = m_criteriaProgress.find(criteriaID);
    CriteriaProgress* progress;
    if (itr == m_criteriaProgress.end())
    {
        // not in progress map
        progress = new CriteriaProgress(criteriaID, 0);
        m_criteriaProgress[criteriaID] = progress;
    }
    else
    {
        progress = itr->second;
    }

    progress->counter = progress->counter + count;
    SendCriteriaUpdate(progress);
    CompletedCriteria(criteria);
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
/// \brief GM has used a command to reset achievement(s) for this player. If
/// finishAll is true, all achievements get reset, otherwise the one specified gets reset
void AchievementMgr::GMResetAchievement(uint32_t achievementID, bool finishAll/* = false*/)
{
    if (finishAll)
    {
        for (auto& m_completedAchievement : m_completedAchievements)
            GetPlayer()->sendPacket(SmsgAchievementDeleted(m_completedAchievement.first).serialise().get());

        m_completedAchievements.clear();
        CharacterDatabase.Execute("DELETE FROM character_achievement WHERE guid = %u", m_player->getGuidLow());
    }
    else
    {
        GetPlayer()->sendPacket(SmsgAchievementDeleted(achievementID).serialise().get());

        m_completedAchievements.erase(achievementID);
        CharacterDatabase.Execute("DELETE FROM character_achievement WHERE guid = %u AND achievement = %u", m_player->getGuidLow(), static_cast<uint32_t>(achievementID));
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
/// GM has used a command to reset achievement criteria for this player. If criteriaID
/// is finishAll true, all achievement criteria get reset, otherwise only the one specified gets reset
void AchievementMgr::GMResetCriteria(uint32_t criteriaID, bool finishAll/* = false*/)
{
    if (finishAll)
    {
        for (auto criteriaProgress : m_criteriaProgress)
        {
            GetPlayer()->sendPacket(SmsgCriteriaDeleted(criteriaProgress.first).serialise().get());
            delete criteriaProgress.second;
        }

        m_criteriaProgress.clear();
        CharacterDatabase.Execute("DELETE FROM character_achievement_progress WHERE guid = %u", m_player->getGuidLow());
    }
    else
    {
        GetPlayer()->sendPacket(SmsgCriteriaDeleted(criteriaID).serialise().get());

        m_criteriaProgress.erase(criteriaID);
        CharacterDatabase.Execute("DELETE FROM character_achievement_progress WHERE guid = %u AND criteria = %u", m_player->getGuidLow(), static_cast<uint32_t>(criteriaID));
    }

    CheckAllAchievementCriteria();
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Date/Time (time_t) the achievement was completed, or 0 if achievement not completed yet
time_t AchievementMgr::GetCompletedTime(DBC::Structures::AchievementEntry const* achievement)
{
    CompletedAchievementMap::iterator iter = m_completedAchievements.find(achievement->ID);
    if (iter != m_completedAchievements.end())
    {
        // achievement is completed, return the date/time it was completed
        return iter->second;
    }
    return 0; // achievement not completed
}

//////////////////////////////////////////////////////////////////////////////////////////
/// \return true if achievementID has been completed by the player, false otherwise.
bool AchievementMgr::HasCompleted(uint32_t achievementID)
{
    return (m_completedAchievements.find(achievementID) != m_completedAchievements.end());
}

#endif
