/* SPDX-License-Identifier: GPL-3.0-only
 * Capture the actual 0.5.1 firmware reply that overflows ED_NAMES for VAMPIRE.
 * The editor's compatibility test consumes these exact SysEx bytes. */
#define EDITOR_TEST_NO_MAIN 1
#include "editor_test.c"

int main(int argc, char **argv)
{
    const uint8_t engine = ENGI_VAMPIRE;
    uint32_t n;
    FILE *out;
    if (argc != 2) return 2;
    reset();
    n = request(ED_NAMES, &engine, 1);
    if (n != 600u || host_wire[0] != 0xF0 || host_wire[4] != ED_NAMES ||
        host_wire[5] != engine || host_wire[6] != ENGINES[engine]->npresets || host_wire[n - 1u] != 0xF7)
        return 1;
    out = fopen(argv[1], "wb");
    if (!out) return 1;
    if (fwrite(host_wire, 1, n, out) != n) { fclose(out); return 1; }
    if (fclose(out)) return 1;
    printf("editor native NAMES reply: %u bytes, %u declared VAMPIRE presets\n", n, ENGINES[engine]->npresets);
    return 0;
}
