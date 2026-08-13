// SPDX-License-Identifier: AGPL-3.0-or-later
//! JTBL_ALIGN=4
#include "rno3.h"

#include "../water_effects.h"

extern EInit g_EInitDodoBird;

// Dodo-bird collision sensors and animation frames; still in the raw data
// blob at RNO3.BIN+0x2AF0.
extern s16 D_us_80182AF0[];
extern s16 D_us_80182AF8[];
extern u8 D_us_80182B08[];
extern u8 D_us_80182B18[];

// EntityDodoBird
void func_us_801C8B8C(Entity* self) {
    Entity* newEntity;
    s32 collision;
    s32 i;

    if ((self->flags & FLAG_DEAD) && self->step < 5) {
        SetStep(5);
    }
    switch (self->step) {
    case 0:
        InitializeEntity(g_EInitDodoBird);
        self->facingLeft = GetSideToPlayer() & 1;
        if (self->params & 0x100) {
            self->animCurFrame = 9;
            self->step = 4;
            self->drawFlags |= ENTITY_ROTATE;
        }
        break;
    case 1:
        if (UnkCollisionFunc3(D_us_80182AF8) & 1) {
            self->step++;
        }
        break;
    case 2:
        AnimateEntity(D_us_80182B08, self);
        if (!(g_Timer & 7)) {
            PlaySfxPositional(SFX_QUIET_STEPS);
        }
        collision = UnkCollisionFunc2(D_us_80182AF0);
        if (collision == 0) {
            self->velocityY += FIX(0.25);
        } else {
            self->velocityY = 0;
        }
        if (self->facingLeft) {
            self->velocityX = FIX(-0.75);
        } else {
            self->velocityX = FIX(0.75);
        }
        if (self->ext.dodoBird.speedUp) {
            self->velocityX *= 2;
        }
        if (collision == 0x80) {
            self->velocityY = FIX(-4);
            SetStep(1);
        }
        if (GetDistanceToPlayerX() < 0x40 &&
            (GetSideToPlayer() & 1) == self->facingLeft) {
            self->ext.dodoBird.speedUp |= 1;
            SetStep(3);
            self->ext.dodoBird.timer = 0x20;
        }
        break;
    case 3:
        AnimateEntity(D_us_80182B18, self);
        if (!(s16)--self->ext.dodoBird.timer) {
            self->facingLeft = (GetSideToPlayer() & 1) ^ 1;
            SetStep(2);
        }
        break;
    case 4:
        MoveEntity();
        switch (self->step_s) {
        case 0:
            self->velocityY += FIX(0.125);
            if (self->velocityY > 0) {
                self->velocityY = 0;
                self->flags |=
                    FLAG_DESTROY_IF_OUT_OF_CAMERA |
                    FLAG_DESTROY_IF_BARELY_OUT_OF_CAMERA;
                self->step_s++;
            }
            break;
        case 1:
            self->velocityY += FIX(1.0 / 64);
            self->velocityX = rsin((s16)self->ext.dodoBird.timer) * 16;
            self->ext.dodoBird.timer += 0x20;
            if (self->velocityX > 0) {
                self->rotate -= 0x40;
            } else {
                self->rotate += 0x40;
            }
            break;
        }
        break;
    case 5:
        switch (self->step_s) {
        case 0:
            self->ext.dodoBird.timer = 0x10;
            self->step_s++;
            PlaySfxPositional(SFX_FLEA_RIDER_EXPLODE);
            self->drawFlags |= ENTITY_ROTATE;
            /* fallthrough */
        case 1:
            AnimateEntity(D_us_80182B18, self);
            self->rotate -= 0x80;
            if (!(self->ext.dodoBird.timer & 3)) {
                for (i = 0; i < 5; i++) {
                    newEntity =
                        AllocEntity(&g_Entities[224], &g_Entities[256]);
                    if (newEntity != NULL) {
                        CreateEntityFromEntity(E_UNK_4E, self, newEntity);
                        newEntity->params = 0x100;
                        newEntity->velocityX = (Random() & 7) << 13;
                        if (i) {
                            newEntity->velocityX = -newEntity->velocityX;
                        }
                        newEntity->velocityY = (Random() & 0x3F) * FIX(-1.0 / 16);
                        newEntity->posX.i.hi += (Random() & 0x1F) - 0x10;
                        newEntity->ext.dodoBird.timer = Random() * 16;
                    }
                }
            }
            if (!--self->ext.dodoBird.timer) {
                self->step_s++;
            }
            break;
        case 2:
            newEntity = AllocEntity(&g_Entities[224], &g_Entities[256]);
            if (newEntity != NULL) {
                CreateEntityFromEntity(E_EXPLOSION, self, newEntity);
                newEntity->params = 1;
            }
            DestroyEntity(self);
            break;
        }
        break;
    }
}
