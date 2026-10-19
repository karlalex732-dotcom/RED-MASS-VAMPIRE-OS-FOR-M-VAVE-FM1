/* SPDX-License-Identifier: GPL-3.0-only
 * RED_MASS sample drum kit: two original factory PCM kits or GM-mapped zones in USR1..3.
 * One-shot, velocity sensitive, independent of scale/octave and key release. */
static const char *const N_SMPKIT[] = {"NIGHT", "USR1", "USR2", "USR3", "RAGE"};
static void smpkit_note_on(track_t *t, voice_t *v)
{
    int32_t pad = groove_pad(v->note);
    uint32_t si = (uint32_t)clamp(t->p[P_E0], 0, 4), zi = 0xFFFFu, i;
    if (pad >= 0 && (!si || si==4u)) zi = 0x4000u | (uint32_t)pad | (si==4u ? 16u : 0u);
    else if (pad >= 0) {
        /* Match by note, not zone order: the uploader sorts zones by root. */
        for (i = 0; i < usr_nz[si-1u]; i++) {
            const smp_zone_t *z = &usr_zone[si-1u][i];
            if (v->note >= z->lo && v->note <= z->hi) { zi = 0x8000u | (si-1u)<<5 | i; break; }
        }
    }
    v->s[4] = zi == 0xFFFFu ? 0 : (int32_t)zi;
    v->ph[0] = v->ph[1] = 0;
    v->s[0] = v->s[1] = v->s[2] = v->s[3] = v->s[7] = 0;
    v->s[6] = zi == 0xFFFFu;
    if (pad == 3) { /* closed hat chokes the open hat on this track */
        for (i = 0; i < NVOICE; i++) if (&t->v[i] != v && t->v[i].note == GROOVE_NOTE[4]) {
            t->v[i].gate = 0; t->v[i].stage = 3;
            t->v[i].s[6] = 1;
        }
    }
}
static void smpkit_render(track_t *t, voice_t *v, int32_t *out, uint32_t n, const vmod_t *m)
{
    vmod_t km = *m;
    if (v->s[6]) { v->active = 0; return; }
    km.pitch16 = smp_zone((uint32_t)v->s[4])->root16 + m->pitch16 - (int32_t)v->note*16;
    sample_render(t, v, out, n, &km);
}
static int32_t smpkit_amp(track_t *t, voice_t *v, int32_t adsr)
{
    (void)t; (void)adsr;
    if (v->s[6]) { v->active = v->gate = 0; return 0; }
    /* Sample itself has a fade at either end. Keep the ADSR alive after key-up. */
    v->env = 1 << 24; v->stage = 2;
    return 32767;
}
static int32_t smpkit_keys(const track_t *t, uint32_t k)
{
    uint32_t i, black = (0x54Au >> ((k+5u)%12u)) & 1u, p = 0;
    (void)t;
    for (i = 0; i < k; i++) p += (((0x54Au >> ((i+5u)%12u))&1u) == black);
    return GROOVE_NOTE[p % NPAD];
}
static const preset_t SMPKIT_PRESETS[] = {
    {"NIGHT TRAP", {0,0,0,0,127,0,12,0}, {0,100,127,100},0,0,FX(0,0,0,3),PAT(12)},
    {"USER KIT 1", {1,0,0,0,127,0,0,0}, {0,100,127,100},0,0,FX(0,0,0,10),PAT(12)},
    {"USER KIT 2", {2,0,0,0,127,0,0,0}, {0,100,127,100},0,0,FX(0,0,0,10),PAT(12)},
    {"USER KIT 3", {3,0,0,0,127,0,0,0}, {0,100,127,100},0,0,FX(0,0,0,10),PAT(12)},
    {"RAGE TRAP", {4,0,0,0,127,0,0,0}, {0,100,127,100},0,0,FX(0,0,0,1),PAT(12)},
};
static const engine_t ENG_SMPKIT = {
    .name = "SMPKIT", .page_title = {"KIT", "TONE"},
    .edit = {
        {"KIT",F_ENUM,0,4,0,N_SMPKIT,0}, {"TUNE",F_SEMI,-24,24,0,0,0},
        {"BITS",F_INT,0,127,0,0,0}, {"-",F_INT,0,0,0,0,0},
        {"CUT",F_INT,0,127,127,0,0}, {"-",F_INT,0,0,0,0,0},
        {"DRV",F_PCT,0,127,0,0,0}, {"-",F_INT,0,0,0,0,0},
    },
    .presets = SMPKIT_PRESETS, .npresets = NELEM(SMPKIT_PRESETS),
    .knob = {P_E0,P_E1,P_E4,P_E6}, .sampled = 1, .oneshot = 1,
    .note_on = smpkit_note_on, .render = smpkit_render, .amp = smpkit_amp, .keys = smpkit_keys,
};
