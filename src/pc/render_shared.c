// SPDX-License-Identifier: AGPL-3.0-or-later
#include <common.h>
#include <log.h>
#include <game.h>
#include <stdlib.h>
#ifdef _MSC_VER
#define SDL_MAIN_HANDLED
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif
#include "pc.h"
#include <assert.h>
#include <string.h>
#include "sdl_defs.h"

enum Renderers render_mode = RENDER_SOFT;

int SoftDrawSync(int mode);
DISPENV* SoftPutDispEnv(DISPENV* env);
void SoftSetDrawEnv(DR_ENV* dr_env, DRAWENV* env);
DRAWENV* SoftPutDrawEnv(DRAWENV* env);
void SoftDrawOTag(OT_TYPE* p);

int GlDrawSync(int mode);
DISPENV* GlPutDispEnv(DISPENV* env);
void GlSetDrawEnv(DR_ENV* dr_env, DRAWENV* env);
DRAWENV* GlPutDrawEnv(DRAWENV* env);
void GlDrawOTag(OT_TYPE* p);

// SOTN's simulation advances exactly one frame per displayed frame, so on a
// high-refresh monitor (e.g. 120 Hz) the whole game runs too fast (2x at
// 120 Hz). Cap the frame cadence to the game's native NTSC 60 Hz here, the one
// place every frame passes through. Vsync stays on for tear-free output; this
// just holds each frame until the next 1/60 s boundary.
#define SOTN_FPS 120
static void LimitFrameRate(void) {
    static Uint64 nextFrame = 0;
    const Uint64 freq = SDL_GetPerformanceFrequency();
    const Uint64 period = freq / SOTN_FPS;
    Uint64 now = SDL_GetPerformanceCounter();

    // First frame, or recover from a long stall (loading, breakpoint) without
    // trying to "catch up" a burst of frames.
    if (nextFrame == 0 || now > nextFrame + period * 4) {
        nextFrame = now;
    }
    nextFrame += period;

    if (now < nextFrame) {
        Uint64 remaining = nextFrame - now;
        Uint32 ms = (Uint32)(remaining * 1000 / freq);
        if (ms > 1) {
            SDL_Delay(ms - 1); // sleep the bulk, keep ~1 ms to spin
        }
        while (SDL_GetPerformanceCounter() < nextFrame) {
            // spin for sub-millisecond accuracy
        }
    }
}

int MyDrawSync(int mode) {
    int ret;
    if (render_mode == RENDER_SOFT) {
        ret = SoftDrawSync(mode);
    } else {
        ret = GlDrawSync(mode);
    }
    LimitFrameRate();
    return ret;
}

DISPENV* MyPutDispEnv(DISPENV* env) {
    if (render_mode == RENDER_SOFT) {
        return SoftPutDispEnv(env);
    } else {
        return GlPutDispEnv(env);
    }
    return env;
}

void MySetDrawEnv(DR_ENV* dr_env, DRAWENV* env) {
    if (render_mode == RENDER_SOFT) {
        SoftSetDrawEnv(dr_env, env);
    } else {
        GlSetDrawEnv(dr_env, env);
    }
}

DRAWENV* MyPutDrawEnv(DRAWENV* env) {
    if (render_mode == RENDER_SOFT) {
        return SoftPutDrawEnv(env);
    } else {
        return GlPutDrawEnv(env);
    }

    return env;
}

void MyDrawOTag(OT_TYPE* p) {
    if (render_mode == RENDER_SOFT) {
        SoftDrawOTag(p);
    } else {
        GlDrawOTag(p);
    }
}

extern u8* GetFb();
extern u16 g_RawVram[VRAM_W * VRAM_H];

int MyLoadImage(RECT* rect, u_long* p) {
    {
        u16* mem = (u16*)p;
        u16* vram = g_RawVram;
        vram += rect->x + rect->y * VRAM_W;

        for (int i = 0; i < rect->h; i++) {
            memcpy(vram, mem, rect->w * 2);
            mem += rect->w;
            vram += VRAM_W;
        }
    }

    // copy to software vram
    {
        u16* mem = (u16*)p;
        u16* vram = GetFb();
        vram += rect->x + rect->y * VRAM_W;

        for (int i = 0; i < rect->h; i++) {
            memcpy(vram, mem, rect->w * 2);
            mem += rect->w;
            vram += VRAM_W;
        }
    }
    return 0;
}
