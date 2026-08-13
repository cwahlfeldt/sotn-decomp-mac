// SPDX-License-Identifier: AGPL-3.0-or-later
/*
 * File: unk_435F8.c
 * Overlay: RNO3
 * Description: ENTITY - the two Orobourous serpents and their riders
 *
 * Each serpent is a bouncing head entity followed by 24 trailing segment
 * entities and a rider. When the rider is destroyed the first serpent
 * gathers its segments and flies at the player; the second serpent hops
 * across the ground. Both explode into the same set of body pieces.
 */

#include "rno3.h"
#include "sfx.h"

extern EInit g_EInitOrobourous;
extern EInit g_EInitOruburos;
extern EInit D_us_801809C8; // serpent pieces
extern EInit g_EInitOruburosRider;
extern EInit g_EInitDragonRider1;
extern EInit g_EInitDragonRider2;
extern EInit D_us_801809F8; // second serpent pieces

// Data still in the raw blob at RNO3.BIN+0x2548.
extern u8 D_us_801827BC[];  // anim: serpent segment undulate
extern u8 D_us_801827CC[];  // anim: serpent head turn
extern u8 D_us_801827D4[];  // physics sensors (drop to ground)
extern s32 D_us_801827E4[]; // piece velocities/rotations (stride 3)
extern s32 D_us_801827EC[]; // = D_us_801827E4 + 2, rotation speeds
extern u16 D_us_80182814[]; // serpent head hitboxes by animCurFrame
extern u8 D_us_8018282C[];  // anim: second serpent segment undulate
extern u8 D_us_8018283C[];  // anim: second serpent head turn
extern u8 D_us_80182844[];  // physics sensors (second serpent)
extern s32 D_us_80182854[]; // second piece velocities (stride 3)
extern s32 D_us_8018285C[]; // = D_us_80182854 + 2, rotation speeds
extern u16 D_us_80182884[]; // second serpent head hitboxes
extern u16 D_us_8018289C[]; // second rider hitboxes

void func_us_801C4334(Entity* self);

// E_UNK_2B: first Orobourous serpent head
void func_us_801C35F8(Entity* self) {
    Collider collider;
    Entity* entity;
    Entity* prev;
    s32 i;
    s32 j;
    s32 dx;
    s32 dy;
    s16 angle;
    u16* data;
    u32 x;

    if (self->flags & FLAG_DEAD) {
        SetStep(8);
    }

    switch (self->step) {
    case 0:
        InitializeEntity(g_EInitOrobourous);
        self->animCurFrame = 0xE;
        self->drawFlags |= ENTITY_ROTATE;
        for (entity = self + 1, i = 0; i < 24;) {
            CreateEntityFromEntity(E_UNK_2C, self, entity);
            i++;
            entity->params = i;
            entity->nextPart = entity - 1;
            entity++;
        }
        prev = self + 24;
        self->parent = NULL;
        self->nextPart = prev;
        CreateEntityFromEntity(E_UNK_2E, self, entity);
        self->ext.orobourous.bounceCount = 2;
        break;

    case 1:
        if (UnkCollisionFunc3(D_us_801827D4) & 1) {
            SetStep(2);
        }
        break;

    case 2:
        SetStep(3);
        break;

    case 3: // Hop around
        MoveEntity();
        self->velocityY += self->ext.orobourous.gravityAccel;
        self->ext.orobourous.gravityAccel += 0x100;
        if (self->velocityY < 0) {
            self->animCurFrame = 1;
        } else {
            self->animCurFrame = 0xE;
        }
        if ((self->ext.orobourous.facingCache != self->facingLeft) &&
            (AnimateEntity(D_us_801827CC, self) == 0)) {
            self->animCurFrame = 0xE;
            self->facingLeft = self->ext.orobourous.facingCache;
        }
        if (self->rotate != 0) {
            self->rotate -= 0x10;
        }
        g_api.CheckCollision(
            self->posX.i.hi, self->posY.i.hi + 0xC, &collider, 0);
        if (collider.effects & EFFECT_SOLID) {
            PlaySfxPositional(0x63D);
            dx = (u16)self->posY.i.hi;
            dy = (u16)collider.unk18;
            angle = self->ext.orobourous.facingCache;
            self->velocityY = FIX(-3);
            self->ext.orobourous.gravityAccel = 0;
            self->posY.i.hi = dx + dy;
            if (angle == (GetSideToPlayer() & 1)) {
                self->ext.orobourous.bounceCount--;
            } else {
                self->ext.orobourous.bounceCount = 2;
            }
            x = g_Tilemap.scrollX.i.hi + self->posX.i.hi;
            if (self->ext.orobourous.facingCache) {
                j = ((g_Tilemap.hSize << 8) - 0x80) < x;
            } else {
                j = (s32)x < 0x80;
            }
            if (j) {
                self->ext.orobourous.bounceCount = 0;
            }
            if (self->ext.orobourous.bounceCount == 0) {
                self->ext.orobourous.bounceCount = 2;
                self->poseTimer = 0;
                self->pose = 0;
                self->ext.orobourous.facingCache ^= 1;
            } else {
                self->rotate = 0x200;
            }
            if (self->ext.orobourous.facingCache) {
                self->velocityX = FIX(1.5);
                EntityGreyPuffSpawner(self, 5, 3, 4, 0xC, 0, -4);
            } else {
                self->velocityX = FIX(-1.5);
                EntityGreyPuffSpawner(self, 5, 3, -4, 0xC, 0, 4);
            }
        }
        if (self->ext.orobourous.gathered) {
            self->animCurFrame = 0xE;
            SetStep(4);
        }
        break;

    case 4: // Rider died: gather the segments
        entity = self + 1;
        for (i = 23; i >= 0; i--, entity++) {
            entity->ext.orobourous.gathered = 1;
        }
        self->ext.orobourous.restTime = 0x800;
        self->drawFlags = 0;
        self->ext.orobourous.bounceCount = 0;
        self->hitboxState = 0;
        self->ext.orobourous.stepTimer = 0x10;
        entity = AllocEntity(&g_Entities[32], &g_Entities[47]);
        if (entity != NULL) {
            DestroyEntity(entity);
            entity->entityId = 0x43; // reused for the roaming hitbox helper
            entity->step = 1;
            entity->pfnUpdate = func_us_801C4334;
            entity->ext.prim = (Primitive*)self;
        } else {
            self->ext.orobourous.restTime = -1;
        }
        SetStep(5);
        // fallthrough
    case 5: // Brighten the segments one by one
        if (!--self->ext.orobourous.stepTimer) {
            self->ext.orobourous.stepTimer = 4;
            entity = self + self->ext.orobourous.bounceCount;
            entity->palette += 2;
            if (self->ext.orobourous.bounceCount++ >= 0x19) {
                SetStep(6);
            }
        }
        // fallthrough
    case 6: // Fly after the player
        FntPrint("rest_time:%x\n", self->ext.orobourous.restTime);
        if (self->step_s == 0) {
            self->ext.orobourous.chaseTimer = 0x40;
            angle = Random() * 8;
            dx = rcos(angle) * 0x60 >> 0xC;
            dy = -(rsin(angle) * 0x60) >> 0xC;
            if (self->ext.orobourous.restTime == 0) {
                dx = 0;
                dy = 0;
                self->ext.orobourous.restTime--;
                self->ext.orobourous.chaseTimer = 0x100;
            }
            if (self->ext.orobourous.restTime < 0) {
                dx = 0x200;
                dy = -0x300;
                self->ext.orobourous.chaseTimer = 0x400;
            }
            self->ext.orobourous.targetX = PLAYER.posX.i.hi + dx;
            self->step_s++;
            self->ext.orobourous.targetY = PLAYER.posY.i.hi + dy;
        }
        if ((self->ext.orobourous.facingCache != self->facingLeft) &&
            (AnimateEntity(D_us_801827CC, self) == 0)) {
            self->animCurFrame = 0xE;
            self->pose = 0;
            self->poseTimer = 0;
            self->facingLeft = self->ext.orobourous.facingCache;
        }
        MoveEntity();
        if (self->velocityX > 0) {
            self->ext.orobourous.facingCache = 1;
        } else {
            self->ext.orobourous.facingCache = 0;
        }
        dx = self->ext.orobourous.targetX - self->posX.i.hi;
        dy = self->ext.orobourous.targetY - self->posY.i.hi;
        angle = GetNormalizedAngle(
            0x20, self->ext.orobourous.angle, ratan2(dy, dx));
        self->velocityX = rcos(angle) * 0x24;
        self->velocityY = rsin(angle) * 0x24;
        self->ext.orobourous.angle = angle;
        if (!--self->ext.orobourous.chaseTimer ||
            (abs(dx) < 8 && abs(dy) < 8)) {
            self->step_s = 0;
        }
        if (self->ext.orobourous.restTime > 0) {
            self->ext.orobourous.restTime--;
        } else if (self->posY.i.hi < -0x200) {
            entity = self + 1;
            for (i = 0; i < 24; i++, entity++) {
                DestroyEntity(entity);
            }
            PreventEntityFromRespawning(self);
            DestroyEntity(self);
            return;
        }
        break;

    case 8: // Death
        PlaySfxPositional(0x629);
        entity = self + 1;
        for (i = 0; i < 25; i++, entity++) {
            entity->flags |= FLAG_DEAD;
        }
        entity = AllocEntity(&g_Entities[224], &g_Entities[256]);
        if (entity != NULL) {
            CreateEntityFromEntity(E_EXPLOSION, self, entity);
            entity->params = 3;
        }
        for (j = 0; j < 4; j++) {
            if (j != 1) {
                entity = AllocEntity(&g_Entities[224], &g_Entities[256]);
                if (entity != NULL) {
                    CreateEntityFromEntity(E_UNK_2D, self, entity);
                    entity->facingLeft = self->facingLeft;
                    entity->velocityY = FIX(-1.5);
                    entity->params = j;
                    entity->velocityX = self->velocityX;
                }
            }
        }
        DestroyEntity(self);
        return;
    }

    // Update the hitbox from the current animation frame
    i = self->animCurFrame - 0xE;
    if (i < 0) {
        i = 0;
    }
    data = D_us_80182814 + i * 4;
    self->hitboxOffX = data[0];
    self->hitboxOffY = data[1];
    self->hitboxWidth = data[2];
    self->hitboxHeight = data[3];
}

// E_UNK_2C: first serpent body segment
void func_us_801C3DE0(Entity* self) {
    Entity* prev;
    s32 dx;
    s32 dy;
    s32 dist;
    s16 angle;
    s16 scale;
    s32 params;
    s32 opacity;
    s32 flags;

    if ((self->flags & FLAG_DEAD) && (self->step < 8)) {
        self->hitboxState = 0;
        self->flags |= FLAG_DESTROY_IF_OUT_OF_CAMERA |
                       FLAG_DESTROY_IF_BARELY_OUT_OF_CAMERA;
        SetStep(8);
    }

    switch (self->step) {
    case 0:
        InitializeEntity(g_EInitOruburos);
        self->pose = self->params % 6;
        opacity = -0x80 - (params = self->params);
        self->hitboxOffY = 1;
        flags = self->drawFlags;
        params -= 0xA;
        self->opacity = opacity;
        self->drawFlags = flags | ENTITY_OPACITY;
        if (params > 0) {
            scale = 0x100 - params * 6;
            self->drawFlags = ENTITY_SCALEX | ENTITY_SCALEY;
            self->scaleY = scale;
            self->scaleX = scale;
        }
        if (self->params == 4) {
            self->animCurFrame = 0x11;
        }
        if (self->params == 0xC) {
            self->animCurFrame = 0x12;
        }
        break;

    case 1: // Chase the previous part
        MoveEntity();
        prev = self - 1;
        dx = prev->posX.i.hi - self->posX.i.hi;
        dy = prev->posY.i.hi - self->posY.i.hi;
        dist = SquareRoot0(dx * dx + dy * dy) * 3;
        angle = ratan2(dy, dx);
        self->velocityX = params = dist * rcos(angle);
        self->velocityY = dist * rsin(angle);
        if (params > 0) {
            self->facingLeft = 1;
        } else {
            self->facingLeft = 0;
        }
        break;

    case 8:
        MoveEntity();
        self->velocityY += 0x1400;
        return;
    }

    if ((self->params != 4) && (self->params != 0xC)) {
        AnimateEntity(D_us_801827BC, self);
    }
    params = self->params & 3;
    if ((g_Timer & 3) == params) {
        self->hitboxState = 3;
    } else {
        self->hitboxState = 0;
    }
    if (self->ext.orobourous.gathered) {
        self->hitboxState = 0;
    }
}

// E_UNK_2D: serpent body piece
void func_us_801C406C(Entity* self) {
    s32* data;

    if (self->step == 0) {
        InitializeEntity(D_us_801809C8);
        self->drawFlags = ENTITY_ROTATE;
        self->animCurFrame = self->params + 8;
        self->hitboxState = 0;
        self->zPriority += self->params;
        data = D_us_801827E4 + self->params * 3;
        if (self->facingLeft) {
            self->velocityX += data[0];
        } else {
            self->velocityX -= data[0];
        }
        self->velocityY += data[1];
    }
    MoveEntity();
    self->velocityY += 0x1800;
    self->rotate += D_us_801827EC[self->params * 3];
}

// E_UNK_2E: first serpent rider
void func_us_801C4178(Entity* self) {
    Entity* head;
    Entity* entity;

    if (self->flags & FLAG_DEAD) {
        PlaySfxPositional(SFX_EXPLODE_B);
        entity = AllocEntity(&g_Entities[224], &g_Entities[256]);
        if (entity != NULL) {
            CreateEntityFromEntity(E_EXPLOSION, self, entity);
            entity->params = 1;
        }
        entity = AllocEntity(&g_Entities[224], &g_Entities[256]);
        if (entity != NULL) {
            CreateEntityFromEntity(E_UNK_2D, self, entity);
            entity->params = 1;
            entity->facingLeft = self->facingLeft;
        }
        head = self - 25;
        head->ext.orobourous.gathered = 1;
        DestroyEntity(self);
        return;
    }

    switch (self->step) {
    case 0:
        InitializeEntity(g_EInitOruburosRider);
        self->animCurFrame = 0xD;
        self->hitboxOffX = 5;
        self->hitboxOffY = -2;
        self->drawFlags = ENTITY_ROTATE;
        break;

    case 1: // Track the head
        head = self - 25;
        self->facingLeft = head->facingLeft;
        self->posX.i.hi = head->posX.i.hi;
        self->posY.i.hi = head->posY.i.hi - 0xA;
        if (head->animCurFrame == 0xE) {
            self->zPriority = head->zPriority + 1;
        } else {
            self->zPriority = head->zPriority - 1;
        }
        if (head->velocityY > 0) {
            self->animCurFrame = 0xD;
            if (self->rotate >= -0x23F) {
                self->rotate -= 0x18;
            }
        } else {
            self->animCurFrame = 0;
            self->rotate = 0;
        }
        break;
    }
}

// Roaming damage hitbox that cycles along the gathered serpent
void func_us_801C4334(Entity* self) {
    Entity* seg;
    s32 segIndex;

    switch (self->step) {
    case 0:
    case 1:
        self->hitboxState = 2;
        self->attack = 0x30;
        self->attackElement = 0x1000;
        self->hitboxWidth = 8;
        self->hitboxHeight = 8;
        self->nFramesInvincibility = 0x10;
        self->stunFrames = 4;
        self->hitEffect = 1;
        self->ext.orobourous.unkB2 = 0;
        self->flags = 0x0C000000;
        g_api.func_80118894(self);
        self->step++;
        break;

    case 2:
        seg = (Entity*)self->ext.prim;
        segIndex = self->ext.orobourous.bounceCount;
        self->ext.orobourous.bounceCount += 2;
        seg += segIndex;
        if (self->ext.orobourous.bounceCount >= 0x19) {
            self->ext.orobourous.bounceCount = 0;
        }
        self->posX.i.hi = seg->posX.i.hi;
        self->posY.i.hi = seg->posY.i.hi;
        if ((u32)(seg->entityId - E_UNK_2B) >= 2) {
            DestroyEntity(self);
        }
        break;
    }
}

// E_UNK_30: second serpent head
void func_us_801C4468(Entity* self) {
    Collider collider;
    Entity* entity;
    Entity* prev;
    s32 i;
    s32 j;
    u16* data;
    u32 x;

    if (self->flags & FLAG_DEAD) {
        SetStep(4);
    }

    switch (self->step) {
    case 0:
        InitializeEntity(g_EInitDragonRider1);
        self->animCurFrame = 1;
        self->ext.orobourous.bounceCount = 2;
        self->drawFlags |= ENTITY_ROTATE;
        break;

    case 1:
        if (UnkCollisionFunc3(D_us_80182844) & 1) {
            entity = self + 1;
            for (i = 0; i < 24;) {
                CreateEntityFromEntity(E_UNK_31, self, entity);
                i = j = i + 1;
                prev = entity - 1;
                entity->params = i;
                entity->nextPart = prev;
                entity->posY.i.hi += 8;
                entity++;
            }
            CreateEntityFromEntity(E_UNK_33, self, entity);
            entity->nextPart = entity - 1;
            entity->parent = self;
            self->parent = NULL;
            self->nextPart = entity;
            SetStep(3);
        }
        break;

    case 3: // Hop around
        MoveEntity();
        self->velocityY += self->ext.orobourous.gravityAccel;
        self->ext.orobourous.gravityAccel += 0x100;
        if ((self->ext.orobourous.facingCache != self->facingLeft) &&
            (AnimateEntity(D_us_8018283C, self) == 0)) {
            self->animCurFrame = 1;
            self->facingLeft = self->ext.orobourous.facingCache;
        }
        if (self->rotate != 0) {
            self->rotate -= 0x10;
        }
        g_api.CheckCollision(
            self->posX.i.hi, self->posY.i.hi + 0xC, &collider, 0);
        if (collider.effects & EFFECT_SOLID) {
            PlaySfxPositional(0x659);
            self->velocityY = FIX(-2.75);
            self->ext.orobourous.gravityAccel = 0;
            self->posY.i.hi += collider.unk18;
            if (self->ext.orobourous.facingCache ==
                (GetSideToPlayer() & 1)) {
                self->ext.orobourous.bounceCount--;
            } else {
                self->ext.orobourous.bounceCount = 2;
            }
            x = g_Tilemap.scrollX.i.hi + self->posX.i.hi;
            if (self->ext.orobourous.facingCache) {
                j = ((g_Tilemap.hSize << 8) - 0x80) < x;
            } else {
                j = (s32)x < 0x80;
            }
            if (j) {
                self->ext.orobourous.bounceCount = 0;
            }
            if (self->ext.orobourous.bounceCount == 0) {
                self->ext.orobourous.bounceCount = 2;
                self->poseTimer = 0;
                self->pose = 0;
                self->ext.orobourous.facingCache ^= 1;
            } else {
                self->rotate = 0x200;
            }
            if (self->ext.orobourous.facingCache) {
                self->velocityX = FIX(1.5);
                EntityGreyPuffSpawner(self, 5, 3, 4, 0xC, 0, -4);
            } else {
                self->velocityX = FIX(-1.5);
                EntityGreyPuffSpawner(self, 5, 3, -4, 0xC, 0, 4);
            }
        }
        break;

    case 4: // Death
        PlaySfxPositional(0x629);
        entity = self + 1;
        for (i = 0; i < 24; i++, entity++) {
            entity->flags |= FLAG_DEAD;
        }
        entity = AllocEntity(&g_Entities[224], &g_Entities[256]);
        if (entity != NULL) {
            CreateEntityFromEntity(E_EXPLOSION, self, entity);
            entity->params = 3;
        }
        for (i = 0; i < 4; i++) {
            entity = AllocEntity(&g_Entities[224], &g_Entities[256]);
            if (entity != NULL) {
                CreateEntityFromEntity(E_UNK_2D, self, entity);
                entity->facingLeft = self->facingLeft;
                x = self->velocityX;
                entity->velocityY = FIX(-1.5);
                entity->params = i;
                entity->velocityX = x;
            }
        }
        DestroyEntity(self);
        return;
    }

    // Update the hitbox from the current animation frame
    data = D_us_80182884;
    j = self->animCurFrame - 1;
    if (j < 0) {
        j = 0;
    }
    data += j * 4;
    self->hitboxOffX = *data++;
    self->hitboxOffY = *data++;
    self->hitboxWidth = *data;
    self->hitboxHeight = data[1];
}

// E_UNK_31: second serpent body segment
void func_us_801C48D8(Entity* self) {
    Collider collider;
    Entity* prev;
    s16 scale;
    s32 velocityX;
    s32 params;

    if ((self->flags & FLAG_DEAD) && (self->step < 8)) {
        self->hitboxState = 0;
        self->flags |= FLAG_DESTROY_IF_OUT_OF_CAMERA |
                       FLAG_DESTROY_IF_BARELY_OUT_OF_CAMERA;
        SetStep(8);
    }

    switch (self->step) {
    case 0:
        InitializeEntity(g_EInitDragonRider2);
        params = self->params;
        velocityX = -0x80 - params;
        self->opacity = velocityX;
        self->drawFlags |= ENTITY_OPACITY;
        self->pose = self->params % 6;
        params -= 0xA;
        if (params > 0) {
            scale = 0x100 - params * 6;
            self->drawFlags = ENTITY_SCALEX | ENTITY_SCALEY;
            self->scaleY = scale;
            self->scaleX = scale;
        }
        self->ext.orobourous.stepTimer = self->params * 5;
        // fallthrough
    case 1: // Wait before starting to hop
        if (params = (s16)self->ext.orobourous.stepTimer) {
            self->ext.orobourous.stepTimer = params - 1;
        } else {
            self->step = 2;
        }
        break;

    case 2: // Hop, following the leading part's facing
        MoveEntity();
        self->velocityY += self->ext.orobourous.gravityAccel;
        self->ext.orobourous.gravityAccel += 0x100;
        g_api.CheckCollision(
            self->posX.i.hi, self->posY.i.hi + 6, &collider, 0);
        if (collider.effects & EFFECT_SOLID) {
            self->velocityY = FIX(-2.75);
            velocityX = collider.unk18;
            self->ext.orobourous.gravityAccel = 0;
            self->posY.i.hi += velocityX;
            prev = self - 1;
            if (prev->ext.orobourous.facingCache !=
                self->ext.orobourous.facingCache) {
                self->ext.orobourous.facingCache =
                    prev->ext.orobourous.facingCache;
            }
            if (self->ext.orobourous.facingCache == 0) {
                velocityX = FIX(-2);
            } else {
                velocityX = FIX(1);
            }
            self->velocityX = velocityX | 0x8000;
        }
        break;

    case 8:
        MoveEntity();
        self->velocityY += 0x1400;
        return;
    }

    AnimateEntity(D_us_8018282C, self);
    params = self->params & 3;
    if ((g_Timer & 3) == params) {
        self->hitboxState = 3;
    } else {
        self->hitboxState = 0;
    }
}

// E_UNK_32: second serpent body piece
void func_us_801C4B44(Entity* self) {
    s32* data;

    if (self->step == 0) {
        InitializeEntity(D_us_801809F8);
        self->drawFlags = ENTITY_ROTATE;
        self->animCurFrame = self->params + 8;
        self->hitboxState = 0;
        self->zPriority += self->params;
        data = D_us_80182854 + self->params * 3;
        if (self->facingLeft) {
            self->velocityX += data[0];
        } else {
            self->velocityX -= data[0];
        }
        self->velocityY += data[1];
    }
    MoveEntity();
    self->velocityY += 0x1800;
    self->rotate += D_us_8018285C[self->params * 3];
}

// E_UNK_33: second serpent rider
void func_us_801C4C50(Entity* self) {
    Entity* head;
    s32 i;
    u16* data;

    if (self->step == 0) {
        InitializeEntity(g_EInitDragonRider1);
        self->animCurFrame = 0;
    }
    head = self - 25;
    data = D_us_8018289C;
    self->posX.i.hi = head->posX.i.hi;
    self->posY.i.hi = head->posY.i.hi;
    i = head->animCurFrame - 1;
    self->facingLeft = head->facingLeft;
    if (i < 0) {
        i = 0;
    }
    data += i * 4;
    self->hitboxOffX = *data++;
    self->hitboxOffY = *data++;
    self->hitboxWidth = *data;
    self->hitboxHeight = data[1];
    if (head->entityId != E_UNK_30) {
        DestroyEntity(self);
    }
}
