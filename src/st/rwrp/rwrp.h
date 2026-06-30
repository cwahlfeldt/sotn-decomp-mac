// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef RWRP_H
#define RWRP_H

#include "common.h"
#include "stage.h"

#define OVL_EXPORT(x) RWRP_##x

#if defined(VERSION_PC)
// rwrp shares warp-room entities/data with wrp under the same symbol names.
// The PC build links every stage into one binary, so prefix rwrp's copies.
#define D_80181D08 RWRP_D_80181D08
#define EntityWarpSmallRocks RWRP_EntityWarpSmallRocks
#define g_EInitSmallRocks RWRP_g_EInitSmallRocks
#define g_EInitReverseSmallRocks RWRP_g_EInitReverseSmallRocks
#endif

typedef enum EntityIDs {
    E_NONE,
    E_BREAKABLE,
    E_EXPLOSION,
    E_PRIZE_DROP,
    E_NUMERIC_DAMAGE,
    E_RED_DOOR,
    E_INTENSE_EXPLOSION,
    E_SOUL_STEAL_ORB,
    E_ROOM_FOREGROUND,
    E_STAGE_NAME_POPUP,
    E_EQUIP_ITEM_DROP,
    E_RELIC_ORB,
    E_HEART_DROP,
    E_ENEMY_BLOOD,
    E_MESSAGE_BOX,
    E_DUMMY_0F,
    E_DUMMY_10,
    E_UNK_11,
    E_UNK_12,
    E_UNK_13,
    E_EXPLOSION_VARIANTS,
    E_GREY_PUFF,
    E_WARP,
    E_SMALL_ROCKS,
} EntityIDs;

extern EInit g_EInitObtainable;
extern EInit g_EInitParticle;
extern EInit g_EInitInteractable;
extern EInit g_EInitCommon;
extern EInit g_EInitLockCamera;
extern EInit g_EInitSmallRocks;

void EntityExplosionVariants(Entity* entity);
void EntityGreyPuff(Entity* entity);

#endif
