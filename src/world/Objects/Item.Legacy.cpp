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


#include "Storage/MySQLDataStore.hpp"
#include "Storage/MySQLStructures.h"
#include "Objects/Item.h"
#include "Objects/Container.h"
#include "Management/ItemInterface.h"
#include "Server/MainServerDefines.h"
#include "Map/Management/MapMgr.hpp"
#include "Spell/SpellMgr.hpp"
#include "Spell/Definitions/ProcFlags.hpp"
#include "Data/WoWItem.hpp"
#include "Server/Packets/SmsgEnchantmentLog.h"
#include "Server/Packets/SmsgItemEnchantmentTimeUpdate.h"
#include "Server/Packets/SmsgItemTimeUpdate.h"
#include "Util/Strings.hpp"

using namespace AscEmu::Packets;

#if VERSION_STRING < Cata
#include "Management/Guild/Guild.hpp"
#endif


void Item::LoadFromDB(Field* fields, Player* plr, bool light)
{
    uint32 itemid = fields[2].GetUInt32();

    m_itemProperties = sMySQLStore.getItemProperties(itemid);
    if (!m_itemProperties)
    {
        sLogger.failure("Item::LoadFromDB: Can't load item %u missing properties!", itemid);
        return;
    }

    if (m_itemProperties->LockId > 1)
        locked = true;
    else
        locked = false;

    setEntry(itemid);
    m_owner = plr;

    wrapped_item_id = fields[3].GetUInt32();
    setGiftCreatorGuid(fields[4].GetUInt32());
    setCreatorGuid(fields[5].GetUInt32());

    uint32 count = fields[6].GetUInt32();
    if (count > m_itemProperties->MaxCount && (m_owner && !m_owner->m_cheats.hasItemStackCheat))
        count = m_itemProperties->MaxCount;
    setStackCount(count);

    SetChargesLeft(fields[7].GetUInt32());

    setFlags(fields[8].GetUInt32());
    uint32 randomProp = fields[9].GetUInt32();
    uint32 randomSuffix = fields[10].GetUInt32();

    setRandomPropertiesId(randomProp);

    int32 rprop = int32(randomProp);
    // If random properties point is negative that means the item uses random suffix as random enchantment
    if (rprop < 0)
        setPropertySeed(randomSuffix);
    else
        setPropertySeed(0);

#ifdef AE_TBC
    setTextId(fields[11].GetUInt32());
#endif

    setMaxDurability(m_itemProperties->MaxDurability);
    setDurability(fields[12].GetUInt32());

    if (light)
        return;

    std::string enchant_field = fields[15].GetString();
    if (!enchant_field.empty())
    {
        std::vector<std::string> enchants = AscEmu::Util::Strings::split(enchant_field, ";");
        uint32 enchant_id;

        uint32 time_left;
        uint32 enchslot;

        for (auto& enchant : enchants)
        {
            if (sscanf(enchant.c_str(), "%u,%u,%u", &enchant_id, &time_left, &enchslot) == 3)
            {
                if (enchant_id)
                    addEnchantment(enchant_id, static_cast<EnchantmentSlot>(enchslot), time_left);
            }
        }
    }

    ItemExpiresOn = fields[16].GetUInt32();

    ///////////////////////////////////////////////////// Refund stuff ////////////////////////
    std::pair<time_t, uint32> refundentry;
    refundentry.first = fields[17].GetUInt32();
    refundentry.second = fields[18].GetUInt32();

    if (refundentry.first != 0 && refundentry.second != 0 && getOwner() != nullptr)
    {
        uint32* played = getOwner()->getPlayedTime();
        if (played[1] < (refundentry.first + 60 * 60 * 2))
            m_owner->getItemInterface()->AddRefundable(this, refundentry.second, refundentry.first);
    }

    ///////////////////////////////////////////////////////////////////////////////////////////

    text = fields[19].GetString();

    ApplyRandomProperties(false);

    // Charter stuff
    if (getEntry() == CharterEntry::Guild)
    {
        addFlags(ITEM_FLAG_SOULBOUND);
        setStackCount(1);
        setPropertySeed(57813883);
        if (plr != nullptr && plr->getCharter(CHARTER_TYPE_GUILD))
            setEnchantmentId(0, plr->getCharter(CHARTER_TYPE_GUILD)->GetID());
    }

    if (getEntry() == CharterEntry::TwoOnTwo)
    {
        addFlags(ITEM_FLAG_SOULBOUND);
        setStackCount(1);
        setPropertySeed(57813883);
        if (plr != nullptr && plr->getCharter(CHARTER_TYPE_ARENA_2V2))
            setEnchantmentId(0, plr->getCharter(CHARTER_TYPE_ARENA_2V2)->GetID());
    }

    if (getEntry() == CharterEntry::ThreeOnThree)
    {
        addFlags(ITEM_FLAG_SOULBOUND);
        setStackCount(1);
        setPropertySeed(57813883);
        if (plr != nullptr && plr->getCharter(CHARTER_TYPE_ARENA_3V3))
            setEnchantmentId(0, plr->getCharter(CHARTER_TYPE_ARENA_3V3)->GetID());
    }

    if (getEntry() == CharterEntry::FiveOnFive)
    {
        addFlags(ITEM_FLAG_SOULBOUND);
        setStackCount(1);
        setPropertySeed(57813883);
        if (plr != nullptr && plr->getCharter(CHARTER_TYPE_ARENA_5V5))
            setEnchantmentId(0, plr->getCharter(CHARTER_TYPE_ARENA_5V5)->GetID());
    }
}

void Item::ApplyRandomProperties(bool apply)
{
    // apply random properties
    if (getRandomPropertiesId() != 0)
    {
        if (int32(getRandomPropertiesId()) > 0)
        {
            auto item_random_properties = sItemRandomPropertiesStore.LookupEntry(getRandomPropertiesId());
            for (uint8 k = 0; k < 3; k++)
            {
                if (item_random_properties == nullptr)
                    continue;

                if (item_random_properties->spells[k] != 0)
                {
                    auto spell_item_enchant = sSpellItemEnchantmentStore.LookupEntry(item_random_properties->spells[k]);
                    if (spell_item_enchant == nullptr)
                        continue;

                    auto Slot = hasEnchantmentReturnSlot(item_random_properties->spells[k]);
                    if (Slot < 0)
                    {
                        EnchantmentSlot slot = PROP_ENCHANTMENT_SLOT_2;
                        if (_findFreeRandomEnchantmentSlot(&slot, RandomEnchantmentType::PROPERTY))
                            addEnchantment(item_random_properties->spells[k], slot, 0, true);
                    }
                    else if (apply)
                    {
                        ApplyEnchantmentBonus(static_cast<EnchantmentSlot>(Slot), true);
                    }
                }
            }
        }
        else
        {
            auto item_random_suffix = sItemRandomSuffixStore.LookupEntry(abs(int(getRandomPropertiesId())));

            for (uint8 k = 0; k < 3; ++k)
            {
                if (item_random_suffix == nullptr)
                    continue;

                if (item_random_suffix->enchantments[k] != 0)
                {
                    auto spell_item_enchant = sSpellItemEnchantmentStore.LookupEntry(item_random_suffix->enchantments[k]);
                    if (spell_item_enchant == nullptr)
                        continue;

                    auto Slot = hasEnchantmentReturnSlot(spell_item_enchant->Id);
                    if (Slot < 0)
                    {
                        EnchantmentSlot slot = PROP_ENCHANTMENT_SLOT_0;
                        if (_findFreeRandomEnchantmentSlot(&slot, RandomEnchantmentType::SUFFIX))
                            addEnchantment(item_random_suffix->enchantments[k], slot, 0, true, item_random_suffix->prefixes[k]);
                    }
                    else if (apply)
                    {
                        ApplyEnchantmentBonus(static_cast<EnchantmentSlot>(Slot), true);
                    }
                }
            }
        }
    }
}

void Item::SaveToDB(int8 containerslot, int8 slot, bool firstsave, QueryBuffer* buf)
{
    if (!m_isDirty && !firstsave)
        return;

    uint64 GiftCreatorGUID = getGiftCreatorGuid();
    uint64 CreatorGUID = getCreatorGuid();

    std::stringstream ss;
    ss << "DELETE FROM playeritems WHERE guid = " << getGuidLow() << ";";

    if (firstsave)
    {
        CharacterDatabase.WaitExecute(ss.str().c_str());
    }
    else
    {
        if (buf == nullptr)
            CharacterDatabase.Execute(ss.str().c_str());
        else
            buf->AddQueryNA(ss.str().c_str());
    }


    ss.rdbuf()->str("");

    ss << "INSERT INTO playeritems VALUES(";

    ss << getOwnerGuidLow() << ",";
    ss << getGuidLow() << ",";
    ss << getEntry() << ",";
    ss << wrapped_item_id << ",";
    ss << (WoWGuid::getGuidLowPartFromUInt64(GiftCreatorGUID)) << ",";
    ss << (WoWGuid::getGuidLowPartFromUInt64(CreatorGUID)) << ",";

    ss << getStackCount() << ",";
    ss << int32(GetChargesLeft()) << ",";
    ss << getFlags() << ",";
    ss << random_prop << ", " << random_suffix << ", ";
    ss << 0 << ",";
    ss << getDurability() << ",";
    ss << static_cast<int>(containerslot) << ",";
    ss << static_cast<int>(slot) << ",'";

    // Pack together enchantment fields
    if (!Enchantments.empty())
    {
        for (EnchantmentMap::iterator itr = Enchantments.begin(); itr != Enchantments.end(); ++itr)
        {
            if (itr->second.RemoveAtLogout)
                continue;

            if (getEnchantmentId(itr->second.Slot) != itr->second.Enchantment->Id)
                continue;

            const auto timeLeft = getEnchantmentDuration(itr->second.Slot);
            if (itr->second.Enchantment && (timeLeft > 5000 || timeLeft == 0))
            {
                ss << itr->second.Enchantment->Id << ",";
                ss << timeLeft << ",";
                ss << static_cast<int>(itr->second.Slot) << ";";
            }
        }
    }
    ss << "','";
    ss << ItemExpiresOn << "','";

    ////////////////////////////////////////////////// Refund stuff /////////////////////////////////

    // Check if the owner is instantiated. When sending mail he/she obviously will not be :P
    if (this->getOwner() != nullptr)
    {
        std::pair<time_t, uint32> refundentry = this->getOwner()->getItemInterface()->LookupRefundable(this->getGuid());

        ss << uint32(refundentry.first) << "','";
        ss << uint32(refundentry.second);
    }
    else
    {
        ss << uint32(0) << "','";
        ss << uint32(0);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////
    ss << "','";
    ss << text;
    ss << "')";

    if (firstsave)
    {
        CharacterDatabase.WaitExecute(ss.str().c_str());
    }
    else
    {
        if (buf == nullptr)
            CharacterDatabase.Execute(ss.str().c_str());
        else
            buf->AddQueryNA(ss.str().c_str());
    }

    m_isDirty = false;
}

void Item::DeleteFromDB()
{
    if (m_itemProperties->ContainerSlots > 0 && isContainer())
    {
        /* deleting a container */
        for (uint32 i = 0; i < m_itemProperties->ContainerSlots; ++i)
        {
            if (dynamic_cast<Container*>(this)->GetItem(static_cast<int16>(i)) != nullptr)
            {
                /* abort the delete */
                return;
            }
        }
    }

    CharacterDatabase.Execute("DELETE FROM playeritems WHERE guid = %u", getGuidLow());
}

void Item::DeleteMe()
{
    if (this->m_owner != nullptr)
        this->m_owner->getItemInterface()->RemoveRefundable(this->getGuid());

    delete this;
}

uint16_t GetSkillByProto(uint32 Class, uint32 SubClass)
{
    if (Class == 4 && SubClass < 7)
    {
        return arm_skills[SubClass];
    }

    if (Class == 2)
    {
        if (SubClass < 20)  //no skill for fishing
            return weap_skills[SubClass];
    }

    return 0;
}

uint32 GetSellPriceForItem(ItemProperties const* proto, uint32 count)
{
    int32 cost = proto->SellPrice * ((count < 1) ? 1 : count);
    return cost;
}

uint32 GetBuyPriceForItem(ItemProperties const* proto, uint32 count, Player* plr, Creature* vendor)
{
    int32 cost = proto->BuyPrice;

    if (plr && vendor)
    {
        Standing plrstanding = plr->getFactionStandingRank(vendor->m_factionTemplate->Faction);
        cost = float2int32(ceilf(proto->BuyPrice * pricemod[plrstanding]));
    }

    return cost * count;
}

void Item::RemoveFromWorld()
{
    // if we have an owner->send destroy
    if (m_owner != nullptr)
        m_owner->sendDestroyObjectPacket(getGuid());

    if (!IsInWorld())
        return;

    m_WorldMap->RemoveObject(this, false);
    m_WorldMap = nullptr;

    // update our event holder
    event_Relocate();
}

void Item::ApplyEnchantmentBonus(EnchantmentSlot Slot, bool Apply)
{
    if (m_owner == nullptr)
        return;

    EnchantmentMap::iterator itr = Enchantments.find(Slot);
    if (itr == Enchantments.end())
        return;

    DBC::Structures::SpellItemEnchantmentEntry const* Entry = itr->second.Enchantment;
    uint32 RandomSuffixAmount = itr->second.RandomSuffix;

    if (itr->second.BonusApplied == Apply)
        return;

    itr->second.BonusApplied = Apply;

    if (Apply)
    {
        // Send the enchantment time update packet.
        const auto duration = getEnchantmentDuration(itr->second.Slot) / 1000;
        if (duration)
            SendEnchantTimeUpdate(itr->second.Slot, duration);
    }

    // Apply the visual on the player.
    uint32 ItemSlot = m_owner->getItemInterface()->GetInventorySlotByGuid(getGuid());
    if (ItemSlot < EQUIPMENT_SLOT_END)
    {
#if VERSION_STRING >= WotLK
        if (Slot == PERM_ENCHANTMENT_SLOT || Slot == TEMP_ENCHANTMENT_SLOT)
#else
        if (Slot < MAX_INSPECTED_ENCHANTMENT_SLOT)
#endif
            m_owner->setVisibleItemEnchantment(ItemSlot, Slot, Apply ? Entry->Id : 0);
    }
    else if (Apply)
    {
        sLogger.failure("Item::ApplyEnchantmentBonus : Tried to apply visual enchantment but equipment slot %i is invalid", ItemSlot);
    }

#if VERSION_STRING >= Cata
    // Apply stats from reforging
    if (Slot == REFORGE_ENCHANTMENT_SLOT)
        m_owner->applyReforgeEnchantment(this, Apply);
#endif

    // Another one of those for loop that where not indented properly god knows what will break
    // but i made it actually affect the code below it
    for (uint32 c = 0; c < 3; c++)
    {
        if (Entry->type[c])
        {
            // Depending on the enchantment type, take the appropriate course of action.
            switch (Entry->type[c])
            {
                case ITEM_ENCHANTMENT_TYPE_COMBAT_SPELL:    // Trigger spell on melee attack.
                {
                    if (Apply)
                    {
                        if (Entry->spell[c] != 0)
                        {
                            // Calculate proc chance equivalent of 1 PPM if proc chance does not exist in DBC (chance or PPM can be modified later in spellscript)
                            const auto procChance = Entry->min[c] == 0 ? float2int32((getItemProperties()->Delay * 0.001f / 60.0f) * 100.0f) : Entry->min[c];
                            switch (m_owner->getItemInterface()->GetInventorySlotByGuid(getGuid()))
                            {
                                // 'Chance on hit' enchantments in main hand should only proc from main hand hits
                                case EQUIPMENT_SLOT_MAINHAND:
                                    m_owner->addProcTriggerSpell(Entry->spell[c], 0, m_owner->getGuid(), procChance, SpellProcFlags(PROC_ON_DONE_MELEE_HIT | PROC_ON_DONE_MELEE_SPELL_HIT), EXTRA_PROC_ON_MAIN_HAND_HIT_ONLY, nullptr, nullptr, nullptr, this);
                                    break;
                                // 'Chance on hit' enchantments in off hand should only proc from off hand hits
                                case EQUIPMENT_SLOT_OFFHAND:
                                    m_owner->addProcTriggerSpell(Entry->spell[c], 0, m_owner->getGuid(), procChance, SpellProcFlags(PROC_ON_DONE_MELEE_HIT | PROC_ON_DONE_MELEE_SPELL_HIT | PROC_ON_DONE_OFFHAND_ATTACK), EXTRA_PROC_ON_OFF_HAND_HIT_ONLY, nullptr, nullptr, nullptr, this);
                                    break;
                                // 'Chance on hit' enchantments in ranged slot should only proc from ranged attacks
                                ///\ todo: does this enchantment type even have ranged spells?
                                case EQUIPMENT_SLOT_RANGED:
                                    m_owner->addProcTriggerSpell(Entry->spell[c], 0, m_owner->getGuid(), procChance, SpellProcFlags(PROC_ON_DONE_RANGED_HIT | PROC_ON_DONE_RANGED_SPELL_HIT), EXTRA_PROC_NULL, nullptr, nullptr, nullptr, this);
                                    break;
                                // In any other slot, proc on any melee hit
                                default:
                                    m_owner->addProcTriggerSpell(Entry->spell[c], 0, m_owner->getGuid(), procChance, SpellProcFlags(PROC_ON_DONE_MELEE_HIT | PROC_ON_DONE_MELEE_SPELL_HIT), EXTRA_PROC_NULL, nullptr, nullptr, nullptr, this);
                                    break;
                            }
                        }
                    }
                    else
                    {
                        m_owner->removeProcTriggerSpell(Entry->spell[c], m_owner->getGuid(), getGuid());
                    }
                }
                break;

                case ITEM_ENCHANTMENT_TYPE_DAMAGE:  // Mod damage done.
                {
                    int32 val = Entry->min[c];
                    if (RandomSuffixAmount)
                        val = RANDOM_SUFFIX_MAGIC_CALCULATION(RandomSuffixAmount, getPropertySeed());

                    if (Apply)
                        m_owner->modModDamageDonePositive(SCHOOL_NORMAL, val);
                    else
                        m_owner->modModDamageDonePositive(SCHOOL_NORMAL, -val);

                    m_owner->CalcDamage();
                }
                break;

                case ITEM_ENCHANTMENT_TYPE_EQUIP_SPELL: // Cast spell (usually means apply aura)
                {
                    if (Apply)
                    {
                        SpellCastTargets targets(m_owner->getGuid());

                        if (Entry->spell[c] != 0)
                        {
                            SpellInfo const* sp = sSpellMgr.getSpellInfo(Entry->spell[c]);
                            if (sp == nullptr)
                                continue;

                            Spell* spell = sSpellMgr.newSpell(m_owner, sp, true, 0);
                            spell->setItemCaster(this);
                            spell->prepare(&targets);
                        }
                    }
                    else
                    {
                        if (Entry->spell[c] != 0)
                            m_owner->removeAuraByItemGuid(Entry->spell[c], getGuid());
                    }
                }
                break;

                case ITEM_ENCHANTMENT_TYPE_RESISTANCE:  // Modify physical resistance
                {
                    int32 val = Entry->min[c];
                    if (RandomSuffixAmount)
                        val = RANDOM_SUFFIX_MAGIC_CALCULATION(RandomSuffixAmount, getPropertySeed());

                    if (Apply)
                        m_owner->FlatResistanceModifierPos[Entry->spell[c]] += val;
                    else
                        m_owner->FlatResistanceModifierPos[Entry->spell[c]] -= val;

                    m_owner->CalcResistance(static_cast<uint8_t>(Entry->spell[c]));
                }
                break;

                case ITEM_ENCHANTMENT_TYPE_STAT:    //Modify rating ...order is PLAYER_FIELD_COMBAT_RATING_1 and above
                {
                    //spellid is enum ITEM_STAT_TYPE
                    //min=max is amount
                    int32 val = Entry->min[c];
                    if (RandomSuffixAmount)
                        val = RANDOM_SUFFIX_MAGIC_CALCULATION(RandomSuffixAmount, getPropertySeed());

                    m_owner->ModifyBonuses(Entry->spell[c], val, Apply);
                    m_owner->UpdateStats();
                }
                break;

                case ITEM_ENCHANTMENT_TYPE_TOTEM:   // Rockbiter weapon (increase damage per second... how the hell do you calc that)
                {
                    if (Apply)
                    {
                        //m_owner->modModDamageDonePositive(SCHOOL_NORMAL, Entry->min[c]);
                        //if I'm not wrong then we should apply DMPS formula for this. This will have somewhat a larger value 28->34
                        int32 val = Entry->min[c];
                        if (RandomSuffixAmount)
                            val = RANDOM_SUFFIX_MAGIC_CALCULATION(RandomSuffixAmount, getPropertySeed());

                        int32 value = getItemProperties()->Delay * val / 1000;
                        m_owner->modModDamageDonePositive(SCHOOL_NORMAL, value);
                    }
                    else
                    {
                        int32 val = Entry->min[c];
                        if (RandomSuffixAmount)
                            val = RANDOM_SUFFIX_MAGIC_CALCULATION(RandomSuffixAmount, getPropertySeed());

                        int32 value = -(int32)(getItemProperties()->Delay * val / 1000);
                        m_owner->modModDamageDonePositive(SCHOOL_NORMAL, value);
                    }
                    m_owner->CalcDamage();
                }
                break;

                case ITEM_ENCHANTMENT_TYPE_USE_SPELL:
                {
                    if (Apply)
                    {
                        for (uint8 i = 0; i < 3; ++i)
                            OnUseSpellIDs[i] = Entry->spell[i];
                    }
                    else
                    {
                        for (uint8 i = 0; i < 3; ++i)
                            OnUseSpellIDs[i] = 0;
                    }
                    break;
                }

                case ITEM_ENCHANTMENT_TYPE_PRISMATIC_SOCKET:
                {
                    // Adding a prismatic socket to belt, hands, etc is type 8, it has no bonus to apply HERE
                    break;
                }

                default:
                    sLogger.failure("Unknown enchantment type: %u (%u)", Entry->type[c], Entry->Id);
                break;
            }
        }
    }
}

void Item::SendEnchantTimeUpdate(uint32 Slot, uint32 Duration)
{
    m_owner->sendPacket(SmsgItemEnchantmentTimeUpdate(getGuid(), Slot, Duration, m_owner->getGuid()).serialise().get());
}

void Item::RemoveRelatedEnchants(DBC::Structures::SpellItemEnchantmentEntry const* newEnchant)
{
    for (EnchantmentMap::iterator itr = Enchantments.begin(); itr != Enchantments.end();)
    {
        EnchantmentMap::iterator itr2 = itr++;
        if (itr2->second.Enchantment->Id == newEnchant->Id || (itr2->second.Enchantment->EnchantGroups > 1 && newEnchant->EnchantGroups > 1))
            removeEnchantment(itr2->first);
    }
}

bool Item::IsGemRelated(DBC::Structures::SpellItemEnchantmentEntry const* Enchantment)
{
#if VERSION_STRING > Classic
    if (getItemProperties()->SocketBonus == Enchantment->Id)
        return true;

    return Enchantment->GemEntry != 0;
#else
    return 0;
#endif
}

uint32 Item::GenerateRandomSuffixFactor(ItemProperties const* m_itemProto)
{
    double value;

    if (m_itemProto->Class == ITEM_CLASS_ARMOR && m_itemProto->Quality > ITEM_QUALITY_UNCOMMON_GREEN)
        value = SuffixMods[m_itemProto->InventoryType] * 1.24;
    else
        value = SuffixMods[m_itemProto->InventoryType];

    value = (value * double(m_itemProto->ItemLevel)) + 0.5;
    return long2int32(value);
}

std::string Item::GetItemLink(uint32 language = 0)
{
    return GetItemLinkByProto(getItemProperties(), language);
}

std::string GetItemLinkByProto(ItemProperties const* iProto, uint32 language = 0)
{
    char buffer[256];
    std::string colour;

    switch (iProto->Quality)
    {
        case ITEM_QUALITY_POOR: //Poor,gray
            colour = "cff9d9d9d";
            break;
        case ITEM_QUALITY_NORMAL: //Common,white
            colour = "cffffffff";
            break;
        case ITEM_QUALITY_UNCOMMON: //Uncommon,green
            colour = "cff1eff00";
            break;
        case ITEM_QUALITY_RARE: //Rare,blue
            colour = "cff0070dd";
            break;
        case ITEM_QUALITY_EPIC: //Epic,purple
            colour = "cffa335ee";
            break;
        case ITEM_QUALITY_LEGENDARY: //Legendary,orange
            colour = "cffff8000";
            break;
        case ITEM_QUALITY_ARTIFACT: //Artifact,pale gold
            colour = "c00fce080";
            break;
        case ITEM_QUALITY_HEIRLOOM: //Heirloom,pale gold
            colour = "c00fce080";
            break;
        default:
            colour = "cff9d9d9d";
    }

    // try to get localized version
    MySQLStructure::LocalesItem const* lit = (language > 0) ? sMySQLStore.getLocalizedItem(iProto->ItemId, language) : nullptr;
    if (lit)
        snprintf(buffer, 256, "|%s|Hitem:%u:0:0:0:0:0:0:0|h[%s]|h|r", colour.c_str(), iProto->ItemId, lit->name);
    else
        snprintf(buffer, 256, "|%s|Hitem:%u:0:0:0:0:0:0:0|h[%s]|h|r", colour.c_str(), iProto->ItemId, iProto->Name.c_str());

    const char* ItemLink = buffer;

    return ItemLink;
}

uint32 Item::CountGemsWithLimitId(uint32 LimitId)
{
#if VERSION_STRING > Classic
    uint32 result = 0;
    for (uint8_t count = 0; count < getSocketSlotCount(); count++)
    {
        const auto ei = getEnchantment(static_cast<EnchantmentSlot>(SOCK_ENCHANTMENT_SLOT1 + count));

        //huh ? Gem without entry ?
        if (ei && ei->Enchantment->GemEntry)
        {
            ItemProperties const* ip = sMySQLStore.getItemProperties(ei->Enchantment->GemEntry);
            if (ip && ip->ItemLimitCategory == LimitId)
                result++;
        }
    }
    return result;
#else
    return 0;
#endif
}

void Item::EventRemoveItem()
{
    if (this->getOwner())
        m_owner->getItemInterface()->SafeFullRemoveItemByGuid(this->getGuid());
}

void Item::SendDurationUpdate()
{
    m_owner->sendPacket(SmsgItemTimeUpdate(this->getGuid(), this->getDuration()).serialise().get());
}

// "Stackable items (such as Frozen Orbs and gems) and
// charged items that can be purchased with an alternate currency are not eligible. "
bool Item::IsEligibleForRefund()
{
    ItemProperties const* proto = this->getItemProperties();
    if (!(proto->Flags & ITEM_FLAG_REFUNDABLE))
        return false;

    if (proto->MaxCount > 1)
        return false;

    for (uint8 i = 0; i < 5; ++i)
    {
        ItemSpell spell = proto->Spells[i];
        if (spell.Charges != -1 && spell.Charges != 0)
            return false;
    }

    return true;
}

void Item::RemoveFromRefundableMap()
{
    Player* owner = this->getOwner();
    uint64 GUID = this->getGuid();

    if (owner && GUID != 0)
        owner->getItemInterface()->RemoveRefundable(GUID);
}

uint32 Item::RepairItemCost()
{
    auto durability_costs = sDurabilityCostsStore.LookupEntry(m_itemProperties->ItemLevel);
    if (durability_costs == nullptr)
    {
        sLogger.failure("Repair: Unknown item level (%u)", durability_costs);
        return 0;
    }

    auto durability_quality = sDurabilityQualityStore.LookupEntry((m_itemProperties->Quality + 1) * 2);
    if (durability_quality == nullptr)
    {
        sLogger.failure("Repair: Unknown item quality (%u)", durability_quality);
        return 0;
    }

    uint32 dmodifier = durability_costs->modifier[m_itemProperties->Class == ITEM_CLASS_WEAPON ? m_itemProperties->SubClass : m_itemProperties->SubClass + 21];
    uint32 cost = long2int32((getMaxDurability() - getDurability()) * dmodifier * double(durability_quality->quality_modifier));
    return cost;
}

bool Item::RepairItem(Player* pPlayer, bool guildmoney, int32* pCost)   //pCost is needed for the guild log
{
    //int32 cost = (int32)pItem->getMaxDurability()) - (int32)pItem->getDurability();
    uint32 cost = RepairItemCost();
    if (cost == 0)
        return false;

    if (guildmoney && pPlayer->isInGuild())
    {
        if (!pPlayer->getGuild()->handleMemberWithdrawMoney(pPlayer->getSession(), cost, true))
            return false;//we should tell the client that he can't repair with the guild gold.

        if (pCost != 0)
            *pCost += cost;
    }
    else //we pay with our gold
    {
        if (!pPlayer->hasEnoughCoinage(cost))
            return false;

        pPlayer->modCoinage(-(int32)cost);
    }
    setDurabilityToMax();
    m_isDirty = true;

    return true;
}
