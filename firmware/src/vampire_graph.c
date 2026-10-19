/* SPDX-License-Identifier: GPL-3.0-only
 * Actual FM1 UI drawing: no simulated data in the waveform. */
static void graph_vampire(void)
{
    uint32_t graph = cur_page()->graph, i;
    const track_t *t = TSEL;
    if (graph == GR_VEUCLID) {
        uint32_t len = (uint32_t)clamp(VP(t, P_VLEN), 1, NSTEP), pulses = vampire_pulses(t);
        for (i = 0; i < len; i++) {
            int32_t x = 16 + (int32_t)(i % 16u) * 13, y = 16 + (int32_t)(i / 16u) * 17;
            uint32_t on = vampire_hit(i, len, pulses, (uint32_t)VP(t, P_VROT));
            cv_rrect(x, y, 9, 10, 2, on ? T_THEME : T_RAISE, T_SURF);
        }
        cv_text_in(0, 94, 240, &AF_S, "[OCT+] APPLY  [SAVE] HOLD: UNDO", T_MID, T_SURF);
    } else if (graph == GR_VTOOLS) {
        static const char *const TITLE[] = {"ROTATE STEPS + LOCKS", "REVERSE STEPS + LOCKS", "TRANSPOSE NOTE DATA",
                                             "DUPLICATE PATTERN x2", "VARY VELOCITY", "SHUFFLE STEPS + LOCKS"};
        uint32_t op = (uint32_t)clamp(VP(t, P_VOP), 0, VT_SHUFFLE);
        cv_text_in(0, 18, 240, &AF_M, TITLE[op], T_TEXT, T_SURF);
        cv_text_in(0, 46, 240, &AF_S, "[OCT+] APPLY WHILE STOPPED", T_MID, T_SURF);
        cv_text_in(0, 70, 240, &AF_S, "[SAVE] HOLD: UNDO / REDO", T_MID, T_SURF);
        cv_text_in(0, 94, 240, &AF_S, "SEED: REPEATABLE VARIATION", T_DIM, T_SURF);
    } else {
        uint32_t blend = vampire_scene.blend[song.sel], v = vampire_scene.valid[song.sel];
        cv_text_in(12, 15, 80, &AF_M, v & 1u ? "A READY" : "A EMPTY", v & 1u ? T_TEXT : T_MID, T_SURF);
        cv_text_in(148, 15, 80, &AF_M, v & 2u ? "B READY" : "B EMPTY", v & 2u ? T_TEXT : T_MID, T_SURF);
        cv_rrect(18, 52, 204, 8, 4, T_RAISE, T_SURF);
        cv_rrect(18, 52, (int32_t)(204u * blend / 100u), 8, 4, T_THEME, T_RAISE);
        cv_text_in(0, 76, 240, &AF_S, "K1 / K2: PICK  [OCT+]: CAPTURE", T_MID, T_SURF);
        cv_text_in(0, 96, 240, &AF_S, "K3: MORPH  K4 + [OCT+]: RESET", T_MID, T_SURF);
    }
}
