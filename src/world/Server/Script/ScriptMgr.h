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

#pragma once

#include "Management/Gossip/GossipScript.hpp"
#include "Management/GameEventMgr.h"
#include "Objects/Units/Unit.h"
#include "Management/ArenaTeam.h"
#include "Map/Maps/InstanceMap.hpp"
#include "Server/Script/AchievementScript.hpp"
#include "Server/ServerState.h"
#include "Server/World.h"
#include "Spell/Definitions/ProcFlags.hpp"
#include "Spell/SpellAuras.h"
#include "Spell/SpellScript.hpp"
#include "ScriptEvent.hpp"

class Channel;
enum EncounterCreditType : uint8_t;
class Guild;
struct QuestProperties;

enum ServerHookEvents
{
    //////////////////////////////////////////////////////////////////////////////////////////   
    //Register Server Hooks
    // Server Hook callbacks can be made by using the function RegisterServerHook(EventId, function)

    SERVER_HOOK_EVENT_ON_NEW_CHARACTER                   = 1,  // -- (event, pName, int Race, int Class)
    SERVER_HOOK_EVENT_ON_KILL_PLAYER                     = 2,  // -- (event, pKiller, pVictim)
    SERVER_HOOK_EVENT_ON_FIRST_ENTER_WORLD               = 3,  // -- (event, pPlayer)                 / a new created character enters for first time the world
    SERVER_HOOK_EVENT_ON_ENTER_WORLD                     = 4,  // -- (event, pPlayer)                 / a character enters the world (login) or moves to another map
    SERVER_HOOK_EVENT_ON_GUILD_JOIN                      = 5,  // -- (event, pPlayer, str GuildName)
    SERVER_HOOK_EVENT_ON_DEATH                           = 6,  // -- (event, pPlayer)
    SERVER_HOOK_EVENT_ON_REPOP                           = 7,  // -- (event, pPlayer)
    SERVER_HOOK_EVENT_ON_EMOTE                           = 8,  // -- (event, pPlayer, pUnit, EmoteId)
    SERVER_HOOK_EVENT_ON_ENTER_COMBAT                    = 9,  // -- (event, pPlayer, pTarget)
    SERVER_HOOK_EVENT_ON_CAST_SPELL                      = 10, // -- (event, pPlayer, SpellId, pSpellObject)
    SERVER_HOOK_EVENT_ON_TICK                            = 11, // -- No arguments passed.
    SERVER_HOOK_EVENT_ON_LOGOUT_REQUEST                  = 12, // -- (event, pPlayer)
    SERVER_HOOK_EVENT_ON_LOGOUT                          = 13, // -- (event, pPlayer)
    SERVER_HOOK_EVENT_ON_QUEST_ACCEPT                    = 14, // -- (event, pPlayer, QuestId, pQuestGiver)
    SERVER_HOOK_EVENT_ON_ZONE                            = 15, // -- (event, pPlayer, ZoneId, OldZoneId)
    SERVER_HOOK_EVENT_ON_CHAT                            = 16, // -- (event, pPlayer, str Message, Type, Language, Misc)
    SERVER_HOOK_EVENT_ON_LOOT                            = 17, // -- (event, pPlayer, pTarget, Money, ItemId)
    SERVER_HOOK_EVENT_ON_GUILD_CREATE                    = 18, // -- (event, pPlayer, pGuildName)
    SERVER_HOOK_EVENT_ON_FULL_LOGIN                      = 19, // -- (event, pPlayer)                 / a character enters the world (login)
    SERVER_HOOK_EVENT_ON_CHARACTER_CREATE                = 20, // -- (event, pPlayer)
    SERVER_HOOK_EVENT_ON_QUEST_CANCELLED                 = 21, // -- (event, pPlayer, QuestId)
    SERVER_HOOK_EVENT_ON_QUEST_FINISHED                  = 22, // -- (event, pPlayer, QuestId, pQuestGiver)
    SERVER_HOOK_EVENT_ON_HONORABLE_KILL                  = 23, // -- (event, pPlayer, pKilled)
    SERVER_HOOK_EVENT_ON_ARENA_FINISH                    = 24, // -- (event, pPlayer, str TeamName, bWinner, bRated)
    SERVER_HOOK_EVENT_ON_OBJECTLOOT                      = 25, // -- (event, pPlayer, pTarget, Money, ItemId)
    SERVER_HOOK_EVENT_ON_AREATRIGGER                     = 26, // -- (event, pPlayer, AreaTriggerId)
    SERVER_HOOK_EVENT_ON_POST_LEVELUP                    = 27, // -- (event, pPlayer)
    SERVER_HOOK_EVENT_ON_PRE_DIE                         = 28, // -- (event, pKiller, pDied)          / general unit die, not only based on players
    SERVER_HOOK_EVENT_ON_ADVANCE_SKILLLINE               = 29, // -- (event, pPlayer, SkillId, SkillLevel)
    SERVER_HOOK_EVENT_ON_DUEL_FINISHED                   = 30, // -- (event, pWinner, pLoser)
    SERVER_HOOK_EVENT_ON_AURA_REMOVE                     = 31, // -- (event, pAuraObject)
    SERVER_HOOK_EVENT_ON_RESURRECT                       = 32, // -- (event, pPlayer)
    NUM_SERVER_HOOKS
};

enum ScriptTypes
{
    SCRIPT_TYPE_MISC            = 0x01,
    SCRIPT_TYPE_SCRIPT_ENGINE   = 0x20
};

// Hook typedefs
typedef bool(*tOnNewCharacter)(uint32 Race, uint32 Class, WorldSession* Session, const char* Name);
typedef void(*tOnKillPlayer)(Player* pPlayer, Player* pVictim);
typedef void(*tOCharacterCreate)(Player* pPlayer);
typedef void(*tOnFirstEnterWorld)(Player* pPlayer);
typedef void(*tOnEnterWorld)(Player* pPlayer);
typedef void(*tOnGuildCreate)(Player* pLeader, Guild* pGuild);
typedef void(*tOnGuildJoin)(Player* pPlayer, Guild* pGuild);
typedef void(*tOnDeath)(Player* pPlayer);
typedef bool(*tOnRepop)(Player* pPlayer);
typedef void(*tOnEmote)(Player* pPlayer, uint32 Emote, Unit* pUnit);
typedef void(*tOnEnterCombat)(Player* pPlayer, Unit* pTarget);
typedef bool(*tOnCastSpell)(Player* pPlayer, SpellInfo const* pSpell, Spell* spell);
typedef void(*tOnTick)();
typedef bool(*tOnLogoutRequest)(Player* pPlayer);
typedef void(*tOnLogout)(Player* pPlayer);
typedef void(*tOnQuestAccept)(Player* pPlayer, QuestProperties const* pQuest, Object* pQuestGiver);
typedef void(*tOnZone)(Player* pPlayer, uint32 Zone, uint32 oldzone);
typedef bool(*tOnChat)(Player* pPlayer, uint32 Type, uint32 Lang, const char* Message, const char* Misc);
typedef void(*tOnLoot)(Player* pPlayer, Unit* pTarget, uint32 Money, uint32 ItemId);
typedef bool(*ItemScript)(Item* pItem, Player* pPlayer);
typedef void(*tOnQuestCancel)(Player* pPlayer, QuestProperties const* pQuest);
typedef void(*tOnQuestFinished)(Player* pPlayer, QuestProperties const* pQuest, Object* pQuestGiver);
typedef void(*tOnHonorableKill)(Player* pPlayer, Player* pKilled);
typedef void(*tOnArenaFinish)(Player* pPlayer, ArenaTeam* pTeam, bool victory, bool rated);
typedef void(*tOnObjectLoot)(Player* pPlayer, Object* pTarget, uint32 Money, uint32 ItemId);
typedef void(*tOnAreaTrigger)(Player* pPlayer, uint32 areaTrigger);
typedef void(*tOnPostLevelUp)(Player* pPlayer);
typedef bool(*tOnPreUnitDie)(Unit* killer, Unit* target);
typedef void(*tOnAdvanceSkillLine)(Player* pPlayer, uint32 SkillLine, uint32 Current);
typedef void(*tOnDuelFinished)(Player* Winner, Player* Looser);
typedef void(*tOnAuraRemove)(Aura* aura);
typedef bool(*tOnResurrect)(Player* pPlayer);

class Spell;
class Aura;
class Creature;
class CreatureAIScript;
class EventScript;
class GameObjectAIScript;
class InstanceScript;
class ScriptMgr;
struct ItemProperties;
class QuestLogEntry;
class QuestScript;

// Factory Imports (from script lib)
typedef CreatureAIScript* (*exp_create_creature_ai)(Creature* pCreature);
typedef GameObjectAIScript* (*exp_create_gameobject_ai)(GameObject* pGameObject);
typedef InstanceScript* (*exp_create_instance_ai)(WorldMap* pMapMgr);

typedef bool(*exp_handle_dummy_spell)(uint8_t effectIndex, Spell* pSpell);
typedef bool(*exp_handle_script_effect)(uint8_t effectIndex, Spell* pSpell);
typedef bool(*exp_handle_dummy_aura)(uint8_t effectIndex, Aura* pAura, bool apply);

typedef void(*exp_script_register)(ScriptMgr* mgr);
typedef void(*exp_engine_reload)();
typedef void(*exp_engine_unload)();
typedef uint32(*exp_get_script_type)();
typedef const char*(*exp_get_version)();
typedef void(*exp_set_serverstate_singleton)(ServerState* state);

// Hashmap typedefs
typedef std::unordered_map<uint32, exp_create_creature_ai> CreatureCreateMap;
typedef std::unordered_map<uint32, exp_create_gameobject_ai> GameObjectCreateMap;
typedef std::unordered_map<uint32, exp_handle_dummy_aura> HandleDummyAuraMap;
typedef std::unordered_map<uint32, exp_handle_dummy_spell> HandleDummySpellMap;
typedef std::unordered_map< uint32, exp_handle_script_effect > HandleScriptEffectMap;
typedef std::unordered_map<uint32, exp_create_instance_ai> InstanceCreateMap;
typedef std::set<GossipScript*> CustomGossipScripts;
typedef std::unordered_map<uint32, GossipScript*> GossipMap;
typedef std::set<EventScript*> EventScripts;
typedef std::set<QuestScript*> QuestScripts;
typedef std::set<void*> ServerHookList;
typedef std::list< Arcemu::DynLib* > DynamicLibraryMap;

// APGL End
// MIT Start
#ifdef FT_ACHIEVEMENTS
typedef std::unordered_map<uint32_t, AchievementCriteriaScript*> AchievementCriteriaScripts;
#endif
typedef std::unordered_map<uint32_t, SpellScript*> SpellScripts;

class SERVER_DECL ScriptMgr
{
private:
    ScriptMgr() = default;
    ~ScriptMgr() = default;

public:
    static ScriptMgr& getInstance();

    ScriptMgr(ScriptMgr&&) = delete;
    ScriptMgr(ScriptMgr const&) = delete;
    ScriptMgr& operator=(ScriptMgr&&) = delete;
    ScriptMgr& operator=(ScriptMgr const&) = delete;

#ifdef FT_ACHIEVEMENTS
    // Achievement criteria script hooks
    bool callScriptedAchievementCriteriaCanComplete(uint32_t criteriaId, Player* player, Object* target) const;
#endif

    // Spell script hooks
    SpellCastResult callScriptedSpellCanCast(Spell* spell, uint32_t* parameter1, uint32_t* parameter2) const;
    void callScriptedSpellAtStartCasting(Spell* spell);
    void callScriptedSpellFilterTargets(Spell* spell, uint8_t effectIndex, std::vector<uint64_t>* effectTargets);
    void callScriptedSpellBeforeHit(Spell* spell, uint8_t effectIndex);
    void callScriptedSpellAfterMiss(Spell* spell, Unit* unitTarget);
    SpellScriptEffectDamage callScriptedSpellDoCalculateEffect(Spell* spell, uint8_t effectIndex, int32_t* damage) const;
    SpellScriptExecuteState callScriptedSpellBeforeSpellEffect(Spell* spell, uint8_t effectIndex) const;
    SpellScriptCheckDummy callScriptedSpellOnDummyOrScriptedEffect(Spell* spell, uint8_t effectIndex) const;
    void callScriptedSpellAfterSpellEffect(Spell* spell, uint8_t effectIndex);

    // Aura script hooks
    void callScriptedAuraOnCreate(Aura* aur);
    void callScriptedAuraOnApply(Aura* aur);
    void callScriptedAuraOnRemove(Aura* aur, AuraRemoveMode mode);
    void callScriptedAuraOnRefreshOrGainNewStack(Aura* aur, uint32_t newStackCount, uint32_t oldStackCount);
    SpellScriptExecuteState callScriptedAuraBeforeAuraEffect(Aura* aur, AuraEffectModifier* aurEff, bool apply) const;
    SpellScriptCheckDummy callScriptedAuraOnDummyEffect(Aura* aur, AuraEffectModifier* aurEff, bool apply) const;
    SpellScriptExecuteState callScriptedAuraOnPeriodicTick(Aura* aur, AuraEffectModifier* aurEff, float_t* damage) const;

    // Spell proc script hooks
    void callScriptedSpellProcCreate(SpellProc* spellProc, Object* obj);
    bool callScriptedSpellCanProc(SpellProc* spellProc, Unit* victim, SpellInfo const* castingSpell, DamageInfo damageInfo) const;
    bool callScriptedSpellCheckProcFlags(SpellProc* spellProc, SpellProcFlags procFlags) const;
    bool callScriptedSpellProcCanDelete(SpellProc* spellProc, uint32_t spellId, uint64_t casterGuid, uint64_t misc) const;
    SpellScriptExecuteState callScriptedSpellProcDoEffect(SpellProc* spellProc, Unit* victim, SpellInfo const* castingSpell, DamageInfo damageInfo) const;
    uint32_t callScriptedSpellCalcProcChance(SpellProc* spellProc, Unit* victim, SpellInfo const* castingSpell) const;
    bool callScriptedSpellCanProcOnTriggered(SpellProc* spellProc, Unit* victim, SpellInfo const* castingSpell, Aura* triggeredFromAura) const;
    SpellScriptExecuteState callScriptedSpellProcCastSpell(SpellProc* spellProc, Unit* caster, Unit* victim, Spell* spellToProc);

#ifdef FT_ACHIEVEMENTS
    AchievementCriteriaScript* getAchievementCriteriaScript(uint32_t criteriaId) const;
    void register_achievement_criteria_script(uint32_t criteriaId, AchievementCriteriaScript* acs);
    void register_achievement_criteria_script(uint32_t* criteriaIds, AchievementCriteriaScript* acs);
#endif

    SpellScript* getSpellScript(uint32_t spellId) const;
    // By default this will register spell script to spell's all different difficulties (if they exist)
    void register_spell_script(uint32_t spellId, SpellScript* ss, bool registerAllDifficulties = true);
    void register_spell_script(uint32_t* spellIds, SpellScript* ss);

    // Creature AI script hooks
    void DamageTaken(Creature* pCreature, Unit* attacker, uint32_t* damage) const;
    CreatureAIScript* getCreatureAIScript(Creature* pCreature) const;

protected:
#ifdef FT_ACHIEVEMENTS
    AchievementCriteriaScripts _achievementCriteriaScripts;
#endif
    SpellScripts _spellScripts;

private:
    void _register_spell_script(uint32_t spellId, SpellScript* ss);

public:
        // MIT End
        // APGL Start

        friend class HookInterface;

        void LoadScripts();
        void UnloadScripts();

        //////////////////////////////////////////////////////////////////////////////////////////
        /// Dumps the IDs of the spells with dummy/script effects or dummy aura
        /// that are not yet implemented.
        ///
        /// \param none   \return none
        ///
        //////////////////////////////////////////////////////////////////////////////////////////
        void DumpUnimplementedSpells();

        CreatureAIScript* CreateAIScriptClassForEntry(Creature* pCreature);
        GameObjectAIScript* CreateAIScriptClassForGameObject(uint32 uEntryId, GameObject* pGameObject);
        InstanceScript* CreateScriptClassForInstance(uint32 pMapId, WorldMap* pMapMgr);

        bool CallScriptedDummySpell(uint32 uSpellId, uint8_t effectIndex, Spell* pSpell);
        bool HandleScriptedSpellEffect(uint32 SpellId, uint8_t effectIndex, Spell* s);
        bool CallScriptedDummyAura(uint32 uSpellId, uint8_t effectIndex, Aura* pAura, bool apply);
        bool CallScriptedItem(Item* pItem, Player* pPlayer);

        //Single Entry Registers
        void register_creature_script(uint32 entry, exp_create_creature_ai callback);
        void register_gameobject_script(uint32 entry, exp_create_gameobject_ai callback);
        void register_dummy_aura(uint32 entry, exp_handle_dummy_aura callback);
        void register_dummy_spell(uint32 entry, exp_handle_dummy_spell callback);
        void register_script_effect(uint32 entry, exp_handle_script_effect callback);
        void register_instance_script(uint32 pMapId, exp_create_instance_ai pCallback);
        void register_hook(ServerHookEvents event, void* function_pointer);
        void register_quest_script(uint32 entry, QuestScript* qs);
        void register_event_script(uint32 entry, EventScript* es);

        // GOSSIP INTERFACE REGISTRATION
        void register_creature_gossip(uint32, GossipScript*);
        void register_item_gossip(uint32, GossipScript*);
        void register_go_gossip(uint32, GossipScript*);

        // Mutliple Entry Registers
        void register_creature_script(uint32* entries, exp_create_creature_ai callback);
        void register_gameobject_script(uint32* entries, exp_create_gameobject_ai callback);
        void register_dummy_aura(uint32* entries, exp_handle_dummy_aura callback);
        void register_dummy_spell(uint32* entries, exp_handle_dummy_spell callback);
        void register_script_effect(uint32* entries, exp_handle_script_effect callback);

        void ReloadScriptEngines();
        void UnloadScriptEngines();

        //////////////////////////////////////////////////////////////////////////////////////////
        // Purpose: Returns true if ScriptMgr has already registered the specified creature id.
        // Parameter: uint32 - the id of the creature to search for.
        //////////////////////////////////////////////////////////////////////////////////////////
        bool has_creature_script(uint32) const;

        //////////////////////////////////////////////////////////////////////////////////////////
        // Purpose: Returns true if ScriptMgr has already registered the specified gameobject id.
        // Parameter: uint32 - the id of the gameobject to search for
        //////////////////////////////////////////////////////////////////////////////////////////
        bool has_gameobject_script(uint32) const;

        //////////////////////////////////////////////////////////////////////////////////////////
        // Purpose: Returns true if ScriptMgr has already registered the specified aura id.
        // Parameter: uint32 - the aura id to search for
        //////////////////////////////////////////////////////////////////////////////////////////
        bool has_dummy_aura_script(uint32) const;

        //////////////////////////////////////////////////////////////////////////////////////////
        // Purpose: Returns true if ScriptMgr has already registered the specified dummy spell id.
        // Parameter: uint32 - the spell id to search for
        //////////////////////////////////////////////////////////////////////////////////////////
        bool has_dummy_spell_script(uint32) const;

        //////////////////////////////////////////////////////////////////////////////////////////
        // Purpose: Returns true if ScriptMgr has already registered the specified spell id.
        // Parameter: uint32 - the spell id to search for
        //////////////////////////////////////////////////////////////////////////////////////////
        bool has_script_effect(uint32) const;

        //////////////////////////////////////////////////////////////////////////////////////////
        // Purpose: Returns true if ScriptMgr has already registered the specified map id.
        // Parameter: uint32 - the map id to search for
        //////////////////////////////////////////////////////////////////////////////////////////
        bool has_instance_script(uint32) const;

        //////////////////////////////////////////////////////////////////////////////////////////
        // Purpose: Returns true if ScriptMgr has registered the specified function ptr to the specified event.
        // Parameter: ServerHookEvents - the event number
        // Parameter: void * - the function pointer to search for.
        //////////////////////////////////////////////////////////////////////////////////////////
        bool has_hook(ServerHookEvents, void*) const;

        //////////////////////////////////////////////////////////////////////////////////////////
        // Purpose: Returns true if ScriptMgr has already registered the specified quest id.
        // Parameter: uint32 - the quest id to search for
        //////////////////////////////////////////////////////////////////////////////////////////
        bool has_quest_script(uint32) const;

        bool has_creature_gossip(uint32) const;
        bool has_item_gossip(uint32) const;
        bool has_go_gossip(uint32) const;

        GossipScript* get_creature_gossip(uint32) const;
        GossipScript* get_go_gossip(uint32) const;
        GossipScript* get_item_gossip(uint32) const;

        // Default Gossip Script Interfaces
        GossipTrainer trainerScript_;
        GossipSpiritHealer spirithealerScript_;
        GossipBanker bankerScript_;
        GossipVendor vendorScript_;
        GossipClassTrainer classtrainerScript_;
        GossipPetTrainer pettrainerScript_;
        GossipFlightMaster flightmasterScript_;
        GossipAuctioneer auctioneerScript_;
        GossipInnKeeper innkeeperScript_;
        GossipBattleMaster battlemasterScript_;
        GossipCharterGiver chartergiverScript_;
        GossipTabardDesigner tabardScript_;
        GossipStableMaster stablemasterScript_;
        GossipGeneric genericScript_;

    protected:

        InstanceCreateMap mInstances;
        CreatureCreateMap _creatures;
        Mutex m_creaturesMutex;
        GameObjectCreateMap _gameobjects;
        HandleDummyAuraMap _auras;
        HandleDummySpellMap _spells;
        HandleScriptEffectMap SpellScriptEffects;
        DynamicLibraryMap dynamiclibs;
        ServerHookList _hooks[NUM_SERVER_HOOKS];
        CustomGossipScripts _customgossipscripts;
        EventScripts _eventscripts;
        QuestScripts _questscripts;
        GossipMap creaturegossip_, gogossip_, itemgossip_;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Base.h stuff
struct LocationExtra
{
    float x;
    float y;
    float z;
    float o;
    uint32_t addition;
};

enum TargetGenerator
{
    // Self
    TargetGen_Self,                         // Target self (Note: doesn't always mean self, also means the spell can choose various target)

    // Current
    TargetGen_Current,                      // Current highest aggro (attacking target)
    TargetGen_Destination,                  // Target is a destination coordinates (X, Y, Z)

    // Second most hated
    TargetGen_SecondMostHated,              // Second highest aggro

    // Predefined target
    TargetGen_Predefined,                   // Pre-defined target unit

    // Random Unit
    TargetGen_RandomUnit,                   // Random target unit (players, totems, pets, etc.)
    TargetGen_RandomUnitDestination,        // Random destination coordinates (X, Y, Z)
    TargetGen_RandomUnitApplyAura,          // Random target unit to self cast aura

    // Random Player
    TargetGen_RandomPlayer,                 // Random target player
    TargetGen_RandomPlayerDestination,      // Random player destination coordinates (X, Y, Z)
    TargetGen_RandomPlayerApplyAura         // Random target player to self cast aura
};

class TargetType;
class CreatureAIScript;
class Unit;

enum RangeStatus
{
    RangeStatus_Ok,
    RangeStatus_TooFar,
    RangeStatus_TooClose
};

typedef void(*EventFunc)(CreatureAIScript* pCreatureAI, int32_t pMiscVal);

typedef std::vector<Unit*> UnitArray;
typedef std::pair<RangeStatus, float> RangeStatusPair;

//////////////////////////////////////////////////////////////////////////////////////////
//Class TargetType
class SERVER_DECL TargetType
{
public:
    TargetType(uint32_t pTargetGen = TargetGen_Self, TargetFilter pTargetFilter = TargetFilter_None, uint32_t pMinTargetNumber = 0, uint32_t pMaxTargetNumber = 0);
    ~TargetType() = default;

    uint32_t mTargetGenerator;      // Defines what kind of target should we try to find
    TargetFilter mTargetFilter;     // Defines filter of target
    uint32_t mTargetNumber[2];      // 0: Defines min. number of creature on hatelist (0 - any, 1 - the most hated etc.)
                                    // 1: Defines max. number of creature on hatelist (0 - any, HateList.size + 1 - the least hated etc.)
};

class GameEvent;
class SERVER_DECL EventScript
{
    public:

        EventScript(){};
        virtual ~EventScript() {};

        virtual bool OnBeforeEventStart(GameEvent* /*pEvent*/, GameEventState /*pOldState*/) { return true; } // Before an event is about to be flagged as starting
        virtual void OnAfterEventStart(GameEvent* /*pEvent*/, GameEventState /*pOldState*/) {} // After an event has spawned all entities
        virtual bool OnBeforeEventStop(GameEvent* /*pEvent*/, GameEventState /*pOldState*/) { return true; } // Before an event is about to be flagged as stopping
        virtual void OnAfterEventStop(GameEvent* /*pEvent*/, GameEventState /*pOldState*/) { } // After an event has despawned all entities

        virtual GameEventState OnEventStateChange(GameEvent* /*pEvent*/, GameEventState /*pOldState*/, GameEventState pNewState) { return pNewState; } // When an event changes state

        virtual bool OnCreatureLoad(GameEvent* /*pEvent*/, Creature* /*pCreature*/) { return true; } // When a creature's data has been loaded, before it is spawned
        virtual void OnCreaturePushToWorld(GameEvent* /*pEvent*/, Creature* /*pCreature*/) {} // After a creature has been added to the world
        virtual void OnBeforeCreatureDespawn(GameEvent* /*pEvent*/, Creature* /*pCreature*/) {} // Before a creature is about to be despawned
        virtual void OnAfterCreatureDespawn(GameEvent* /*pEvent*/, Creature* /*pCreature*/) {} // After a creature has been despawned

        virtual bool OnGameObjectLoad(GameEvent* /*pEvent*/, GameObject* /*pGameObject*/) { return true; } // When a game object's data has been loaded, before it is spawned
        virtual void OnGameObjectPushToWorld(GameEvent* /*pEvent*/, GameObject* /*pGameObject*/) {} // After a game object has been added to the world
        virtual void OnBeforeGameObjectDespawn(GameEvent* /*pEvent*/, GameObject* /*pGameObject*/) {} // Before a game object is about to be despawned
        virtual void OnAfterGameObjectDespawn(GameEvent* /*pEvent*/, GameObject* /*pGameObject*/) {} // After a game object is about to be despawned

        // Standard virtual methods
        virtual void OnLoad() {}
        virtual void UpdateEvent() {}
        virtual void Destroy() {}

        // Data sharing between scripts
        virtual void setInstanceData(uint32 /*dataType*/, uint32 /*value*/) {}
        virtual uint32 getInstanceData(uint32 /*data*/) const { return 0;  }
        virtual void setGuidData(uint32 /*guidType*/, uint64 /*guidData*/) {}
        virtual uint64 getGuidData(uint32 /*guidType*/) const { return 0; }

        // UpdateEvent
        void RegisterUpdateEvent(uint32 pFrequency);
        void ModifyUpdateEvent(uint32 pNewFrequency);
        void RemoveUpdateEvent();
};

class SERVER_DECL GameObjectAIScript
{
    public:

        GameObjectAIScript(GameObject* goinstance);
        virtual ~GameObjectAIScript() {}

        virtual void OnCreate() {}
        virtual void OnSpawn() {}
        virtual void OnDespawn() {}
        virtual void OnLootTaken(Player* /*pLooter*/, ItemProperties const* /*pItemInfo*/) {}
        virtual void OnActivate(Player* /*pPlayer*/) {}
        virtual void OnDamaged(uint32 /*damage*/){}
        virtual void OnDestroyed(){}
        virtual void AIUpdate() {}
        virtual void Destroy() { delete this; }

        // Data sharing between scripts
        virtual void setGameObjectData(uint32 /*type*/) {}
        virtual uint32 getGameObjectData(uint32 /*type*/) const { return 0; }
        virtual void setGuidData(uint32 /*guidType*/, uint64 /*guidData*/) {}
        virtual uint64 getGuidData(uint32 /*guidType*/) const { return 0; }

        void RegisterAIUpdateEvent(uint32 frequency);
        void ModifyAIUpdateEvent(uint32 newfrequency);
        void RemoveAIUpdateEvent();

    protected:

        GameObject* _gameobject;
};

class SERVER_DECL QuestScript
{
    public:

        QuestScript() {};
        virtual ~QuestScript() {};

        virtual void OnQuestStart(Player* /*mTarget*/, QuestLogEntry* /*qLogEntry*/) {}
        virtual void OnQuestComplete(Player* /*mTarget*/, QuestLogEntry* /*qLogEntry*/) {}
        virtual void OnQuestCancel(Player* /*mTarget*/) {}
        virtual void OnGameObjectActivate(uint32 /*entry*/, Player* /*mTarget*/, QuestLogEntry* /*qLogEntry*/) {}
        virtual void OnCreatureKill(uint32 /*entry*/, Player* /*mTarget*/, QuestLogEntry* /*qLogEntry*/) {}
        virtual void OnExploreArea(uint32 /*areaId*/, Player* /*mTarget*/, QuestLogEntry* /*qLogEntry*/) {}
        virtual void OnPlayerItemPickup(uint32 /*itemId*/, uint32 /*totalCount*/, Player* /*mTarget*/, QuestLogEntry* /*qLogEntry*/) {}
};

//////////////////////////////////////////////////////////////////////////////////////////
// Instanced class created for each instance of the map, holds all scriptable exports

enum EncounterFrameType
{
#if VERSION_STRING > WotLK
    EncounterFrameSetCombatResLimit,
    EncounterFrameResetCombatResLimit,
#endif
    EncounterFrameEngage,
    EncounterFrameDisengaged,
    EncounterFrameUpdatePriority,
    EncounterFrameAddTimer,
    EncounterFrameEnableObjective,
    EncounterFrameUpdateObjective,
    EncounterFrameDisableObjective,
    EncounterFrameUnknown,
#if VERSION_STRING > WotLK
    EncounterFrameAddCombatResLimit,
#endif
};

enum EncounterStates : uint8_t
{
    NotStarted          = 0,
    InProgress          = 1,
    Failed              = 2,
    Performed           = 3,
    PreProgress         = 4,
    InvalidState        = 0xff
};

// Maybe Save more in future
struct BossInfo
{
    BossInfo() : state(InvalidState) {}
    uint32_t entry = 0;
    EncounterStates state;
};

typedef std::set<Creature*> CreatureSet;
typedef std::set<GameObject*> GameObjectSet;

typedef std::pair<uint32_t, uint32_t> InstanceTimerPair;
typedef std::vector<InstanceTimerPair> InstanceTimerArray;

const int32 INVALIDATE_TIMER = -1;
const uint32 DEFAULT_DESPAWN_TIMER = 2000;      //milliseconds

const uint32_t defaultUpdateFrequency = 1000;

class scriptEventMap;

class SERVER_DECL InstanceScript
{
    public:

        InstanceScript(WorldMap* pMapMgr);
        virtual ~InstanceScript() {}

        // Procedures that had been here before
        virtual GameObject* GetObjectForOpenLock(Player* /*pCaster*/, Spell* /*pSpell*/, SpellInfo const* /*pSpellEntry*/) { return NULL; }
        virtual void SetLockOptions(uint32 /*pEntryId*/, GameObject* /*pGameObject*/) {}
        virtual uint32 GetRespawnTimeForCreature(uint32 /*pEntryId*/, Creature* /*pCreature*/) { return 240000; }

        // Player
        virtual void OnPlayerDeath(Player* /*pVictim*/, Unit* /*pKiller*/) {}

        // Spawn Groups
        virtual void OnSpawnGroupKilled(uint32_t /*groupId*/) {}

        // Area and AreaTrigger
        virtual void OnPlayerEnter(Player* /*pPlayer*/) {}
        virtual void OnAreaTrigger(Player* /*pPlayer*/, uint32 /*pAreaId*/) {}
        virtual void OnZoneChange(Player* /*pPlayer*/, uint32 /*pNewZone*/, uint32 /*pOldZone*/) {}

        // Creature / GameObject - part of it is simple reimplementation for easier use Creature / GO < --- > Script
        virtual void OnCreatureDeath(Creature* /*pVictim*/, Unit* /*pKiller*/) {}
        virtual void OnCreaturePushToWorld(Creature* /*pCreature*/) {}
        virtual void OnGameObjectActivate(GameObject* /*pGameObject*/, Player* /*pPlayer*/) {}
        virtual void OnGameObjectPushToWorld(GameObject* /*pGameObject*/) {}

        // Standard virtual methods
        virtual void OnLoad() {}
        virtual void UpdateEvent() {}

        virtual void OnEncounterStateChange(uint32_t /*entry*/, uint32_t /*state*/) {}
        virtual void TransportBoarded(Unit* /*punit*/, Transporter* /*transport*/) {}
        virtual void TransportUnboarded(Unit* /*punit*/, Transporter* /*transport*/) {}

        virtual void Destroy() {}

        // Something to return Instance's MapMgr
        WorldMap* getWorldMap() { return mInstance; }
        InstanceMap* getInstance() { return mInstance->getInstance(); }
        uint8_t GetDifficulty() { return Difficulty; }

        // MIT start
        //////////////////////////////////////////////////////////////////////////////////////////
        // data

        // not saved to database, only for scripting
        virtual void setLocalData(uint32_t /*type*/, uint32_t /*data*/) {}
        virtual void setLocalData64(uint32_t /*type*/, uint64_t /*data*/) {}
        virtual uint32_t getLocalData(uint32_t /*type*/) const { return 0; }
        virtual Creature* getLocalCreatureData(uint32_t /*type*/) const { return nullptr; }
        virtual uint64_t getLocalData64(uint32_t /*type*/) const { return 0; }
        virtual void DoAction(int32_t /*action*/) {}
        virtual void TransporterEvents(Transporter* /*transport*/, uint32_t /*eventId*/) {}
        uint8_t Difficulty;

        void setZoneMusic(uint32_t zoneId, uint32_t musicId)
        {
            WorldPacket data(SMSG_PLAY_MUSIC, 4);
            data << uint32_t(musicId);
            sWorld.sendZoneMessage(&data, zoneId);
        }

        //////////////////////////////////////////////////////////////////////////////////////////
        // encounters

        // called for all initialized instancescripts!
        void generateBossDataState();
        void loadSavedInstanceData(char const* data);
        void sendUnitEncounter(uint32_t type, Unit* unit = nullptr, uint8_t value_a = 0, uint8_t value_b = 0);

        bool setBossState(uint32_t id, EncounterStates state);
        std::vector<BossInfo> getBosses() { return bosses; }
        EncounterStates getBossState(uint32_t id) const { return id < bosses.size() ? bosses[id].state : InvalidState; }
        //used for debug
        std::string getDataStateString(uint8_t state);

        uint32_t getEncounterCount() const { return static_cast<uint32_t>(bosses.size()); }

        void saveToDB();
        void updateEncounterState(EncounterCreditType type, uint32_t creditEntry);

        // Checks encounter state
        void updateEncountersStateForCreature(uint32_t creditEntry, uint8_t difficulty);
        void updateEncountersStateForSpell(uint32_t creditEntry, uint8_t difficulty);

        // Used only during loading
        void setCompletedEncountersMask(uint32_t newMask) { completedEncounters = newMask; }

        // Returns completed encounters mask for packets
        uint32_t getCompletedEncounterMask() const { return completedEncounters; }

        void readSaveDataBossStates(std::istringstream& data);
        void writeSaveDataBossStates(std::ostringstream& data);
        virtual std::string getSaveData();

        //used for debug
        void displayDataStateList(Player* player);

        void setBossNumber(uint32_t number) { bosses.resize(number); }

        //////////////////////////////////////////////////////////////////////////////////////////
        // timers

    private:

        InstanceTimerArray mTimers;
        uint32_t mTimerCount;

    public:

        uint32_t addTimer(uint32_t durationInMs);
        uint32_t getTimeForTimer(uint32_t timerId);
        uint32_t completedEncounters = 0; // completed encounter mask, bit indexes are DungeonEncounter.dbc boss numbers, used for packets
        void removeTimer(uint32_t& timerId);
        void resetTimer(uint32_t timerId, uint32_t durationInMs);
        bool isTimerFinished(uint32_t timerId);
        void cancelAllTimers();

        //only for internal use!
        void updateTimers();

        //used for debug
        void displayTimerList(Player* player);

        //////////////////////////////////////////////////////////////////////////////////////////
        // instance update

    private:

        uint32_t mUpdateFrequency;

    public:

        uint32_t getUpdateFrequency() { return mUpdateFrequency; }
        void setUpdateFrequency(uint32_t frequencyInMs) { mUpdateFrequency = frequencyInMs; }

        void registerUpdateEvent();
        void modifyUpdateEvent(uint32_t frequencyInMs);
        void removeUpdateEvent();

        //////////////////////////////////////////////////////////////////////////////////////////
        // script events
    protected:
        scriptEventMap scriptEvents;

        //////////////////////////////////////////////////////////////////////////////////////////
        // misc

    public:

        void setCellForcedStates(float xMin, float xMax, float yMin, float yMax, bool forceActive = true);

        Creature* spawnCreature(uint32_t entry, float posX, float posY, float posZ, float posO, uint32_t factionId = 0);
        Creature* getCreatureBySpawnId(uint32_t entry);
        Creature* GetCreatureByGuid(uint32_t guid);
        Creature* findNearestCreature(Object* pObject, uint32_t entry, float maxSearchRange /*= 250.0f*/);

        CreatureSet getCreatureSetForEntry(uint32_t entry, bool debug = false, Player* player = nullptr);
        CreatureSet getCreatureSetForEntries(std::vector<uint32_t> entryVector);

        GameObject* spawnGameObject(uint32_t entry, float posX, float posY, float posZ, float posO, bool addToWorld = true, uint32_t misc1 = 0, uint32_t phase = 0);
        GameObject* getGameObjectBySpawnId(uint32_t entry);
        GameObject* GetGameObjectByGuid(uint32_t guid);

        GameObject* getClosestGameObjectForPosition(uint32_t entry, float posX, float posY, float posZ);

        GameObjectSet getGameObjectsSetForEntry(uint32_t entry);

        float getRangeToObjectForPosition(Object* object, float posX, float posY, float posZ);

        void setGameObjectStateForEntry(uint32_t entry, uint8_t state);

    private:

        bool mSpawnsCreated;

    public:

        bool spawnsCreated() { return mSpawnsCreated; }
        void setSpawnsCreated(bool created = true) { mSpawnsCreated = created; }

    protected:

        std::vector<BossInfo> bosses;

        //MIT end

        WorldMap* mInstance;
};


class SERVER_DECL HookInterface
{
    private:

        HookInterface() = default;
        ~HookInterface() = default;

    public:

        static HookInterface& getInstance();

        HookInterface(HookInterface&&) = delete;
        HookInterface(HookInterface const&) = delete;
        HookInterface& operator=(HookInterface&&) = delete;
        HookInterface& operator=(HookInterface const&) = delete;

        friend class ScriptMgr;

        bool OnNewCharacter(uint32 Race, uint32 Class, WorldSession* Session, const char* Name);
        void OnKillPlayer(Player* pPlayer, Player* pVictim);
        void OnFirstEnterWorld(Player* pPlayer);
        void OnEnterWorld(Player* pPlayer);
        void OnGuildCreate(Player* pLeader, Guild* pGuild);
        void OnGuildJoin(Player* pPlayer, Guild* pGuild);
        void OnDeath(Player* pPlayer);
        bool OnRepop(Player* pPlayer);
        void OnEmote(Player* pPlayer, uint32 Emote, Unit* pUnit);
        void OnEnterCombat(Player* pPlayer, Unit* pTarget);
        bool OnCastSpell(Player* pPlayer, SpellInfo const* pSpell, Spell* spell);
        bool OnLogoutRequest(Player* pPlayer);
        void OnLogout(Player* pPlayer);
        void OnQuestAccept(Player* pPlayer, QuestProperties const* pQuest, Object* pQuestGiver);
        void OnZone(Player* pPlayer, uint32 Zone, uint32 oldZone);
        bool OnChat(Player* pPlayer, uint32 Type, uint32 Lang, const char* Message, const char* Misc);
        void OnLoot(Player* pPlayer, Unit* pTarget, uint32 Money, uint32 ItemId);
        void OnFullLogin(Player* pPlayer);
        void OnCharacterCreate(Player* pPlayer);
        void OnQuestCancelled(Player* pPlayer, QuestProperties const* pQuest);
        void OnQuestFinished(Player* pPlayer, QuestProperties const* pQuest, Object* pQuestGiver);
        void OnHonorableKill(Player* pPlayer, Player* pKilled);
        void OnArenaFinish(Player* pPlayer, ArenaTeam* pTeam, bool victory, bool rated);
        void OnObjectLoot(Player* pPlayer, Object* pTarget, uint32 Money, uint32 ItemId);
        void OnAreaTrigger(Player* pPlayer, uint32 areaTrigger);
        void OnPostLevelUp(Player* pPlayer);
        bool OnPreUnitDie(Unit* Killer, Unit* Victim);
        void OnAdvanceSkillLine(Player* pPlayer, uint32 SkillLine, uint32 Current);
        void OnDuelFinished(Player* Winner, Player* Looser);
        void OnAuraRemove(Aura* aura);
        bool OnResurrect(Player* pPlayer);
};

#define sScriptMgr ScriptMgr::getInstance()
#define sHookInterface HookInterface::getInstance()
