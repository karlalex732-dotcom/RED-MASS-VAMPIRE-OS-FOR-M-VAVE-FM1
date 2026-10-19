/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 Leo Kuroshita (@kurogedelic), Hügelton Instruments */
/* A song uses the sequences of four saved projects, with the current sounds.
 * Sources are copied in the main loop before PLAY. The ISR changes only their
 * index and four timing parameters; the editable steps stay untouched. */
typedef union {uint32_t align;uint8_t raw[5632];} project_store_t;
project_store_t proj_slot[4] __attribute__((section(".noinit")));
typedef struct {
    uint8_t micro[NTRK][NSTEP];
    int16_t timing[NTRK][4];
    motion_store_t motion;
} chain_pattern_t;
#include "song_sounds.c"
static chain_config_t chain_config;
static struct {
    chain_pattern_t source[4];
    step_t step[NTRK][NSTEP];
    chain_config_t config;
    int16_t timing[NTRK][4];
    volatile uint8_t armed, running, row, remaining;
    uint8_t slot, rec, elapsed;
    uint32_t carry;
} chain;

static void seq_release(track_t *t);
static void seq_stop(void);
static uint32_t div_samples(uint32_t div);
static uint32_t step_samples(const track_t *t, uint32_t period, uint32_t idx);

static void chain_defaults(chain_config_t *c)
{
    uint32_t i;
    memset(c, 0, sizeof *c);
    for (i = 0; i < CHAIN_ROWS; i++)
        c->row[i].repeat = 1;
}
static int chain_valid(const chain_config_t *c)
{
    uint32_t i;
    if (c->count > CHAIN_ROWS || (c->rsv[0] & ~3u) || c->rsv[1] >= CHAIN_ROWS || c->rsv[2] >= CHAIN_ROWS)
        return 0;
    for (i = 0; i < c->count; i++)
        if (c->row[i].slot >= 4u || !c->row[i].repeat || c->row[i].repeat > 64u || c->row[i].mute >= (1u << NTRK) || c->row[i].length > NSTEP ||
            (c->row[i].bpm && (c->row[i].bpm < 20u || c->row[i].bpm > 300u)))
            return 0;
    return 1;
}
static const step_t *seq_steps(const track_t *t)
{
    return chain.running ? chain.step[t - trk] : t->step;
}
static void motion_restore(track_t *t);
static void chain_read_steps(uint32_t slot,step_t out[NTRK][NSTEP],const uint8_t micro[NTRK][NSTEP]);
static void chain_apply(void)
{
    uint32_t i;
    const chain_row_t *r = &chain.config.row[chain.row];
    chain.slot = r->slot;
    chain_read_steps(chain.slot,chain.step,chain.source[chain.slot].micro);
    for (i = 0; i < NTRK; i++) { seq_release(&trk[i]); motion_restore(&trk[i]); }
    if (chain.config.rsv[0] & CHAIN_SOUNDS) chain_sound_wire(chain.slot);
    if (r->bpm) song.g[G_BPM] = (int16_t)r->bpm;
    chain.elapsed = 0;
    for (i = 0; i < NTRK; i++) {
        track_t *t = &trk[i];
        memcpy(&t->p[P_SLEN], chain.source[chain.slot].timing[i], sizeof chain.timing[i]);
        t->p[P_MUTE] = (int16_t)(((r->mute >> i) & 1u) ||
            ((chain.config.rsv[0] & CHAIN_SOUNDS) ? chain_wire_param(chain.slot,i,P_MUTE) : chain_original.p[i][P_MUTE]));
        t->seq_idx = (uint16_t)(t->p[P_SLEN] - 1);
        t->seq_pos = 0x7FFFFFFFu;
        t->seq_cycle = 0; t->micro_mode = 0; t->rat_left = 0;
        t->rh_n = t->rskip_n = 0;
    }
}
static void chain_start(void)
{
    uint32_t i;
    if (!chain.armed)
        return;
    chain.rec = song.rec;
    song.rec = 0;
    for (i = 0; i < NTRK; i++)
        memcpy(chain.timing[i], &trk[i].p[P_SLEN], sizeof chain.timing[i]);
    chain_sound_capture(&chain_original);
    chain.row = chain.config.rsv[1] < chain.config.count ? chain.config.rsv[1] : 0u;
    chain.remaining = chain.config.row[chain.row].repeat;
    chain.carry = 0;
    chain.running = 1;
    chain.armed = 0;
    chain_apply();
}
static void chain_stop(void)
{
    uint32_t i;
    chain.armed = 0;
    if (!chain.running)
        return;
    if (chain.config.rsv[0] & CHAIN_SOUNDS) chain_sound_apply(&chain_original);
    for (i = 0; i < NTRK; i++) {
        memcpy(&trk[i].p[P_SLEN], chain.timing[i], sizeof chain.timing[i]);
        trk[i].p[P_MUTE] = chain_original.p[i][P_MUTE]; trk[i].micro_mode = 0;
    }
    song.g[G_BPM] = chain_original.g[G_BPM];
    song.rec = chain.rec;
    chain.running = 0;
}
static void chain_tick(uint32_t n)
{
    const track_t *t = &trk[0];
    const chain_row_t *r;
    uint32_t duration, length;
    if (!chain.running || t->seq_pos >= 0x7FFFFFFFu) return;
    duration = step_samples(t, div_samples((uint32_t)t->p[P_SDIV]), t->seq_idx);
    if (t->seq_pos + n < duration) return;
    r = &chain.config.row[chain.row];
    length = r->length ? r->length : (uint32_t)t->p[P_SLEN];
    if (r->length ? ++chain.elapsed < length : t->seq_idx+1u < length) return;
    chain.elapsed = 0;
    chain.carry = t->seq_pos + n - duration;
    if (chain.remaining > 1u) {
        chain.remaining--;
        if (r->length) for (uint32_t i = 0; i < NTRK; i++) {
            seq_release(&trk[i]);
            trk[i].seq_idx = (uint16_t)(trk[i].p[P_SLEN] - 1);
            trk[i].seq_pos = 0x7FFFFFFFu; trk[i].seq_cycle++;
            trk[i].micro_mode = 0; trk[i].rat_left = 0;
        }
        return;
    }
    if (chain.row + 1u >= chain.config.count) {
        if (!(chain.config.rsv[0] & CHAIN_LOOP)) { seq_stop(); return; }
        chain.row = chain.config.rsv[2] < chain.config.count ? chain.config.rsv[2] : 0u;
    } else chain.row++;
    chain.remaining = chain.config.row[chain.row].repeat;
    chain_apply();
}

static void chain_from_legacy(chain_config_t *out, const chain_v6_t *in)
{
    chain_defaults(out);
    out->count = in->count > 16u ? 16u : in->count;
    for (uint32_t i = 0; i < out->count; i++) {
        out->row[i].slot = in->row[i].slot; out->row[i].repeat = in->row[i].repeat;
    }
}
static int chain_legacy_valid(const chain_v6_t *c)
{
    if (c->count > 16u) return 0;
    for (uint32_t i = 0; i < c->count; i++)
        if (c->row[i].slot > 3u || !c->row[i].repeat || c->row[i].repeat > 16u) return 0;
    return 1;
}
static void chain_to_legacy(chain_v6_t *out, const chain_config_t *in)
{
    memset(out, 0, sizeof *out); out->count = in->count > 16u ? 16u : in->count;
    for (uint32_t i = 0; i < out->count; i++) {
        out->row[i].slot = in->row[i].slot;
        out->row[i].repeat = in->row[i].repeat > 16u ? 16u : in->row[i].repeat;
    }
}

static void chain_legacy_defaults(chain_v6_t *c) { memset(c,0,sizeof *c);for(uint32_t i=0;i<16u;i++)c->row[i].repeat=1; }

#include "pattern_wire.c"
