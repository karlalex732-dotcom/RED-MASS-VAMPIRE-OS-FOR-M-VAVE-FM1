/* SPDX-License-Identifier: GPL-3.0-only
 * RED_MASS 0.4: bounded runtime state, macros and sixteen step-edit transactions. */
static uint32_t upgrade_pad_chance(uint32_t k, uint32_t pad)
{
    uint32_t c = upgrade.pad_chance[k % NTRK][pad % NPAD];
    return !c ? 100u : c <= 100u ? c : 0u;
}
static void upgrade_set_pad_chance(uint32_t k, uint32_t pad, uint32_t c)
{
    upgrade.pad_chance[k % NTRK][pad % NPAD] = (uint8_t)(c >= 100u ? 0u : c ? c : 101u);
    upgrade_gen++;
}
static int upgrade_sound_param(const track_t *t, uint32_t id)
{
    return id < P_COUNT && motion_param(id) && track_desc(t,id)->fmt != F_ENUM;
}
static void upgrade_macro_rebase(track_t *t)
{
    uint32_t k = trk_index(t), m, d;
    memset(upgrade.macro_value[k], 0, sizeof upgrade.macro_value[k]);
    for (m = 0; m < 4u; m++) for (d = 0; d < 4u; d++) {
        upgrade_macro_target_t *v = &upgrade.macro[k][m][d];
        v->base = v->param < P_COUNT ? motion_base_value(t,v->param) : 0;
    }
    upgrade_gen++;
}
static int upgrade_macro_set(track_t *t, uint32_t m, uint32_t value)
{
    uint32_t k = trk_index(t), f, a, b, seen[3] = {0,0,0};
    if (m >= 4u || value > 127u || chain.running || song.rec || (motion_enabled(t) && motion_count(t))) return 1;
    f = motion_guard(); upgrade.macro_value[k][m] = (uint8_t)value;
    for (a = 0; a < 4u; a++) for (b = 0; b < 4u; b++) {
        const upgrade_macro_target_t *v = &upgrade.macro[k][a][b];
        uint32_t id = v->param;
        if (!v->amount || !upgrade_sound_param(t,id) || ((seen[id >> 5] >> (id & 31u)) & 1u)) continue;
        seen[id >> 5] |= 1u << (id & 31u);
        int32_t out = v->base;
        const param_desc_t *pd = track_desc(t,id);
        for (uint32_t x = 0; x < 4u; x++) for (uint32_t y = 0; y < 4u; y++) {
            const upgrade_macro_target_t *w = &upgrade.macro[k][x][y];
            if (w->param == id) out += (int32_t)w->amount * upgrade.macro_value[k][x] * (pd->max - pd->min) / (127 * 127);
        }
        t->p[id] = (int16_t)param_fit(pd,out);
    }
    upgrade_gen++; motion_unguard(f); return 0;
}

#define UPGRADE_HISTORY 16u
typedef struct { step_t before, after; uint8_t track, step; } upgrade_edit_t;
static struct {
    upgrade_edit_t edit[UPGRADE_HISTORY];
    step_t before;
    uint8_t count, pos, armed, track, step;
} upgrade_history;
static void upgrade_history_clear(void) { memset(&upgrade_history,0,sizeof upgrade_history); upgrade_gen++; }
static void upgrade_note_before(track_t *t, uint32_t step)
{
    upgrade_history.armed = 0;
    if (step >= NSTEP || song.playing || song.rec || chain.running) return;
    upgrade_history.before = t->step[step];
    upgrade_history.track = (uint8_t)trk_index(t); upgrade_history.step = (uint8_t)step;
    upgrade_history.armed = 1;
}
static void upgrade_note_after(void)
{
    upgrade_edit_t *v;
    if (!upgrade_history.armed) return;
    upgrade_history.armed = 0;
    const step_t *s = &trk[upgrade_history.track].step[upgrade_history.step];
    if (!memcmp(s,&upgrade_history.before,sizeof *s)) return;
    upgrade_history.count = upgrade_history.pos;
    if (upgrade_history.count == UPGRADE_HISTORY) {
        for (uint32_t i=1;i<UPGRADE_HISTORY;i++) upgrade_history.edit[i-1u]=upgrade_history.edit[i];
        upgrade_history.count--; upgrade_history.pos--;
    }
    v = &upgrade_history.edit[upgrade_history.count++];
    v->before = upgrade_history.before; v->after = *s;
    v->track = upgrade_history.track; v->step = upgrade_history.step;
    upgrade_history.pos = upgrade_history.count; upgrade_gen++;
}
static int upgrade_history_move(int redo)
{
    upgrade_edit_t *v;
    if (song.playing || song.rec || chain.running ||
        (redo ? upgrade_history.pos == upgrade_history.count : !upgrade_history.pos)) return 1;
    uint32_t f = motion_guard();
    v = &upgrade_history.edit[redo ? upgrade_history.pos++ : --upgrade_history.pos];
    trk[v->track].step[v->step] = redo ? v->after : v->before;
    motion_unguard(f); upgrade_gen++; return 0;
}
