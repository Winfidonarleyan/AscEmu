/*
Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/


#include "Server/MainServerDefines.h"
#include "Chat/ChatHandler.hpp"

#if VERSION_STRING > TBC
//.achieve complete
bool ChatHandler::HandleAchievementCompleteCommand(const char* args, WorldSession* m_session)
{
    Player* selected_player = GetSelectedPlayer(m_session, true, true);
    if (selected_player == nullptr)
        return true;

    if (!*args)
        return false;

    if (stricmp(args, "all") == 0)
    {
        selected_player->getAchievementMgr().GMCompleteAchievement(m_session, 0, true);
        SystemMessage(m_session, "All achievements have now been completed for that player.");
        sGMLog.writefromsession(m_session, "completed all achievements for player %s", selected_player->getName().c_str());
        return true;
    }

    uint32 achievement_id = atol(args);
    if (achievement_id == 0)
    {
            return false;
    }
    else if (selected_player->getAchievementMgr().GMCompleteAchievement(m_session, achievement_id))
    {
        SystemMessage(m_session, "The achievement has now been completed for that player.");
        sGMLog.writefromsession(m_session, "completed achievement %u for player %s", achievement_id, selected_player->getName().c_str());
    }
    return true;
}

//.achieve criteria
bool ChatHandler::HandleAchievementCriteriaCommand(const char* args, WorldSession* m_session)
{
    Player* selected_player = GetSelectedPlayer(m_session, true, true);
    if (selected_player == nullptr)
        return true;

    if (!*args)
        return false;

    uint32 criteria_id = atol(args);
    if (criteria_id == 0)
    {
        if (stricmp(args, "all") == 0)
        {
            selected_player->getAchievementMgr().GMCompleteCriteria(m_session, 0, true);
            SystemMessage(m_session, "All achievement criteria have now been completed for that player.");
            sGMLog.writefromsession(m_session, "completed all achievement criteria for player %s", selected_player->getName().c_str());
            return true;
        }
        return false;
    }
    else if (selected_player->getAchievementMgr().GMCompleteCriteria(m_session, criteria_id))
    {
        SystemMessage(m_session, "The achievement criteria has now been completed for that player.");
        sGMLog.writefromsession(m_session, "completed achievement criteria %u for player %s", criteria_id, selected_player->getName().c_str());
    }
    return true;
}

//.achieve reset
bool ChatHandler::HandleAchievementResetCommand(const char* args, WorldSession* m_session)
{
    Player* selected_player = GetSelectedPlayer(m_session, true, true);
    if (selected_player == nullptr)
        return true;

    if (!*args)
        return false;

    bool reset_achievement = true;
    bool reset_criteria = false;
    bool resetAll = false;
    uint32_t achievement_id = 0;

    if (strnicmp(args, "criteria ", 9) == 0)
    {
        achievement_id = atol(args + 9);
        if (achievement_id == 0)
        {
            if (stricmp(args + 9, "all") != 0)
            {
                return false;
            }
            resetAll = true;
        }
        reset_criteria = true;
        reset_achievement = false;
    }
    else if (stricmp(args, "all") == 0)
    {
        resetAll = true;
        reset_criteria = true;
    }
    else
    {
        achievement_id = atol(args);
        if (achievement_id == 0)
        {
            achievement_id = GetAchievementIDFromLink(args);
            if (achievement_id == 0)
                return false;
        }
    }

    if (reset_achievement)
        selected_player->getAchievementMgr().GMResetAchievement(achievement_id, resetAll);

    if (reset_criteria)
        selected_player->getAchievementMgr().GMResetCriteria(achievement_id, resetAll);

    return true;
}
#endif
