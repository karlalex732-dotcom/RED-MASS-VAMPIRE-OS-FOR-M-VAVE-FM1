/* SPDX-License-Identifier: GPL-3.0-only
 * Screens from the actual firmware drawing code at native FM1 resolution.
 * Animated oscilloscope samples are taken from mix_block, exactly as audio.c.
 */
#define UI_TEST_NO_MAIN 1
#define FELUCCA_VERSION "v0.3.0-alpha"
#include "ui_test.c"
static const char *outdir;
static void save_screen(const char *name)
{
    char p[512]; uint32_t i; snprintf(p, sizeof p, "%s/%s.ppm", outdir, name);
    FILE *f = fopen(p, "wb"); if (!f) exit(2);
    fprintf(f, "P6\n240 240\n255\n");
    for (i = 0; i < 240u * 240u; i++) {
        uint16_t c = swap16(host_screen[i]);
        uint8_t rgb[3] = {(uint8_t)(((c >> 11) & 31u) * 255u / 31u),
                         (uint8_t)(((c >> 5) & 63u) * 255u / 63u), (uint8_t)((c & 31u) * 255u / 31u)};
        fwrite(rgb, 1, 3, f);
    }
    fclose(f);
}
static void preview_state(void)
{
    ui_power_on(); settings.palette = UI_VAMP_INDEX; palette_set(UI_VAMP_INDEX);
    set_engine_of(TSEL, ENGI_VAMPIRE); apply_preset_to(TSEL, 0); TSEL->engine = ENGI_VAMPIRE;
    panic_req = 0; vamp_scope = 1; ui_prefs = PREF_BPM_LOCK | PREF_ACCEL;
    song.master_q12 = 4096; ui.force = 1;
}
int main(int argc, char **argv)
{
    uint32_t i, k; char name[32]; int32_t audio[CTL * 2];
    if (argc != 2) return 2; outdir = argv[1]; preview_state();
    lcd_fill(0, 0, 240, 240, T_BG); vampire_boot(47); save_screen("boot");
    for (i = 0; i < 48; i++) { vampire_boot(i); snprintf(name, sizeof name, "boot-%02u", i); save_screen(name); }
    preview_state(); song.groove = 1; track_select(3); set_engine_of(TSEL, ENGI_SMPKIT); TSEL->engine = TSEL->eng_req;
    go_page(GR_ROLL); ui.lane = 12; groove_grid_bank = 8;
    for (i = 0; i < 16u; i++) grid_hit(TSEL,i,i,1);
    ui.force = 1; ui_draw(); save_screen("groove-grid");
    ui.menu = 1; ui.menu_sel = MI_GMODE; ui.force = 1; ui_draw(); save_screen("groove-menu");
    preview_state(); go_page(GR_CHANCE); step_set_condition(&TSEL->step[ui.cursor],COND_NOTFILL);
    ui.force = 1; ui_draw(); save_screen("conditions");
    step_set_condition(&TSEL->step[ui.cursor],COND_THREE4); ui.force = 1; ui_draw(); save_screen("conditions-3-4");
    preview_state(); go_page(GR_VEUCLID); VP(TSEL,P_VLEN) = 32; VP(TSEL,P_VPULSE) = 11;
    ui.force = 1; ui_draw(); save_screen("euclid");
    go_page(GR_VTOOLS); VP(TSEL,P_VOP) = VT_SHUFFLE; VP(TSEL,P_VAMT) = 9;
    ui.force = 1; ui_draw(); save_screen("transform");
    TSEL->p[P_E1] = 24; vampire_capture(0); TSEL->p[P_E1] = 108; vampire_capture(1); vampire_blend(63);
    go_page(GR_VSCENE); ui.msg_t = 0; ui.force = 1; ui_draw(); save_screen("scenes");
    go_page(GR_NONE); /* draw engine SOURCE page, not the first NONE page */
    for (i = 0; i < NPAGES; i++) if (PAGES[i].scope == SC_ENGINE) { ui.page = i; break; }
    ui.force = 1; ui_draw(); save_screen("source");
    go_page(GR_BROWSE); ui.force = 1; ui_draw(); save_screen("bank");
    ui.menu = 1; ui.menu_sel = MI_SCOPE; ui.force = 1; ui_draw(); save_screen("display");
    ui.menu = 2; ui.force = 1; ui_draw(); save_screen("about");
    preview_state(); go_home(); trk_note_on(TSEL, 48, 96); trk_note_on(TSEL, 55, 96);
    trk_note_on(TSEL, 60, 96); trk_note_on(TSEL, 63, 96);
    for (k = 0; k < 96; k++) {
        vamp_scope = k / 24u;
        for (i = 0; i < FS / 30u; i += CTL) {
            mix_block(audio, CTL);
            for (uint32_t j = 1; j < CTL; j += 2) scope_buf[scope_w++ & (SCOPE_N - 1u)] = (int16_t)audio[2*j];
        }
        ui.frame += 2; ui.force = 1; ui_draw();
        snprintf(name, sizeof name, "wave-%02u", k); save_screen(name);
        if (k % 24u == 12u) { snprintf(name, sizeof name, "scope-%u", k/24u); save_screen(name); }
    }
    palette_set(UI_VAMP_INDEX + 1u); ui.force = 1; ui_draw(); save_screen("crypt");
    return 0;
}
