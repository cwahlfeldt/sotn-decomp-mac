// SPDX-License-Identifier: AGPL-3.0-or-later
#include "rare.h"
#include "../pfn_entity_update.h"

// common
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
void UpdateStageEntities(void);

AbbreviatedOverlay OVL_EXPORT(Overlay) = {
    .Update = Update,
    .HitDetection = HitDetection,
    .UpdateRoomPosition = UpdateRoomPosition,
    .InitRoomEntities = InitRoomEntities,
    .rooms = OVL_EXPORT(rooms),
    .spriteBanks = OVL_EXPORT(spriteBanks),
    .cluts = OVL_EXPORT(cluts),
    .objLayoutHorizontal = &OBJ_LAYOUT_HORIZONTAL,
    .tileLayers = OVL_EXPORT(rooms_layers),
    .gfxBanks = OVL_EXPORT(gfxBanks),
    .UpdateStageEntities = UpdateStageEntities,
};

#if defined(VERSION_PC)
#include "gen/sprite_banks.h"
#include "gen/palette_def.h"
#include "gen/layers.h"
#include "gen/graphics_banks.h"
#endif
