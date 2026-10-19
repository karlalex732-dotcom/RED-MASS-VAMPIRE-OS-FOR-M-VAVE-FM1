/* SPDX-License-Identifier: GPL-3.0-only
 * RED MASS VAMPIRE OS: deterministic pattern tools and RAM A/B scenes.
 * Runs in the main loop. Pattern edits are stopped-only and use the existing undo.
 * No allocation, flash-layout changes, or modifications to the update loader. */
enum { VT_ROT, VT_REV, VT_TRANS, VT_DUP, VT_HUMAN, VT_SHUFFLE };
static uint8_t vampire_pending_graph;
static uint32_t vampire_rand(uint32_t *state)
{
    uint32_t s = *state;
    s ^= s << 13; s ^= s >> 17; s ^= s << 5;
    *state = s;
    return s;
}
static uint32_t vampire_graph(uint32_t graph)
{
    return graph == GR_VEUCLID || graph == GR_VTOOLS || graph == GR_VSCENE;
}
static uint32_t vampire_pulses(const track_t *t)
{
    return (uint32_t)clamp(VP(t, P_VPULSE), 0, clamp(VP(t, P_VLEN), 1, NSTEP));
}
static uint32_t vampire_hit(uint32_t step, uint32_t len, uint32_t pulse, uint32_t rotate)
{
    uint32_t place;
    if (!len || !pulse) return 0;
    if (pulse >= len) return 1;
    place = (step + len - rotate % len) % len;
    return (place * pulse) % len < pulse;
}
static void vampire_euclid(track_t *t)
{
    uint32_t len = (uint32_t)clamp(VP(t, P_VLEN), 1, NSTEP), i;
    uint32_t pulses = vampire_pulses(t), rotate = (uint32_t)VP(t, P_VROT);
    uint32_t lane = (uint32_t)clamp(VP(t, P_VLANE), 0, NPAD);
    motion_store_t keep;
    motion_snapshot_track(t, &keep);
    undo.keep = 0;                                    /* one apply = one undo boundary */
    load_begin(t, UNDO_PAT);
    if (!lane) {
        memset(t->step, 0, sizeof t->step);
        /* Replacing notes discards old automation; the prior copy is in UNDO. */
    } else {
        (void)motion_replace_track(t, &keep);           /* editing one drum lane keeps its locks */
    }
    for (i = 0; i < NSTEP; i++) {
        step_t *s = &t->step[i];
        uint32_t on = i < len && vampire_hit(i, len, pulses, rotate);
        if (lane) {
            if (lane <= NLANE || t == &trk[NTRK-1u]) {
                uint32_t bit = 1u << (lane - 1u);
                s->hit = (uint16_t)((s->hit & ~bit) | (on ? bit : 0u));
                s->acc &= (uint16_t)~bit; s->ghost &= (uint16_t)~bit;
            } else {
                uint32_t n=0, j; /* on melodic tracks, extra percussion pads remain ordinary GM notes */
                for (j=0;j<s->n && j<4u;j++) if (s->note[j] != GROOVE_NOTE[lane-1u]) s->note[n++]=s->note[j];
                if (on && n<4u) s->note[n++]=GROOVE_NOTE[lane-1u]; s->n=(uint8_t)n;
            }
        } else if (on) {
            s->n = 1; s->note[0] = last_note; s->vel = 96;
        }
        if (s->n || s->hit) s->time = ST_NOTE;
        else if (!lane) s->time = ST_REST;
    }
    t->p[P_SLEN] = (int16_t)len;
    pat_last[trk_index(t)] = 0;
    load_end(t);
    ui.force = 1;
    ui_message("EUCLID APPLIED");
}
static int vampire_tools(track_t *t)
{
    step_t work[NSTEP];
    motion_store_t keep;
    uint8_t map[NSTEP];
    uint32_t len = (uint32_t)clamp(t->p[P_SLEN], 1, NSTEP), i, j, k = trk_index(t);
    uint32_t op = (uint32_t)clamp(VP(t, P_VOP), 0, VT_SHUFFLE);
    int32_t amount = clamp(VP(t, P_VAMT), -63, 63);
    uint32_t seed = 0x56414D50u ^ ((uint32_t)VP(t, P_VSEED) + 1u) * 2654435761u;
    uint32_t duplicate = 0;
    if (op == VT_DUP && len > NSTEP / 2u) { ui_message("MAX 64 STEPS"); return 1; }
    motion_snapshot_track(t, &keep);
    if (op == VT_DUP) {
        for (i = 0; i < keep.count; i++) duplicate += (keep.event[i].place & 63u) < len;
        if (motion.count + duplicate > MOTION_MAX) { ui_message("AUTOMATION FULL"); return 1; }
    }
    memcpy(work, t->step, sizeof work);
    for (i = 0; i < NSTEP; i++) map[i] = (uint8_t)i;
    if (op == VT_ROT)
        for (i = 0; i < len; i++) map[i] = (uint8_t)(((int32_t)i + amount % (int32_t)len + (int32_t)len) % (int32_t)len);
    if (op == VT_REV)
        for (i = 0; i < len; i++) map[i] = (uint8_t)(len - 1u - i);
    if (op == VT_SHUFFLE)
        for (i = len - 1u; i > 0; i--) {
            uint8_t tmp = map[i];
            j = vampire_rand(&seed) % (i + 1u); map[i] = map[j]; map[j] = tmp;
        }
    undo.keep = 0;
    load_begin(t, UNDO_PAT);
    for (i = 0; i < len; i++) t->step[map[i]] = work[i];
    for (i = 0; i < keep.count; i++) {
        uint32_t old = keep.event[i].place & 63u;
        keep.event[i].place = (uint8_t)((k << 6) | map[old]);
    }
    if (op == VT_TRANS)
        for (i = 0; i < len; i++)
            for (j = 0; j < t->step[i].n && j < 4u; j++)
                t->step[i].note[j] = (uint8_t)clamp((int32_t)t->step[i].note[j] + amount, 1, 127);
    if (op == VT_HUMAN) {
        uint32_t depth = (uint32_t)(amount < 0 ? -amount : amount);
        for (i = 0; i < len; i++) {
            step_t *s = &t->step[i];
            if (!s->n && !s->hit) continue;
            int32_t vel = s->vel ? s->vel : 96;
            vel += (int32_t)(vampire_rand(&seed) % (2u * depth + 1u)) - (int32_t)depth;
            s->vel = (uint8_t)clamp(vel, 1, 126);
            s->flags &= (uint8_t)~SF_ACCENT;
            s->acc = 0;                              /* accents would otherwise override velocity */
        }
    }
    if (op == VT_DUP) {
        uint32_t old_count = keep.count;
        memcpy(t->step + len, work, len * sizeof work[0]);
        /* Any old hidden steps in the newly active range are replaced, including their locks. */
        j = 0;
        for (i = 0; i < old_count; i++)
            if ((keep.event[i].place & 63u) < len || (keep.event[i].place & 63u) >= 2u * len)
                keep.event[j++] = keep.event[i];
        keep.count = (uint8_t)j;
        old_count = keep.count;
        for (i = 0; i < old_count; i++)
            if ((keep.event[i].place & 63u) < len) {
                motion_event_t e = keep.event[i];
                e.place = (uint8_t)(e.place + len);
                keep.event[keep.count++] = e;
            }
        t->p[P_SLEN] = (int16_t)(2u * len);
    }
    (void)motion_replace_track(t, &keep);              /* capacity was checked before touching the pattern */
    pat_last[k] = 0;
    load_end(t);
    ui.force = 1;
    ui_message("PATTERN CHANGED");
    return 0;
}
static int vampire_scene_param(const track_t *t, uint32_t id)
{
    return motion_param(id) && track_desc(t, id)->fmt != F_ENUM;
}
static void vampire_capture(uint32_t side)
{
    uint32_t i, k = song.sel, f = motion_guard();
    side &= 1u;
    for (i = 0; i < P_COUNT; i++) vampire_scene.p[k][side][i] = motion_base_value(TSEL, i);
    vampire_scene.engine[k][side] = TSEL->eng_req;
    vampire_scene.valid[k] |= (uint8_t)(1u << side);
    vampire_scene.gen++;
    motion_unguard(f);
    ui.force = 1;
    ui_message(side ? "SCENE B CAPTURED" : "SCENE A CAPTURED");
}
static int vampire_blend(uint32_t blend)
{
    uint32_t i, k = song.sel, f;
    if (vampire_scene.valid[k] != 3u) { ui_message("CAPTURE A AND B"); return 1; }
    if (vampire_scene.engine[k][0] != TSEL->eng_req || vampire_scene.engine[k][1] != TSEL->eng_req) {
        ui_message("SCENES NEED SAME ENG"); return 1;
    }
    if (song.rec) { ui_message("DISARM REC FIRST"); return 1; }
    if (chain_busy() || (motion_enabled(TSEL) && motion_count(TSEL))) {
        ui_message("TURN AUTOMATION OFF"); return 1;
    }
    blend = blend > 100u ? 100u : blend;
    f = motion_guard();
    for (i = 0; i < P_COUNT; i++) {
        const param_desc_t *d = track_desc(TSEL, i);
        if (!vampire_scene_param(TSEL, i)) continue;
        int32_t a = vampire_scene.p[k][0][i], b = vampire_scene.p[k][1][i];
        TSEL->p[i] = (int16_t)param_fit(d, a + (b - a) * (int32_t)blend / 100);
    }
    vampire_scene.blend[k] = (uint8_t)blend;
    vampire_scene.gen++;
    motion_unguard(f);
    ui.force = 1;
    return 0;
}
static void vampire_scene_action(uint32_t col)
{
    if (col == 0u || col == 1u) vampire_capture(col);
    else if (col == 3u) {
        if (!vampire_blend(0u)) ui_message("BACK TO SCENE A");
    }
}
static void vampire_apply(uint32_t graph, uint32_t track)
{
    if (song.playing || chain_busy() || transport_req == 1u) { ui_message("STOP TO APPLY"); return; }
    track_t *t = &trk[track % NTRK];
    if (graph == GR_VEUCLID) vampire_euclid(t);
    else if (graph == GR_VTOOLS) (void)vampire_tools(t);
}
