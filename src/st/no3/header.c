// SPDX-License-Identifier: AGPL-3.0-or-later
#include "no3.h"
#include "../pfn_entity_update.h"

extern RoomHeader OVL_EXPORT(rooms)[];
extern RoomDef OVL_EXPORT(rooms_layers)[];
#if defined(VERSION_PC)
// PC uses the generated uncompressed banks (see the #include block below).
extern s16** OVL_EXPORT(spriteBanks)[];
extern u_long* OVL_EXPORT(cluts)[];
extern u_long** OVL_EXPORT(gfxBanks)[];
#else
extern SpriteParts* OVL_EXPORT(spriteBanks)[];
extern u_long* OVL_EXPORT(cluts)[];
extern GfxBank* OVL_EXPORT(gfxBanks)[];
#endif

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
