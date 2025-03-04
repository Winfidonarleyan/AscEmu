/*
Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Common.hpp"

struct WMOAreaTableTripple
{
    WMOAreaTableTripple(int32_t r, int32_t a, int32_t g) : groupId(g), rootId(r), adtId(a)
    { }

    bool operator <(const WMOAreaTableTripple & b) const
    {
        return memcmp(this, &b, sizeof(WMOAreaTableTripple)) < 0;
    }

    int32_t groupId;
    int32_t rootId;
    int32_t adtId;
};

struct DBCPosition3D
{
    float X;
    float Y;
    float Z;
};

enum MapTypes
{
    MAP_COMMON          = 0, // none
    MAP_INSTANCE        = 1, // party
    MAP_RAID            = 2, // raid
    MAP_BATTLEGROUND    = 3  // pvp
};

namespace DBC::Structures
{
    namespace
    {
        char const area_table_entry_format[] = "niiiixxxxxissssssssxixxxi";
        char const area_trigger_entry_format[] = "niffffffff";
        char const auction_house_format[] = "niiixxxxxxxxx";
        char const bank_bag_slot_prices_format[] = "ni";
        char const char_start_outfit_format[] = "dbbbXiiiiiiiiiiiixxxxxxxxxxxxxxxxxxxxxxxx"; //1.12.1
        char const char_titles_format[] = "nxssssssssssssssssxssssssssssssssssxi";
        char const chat_channels_format[] = "iixssssssssxxxxxxxxxx";
        char const chr_classes_format[] = "nxxixssssssssxxix"; // 1.12.1
        char const chr_races_format[] = "nxixiixxixxxxxixissssssssxxxx"; //1.12.1
        char const creature_display_info_format[] = "nixifxxxxxxx";
        char const creature_model_Data_format[] = "nisxfxxxxxxxxxxf";
        char const creature_family_format[] = "nfifiiiissssssssxx";
        char const creature_spell_data_format[] = "niiiiiiii";
        char const durability_costs_format[] = "niiiiiiiiiiiiiiiiiiiiiiiiiiiii";
        char const durability_quality_format[] = "nf";
        char const emotes_text_format[] = "nxiiiixixixxxxxxxxx";
        char const faction_format[] = "niiiiiiiiiiiiiiiiiissssssssxxxxxxxxxx";
        char const faction_template_format[] = "niiiiiiiiiiiii";
        char const game_object_display_info_format[] = "nsxxxxxxxxxx";
        char const gem_properties_format[] = "nixxi";
        char const gt_chance_to_melee_crit_format[] = "f";
        char const gt_chance_to_melee_crit_base_format[] = "f";
        char const gt_chance_to_spell_crit_format[] = "f";
        char const gt_chance_to_spell_crit_base_format[] = "f";
        char const gt_combat_ratings_format[] = "f";
        char const gt_oct_regen_hp_format[] = "f";
        char const gt_oct_regen_mp_format[] = "f";
        char const gt_regen_hp_per_spt_format[] = "f";
        char const gt_regen_mp_per_spt_format[] = "f";
        char const item_entry_format[] = "niii";
        char const item_extended_cost_format[] = "niiiiiiiiiiiii";
        char const item_random_properties_format[] = "nxiiixxxxxxxxxxx";
        char const item_random_suffix_format[] = "nssssssssssssssssxxiiiiii";
        char const item_set_format[] = "dssssssssxxxxxxxxxxxxxxxxxxiiiiiiiiiiiiiiiiii";
        char const lfg_dungeon_entry_format[] = "nssssssssssssssssxiiiiiiiiixxixixxxxxxxxxxxxxxxxx";
        char const liquid_type_entry_format[] = "niii";
        char const lock_format[] = "niiiiiiiiiiiiiiiiiiiiiiiixxxxxxxx";
        char const mail_template_format[] = "nxxxxxxxxx";
        char const map_format[] = "nxixssssssssxxxxxxxixxxxxxxxxxxxxxxxxxixxx";
        char const name_gen_format[] = "nsii";
        char const skill_line_format[] = "nixssssssssxxxxxxxxxxi";
        char const skill_line_ability_format[] = "niiiixxiiiiixxi";
        char const stable_slot_prices_format[] = "ni";
        char const spell_cast_times_format[] = "nixx";
        char const spell_duration_format[] = "niii";
        char const spell_entry_format[] = "niixiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiifxiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiffffffiiiiiiiiiiiiiiiiiiiiifffiiiiiiiiiiiifffiiiiissssssssxssssssssxxxxxxxxxxxxxxxxxxxiiiiiiiiiixfffxxx";
        char const spell_item_enchantment_format[] = "niiiiiixxxiiissssssssxii";
        char const spell_radius_format[] = "nfff";
        char const spell_range_format[] = "nffixxxxxxxxxxxxxxxxxx";
        char const spell_shapeshift_form_format[] = "nxxxxxxxxxxiix";
        char const summon_properties_format[] = "niiiii";
        char const talent_format[] = "niiiiiiiixxxxixxixxxi";
        char const talent_tab_format[] = "nxxxxxxxxxxxiix";
        char const taxi_nodes_format[] = "nifffssssssssxii";
        char const taxi_path_format[] = "niii";
        char const taxi_path_node_format[] = "diiifffii";
        char const transport_animation_format[] = "diifffx";
        char const vehicle_format[] = "niffffiiiiiiiifffffffffffffffssssfifiixx";
        char const vehicle_seat_format[] = "niiffffffffffiiiiiifffffffiiifffiiiiiiiffiiiiixxxxxxxxxxxx";
        char const wmo_area_table_format[] = "niiixxxxxiixxxxxxxxx";
        char const world_map_overlay_format[] = "nxiiiixxxxxxxxxxx";
    }

    #pragma pack(push, 1)
    struct AchievementCategoryEntry
    {
        uint32_t ID;                                                // 0
        uint32_t parentCategory;                                    // 1 -1 for main category
        const char* name;                                           // 2-17
        uint32_t name_flags;                                        // 18
        uint32_t sortOrder;                                         // 19
    };

    struct AreaTableEntry
    {
        uint32_t id;                                                // 0
        uint32_t map_id;                                            // 1
        uint32_t zone;                                              // 2 if 0 then it's zone, else it's zone id of this area
        uint32_t explore_flag;                                      // 3, main index
        uint32_t flags;                                             // 4, unknown value but 312 for all cities
                                                                    // 5-9 unused
        int32_t area_level;                                         // 10
        char* area_name[8];                                         // 11-26
                                                                    // 27, string flags, unused
        uint32_t team;                                              // 28
        uint32_t liquid_type_override;                              // 29-32 liquid override by type
    };

    struct AreaTriggerEntry
    {
        uint32_t id;                                                // 0
        uint32_t mapid;                                             // 1
        float x;                                                    // 2
        float y;                                                    // 3
        float z;                                                    // 4
        float box_radius;                                           // 5 radius
        float box_x;                                                // 6 extent x edge
        float box_y;                                                // 7 extent y edge
        float box_z;                                                // 8 extent z edge
        float box_o;                                                // 9 extent rotation by about z axis
    };

    struct AuctionHouseEntry
    {
        uint32_t id;                                                // 0
        uint32_t faction;                                           // 1
        uint32_t fee;                                               // 2
        uint32_t tax;                                               // 3
        //char* name[16];                                           // 4-19
        //uint32_t name_flags;                                      // 20
    };

    struct BankBagSlotPrices
    {
        uint32_t Id;                                                // 0
        uint32_t Price;                                             // 1
    };

    struct CharTitlesEntry
    {
        uint32_t ID;                                                // 0, title ids
        //uint32_t unk1;                                            // 1 flags?
        char* name_male[16];                                        // 2-17
        //uint32_t name_flag;                                       // 18 string flag, unused
        char* name_female[16];                                      // 19-34
        //const char* name2_flag;                                   // 35 string flag, unused
        uint32_t bit_index;                                         // 36 used in PLAYER_CHOSEN_TITLE and 1<<index in PLAYER__FIELD_KNOWN_TITLES
    };

#define OUTFIT_ITEMS 12

    struct CharStartOutfitEntry
    {
        //uint32_t Id;                                              // 0
        uint8_t Race;                                               // 1
        uint8_t Class;                                              // 2
        uint8_t Gender;                                             // 3
        //uint8_t Unused;                                           // 4
        int32_t ItemId[OUTFIT_ITEMS];                               // 5-16
        //int32_t ItemDisplayId[OUTFIT_ITEMS];                      // 17-28
        //int32_t ItemInventorySlot[OUTFIT_ITEMS];                  // 29-40
    };


    struct ChatChannelsEntry
    {
        uint32_t id;                                                // 0
        uint32_t flags;                                             // 1
        char* name_pattern[8];                                      // 3-18
        //uint32_t name_pattern_flags;                              // 19
        //char* channel_name[16];                                   // 20-35
        //uint32_t channel_name_flags;                              // 36
    };

    struct ChrClassesEntry
    {
        uint32_t class_id;                                          // 0
        //uint32_t unk1;                                            // 1
        //uint32_t unk2;                                            // 2
        uint32_t power_type;                                        // 3
        //uint32_t unk3;                                            // 4
        char* name[8];                                              // 5-12
        //uint32_t nameflags;                                       // 13
        //uint32_t unk4;                                            // 14
        uint32_t spellfamily;                                       // 15
        //uint32_t unk4;                                            // 16
    };

    struct ChrRacesEntry
    {
        uint32_t race_id;                                           // 0
        //uint32_t flags;                                           // 1
        uint32_t faction_id;                                        // 2
        //uint32_t unk1;                                            // 3
        uint32_t model_male;                                        // 4
        uint32_t model_female;                                      // 5
        //uint32_t unk2;                                            // 6
        //uint32_t unk3;                                            // 7
        uint32_t team_id;                                           // 8
        //uint32_t unk4[4];                                         // 9-12
        //uint32_t unk5;                                            // 13
        uint32_t start_taxi_mask;                                   // 14
        //uint32_t unk6;                                            // 15
        uint32_t cinematic_id;                                      // 16 CinematicSequences.dbc
        char* name[8];                                              // 17-24
        //uint32_t name_flags                                       // 25
        //uint32_t unk7[2]                                          // 26-27
        //uint32_t unk8;                                            // 28
    };

    struct CreatureDisplayInfoEntry
    {
        uint32_t ID;                                                // 0
        uint32_t ModelID;                                           // 1
        //uint32_t SoundID                                          // 2
        uint32_t ExtendedDisplayInfoID;                             // 3
        float CreatureModelScale;                                   // 4
                                                                    // 5 - 15 unk
    };

    enum CreatureModelDataFlags
    {
        CREATURE_MODEL_DATA_FLAGS_CAN_MOUNT = 0x00000080
    };

    struct CreatureModelDataEntry
    {
        uint32_t ID;                                                // 0
        uint32_t Flags;                                             // 1
        char const* ModelName;                                      // 2
        //uint32_t SizeClass;                                       // 3
        float ModelScale;                                           // 4 Used in calculation of unit collision data
        //int32_t BloodID;                                          // 5
        //int32_t FootprintTextureID;                               // 6
        //uint32_t FootprintTextureLength;                          // 7
        //uint32_t FootprintTextureWidth;                           // 8
        //float FootprintParticleScale;                             // 9
        //uint32_t FoleyMaterialID;                                 // 10
        //float FootstepShakeSize;                                  // 11
        //uint32_t DeathThudShakeSize;                              // 12
        //uint32_t SoundID;                                         // 13
        //float CollisionWidth;                                     // 14
        float CollisionHeight;                                      // 15

        inline bool hasFlag(CreatureModelDataFlags flag) const { return (Flags & flag) != 0; }
    };

    struct CreatureFamilyEntry
    {
        uint32_t ID;                                                // 0
        float minsize;                                              // 1
        uint32_t minlevel;                                          // 2
        float maxsize;                                              // 3
        uint32_t maxlevel;                                          // 4
        uint32_t skilline;                                          // 5
        uint32_t tameable;                                          // 6 second skill line - 270 Generic
        uint32_t petdietflags;                                      // 7
        char* name[8];                                              // 8-23
        //uint32_t nameflags;                                       // 24
        //uint32_t iconFile;                                        // 25
    };

    struct CreatureSpellDataEntry
    {
        uint32_t id;                                                // 0
        uint32_t Spells[3];                                         // 1-3
        uint32_t PHSpell;                                           // 4
        uint32_t Cooldowns[3];                                      // 5-7
        uint32_t PH;                                                // 8
    };

    struct DurabilityCostsEntry
    {
        uint32_t itemlevel;                                         // 0
        uint32_t modifier[29];                                      // 1-29
    };

    struct DurabilityQualityEntry
    {
        uint32_t id;                                                // 0
        float quality_modifier;                                     // 1
    };

    struct EmotesTextEntry
    {
        uint32_t Id;                                                // 0
        //uint32_t name;                                            // 1
        uint32_t textid;                                            // 2
        uint32_t textid2;                                           // 3
        uint32_t textid3;                                           // 4
        uint32_t textid4;                                           // 5
        //uint32_t unk1;                                            // 6
        uint32_t textid5;                                           // 7
        //uint32_t unk2;                                            // 8
        uint32_t textid6;                                           // 9
        //uint32_t unk3;                                            // 10
        //uint32_t unk4;                                            // 11
        //uint32_t unk5;                                            // 12
        //uint32_t unk6;                                            // 13
        //uint32_t unk7;                                            // 14
        //uint32_t unk8;                                            // 15
        //uint32_t unk9;                                            // 16
        //uint32_t unk10;                                           // 17
        //uint32_t unk11;                                           // 18
    };

    struct FactionEntry
    {
        uint32_t ID;                                                // 0
        int32_t RepListId;                                          // 1
        uint32_t RaceMask[4];                                       // 2-5
        uint32_t ClassMask[4];                                      // 6-9
        int32_t baseRepValue[4];                                    // 10-13
        uint32_t repFlags[4];                                       // 14-17
        uint32_t parentFaction;                                     // 18
        char* Name[8];                                              // 19-34
        //uint32_t name_flags;                                      // 35
        //uint32_t Description[16];                                 // 36-51
        //uint32_t description_flags;                               // 52
    };

    struct FactionTemplateEntry
    {
        uint32_t ID;                                                // 0
        uint32_t Faction;                                           // 1
        uint32_t FactionGroup;                                      // 2
        uint32_t Mask;                                              // 3
        uint32_t FriendlyMask;                                      // 4
        uint32_t HostileMask;                                       // 5
        uint32_t EnemyFactions[4];                                  // 6-9
        uint32_t FriendlyFactions[4];                               // 10-13
    };

    struct GameObjectDisplayInfoEntry
    {
        uint32_t Displayid;                                         // 0
        char* filename;                                             // 1
        //uint32_t unk1[10];                                        // 2-11
    };

    struct GemPropertiesEntry
    {
        uint32_t Entry;                                             // 0
        uint32_t EnchantmentID;                                     // 1
        //uint32_t unk1;                                            // 2 bool
        //uint32_t unk2;                                            // 3 bool
        uint32_t SocketMask;                                        // 4
    };

    struct GtChanceToMeleeCritEntry
    {
        float val;                                                  // 0
    };

    struct GtChanceToMeleeCritBaseEntry
    {
        float val;                                                  // 0
    };

    struct GtChanceToSpellCritEntry
    {
        float val;                                                  // 0
    };

    struct GtChanceToSpellCritBaseEntry
    {
        float val;                                                  // 0
    };

    struct GtCombatRatingsEntry
    {
        float val;                                                  // 0
    };

    struct GtOCTRegenHPEntry
    {
        float ratio;                                                // 0
    };

    struct GtOCTRegenMPEntry
    {
        float ratio;                                                // 0
    };

    struct GtRegenHPPerSptEntry
    {
        float ratio;                                                // 0 regen base
    };

    struct GtRegenMPPerSptEntry
    {
        float ratio;                                                // 0 regen base
    };

    struct ItemEntry
    {
        uint32_t ID;                                                // 0
        uint32_t DisplayId;                                         // 1
        uint32_t InventoryType;                                     // 2
        uint32_t Sheath;                                            // 3
    };

    struct ItemExtendedCostEntry
    {
        uint32_t costid;                                            // 0
        uint32_t honor_points;                                      // 1
        uint32_t arena_points;                                      // 2
        uint32_t item[5];                                           // 4-8
        uint32_t count[5];                                          // 9-13
        uint32_t personalrating;                                    // 14
    };

    struct ItemRandomPropertiesEntry
    {
        uint32_t ID;                                                // 0
        //uint32_t name1;                                           // 1
        uint32_t spells[3];                                         // 2-4
        //uint32_t unk1;                                            // 5
        //uint32_t unk2;                                            // 6
        //char* name_suffix[16];                                    // 7-22
        //uint32_t name_suffix_flags;                               // 23
    };

    struct ItemRandomSuffixEntry
    {
        uint32_t id;                                                // 0
        char* name_suffix[16];                                      // 1-16
        //uint32_t name_suffix_flags;                               // 17
        //uint32_t unk1;                                            // 18
        uint32_t enchantments[3];                                   // 19-21
        //uint32_t unk2[2];                                         // 22-23
        uint32_t prefixes[3];                                       // 24-26
        //uint32_t[2];                                              // 27-28
    };

    struct ItemSetEntry
    {
        //uint32_t id;                                              // 1
        char* name[8];                                              // 1-16 name (lang)
        //uint32_t localeflag;                                      // 17 constant
        //uint32_t itemid[10];                                      // 18-27 item set items
        //uint32_t unk[7];                                          // 28-34 all 0
        uint32_t SpellID[8];                                        // 35-42
        uint32_t itemscount[8];                                     // 43-50
        uint32_t RequiredSkillID;                                   // 51
        uint32_t RequiredSkillAmt;                                  // 52
    };

    struct LFGDungeonEntry
    {
        uint32_t ID;                                                // 0
        char* name[16];                                             // 1-17 Name lang
        uint32_t minlevel;                                          // 18
        uint32_t maxlevel;                                          // 19
        uint32_t reclevel;                                          // 20
        uint32_t recminlevel;                                       // 21
        uint32_t recmaxlevel;                                       // 22
        int32_t map;                                                // 23
        uint32_t difficulty;                                        // 24
        uint32_t flags;                                             // 25
        uint32_t type;                                              // 26
        //uint32_t  unk;                                            // 27
        //char* iconname;                                           // 28
        uint32_t expansion;                                         // 29
        //uint32_t unk4;                                            // 30
        uint32_t grouptype;                                         // 31
        //char* desc[16];                                           // 32-47 Description

        // Helpers
        uint32_t Entry() const { return ID + (type << 24); }
    };

    struct LiquidTypeEntry
    {
        uint32_t Id;                                                // 0
        uint32_t liquid_id;                                         // 1
        uint32_t Type;                                              // 2
        uint32_t SpellId;                                           // 3
    };

#define LOCK_NUM_CASES 8

    struct LockEntry
    {
        uint32_t Id;                                                // 0
        uint32_t locktype[LOCK_NUM_CASES];                          // 1-8 If this is 1, then the next lockmisc is an item ID, if it's 2, then it's an iRef to LockTypes.dbc.
        uint32_t lockmisc[LOCK_NUM_CASES];                          // 9-16 Item to unlock or iRef to LockTypes.dbc depending on the locktype.
        uint32_t minlockskill[LOCK_NUM_CASES];                      // 17-24 Required skill needed for lockmisc (if locktype = 2).
        //uint32_t action[LOCK_NUM_CASES];                          // 25-32 Something to do with direction / opening / closing.
    };

    struct MailTemplateEntry
    {
        uint32_t ID;                                                // 0
        //char* subject;                                            // 1
        //float unused1[15]                                         // 2-16
        //uint32_t flags1                                           // 17 name flags, unused
        //char* content;                                            // 18
        //float unused2[15]                                         // 19-34
        //uint32_t flags2                                           // 35 name flags, unused
    };

    struct MapEntry
    {
        uint32_t id;                                                // 0
        //char* name_internal;                                      // 1
        uint32_t map_type;                                          // 2
        //uint32_t is_pvp_zone;                                     // 3 -0 false -1 true
        char* map_name[8];                                          // 4-19
        //uint32_t name_flags;                                      // 20
        uint32_t linked_zone;                                       // 22 common zone for instance and continent map
        //char* horde_intro[16];                                    // 23-38 horde text for PvP Zones
        //uint32_t hordeIntro_flags;                                // 39
        //char* alliance_intro[16];                                 // 40-55 alliance text for PvP Zones
        //uint32_t allianceIntro_flags;                             // 56
        uint32_t multimap_id;                                       // 57
        //uint32_t battlefield_map_icon;                            // 58

        bool isDungeon() const { return map_type == MAP_INSTANCE || map_type == MAP_RAID; }
        bool isNonRaidDungeon() const { return map_type == MAP_INSTANCE; }
        bool instanceable() const { return map_type == MAP_INSTANCE || map_type == MAP_RAID || map_type == MAP_BATTLEGROUND; }
        bool isRaid() const { return map_type == MAP_RAID; }
        bool isBattleground() const { return map_type == MAP_BATTLEGROUND; }
        bool isBattlegroundOrArena() const { return map_type == MAP_BATTLEGROUND; }
        bool isWorldMap() const { return map_type == MAP_COMMON; }

        bool isContinent() const
        {
            return id == 0 || id == 1 || id == 530 || id == 571;
        }
    };

    struct MapDifficulty
    {
        MapDifficulty() : resetTime(0), maxPlayers(0), hasErrorMessage(false) { }
        MapDifficulty(uint32_t _resetTime, uint32_t _maxPlayers, bool _hasErrorMessage) : resetTime(_resetTime), maxPlayers(_maxPlayers), hasErrorMessage(_hasErrorMessage) { }

        uint32_t resetTime;
        uint32_t maxPlayers;
        bool hasErrorMessage;
    };

    struct NameGenEntry
    {
        uint32_t ID;                                                // 0
        char* Name;                                                 // 1
        uint32_t unk1;                                              // 2
        uint32_t type;                                              // 3
    };

    struct SkillLineEntry
    {
        uint32_t id;                                                // 0
        uint32_t type;                                              // 1
        //uint32_t skillCostsID;                                    // 2
        char* Name[8];                                              // 3-18
        //uint32_t NameFlags;                                       // 19
        //char* Description[16];                                    // 20-35
        //uint32_t DescriptionFlags;                                // 36
        uint32_t spell_icon;                                        // 37
    };

    struct SkillLineAbilityEntry
    {
        uint32_t Id;                                                // 0
        uint32_t skilline;                                          // 1 skill id
        uint32_t spell;                                             // 2
        uint32_t race_mask;                                         // 3
        uint32_t class_mask;                                        // 4
        //uint32_t excludeRace;                                     // 5
        //uint32_t excludeClass;                                    // 6
        uint32_t minSkillLineRank;                                  // 7 req skill value
        uint32_t next;                                              // 8
        uint32_t acquireMethod;                                     // 9 auto learn
        uint32_t grey;                                              // 10 max
        uint32_t green;                                             // 11 min
        //uint32_t abandonable;                                     // 12
        //uint32_t reqTP;                                           // 13
        uint32_t reqtrainpoints;                                    // 14
    };

    struct StableSlotPrices
    {
        uint32_t Id;                                                // 0
        uint32_t Price;                                             // 1
    };

    struct SpellCastTimesEntry
    {
        uint32_t ID;                                                // 0
        uint32_t CastTime;                                          // 1
        //uint32_t unk1;                                            // 2
        //uint32_t unk2;                                            // 3
    };

    struct SpellDurationEntry
    {
        uint32_t ID;                                                // 0
        int32_t Duration1;                                          // 1
        int32_t Duration2;                                          // 2
        int32_t Duration3;                                          // 3
    };

#define MAX_SPELL_EFFECTS 3
#define MAX_SPELL_REAGENTS 8
#define MAX_SPELL_TOTEMS 2

    struct SpellEntry
    {
        uint32_t Id;                                                // 0
        uint32_t School;                                            // 1 NOT in bitmask!
        uint32_t Category;                                          // 2
        //uint32_t castUI;                                          // 3 not used
        uint32_t DispelType;                                        // 4
        uint32_t MechanicsType;                                     // 5
        uint32_t Attributes;                                        // 6
        uint32_t AttributesEx;                                      // 7
        uint32_t AttributesExB;                                     // 8
        uint32_t AttributesExC;                                     // 9
        uint32_t AttributesExD;                                     // 10
        uint32_t Shapeshifts;                                       // 11
        uint32_t ShapeshiftsExcluded;                               // 12
        uint32_t Targets;                                           // 13
        uint32_t TargetCreatureType;                                // 14
        uint32_t RequiresSpellFocus;                                // 15
        uint32_t CasterAuraState;                                   // 16
        uint32_t TargetAuraState;                                   // 17
        uint32_t CastingTimeIndex;                                  // 18
        uint32_t RecoveryTime;                                      // 19
        uint32_t CategoryRecoveryTime;                              // 20
        uint32_t InterruptFlags;                                    // 21
        uint32_t AuraInterruptFlags;                                // 22
        uint32_t ChannelInterruptFlags;                             // 23
        uint32_t procFlags;                                         // 24
        uint32_t procChance;                                        // 25
        uint32_t procCharges;                                       // 26
        uint32_t maxLevel;                                          // 27
        uint32_t baseLevel;                                         // 28
        uint32_t spellLevel;                                        // 29
        uint32_t DurationIndex;                                     // 30
        int32_t powerType;                                          // 31
        uint32_t manaCost;                                          // 32
        uint32_t manaCostPerlevel;                                  // 33
        uint32_t manaPerSecond;                                     // 34
        uint32_t manaPerSecondPerLevel;                             // 35
        uint32_t rangeIndex;                                        // 36
        float speed;                                                // 37
        //uint32_t modalNextSpell;                                  // 38 not used
        uint32_t MaxStackAmount;                                    // 39
        uint32_t Totem[MAX_SPELL_TOTEMS];                           // 40 - 41
        int32_t Reagent[MAX_SPELL_REAGENTS];                        // 42 - 49
        uint32_t ReagentCount[MAX_SPELL_REAGENTS];                  // 50 - 57
        int32_t EquippedItemClass;                                  // 58
        int32_t EquippedItemSubClass;                               // 59
        int32_t EquippedItemInventoryTypeMask;                      // 60
        uint32_t Effect[MAX_SPELL_EFFECTS];                         // 61 - 63
        int32_t EffectDieSides[MAX_SPELL_EFFECTS];                  // 64 - 66
        uint32_t EffectBaseDice[MAX_SPELL_EFFECTS];                 // 67 - 69
        float EffectDicePerLevel[MAX_SPELL_EFFECTS];                // 70 - 72
        float EffectRealPointsPerLevel[MAX_SPELL_EFFECTS];          // 73 - 75
        int32_t EffectBasePoints[MAX_SPELL_EFFECTS];                // 76 - 78
        uint32_t EffectMechanic[MAX_SPELL_EFFECTS];                 // 79 - 81
        uint32_t EffectImplicitTargetA[MAX_SPELL_EFFECTS];          // 82 - 84
        uint32_t EffectImplicitTargetB[MAX_SPELL_EFFECTS];          // 85 - 87
        uint32_t EffectRadiusIndex[MAX_SPELL_EFFECTS];              // 88 - 90
        uint32_t EffectApplyAuraName[MAX_SPELL_EFFECTS];            // 91 - 93
        uint32_t EffectAmplitude[MAX_SPELL_EFFECTS];                // 94 - 96
        float EffectMultipleValue[MAX_SPELL_EFFECTS];               // 97 - 99
        uint32_t EffectChainTarget[MAX_SPELL_EFFECTS];              // 100 - 102
        uint32_t EffectItemType[MAX_SPELL_EFFECTS];                 // 107 - 105
        int32_t EffectMiscValue[MAX_SPELL_EFFECTS];                 // 106 - 108
        uint32_t EffectTriggerSpell[MAX_SPELL_EFFECTS];             // 109 - 111
        float EffectPointsPerComboPoint[MAX_SPELL_EFFECTS];         // 112 - 114
        uint32_t SpellVisual;                                       // 115
        uint32_t SpellVisual1;                                      // 116
        uint32_t spellIconID;                                       // 117
        uint32_t activeIconID;                                      // 118 activeIconID;
        uint32_t spellPriority;                                     // 119
        const char* Name[8];                                        // 120 - 127
        //uint32_t NameFlags;                                       // 128 not used
        const char* Rank[8];                                        // 129 - 136
        //uint32_t RankFlags;                                       // 137 not used
        //const char* Description[8];                               // 138 - 145 not used
        //uint32_t DescriptionFlags;                                // 146 not used
        //const char* BuffDescription[8];                           // 147 - 154 not used
        //uint32_t buffdescflags;                                   // 155 not used
        uint32_t ManaCostPercentage;                                // 156
        uint32_t StartRecoveryCategory;                             // 157
        uint32_t StartRecoveryTime;                                 // 158
        uint32_t MaxTargetLevel;                                    // 159
        uint32_t SpellFamilyName;                                   // 160
        uint32_t SpellFamilyFlags[2];                               // 161 - 162
        uint32_t MaxTargets;                                        // 163
        uint32_t DmgClass;                                          // 164
        uint32_t PreventionType;                                    // 165
        //int32_t StanceBarOrder;                                   // 166 not used
        float EffectDamageMultiplier[MAX_SPELL_EFFECTS];            // 167 - 169
        //uint32_t MinFactionID;                                    // 170 not used
        //uint32_t MinReputation;                                   // 171 not used
        //uint32_t RequiredAuraVision;                              // 172 not used
    };

    struct SpellItemEnchantmentEntry
    {
        uint32_t Id;                                                // 0
        uint32_t type[3];                                           // 1-3
        uint32_t min[3];                                            // 4-6 for combat, in practice min==max
        //uint32_t max[3];                                          // 7-9
        uint32_t spell[3];                                          // 10-12
        char* Name[8];                                              // 13-28
        //uint32_t NameFlags;                                       // 29
        uint32_t visual;                                            // 30 aura
        uint32_t EnchantGroups;                                     // 31 slot
    };

    struct SpellRadiusEntry
    {
        uint32_t ID;                                                // 0
        float radius_min;                                           // 1 Radius
        float radius_per_level;                                     // 2
        float radius_max;                                           // 3 Radius2
    };

    struct SpellRangeEntry
    {
        uint32_t ID;                                                // 0
        float minRange;                                             // 1
        float maxRange;                                             // 3
        uint32_t range_type;                                        // 4
        //char* name1[16]                                           // 6-21
        //uint32_t name1_falgs;                                     // 22
        //char* name2[16]                                           // 23-38
        //uint32_t name2_falgs;                                     // 39
    };

    struct SpellShapeshiftFormEntry
    {
        uint32_t id;                                                // 0
        //uint32_t button_pos;                                      // 1
        //char* name[16];                                           // 2-17
        //uint32_t name_flags;                                      // 18
        uint32_t Flags;                                             // 19
        uint32_t unit_type;                                         // 20
        //uint32_t unk1                                             // 21
    };

    struct SummonPropertiesEntry
    {
        uint32_t ID;                                                // 0
        uint32_t ControlType;                                       // 1
        uint32_t FactionID;                                         // 2
        uint32_t Type;                                              // 3
        uint32_t Slot;                                              // 4
        uint32_t Flags;                                             // 5
    };

    struct TalentEntry
    {
        uint32_t TalentID;                                          // 0
        uint32_t TalentTree;                                        // 1
        uint32_t Row;                                               // 2
        uint32_t Col;                                               // 3
        uint32_t RankID[5];                                         // 4-8
        //uint32_t unk[4];                                          // 9-12
        uint32_t DependsOn;                                         // 13
        //uint32_t unk1[2];                                         // 14-15
        uint32_t DependsOnRank;                                     // 16
        //uint32_t unk2[2];                                         // 17-18
        //uint32_t unk3;                                            // 19
        uint32_t DependsOnSpell;                                    // 20
    };

    struct TalentTabEntry
    {
        uint32_t TalentTabID;                                       // 0
        //char* Name[16];                                           // 1-16
        //uint32_t name_flags;                                      // 17
        //uint32_t unk4;                                            // 18
        //uint32_t unk5;                                            // 19
        uint32_t ClassMask;                                         // 20
        uint32_t TabPage;                                           // 21
        //char* InternalName;                                       // 22
    };

    struct TaxiNodesEntry
    {
        uint32_t id;                                                // 0
        uint32_t mapid;                                             // 1
        float x;                                                    // 2
        float y;                                                    // 3
        float z;                                                    // 4
        char* name[8];                                              // 5-21
        //uint32_t nameflags;                                       // 22
        uint32_t horde_mount;                                       // 23
        uint32_t alliance_mount;                                    // 24
    };

    struct TaxiPathEntry
    {
        uint32_t id;                                                // 0
        uint32_t from;                                              // 1
        uint32_t to;                                                // 2
        uint32_t price;                                             // 3
    };

    struct TaxiPathNodeEntry
    {
        //uint32_t id;                                              // 0
        uint32_t path;                                              // 1
        uint32_t seq;                                               // 2 nodeIndex
        uint32_t mapid;                                             // 3
        float x;                                                    // 4
        float y;                                                    // 5
        float z;                                                    // 6
        uint32_t flags;                                             // 7
        uint32_t waittime;                                          // 8
        //uint32_t arivalEventID;                                   // 9
        //uint32_t departureEventID;                                // 10
    };

    struct TransportAnimationEntry
    {
        //uint32_t Id;                                              // 0
        uint32_t TransportID;                                       // 1
        uint32_t TimeIndex;                                         // 2
        DBCPosition3D Pos;                                          // 3
        //uint32_t SequenceID;                                      // 4
    };

#define MAX_VEHICLE_SEATS 8

    struct VehicleEntry
    {
        uint32_t ID;                                                // 0
        uint32_t flags;                                             // 1
        float turnSpeed;                                            // 2
        float pitchSpeed;                                           // 3
        float pitchMin;                                             // 4
        float pitchMax;                                             // 5
        uint32_t seatID[MAX_VEHICLE_SEATS];                         // 6-13
        float mouseLookOffsetPitch;                                 // 14
        float cameraFadeDistScalarMin;                              // 15
        float cameraFadeDistScalarMax;                              // 16
        float cameraPitchOffset;                                    // 17
        float facingLimitRight;                                     // 18
        float facingLimitLeft;                                      // 19
        float msslTrgtTurnLingering;                                // 20
        float msslTrgtPitchLingering;                               // 21
        float msslTrgtMouseLingering;                               // 22
        float msslTrgtEndOpacity;                                   // 23
        float msslTrgtArcSpeed;                                     // 24
        float msslTrgtArcRepeat;                                    // 25
        float msslTrgtArcWidth;                                     // 26
        float msslTrgtImpactRadius[2];                              // 27-28
        char* msslTrgtArcTexture;                                   // 29
        char* msslTrgtImpactTexture;                                // 30
        char* msslTrgtImpactModel[2];                               // 31-32
        float cameraYawOffset;                                      // 33
        uint32_t uiLocomotionType;                                  // 34
        float msslTrgtImpactTexRadius;                              // 35
        uint32_t uiSeatIndicatorType;                               // 36
        uint32_t powerType;                                         // 37, new in 3.1
        //uint32_t unk1;                                            // 38
        //uint32_t unk2;                                            // 39
    };

    enum VehicleSeatFlags
    {
        VEHICLE_SEAT_FLAG_HIDE_PASSENGER             = 0x00000200,  // Passenger is hidden
        VEHICLE_SEAT_FLAG_UNK11                      = 0x00000400,
        VEHICLE_SEAT_FLAG_CAN_CONTROL                = 0x00000800,  // Lua_UnitInVehicleControlSeat
        VEHICLE_SEAT_FLAG_CAN_ATTACK                 = 0x00004000,  // Can attack, cast spells and use items from vehicle?
        VEHICLE_SEAT_FLAG_USABLE                     = 0x02000000,  // Lua_CanExitVehicle
        VEHICLE_SEAT_FLAG_CAN_SWITCH                 = 0x04000000,  // Lua_CanSwitchVehicleSeats
        VEHICLE_SEAT_FLAG_CAN_CAST                   = 0x20000000,  // Lua_UnitHasVehicleUI
    };

    enum VehicleSeatFlagsB
    {
        VEHICLE_SEAT_FLAG_B_NONE                     = 0x00000000,
        VEHICLE_SEAT_FLAG_B_USABLE_FORCED            = 0x00000002,
        VEHICLE_SEAT_FLAG_B_USABLE_FORCED_2          = 0x00000040,
        VEHICLE_SEAT_FLAG_B_USABLE_FORCED_3          = 0x00000100,
    };

    struct VehicleSeatEntry
    {
        uint32_t ID;                                                // 0
        uint32_t flags;                                             // 1
        int32_t attachmentID;                                       // 2
        float attachmentOffsetX;                                    // 3
        float attachmentOffsetY;                                    // 4
        float attachmentOffsetZ;                                    // 5
        float enterPreDelay;                                        // 6
        float enterSpeed;                                           // 7
        float enterGravity;                                         // 8
        float enterMinDuration;                                     // 9
        float enterMaxDuration;                                     // 10
        float enterMinArcHeight;                                    // 11
        float enterMaxArcHeight;                                    // 12
        int32_t enterAnimStart;                                     // 13
        int32_t enterAnimLoop;                                      // 14
        int32_t rideAnimStart;                                      // 15
        int32_t rideAnimLoop;                                       // 16
        int32_t rideUpperAnimStart;                                 // 17
        int32_t rideUpperAnimLoop;                                  // 18
        float exitPreDelay;                                         // 19
        float exitSpeed;                                            // 20
        float exitGravity;                                          // 21
        float exitMinDuration;                                      // 22
        float exitMaxDuration;                                      // 23
        float exitMinArcHeight;                                     // 24
        float exitMaxArcHeight;                                     // 25
        int32_t exitAnimStart;                                      // 26
        int32_t exitAnimLoop;                                       // 27
        int32_t exitAnimEnd;                                        // 28
        float passengerYaw;                                         // 29
        float passengerPitch;                                       // 30
        float passengerRoll;                                        // 31
        int32_t passengerAttachmentID;                              // 32
        int32_t vehicleEnterAnim;                                   // 33
        int32_t vehicleExitAnim;                                    // 34
        int32_t vehicleRideAnimLoop;                                // 35
        int32_t vehicleEnterAnimBone;                               // 36
        int32_t vehicleExitAnimBone;                                // 37
        int32_t vehicleRideAnimLoopBone;                            // 38
        float vehicleEnterAnimDelay;                                // 39
        float vehicleExitAnimDelay;                                 // 40
        uint32_t vehicleAbilityDisplay;                             // 41
        uint32_t enterUISoundID;                                    // 42
        uint32_t exitUISoundID;                                     // 43
        int32_t uiSkin;                                             // 44
        uint32_t flagsB;                                            // 45

        bool IsUsable() const
        {
            if ((flags & VEHICLE_SEAT_FLAG_USABLE) != 0)
                return true;
            else
                return false;
        }

        bool IsController() const
        {
            if ((flags & VEHICLE_SEAT_FLAG_CAN_CONTROL) != 0)
                return true;
            else
                return false;
        }

        bool HidesPassenger() const
        {
            if ((flags & VEHICLE_SEAT_FLAG_HIDE_PASSENGER) != 0)
                return true;
            else
                return false;
        }
    };

    struct WMOAreaTableEntry
    {
        uint32_t id;                                                // 0
        int32_t rootId;                                             // 1
        int32_t adtId;                                              // 2
        int32_t groupId;                                            // 3
        //uint32_t field4;                                          // 4
        //uint32_t field5;                                          // 5
        //uint32_t field6;                                          // 6
        //uint32_t field7;                                          // 7
        //uint32_t field8;                                          // 8
        uint32_t flags;                                             // 9
        uint32_t areaId;                                            // 10 ref -> AreaTableEntry
        //char Name[16];                                            // 11-26
        //uint32_t nameflags;                                       // 27
    };

    struct WorldMapOverlayEntry
    {
        uint32_t ID;                                                // 0
        //uint32_t worldMapID;                                      // 1
        uint32_t areaID;                                            // 2 - index to AreaTable
        uint32_t areaID_2;                                          // 3 - index to AreaTable
        uint32_t areaID_3;                                          // 4 - index to AreaTable
        uint32_t areaID_4;                                          // 5 - index to AreaTable
        //uint32_t unk1[2];                                         // 6-7
        //uint32_t unk2;                                            // 8
        //uint32_t unk3[7];                                         // 9-16
    };
    #pragma pack(pop)
}
