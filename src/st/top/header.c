// SPDX-License-Identifier: AGPL-3.0-or-later
#include "top.h"
#include "../pfn_entity_update.h"

extern RoomHeader OVL_EXPORT(rooms)[];
#if defined(VERSION_PC)
extern s16** OVL_EXPORT(spriteBanks)[];
#else
extern SpriteParts* OVL_EXPORT(spriteBanks)[];
#endif
extern u_long* OVL_EXPORT(cluts)[];
extern RoomDef OVL_EXPORT(rooms_layers)[];
#if defined(VERSION_PC)
extern u_long** OVL_EXPORT(gfxBanks)[];
#else
extern GfxBank* OVL_EXPORT(gfxBanks)[];
#endif
extern u8* D_us_80182E44[];

AbbreviatedOverlay2 OVL_EXPORT(Overlay) = {
    .Update = Update,
    .HitDetection = HitDetection,
    .UpdateRoomPosition = UpdateRoomPosition,
    .InitRoomEntities = InitRoomEntities,
    .rooms = OVL_EXPORT(rooms),
    .spriteBanks = OVL_EXPORT(spriteBanks),
    .cluts = OVL_EXPORT(cluts),
#if defined(VERSION_PC)
    .objLayoutHorizontal = &OBJ_LAYOUT_HORIZONTAL,
#else
    .objLayoutHorizontal = OBJ_LAYOUT_HORIZONTAL,
#endif
    .tileLayers = OVL_EXPORT(rooms_layers),
    .gfxBanks = OVL_EXPORT(gfxBanks),
    .UpdateStageEntities = UpdateStageEntities,
    .unk2C = NULL,
    .unk30 = D_us_80182E44,
};

#if defined(VERSION_PC)
#include "gen/palette_def.h"
#include "gen/layers.h"
#include "gen/graphics_banks.h"
#endif
