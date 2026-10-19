/* SPDX-License-Identifier: GPL-3.0-only
 * Predecoded section sounds. No flash reads at an audio/arranger boundary. */
typedef struct {
    int16_t p[NTRK][P_COUNT], g[G_COUNT];
    uint8_t fm6[NTRK][FM6_PACKED], engine[NTRK], preset[NTRK], user[NTRK], route[NTRK][3];
    uint8_t chance[NTRK][NPAD], groove;
} chain_sound_t;
static uint8_t chain_chance[4][NTRK][NPAD];
static void chain_sound_wire(uint32_t slot);
static int16_t chain_wire_param(uint32_t slot,uint32_t track,uint32_t param);
static chain_sound_t chain_original __attribute__((section(".auxpool")));
static void chain_sound_capture(chain_sound_t *s)
{
    uint32_t k;
    memcpy(s->g, song.g, sizeof s->g); s->groove = song.groove;
    memcpy(s->chance, upgrade.pad_chance, sizeof s->chance);
    for (k = 0; k < NTRK; k++) {
        memcpy(s->p[k], trk[k].p, sizeof s->p[k]);
        s->engine[k] = trk[k].eng_req; s->preset[k] = trk[k].preset;
        s->user[k] = trk[k].user;
        s->route[k][0] = trk[k].route; s->route[k][1] = trk[k].midi_ch; s->route[k][2] = trk[k].midi_send;
        fm6_pack(fm6_patch[k], s->fm6[k]);
    }
}
static void chain_sound_apply(const chain_sound_t *s)
{
    uint32_t k, i;
    song.groove = s->groove;
    for (i = G_DTIME; i <= G_CDEPTH; i++) song.g[i] = s->g[i];
    song.g[G_SWING] = s->g[G_SWING]; song.g[G_TUNE] = s->g[G_TUNE];
    song.g[G_RTYPE] = s->g[G_RTYPE];
    song.g[G_DRLVL] = s->g[G_DRLVL]; song.g[G_DRREV] = s->g[G_DRREV];
    memcpy(upgrade.pad_chance, s->chance, sizeof s->chance);
    for (k = 0; k < NTRK; k++) {
        uint8_t v[FP_SIZE + 1u];
        memcpy(trk[k].p, s->p[k], sizeof s->p[k]);
        trk[k].eng_req = s->engine[k]; trk[k].preset = s->preset[k];
        trk[k].user = s->user[k];
        trk[k].route = s->route[k][0]; trk[k].midi_ch = s->route[k][1]; trk[k].midi_send = s->route[k][2];
        fm6_unpack(s->fm6[k], v); fm6_set_patch(k, v);
        /* The packed sound is authoritative. Avoid a factory-bank scan in the ISR. */
        fm6_slot[k] = (uint8_t)trk[k].p[P_E7]; fm6_own_ok |= (uint8_t)(1u << k);
        memcpy(fm6_own[k], s->fm6[k], FM6_PACKED);
    }
}
