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
 *
 */


#include "VMapFactory.h"
#include "VMapManager2.h"
#include "Chat/ChatHandler.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "Server/MainServerDefines.h"
#include "Map/Area/AreaStorage.hpp"
#include "Map/Management/MapMgr.hpp"
#include "Spell/SpellAuras.h"
#include "Spell/Definitions/SpellCastTargetFlags.hpp"
#include "Spell/SpellMgr.hpp"
#include "Server/Packets/SmsgMoveKnockBack.h"
#include "Movement/Spline/MoveSplineInit.h"
#include "Objects/Units/ThreatHandler.h"

bool ChatHandler::HandleDebugDumpMovementCommand(const char* /*args*/, WorldSession* session)
{
    try
    {
        auto me = session->GetPlayerOrThrow();

        SystemMessage(session, "Position: [%f, %f, %f, %f]", me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());
        SystemMessage(session, "On transport: %s", me->obj_movement_info.transport_guid != 0 ? "yes" : "no");
        SystemMessage(session, "Transport GUID: %lu", uint64_t(me->obj_movement_info.transport_guid));
        SystemMessage(session, "Transport relative position: [%f, %f, %f, %f]", me->obj_movement_info.transport_position.x,
            me->obj_movement_info.transport_position.y, me->obj_movement_info.transport_position.z, me->obj_movement_info.transport_position.o);

        return true;
    }
    catch(...)
    {
        return false;
    }
}

bool ChatHandler::HandleDebugInFrontCommand(const char* /*args*/, WorldSession* m_session)
{
    Object* obj;

    uint64 guid = m_session->GetPlayer()->getTargetGuid();
    if (guid != 0)
    {
        if ((obj = m_session->GetPlayer()->getWorldMap()->getUnit(guid)) == 0)
        {
            SystemMessage(m_session, "You should select a character or a creature.");
            return true;
        }
    }
    else
    {
        obj = m_session->GetPlayer();
    }

    char buf[256];
    snprintf((char*)buf, 256, "%d", m_session->GetPlayer()->isInFront(obj));

    SystemMessage(m_session, buf);

    return true;
}

bool ChatHandler::HandleShowReactionCommand(const char* args, WorldSession* m_session)
{
    Object* obj = nullptr;

    WoWGuid wowGuid;
    wowGuid.Init(m_session->GetPlayer()->getTargetGuid());

    if (wowGuid.getRawGuid() != 0)
    {
        obj = m_session->GetPlayer()->getWorldMap()->getCreature(wowGuid.getGuidLowPart());
    }

    if (!obj)
    {
        SystemMessage(m_session, "You should select a creature.");
        return true;
    }


    char* pReaction = strtok((char*)args, " ");
    if (!pReaction)
        return false;

    uint32 Reaction = atoi(pReaction);

    obj->SendAIReaction(Reaction);

    std::stringstream sstext;
    sstext << "Sent Reaction of " << Reaction << " to " << obj->GetUIdFromGUID() << '\0';

    SystemMessage(m_session, sstext.str().c_str());
    return true;
}

bool ChatHandler::HandleDistanceCommand(const char* /*args*/, WorldSession* m_session)
{
    Object* obj;

    uint64 guid = m_session->GetPlayer()->getTargetGuid();
    if (guid != 0)
    {
        if ((obj = m_session->GetPlayer()->getWorldMap()->getUnit(guid)) == 0)
        {
            SystemMessage(m_session, "You should select a character or a creature.");
            return true;
        }
    }
    else
    {
        obj = m_session->GetPlayer();
    }

    float dist = m_session->GetPlayer()->CalcDistance(obj);
    std::stringstream sstext;
    sstext << "Distance is: " << dist << '\0';

    SystemMessage(m_session, sstext.str().c_str());
    return true;
}


bool ChatHandler::HandleAIMoveCommand(const char* args, WorldSession* m_session)
{
    Creature* creature = nullptr;
    auto player = m_session->GetPlayer();
    WoWGuid wowGuid;
    wowGuid.Init(player->getTargetGuid());
    if (wowGuid.getRawGuid() != 0)
    {
        creature = player->getWorldMap()->getCreature(wowGuid.getGuidLowPart());
    }

    if (creature == nullptr)
    {
        SystemMessage(m_session, "You should select a creature.");
        return true;
    }

    uint32 Move = 1;
    uint32 Run = 0;
    uint32 Time = 0;
    uint32 Meth = 0;

    char* pMove = strtok((char*)args, " ");
    if (pMove)
        Move = atoi(pMove);

    char* pRun = strtok(NULL, " ");
    if (pRun)
        Run = atoi(pRun);

    char* pTime = strtok(NULL, " ");
    if (pTime)
        Time = atoi(pTime);

    char* pMeth = strtok(NULL, " ");
    if (pMeth)
        Meth = atoi(pMeth);

    float x = player->GetPositionX();
    float y = player->GetPositionY();
    float z = player->GetPositionZ();
    //float o = m_session->GetPlayer()->GetOrientation();

    MovementNew::MoveSplineInit init(creature);

    float distance = creature->CalcDistance(x, y, z);
    if (Move == 1)
    {
        if (Meth == 1)
        {
            float q = distance - 0.5f;
            x = (creature->GetPositionX() + x * q) / (1 + q);
            y = (creature->GetPositionY() + y * q) / (1 + q);
            z = (creature->GetPositionZ() + z * q) / (1 + q);
        }
        else if (Meth == 2)
        {
            float q = distance - 1;
            x = (creature->GetPositionX() + x * q) / (1 + q);
            y = (creature->GetPositionY() + y * q) / (1 + q);
            z = (creature->GetPositionZ() + z * q) / (1 + q);
        }
        else if (Meth == 3)
        {
            float q = distance - 2;
            x = (creature->GetPositionX() + x * q) / (1 + q);
            y = (creature->GetPositionY() + y * q) / (1 + q);
            z = (creature->GetPositionZ() + z * q) / (1 + q);
        }
        else if (Meth == 4)
        {
            float q = distance - 2.5f;
            x = (creature->GetPositionX() + x * q) / (1 + q);
            y = (creature->GetPositionY() + y * q) / (1 + q);
            z = (creature->GetPositionZ() + z * q) / (1 + q);
        }
        else if (Meth == 5)
        {
            float q = distance - 3;
            x = (creature->GetPositionX() + x * q) / (1 + q);
            y = (creature->GetPositionY() + y * q) / (1 + q);
            z = (creature->GetPositionZ() + z * q) / (1 + q);
        }
        else if (Meth == 6)
        {
            float q = distance - 3.5f;
            x = (creature->GetPositionX() + x * q) / (1 + q);
            y = (creature->GetPositionY() + y * q) / (1 + q);
            z = (creature->GetPositionZ() + z * q) / (1 + q);
        }
        else
        {
            float q = distance - 4;
            x = (creature->GetPositionX() + x * q) / (1 + q);
            y = (creature->GetPositionY() + y * q) / (1 + q);
            z = (creature->GetPositionZ() + z * q) / (1 + q);
        }

        init.MoveTo(x, y, z);
        if (Run)
            init.SetWalk(false);
        else
            init.SetWalk(true);

        creature->getMovementManager()->launchMoveSpline(std::move(init));
    }
    else
    {
        init.MoveTo(x, y, z);
        if (Run)
            init.SetWalk(false);
        else
            init.SetWalk(true);

        creature->getMovementManager()->launchMoveSpline(std::move(init));
    }

    return true;
}

bool ChatHandler::HandleFaceCommand(const char* args, WorldSession* m_session)
{
    Object* obj = nullptr;

    WoWGuid wowGuid;
    wowGuid.Init(m_session->GetPlayer()->getTargetGuid());

    if (wowGuid.getRawGuid() != 0)
    {
        obj = m_session->GetPlayer()->getWorldMap()->getCreature(wowGuid.getGuidLowPart());
    }

    if (obj == nullptr)
    {
        SystemMessage(m_session, "You should select a creature.");
        return true;
    }

    uint32 Orentation = 0;
    char* pOrentation = strtok((char*)args, " ");
    if (pOrentation)
        Orentation = atoi(pOrentation);

    // Convert to Blizzards Format
    float theOrientation = Orentation / (180.0f / M_PI_FLOAT);

    obj->SetPosition(obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ(), theOrientation, false);

    sLogger.debug("facing sent");
    return true;

}

bool ChatHandler::HandleSetBytesCommand(const char* /*args*/, WorldSession* m_session)
{
    SystemMessage(m_session, "This command is no longer availabe!");
    /*Object* obj;

    uint64 guid = m_session->GetPlayer()->getTargetGuid();
    if (guid != 0)
    {
        if ((obj = m_session->GetPlayer()->getWorldMap()->getUnit(guid)) == 0)
        {
            SystemMessage(m_session, "You should select a character or a creature.");
            return true;
        }
    }
    else
    {
        obj = m_session->GetPlayer();
    }

    char* pBytesIndex = strtok((char*)args, " ");
    if (!pBytesIndex)
        return false;

    uint16 BytesIndex = static_cast<uint16>(atoi(pBytesIndex));

    char* pValue1 = strtok(NULL, " ");
    if (!pValue1)
        return false;

    uint8 Value1 = static_cast<uint8>(atol(pValue1));

    char* pValue2 = strtok(NULL, " ");
    if (!pValue2)
        return false;

    uint8 Value2 = static_cast<uint8>(atol(pValue2));

    char* pValue3 = strtok(NULL, " ");
    if (!pValue3)
        return false;

    uint8 Value3 = static_cast<uint8>(atol(pValue3));

    char* pValue4 = strtok(NULL, " ");
    if (!pValue4)
        return false;

    uint8 Value4 = static_cast<uint8>(atol(pValue4));

    std::stringstream sstext;
    sstext << "Set Field " << BytesIndex
        << " bytes to " << uint16((uint8)Value1)
        << " " << uint16((uint8)Value2)
        << " " << uint16((uint8)Value3)
        << " " << uint16((uint8)Value4)
        << '\0';
    obj->setUInt32Value(BytesIndex, ((Value1) | (Value2 << 8) | (Value3 << 16) | (Value4 << 24)));
    SystemMessage(m_session, sstext.str().c_str());*/

    return true;
}

bool ChatHandler::HandleGetBytesCommand(const char* /*args*/, WorldSession* m_session)
{
    SystemMessage(m_session, "This command is no longer availabe!");

    /*Object* obj;

    uint64 guid = m_session->GetPlayer()->getTargetGuid();
    if (guid != 0)
    {
        if ((obj = m_session->GetPlayer()->getWorldMap()->getUnit(guid)) == nullptr)
        {
            SystemMessage(m_session, "You should select a character or a creature.");
            return true;
        }
    }
    else
        obj = m_session->GetPlayer();

    char* pBytesIndex = strtok((char*)args, " ");
    if (!pBytesIndex)
        return false;

    uint16 BytesIndex = static_cast<uint16>(atoi(pBytesIndex));
    uint32 theBytes = obj->getUInt32Value(BytesIndex);

    std::stringstream sstext;
    sstext << "bytes for Field " << BytesIndex << " are " << uint16((uint8)theBytes & 0xFF) << " " << uint16((uint8)(theBytes >> 8) & 0xFF) << " ";
    sstext << uint16((uint8)(theBytes >> 16) & 0xFF) << " " << uint16((uint8)(theBytes >> 24) & 0xFF) << '\0';

    SystemMessage(m_session, sstext.str().c_str());*/
    return true;
}
bool ChatHandler::HandleDebugLandWalk(const char* /*args*/, WorldSession* m_session)
{
    Player* chr = GetSelectedPlayer(m_session, true, true);
    if (chr == nullptr)
        return true;

    char buf[256];

    chr->setMoveLandWalk();
    snprintf((char*)buf, 256, "Land Walk Test Ran.");
    SystemMessage(m_session, buf);

    return true;
}

bool ChatHandler::HandleAggroRangeCommand(const char* /*args*/, WorldSession* m_session)
{
    Unit* unit = GetSelectedUnit(m_session, true);
    if (unit == nullptr)
        return true;

    float aggroRange = unit->getAIInterface()->calcAggroRange(m_session->GetPlayer());

    GreenSystemMessage(m_session, "Aggrorange is %f", aggroRange);

    return true;
}

bool ChatHandler::HandleKnockBackCommand(const char* args, WorldSession* m_session)
{
    float f = args ? (float)atof(args) : 0.0f;
    if (f == 0.0f)
        f = 5.0f;

    float dx = sinf(m_session->GetPlayer()->GetOrientation());
    float dy = cosf(m_session->GetPlayer()->GetOrientation());

    float z = f * 0.66f;

    m_session->SendPacket(AscEmu::Packets::SmsgMoveKnockBack(m_session->GetPlayer()->GetNewGUID(), Util::getMSTime(), dy, dx, f, z).serialise().get());
    return true;
}

bool ChatHandler::HandleFadeCommand(const char* args, WorldSession* m_session)
{
    Unit* target = m_session->GetPlayer()->getWorldMap()->getUnit(m_session->GetPlayer()->getTargetGuid());
    if (!target)
        target = m_session->GetPlayer();
    char* v = strtok((char*)args, " ");
    if (!v)
        return false;

    target->modThreatModifyer(atoi(v));

    std::stringstream sstext;
    sstext << "threat is now reduced by: " << target->getThreatModifyer() << '\0';

    SystemMessage(m_session, sstext.str().c_str());
    return true;
}

bool ChatHandler::HandleThreatModCommand(const char* args, WorldSession* m_session)
{
    Unit* target = m_session->GetPlayer()->getWorldMap()->getUnit(m_session->GetPlayer()->getTargetGuid());
    if (!target)
        target = m_session->GetPlayer();
    char* v = strtok((char*)args, " ");
    if (!v)
        return false;

    target->modGeneratedThreatModifyer(0, atoi(v));

    std::stringstream sstext;
    sstext << "new threat caused is now reduced by: " << target->getGeneratedThreatModifyer(0) << "%" << '\0';

    SystemMessage(m_session, sstext.str().c_str());
    return true;
}

bool ChatHandler::HandleMoveFallCommand(const char* /*args*/, WorldSession* m_session)
{
    Unit* target = m_session->GetPlayer()->getWorldMap()->getUnit(m_session->GetPlayer()->getTargetGuid());
    if (!target)
        return true;

    bool needsFalling = (target->IsFlying() || target->isHovering()) && !target->isUnderWater();
    target->setMoveHover(false);
    target->setMoveDisableGravity(false);

    if (needsFalling)
        target->getMovementManager()->moveFall();

    return true;
}

bool ChatHandler::HandleThreatListCommand(const char* /*args*/, WorldSession* m_session)
{
    Unit* target = nullptr;
    target = m_session->GetPlayer()->getWorldMap()->getUnit(m_session->GetPlayer()->getTargetGuid());
    if (!target)
    {
        SystemMessage(m_session, "You should select a creature.");
        return true;
    }

    WoWGuid wowGuid;
    wowGuid.Init(m_session->GetPlayer()->getTargetGuid());

    std::stringstream sstext;
    sstext << "threatlist of creature: " << wowGuid.getGuidLowPart() << " " << wowGuid.getGuidHighPart() << '\n';

    for (ThreatReference* ref : target->getThreatManager().getModifiableThreatList())
    {
        sstext << "guid: " << ref->getOwner()->getGuid() << " | threat: " << ref->getThreat() << "\n";
    }

    SendMultilineMessage(m_session, sstext.str().c_str());
    return true;
}

bool ChatHandler::HandleSendItemPushResult(const char* args, WorldSession* m_session)
{
    uint32 uint_args[7];
    char* arg = const_cast<char*>(args);
    char* token = strtok(arg, " ");

    uint8 i = 0;
    while (token != NULL && i < 7)
    {
        uint_args[i] = atol(token);
        token = strtok(NULL, " ");
        i++;
    }
    for (; i < 7; i++)
        uint_args[i] = 0;

    if (uint_args[0] == 0)   // null itemid
        return false;

    WorldPacket data;
    data.SetOpcode(SMSG_ITEM_PUSH_RESULT);
    data << m_session->GetPlayer()->getGuid();    // recivee_guid
    data << uint_args[2];   // type
    data << uint32(1);      // unk
    data << uint_args[1];   // count
    data << uint8(0xFF);    // uint8 unk const 0xFF
    data << uint_args[3];   // unk1
    data << uint_args[0];   // item id
    data << uint_args[4];   // unk2
    data << uint_args[5];   // random prop
    data << uint_args[6];   // unk3
    m_session->SendPacket(&data);

    return true;
}

bool ChatHandler::HandleModifyBitCommand(const char* /*args*/, WorldSession* m_session)
{
    SystemMessage(m_session, "This command is no longer availabe!");

    /*Object* obj;

    uint64 guid = m_session->GetPlayer()->getTargetGuid();
    if (guid != 0)
    {
        if ((obj = m_session->GetPlayer()->getWorldMap()->getUnit(guid)) == 0)
        {
            SystemMessage(m_session, "You should select a character or a creature.");
            return true;
        }
    }
    else
    {
        obj = m_session->GetPlayer();
    }

    char* pField = strtok((char*)args, " ");
    if (!pField)
        return false;

    char* pBit = strtok(NULL, " ");
    if (!pBit)
        return false;

    uint16 field = static_cast<uint16>(atoi(pField));
    uint32 bit = atoi(pBit);

    if (field < 1 || field >= getSizeOfStructure(WoWPlayer))
    {
        SystemMessage(m_session, "Incorrect values.");
        return true;
    }

    if (bit < 1 || bit > 32)
    {
        SystemMessage(m_session, "Incorrect values.");
        return true;
    }

    char buf[256];

    if (obj->HasFlag(field, (1 << (bit - 1))))
    {
        obj->RemoveFlag(field, (1 << (bit - 1)));
        snprintf((char*)buf, 256, "Removed bit %i in field %i.", (unsigned int)bit, (unsigned int)field);
    }
    else
    {
        obj->SetFlag(field, (1 << (bit - 1)));
        snprintf((char*)buf, 256, "Set bit %i in field %i.", (unsigned int)bit, (unsigned int)field);
    }

    SystemMessage(m_session, buf);*/
    return true;
}

bool ChatHandler::HandleModifyValueCommand(const char* /*args*/, WorldSession* m_session)
{
    SystemMessage(m_session, "This command is no longer availabe!");

    /*Object* obj;

    uint64 guid = m_session->GetPlayer()->getTargetGuid();
    if (guid != 0)
    {
        if ((obj = m_session->GetPlayer()->getWorldMap()->getUnit(guid)) == 0)
        {
            SystemMessage(m_session, "You should select a character or a creature.");
            return true;
        }
    }
    else
        obj = m_session->GetPlayer();

    char* pField = strtok((char*)args, " ");
    if (!pField)
        return false;

    char* pValue = strtok(NULL, " ");
    if (!pValue)
        return false;

    uint16 field = static_cast<uint16>(atoi(pField));
    uint32 value = atoi(pValue);

    if (field < 1 || field >= getSizeOfStructure(WoWPlayer))
    {
        SystemMessage(m_session, "Incorrect Field.");
        return true;
    }

    char buf[256];
    uint32 oldValue = obj->getUInt32Value(field);
    obj->setUInt32Value(field, value);

    snprintf((char*)buf, 256, "Set Field %i from %i to %i.", (unsigned int)field, (unsigned int)oldValue, (unsigned int)value);

    if (obj->isPlayer())
        static_cast< Player* >(obj)->UpdateChances();

    SystemMessage(m_session, buf);*/

    return true;
}

bool ChatHandler::HandleDebugDumpCoordsCommmand(const char* /*args*/, WorldSession* m_session)
{
    Player* p = m_session->GetPlayer();
    //char buffer[200] = {0};
    FILE* f = fopen("C:\\script_dump.txt", "a");
    if (!f)
        return false;

    fprintf(f, "mob.CreateWaypoint(%f, %f, %f, %f, 0, 0, 0);\n", p->GetPositionX(), p->GetPositionY(), p->GetPositionZ(), p->GetOrientation());
    fclose(f);

    return true;
}

bool ChatHandler::HandleDebugSpawnWarCommand(const char* args, WorldSession* m_session)
{
    uint32 count, npcid;
    uint32 health = 0;

    // takes 2 or 3 arguments: npcid, count, (health)
    if (sscanf(args, "%u %u %u", &npcid, &count, &health) != 3)
    {
        if (sscanf(args, "%u %u", &count, &npcid) != 2)
        {
            return false;
        }
    }

    if (!count || !npcid)
    {
        return false;
    }

    CreatureProperties const* cp = sMySQLStore.getCreatureProperties(npcid);
    if (cp == nullptr)
    {
        return false;
    }

    WorldMap* m = m_session->GetPlayer()->getWorldMap();

    // if we have selected unit, use its position
    Unit* unit = m->getUnit(m_session->GetPlayer()->getTargetGuid());
    if (unit == nullptr)
    {
        unit = m_session->GetPlayer(); // otherwise ours
    }

    float bx = unit->GetPositionX();
    float by = unit->GetPositionY();
    float x, y, z;

    float angle = 1;
    float r = 3; // starting radius
    for (; count > 0; --count)
    {
        // spawn in spiral
        x = r * sinf(angle);
        y = r * cosf(angle);
        z = unit->getMapHeight(LocationVector(bx + x, by + y, unit->GetPositionZ() + 2));

        Creature* c = m->createCreature(npcid);
        c->Load(cp, bx + x, by + y, z, 0.0f);
        if (health != 0)
        {
            c->setMaxHealth(health);
            c->setHealth(health);
        }
        c->setFactionTemplate((count % 2) ? 1 : 2);
        c->setServersideFaction();
        c->PushToWorld(m);

        r += 0.5;
        angle += 8 / r;
    }
    return true;
}

bool ChatHandler::HandleUpdateWorldStateCommand(const char *args, WorldSession* session)
{
    if (*args == '\0')
    {
        RedSystemMessage(session, "You need to specify the worldstate field and the new value.");
        return true;
    }

    uint32 field = 0;
    uint32 state = 0;

    std::stringstream ss(args);

    ss >> field;
    if (ss.fail())
    {
        RedSystemMessage(session, "You need to specify the worldstate field and the new value.");
        return true;
    }

    ss >> state;
    if (ss.fail())
    {
        RedSystemMessage(session, "You need to specify the worldstate field and the new value.");
        return true;
    }

    session->GetPlayer()->sendWorldStateUpdate(field, state);

    return true;
}

bool ChatHandler::HandleInitWorldStatesCommand(const char* /*args*/, WorldSession* session)
{
    Player* p = session->GetPlayer();

    uint32 zone = p->GetZoneId();
    if (zone == 0)
        zone = p->getAreaId();

    BlueSystemMessage(session, "Sending initial worldstates for zone %u", zone);

    p->sendInitialWorldstates();

    return true;
}

bool ChatHandler::HandleClearWorldStatesCommand(const char* /*args*/, WorldSession* session)
{
    Player* p = session->GetPlayer();

    uint32 zone = p->GetZoneId();
    if (zone == 0)
        zone = p->getAreaId();

    BlueSystemMessage(session, "Clearing worldstates for zone %u", zone);

    WorldPacket data(SMSG_INIT_WORLD_STATES, 16);

    data << uint32(p->GetMapId());
    data << uint32(p->GetZoneId());
    data << uint32(p->getAreaId());
    data << uint16(0);

    p->sendPacket(&data);

    return true;
}

bool ChatHandler::HandleAuraUpdateRemove(const char* args, WorldSession* m_session)
{
    if (!args)
        return false;

    char* pArgs = strtok((char*)args, " ");
    if (!pArgs)
        return false;
    uint8 VisualSlot = (uint8)atoi(pArgs);
    Player* Pl = m_session->GetPlayer();
    Aura* AuraPtr = Pl->getAuraWithId(Pl->getVisualAuraList().at(VisualSlot));
    if (!AuraPtr)
    {
        SystemMessage(m_session, "No auraid found in slot %u", VisualSlot);
        return true;
    }
    SystemMessage(m_session, "SMSG_AURA_UPDATE (remove): VisualSlot %u - SpellID 0", VisualSlot);
    AuraPtr->removeAura();
    return true;
}

bool ChatHandler::HandleAuraUpdateAdd(const char* args, WorldSession* m_session)
{
    if (!args)
        return false;

    uint32 SpellID = 0;
    int Flags = 0;
    int StackCount = 0;
    if (sscanf(args, "%u 0x%X %i", &SpellID, &Flags, &StackCount) != 3 && sscanf(args, "%u %u %i", &SpellID, &Flags, &StackCount) != 3)
        return false;

    Player* Pl = m_session->GetPlayer();
    if (Aura* AuraPtr = Pl->getAuraWithId(SpellID))
    {
        uint8 VisualSlot = AuraPtr->m_visualSlot;
        Pl->sendAuraUpdate(AuraPtr, false);
        SystemMessage(m_session, "SMSG_AURA_UPDATE (update): VisualSlot %u - SpellID %u - Flags %i (0x%04X) - StackCount %i", VisualSlot, SpellID, Flags, Flags, StackCount);
    }
    else
    {
        SpellInfo const* Sp = sSpellMgr.getSpellInfo(SpellID);
        if (!Sp)
        {
            SystemMessage(m_session, "SpellID %u is invalid.", SpellID);
            return true;
        }
        Spell* SpellPtr = sSpellMgr.newSpell(Pl, Sp, false, NULL);
        AuraPtr = sSpellMgr.newAura(Sp, SpellPtr->getDuration(), Pl, Pl);
        SystemMessage(m_session, "SMSG_AURA_UPDATE (add): VisualSlot %u - SpellID %u - Flags %i (0x%04X) - StackCount %i", AuraPtr->m_visualSlot, SpellID, Flags, Flags, StackCount);
        Pl->addAura(AuraPtr);       // Serves purpose to just add the aura to our auraslots

        delete SpellPtr;
    }
    return true;
}

float CalculateDistance(float x1, float y1, float z1, float x2, float y2, float z2)
{
    float dx = x1 - x2;
    float dy = y1 - y2;
    float dz = z1 - z2;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

bool ChatHandler::HandleSimpleDistanceCommand(const char* args, WorldSession* m_session)
{
    float toX, toY, toZ;
    if (sscanf(args, "%f %f %f", &toX, &toY, &toZ) != 3)
        return false;

    if (toX >= Map::Terrain::_maxX || toX <= Map::Terrain::_minX || toY <= Map::Terrain::_minY || toY >= Map::Terrain::_maxY)
        return false;

    float distance = CalculateDistance(
        m_session->GetPlayer()->GetPositionX(),
        m_session->GetPlayer()->GetPositionY(),
        m_session->GetPlayer()->GetPositionZ(),
        toX, toY, toZ);

    m_session->SystemMessage("Your distance to location (%f, %f, %f) is %0.2f meters.", toX, toY, toZ, distance);

    return true;
}

bool ChatHandler::HandleRangeCheckCommand(const char* /*args*/, WorldSession* m_session)
{
    uint64 guid = m_session->GetPlayer()->getTargetGuid();
    m_session->SystemMessage("=== RANGE CHECK ===");
    if (guid == 0)
    {
        m_session->SystemMessage("No selection.");
        return true;
    }

    Unit* unit = m_session->GetPlayer()->getWorldMap()->getUnit(guid);
    if (!unit)
    {
        m_session->SystemMessage("Invalid selection.");
        return true;
    }
    float DistSq = unit->getDistanceSq(m_session->GetPlayer());
    m_session->SystemMessage("getDistanceSq  :   %u", float2int32(DistSq));
    LocationVector locvec(m_session->GetPlayer()->GetPositionX(), m_session->GetPlayer()->GetPositionY(), m_session->GetPlayer()->GetPositionZ());
    float DistReal = unit->CalcDistance(locvec);
    m_session->SystemMessage("CalcDistance   :   %u", float2int32(DistReal));
    float Dist2DSq = unit->GetDistance2dSq(m_session->GetPlayer());
    m_session->SystemMessage("GetDistance2dSq:   %u", float2int32(Dist2DSq));
    return true;
}

bool ChatHandler::HandleCollisionTestIndoor(const char* /*args*/, WorldSession* m_session)
{
    if (worldConfig.terrainCollision.isCollisionEnabled)
    {
        Player* plr = m_session->GetPlayer();
        const LocationVector & loc = plr->GetPosition();
        bool res = !MapManagement::AreaManagement::AreaStorage::IsOutdoor(plr->GetMapId(), loc.x, loc.y, loc.z + 2.0f);
        SystemMessage(m_session, "Result was: %s.", res ? "indoors" : "outside");
        return true;
    }
    else
    {
        SystemMessage(m_session, "Collision is not enabled.");
        return true;
    }
}

bool ChatHandler::HandleCollisionTestLOS(const char* /*args*/, WorldSession* m_session)
{
    if (worldConfig.terrainCollision.isCollisionEnabled)
    {
        Object* pObj = NULL;
        Creature* pCreature = GetSelectedCreature(m_session, false);
        Player* pPlayer = GetSelectedPlayer(m_session, true, true);
        if (pCreature)
            pObj = pCreature;
        else if (pPlayer)
            pObj = pPlayer;

        if (pObj == NULL)
        {
            SystemMessage(m_session, "Invalid target.");
            return true;
        }

        bool res = pObj->IsWithinLOSInMap(m_session->GetPlayer());

        SystemMessage(m_session, "Result was: %s.", res ? "in LOS" : "not in LOS");
        return true;
    }
    else
    {
        SystemMessage(m_session, "Collision is not enabled.");
        return true;
    }
}

bool ChatHandler::HandleCollisionGetHeight(const char* /*args*/, WorldSession* m_session)
{
    if (worldConfig.terrainCollision.isCollisionEnabled)
    {
        Player* plr = m_session->GetPlayer();
        float radius = 5.0f;

        float posX = plr->GetPositionX();
        float posY = plr->GetPositionY();
        float posZ = plr->GetPositionZ();
        float ori = plr->GetOrientation();

        LocationVector src(posX, posY, posZ);

        LocationVector dest(posX + (radius * (cosf(ori))), posY + (radius * (sinf(ori))), posZ);
        //LocationVector destest(posX+(radius*(cosf(ori))),posY+(radius*(sinf(ori))),posZ);

        const auto mgr = VMAP::VMapFactory::createOrGetVMapManager();
        float z = mgr->getHeight(plr->GetMapId(), posX, posY, posZ + 2.0f, 10000.0f);
        float z2 = mgr->getHeight(plr->GetMapId(), posX, posY, posZ + 5.0f, 10000.0f);
        float z3 = mgr->getHeight(plr->GetMapId(), posX, posY, posZ, 10000.0f);
        float z4 = plr->getWorldMap()->getGridHeight(plr->GetPositionX(), plr->GetPositionY());
        bool fp = mgr->getObjectHitPos(plr->GetMapId(), src.x, src.y, src.z, dest.x, dest.y, dest.z, dest.x, dest.y, dest.z, -1.5f);

        SystemMessage(m_session, "Results were: %f(offset2.0f) | %f(offset5.0f) | %f(org) | landheight:%f | target radius5 FP:%d", z, z2, z3, z4, fp);
        return true;
    }
    else
    {
        SystemMessage(m_session, "Collision is not enabled.");
        return true;
    }
}

bool ChatHandler::HandleGetDeathState(const char* /*args*/, WorldSession* m_session)
{
    Player* SelectedPlayer = GetSelectedPlayer(m_session, true, true);
    if (!SelectedPlayer)
        return true;

    SystemMessage(m_session, "Death State: %d", SelectedPlayer->getDeathState());
    return true;
}

struct spell_thingo
{
    uint32 type;
    uint32 target;
};

std::list<SpellInfo const*> aiagent_spells;
std::map<uint32, spell_thingo> aiagent_extra;

SpellCastTargets SetTargets(SpellInfo const* /*sp*/, uint32 /*type*/, uint32 targettype, Unit* dst, Creature* src)
{
    SpellCastTargets targets;
    targets.setUnitTarget(0);
    targets.setItemTarget(0);
    targets.setSource(LocationVector(0, 0, 0));
    targets.setDestination(LocationVector(0, 0, 0));

    if (targettype == TTYPE_SINGLETARGET)
    {
        targets.setTargetMask(TARGET_FLAG_UNIT);
        targets.setUnitTarget(dst->getGuid());
    }
    else if (targettype == TTYPE_SOURCE)
    {
        targets.setTargetMask(TARGET_FLAG_SOURCE_LOCATION);
        targets.setSource(src->GetPosition());
    }
    else if (targettype == TTYPE_DESTINATION)
    {
        targets.setTargetMask(TARGET_FLAG_DEST_LOCATION);
        targets.setDestination(dst->GetPosition());
    }

    return targets;
};

bool ChatHandler::HandleAIAgentDebugSkip(const char* args, WorldSession* m_session)
{
    uint32 count = atoi(args);
    if (!count) return false;

    for (uint32 i = 0; i < count; ++i)
    {
        if (!aiagent_spells.size())
            break;

        aiagent_spells.erase(aiagent_spells.begin());
    }
    BlueSystemMessage(m_session, "Erased %u spells.", count);
    return true;
}

bool ChatHandler::HandleAIAgentDebugContinue(const char* /*args*/, WorldSession* /*m_session*/)
{
    return true;
}

bool ChatHandler::HandleAIAgentDebugBegin(const char* /*args*/, WorldSession* m_session)
{
    QueryResult* result = WorldDatabase.Query("SELECT DISTINCT spell FROM ai_agents");
    if (!result) return false;

    do
    {
        SpellInfo const* se = sSpellMgr.getSpellInfo(result->Fetch()[0].GetUInt32());
        if (se)
            aiagent_spells.push_back(se);
    } while (result->NextRow());
    delete result;

    for (std::list<SpellInfo const*>::iterator itr = aiagent_spells.begin(); itr != aiagent_spells.end(); ++itr)
    {
        result = WorldDatabase.Query("SELECT * FROM ai_agents WHERE spell = %u", (*itr)->getId());
        if (result != nullptr)
        {
            spell_thingo t;
            t.type = result->Fetch()[6].GetUInt32();
            t.target = result->Fetch()[7].GetUInt32();
            delete result;
            aiagent_extra[(*itr)->getId()] = t;
        }
    }

    GreenSystemMessage(m_session, "Loaded %u spells for testing.", static_cast<uint32_t>(aiagent_spells.size()));
    return true;
}

bool ChatHandler::HandleCastSpellCommand(const char* args, WorldSession* m_session)
{
    Unit* caster = m_session->GetPlayer();
    Unit* target = GetSelectedPlayer(m_session, true, true);
    if (!target)
        target = GetSelectedCreature(m_session, false);
    if (!target)
    {
        RedSystemMessage(m_session, "Must select a char or creature.");
        return false;
    }

    uint32 spellid = atol(args);
    SpellInfo const* spellentry = sSpellMgr.getSpellInfo(spellid);
    if (!spellentry)
    {
        RedSystemMessage(m_session, "Invalid spell id!");
        return false;
    }

    Spell* sp = sSpellMgr.newSpell(caster, spellentry, false, NULL);

    BlueSystemMessage(m_session, "Casting spell %d on target.", spellid);
    SpellCastTargets targets(target->getGuid());
    sp->prepare(&targets);

    switch (target->getObjectTypeId())
    {
        case TYPEID_PLAYER:
            if (caster != target)
                sGMLog.writefromsession(m_session, "cast spell %d on PLAYER %s", spellid, static_cast< Player* >(target)->getName().c_str());
            break;
        case TYPEID_UNIT:
            sGMLog.writefromsession(m_session, "cast spell %d on CREATURE %u [%s], sqlid %u", spellid, target->getEntry(), static_cast< Creature* >(target)->GetCreatureProperties()->Name.c_str(), static_cast< Creature* >(target)->GetSQL_id());
            break;
    }

    return true;
}

bool ChatHandler::HandleCastSpellNECommand(const char* args, WorldSession* m_session)
{
    Unit* caster = m_session->GetPlayer();
    Unit* target = GetSelectedPlayer(m_session, true, true);
    if (!target)
        target = GetSelectedCreature(m_session, false);
    if (!target)
    {
        RedSystemMessage(m_session, "Must select a char or creature.");
        return false;
    }

    uint32 spellId = atol(args);
    SpellInfo const* spellentry = sSpellMgr.getSpellInfo(spellId);
    if (!spellentry)
    {
        RedSystemMessage(m_session, "Invalid spell id!");
        return false;
    }
    BlueSystemMessage(m_session, "Casting spell %d on target.", spellId);

    WorldPacket data;

    data.Initialize(SMSG_SPELL_START);
    data << caster->GetNewGUID();
    data << caster->GetNewGUID();
    data << spellId;
    data << uint8(0);
    data << uint16(0);
    data << uint32(0);
    data << uint16(2);
    data << target->getGuid();
    //        WPARCEMU_ASSERT(  data.size() == 36);
    m_session->SendPacket(&data);

    data.Initialize(SMSG_SPELL_GO);
    data << caster->GetNewGUID();
    data << caster->GetNewGUID();
    data << spellId;
    data << uint8(0) << uint8(1) << uint8(1);
    data << target->getGuid();
    data << uint8(0);
    data << uint16(2);
    data << target->getGuid();
    //        WPARCEMU_ASSERT(  data.size() == 42);
    m_session->SendPacket(&data);

    switch (target->getObjectTypeId())
    {
        case TYPEID_PLAYER:
            if (caster != target)
                sGMLog.writefromsession(m_session, "cast spell %d on PLAYER %s", spellId, static_cast< Player* >(target)->getName().c_str());
            break;
        case TYPEID_UNIT:
            sGMLog.writefromsession(m_session, "cast spell %d on CREATURE %u [%s], sqlid %u", spellId, target->getEntry(), static_cast< Creature* >(target)->GetCreatureProperties()->Name.c_str(), static_cast< Creature* >(target)->GetSQL_id());
            break;
    }

    return true;
}

bool ChatHandler::HandleCastSelfCommand(const char* args, WorldSession* m_session)
{
    Unit* target = GetSelectedPlayer(m_session, true, true);
    if (!target)
        target = GetSelectedCreature(m_session, false);
    if (!target)
    {
        RedSystemMessage(m_session, "Must select a char or creature.");
        return false;
    }

    uint32 spellid = atol(args);
    SpellInfo const* spellentry = sSpellMgr.getSpellInfo(spellid);
    if (!spellentry)
    {
        RedSystemMessage(m_session, "Invalid spell id!");
        return false;
    }

    Spell* sp = sSpellMgr.newSpell(target, spellentry, false, NULL);

    BlueSystemMessage(m_session, "Target is casting spell %d on himself.", spellid);
    SpellCastTargets targets(target->getGuid());
    sp->prepare(&targets);

    switch (target->getObjectTypeId())
    {
        case TYPEID_PLAYER:
            if (m_session->GetPlayer() != target)
                sGMLog.writefromsession(m_session, "used castself with spell %d on PLAYER %s", spellid, static_cast< Player* >(target)->getName().c_str());
            break;
        case TYPEID_UNIT:
            sGMLog.writefromsession(m_session, "used castself with spell %d on CREATURE %u [%s], sqlid %u", spellid, target->getEntry(), static_cast< Creature* >(target)->GetCreatureProperties()->Name.c_str(), static_cast< Creature* >(target)->GetSQL_id());
            break;
    }

    return true;
}
