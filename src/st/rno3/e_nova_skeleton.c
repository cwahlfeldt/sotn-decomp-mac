// SPDX-License-Identifier: AGPL-3.0-or-later
/*
 * File: e_nova_skeleton.c
 * Overlay: RNO3
 * Description: ENTITY - Nova Skeleton and its laser beam
 *
 * Walks like ARE's Blade Soldier (shared death-parts pattern) but fires a
 * charged laser beam. The beam is three stretched GT4 prims plus a GTE
 * rotated/scaled muzzle glow quad, and leaves scaling after-glow sprites.
 */

#include "rno3.h"
#include "sfx.h"

extern EInit g_EInitNovaSkeleton;
extern EInit D_us_801809A4; // beam / after-glow

// Data still in the raw blob at RNO3.BIN+0x2548.
extern u8 D_us_80182634[];  // physics sensors (drop to ground)
extern u8 D_us_80182644[];  // walk sensors
extern u8 D_us_8018264C[];  // anim: walk towards player
extern u8 D_us_8018265C[];  // anim: walk away from player
extern u8 D_us_80182684[];  // anim: idle
extern u8 D_us_80182690[];  // anim: beam charge
extern u8 D_us_801826D4[];  // anim: beam fire
extern u16 D_us_80182704[]; // death piece rotation speeds
extern u8 D_us_80182714[];  // death piece explosion timers
extern s32 D_us_8018271C[]; // death piece X velocities
extern s32 D_us_8018273C[]; // death piece Y velocities
extern u16 D_us_8018275C[]; // death piece X offsets
extern u16 D_us_8018276C[]; // death piece Y offsets
extern u8 D_us_8018277C[];  // attack cooldown cycle

extern SVECTOR D_us_80182790; // muzzle glow quad corners
extern SVECTOR D_us_80182798;
extern SVECTOR D_us_801827A0;
extern SVECTOR D_us_801827A8;

extern u8 D_us_801827B0[]; // beam segment u/color data

extern SVECTOR D_us_801B013C; // zero rotation for the muzzle glow

// Walk-state helper: turn towards the player when close, after a cooldown
void func_us_801C2740(void) {
    u16 facingLeft;

    UnkCollisionFunc2(D_us_80182644);
    if (g_CurrentEntity->ext.novaSkeleton.attackTimer == 0) {
        if (GetDistanceToPlayerX() < 0x80) {
            facingLeft = g_CurrentEntity->facingLeft;
            if (facingLeft != (GetSideToPlayer() & 1)) {
                SetStep(6);
            }
        }
    } else {
        g_CurrentEntity->ext.novaSkeleton.attackTimer--;
    }
}

INCLUDE_RODATA("st/rno3/nonmatchings/e_nova_skeleton", D_us_801B013C);

// Muzzle glow: a GTE-rotated, scaled quad in front of the skull
void func_us_801C27E0(void) {
    SVECTOR rot;
    VECTOR v;
    MATRIX m;
    SVECTOR sv;
    s32 p;
    s32 flag;
    Primitive* prim;
    s32 x;
    s32 y;

    sv = D_us_801B013C;
    switch (g_CurrentEntity->ext.novaSkeleton.beamStep) {
    case 0:
        prim = g_CurrentEntity->ext.prim;
        g_CurrentEntity->ext.novaSkeleton.beamScale = 0;
        prim->r0 = prim->g0 = prim->b0 = 0xC0;
        prim->drawMode =
            DRAW_TPAGE2 | DRAW_TPAGE | DRAW_COLORS | DRAW_TRANSP | DRAW_UNK02;
        LOW(prim->r1) = LOW(prim->r0);
        LOW(prim->r2) = LOW(prim->r0);
        LOW(prim->r3) = LOW(prim->r0);
        g_CurrentEntity->ext.novaSkeleton.beamStep = 1;
        break;
    case 1:
        g_CurrentEntity->ext.novaSkeleton.beamRot += 0x100;
        g_CurrentEntity->ext.novaSkeleton.beamScale += 0x200;
        break;
    }

    SetGeomScreen(0x200);
    x = g_CurrentEntity->posX.i.hi;
    y = g_CurrentEntity->posY.i.hi;
    if (g_CurrentEntity->facingLeft) {
        x += 0xA;
    } else {
        x -= 0xA;
    }
    SetGeomOffset(x, y - 2);

    rot.vx = 0;
    if (g_CurrentEntity->facingLeft) {
        rot.vy = -0x2E0;
    } else {
        rot.vy = 0x2E0;
    }
    rot.vz = g_CurrentEntity->ext.novaSkeleton.beamRot;
    RotMatrix(&sv, &m);
    RotMatrixZ(rot.vz, &m);
    RotMatrixY(rot.vy, &m);
    v.vx = 0;
    v.vy = 0;
    v.vz = 0x200;
    TransMatrix(&m, &v);
    v.vx = g_CurrentEntity->ext.novaSkeleton.beamScale;
    v.vy = g_CurrentEntity->ext.novaSkeleton.beamScale;
    v.vz = 0x1000;
    ScaleMatrix(&m, &v);
    SetRotMatrix(&m);
    SetTransMatrix(&m);
    prim = g_CurrentEntity->ext.prim;
    RotTransPers4(&D_us_80182790, &D_us_80182798, &D_us_801827A0,
                  &D_us_801827A8, (long*)&prim->x0, (long*)&prim->x1,
                  (long*)&prim->x2, (long*)&prim->x3, &p, &flag);
}

// E_NOVA_SKELETON
void EntityNovaSkeleton(Entity* self) {
    Primitive* prim;
    Entity* entity;
    s32 primIdx;
    s32 i;
    s32 j;
    u16* xOfs;

    if (self->flags & FLAG_DEAD) {
        SetStep(8);
    }

    switch (self->step) {
    case 0:
        InitializeEntity(g_EInitNovaSkeleton);
        self->ext.novaSkeleton.attackTimer = 0x50;

        primIdx = g_api.AllocPrimitives(PRIM_GT4, 1);
        if (primIdx == -1) {
            DestroyEntity(self);
            return;
        }
        self->primIndex = primIdx;
        prim = &g_PrimBuf[primIdx];
        self->ext.prim = prim;
        self->flags |= FLAG_HAS_PRIMS;
        UnkPolyFunc2(prim, primIdx);
        prim->tpage = 0x12;
        prim->clut = 0x216;
        prim->u0 = prim->u2 = 0xC0;
        prim->u1 = prim->u3 = 0xFF;
        prim->v0 = prim->v1 = 0;
        prim->v2 = prim->v3 = 0x40;
        prim->drawMode = DRAW_HIDE;
        prim->priority = self->zPriority + 1;
        break;

    case 1:
        if (UnkCollisionFunc3(D_us_80182634)) {
            SetStep(2);
        }
        break;

    case 2: // Idle until the player is close
        self->facingLeft = (GetSideToPlayer() & 1) ^ 1;
        AnimateEntity(D_us_80182684, self);
        if (GetDistanceToPlayerX() < 0x70) {
            SetStep(4);
        }
        break;

    case 3: // Walk towards the player
        if (AnimateEntity(D_us_8018264C, self) == 0) {
            self->facingLeft = (GetSideToPlayer() & 1) ^ 1;
        }
        self->ext.novaSkeleton.facingCache = self->facingLeft;
        if (self->ext.novaSkeleton.facingCache) {
            self->velocityX = FIX(0.5);
        } else {
            self->velocityX = FIX(-0.5);
        }
        if (GetDistanceToPlayerX() < 0x4C) {
            self->step = 4;
        }
        func_us_801C2740();
        break;

    case 4: // Walk away from the player
        if (AnimateEntity(D_us_8018265C, self) == 0) {
            self->facingLeft = (GetSideToPlayer() & 1) ^ 1;
        }
        self->ext.novaSkeleton.facingCache = self->facingLeft ^ 1;
        if (self->ext.novaSkeleton.facingCache) {
            self->velocityX = FIX(0.5);
        } else {
            self->velocityX = FIX(-0.5);
        }
        if (GetDistanceToPlayerX() >= 0x5D) {
            self->step = 3;
        }
        func_us_801C2740();
        break;

    case 6: // Charge the beam
        if (AnimateEntity(D_us_80182690, self) == 0) {
            self->ext.novaSkeleton.beamStep = 0;
            SetStep(7);
        }
        if (LOW(self->pose) == 2) {
            PlaySfxPositional(0x614);
        }
        break;

    case 7: // Fire the beam
        switch (self->step_s) {
        case 0:
            entity = self + 1;
            CreateEntityFromEntity(E_UNK_28, self, entity);
            if (self->facingLeft) {
                entity->posX.i.hi += 0xA;
            } else {
                entity->posX.i.hi -= 0xA;
            }
            entity->posY.i.hi -= 4;
            entity->facingLeft = self->facingLeft;
            self->step_s++;
            break;
        case 1:
            PrimDecreaseBrightness(self->ext.prim, 5);
            break;
        }
        func_us_801C27E0();
        if (AnimateEntity(D_us_801826D4, self) == 0) {
            self->ext.prim->drawMode = DRAW_HIDE;
            self->ext.novaSkeleton.attackTimerIndex++;
            self->ext.novaSkeleton.attackTimer =
                D_us_8018277C[self->ext.novaSkeleton.attackTimerIndex & 7];
            SetStep(4);
        }
        break;

    case 8: // Death, spawn bone pieces
        i = 0;
        j = 0;
        xOfs = D_us_8018275C;
        do {
            entity = AllocEntity(&g_Entities[224], &g_Entities[256]);
            if (entity == NULL) {
                break;
            }
            CreateEntityFromCurrentEntity(E_BLADE_SOLDIER_DEATH_PARTS, entity);
            entity->params = i;
            entity->facingLeft = self->facingLeft;
            entity->ext.novaSkeleton.explosionTimer = D_us_80182714[i];
            if (self->facingLeft) {
                entity->posX.i.hi -= *xOfs;
            } else {
                entity->posX.i.hi += *xOfs;
            }
            entity->posY.i.hi += *(u16*)((u8*)D_us_8018276C + j);
            j += 2;
            entity->velocityX = D_us_8018271C[i];
            entity->velocityY = D_us_8018273C[i];
            i++;
            xOfs += 2;
        } while (i < 6);
        PlaySfxPositional(0x62A);
        DestroyEntity(self);
        break;
    }
}

// E_BLADE_SOLDIER_DEATH_PARTS
void EntityBladeSoldierDeathParts(Entity* self) {
    if (self->step) {
        if (--self->ext.novaSkeleton.explosionTimer) {
            self->rotate += D_us_80182704[self->params];
            FallEntity();
            MoveEntity();
            return;
        }
        self->entityId = E_EXPLOSION;
        self->pfnUpdate = (PfnEntityUpdate)EntityExplosion;
        self->params = 0;
        self->step = 0;
        return;
    }

    InitializeEntity(g_EInitNovaSkeleton);
    self->flags |= FLAG_DESTROY_IF_OUT_OF_CAMERA |
                   FLAG_DESTROY_IF_BARELY_OUT_OF_CAMERA | FLAG_UNK_00200000 |
                   FLAG_UNK_2000;
    self->drawFlags = ENTITY_ROTATE;
    self->hitboxState = 0;
    self->animCurFrame = self->params + 0x1D;
    if (self->facingLeft) {
        self->velocityX = -self->velocityX;
    }
}

// E_UNK_28: Nova Skeleton laser beam
void func_us_801C2FF0(Entity* self) {
    Primitive* prim;
    Entity* entity;
    s32 primIdx;
    s32 i;
    u8* data;
    s16 x;
    s16 y;
    u8 u;
    u8 brightness;

    switch (self->step) {
    case 0:
        InitializeEntity(D_us_801809A4);
        primIdx = g_api.AllocPrimitives(PRIM_GT4, 3);
        if (primIdx == -1) {
            DestroyEntity(self);
            return;
        }
        self->primIndex = primIdx;
        prim = &g_PrimBuf[primIdx];
        self->ext.prim = prim;
        self->flags |= FLAG_HAS_PRIMS;
        data = D_us_801827B0;
        for (i = 0; i < 3; i++) {
            prim->tpage = 0x12;
            prim->clut = 0x216;
            u = *data++ - 0x80;
            prim->u0 = prim->u2 = u;
            prim->v0 = prim->v1 = 0x40;
            prim->v2 = prim->v3 = 0x5F;
            u = *data++ - 0x80;
            prim->u1 = prim->u3 = u;
            brightness = *data++;
            prim->r0 = prim->g0 = prim->b0 = brightness;
            LOW(prim->r2) = LOW(prim->r0);
            brightness = *data++;
            prim->r1 = prim->g1 = prim->b1 = brightness;
            LOW(prim->r3) = LOW(prim->r1);
            prim->drawMode = DRAW_TPAGE2 | DRAW_TPAGE | DRAW_COLORS |
                             DRAW_TRANSP | DRAW_UNK02;
            prim->priority = self->zPriority + 2;
            prim = prim->next;
        }
        self->ext.novaSkeletonBeam.timer = 0x60;
        self->ext.novaSkeletonBeam.width = 0;
        // fallthrough
    case 1: // Extend
        self->ext.novaSkeletonBeam.halfHeight = 0x10;
        if (self->ext.novaSkeletonBeam.width < 0x80) {
            self->ext.novaSkeletonBeam.width += 0x10;
        } else {
            self->ext.novaSkeletonBeam.width = 0x80;
            self->hitboxState = 1;
            self->step++;
        }
        // fallthrough
    case 2: // Spawn after-glow
        if (!(self->ext.novaSkeletonBeam.timer & 3)) {
            entity = AllocEntity(&g_Entities[224], &g_Entities[256]);
            if (entity != NULL) {
                CreateEntityFromEntity(E_UNK_29, self, entity);
                entity->zPriority = self->zPriority - 1;
                entity->ext.novaSkeletonBeam.width =
                    self->ext.novaSkeletonBeam.width;
                entity->facingLeft = self->facingLeft;
            }
        }
        if (!(self->ext.novaSkeletonBeam.timer & 0xF)) {
            PlaySfxPositional(0x61E);
        }
        if ((s16)self->ext.novaSkeletonBeam.timer < 0x10) {
            PlaySfxPositional(0x621);
            self->step++;
        }
        // fallthrough
    case 3: // Sustain
        if (Random() & 1) {
            x = self->ext.novaSkeletonBeam.width;
            if (x < 0x88) {
                self->ext.novaSkeletonBeam.width = x + 1;
            } else if (x >= 0x79) {
                self->ext.novaSkeletonBeam.width = x - 1;
            }
        }
        self->hitboxHeight = 8;
        self->hitboxWidth = ((s16)self->ext.novaSkeletonBeam.width / 2) + 0x10;
        self->hitboxOffX = (-(s16)self->ext.novaSkeletonBeam.width / 2) - 0x10;
        entity = self - 1; // Parent skeleton
        if (entity->entityId != E_NOVA_SKELETON) {
            self->ext.novaSkeletonBeam.timer = 1;
        }
        if (!--self->ext.novaSkeletonBeam.timer) {
            self->hitboxState = 0;
            self->step++;
        }
        break;

    case 4: // Collapse
        if (!--self->ext.novaSkeletonBeam.halfHeight) {
            DestroyEntity(self);
            return;
        }
        break;
    }

    // Layout the three beam quads
    x = self->posX.i.hi;
    y = self->posY.i.hi;
    prim = self->ext.prim;
    for (i = 0; i < 3; i++) {
        prim->y0 = prim->y1 = y - self->ext.novaSkeletonBeam.halfHeight;
        prim->y2 = prim->y3 = y + self->ext.novaSkeletonBeam.halfHeight;
        if (g_Timer & 1) {
            prim->clut = 0x216;
        } else {
            prim->clut = 0x217;
        }
        prim = prim->next;
    }
    prim = self->ext.prim;
    if (self->facingLeft) {
        x -= 0x10;
    } else {
        x += 0x10;
    }
    prim->x1 = prim->x3 = x;
    if (self->facingLeft) {
        x += 0x20;
    } else {
        x -= 0x20;
    }
    prim->x0 = prim->x2 = x;
    prim = prim->next;
    prim->x1 = prim->x3 = x;
    if (self->facingLeft) {
        x += self->ext.novaSkeletonBeam.width;
    } else {
        x -= self->ext.novaSkeletonBeam.width;
    }
    prim->x0 = prim->x2 = x;
    prim = prim->next;
    prim->x1 = prim->x3 = x;
    if (self->facingLeft) {
        x += 0x20;
    } else {
        x -= 0x20;
    }
    prim->x0 = prim->x2 = x;
}

// E_UNK_29: beam after-glow, scales up then fades out off-screen
void func_us_801C34A0(Entity* self) {
    s32 vel;
    s32 traveled;
    s32 scale;
    s32 limit;

    switch (self->step) {
    case 0:
        InitializeEntity(D_us_801809A4);
        self->animCurFrame = 0x24;
        self->scaleX = self->scaleY = 0x10;
        self->hitboxState = 0;
        self->drawFlags |= ENTITY_SCALEX | ENTITY_SCALEY;
        if (self->facingLeft) {
            self->velocityX = FIX(8);
        } else {
            self->velocityX = FIX(-8);
        }
        // fallthrough
    case 1: // Scale up
        MoveEntity();
        traveled =
            abs(self->velocityX) + self->ext.novaSkeletonBeam.traveled;
        scale = (u16)self->scaleY + 0x40;
        self->ext.novaSkeletonBeam.traveled = traveled;
        self->scaleX = self->scaleY = scale;
        if ((s16)scale >= 0x100) {
            self->step++;
        }
        break;

    case 2: // Fly until past the beam tip
        MoveEntity();
        limit = (s16)self->ext.novaSkeletonBeam.width;
        limit = (limit + 0x20) << 0x10;
        traveled = self->ext.novaSkeletonBeam.traveled =
            abs(self->velocityX) + self->ext.novaSkeletonBeam.traveled;
        limit -= traveled;
        if ((limit >> 0x10) < 0) {
            DestroyEntity(self);
            return;
        }
        break;
    }
}
