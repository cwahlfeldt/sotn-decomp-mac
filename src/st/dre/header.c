// SPDX-License-Identifier: AGPL-3.0-or-later
#include "dre.h"

// common
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
void UpdateStageEntities(void);

Overlay OVL_EXPORT(Overlay) = {
    .Update = Update,
    .HitDetection = HitDetection,
    .UpdateRoomPosition = UpdateRoomPosition,
    .InitRoomEntities = InitRoomEntities,
    .rooms = OVL_EXPORT(rooms),
    .spriteBanks = OVL_EXPORT(spriteBanks),
    .cluts = OVL_EXPORT(cluts),
    .objLayoutHorizontal = NULL,
    .tileLayers = OVL_EXPORT(rooms_layers),
    .gfxBanks = OVL_EXPORT(gfxBanks),
    .UpdateStageEntities = UpdateStageEntities,
    .unk2C = NULL,
    .unk30 = NULL,
    .unk34 = NULL,
    .unk38 = NULL,
    .StageEndCutScene = NULL,
};

#if defined(VERSION_PC)
#include "gen/us/sprite_banks.h"
#include "gen/palette_def.h"
#include "gen/layers.h"
#include "gen/us/gfx_banks.h"
#endif
