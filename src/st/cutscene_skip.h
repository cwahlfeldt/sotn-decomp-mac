// SPDX-License-Identifier: AGPL-3.0-or-later
extern Dialogue g_Dialogue;
extern s32 g_SkipCutscene;

// Static everywhere except pspeu so the PC build (which links all stages into
// one binary) doesn't get duplicate definitions from each cutscene stage.
#ifndef VERSION_PSP
static
#endif
    void
    CutsceneSkip(Entity* self) {
    if (g_pads[0].tapped == PAD_START) {
        g_SkipCutscene = true;
        g_api.FreePrimitives(self->primIndex);
        self->flags ^= FLAG_HAS_PRIMS;
        if (g_Dialogue.primIndex[1] != -1) {
            g_api.FreePrimitives(g_Dialogue.primIndex[1]);
        }
        if (g_Dialogue.primIndex[0] != -1) {
            g_api.FreePrimitives(g_Dialogue.primIndex[0]);
        }
        g_api.PlaySfx(SET_STOP_MUSIC);
        self->step = 1;
        self->step_s = 0;
    }
}
