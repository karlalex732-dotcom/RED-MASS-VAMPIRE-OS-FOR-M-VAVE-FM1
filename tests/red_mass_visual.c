/* SPDX-License-Identifier: GPL-3.0-only
 * Real firmware renderer: check every HOME track frame and capture both preset generations. */
#define UI_TEST_NO_MAIN 1
#include "ui_test.c"

static int same(uint32_t x, uint32_t y, uint16_t color)
{
    return swap16(host_screen[y * 240u + x]) == color;
}

static int name_ink_span(int *top, int *bottom)
{
    uint32_t x, y;
    *top = 240; *bottom = -1;
    for (y = 108; y < 130; y++)
        for (x = 10; x < 198; x++)
            if (same(x, y, T_TEXT)) {
                if ((int)y < *top) *top = (int)y;
                if ((int)y > *bottom) *bottom = (int)y;
            }
    return *bottom >= *top;
}

int main(int argc, char **argv)
{
    int first_top = -1, first_bottom = -1, top, bottom, bad = 0;
    uint32_t i;
    char name[32];
    if (argc != 2) return 2;
    for (i = 0; i < NTRK; i++) {
        uint32_t y = i == 5u ? 148u + 7u : 24u + 11u + i * 21u;
        ui_power_on(); palette_set(UI_REDMASS_INDEX); settings.palette = UI_REDMASS_INDEX;
        song.sel = i; go_home();
        snprintf(name, sizeof name, "home-track-%u", i + 1u);
        screen_save(argv[1], name);
        for (uint32_t x = 207; x <= 231; x++)
            if (!same(x, y, T_THEME) || !same(x, y + 18u, T_THEME)) bad++;
        for (uint32_t yy = y; yy <= y + 18u; yy++)
            if (!same(207, yy, T_THEME) || !same(231, yy, T_THEME)) bad++;
        if (bad) fprintf(stderr, "HOME track %u selection frame incomplete (%d pixels)\n", i + 1u, bad);
    }
    for (i = 0; i < 2u; i++) {
        uint32_t e = i ? CLUB_ENGINE[0] : 0u, k = i ? CLUB_FACTORY[0] : 0u;
        ui_power_on(); palette_set(UI_REDMASS_INDEX); settings.palette = UI_REDMASS_INDEX;
        set_engine_of(TSEL, e); apply_preset_to(TSEL, k);
        sound_name(TSEL, name);
        if (text_w(&AF_M, name) > 193) { fprintf(stderr, "HOME name exceeds space: %s\n", name); bad++; }
        go_home();
        screen_save(argv[1], i ? "home-new-preset" : "home-old-preset");
        if (!name_ink_span(&top, &bottom)) { fprintf(stderr, "HOME name missing: %s\n", name); bad++; }
        if (!i) { first_top = top; first_bottom = bottom; }
        else if (top != first_top || bottom != first_bottom) {
            fprintf(stderr, "HOME preset type heights differ: old %d..%d, new %d..%d\n", first_top, first_bottom, top, bottom);
            bad++;
        }
        go_page(GR_BROWSE);
        screen_save(argv[1], i ? "browse-new-preset" : "browse-old-preset");
        printf("visual: %s e%u:%u HOME name %s, ink %d..%d\n", i ? "new" : "old", e, k, name, top, bottom);
    }
    {
        uint32_t total = 0, wide_home = 0, wide_list = 0;
        int32_t widest = 0;
        const char *longest = "";
        for (uint32_t e = 0; e < NENGINES; e++)
            for (uint32_t k = 0; k < ENGINES[e]->npresets; k++) {
                const char *n = ENGINES[e]->presets[k].name;
                int32_t w = text_w(&AF_M, n);
                total++;
                if (w > 193) wide_home++;
                if (w > 152) wide_list++;
                if (w > widest) widest = w, longest = n;
            }
        printf("visual: %u factory names, longest %s (%d px); HOME overflow %u, browser overflow %u\n",
               total, longest, widest, wide_home, wide_list);
        if (total < 256u || wide_home || wide_list) bad++;
    }
    printf("visual: six complete HOME frames, two preset generations: %d failures\n", bad);
    return bad != 0;
}
