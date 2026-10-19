/* SPDX-License-Identifier: GPL-3.0-only
 * New OS behaviour through the real UI and audio code, with host HAL stubs.
 * Optional argument: render all 64 factory patches, after their actual OS FX.
 */
#define UI_TEST_NO_MAIN 1
#include "ui_test.c"

static int vt_check(const char *s, int ok) { return check(s, ok); }
static void vt_fresh(uint32_t pi)
{
    ui_power_on();
    memset(&vampire_scene, 0, sizeof vampire_scene);
    memset(dly_buf, 0, sizeof dly_buf); memset(cho_buf, 0, sizeof cho_buf);
    memset(rev_comb, 0, sizeof rev_comb); memset(&rev_u, 0, sizeof rev_u);
    memset(&fx, 0, sizeof fx); memset(&mod, 0, sizeof mod);
    lim_env = LIM_T; vage = 0; song.master_q12 = 4096;
    set_engine_of(TSEL, ENGI_VAMPIRE); apply_preset_to(TSEL, pi);
    TSEL->engine = TSEL->eng_req; panic_req = 0; undo.trk = 0;
}
static int vt_tools(void)
{
    uint32_t l, p, i, n; int bad = 0, good = 1;
    for (l = 1; l <= NSTEP; l++) for (p = 0; p <= l; p++) {
        n = 0; for (i = 0; i < l; i++) n += vampire_hit(i, l, p, l / 3u);
        if (n != p) good = 0;
    }
    bad += vt_check("Euclid: exact hit count for every length and fill 1..64", good);
    vt_fresh(0); track_t *t = TSEL;
    VP(t, P_VLEN) = 16; VP(t, P_VPULSE) = 5; VP(t, P_VROT) = 2;
    go_page(GR_VEUCLID); press(B_OCTUP);
    n = 0; for (i = 0; i < NSTEP; i++) n += t->step[i].n != 0;
    bad += vt_check("Euclid: real OCT+ applies 5 notes, length 16", n == 5 && t->p[P_SLEN] == 16);
    step_t backup[NSTEP]; memcpy(backup, t->step, sizeof backup);
    VP(t, P_VPULSE) = 7; press(B_OCTUP);
    bad += vt_check("Euclid replacement asks before changing existing steps", ui.confirm == CF_VAMPIRE && !memcmp(backup, t->step, sizeof backup));
    press(B_OCTUP); hold(B_SAVE);
    bad += vt_check("Euclid: SAVE held undoes replacement", !memcmp(backup, t->step, sizeof backup));
    song.playing = 1; VP(t, P_VPULSE) = 8; press(B_OCTUP);
    bad += vt_check("Euclid: playing blocks pattern replacement", !memcmp(backup, t->step, sizeof backup));
    stop_transport(); ui.confirm = 0;
    track_defaults_steps(t); t->p[P_SLEN] = 8;
    for (i = 0; i < 8; i++) { t->step[i].n = 1; t->step[i].note[0] = 60 + i; t->step[i].time = ST_NOTE; t->step[i].vel = 96; }
    motion_set_event(t, 2, P_E1, 83); VP(t, P_VOP) = VT_ROT; VP(t, P_VAMT) = 3;
    vampire_tools(t);
    bad += vt_check("Rotate: note data and matching parameter lock move together", t->step[5].note[0] == 62 && (motion.event[0].place & 63u) == 5);
    undo_swap();
    bad += vt_check("Rotate undo: restores note and lock", t->step[2].note[0] == 62 && (motion.event[0].place & 63u) == 2);
    VP(t, P_VOP) = VT_REV; vampire_tools(t);
    bad += vt_check("Reverse: note data and lock move together", t->step[5].note[0] == 62 && (motion.event[0].place & 63u) == 5);
    undo_swap(); VP(t, P_VOP) = VT_DUP; vampire_tools(t);
    bad += vt_check("Duplicate: doubles length and duplicates automation", t->p[P_SLEN] == 16 && t->step[10].note[0] == 62 && motion_count(t) == 2);
    undo_swap(); VP(t, P_VOP) = VT_TRANS; VP(t, P_VAMT) = -12; vampire_tools(t);
    bad += vt_check("Transpose: all programmed notes move one octave", t->step[0].note[0] == 48 && t->step[7].note[0] == 55);
    undo_swap(); VP(t, P_VOP) = VT_HUMAN; VP(t, P_VAMT) = 12; vampire_tools(t);
    memcpy(backup, t->step, sizeof backup); undo_swap(); vampire_tools(t);
    bad += vt_check("Humanize: identical seed gives identical variation", !memcmp(backup, t->step, sizeof backup));
    undo_swap(); VP(t, P_VOP) = VT_SHUFFLE; vampire_tools(t);
    n = 0; for (i = 0; i < 8; i++) n += t->step[i].note[0];
    bad += vt_check("Shuffle: preserves note multiset and lock association", n == 508 && t->step[motion.event[0].place & 63u].note[0] == 62);
    undo_swap(); VP(t, P_VLEN) = 8; VP(t, P_VPULSE) = 3; VP(t, P_VLANE) = 1;
    t->step[0].hit = 2; vampire_euclid(t);
    n = 0; for (i = 0; i < 8; i++) n += !!(t->step[i].hit & 1);
    bad += vt_check("Euclid drum lane preserves other lane, notes and automation", n == 3 && (t->step[0].hit & 2) && t->step[0].n && motion_count(t) == 1);
    return bad;
}
static int vt_scenes_scope(void)
{
    int bad = 0; uint32_t i, mode; uint64_t h[4];
    vt_fresh(0); go_page(GR_VSCENE); TSEL->p[P_E1] = 20;
    turn(EN_K1, 1); press(B_OCTUP); TSEL->p[P_E1] = 100;
    turn(EN_K2, 1); press(B_OCTUP); turn(EN_K3, 50);
    bad += vt_check("Scenes: real knobs capture A/B and interpolate COLOR", vampire_scene.valid[0] == 3 && TSEL->p[P_E1] == 60);
    song.rec = 1;
    bad += vt_check("Scenes: recording refuses morph without modifying sound", vampire_blend(80) && TSEL->p[P_E1] == 60);
    song.rec = 0; motion_set_event(TSEL, 0, P_E1, 50);
    bad += vt_check("Scenes: active automation refuses morph", vampire_blend(80) && TSEL->p[P_E1] == 60);
    motion_clear(TSEL); TSEL->eng_req = 0;
    bad += vt_check("Scenes: another engine refuses stale scene values", vampire_blend(80));
    TSEL->eng_req = ENGI_VAMPIRE; vampire_blend(0);
    bad += vt_check("Scenes: RESET restores captured A", TSEL->p[P_E1] == 20);
    go_home(); palette_set(UI_VAMP_INDEX);
    for (i = 0; i < SCOPE_N; i++) scope_buf[i] = (int16_t)(11000 * sin(i * 0.1) + 4000 * sin(i * 0.23));
    for (mode = 0; mode < 4; mode++) {
        vamp_scope = mode; cv_begin(240, H_GRAPH, T_SURF); graph_scope(T_THEME); cv_blit(0, Y_GRAPH);
        h[mode] = 1469598103934665603ull;
        for (i = 0; i < 240 * 240; i++) h[mode] = (h[mode] ^ host_screen[i]) * 1099511628211ull;
    }
    bad += vt_check("Wave modes: LIVE GLOW MIRROR BARS produce four distinct screens", h[0] != h[1] && h[1] != h[2] && h[2] != h[3] && h[0] != h[2] && h[0] != h[3] && h[1] != h[3]);
    return bad;
}
static int vt_persistence(void)
{
    int bad = 0; project_t p, q; project_store_t packed;
    vt_fresh(63); VP(TSEL,P_VLEN) = 32; VP(TSEL,P_VPULSE) = 11;
    vampire_euclid(TSEL); motion_set_event(TSEL, 3, P_E1, 92);
    project_capture(&p);
    bad += vt_check("VAMPIRE project: unchanged FUN8 extent, steps/locks/sound round-trip", P_COUNT == 91 &&
        proj_pack(&packed, &p) && sizeof packed == PROJ_STORE_SIZE && proj_import(&q, &packed, sizeof packed) &&
        q.t[0].engine == ENGI_VAMPIRE && !memcmp(q.t[0].p, p.t[0].p, sizeof p.t[0].p) &&
        !memcmp(q.t[0].step, p.t[0].step, sizeof p.t[0].step) && q.motion.count == p.motion.count &&
        q.motion.event[0].param == P_E1 && q.motion.event[0].value == 92);
    int rc = up_store(0, "ETERNAL"); set_engine_of(TSEL, 0); up_load(0);
    bad += vt_check("VAMPIRE user preset: engine, four modulation slots and FX restored", (rc == 0 || rc == 3) &&
        TSEL->eng_req == ENGI_VAMPIRE && TSEL->p[P_M1DST] == 5 && TSEL->p[P_M2DST] == 2 &&
        TSEL->p[P_M3DST] == 4 && TSEL->p[P_M4DST] == 1 && TSEL->p[P_DIST] == p.t[0].p[P_DIST]);
    persist_t settings_copy; vamp_scope = 2; settings_export(&settings_copy); vamp_scope = 0;
    bad += vt_check("Wave display mode survives settings export/import", settings_import(&settings_copy, sizeof settings_copy) == 1 && vamp_scope == 2);
    return bad;
}
static int vt_render(const char *dir)
{
    uint32_t pi, f, i, j; int bad = 0; uint64_t hashes[64];
    static const uint8_t notes[] = {48, 55, 60, 63};
    int32_t out[CTL * 2]; char path[512]; FILE *stats = NULL, *dump = NULL;
    if (dir) {
        snprintf(path, sizeof path, "%s/audio-health.csv", dir); stats = fopen(path, "w");
        snprintf(path, sizeof path, "%s/native-presets.json", dir); dump = fopen(path, "w");
        if (!stats || !dump) return 1;
        fprintf(stats, "number,name,peak,rms,dc,overrange,voices,remaining,hash\n"); fputc('[', dump);
    }
    for (pi = 0; pi < 64; pi++) {
        vt_fresh(pi); FILE *w = NULL; int32_t peak = 0; uint32_t over = 0, maxv = 0;
        double energy = 0, dc = 0; uint64_t hash = 1469598103934665603ull;
        if (dir) {
            snprintf(path, sizeof path, "%s/%02u-%s.wav", dir, pi + 1, VAMPIRE_PRESETS[pi].name);
            w = fopen(path, "wb"); if (!w) return 1; wav_hdr(w, FS * 12u);
            fprintf(dump, "%s{\"name\":\"%s\",\"params\":[", pi ? "," : "", VAMPIRE_PRESETS[pi].name);
            for (i = 0; i < P_COUNT; i++) fprintf(dump, "%s%d", i ? "," : "", TSEL->p[i]);
            fputs("]}", dump);
        }
        for (j = 0; j < 4; j++) trk_note_on(TSEL, notes[j], 96);
        for (f = 0; f < FS * 18u; f += CTL) {
            if (f <= FS * 3u && f + CTL > FS * 3u)
                for (j = 0; j < 4; j++) trk_note_off(TSEL, notes[j]);
            mix_block(out, CTL);
            uint32_t active = 0; for (j = 0; j < NVOICE; j++) active += TSEL->v[j].active;
            if (active > maxv) maxv = active;
            for (i = 0; i < CTL && f + i < FS * 12u; i++) {
                int32_t l = out[2*i], r = out[2*i+1], al = l < 0 ? -l : l, ar = r < 0 ? -r : r;
                if (al > peak) peak = al; if (ar > peak) peak = ar;
                over += l > 32767 || l < -32768 || r > 32767 || r < -32768;
                energy += (double)l * l + (double)r * r; dc += l + r;
                hash = (hash ^ (uint16_t)l) * 1099511628211ull; hash = (hash ^ (uint16_t)r) * 1099511628211ull;
                if (w) wav_put(w, l, r);
            }
        }
        uint32_t left = 0; for (j = 0; j < NVOICE; j++) left += TSEL->v[j].active;
        energy = sqrt(energy / (FS * 24.0)); dc /= FS * 24.0;
        hashes[pi] = hash; for (j = 0; j < pi; j++) if (hash == hashes[j]) bad++;
        int ok = !over && peak > 500 && peak < 32700 && energy > 30 && fabs(dc) < 350 && maxv == 4 && !left && TSEL->p[P_VOICE] == V_POLY;
        if (!ok) bad++;
        printf("vampire: %02u %-13s peak=%5d rms=%6.1f dc=%6.1f voices=%u left=%u %s\n", pi+1, VAMPIRE_PRESETS[pi].name, peak, energy, dc, maxv, left, ok ? "ok" : "FAIL");
        if (stats) fprintf(stats, "%u,%s,%d,%.2f,%.2f,%u,%u,%u,%016llx\n", pi+1, VAMPIRE_PRESETS[pi].name, peak, energy, dc, over, maxv, left, (unsigned long long)hash);
        if (w) fclose(w);
    }
    if (stats) fclose(stats); if (dump) { fputs("]\n", dump); fclose(dump); }
    return bad;
}
#ifndef VAMPIRE_TEST_NO_MAIN
int main(int argc, char **argv)
{
    int bad = vt_tools() + vt_scenes_scope() + vt_persistence() + vt_render(argc > 1 ? argv[1] : NULL);
    printf("RED_MASS: %s (%d failures)\n", bad ? "FAILED" : "PASS", bad); return bad != 0;
}
#endif
