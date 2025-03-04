/*
Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Channel.hpp"
#include "ChannelMgr.hpp"

#include "Map/Area/AreaManagementGlobals.hpp"
#include "Map/Area/AreaStorage.hpp"
#include "Objects/Units/Players/Player.h"
#include "Server/World.h"
#include "Server/WorldSession.h"
#include "Server/Packets/SmsgChannelNotify.h"
#include "Util/Strings.hpp"

using namespace AscEmu::Packets;

ChannelMgr& ChannelMgr::getInstance()
{
    static ChannelMgr mInstance;
    return mInstance;
}

void ChannelMgr::initialize()
{
    m_seperateChannels = false;
}

void ChannelMgr::finalize()
{
    for (uint8 i = 0; i < 2; ++i)
    {
        for (auto& channelList : this->m_channelList[i])
            delete channelList.second;

        m_channelList[i].clear();
    }
}

void ChannelMgr::loadConfigSettings()
{
    auto bannedChannels = worldConfig.chat.bannedChannels;
    auto minimumLevel = worldConfig.chat.minimumTalkLevel;

    std::lock_guard<std::mutex> guard(m_mutexConfig);

    m_bannedChannels = AscEmu::Util::Strings::split(bannedChannels, ";");
    m_minimumChannel = AscEmu::Util::Strings::split(minimumLevel, ";");
}

void ChannelMgr::setSeperatedChannels(bool enabled)
{
    m_seperateChannels = enabled;
}

Channel* ChannelMgr::getOrCreateChannel(std::string name, Player const* player, uint32_t typeId)
{
    auto channelList = &m_channelList[0];
    if (m_seperateChannels && player && name != worldConfig.getGmClientChannelName())
        channelList = &m_channelList[player->getTeam()];

    std::lock_guard<std::mutex> channelGuard(m_mutexChannels);

    for (auto& channelListMember : *channelList)
    {
        if (name == channelListMember.first)
            return channelListMember.second;
    }

    std::lock_guard<std::mutex> configGuard(m_mutexConfig);

    for (auto& m_bannedChannel : m_bannedChannels)
    {
        if (name == m_bannedChannel)
            return nullptr;
    }

    auto channel = new Channel(name, (m_seperateChannels && player) ? player->getTeam() : TEAM_ALLIANCE, typeId);

    channelList->insert(make_pair(channel->getChannelName(), channel));

    return channel;
}

void ChannelMgr::removeChannel(Channel* channel)
{
    if (!channel)
        return;

    auto channelList = &m_channelList[0];
    if (m_seperateChannels)
        channelList = &m_channelList[channel->getChannelTeam()];

    std::lock_guard<std::mutex> guard(m_mutexChannels);

    for (auto channelListMember = channelList->begin(); channelListMember != channelList->end(); ++channelListMember)
    {
        if (channelListMember->second == channel)
        {
            channelList->erase(channelListMember);
            delete channel;

            return;
        }
    }
}

Channel* ChannelMgr::getChannel(std::string name, Player const* player) const
{
    auto channelList = &m_channelList[0];
    if (m_seperateChannels && player && name != worldConfig.getGmClientChannelName())
        channelList = &m_channelList[player->getTeam()];

    std::lock_guard<std::mutex> guard(m_mutexChannels);

    for (auto& channelListMember : *channelList)
    {
        if (name == channelListMember.first)
            return channelListMember.second;
    }

    return nullptr;
}

Channel* ChannelMgr::getChannel(std::string name, uint32_t team) const
{
    auto channelList = &m_channelList[0];
    if (m_seperateChannels && name != worldConfig.getGmClientChannelName())
        channelList = &m_channelList[team];

    std::lock_guard<std::mutex> guard(m_mutexChannels);

    for (auto& channelListMember : *channelList)
    {
        if (name == channelListMember.first)
            return channelListMember.second;
    }

    return nullptr;
}

bool ChannelMgr::canPlayerJoinDefaultChannel(Player const* player, DBC::Structures::AreaTableEntry const* areaEntry, DBC::Structures::ChatChannelsEntry const* channelDbc) const
{
    if (player == nullptr || channelDbc == nullptr)
        return false;

    // City specific channels
    if (channelDbc->flags & (CHANNEL_DBC_CITY_ONLY_1 | CHANNEL_DBC_CITY_ONLY_2))
    {
        if (areaEntry == nullptr || !(areaEntry->flags & MapManagement::AreaManagement::AREA_CITY_AREA))
            return false;
    }

    // Channels with zone name
    if (channelDbc->flags & CHANNEL_DBC_HAS_ZONENAME)
    {
        if (areaEntry == nullptr)
            return false;
    }

    // Guild recruitment channel
    if (channelDbc->flags & CHANNEL_DBC_GUILD_RECRUIT)
    {
        if (player->getGuild() != nullptr)
            return false;
    }

    return true;
}

std::string ChannelMgr::generateChannelName(DBC::Structures::ChatChannelsEntry const* channelDbc, DBC::Structures::AreaTableEntry const* areaEntry) const
{
#if VERSION_STRING < Cata
    char* channelNameDbc = channelDbc->name_pattern[sWorld.getDbcLocaleLanguageId()];
#else
    char* channelNameDbc = channelDbc->name_pattern;
#endif

    if (channelDbc->flags & CHANNEL_DBC_GLOBAL || !(channelDbc->flags & CHANNEL_DBC_HAS_ZONENAME))
        return std::string(channelNameDbc);

    char channelName[95];
    char const* defaultAreaName = "City";

    if (const auto defaultArea = MapManagement::AreaManagement::AreaStorage::GetAreaById(3459))
    {
#if VERSION_STRING < Cata
        defaultAreaName = defaultArea->area_name[sWorld.getDbcLocaleLanguageId()];
#else
        defaultAreaName = defaultArea->area_name;
#endif
    }

    // City specific channels
    if (channelDbc->flags & (CHANNEL_DBC_CITY_ONLY_1 | CHANNEL_DBC_CITY_ONLY_2))
    {
        std::snprintf(channelName, 95, channelNameDbc, defaultAreaName);
    }
    else
    {
        if (areaEntry != nullptr)
        {
#if VERSION_STRING < Cata
            std::snprintf(channelName, 95, channelNameDbc, areaEntry->area_name[sWorld.getDbcLocaleLanguageId()]);
#else
            std::snprintf(channelName, 95, channelNameDbc, areaEntry->area_name);
#endif
        }
        else
        {
            std::snprintf(channelName, 95, channelNameDbc, defaultAreaName);
        }
    }

    return std::string(channelName);
}
