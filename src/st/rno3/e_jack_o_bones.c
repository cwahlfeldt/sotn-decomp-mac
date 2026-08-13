// SPDX-License-Identifier: AGPL-3.0-or-later
#include "rno3.h"
#include "sfx.h"

// Jack O'Bones is a reverse-castle variant of the shared skeleton
// (src/st/e_skeleton.h): same four-function layout and ext fields, but the
// thrown bone bounces around the room instead of falling off-screen, and the
// params bit selects an alternate palette/throw arc.

extern EInit g_EInitJackOBones;
extern EInit D_us_80180980; // death pieces
extern EInit D_us_8018098C; // thrown bone

// Animation frames, tables and collision sensors; still in the raw data
// blob at RNO3.BIN+0x2548.
extern u8 D_us_80182548[];     // anim: walk towards player
extern u8 D_us_80182558[];     // anim: walk away from player
extern u8 D_us_80182568[];     // anim: throw bone
extern u8 D_us_80182580[];     // anim: jump
extern u8 D_us_8018258C[];     // anim: land
extern u16 D_us_80182598[];    // death piece rotation speeds
extern u8 D_us_801825A8[];     // death piece explosion timers
extern s32 D_us_801825B0[];    // death piece X velocities
extern s32 D_us_801825CC[];    // death piece Y velocities
extern u16 D_us_801825E8[];    // death piece X offsets
extern u16 D_us_801825F8[];    // death piece Y offsets
extern u8 D_us_80182608[2][4]; // attack timer cycles
extern s16 D_us_80182610[];    // ground sensors
extern s16 D_us_80182620[];    // special sensors
extern s16 D_us_80182628[];    // move sensors

void func_us_801C1DCC(void) {
    s32 collision;
    s32 moveCollision;

    collision = UnkCollisionFunc2(D_us_80182620);
    moveCollision = UnkCollisionFunc(D_us_80182628, 3);
    if (collision == 0x80 || (moveCollision & 2)) {
        SetStep(5);
    } else if (!g_CurrentEntity->ext.skeleton.attackTimer) {
        SetStep(4);
    } else {
        g_CurrentEntity->ext.skeleton.attackTimer--;
    }
}

void EntityJackOBones(Entity* self) {
    Entity* newEntity;
    u8 animStatus;
    s32 xOfs;
    s32 i;

    if (self->flags & FLAG_DEAD) {
        self->step = 6;
    }

    switch (self->step) {
    case 0:
        InitializeEntity(g_EInitJackOBones);
        if (self->params) {
            self->palette++;
        }
        self->facingLeft = (GetSideToPlayer() & 1) ^ 1;
        self->ext.skeleton.attackTimer = 80;
        self->ext.skeleton.facingLeft = 0;
        self->ext.skeleton.attackTimerIndex = 0;
        break;
    case 1: // Wait for player to be close enough
        if (UnkCollisionFunc3(D_us_80182610) != 0) {
            self->step++;
        }
        break;
    case 2: // Walk towards player
        if (!AnimateEntity(D_us_80182548, self)) {
            self->facingLeft = (GetSideToPlayer() & 1) ^ 1;
        }
        self->ext.skeleton.facingLeft = self->facingLeft;

        if (self->ext.skeleton.facingLeft) {
            self->velocityX = FIX(0.5);
        } else {
            self->velocityX = FIX(-0.5);
        }

        if (GetDistanceToPlayerX() < 76) {
            self->step = 3;
        }
        func_us_801C1DCC();
        break;
    case 3: // Walk away from player
        if (!AnimateEntity(D_us_80182558, self)) {
            self->facingLeft = (GetSideToPlayer() & 1) ^ 1;
        }
        self->ext.skeleton.facingLeft = self->facingLeft ^ 1;

        if (self->ext.skeleton.facingLeft) {
            self->velocityX = FIX(0.5);
        } else {
            self->velocityX = FIX(-0.5);
        }

        if (GetDistanceToPlayerX() > 92) {
            self->step = 2;
        }
        func_us_801C1DCC();
        break;
    case 4: // Throw bone
        animStatus = AnimateEntity(D_us_80182568, self);
        if (!animStatus) {
            SetStep(3);
            self->ext.skeleton.attackTimer =
                D_us_80182608[self->params & 1]
                             [++self->ext.skeleton.attackTimerIndex & 3];
            break;
        }

        if ((animStatus & 0x80) && (self->animCurFrame == 11)) {
            newEntity = AllocEntity(&g_Entities[160], &g_Entities[192]);
            if (newEntity != NULL) {
                PlaySfxPositional(SFX_BONE_THROW);
                CreateEntityFromCurrentEntity(E_UNK_26, newEntity);
                xOfs = 8;
                if (self->params) {
                    xOfs = -16;
                }
                if (self->facingLeft) {
                    newEntity->posX.i.hi -= xOfs;
                } else {
                    newEntity->posX.i.hi += xOfs;
                }
                newEntity->posY.i.hi -= 16;
                newEntity->params = self->params;
                newEntity->facingLeft = self->facingLeft;
            }
        }
        break;
    case 5: // Jump
        switch (self->step_s) {
        case 0:
            if (!(AnimateEntity(D_us_80182580, self) & 1)) {
                animStatus = self->ext.skeleton.facingLeft;
                if (!(Random() & 3)) {
                    animStatus ^= 1;
                }

                if (animStatus) {
                    self->velocityX = FIX(2);
                } else {
                    self->velocityX = FIX(-2);
                }

                self->velocityY = FIX(-3);
                self->pose = 0;
                self->poseTimer = 0;
                self->step_s++;
            }
            break;
        case 1:
            if (UnkCollisionFunc3(D_us_80182610) != 0) {
                self->step_s++;
            }
            CheckFieldCollision(D_us_80182628, 2);
            break;
        case 2:
            if (!AnimateEntity(D_us_8018258C, self)) {
                SetStep(3);
            }
            break;
        }
        break;
    case 6: // Death, spawn bone pieces
        PlaySfxPositional(SFX_SKELETON_DEATH_B);
        for (i = 0; i < 6; i++) {
            newEntity = AllocEntity(&g_Entities[224], &g_Entities[256]);
            if (newEntity != NULL) {
                CreateEntityFromCurrentEntity(E_UNK_25, newEntity);
                newEntity->params = i;
                newEntity->facingLeft = self->facingLeft;
                newEntity->params = i | (self->params << 8);
                newEntity->ext.skeleton.explosionTimer = D_us_801825A8[i];
                if (self->facingLeft) {
                    newEntity->posX.i.hi -= D_us_801825E8[i];
                } else {
                    newEntity->posX.i.hi += D_us_801825E8[i];
                }
                newEntity->posY.i.hi += D_us_801825F8[i];
                newEntity->velocityX = D_us_801825B0[i];
                newEntity->velocityY = D_us_801825CC[i];
            } else {
                break;
            }
        }
        DestroyEntity(self);
        break;
    }
}

void func_us_801C2380(Entity* self) { // From Jack O'Bones death explosion
    if (self->step) {
        if (--self->ext.skeleton.explosionTimer) {
            self->rotate += D_us_80182598[self->params];
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

    InitializeEntity(D_us_80180980);
    self->animCurFrame = (u8)self->params + 15;
    if (self->params & 0x100) {
        self->palette++;
    }
    self->drawFlags = ENTITY_ROTATE;

    if (self->facingLeft) {
        self->velocityX = -self->velocityX;
    }
}

void func_us_801C247C(Entity* self) { // Bone projectile from Jack O'Bones
    Collider collider;
    s32 newVelocityY;
    s32 velocityX;
    s32 velocityY;
    s32 bounceCount;
    s32 solidMask;

    solidMask = EFFECT_SOLID;
    if (self->step == 0) {
        InitializeEntity(D_us_8018098C);
        if (self->params) {
            self->palette++;
        }
        self->animCurFrame = 21;
        self->drawFlags |= ENTITY_ROTATE;

        if (self->params) {
            velocityY = FIX(4);
            velocityX = FIX(1);
        } else {
            velocityY = FIX(-1);
            velocityX = FIX(2.5);
        }
        if (self->facingLeft) {
            self->velocityX = velocityX;
        } else {
            self->velocityX = -velocityX;
        }
        self->velocityY = velocityY;
    }

    MoveEntity();
    self->velocityY += FIX(0.1875);
    self->rotate -= 0x40;

    g_api.CheckCollision(self->posX.i.hi, self->posY.i.hi + 5, &collider, 0);
    if (collider.effects & solidMask) { // Bounce off the floor
        PlaySfxPositional(SFX_SKULL_KNOCK_A);
        bounceCount = (u16)self->ext.skeleton.bounceCount + 1;
        newVelocityY = self->velocityY;
        if (newVelocityY < 0) {
            newVelocityY = -newVelocityY;
        }
        velocityY = -newVelocityY;
        self->ext.skeleton.bounceCount = bounceCount;
        self->velocityY = velocityY;
        velocityX = collider.unk18;
        self->posY.i.hi += velocityX;
        if (self->params) {
            self->velocityY = FIX(-7) / self->ext.skeleton.bounceCount;
        } else {
            self->velocityY = velocityY - velocityY / 16;
        }
    }

    g_api.CheckCollision(self->posX.i.hi, self->posY.i.hi - 5, &collider, 0);
    if (collider.effects & EFFECT_SOLID) { // Bounce off the ceiling
        bounceCount = (velocityY = (u16)self->posY.i.hi) + (u16)collider.unk20;
        newVelocityY = self->velocityY;
        if (newVelocityY < 0) {
            self->posY.i.hi = bounceCount;
            newVelocityY = -newVelocityY;
            self->velocityY = newVelocityY;
        }
    }

    velocityX = self->posX.i.hi;
    velocityY = self->posY.i.hi;
    if (self->velocityX > 0) {
        velocityX += 5;
    } else {
        velocityX -= 5;
    }
    g_api.CheckCollision(velocityX, velocityY, &collider, 0);
    if (collider.effects & EFFECT_SOLID) { // Bounce off walls
        self->velocityX = -self->velocityX;
    }

    if (self->params && self->ext.skeleton.bounceCount >= 9) {
        self->flags |= FLAG_DEAD;
    }

    if (self->flags & FLAG_DEAD) {
        self->entityId = E_EXPLOSION;
        self->drawFlags = 0;
        self->pfnUpdate = (PfnEntityUpdate)EntityExplosion;
        self->params = 0;
        self->step = 0;
    }
}
