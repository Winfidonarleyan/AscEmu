/*
Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "GameobjectFunctions.h"
#include "LuaMacros.h"
#include <Objects/GameObject.h>
#include <Map/Management/MapMgr.hpp>
#include "LuaGlobal.h"
#include "LUAEngine.h"
#include <Map/Maps/MapScriptInterface.h>
#include <Objects/Units/Creatures/Creature.h>
#include <Storage/MySQLDataStore.hpp>
#include <Spell/SpellMgr.hpp>
#include <Server/MainServerDefines.h>
#include "LuaHelpers.h"
#include "Management/WeatherMgr.h"

extern "C"
{
    #include <lua/lauxlib.h>
}

int LuaGameObject::GossipCreateMenu(lua_State* L, GameObject* ptr)
{
    int text_id = static_cast<int>(luaL_checkinteger(L, 1));
    Player* target = CHECK_PLAYER(L, 2);
    int autosend = static_cast<int>(luaL_checkinteger(L, 3));
    if (!target || !ptr)
        return 0;

    if (LuaGlobal::instance()->m_menu != NULL)
        delete LuaGlobal::instance()->m_menu;

    LuaGlobal::instance()->m_menu = new GossipMenu(ptr->getGuid(), text_id);

    if (autosend)
        LuaGlobal::instance()->m_menu->sendGossipPacket(target);

    return 0;
}

int LuaGameObject::GossipMenuAddItem(lua_State* L, GameObject* /*ptr*/)
{
    uint8_t icon = static_cast<uint8_t>(luaL_checkinteger(L, 1));
    const char* menu_text = luaL_checkstring(L, 2);
    int IntId = static_cast<int>(luaL_checkinteger(L, 3));
    bool coded = (luaL_checkinteger(L, 4)) ? true : false;
    const char* boxmessage = luaL_optstring(L, 5, "");
    uint32_t boxmoney = static_cast<uint32_t>(luaL_optinteger(L, 6, 0));

    if (LuaGlobal::instance()->m_menu == NULL)
    {
        DLLLogDetail("There is no menu to add items to!");
        return 0;
    }

    LuaGlobal::instance()->m_menu->addItem(icon, 0, IntId, menu_text, boxmoney, boxmessage, coded);
    return 0;
}

int LuaGameObject::GossipSendMenu(lua_State* L, GameObject* /*ptr*/)
{
    Player* target = CHECK_PLAYER(L, 1);
    if (!target)
        return 0;

    if (LuaGlobal::instance()->m_menu == NULL)
    {
        DLLLogDetail("There is no menu to send!");
        return 0;
    }

    LuaGlobal::instance()->m_menu->sendGossipPacket(target);

    return 0;
}

int LuaGameObject::GossipComplete(lua_State* L, GameObject* /*ptr*/)
{
    Player* target = CHECK_PLAYER(L, 1);
    if (!target)
        return 0;

    if (LuaGlobal::instance()->m_menu == NULL)
    {
        DLLLogDetail("There is no menu to complete!");
        return 0;
    }

    LuaGlobal::instance()->m_menu->senGossipComplete(target);

    return 0;
}

int LuaGameObject::GossipSendPOI(lua_State* L, GameObject* /*ptr*/)
{
    Player* plr = CHECK_PLAYER(L, 1);
    float x = CHECK_FLOAT(L, 2);
    float y = CHECK_FLOAT(L, 3);
    int icon = static_cast<int>(luaL_checkinteger(L, 4));
    int flags = static_cast<int>(luaL_checkinteger(L, 5));
    int data = static_cast<int>(luaL_checkinteger(L, 6));
    const char* name = luaL_checkstring(L, 7);
    if (!plr)
        return 0;

    plr->sendGossipPoiPacket(x, y, icon, flags, data, name);
    return 0;
}

int LuaGameObject::GossipSendQuickMenu(lua_State* L, GameObject* ptr)
{
    TEST_GO()

    uint32_t text_id = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    Player* player = CHECK_PLAYER(L, 2);
    uint32_t itemid = static_cast<uint32_t>(luaL_checkinteger(L, 3));
    uint8_t itemicon = CHECK_UINT8(L, 4);
    const char* itemtext = luaL_checkstring(L, 5);
    uint32_t requiredmoney = CHECK_ULONG(L, 6);
    const char* moneytext = luaL_checkstring(L, 7);
    uint8_t extra = CHECK_UINT8(L, 8);

    if (player == NULL)
        return 0;

    GossipMenu::sendQuickMenu(ptr->getGuid(), text_id, player, itemid, itemicon, itemtext, requiredmoney, moneytext, extra);

    return 0;
}

int LuaGameObject::RegisterAIUpdate(lua_State* L, GameObject* ptr)
{
    TEST_GO()
    uint32_t time = CHECK_ULONG(L, 1);
    sEventMgr.AddEvent(ptr, &GameObject::CallScriptUpdate, EVENT_SCRIPT_UPDATE_EVENT, time, 0, 0);
    return 0;
}

int LuaGameObject::ModAIUpdate(lua_State* L, GameObject* ptr)
{
    TEST_GO()
    uint32_t newtime = CHECK_ULONG(L, 1);
    sEventMgr.ModifyEventTimeAndTimeLeft(ptr, EVENT_SCRIPT_UPDATE_EVENT, newtime);
    return 0;
}

int LuaGameObject::RemoveAIUpdate(lua_State* /*L*/, GameObject* ptr)
{
    TEST_GO()
    sEventMgr.RemoveEvents(ptr, EVENT_SCRIPT_UPDATE_EVENT);
    return 0;
}

int LuaGameObject::GetMapId(lua_State* L, GameObject* ptr)
{
    (ptr != NULL) ? lua_pushinteger(L, ptr->GetMapId()) : lua_pushnil(L);
    return 1;
}

int LuaGameObject::RemoveFromWorld(lua_State* /*L*/, GameObject* ptr)
{
    if (ptr)
        ptr->RemoveFromWorld(true);
    return 0;
}

int LuaGameObject::GetName(lua_State* L, GameObject* ptr)
{
    TEST_GO()
    if (!ptr->GetGameObjectProperties())
        return 0;
    lua_pushstring(L, ptr->GetGameObjectProperties()->name.c_str());
    return 1;
}

int LuaGameObject::GetCreatureNearestCoords(lua_State* L, GameObject* ptr)
{
    TEST_GO_RET();
    float x = CHECK_FLOAT(L, 1);
    float y = CHECK_FLOAT(L, 2);
    float z = CHECK_FLOAT(L, 3);
    uint32_t entryid = CHECK_ULONG(L, 4);
    Creature* crc = ptr->getWorldMap()->getInterface()->getCreatureNearestCoords(x, y, z, entryid);
    if (crc && crc->isCreatureOrPlayer())
    {
        PUSH_UNIT(L, crc);
    }
    else
    {
        lua_pushnil(L);
    }
    return 1;
}

int LuaGameObject::GetGameObjectNearestCoords(lua_State* L, GameObject* ptr)
{
    TEST_GO_RET();
    float x = CHECK_FLOAT(L, 1);
    float y = CHECK_FLOAT(L, 2);
    float z = CHECK_FLOAT(L, 3);
    uint32_t entryid = CHECK_ULONG(L, 4);
    GameObject* go = ptr->getWorldMap()->getInterface()->getGameObjectNearestCoords(x, y, z, entryid);
    if (go != NULL)
    PUSH_GO(L, go);
    else
        lua_pushnil(L);
    return 1;
}

int LuaGameObject::GetClosestPlayer(lua_State* L, GameObject* ptr)
{
    TEST_GO();
    float d2 = 0;
    float dist = 0;
    Player* ret = nullptr;

    for (const auto& itr : ptr->getInRangePlayersSet())
    {
        if (itr)
        {
            d2 = static_cast<Player*>(itr)->getDistanceSq(ptr);
            if (!ret || d2 < dist)
            {
                dist = d2;
                ret = static_cast<Player*>(itr);
            }
        }
    }
    if (ret == nullptr)
        lua_pushnil(L);
    else
    PUSH_UNIT(L, ret);
    return 1;
}

int LuaGameObject::GetDistance(lua_State* L, GameObject* ptr)
{
    TEST_GO_RET();
    Object* target = CHECK_OBJECT(L, 1);
    lua_pushnumber(L, ptr->GetDistance2dSq(target));
    return 1;
}

int LuaGameObject::IsInWorld(lua_State* L, GameObject* ptr)
{
    if (ptr)
    {
        if (ptr->IsInWorld())
        {
            lua_pushboolean(L, 1);
            return 1;
        }
    }
    lua_pushboolean(L, 0);
    return 1;
}

int LuaGameObject::GetZoneId(lua_State* L, GameObject* ptr)
{
    TEST_GO_RET();
    lua_pushinteger(L, ptr->GetZoneId());
    return 1;
}

int LuaGameObject::PlaySoundToSet(lua_State* L, GameObject* ptr)
{
    if (!ptr)
        return 0;
    int soundid = static_cast<int>(luaL_checkinteger(L, 1));
    ptr->PlaySoundToSet(soundid);
    return 0;
}

int LuaGameObject::SpawnCreature(lua_State* L, GameObject* ptr)
{
    TEST_GO()
    uint32_t entry = CHECK_ULONG(L, 1);
    float x = CHECK_FLOAT(L, 2);
    float y = CHECK_FLOAT(L, 3);
    float z = CHECK_FLOAT(L, 4);
    float o = CHECK_FLOAT(L, 5);
    uint32_t faction = CHECK_ULONG(L, 6);
    uint32_t duration = CHECK_ULONG(L, 7);
    uint32_t equip1 = static_cast<uint32_t>(luaL_optinteger(L, 8, 1));
    uint32_t equip2 = static_cast<uint32_t>(luaL_optinteger(L, 9, 1));
    uint32_t equip3 = static_cast<uint32_t>(luaL_optinteger(L, 10, 1));
    uint32_t phase = static_cast<uint32_t>(luaL_optinteger(L, 11, ptr->m_phase));
    bool save = (luaL_optinteger(L, 12, 0) ? true : false);

    if (!entry)
    {
        lua_pushnil(L);
        return 1;
    }
    CreatureProperties const* p = sMySQLStore.getCreatureProperties(entry);
    if (p == nullptr)
    {
        lua_pushnil(L);
        return 1;
    }
    Creature* pCreature = ptr->getWorldMap()->createCreature(entry);
    if (pCreature == nullptr)
    {
        lua_pushnil(L);
        return 1;
    }
    pCreature->Load(p, x, y, z, o);
    pCreature->m_loadedFromDB = true;
    pCreature->setFaction(faction);
    pCreature->setVirtualItemSlotId(MELEE, equip1);
    pCreature->setVirtualItemSlotId(OFFHAND, equip2);
    pCreature->setVirtualItemSlotId(RANGED, equip3);
    pCreature->setPhase(PHASE_SET, phase);
    pCreature->m_noRespawn = true;
    pCreature->PushToWorld(ptr->getWorldMap());
    if (duration)
        pCreature->Despawn(duration, 0);
    if (save)
        pCreature->SaveToDB();
    PUSH_UNIT(L, pCreature);
    return 1;
}

int LuaGameObject::SpawnGameObject(lua_State* L, GameObject* ptr)
{
    TEST_GO();
    uint32_t entry_id = CHECK_ULONG(L, 1);
    float x = CHECK_FLOAT(L, 2);
    float y = CHECK_FLOAT(L, 3);
    float z = CHECK_FLOAT(L, 4);
    float o = CHECK_FLOAT(L, 5);
    uint32_t duration = CHECK_ULONG(L, 6);
    float scale = (float)(luaL_optinteger(L, 7, 100) / 100.0f);
    uint32_t phase = static_cast<uint32_t>(luaL_optinteger(L, 8, ptr->m_phase));
    bool save = luaL_optinteger(L, 9, 0) ? true : false;
    if (!entry_id)
        return 0;

    GameObject* go = ptr->getWorldMap()->createGameObject(entry_id);
    uint32_t mapid = ptr->GetMapId();
    go->create(entry_id, ptr->getWorldMap(), ptr->GetPhase(), LocationVector(x, y, z, o), QuaternionData(), GO_STATE_CLOSED);
    go->Phase(PHASE_SET, phase);
    go->setScale(scale);
    go->AddToWorld(ptr->getWorldMap());

    if (duration)
        go->despawn( duration, 0);
    if (save)
        go->saveToDB(true);
    PUSH_GO(L, go);
    return 1;
}

int LuaGameObject::GetSpawnX(lua_State* L, GameObject* ptr)
{
    TEST_GO()
    lua_pushnumber(L, ptr->GetSpawnX());
    return 1;
}

int LuaGameObject::GetSpawnY(lua_State* L, GameObject* ptr)
{
    TEST_GO()
    lua_pushnumber(L, ptr->GetSpawnY());
    return 1;
}

int LuaGameObject::GetSpawnZ(lua_State* L, GameObject* ptr)
{
    TEST_GO()
    lua_pushnumber(L, ptr->GetSpawnZ());
    return 1;
}

int LuaGameObject::GetSpawnO(lua_State* L, GameObject* ptr)
{
    TEST_GO()
    lua_pushnumber(L, ptr->GetSpawnO());
    return 1;
}

int LuaGameObject::GetX(lua_State* L, GameObject* ptr)
{
    TEST_GO()
    lua_pushnumber(L, ptr->GetPositionX());
    return 1;
}

int LuaGameObject::GetY(lua_State* L, GameObject* ptr)
{
    TEST_GO()
    lua_pushnumber(L, ptr->GetPositionY());
    return 1;
}

int LuaGameObject::GetZ(lua_State* L, GameObject* ptr)
{
    TEST_GO()
    if (ptr)
        lua_pushnumber(L, ptr->GetPositionZ());
    return 1;
}

int LuaGameObject::GetO(lua_State* L, GameObject* ptr)
{
    TEST_GO()
    lua_pushnumber(L, ptr->GetOrientation());
    return 1;
}

int LuaGameObject::GetInRangePlayersCount(lua_State* L, GameObject* ptr)
{
    TEST_GO()
    lua_pushnumber(L, static_cast<lua_Number>(ptr->getInRangePlayersCount()));
    return 1;
}

int LuaGameObject::GetEntry(lua_State* L, GameObject* ptr)
{
    TEST_GO()
    lua_pushnumber(L, ptr->getEntry());
    return 1;
}

int LuaGameObject::SetOrientation(lua_State* L, GameObject* ptr)
{
    float newo = CHECK_FLOAT(L, 1);
    if (!newo | !ptr)
        return 0;
    ptr->SetOrientation(newo);
    return 0;
}

int LuaGameObject::CalcRadAngle(lua_State* L, GameObject* ptr)
{
    float x = CHECK_FLOAT(L, 1);
    float y = CHECK_FLOAT(L, 2);
    float x2 = CHECK_FLOAT(L, 3);
    float y2 = CHECK_FLOAT(L, 4);
    if (!x || !y || !x2 || !y2 || !ptr)
        return 0;
    lua_pushnumber(L, ptr->calcRadAngle(x, y, x2, y2));
    return 1;
}

int LuaGameObject::GetInstanceID(lua_State* L, GameObject* ptr)
{
    TEST_GO_RET();
    if (ptr->getWorldMap()->getBaseMap()->getMapInfo()->isNonInstanceMap())
        lua_pushnil(L);
    else
        lua_pushinteger(L, ptr->GetInstanceID());
    return 1;
}

int LuaGameObject::GetInRangePlayers(lua_State* L, GameObject* ptr)
{
    TEST_GO_RET();
    uint32_t count = 0;
    lua_newtable(L);
    for (const auto& itr : ptr->getInRangePlayersSet())
    {
        if (itr && itr->isCreatureOrPlayer())
        {
            count++ ,
            lua_pushinteger(L, count);
            PUSH_UNIT(L, itr);
            lua_rawset(L, -3);
        }
    }
    return 1;
}

int LuaGameObject::GetInRangeGameObjects(lua_State* L, GameObject* ptr)
{
    TEST_GO_RET();
    uint32_t count = 0;
    lua_newtable(L);
    for (const auto& itr : ptr->getInRangeObjectsSet())
    {
        if (itr && itr->isGameObject())
        {
            count++ ,
            lua_pushinteger(L, count);
            PUSH_GO(L, itr);
            lua_rawset(L, -3);
        }
    }
    return 1;
}

int LuaGameObject::GetInRangeUnits(lua_State* L, GameObject* ptr)
{
    TEST_GO();
    uint32_t count = 0;
    lua_newtable(L);
    for (const auto& itr : ptr->getInRangeObjectsSet())
    {
        if (itr && itr->isCreatureOrPlayer())
        {
            count++ ,
            lua_pushinteger(L, count);
            PUSH_UNIT(L, itr);
            lua_rawset(L, -3);
        }
    }
    return 1;
}

int LuaGameObject::IsInFront(lua_State* L, GameObject* ptr)
{
    TEST_GO_RET();
    Object* target = CHECK_OBJECT(L, 1);
    if (!target)
    {
        lua_pushnil(L);
        return 1;
    }
    if (ptr->isInFront(target))
        lua_pushboolean(L, 1);
    else
        lua_pushboolean(L, 0);
    return 1;
}

int LuaGameObject::IsInBack(lua_State* L, GameObject* ptr)
{
    TEST_GO_RET();
    Object* target = CHECK_OBJECT(L, 1);
    if (!target)
    {
        lua_pushnil(L);
        return 1;
    }
    if (ptr->isInBack(target))
        lua_pushboolean(L, 1);
    else
        lua_pushboolean(L, 0);
    return 1;
}

int LuaGameObject::GetUInt32Value(lua_State* /*L*/, GameObject* /*ptr*/)
{
    /*uint16_t field = static_cast<uint16_t>(luaL_checkinteger(L, 1));
    if (ptr && field > 0)
        lua_pushinteger(L, ptr->getUInt32Value(field));
    else
        lua_pushnil(L);*/
    return 1;
}

int LuaGameObject::GetUInt64Value(lua_State* /*L*/, GameObject* /*ptr*/)
{
    /*uint16_t field = static_cast<uint16_t>(luaL_checkinteger(L, 1));
    if (ptr && field)
    PUSH_GUID(L, ptr->getUInt64Value(field));
    else
        lua_pushnil(L);*/
    return 1;
}

int LuaGameObject::SetUInt32Value(lua_State* /*L*/, GameObject* /*ptr*/)
{
    /*uint16_t field = static_cast<uint16_t>(luaL_checkinteger(L, 1));
    uint32_t value = static_cast<uint32_t>(luaL_checkinteger(L, 2));
    if (ptr && field)
        ptr->setUInt32Value(field, value);*/
    return 0;
}

int LuaGameObject::SetUInt64Value(lua_State* /*L*/, GameObject* /*ptr*/)
{
    /*uint16_t field = static_cast<uint16_t>(luaL_checkinteger(L, 1));
    uint64_t guid = CHECK_GUID(L, 1);
    if (ptr && field)
        ptr->setUInt64Value(field, guid);*/
    return 0;
}

int LuaGameObject::SetFloatValue(lua_State* /*L*/, GameObject* /*ptr*/)
{
    /*uint16_t field = static_cast<uint16_t>(luaL_checkinteger(L, 1));
    float value = CHECK_FLOAT(L, 2);
    if (ptr)
        ptr->setFloatValue(field, value);*/
    return 0;
}

int LuaGameObject::RemoveFlag(lua_State* /*L*/, GameObject* /*ptr*/)
{
    /*uint16_t field = static_cast<uint16_t>(luaL_checkinteger(L, 1));
    uint32_t value = static_cast<uint32_t>(luaL_checkinteger(L, 2));
    if (ptr)
        ptr->RemoveFlag(field, value);*/
    return 0;
}

int LuaGameObject::SetFlag(lua_State* /*L*/, GameObject* /*ptr*/)
{
    /*uint16_t field = static_cast<uint16_t>(luaL_checkinteger(L, 1));
    uint32_t value = static_cast<uint32_t>(luaL_checkinteger(L, 2));
    if (ptr)
        ptr->SetFlag(field, value)*/;
    return 0;
}

int LuaGameObject::Update(lua_State* /*L*/, GameObject* ptr)
{
    //just despawns/respawns to update GO visuals
    //credits: Sadikum
    TEST_GO()
    auto* mapmgr = ptr->getWorldMap();
    uint32_t NewGuid = mapmgr->generateGameobjectGuid();
    ptr->RemoveFromWorld(true);
    ptr->SetNewGuid(NewGuid);
    ptr->PushToWorld(mapmgr);
    ptr->saveToDB();
    return 0;
}

int LuaGameObject::GetFloatValue(lua_State* /*L*/, GameObject* /*ptr*/)
{
    /*uint16_t field = static_cast<uint16_t>(luaL_checkinteger(L, 1));
    if (ptr && field)
        lua_pushnumber(L, ptr->getFloatValue(field));
    else
        lua_pushnil(L);*/
    return 1;
}

int LuaGameObject::ModUInt32Value(lua_State* /*L*/, GameObject* /*ptr*/)
{
    /*uint16_t field = static_cast<uint16_t>(luaL_checkinteger(L, 1));
    uint32_t value = static_cast<uint32_t>(luaL_checkinteger(L, 2));
    if (ptr && field)
        ptr->modInt32Value(field, value);*/
    return 0;
}

int LuaGameObject::CastSpell(lua_State* L, GameObject* ptr)
{
    TEST_GO()
    uint32_t sp = CHECK_ULONG(L, 1);
    if (sp)
    {
        Spell* tSpell = sSpellMgr.newSpell(ptr, sSpellMgr.getSpellInfo(sp), true, NULL);
        SpellCastTargets tar(ptr->getGuid());
        tSpell->prepare(&tar);
    }
    return 0;
}

int LuaGameObject::CastSpellOnTarget(lua_State* L, GameObject* ptr)
{
    TEST_GO()
    uint32_t sp = CHECK_ULONG(L, 1);
    Object* target = CHECK_OBJECT(L, 2);
    if (sp && target != NULL)
    {
        Spell* tSpell = sSpellMgr.newSpell(ptr, sSpellMgr.getSpellInfo(sp), true, NULL);
        SpellCastTargets spCastTargets(target->getGuid());
        tSpell->prepare(&spCastTargets);
    }
    return 0;
}

int LuaGameObject::GetLandHeight(lua_State* L, GameObject* ptr)
{
    float x = CHECK_FLOAT(L, 1);
    float y = CHECK_FLOAT(L, 2);
    if (!ptr || !x || !y)
        lua_pushnil(L);
    else
    {
        float lH = ptr->getWorldMap()->getGridHeight(x, y);
        lua_pushnumber(L, lH);
    }
    return 1;
}

int LuaGameObject::SetZoneWeather(lua_State* L, GameObject* /*ptr*/)
{
    const uint32_t zoneId = CHECK_ULONG(L, 1);
    const uint32_t type = CHECK_ULONG(L, 2);
    const float density = CHECK_FLOAT(L, 3);
    if (!zoneId)
        return 0;

    sWeatherMgr.sendWeatherForZone(type, density, zoneId);

    return 0;
}

int LuaGameObject::GetDistanceYards(lua_State* L, GameObject* ptr)
{
    Object* target = CHECK_OBJECT(L, 1);
    if (!ptr || !target)
        lua_pushnil(L);
    else
    {
        LocationVector vec = ptr->GetPosition();
        lua_pushnumber(L, (float)vec.Distance(target->GetPosition()));
    }
    return 1;
}

int LuaGameObject::PhaseSet(lua_State* L, GameObject* ptr)
{
    uint32_t newphase = CHECK_ULONG(L, 1);
    bool Save = (luaL_optinteger(L, 2, false) > 0 ? true : false);
    if (!ptr)
        return 0;
    ptr->Phase(PHASE_SET, newphase);
    if (ptr->m_spawn)
        ptr->m_spawn->phase = newphase;
    if (Save)
    {
        ptr->saveToDB();
        ptr->m_loadedFromDB = true;
    }
    return 0;
}

int LuaGameObject::PhaseAdd(lua_State* L, GameObject* ptr)
{
    uint32_t newphase = CHECK_ULONG(L, 1);
    bool Save = (luaL_optinteger(L, 2, false) > 0 ? true : false);
    if (!ptr)
        return 0;
    ptr->Phase(PHASE_ADD, newphase);
    if (ptr->m_spawn)
        ptr->m_spawn->phase |= newphase;
    if (Save)
    {
        ptr->saveToDB();
        ptr->m_loadedFromDB = true;
    }
    return 0;
}

int LuaGameObject::PhaseDelete(lua_State* L, GameObject* ptr)
{
    uint32_t newphase = CHECK_ULONG(L, 1);
    bool Save = (luaL_optinteger(L, 2, false) > 0 ? true : false);
    if (!ptr)
        return 0;
    ptr->Phase(PHASE_DEL, newphase);
    if (ptr->m_spawn)
        ptr->m_spawn->phase &= ~newphase;
    if (Save)
    {
        ptr->saveToDB();
        ptr->m_loadedFromDB = true;
    }
    return 0;
}

int LuaGameObject::GetPhase(lua_State* L, GameObject* ptr)
{
    if (ptr)
        lua_pushnumber(L, ptr->m_phase);
    else
        lua_pushnil(L);
    return 1;
}

int LuaGameObject::SendPacket(lua_State* L, GameObject* ptr)
{
    WorldPacket* data = CHECK_PACKET(L, 1);
    bool self = CHECK_BOOL(L, 2);
    if (ptr != NULL && data != NULL)
        ptr->sendMessageToSet(data, self);
    return 0;
}

int LuaGameObject::GetGUID(lua_State* L, GameObject* ptr)
{
    TEST_GO_RET();
    PUSH_GUID(L, ptr->getGuid());
    return 1;
}

int LuaGameObject::IsActive(lua_State* L, GameObject* ptr)
{
    TEST_GO_RET();
    if (ptr->getState())
    RET_BOOL(true)
    RET_BOOL(false)
}

int LuaGameObject::Activate(lua_State* L, GameObject* ptr)
{
    TEST_GO_RET();
    if (ptr->getState() == 1)
        ptr->setState(GO_STATE_OPEN);
    else
        ptr->setState(GO_STATE_CLOSED);
    ptr->removeFlags(GO_FLAG_NONSELECTABLE);
    RET_BOOL(true)
}

int LuaGameObject::DespawnObject(lua_State* L, GameObject* ptr)
{
    TEST_GO()
    int delay = static_cast<int>(luaL_checkinteger(L, 1));
    int respawntime = static_cast<int>(luaL_checkinteger(L, 2));
    if (!delay)
        delay = 1; //Delay 0 might cause bugs
    ptr->despawn(delay, respawntime);
    return 0;
}

int LuaGameObject::AddLoot(lua_State* L, GameObject* ptr)
{
    TEST_GO()
    if ((lua_gettop(L) != 3) || (lua_gettop(L) != 5))
        return 0;

    if (!ptr->IsLootable())
        return 0;
    GameObject_Lootable* lt = static_cast<GameObject_Lootable*>(ptr);

    uint32_t itemid = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    uint32_t mincount = static_cast<uint32_t>(luaL_checkinteger(L, 2));
    uint32_t maxcount = static_cast<uint32_t>(luaL_checkinteger(L, 3));
    std::vector<float> ichance;

    float chance = CHECK_FLOAT(L, 5);

    for (uint8_t i = 0; i == 3; i++)
        ichance.push_back(chance);

    bool perm = ((luaL_optinteger(L, 4, 0) == 1) ? true : false);
    if (perm)
    {
        QueryResult* result = WorldDatabase.Query("SELECT * FROM loot_gameobjects WHERE entryid = %u, itemid = %u", ptr->getEntry(), itemid);
        if (!result)
        WorldDatabase.Execute("REPLACE INTO loot_gameobjects VALUES (%u, %u, %f, 0, 0, 0, %u, %u )", ptr->getEntry(), itemid, chance, mincount, maxcount);
        delete result;
    }
    sLootMgr.addLoot(&lt->loot, itemid, ichance, mincount, maxcount, ptr->getWorldMap()->getDifficulty());
    return 0;
}

int LuaGameObject::GetDungeonDifficulty(lua_State* L, GameObject* ptr)
{
    MySQLStructure::MapInfo const* pMapinfo = sMySQLStore.getWorldMapInfo(ptr->GetMapId());
    if (pMapinfo) //this block = IsInInstace()
    {
        if (!pMapinfo->isNonInstanceMap())
        {
            lua_pushboolean(L, 0);
            return 1;
        }
    }
    WorldMap* pInstance = sMapMgr.findWorldMap(ptr->GetMapId(), ptr->GetInstanceID());
    if (pInstance != nullptr)
    {
        lua_pushnumber(L, pInstance->getDifficulty());
        return 1;
    }
    return 0;
}

int LuaGameObject::SetDungeonDifficulty(lua_State* L, GameObject* ptr)
{
    uint8_t difficulty = static_cast<uint8_t>(luaL_checkinteger(L, 1));
    MySQLStructure::MapInfo const* pMapinfo = sMySQLStore.getWorldMapInfo(ptr->GetMapId());
    if (pMapinfo) //this block = IsInInstace()
    {
        if (!pMapinfo->isNonInstanceMap())
        {
            lua_pushboolean(L, 0);
            return 1;
        }
    }
    WorldMap* pInstance = sMapMgr.findWorldMap(ptr->GetMapId(), ptr->GetInstanceID());
    if (pInstance != nullptr)
    {
        pInstance->setSpawnMode(difficulty);
        lua_pushboolean(L, 1);
        return 1;
    }
    return 0;
}

int LuaGameObject::HasFlag(lua_State* L, GameObject* ptr)
{
    TEST_GO_RET();
    /*uint16_t index = static_cast<uint16_t>(luaL_checkinteger(L, 1));
    uint32_t flag = static_cast<uint32_t>(luaL_checkinteger(L, 2));
    lua_pushboolean(L, ptr->HasFlag(index, flag) ? 1 : 0);*/
    return 1;
}

int LuaGameObject::IsInPhase(lua_State* L, GameObject* ptr)
{
    TEST_GO_RET();
    uint32_t phase = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    lua_pushboolean(L, ((ptr->m_phase & phase) != 0) ? 1 : 0);
    return 1;
}

int LuaGameObject::GetSpawnId(lua_State* L, GameObject* ptr)
{
    TEST_GO()
    lua_pushnumber(L, ptr->m_spawn != NULL ? ptr->m_spawn->id : 0);
    return 1;
}

int LuaGameObject::GetAreaId(lua_State* L, GameObject* ptr)
{
    TEST_GO_RET()

    auto area = ptr->GetArea();
    lua_pushnumber(L, area ? area->id : 0);
    return 1;
}

int LuaGameObject::SetPosition(lua_State* L, GameObject* ptr)
{
    TEST_GO_RET();
    auto* mapMgr = ptr->getWorldMap();
    uint32_t NewGuid = mapMgr->generateGameobjectGuid();
    ptr->RemoveFromWorld(true);
    ptr->SetNewGuid(NewGuid);
    float x = CHECK_FLOAT(L, 1);
    float y = CHECK_FLOAT(L, 2);
    float z = CHECK_FLOAT(L, 3);
    float o = CHECK_FLOAT(L, 4);
    bool save = luaL_optinteger(L, 5, 0) ? true : false;
    ptr->SetPosition(x, y, z, o);
    ptr->PushToWorld(mapMgr);
    if (save)
        ptr->saveToDB();
    RET_BOOL(true)
}

int LuaGameObject::GetObjectType(lua_State* L, GameObject* ptr)
{
    TEST_GO()
    RET_STRING("GameObject");
}

int LuaGameObject::ChangeScale(lua_State* L, GameObject* ptr)
{
    TEST_GO_RET();
    float nScale = CHECK_FLOAT(L, 1);
    bool updateNow = CHECK_BOOL(L, 2);
    nScale = (nScale <= 0) ? 1 : nScale;
    ptr->setScale(nScale);
    if (updateNow)
    {
        auto* mapMgr = ptr->getWorldMap();
        uint32_t nguid = mapMgr->generateGameobjectGuid();
        ptr->RemoveFromWorld(true);
        ptr->SetNewGuid(nguid);
        ptr->PushToWorld(mapMgr);
    }
    RET_BOOL(true)
}

int LuaGameObject::GetByte(lua_State* L, GameObject* ptr)
{
    TEST_GO()
    /*    uint16_t index = static_cast<uint16_t>(luaL_checkinteger(L, 1));
    uint8_t index2 = static_cast<uint8_t>(luaL_checkinteger(L, 2));
    uint8_t value = ptr->getByteValue(index, index2);
    RET_INT(value);*/
    RET_INT(0)
}

int LuaGameObject::SetByte(lua_State* L, GameObject* ptr)
{
    TEST_GO_RET();
    /*uint16_t index = static_cast<uint16_t>(luaL_checkinteger(L, 1));
    uint8_t index2 = static_cast<uint8_t>(luaL_checkinteger(L, 2));
    uint8_t value = static_cast<uint8_t>(luaL_checkinteger(L, 3));
    ptr->setByteValue(index, index2, value);*/
    RET_BOOL(true)
}

int LuaGameObject::FullCastSpellOnTarget(lua_State* L, GameObject* ptr)
{
    TEST_GO()
        uint32_t sp = CHECK_ULONG(L, 1);
    Object* target = CHECK_OBJECT(L, 2);
    if (sp && target != NULL)
    {
        Spell* tSpell = sSpellMgr.newSpell(ptr, sSpellMgr.getSpellInfo(sp), false, NULL);
        SpellCastTargets sct(target->getGuid());
        tSpell->prepare(&sct);
    }
    return 0;
}

int LuaGameObject::FullCastSpell(lua_State* L, GameObject* ptr)
{
    TEST_GO()
    uint32_t sp = CHECK_ULONG(L, 1);
    if (sp)
    {
        Spell* tSpell = sSpellMgr.newSpell(ptr, sSpellMgr.getSpellInfo(sp), false, NULL);
        SpellCastTargets sct(ptr->getGuid());
        tSpell->prepare(&sct);
    }
    return 0;
}

int LuaGameObject::CustomAnimate(lua_State* L, GameObject* ptr)
{
    TEST_GO()
    uint32_t aindex = CHECK_ULONG(L, 1);
    if (aindex < 2 && ptr != NULL)
    {
        ptr->sendGameobjectCustomAnim(aindex);
        RET_BOOL(true)
    }
    RET_BOOL(false)
}

int LuaGameObject::GetLocation(lua_State* L, GameObject* ptr)
{
    TEST_GO()

    lua_pushnumber(L, ptr->GetPositionX());
    lua_pushnumber(L, ptr->GetPositionY());
    lua_pushnumber(L, ptr->GetPositionZ());
    lua_pushnumber(L, ptr->GetOrientation());
    return 4;
}

int LuaGameObject::GetSpawnLocation(lua_State* L, GameObject* ptr)
{
    TEST_GO();
    lua_pushnumber(L, ptr->GetPositionX());
    lua_pushnumber(L, ptr->GetPositionY());
    lua_pushnumber(L, ptr->GetPositionZ());
    lua_pushnumber(L, ptr->GetOrientation());
    return 4;
}

int LuaGameObject::GetWoWObject(lua_State* L, GameObject* ptr)
{
    TEST_GO();
    uint64_t guid = CHECK_GUID(L, 1);
    Object* obj = ptr->getWorldMap()->getObject(guid);
    if (obj != NULL && obj->isCreatureOrPlayer())
    PUSH_UNIT(L, obj);
    else if (obj != NULL && obj->isGameObject())
    PUSH_GO(L, obj);
    else
        lua_pushnil(L);
    return 1;
}

int LuaGameObject::RegisterEvent(lua_State* L, GameObject* ptr)
{
    TEST_GO();
    const char* typeName = luaL_typename(L, 1);
    int delay = static_cast<int>(luaL_checkinteger(L, 2));
    int repeats = static_cast<int>(luaL_checkinteger(L, 3));
    if (!delay)
        return 0;

    lua_settop(L, 1);
    int functionRef = 0;
    if (!strcmp(typeName, "function"))
        functionRef = luaL_ref(L, LUA_REGISTRYINDEX);
    else if (!strcmp(typeName, "string"))
        functionRef = LuaHelpers::ExtractfRefFromCString(L, luaL_checkstring(L, 1));
    if (functionRef)
    {
        TimedEvent* ev = TimedEvent::Allocate(ptr, new CallbackP1<LuaEngine, int>(LuaGlobal::instance()->luaEngine().get(), &LuaEngine::CallFunctionByReference, functionRef), EVENT_LUA_GAMEOBJ_EVENTS, delay, repeats);
        ptr->event_AddEvent(ev);
        std::map<uint64_t, std::set<int>>& objRefs = LuaGlobal::instance()->luaEngine()->getObjectFunctionRefs();
        std::map<uint64_t, std::set<int>>::iterator itr = objRefs.find(ptr->getGuid());
        if (itr == objRefs.end())
        {
            std::set<int> refs;
            refs.insert(functionRef);
            objRefs.insert(make_pair(ptr->getGuid(), refs));
        }
        else
        {
            std::set<int>& refs = itr->second;
            refs.insert(functionRef);
        }
    }
    return 0;
}

int LuaGameObject::RemoveEvents(lua_State* L, GameObject* ptr)
{
    TEST_GO();
    sEventMgr.RemoveEvents(ptr, EVENT_LUA_GAMEOBJ_EVENTS);
    std::map<uint64_t, std::set<int>>& objRefs = LuaGlobal::instance()->luaEngine()->getObjectFunctionRefs();
    std::map<uint64_t, std::set<int>>::iterator itr = objRefs.find(ptr->getGuid());
    if (itr != objRefs.end())
    {
        std::set<int>& refs = itr->second;
        for (std::set<int>::iterator it = refs.begin(); it != refs.end(); ++it)
            luaL_unref(L, LUA_REGISTRYINDEX, (*it));
        refs.clear();
    }
    return 0;
}

int LuaGameObject::SetScale(lua_State* L, GameObject* ptr)
{
    TEST_GO();
    float scale = static_cast<float>(luaL_checknumber(L, 1));
    if (scale > 0)
        ptr->setScale(scale);
    return 0;
}

int LuaGameObject::GetScale(lua_State* L, GameObject* ptr)
{
    TEST_GO();
    lua_pushnumber(L, ptr->getScale());
    return 1;
}

int LuaGameObject::GetClosestUnit(lua_State* L, GameObject* ptr)
{
    TEST_GO()
    float closest_dist = 99999.99f;
    float current_dist = 0;
    Unit* ret = nullptr;
    for (const auto& itr : ptr->getInRangeObjectsSet())
    {
        if (!itr || !itr->isCreatureOrPlayer())
            continue;

        current_dist = ptr->GetDistance2dSq(itr);
        if (current_dist < closest_dist)
        {
            closest_dist = current_dist;
            ret = static_cast<Unit*>(itr);
        }
    }
    PUSH_UNIT(L, ret);
    return 1;
}

int LuaGameObject::Damage(lua_State* L, GameObject* ptr)
{
    TEST_GO();

    if (ptr->GetGameObjectProperties()->type != GAMEOBJECT_TYPE_DESTRUCTIBLE_BUILDING)
        return 0;

    if (lua_gettop(L) != 3)
        return 0;

    GameObject_Destructible* dt = static_cast<GameObject_Destructible*>(ptr);
    uint32_t damage = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    uint64_t guid = CHECK_GUID(L, 2);
    uint32_t spellid = static_cast<uint32_t>(luaL_checkinteger(L, 3));

    dt->Damage(damage, guid, guid, spellid);

    return 0;
}

int LuaGameObject::Rebuild(lua_State* /*L*/, GameObject* ptr)
{
    TEST_GO();

    if (ptr->GetGameObjectProperties()->type != GAMEOBJECT_TYPE_DESTRUCTIBLE_BUILDING)
        return 0;

    GameObject_Destructible* dt = static_cast<GameObject_Destructible*>(ptr);
    dt->Rebuild();

    return 1;
}

int LuaGameObject::GetHP(lua_State* L, GameObject* ptr)
{
    TEST_GO();

    if (ptr->GetGameObjectProperties()->type != GAMEOBJECT_TYPE_DESTRUCTIBLE_BUILDING)
        return 0;

    GameObject_Destructible* dt = static_cast<GameObject_Destructible*>(ptr);

    lua_pushinteger(L, dt->GetHP());

    return 1;
}

int LuaGameObject::GetMaxHP(lua_State* L, GameObject* ptr)
{
    TEST_GO();

    if (ptr->GetGameObjectProperties()->type != GAMEOBJECT_TYPE_DESTRUCTIBLE_BUILDING)
        return 0;

    GameObject_Destructible* dt = static_cast<GameObject_Destructible*>(ptr);

    lua_pushinteger(L, dt->GetMaxHP());

    return 1;
}

int LuaGameObject::GetWorldStateForZone(lua_State* L, GameObject* ptr)
{
    TEST_GO();

    if (lua_gettop(L) != 1)
        return 0;

    uint32_t field = static_cast<uint32_t>(luaL_checkinteger(L, 1));

    uint32_t zoneId;
    uint32_t areaId;
    uint32_t entry = 0;

   ptr->getWorldMap()->getZoneAndAreaId(ptr->GetPhase(), zoneId, areaId, ptr->GetPosition());

   if (zoneId == 0)
       entry = areaId;
   else
       entry = zoneId;

    if (entry == 0)
        return 0;

    uint32_t value
            = ptr->getWorldMap()->getWorldStatesHandler().GetWorldStateForZone(entry, 0, field);

    lua_pushinteger(L, value);

    return 1;
}

int LuaGameObject::SetWorldStateForZone(lua_State* L, GameObject* ptr)
{
    TEST_GO();

    if (lua_gettop(L) != 2)
        return 0;

    uint32_t field = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    uint32_t value = static_cast<uint32_t>(luaL_checkinteger(L, 2));

    uint32_t zoneId;
    uint32_t areaId;
    uint32_t entry = 0;

    ptr->getWorldMap()->getZoneAndAreaId(ptr->GetPhase(), zoneId, areaId, ptr->GetPosition());

    if (zoneId == 0)
        entry = areaId;
    else
        entry = zoneId;

    if (entry == 0)
        return 0;

    ptr->getWorldMap()->getWorldStatesHandler().SetWorldStateForZone(entry, 0, field, value);

    return 0;
}
