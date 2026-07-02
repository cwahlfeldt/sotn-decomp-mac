// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef RTOP_H
#define RTOP_H

#define STAGE_IS_RTOP

#include <stage.h>

#define OVL_EXPORT(x) RTOP_##x

#if defined(VERSION_PC)
#include "pc_data.h"
#endif

#if defined(VERSION_PC)
// Prefix the symbols rtop shares with other stages so they don't collide
// in the single PC binary.
#define EntityBreakableWall RTOP_EntityBreakableWall
#define EntityLionLamp RTOP_EntityLionLamp
#define EntitySecretStairs RTOP_EntitySecretStairs
#define EntitySkeletonPieces RTOP_EntitySkeletonPieces
#define EntityStairSwitch RTOP_EntityStairSwitch
#define EntityTriangleElevator RTOP_EntityTriangleElevator
#define EntityTriggerBeforeCastleWarp RTOP_EntityTriggerBeforeCastleWarp
#define PlayerIsWithinHitbox RTOP_PlayerIsWithinHitbox
#define g_EInitSkeletonPieces RTOP_g_EInitSkeletonPieces
#define g_EInitSkullLord RTOP_g_EInitSkullLord
#define g_EInitSkullLordEffects RTOP_g_EInitSkullLordEffects
#define g_EInitTombstone RTOP_g_EInitTombstone
#define cloudVectorOne RTOP_cloudVectorOne
#define cloudVectorTwo RTOP_cloudVectorTwo
#define cloudVectorThree RTOP_cloudVectorThree
#define cloudVectorFour RTOP_cloudVectorFour
#define data RTOP_data
#define empty RTOP_empty
#define s_RoofTextureData RTOP_s_RoofTextureData
#define s_TowerTextureData RTOP_s_TowerTextureData
#define s_ClockRoofScript RTOP_s_ClockRoofScript
#define s_ClockTowerScript RTOP_s_ClockTowerScript
#define s_ClockVertexSets RTOP_s_ClockVertexSets
#define LionLampAnim RTOP_LionLampAnim
#define D_us_80181BFC RTOP_D_us_80181BFC
#endif
#define INVERTED_STAGE

enum OVL_EXPORT(Palette) {
    PAL_NONE = 0,
    PAL_BREAKABLE = 0x200,
    PAL_BREAKABLE_DEBRIS = 0x204,
    PAL_UNK_220 = 0x220,
};

typedef enum EntityIDs {
    /* 0x00 */ E_NONE,
    /* 0x01 */ E_BREAKABLE,
    /* 0x02 */ E_EXPLOSION,
    /* 0x03 */ E_PRIZE_DROP,
    /* 0x04 */ E_NUMERIC_DAMAGE,
    /* 0x05 */ E_RED_DOOR,
    /* 0x06 */ E_INTENSE_EXPLOSION,
    /* 0x07 */ E_SOUL_STEAL_ORB,
    /* 0x08 */ E_ROOM_FOREGROUND,
    /* 0x09 */ E_STAGE_NAME_POPUP,
    /* 0x0A */ E_EQUIP_ITEM_DROP,
    /* 0x0B */ E_RELIC_ORB,
    /* 0x0C */ E_HEART_DROP,
    /* 0x0D */ E_ENEMY_BLOOD,
    /* 0x0E */ E_MESSAGE_BOX,
    /* 0x0F */ E_DUMMY_0F,
    /* 0x10 */ E_DUMMY_10,
    /* 0x11 */ E_ID_11 = 0x11,
    /* 0x13 */ E_UNK_ID_13 = 0x13,
    /* 0x14 */ E_EXPLOSION_VARIANTS,
    /* 0x15 */ E_GREY_PUFF,
    /* 0x18 */ E_STAIR_SEGMENT = 0x18,
    /* 0x1C */ E_BREAKABLE_DEBRIS = 0x1C,
    /* 0x22 */ E_YORICK = 0x22,
    /* 0x23 */ E_YORICK_SKULL,
    /* 0x24 */ E_SKELETON_PARTS,
    /* 0x25 */ E_TOMBSTONE,
    /* 0x26 */ E_SKULL_LORD,
    /* 0x27 */ E_SKULL_LORD_OUTLINE,
    /* 0x28 */ E_SKULL_LORD_FLAMES,
    /* 0x29 */ E_SKULL_LORD_PIECES,
} EntityIDs;

extern EInit OVL_EXPORT(EInitBreakable);
extern EInit g_EInitObtainable;
extern EInit g_EInitParticle;
extern EInit g_EInitSpawner;
extern EInit g_EInitInteractable;
extern EInit g_EInitUnkId13;
extern EInit g_EInitLockCamera;
extern EInit g_EInitCommon;
extern EInit g_EInitDamageNum;
#ifndef VERSION_PC
extern EInit __unused__;
#endif
extern EInit g_EInitRTOPCommon;
extern EInit g_EInitTombstone;
extern EInit g_EInitSkeletonPieces;
extern EInit D_us_801805E4;
extern EInit g_EInitSkullLord;
extern EInit g_EInitSkullLordEffects;

#endif // RTOP_H
