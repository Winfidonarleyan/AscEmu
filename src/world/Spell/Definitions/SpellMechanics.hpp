/*
Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

enum SpellMechanic : uint8_t
{
    MECHANIC_NONE          = 0,
    MECHANIC_CHARMED       = 1,
    MECHANIC_DISORIENTED   = 2,
    MECHANIC_DISARMED      = 3,
    MECHANIC_DISTRACED     = 4,
    MECHANIC_FLEEING       = 5,
    MECHANIC_CLUMSY        = 6,
    MECHANIC_ROOTED        = 7,
    MECHANIC_PACIFIED      = 8,
    MECHANIC_SILENCED      = 9,
    MECHANIC_ASLEEP        = 10,
    MECHANIC_ENSNARED      = 11,
    MECHANIC_STUNNED       = 12,
    MECHANIC_FROZEN        = 13,
    MECHANIC_INCAPACIPATED = 14,
    MECHANIC_BLEEDING      = 15,
    MECHANIC_HEALING       = 16,
    MECHANIC_POLYMORPHED   = 17,
    MECHANIC_BANISHED      = 18,
    MECHANIC_SHIELDED      = 19,
    MECHANIC_SHACKLED      = 20,
    MECHANIC_MOUNTED       = 21,
    MECHANIC_SEDUCED       = 22,
    MECHANIC_TURNED        = 23,
    MECHANIC_HORRIFIED     = 24,
    MECHANIC_INVULNARABLE  = 25,
    MECHANIC_INTERRUPTED   = 26,
    MECHANIC_DAZED         = 27,
    MECHANIC_DISCOVERY     = 28,
    MECHANIC_INVULNERABLE  = 29,
    MECHANIC_SAPPED        = 30,
    MECHANIC_ENRAGED       = 31,
    TOTAL_SPELL_MECHANICS
};

#define MOVEMENT_IMPAIRMENTS_AND_LOSS_OF_CONTROL_MASK (\
(1 << (MECHANIC_CHARMED - 1)) | (1 << (MECHANIC_DISORIENTED - 1)) | (1 << (MECHANIC_FLEEING - 1)) | \
(1 << (MECHANIC_ROOTED - 1)) | (1 << (MECHANIC_ASLEEP - 1)) | (1 << (MECHANIC_ENSNARED - 1)) | \
(1 << (MECHANIC_STUNNED - 1)) | (1 << (MECHANIC_FROZEN - 1)) | (1 << (MECHANIC_INCAPACIPATED - 1)) | \
(1 << (MECHANIC_POLYMORPHED - 1)) | (1 << (MECHANIC_BANISHED - 1)) | (1 << (MECHANIC_SHACKLED - 1)) | \
(1 << (MECHANIC_TURNED - 1)) | (1 << (MECHANIC_HORRIFIED - 1)) | (1 << (MECHANIC_DAZED - 1)) | \
(1 << (MECHANIC_SAPPED - 1)))
