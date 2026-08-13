// SPDX-License-Identifier: AGPL-3.0-or-later
/*
 * File: e_blue_venus_weed.c
 * Overlay: RNO3
 * Description: ENTITY - Blue Venus Weed (Alura Une)
 *
 * Reverse-castle variant of CHI's Venus Weed (src/st/chi/en_venus_weed.c):
 * same root/flower/tendril/dart/spike layout, but the flower picks its
 * attack by player distance instead of alternating, the eight (not ten)
 * tendrils chase the player and attack on arrival via a flower handshake
 * (attackTrigger/tendrilsReady), and the blue palette cluts differ.
 * Retains three Konami debug FntPrints ("arla" = Alura).
 */

#include "rno3.h"
#include "sfx.h"

#define TENDRIL_COUNT 8

extern EInit g_EInitBlueVenusWeed1; // root
extern EInit g_EInitBlueVenusWeed2; // flower
extern EInit D_us_80180A1C;         // tendril
extern EInit D_us_80180A28;         // dart

extern s16* sprites_rno3_6[];

// Animation frames and collision sensors; still in the raw data blob at
// RNO3.BIN+0x2548.
extern u8 D_us_801828B4[];  // physics sensors (drop to ground)
extern s16 D_us_801828C4[]; // walk sensors (tendril)
extern u8 D_us_801828CC[];  // anim: flower reveal
extern u8 D_us_801828E4[];  // anim: flower spikes charge
extern u8 D_us_801828F0[];  // anim: flower spikes launch
extern u8 D_us_801828F8[];  // anim: flower spikes reset
extern u8 D_us_80182914[];  // anim: flower pulse
extern u8 D_us_80182920[];  // anim: flower darts charge
extern u8 D_us_80182930[];  // anim: flower darts launch
extern u8 D_us_80182948[];  // anim: tendril arrival attack
extern u8 D_us_80182964[];  // anim: tendril attack charge
extern u8 D_us_801829A8[];  // anim: tendril attack launch
extern u8 D_us_801829C8[];  // anim: tendril bounce
extern u8 D_us_801829D4[];  // anim: thornweed disguise
extern u8 D_us_801829E0[];  // anim: thornweed quick wiggle
extern u8 D_us_801829EA[];  // tendril hitbox indices - 0x22
extern s8 D_us_801829FC[];  // tendril hitbox data

// func_us_801C4D18
static Primitive* SetupPrimsForEntitySpriteParts(
    Entity* entity, Primitive* prim) {
    s16 y;
    s32 spritePartCount;
    s16 x;
    u8 spriteU0;
    u8 spriteV0;
    s16* spriteData;
    s32 i;
    u8 spriteU1;
    s16 spriteDestX;
    s16 spriteDestY;
    s16 spriteDestW;
    s16 spriteDestH;
    s16 spriteFlags;
    u8 spriteV1;
    s32 xFlip;

    spriteData = sprites_rno3_6[entity->animCurFrame];
    spritePartCount = *spriteData;
    spriteData++;

    for (i = 0; i < spritePartCount; i++, spriteData += 11) {
        spriteFlags = spriteData[0];
        spriteDestX = spriteData[1];
        spriteDestY = spriteData[2];
        spriteDestW = spriteData[3];
        spriteDestH = spriteData[4];

        // Adjust sprite position to respect sprite flags
        if (spriteFlags & 4) {
            spriteDestW -= 1;
            if (spriteFlags & 2) {
                spriteDestX += 1;
            }
        }
        if (spriteFlags & 8) {
            spriteDestH -= 1;
            if (spriteFlags & 1) {
                spriteDestY += 1;
            }
        }
        if (spriteFlags & 0x10) {
            spriteDestW -= 1;
            if (!(spriteFlags & 2)) {
                spriteDestX += 1;
            }
        }
        if (spriteFlags & 0x20) {
            spriteDestH -= 1;
            if (!(spriteFlags & 1)) {
                spriteDestY += 1;
            }
        }

        // Calculate sprite position to respect facing
        x = entity->posX.i.hi;
        y = entity->posY.i.hi;
        if (entity->facingLeft) {
            x -= spriteDestX;
        } else {
            x += spriteDestX;
        }
        y += spriteDestY;

        // Set sprite position to respect the above, plus sprite dimensions
        if (entity->facingLeft) {
            LOH(prim->x0) = x - spriteDestW + 1;
            LOH(prim->y0) = y;
            LOH(prim->x1) = x + 1;
            LOH(prim->y1) = y;
            LOH(prim->x2) = x - spriteDestW + 1;
            LOH(prim->y2) = y + spriteDestH;
            LOH(prim->x3) = x + 1;
            LOH(prim->y3) = y + spriteDestH;
        } else {
            LOH(prim->x0) = x;
            LOH(prim->y0) = y;
            LOH(prim->x1) = x + spriteDestW;
            LOH(prim->y1) = y;
            LOH(prim->x2) = x;
            LOH(prim->y2) = y + spriteDestH;
            LOH(prim->x3) = x + spriteDestW;
            LOH(prim->y3) = y + spriteDestH;
        }

        // Entity-relative clut
        prim->clut = entity->palette + spriteData[5];

        spriteU0 = spriteData[7];
        spriteV0 = spriteData[8];
        spriteU1 = spriteData[9];
        spriteV1 = spriteData[10];

        // Adjust sprite UVs to respect sprite flags
        if (spriteFlags & 4) {
            spriteU1--;
        }
        if (spriteFlags & 8) {
            spriteV1--;
        }
        if (spriteFlags & 0x10) {
            spriteU0++;
        }
        if (spriteFlags & 0x20) {
            spriteV0++;
        }

        // Set sprite UVs to respect the above, plus facing
        xFlip = (spriteFlags & 2) ^ entity->facingLeft;
        if (!xFlip) {
            if (!(spriteFlags & 1)) {
                prim->u0 = spriteU0;
                prim->v0 = spriteV0;
                prim->u1 = spriteU1;
                prim->v1 = spriteV0;
                prim->u2 = spriteU0;
                prim->v2 = spriteV1;
                prim->u3 = spriteU1;
                prim->v3 = spriteV1;
            } else {
                prim->u0 = spriteU0;
                prim->v0 = spriteV1 - 1;
                prim->u1 = spriteU1;
                prim->v1 = spriteV1 - 1;
                prim->u2 = spriteU0;
                prim->v2 = spriteV0 - 1;
                prim->u3 = spriteU1;
                prim->v3 = spriteV0 - 1;
            }
        } else {
            if (!(spriteFlags & 1)) {
                prim->u0 = spriteU1 - 1;
                prim->v0 = spriteV0;
                prim->u1 = spriteU0 - 1;
                prim->v1 = spriteV0;
                prim->u2 = spriteU1 - 1;
                prim->v2 = spriteV1;
                prim->u3 = spriteU0 - 1;
                prim->v3 = spriteV1;
            } else {
                prim->u0 = spriteU1 - 1;
                prim->v0 = spriteV1 - 1;
                prim->u1 = spriteU0 - 1;
                prim->v1 = spriteV1 - 1;
                prim->u2 = spriteU1 - 1;
                prim->v2 = spriteV0 - 1;
                prim->u3 = spriteU0 - 1;
                prim->v3 = spriteV0 - 1;
            }
        }

        prim->tpage = 0x14;
        // Entity-relative z priority
        prim->priority = entity->zPriority + 1;

        // Next!
        prim = prim->next;
    }
    return prim;
}

// E_VENUS_WEED
void EntityVenusWeed(Entity* self) {
    Entity* entity;
    s32 x;
    s32 primIdx;
    s32 y;
    Primitive* prim;
    s32 checkCount;
    s32 i;
    s16 rot;

    // Death check
    if ((self->flags & FLAG_DEAD) && (self->step < 8)) {
        SetStep(8);
    }

    switch (self->step) {
    case 0:
        InitializeEntity(g_EInitBlueVenusWeed1);
        self->hitboxOffX = 1;
        self->hitboxOffY = -7;
        self->zPriority -= 8;

        // 3 Prims: 2x Leaves (left/right) + Stem
        primIdx = g_api.AllocPrimitives(PRIM_GT4, 3);
        if (primIdx == -1) {
            DestroyEntity(self);
            return;
        }
        self->primIndex = primIdx;
        prim = &g_PrimBuf[primIdx];
        self->ext.prim = prim;
        self->flags |= FLAG_HAS_PRIMS;

        // Leaves
        for (i = 0; i < 2; i++) {
            prim->tpage = 0x14;
            prim->clut = 0x250;
            prim->u0 = prim->u2 = 0x48;
            prim->u1 = prim->u3 = 0x48 + 0x38;
            prim->v0 = prim->v1 = 0;
            prim->v2 = prim->v3 = 0x22;
            prim->priority = self->zPriority - 1;
            prim->drawMode = DRAW_HIDE;

            prim = prim->next;
        }

        // Stem
        self->ext.venusWeed.stemPrim = prim;
        prim->tpage = 0x14;
        prim->clut = 0x250;
        prim->u1 = prim->u3 = 0x18;
        prim->v0 = prim->v1 = 0x30;
        prim->u0 = prim->u2 = 0;
        prim->v2 = prim->v3 = 0x30 + 0x22;
        prim->priority = self->zPriority - 2;
        prim->drawMode = DRAW_HIDE;
        break;

    case 1:
        if (UnkCollisionFunc3(D_us_801828B4) & 1) {
            SetStep(2);
        }
        break;

    case 2:
        AnimateEntity(D_us_801829D4, self);
        if (GetDistanceToPlayerX() < 0xA0) {
            self->hitboxState = 0;
            SetStep(3);
        }

        // Death check
        if (self->flags & FLAG_DEAD) {
            entity = AllocEntity(&g_Entities[224], &g_Entities[256]);
            if (entity != NULL) {
                CreateEntityFromEntity(E_EXPLOSION, self, entity);
                entity->params = 0;
                entity->posY.i.hi -= 4;
            }

            PlaySfxPositional(SFX_STUTTER_EXPLODE_LOW);
            DestroyEntity(self);
            return;
        }
        break;

    case 3:
        AnimateEntity(D_us_801829E0, self);

        checkCount = 0;
        switch (self->step_s) {
        case 0:
            // Update leaves width
            self->ext.venusWeed.leavesWidth += 2;
            if (self->ext.venusWeed.leavesWidth > 0x38) {
                self->ext.venusWeed.leavesWidth = 0x38;
                checkCount += 1;
            }

            // Update leaves height
            self->ext.venusWeed.leavesHeight += 2;
            if (self->ext.venusWeed.leavesHeight > 0x22) {
                self->ext.venusWeed.leavesHeight = 0x22;
                checkCount += 1;
            }

            // Update prims to match
            prim = self->ext.prim;
            x = self->posX.i.hi;
            y = self->posY.i.hi;
            y -= self->ext.venusWeed.leavesHeight;
            // 2 primitives: One for left and one for right
            for (i = -1; i < 2; i += 2) {
                prim->x0 = prim->x2 = x;
                prim->x1 = prim->x3 = x + self->ext.venusWeed.leavesWidth * i;
                prim->y0 = prim->y1 = y;
                prim->y2 = prim->y3 = self->posY.i.hi;
                prim->drawMode = DRAW_UNK02;

                prim = prim->next;
            }

            // Check for completion
            if (checkCount == 2) {
                self->step_s++;
            }
            break;

        case 1:
            // Update stem width
            self->ext.venusWeed.stemWidth += 2;
            if (self->ext.venusWeed.stemWidth > 0xC) {
                self->ext.venusWeed.stemWidth = 0xC;
                checkCount += 1;
            }

            // Update stem height
            self->ext.venusWeed.stemHeight += 2;
            if (self->ext.venusWeed.stemHeight > 0x22) {
                self->ext.venusWeed.stemHeight = 0x22;
                checkCount += 1;
            }

            // Update prim to match
            prim = self->ext.venusWeed.stemPrim;
            x = self->posX.i.hi;
            y = self->posY.i.hi - self->ext.venusWeed.stemHeight;
            prim->x0 = prim->x2 = x - self->ext.venusWeed.stemWidth;
            prim->y0 = prim->y1 = y;
            prim->x1 = prim->x3 = x + self->ext.venusWeed.stemWidth;
            prim->y2 = prim->y3 = self->posY.i.hi;
            prim->drawMode = DRAW_UNK02;

            // Check for completion
            if (checkCount == 2) {
                self->step_s++;
            }
            break;

        case 2:
            entity = self + 1; // Flower

            // Spawn flower
            CreateEntityFromCurrentEntity(E_UNK_35, entity);
            entity->posX.i.hi = self->posX.i.hi;
            entity->posY.i.hi = self->posY.i.hi - 0x1B;

            // Face the player
            entity->facingLeft = GetSideToPlayer() & 1;
            entity->zPriority = (s32)self->zPriority;

            self->step_s++;
            break;

        case 3:
            entity = self + 2; // Tendrils start
            for (i = 0; i < TENDRIL_COUNT; i++, entity++) {
                CreateEntityFromCurrentEntity(E_UNK_36, entity);
                entity->params = i;
                entity->zPriority = self->zPriority + 1;
            }

            self->step_s++;
            break;

        case 4:
            break;
        }
        break;

    case 4: // Set by the flower entity (self + 1)
        AnimateEntity(D_us_801829D4, self);
        break;

    case 5: // Set by the flower entity (self + 1)
        if (self->ext.venusWeed.triggerAttack) {
            self->ext.venusWeed.triggerAttack = false;
            self->ext.venusWeed.timer = 0x30;
        }
        if (self->ext.venusWeed.timer) {
            AnimateEntity(D_us_801829E0, self);
            self->ext.venusWeed.timer--;
        }
        break;

    case 8:
        switch (self->step_s) {
        case 0:
            self->ext.venusWeed.wiggleT = 0;
            self->step_s++;
            // fallthrough
        case 1:
            // Cycle thru cluts
            if (!(g_Timer & 7)) {
                // Switch to next clut
                self->palette += 1;
                // For primitives too
                prim = self->ext.prim;
                while (prim != NULL) {
                    prim->clut += 1;
                    prim = prim->next;
                }
                if (self->palette == 0x246) {
                    self->step_s++;
                }
            }
            break;

        case 2:
            checkCount = 0;
            self->ext.venusWeed.timer++;
            // Every other frame
            if (self->ext.venusWeed.timer & 1) {
                prim = self->ext.prim;
                x = self->posX.i.hi;

                // Shrink leaves
                if (self->ext.venusWeed.leavesWidth) {
                    self->ext.venusWeed.leavesWidth--;
                    if (self->ext.venusWeed.leavesWidth < 0) {
                        self->ext.venusWeed.leavesWidth = 0;
                    }
                }
                // Update leaves sprites
                for (i = -1; i < 2; i += 2) {
                    prim->x1 =
                        x + (self->ext.venusWeed.leavesWidth + 0x38) / 2 * i;
                    prim->y1 = rot = prim->y1 + 1;
                    prim->y0++;
                    prim->x3 = x + (self->ext.venusWeed.leavesWidth * i);
                    if (prim->y2 < rot) {
                        prim->drawMode = DRAW_HIDE;
                        checkCount += 1;
                    }
                    prim = prim->next;
                }
            }

            // Collapse stem
            prim = self->ext.venusWeed.stemPrim;
            prim->y0 = ++prim->y1;
            if (prim->y0 > prim->y2) {
                prim->drawMode = DRAW_HIDE;
                checkCount += 1;
            }

            // Check for completion
            if (checkCount == 3) {
                self->step_s += 1;
            }
            break;

        case 3:
            // Spawn explosion
            entity = AllocEntity(&g_Entities[224], &g_Entities[256]);
            if (entity != NULL) {
                CreateEntityFromEntity(E_EXPLOSION, self, entity);
                entity->params = 2;
                entity->posY.i.hi -= 0xC;
            }

            PlaySfxPositional(SFX_EXPLODE_B);

            // Destroy
            PreventEntityFromRespawning(self);
            DestroyEntity(self);
            return;
        }
    }

    // Update wiggle
    if (self->ext.venusWeed.wiggleT) {
        rot = self->rotate;
        self->rotate += 0x180;
        x = rcos(rot) * 3 >> 0xC;
        y = rsin(rot) * 3 >> 0xC;
        prim = self->ext.prim;

        // Update leaves
        for (i = -1; i < 2; i += 2) {
            prim->x1 = self->posX.i.hi + (x + 0x38) * i;
            prim->y1 = self->posY.i.hi - 0x22 + y * i;

            prim = prim->next;
        }
        x /= 2;

        // Update stem and flower
        entity = self + 1; // Flower
        prim = self->ext.venusWeed.stemPrim;
        self->ext.venusWeed.wiggleT--;
        if (!self->ext.venusWeed.wiggleT) {
            entity->posX.i.hi = self->posX.i.hi;
            prim->x0 = self->posX.i.hi - 0xC;
            prim->x1 = self->posX.i.hi + 0xC;
        } else {
            primIdx = x - 0xC;
            prim->x0 = self->posX.i.hi + primIdx;
            primIdx = x + 0xC;
            prim->x1 = self->posX.i.hi + primIdx;
            entity->posX.i.hi = self->posX.i.hi + x;
        }
    }
}

// E_UNK_35: Blue Venus Weed flower
void func_us_801C5850(Entity* self) {
    Entity* entity;
    s32 x;
    s32 rot;
    s32 i;
    s32 rotDelta;
    s32 y;
    s32 launchY;

    FntPrint("arla_step %x\n", self->step);
    FntPrint("arla_color %x\n", self->palette);

    // Hurt check
    if (self->hitFlags & 3) {
        PlaySfxPositional(SFX_VENUS_WEED_HURT);

        // Tell root to wiggle for a bit
        entity = self - 1; // Root
        entity->ext.venusWeed.wiggleT = 0x40;
    }
    // Death check
    if ((self->flags & FLAG_DEAD) && (self->step < 8)) {
        PlaySfxPositional(SFX_VENUS_WEED_DEATH);
        SetStep(8);
    }

    switch (self->step) {
    case 0:
        InitializeEntity(g_EInitBlueVenusWeed2);
        self->hitboxOffX = 6;
        self->hitboxOffY = -16;
        self->hitboxWidth = 14;
        self->hitboxHeight = 14;
        self->animCurFrame = 1;
        self->scaleX = self->scaleY = 0;
        self->hitboxState = 0;
        self->drawFlags |= ENTITY_SCALEX | ENTITY_SCALEY;
        break;

    case 1:
        self->scaleX = self->scaleY += 6;
        if (self->scaleX >= 0x100) {
            self->drawFlags = ENTITY_DEFAULT;
            self->hitboxState = 3;

            PlaySfxPositional(SFX_MAGIC_WEAPON_APPEAR_A);
            SetStep(2);
        }
        break;

    case 2:
        if (AnimateEntity(D_us_801828CC, self) == 0) {
            // Tell root to idle
            entity = self - 1; // Root
            entity->step = 4;
            entity->step_s = 0;

            SetStep(3);
        }
        break;

    case 3:
        // Init
        if (!self->step_s) {
            self->ext.venusWeedFlower.triggerAttack = 1;
            self->step_s++;
        }

        // Animate, occasionally turning to face player
        if (AnimateEntity(D_us_80182914, self) == 0) {
            self->facingLeft = GetSideToPlayer() & 1;
        }

        // Only once, when entering IDLE state
        if (!--self->ext.venusWeedFlower.triggerAttack) {
            // Face player
            self->facingLeft = GetSideToPlayer() & 1;

            // Spikes when the player is close, darts otherwise
            SetStep(5);
            if (GetDistanceToPlayerX() < 0x40) {
                SetStep(4);
            }
        }
        break;

    case 4: // Spikes attack
        switch (self->step_s) {
        case 0:
            // Ask all tendrils to get in position
            self->ext.venusWeedFlower.tendrilsReady = 0;
            entity = self + 1; // Tendrils start
            i = TENDRIL_COUNT - 1;
            do {
                entity->ext.venusWeedTendril.attackTrigger = 1;
                entity++;
            } while (--i >= 0);

            self->step_s++;
            // fallthrough
        case 1:
            AnimateEntity(D_us_80182914, self);
            if (self->ext.venusWeedFlower.tendrilsReady == TENDRIL_COUNT) {
                // Set root entity to attack
                entity = self - 1; // Root
                entity->step = 5;
                entity->step_s = 0;

                SetSubStep(2);
            }
            break;

        case 2:
            if (AnimateEntity(D_us_801828E4, self) == 0) {
                SetSubStep(3);
            }
            break;

        case 3:
            PlaySfxPositional(SFX_GLASS_SHARDS);

            // Spawn spikes
            entity = AllocEntity(&g_Entities[224], &g_Entities[256]);
            if (entity != NULL) {
                CreateEntityFromEntity(E_VENUS_WEED_SPIKE, self, entity);
                entity->facingLeft = self->facingLeft;
                entity->ext.venusWeedSpike.flower = self;
            }
            self->step_s++;
            // fallthrough
        case 4:
            if (AnimateEntity(D_us_801828F0, self) == 0) {
                entity = self + 1; // Tendrils start
                i = TENDRIL_COUNT - 1;
                do {
                    entity->ext.venusWeedTendril.spikeStartTimeOffsetIndex = 1;
                    entity++;
                } while (--i >= 0);
                SetSubStep(5);
            }
            break;

        case 5: // Anim: Reset to idle
            if (AnimateEntity(D_us_801828F8, self) == 0) {
                SetSubStep(6);
            }
            break;

        case 6:
            // Tell root to idle
            entity = self - 1; // Root
            entity->step = 4;

            SetStep(3);
            break;
        }
        break;

    case 5: // Darts attack
        switch (self->step_s) {
        case 0:
            entity = self - 1; // Root
            entity->step = 6;  // Non-existent state: "Do nothing"
            entity->step_s = 0;

            self->step_s += 1;
            // fallthrough
        case 1:
            if (AnimateEntity(D_us_80182920, self) == 0) {
                SetSubStep(2);
            }
            break;

        case 2:
            PlaySfxPositional(SFX_GLASS_SHARDS);
            entity = AllocEntity(&g_Entities[224], &g_Entities[256]);
            if (entity != NULL) {
                CreateEntityFromEntity(E_VENUS_WEED_SPIKE, self, entity);
                entity->facingLeft = self->facingLeft;
                entity->ext.venusWeedSpike.flower = self;
            }
            self->step_s++;
            // fallthrough
        case 3:
            if (AnimateEntity(D_us_80182930, self) == 0) {
                self->step_s++;
            }
            if (LOW(self->pose) == 3) {
                PlaySfxPositional(SFX_ARROW_SHOT_B);

                // Calculate launch start pos
                if (self->facingLeft) {
                    x = self->posX.i.hi - 0x18;
                } else {
                    x = self->posX.i.hi + 0x18;
                }

                // Calculate launch angle and delta between darts
                launchY = self->posY.i.hi - 0x18;
                y = rot = ratan2(
                    PLAYER.posY.i.hi - launchY, PLAYER.posX.i.hi - x);
                if (self->facingLeft) {
                    if ((s16)rot < 0) { // Angled up
                        rotDelta = -0x60;
                        if ((s16)rot >= -0x47F) {
                            y = -0x480;
                        }
                    } else { // Angled down
                        rotDelta = 0x60;
                        if ((s16)rot < 0x500) {
                            y = 0x500;
                        }
                    }
                } else if ((s16)rot < 0) { // Angled up
                    rotDelta = 0x60;
                    if ((s16)rot < -0x380) {
                        y = -0x380;
                    }
                } else { // Angled down
                    rotDelta = -0x60;
                    if ((s16)rot >= 0x301) {
                        y = 0x300;
                    }
                }

                // Spawn darts
                for (i = 0; i < 5; i++) {
                    entity = AllocEntity(&g_Entities[160], &g_Entities[192]);
                    if (entity != NULL) {
                        CreateEntityFromEntity(
                            E_VENUS_WEED_DART, self, entity);
                        entity->rotate = y;
                        entity->params = i;
                        entity->posX.i.hi = x;
                        entity->posY.i.hi -= 0x18;
                    }
                    y += rotDelta;
                }
            }
            break;

        case 4:
            entity = self - 1; // Root
            entity->step = 4;

            SetStep(3);
            break;
        }
        break;

    case 8:
        // Kill tendrils
        entity = self + 1; // Tendrils start
        for (i = 0; i < TENDRIL_COUNT; i++, entity++) {
            entity->flags |= FLAG_DEAD;
        }

        PlaySfxPositional(SFX_FM_EXPLODE_B);
        self->hitboxState = 0;

        entity = AllocEntity(&g_Entities[224], &g_Entities[256]);
        if (entity != NULL) {
            CreateEntityFromEntity(E_EXPLOSION, self, entity);
            entity->params = 3;
        }

        // Kill root
        entity = self - 1; // Root
        entity->flags |= FLAG_DEAD;

        DestroyEntity(self);
        return;
    }

    // Cycle clut when a tendril took a hit
    if (self->ext.venusWeedFlower.clutOffset) {
        entity = self - 1; // Root
        entity->ext.venusWeed.triggerAttack = true;
        if (!(self->palette & PAL_UNK_FLAG)) {
            self->palette += self->ext.venusWeedFlower.clutOffset >> 4;
            self->ext.venusWeedFlower.clutOffset &= 0xF;
            if (self->palette >= 0x250) {
                self->palette = 0x24F;
            }
        }
    }
}

// E_UNK_36: Blue Venus Weed tendril
// params: Index in group
void func_us_801C5F40(Entity* self) {
    s32 x;
    s8* hitboxData;
    Entity* entity;
    u32 hitboxIndex;

    if ((self->flags & FLAG_DEAD) && (self->step < 8)) {
        SetStep(8);
    }

    switch (self->step) {
    case 0:
        InitializeEntity(D_us_80180A1C);
        self->animCurFrame = 0;
        break;

    case 1:
        if (UnkCollisionFunc3(D_us_801828B4) & 1) {
            SetStep(2);
        }
        break;

    case 2:
        // Calculate target x positions
        if (!self->step_s) {
            s32 spread = self->params << 5;

            x = spread - 0x70;
            if (x > 0) {
                x = spread - 0x58;
            } else {
                x = spread - 0x88;
            }
            x += (Random() & 0x3F) - 0x1F;
            self->ext.venusWeedTendril.targetX = x;
            self->step_s++;
        }

        AnimateEntity(D_us_801829C8, self);
        UnkCollisionFunc2(D_us_801828C4); // "Walk", respecting walls/etc

        // Move towards the player when close, the random spot otherwise
        x = PLAYER.posX.i.hi - self->posX.i.hi;
        if (abs(x) >= 0x19) {
            entity = self - 1 - self->params; // Flower
            x = entity->posX.i.hi + self->ext.venusWeedTendril.targetX;
            x -= self->posX.i.hi; // Remaining distance
        }
        if (abs(x) < 2) {
            SetStep(5);
        } else if (x > 0) {
            self->velocityX = abs(x) << 0xC;
        } else {
            self->velocityX = -(abs(x) << 0xC);
        }

        // Attack requested by the flower
        if (self->ext.venusWeedTendril.attackTrigger) {
            self->ext.venusWeedTendril.attackTrigger = 0;
            entity = self - 1 - self->params; // Flower
            entity->ext.venusWeedFlower.tendrilsReady++;
            SetStep(3);
        }
        break;

    case 5: // Reached the player: bite
        if (AnimateEntity(D_us_80182948, self) == 0) {
            SetStep(3);
            self->step_s = 1;
            self->pose = 8;
        }
        break;

    case 3: // Attack
        switch (self->step_s) {
        case 0:
            AnimateEntity(D_us_801829C8, self);
            if (self->ext.venusWeedTendril.spikeStartTimeOffsetIndex) {
                self->ext.venusWeedTendril.spikeStartTimeOffsetIndex = 0;
                SetSubStep(1);
            }
            break;

        case 1:
            if (AnimateEntity(D_us_80182964, self) == 0) {
                SetSubStep(2);
            }
            if (LOW(self->pose) == 0xA) {
                PlaySfxPositional(SFX_VENUS_WEED_CHARGE_ATTACK);
            }
            break;

        case 2:
            if (AnimateEntity(D_us_801829A8, self) == 0) {
                SetStep(2);
            }
            break;
        }
        if (self->hitFlags & 0x80) {
            entity = self - 1 - self->params; // Flower
            entity->ext.venusWeedFlower.clutOffset++;
        }
        break;

    case 8:
        if (!self->step_s) {
            self->ext.venusWeedTendril.timer = self->params * 8 + 1;
            self->step_s++;
        }
        if (!--self->ext.venusWeedTendril.timer) {
            entity = AllocEntity(&g_Entities[224], &g_Entities[256]);
            if (entity != NULL) {
                CreateEntityFromEntity(E_EXPLOSION, self, entity);
                entity->params = 2;
                entity->posY.i.hi -= 0xC;
            }
            PlaySfxPositional(SFX_EXPLODE_B);
            DestroyEntity(self);
            return;
        }
        break;
    }

    // Update the hitbox based on the current animation frame
    hitboxData = D_us_801829FC;
    hitboxIndex = D_us_801829EA[self->animCurFrame];
    hitboxData += hitboxIndex * 4; // 4 entries per index
    self->hitboxOffX = *hitboxData++;
    self->hitboxOffY = *hitboxData++;
    self->hitboxWidth = *hitboxData++;
    self->hitboxHeight = *hitboxData++;
}

// E_VENUS_WEED_DART
// params: Index in group
//         Slightly influences acceleration
void EntityVenusWeedDart(Entity* self) {
    Collider collider;
    Entity* entity;
    s16 rot;
    s32 x;
    s32 speed;
    s32 y;
    u32 tmp;

    switch (self->step) {
    case 0:
        InitializeEntity(D_us_80180A28);
        self->animCurFrame = 0x37;
        self->drawFlags = ENTITY_ROTATE;
        rot = self->rotate;
        tmp = rcos(rot) * 3;
        self->hitboxOffX = tmp >> 0xB;
        tmp = rsin(rot) * 3;
        self->hitboxOffY = tmp >> 0xB;
        tmp = rcos(rot);
        self->ext.venusWeedDart.nextPosDeltaX = tmp >> 9;
        tmp = rsin(rot);
        self->ext.venusWeedDart.nextPosDeltaY = tmp >> 9;
        self->ext.venusWeedDart.speed = 0x8000;
        // fallthrough
    case 1:
        MoveEntity();

        rot = self->rotate;
        speed = self->ext.venusWeedDart.speed;
        self->velocityX = (speed * rcos(rot)) >> 0xC;
        self->velocityY = (speed * rsin(rot)) >> 0xC;
        self->ext.venusWeedDart.speed += self->ext.venusWeedDart.accel;
        self->ext.venusWeedDart.accel += (self->params + 1) * 0x800;
        if (self->ext.venusWeedDart.accel > 0x10000) {
            self->ext.venusWeedDart.accel = 0x10000;
        }
        if (self->ext.venusWeedDart.speed > 0x60000) {
            self->ext.venusWeedDart.speed = 0x60000;
        }

        x = self->posX.i.hi + self->ext.venusWeedDart.nextPosDeltaX;
        y = self->posY.i.hi + self->ext.venusWeedDart.nextPosDeltaY;
        g_api.CheckCollision(x, y, &collider, 0);
        if (collider.effects & EFFECT_SOLID) {
            PlaySfxPositional(SFX_STOMP_HARD_E);
            // Correct position to be against the edge
            if (self->velocityY > 0) {
                self->posY.i.hi += collider.unk18;
            }
            if (self->velocityY < 0) {
                self->posY.i.hi += collider.unk20;
            }
            self->hitboxState = 0;
            self->ext.venusWeedDart.clutIndex = 0x20;
            SetStep(3);
        }
        if (self->hitFlags & 0x80) {
            entity = &PLAYER;
            self->ext.venusWeedDart.nextPosDeltaX =
                entity->posX.i.hi - self->posX.i.hi;
            self->ext.venusWeedDart.nextPosDeltaY =
                entity->posY.i.hi - self->posY.i.hi;
            self->ext.venusWeedDart.clutIndex = 0;
            self->hitboxState = 0;
            SetStep(2);
        }
        break;

    case 2:
        if (!(self->palette & PAL_UNK_FLAG)) {
            s32 clutIdx = (u16)self->ext.venusWeedDart.clutIndex;

            self->palette = clutIdx + 0x241;
            self->ext.venusWeedDart.clutIndex = clutIdx + 1;

            if (self->palette >= 0x250) {
                self->palette = 0x24F;
            }
        }
        if (self->ext.venusWeedDart.clutIndex > 0x30) {
            self->flags |= FLAG_DEAD;
        }

        // Stick to player
        entity = &PLAYER;
        self->posX.i.hi =
            entity->posX.i.hi - self->ext.venusWeedDart.nextPosDeltaX;
        self->posY.i.hi =
            entity->posY.i.hi - self->ext.venusWeedDart.nextPosDeltaY;
        break;

    case 3:
        if (!--self->ext.venusWeedDart.clutIndex) {
            self->flags |= FLAG_DEAD;
        }
        break;
    }

    // Death check
    if (self->flags & FLAG_DEAD) {
        entity = AllocEntity(&g_Entities[224], &g_Entities[256]);
        if (entity != NULL) {
            CreateEntityFromEntity(E_EXPLOSION, self, entity);
            entity->params = 0;
        }
        DestroyEntity(self);
    }
}

// E_VENUS_WEED_SPIKE
void EntityVenusWeedSpike(Entity* self) {
    Primitive* prim;
    Primitive* primItr;
    Primitive* primNext;
    s32 primIdx;
    Entity* entity;
    s16 clut;

    switch (self->step) {
    case 0:
        InitializeEntity(g_EInitBlueVenusWeed2);

        self->hitboxState = 0;
        self->palette = PAL_FLAG(0x25A);
        self->flags |= FLAG_UNK_2000 | FLAG_UNK_00200000;

        primIdx = g_api.AllocPrimitives(PRIM_GT4, 5);
        if (primIdx == -1) {
            DestroyEntity(self);
            break;
        } else {
            self->primIndex = primIdx;
            prim = &g_PrimBuf[primIdx];
            self->ext.venusWeedSpike.firstPart = prim;

            entity = self->ext.venusWeedSpike.flower;
            entity--; // Root
            self->flags |= FLAG_HAS_PRIMS;

            // Draw sprite parts
            prim = SetupPrimsForEntitySpriteParts(entity, prim);
            // Above returns the following prim

            // Copy prims to a later index (while maintaining linked list order)
            for (primItr = entity->ext.venusWeedSpike.firstPart;
                 primItr != NULL; primItr = primItr->next, prim = primNext) {
                primNext = prim->next;

                *prim = *primItr;
                prim->next = primNext;
                prim->priority = primItr->priority + 1;
            }
        }

        // Update to match flower
        entity = entity + 1; // Flower
        self->animCurFrame = entity->animCurFrame;
        self->zPriority = entity->zPriority + 1;
        // Fallthrough
    case 1:
        clut = self->palette & 0xFFF;
        FntPrint("color %x\n", self->palette);

        prim = self->ext.venusWeedSpike.firstPart;
        while (prim != NULL) {
            prim->clut = clut;
            prim->drawMode = DRAW_UNK02;

            prim = prim->next;
        }

        // Update to match flower
        entity = self->ext.venusWeedSpike.flower;
        self->animCurFrame = entity->animCurFrame;
        clut = self->palette + 1;
        self->palette = clut;
        if (((clut & 0xFFF) >= 0x269) || (entity->entityId != E_UNK_35)) {
            DestroyEntity(self);
        }
        break;
    }
}
