/*
Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CommonTypes.hpp"
#include "Spell/SpellMgr.hpp"
#include "Chat/ChatDefines.hpp"
#include "Map/Maps/InstanceDefines.hpp"
#include "Objects/Units/Creatures/AIInterface.h"
#include "ScriptMgr.h"
#include "ScriptEvent.hpp"
#include "Map/Management/MapMgr.hpp"
#include "Map/Maps/InstanceMap.hpp"
#include "Movement/WaypointDefines.h"

class Creature;

class SERVER_DECL SummonList
{
public:
    typedef std::list<uint64_t> StorageType;
    typedef StorageType::iterator iterator;
    typedef StorageType::const_iterator const_iterator;
    typedef StorageType::size_type size_type;
    typedef StorageType::value_type value_type;

    explicit SummonList(Creature* creature) : _creature(creature) { }

    iterator begin()
    {
        return _storage.begin();
    }

    const_iterator begin() const
    {
        return _storage.begin();
    }

    iterator end()
    {
        return _storage.end();
    }

    const_iterator end() const
    {
        return _storage.end();
    }

    iterator erase(iterator i)
    {
        return _storage.erase(i);
    }

    bool empty() const
    {
        return _storage.empty();
    }

    size_type size() const
    {
        return _storage.size();
    }

    void clear()
    {
        _storage.clear();
    }

    void summon(Creature const* summon);
    void despawn(Creature const* summon);
    void despawnEntry(uint32_t entry);
    void despawnAll();

    template <typename T>
    void despawnIf(T const& predicate)
    {
        _storage.remove_if(predicate);
    }

    void removeNotExisting();
    bool hasEntry(uint32_t entry) const;

    //////////////////////////////////////////////////////////////////////////////////////////
    // basic
private:
    Creature* _creature;
    StorageType _storage;
};

class SERVER_DECL CreatureAIScript
{
public:
    CreatureAIScript(Creature* creature);
    virtual ~CreatureAIScript();

    virtual void OnCombatStart(Unit* /*_target*/) {}
    virtual void OnCombatStop(Unit* /*_target*/) {}
    virtual void OnDamageTaken(Unit* /*_attacker*/, uint32_t /*_amount*/) {}
    virtual void DamageTaken(Unit* /*_attacker*/, uint32_t* /*damage*/) {} // Warning triggers before dmg applied, you can modify the damage done here
    virtual void OnCastSpell(uint32_t /*_spellId*/) {}
    virtual void OnSpellHitTarget(Object* /*target*/, SpellInfo const* /*info*/) {} // Triggers when a casted Spell Hits a Target
    virtual void OnTargetParried(Unit* /*_target*/) {}
    virtual void OnTargetDodged(Unit* /*_target*/) {}
    virtual void OnTargetBlocked(Unit* /*_target*/, int32 /*_amount*/) {}
    virtual void OnTargetCritHit(Unit* /*_target*/, int32 /*_amount*/) {}
    virtual void OnTargetDied(Unit* /*_target*/) {}
    virtual void OnParried(Unit* /*_target*/) {}
    virtual void OnDodged(Unit* /*_target*/) {}
    virtual void OnBlocked(Unit* /*_target*/, int32 /*_amount*/) {}
    virtual void OnCritHit(Unit* /*_target*/, int32 /*_amount*/) {}
    virtual void OnHit(Unit* /*_target*/, float /*_amount*/) {}
    virtual void OnDied(Unit* /*_killer*/) {}
    virtual void OnAssistTargetDied(Unit* /*_assistTarget*/) {}
    virtual void OnFear(Unit* /*_fearer*/, uint32_t /*_spellId*/) {}
    virtual void OnFlee(Unit* /*_flee*/) {}
    virtual void OnCallForHelp() {}
    virtual void OnLoad() {}
    virtual void OnDespawn() {}
    virtual void OnReachWP(uint32_t /*type*/, uint32_t /*id*/) {}
    virtual void OnLootTaken(Player* /*player*/, ItemProperties const* /*_itemProperties*/) {}
    virtual void AIUpdate() {}
    virtual void AIUpdate(unsigned long /*time_passed*/) {}
    virtual void OnEmote(Player* /*_player*/, EmoteType /*_emote*/) {}
    virtual void StringFunctionCall(int) {}
    virtual void onSummonedCreature(Creature* /*summon*/) {}
    virtual void OnSummon(Unit* /*summoner*/) {}
    virtual void OnSummonDies(Creature* /*summon*/, Unit* /*killer*/) {}

    // Quests
    virtual void onQuestAccept(Player* /*player*/, QuestProperties const* /*qst*/) {}
    virtual void onQuestRewarded(Player* /*player*/, QuestProperties const* /*qst*/) {}

    // Vehicles
    virtual void OnSpellClick(Unit* /*_clicker*/, bool /*spellClickHandled*/) { }
    virtual void OnVehicleInitialize() { }
    virtual void OnVehicleDeactivate() { }
    virtual void OnInstallAccessory(Creature* /*_accessory*/) { }
    virtual void OnAddPassenger(Unit* /*_passenger*/, int8_t /*_seatId*/) { }
    virtual void OnRemovePassenger(Unit* /*_passenger*/) { }

    virtual void OnScriptPhaseChange(uint32_t /*_phaseId*/) {}
    virtual void OnHitBySpell(uint32_t /*_spellId*/, Unit* /*_caster*/) {}

    // Data sharing between scripts
    virtual void SetCreatureData(uint32_t /*type*/, uint32_t /*data*/) {}
    virtual void SetCreatureData64(uint32_t /*type*/, uint64_t /*data*/) {}
    virtual uint32_t GetCreatureData(uint32_t /*type*/) const { return 0; }
    virtual uint64_t GetCreatureData64(uint32_t /*type*/) const { return 0; }
    virtual void DoAction(int32_t /*action*/) {}

    virtual void Destroy() { delete this; }

    //////////////////////////////////////////////////////////////////////////////////////////
    // Event default management
    // \brief: These functions are called internal for script events. Do NOT use them in your scripts!
    void _internalOnDied(Unit* killer);
    void _internalOnTargetDied(Unit* target);
    void _internalOnCombatStart(Unit* target);
    void _internalOnCombatStop();
    void _internalAIUpdate(unsigned long time_passed);
    void _internalOnScriptPhaseChange();

    //////////////////////////////////////////////////////////////////////////////////////////
    // player
    Player* getNearestPlayer();

    //////////////////////////////////////////////////////////////////////////////////////////
    // creature
    CreatureAIScript* getNearestCreatureAI(uint32_t entry);

    Creature* getNearestCreature(uint32_t entry);
    Creature* getNearestCreature(float posX, float posY, float posZ, uint32_t entry);

    void GetCreatureListWithEntryInGrid(std::list<Creature*>& container, uint32 entry, float maxSearchRange /*= 250.0f*/);
    Creature* findNearestCreature(uint32_t entry, float maxSearchRange /*= 250.0f*/);
    void GetGameObjectListWithEntryInGrid(std::list<GameObject*>& container, uint32 entry, float maxSearchRange /*= 250.0f*/);
    GameObject* findNearestGameObject(uint32_t entry, float maxSearchRange /*= 250.0f*/);

    float getRangeToObject(Object* object);

    CreatureAIScript* spawnCreatureAndGetAIScript(uint32_t entry, float posX, float posY, float posZ, float posO, uint32_t factionId = 0, uint32_t phase = 1);

    Creature* spawnCreature(uint32_t entry, LocationVector pos, uint32_t factionId = 0, uint32_t phase = 1);
    Creature* spawnCreature(uint32_t entry, float posX, float posY, float posZ, float posO, uint32_t factionId = 0, uint32_t phase = 1);
    void despawn(uint32_t delay = 2000, uint32_t respawnTime = 0);

    bool isAlive();

    //////////////////////////////////////////////////////////////////////////////////////////
    // AIAgent
    void setAIAgent(AI_Agent agent);
    uint8_t getAIAgent();

    //////////////////////////////////////////////////////////////////////////////////////////
    // movement
    void setRooted(bool set);
    bool isRooted();

    void setFlyMode(bool fly);

    // single point movement
    void moveTo(float posX, float posY, float posZ, bool setRun = true);
    void MoveTeleport(float posX, float posY, float posZ, float posO);
    void MoveTeleport(LocationVector loc);
    void moveToUnit(Unit* unit);
    void moveToSpawn();
    void stopMovement();

    //////////////////////////////////////////////////////////////////////////////////////////
    // wp movement
    WaypointNode createWaypoint(uint32_t pId, uint32_t pWaittime, uint32_t pMoveType, LocationVector pCoords);
    void addWaypoint(uint32_t pathid, WaypointNode pWayPoint);
    WaypointPath* getCustomPath(uint32_t pathId);

    void setWaypointToMove(uint32_t pathid, uint32_t pWaypointId);
    void stopWaypointMovement();

    // loads waypoints from database and initialise them
    void loadCustomWaypoins(uint32_t pathId);

    uint32_t getCurrentWaypoint();

    size_t getWaypointCount(uint32_t pathId);
    bool hasWaypoints(uint32_t pathId); //todo aaron02

private:
    std::unordered_map<uint32_t, WaypointPath> _waypointStore;

    //////////////////////////////////////////////////////////////////////////////////////////
    // combat setup
public:
    bool canEnterCombat();
    void setCanEnterCombat(bool enterCombat);
    bool _isInCombat();
    void _delayNextAttack(uint32_t milliseconds);

    void _setMeleeDisabled(bool disable);
    bool _isMeleeDisabled();
    void _setRangedDisabled(bool disable);
    bool _isRangedDisabled();
    void _setCastDisabled(bool disable);
    bool _isCastDisabled();
    void _setTargetingDisabled(bool disable);
    bool _isTargetingDisabled();

    void _clearHateList();
    void _wipeHateList();
    int32_t _getHealthPercent();
    int32_t _getManaPercent();
    void _regenerateHealth();

    bool _isCasting();

    //////////////////////////////////////////////////////////////////////////////////////////
    // script phase
    // \brief: script phase is reset to 0 in _internalOnDied() and _internalOnCombatStop()
private:
    uint32_t mScriptPhase;

public:
    uint32_t getScriptPhase();
    void setScriptPhase(uint32_t scriptPhase);
    void resetScriptPhase();
    bool isScriptPhase(uint32_t scriptPhase);

    //////////////////////////////////////////////////////////////////////////////////////////
     // script events
     // \brief: 
protected:
    scriptEventMap scriptEvents;
    SummonList summons;    

    //////////////////////////////////////////////////////////////////////////////////////////
    // timers
    //\brief: timers are stored and updated in InstanceScript if a instance script is
    //        available (instanceUpdateFrequency). If the creature is on a map without a
    //        instance script, the timer gets updated locale (AIUpdateFrequency).
private:
    //reference to instance time - used for creatures located on a map with a instance script.
    typedef std::list<uint32_t> creatureTimerIds;
    creatureTimerIds mCreatureTimerIds;

    //creature timer - used for creatures located on a map with NO instance script.
    typedef std::pair<uint32_t, uint32_t> CreatureTimerPair;
    typedef std::vector<CreatureTimerPair> CreatureTimerArray;

    CreatureTimerArray mCreatureTimer;

    uint32_t mCreatureTimerCount;

public:
    uint32_t _addTimer(uint32_t durationInMs);
    uint32_t _getTimeForTimer(uint32_t timerId);
    void _removeTimer(uint32_t& timerId);
    void _resetTimer(uint32_t timerId, uint32_t durationInMs);
    bool _isTimerFinished(uint32_t timerId);
    void _cancelAllTimers();

    uint32_t _getTimerCount();

    //only for internal use!
    void updateAITimers(unsigned long time_passed);

    //used for debug
    void displayCreatureTimerList(Player* player);

    //////////////////////////////////////////////////////////////////////////////////////////
    // ai upodate frequency
private:
    uint32_t mAIUpdateFrequency;

    uint32_t mCustomAIUpdateDelayTimerId;
    uint32_t mCustomAIUpdateDelay;

    SmallTimeTracker m_oldAIUpdate;

public:
    //old stuff
    void SetAIUpdateFreq(uint32_t pUpdateFreq);
    uint32_t GetAIUpdateFreq();

    void RegisterAIUpdateEvent(uint32_t frequency);
    void ModifyAIUpdateEvent(uint32_t newfrequency);
    void RemoveAIUpdateEvent();

    //////////////////////////////////////////////////////////////////////////////////////////
    // appearance
    void _setScale(float scale);
    float _getScale();
    void _setDisplayId(uint32_t displayId);
    void _setWieldWeapon(bool setWieldWeapon);
    void _setDisplayWeapon(bool setMainHand, bool setOffHand);
    void _setDisplayWeaponIds(uint32_t itemId1, uint32_t itemId2);

    //////////////////////////////////////////////////////////////////////////////////////////
    // spell
    typedef std::vector<CreatureAISpells*> CreatureAISpellsArray;
    CreatureAISpellsArray mCreatureAISpells;

public:
    //addAISpell(spellID, Chance, TargetType, Duration (s), waitBeforeNextCast (s))
    CreatureAISpells* addAISpell(uint32_t spellId, float castChance, uint32_t targetType, uint32_t duration = 0, uint32_t cooldown = 0, bool forceRemove = false, bool isTriggered = false);

    void _applyAura(uint32_t spellId);
    void _removeAura(uint32_t spellId);
    void _removeAllAuras();

    void _removeAuraOnPlayers(uint32_t spellId);
    void _castOnInrangePlayers(uint32_t spellId, bool triggered = false);
    void _castOnInrangePlayersWithinDist(float minDistance, float maxDistance, uint32_t spellId, bool triggered = false);

    void _castAISpell(CreatureAISpells* aiSpell);

    void _setTargetToChannel(Unit* target, uint32_t spellId);
    void _unsetTargetToChannel();
    Unit* _getTargetToChannel();

    // only for internal use
    void castSpellOnRandomTarget(CreatureAISpells* AiSpell);

    //////////////////////////////////////////////////////////////////////////////////////////
    // gameobject
    GameObject* getNearestGameObject(uint32_t entry);
    GameObject* getNearestGameObject(float posX, float posY, float posZ, uint32_t entry);

    //////////////////////////////////////////////////////////////////////////////////////////
    // chat message
    enum EmoteEventType
    {
        Event_OnCombatStart = 0,
        Event_OnTargetDied = 1,
        Event_OnDied = 2,
        Event_OnTaunt = 3,
        Event_OnIdle = 4     // new not part of db definitions!
    };

private:
    typedef std::vector<uint32_t> definedEmoteVector;
    definedEmoteVector mEmotesOnCombatStart;
    definedEmoteVector mEmotesOnTargetDied;
    definedEmoteVector mEmotesOnDied;
    definedEmoteVector mEmotesOnTaunt;
    definedEmoteVector mEmotesOnIdle;

public:
    void sendChatMessage(uint8_t type, uint32_t soundId, std::string text);
    void sendDBChatMessage(uint32_t textId, Unit* target = nullptr);

    void sendRandomDBChatMessage(std::vector<uint32_t> emoteVector, Unit* target);

    void addEmoteForEvent(uint32_t eventType, uint32_t scriptTextId);

    void sendAnnouncement(std::string stringAnnounce);

    //////////////////////////////////////////////////////////////////////////////////////////
    // idle emote timer
    // \brief: idle timer is seperated from custom timers. If isIdleEmoteEnabled is true,
    //         a random chat message is send by _internalAIUpdate stored in mEmotesOnIdle
private:
    bool isIdleEmoteEnabled;
    uint32_t idleEmoteTimerId;

    uint32_t idleEmoteTimeMin;
    uint32_t idleEmoteTimeMax;

public:
    void enableOnIdleEmote(bool enable, uint32_t durationInMs = 0);
    void setIdleEmoteTimerId(uint32_t timerId);
    uint32_t getIdleEmoteTimerId();
    void resetIdleEmoteTime(uint32_t durationInMs);

    void setRandomIdleEmoteTime(uint32_t minTime, uint32_t maxTime);
    void generateNextRandomIdleEmoteTime();

    //////////////////////////////////////////////////////////////////////////////////////////
    // basic
private:
    Creature* _creature;

public:
    Creature* getCreature() { return _creature; }

    //////////////////////////////////////////////////////////////////////////////////////////
    // instance
    InstanceScript* getInstanceScript();

    bool _isHeroic();

    template<class T> inline
        const T& RAID_MODE(const T& normal10, const T& normal25, const T& heroic10, const T& heroic25) const
    {
        if (_creature->getWorldMap()->getInstance())
        {
            switch (_creature->getWorldMap()->getDifficulty())
            {
            case InstanceDifficulty::RAID_10MAN_NORMAL:
                return normal10;
            case InstanceDifficulty::RAID_25MAN_NORMAL:
                return normal25;
            case InstanceDifficulty::RAID_10MAN_HEROIC:
                return heroic10;
            case InstanceDifficulty::RAID_25MAN_HEROIC:
                return heroic25;
            default:
                break;
            }
        }

        return normal10;
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // linked creature AI scripts
private:
    CreatureAIScript* linkedCreatureAI;

public:
    CreatureAIScript* getLinkedCreatureAIScript() { return linkedCreatureAI; }
    void setLinkedCreatureAIScript(CreatureAIScript* creatureAI);
    void removeLinkToCreatureAIScript();

    //////////////////////////////////////////////////////////////////////////////////////////
    // target
    Unit* getBestPlayerTarget(TargetFilter pFilter = TargetFilter_None, float pMinRange = 0.0f, float pMaxRange = 0.0f, int32_t auraId = 0);
    Unit* getBestUnitTarget(TargetFilter pFilter = TargetFilter_None, float pMinRange = 0.0f, float pMaxRange = 0.0f, int32_t auraid = 0);
    Unit* getBestTargetInArray(UnitArray& pTargetArray, TargetFilter pFilter);
    Unit* getNearestTargetInArray(UnitArray& pTargetArray);
    Unit* getSecondMostHatedTargetInArray(UnitArray& pTargetArray);
    bool isValidUnitTarget(Object* pObject, TargetFilter pFilter, float pMinRange = 0.0f, float pMaxRange = 0.0f, int32_t auraId = 0);
};
