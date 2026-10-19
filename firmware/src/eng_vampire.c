/* SPDX-License-Identifier: GPL-3.0-only
 * Original hybrid Vampire engine: generated PCM textures + tonal oscillator,
 * drift, two-pole filter, soft drive and air. Four simultaneous notes per part.
 * Its sound then enters the OS envelope, matrix, distortion/chorus/delay/reverb.
 * CPU-bound loops only, no floating-point arithmetic in the audio ISR. */
#include "vampire_pcm.h"
#include "vampire_presets.h"
static const char *const VAMP_MODE[] = {"CHOIR", "CRY", "RAIN", "STORM", "MIST", "BELL", "STRNG", "ORGAN"};
/* Kept in one place for the device UI and native audio renderer. */
static void vampire_preset_controls(track_t *t, uint32_t pi)
{
    pi &= 63u;
    t->p[P_VOICE] = V_POLY;
    t->p[P_LRATE] = VAMPIRE_RATE[pi];
    t->p[P_LD_FLT] = VAMPIRE_DEPTH[pi];
    t->p[P_M1SRC] = 1; t->p[P_M1DST] = 5; t->p[P_M1AMT] = VAMPIRE_MATRIX[pi]; /* LFO -> PAN */
    t->p[P_M2SRC] = 2; t->p[P_M2DST] = 2; t->p[P_M2AMT] = 8 + pi % 8u;   /* ENV -> CUT */
    t->p[P_M3SRC] = 3; t->p[P_M3DST] = 4; t->p[P_M3AMT] = 16;             /* VEL -> AMP */
    t->p[P_M4SRC] = 5; t->p[P_M4DST] = 1; t->p[P_M4AMT] = 1;              /* RAND -> PITCH */
}
static void vamp_note_on(track_t *t, voice_t *v)
{
    uint32_t seed = t->lfo_rnd ^ ((uint32_t)v->note + v->age * 17u + 1u) * 2654435761u;
    if (!seed) seed = 1u;
    v->s[0] = (int32_t)(((seed & (VAMP_PCM_N - 1u)) * (uint32_t)clamp(t->p[P_E6], 0, 127) / 127u) << 16);
    v->s[1] = v->s[2] = 0;
    v->s[3] = (int32_t)seed;
    v->ph[1] = seed;
}
static void vamp_render(track_t *t, voice_t *v, int32_t *out, uint32_t n, const vmod_t *m)
{
    const int16_t *p = t->p;
    uint32_t mode = (uint32_t)clamp(p[P_E0], 0, 7), source = mode < 4u ? mode : 2u;
    uint32_t i, pos = (uint32_t)v->s[0], inc = m->inc;
    uint32_t step = inc / 1555u + 1u;             /* source at C4 is ~1/4 speed: 11.025 -> 44.1 kHz */
    int32_t blend = clamp(p[P_E3], 0, 127), bite = clamp(p[P_E2] + (m->shape >> 8), 0, 127);
    int32_t tone = clamp(p[P_E1] + (m->cutoff >> 8), 2, 127);
    int32_t coef = 384 + tone * tone, lp = v->s[1], lp2 = v->s[2];
    int32_t air = clamp(p[P_E5], 0, 127), detune = clamp(p[P_E4], 0, 127);
    int32_t interval = clamp(p[P_E7], -12, 12);
    uint32_t oscinc = pitch_inc(clamp(m->pitch16 + interval * 16, 0, 2047));
    uint32_t other = oscinc + (uint32_t)((oscinc >> 12) * (uint32_t)detune);
    for (i = 0; i < n; i++) {
        uint32_t index = pos >> 16, next = (index + 1u) & (VAMP_PCM_N - 1u);
        int32_t a = VAMP_PCM[source][index], b = VAMP_PCM[source][next];
        int32_t sample = a + (((b - a) * (int32_t)((pos & 65535u) >> 1)) >> 15);
        int32_t osc = (osc_sine(v->ph[0]) + osc_sine(v->ph[1])) / 2;
        if (mode == 5u) {                        /* inharmonic toll, frequency modulation with bounded depth */
            osc = osc_sine(v->ph[0] + (uint32_t)(osc_sine(v->ph[1]) * (4096 + bite * 256)));
        } else if (mode == 6u) {
            osc = (osc_tri(v->ph[0]) + osc_tri(v->ph[1])) / 2;
        } else if (mode == 7u) {
            osc = (osc_sine(v->ph[0]) + osc_sine(v->ph[0] * 2u) / 2 + osc_sine(v->ph[0] * 3u) / 3) / 2;
        }
        int32_t y = (sample * (127 - blend) + osc * blend) / 127;
        int32_t hiss = ((int32_t)(noise32(&v->s[3]) >> 17) - 16384);
        y += hiss * air / 508;
        y = softclip(y + y * bite / 64);
        lp += mulq15(y - lp, coef);
        lp2 += mulq15(lp - lp2, coef);
        out[i] += voice_amp(lp2, m, i);
        v->ph[0] += oscinc; v->ph[1] += mode == 5u ? oscinc * 2u + oscinc / 5u : other;
        pos = (pos + step) & ((VAMP_PCM_N << 16) - 1u);
    }
    v->s[0] = (int32_t)pos; v->s[1] = lp; v->s[2] = lp2;
}
static const engine_t ENG_VAMPIRE = {
    .name = "VAMPIRE", .page_title = {"SOURCE", "SHADOW"},
    .edit = {
        {"MODE", F_ENUM, 0, 7, 0, VAMP_MODE, 0},
        {"COLOR", F_PCT, 0, 127, 64, 0, 0},  /* colour intensity; no inaccurate Hz readout */
        {"BITE", F_PCT, 0, 127, 24, 0, 0},
        {"BLEND", F_PCT, 0, 127, 64, 0, 0},
        {"DRIFT", F_PCT, 0, 127, 16, 0, 0},
        {"AIR", F_PCT, 0, 127, 16, 0, 0},
        {"SPREAD", F_PCT, 0, 127, 40, 0, 0},
        {"OCT", F_SEMI, -12, 12, 0, 0, 0},
    },
    .presets = VAMPIRE_PRESETS, .npresets = NELEM(VAMPIRE_PRESETS),
    .knob = {P_E1, P_E2, P_E3, P_REL}, .poly = 4,
    .note_on = vamp_note_on, .render = vamp_render,
};
