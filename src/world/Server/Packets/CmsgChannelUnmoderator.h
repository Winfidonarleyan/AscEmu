/*
Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu::Packets
{
    class CmsgChannelUnmoderator : public ManagedPacket
    {
    public:
        std::string name;
        std::string unmodName;

        CmsgChannelUnmoderator() : CmsgChannelUnmoderator("", "")
        {
        }

        CmsgChannelUnmoderator(std::string name, std::string unmodName) :
            ManagedPacket(CMSG_CHANNEL_UNMODERATOR, 0),
            name(name),
            unmodName(unmodName)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> name >> unmodName;
            return true;
        }
    };
}
