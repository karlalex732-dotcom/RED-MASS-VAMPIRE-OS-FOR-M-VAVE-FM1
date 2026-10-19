/* SPDX-License-Identifier: GPL-3.0-only
 * RED MASS VAMPIRE OS additions, 2026. */
#include "vampire_logo.h"
#ifndef FELUCCA_VERSION
#define FELUCCA_VERSION "v0.5.1-alpha"
#endif
static void vampire_logo(int32_t x, int32_t y, uint16_t color)
{
    uint32_t row, p = 0;
    for (row = 0; row < VAMP_LOGO_H; row++) {
        uint32_t n = VAMP_LOGO[p++];
        while (n--) {
            uint32_t start = VAMP_LOGO[p++], len = VAMP_LOGO[p++];
            cv_rect(x + (int32_t)start, y + (int32_t)row, (int32_t)len, 1, color);
        }
    }
}
static void rm_runs(int32_t x,int32_t y,const uint8_t *runs,uint32_t h,uint16_t color) {
    uint32_t p=0;for(uint32_t row=0;row<h;row++){uint32_t n=runs[p++];while(n--){uint32_t start=runs[p++],len=runs[p++];cv_rect(x+(int32_t)start,y+(int32_t)row,(int32_t)len,1,color);}}
}
static void rm_bat(int32_t x,int32_t y,uint16_t color) {
    cv_rect(x,y,2,8,color);cv_rect(x+10,y,2,8,color);cv_rect(x+2,y+2,8,9,color);cv_rect(x+4,y+11,4,4,color);
}
static uint16_t vampire_boot_color(uint16_t a, uint16_t b, uint32_t k)
{
    uint32_t r = (((a>>11)&31u)*(32u-k)+((b>>11)&31u)*k)>>5;
    uint32_t g = (((a>>5)&63u)*(32u-k)+((b>>5)&63u)*k)>>5;
    uint32_t bl = ((a&31u)*(32u-k)+(b&31u)*k)>>5;
    return (uint16_t)(r<<11 | g<<5 | bl);
}
static void vampire_bat(int32_t x, int32_t y, int32_t size, uint32_t flap, uint16_t color)
{
    uint32_t wing = flap & 15u;
    int32_t lift = (int32_t)(wing < 8u ? wing : 15u-wing)-3, s = size;
    cv_line_t(x-s*3,y-lift,x-s*2,y+3,color,2);
    cv_line_t(x-s*2,y+3,x-s,y+1,color,2);
    cv_line_t(x-s,y+1,x,y+5,color,2);
    cv_line_t(x,y+5,x+s,y+1,color,2);
    cv_line_t(x+s,y+1,x+s*2,y+3,color,2);
    cv_line_t(x+s*2,y+3,x+s*3,y-lift,color,2);
    cv_line_t(x-s*3,y-lift,x,y,color,1);
    cv_line_t(x,y,x+s*3,y-lift,color,1);
    cv_rect(x-1,y-2,3,8,color);
}
static int32_t vampire_wave(uint32_t phase)
{
    int32_t p = (int32_t)((phase >> 23) & 255u), y = p*(256-p)*2;
    return phase & 0x80000000u ? -y : y;
}
/* Native 240x240 sequence, 48 frames. Two strips reuse the existing 124-row
 * canvas; no extra frame buffer, image decoder or audio-ISR animation work. */
static void vampire_boot(uint32_t frame)
{
    uint32_t f = frame > 47u ? 47u : frame, i, reveal = f < 20u ? f*32u/20u : 32u;
    int32_t x, prev = 0;
    uint16_t ink = vampire_boot_color(T_BG,T_TEXT,reveal);
    uint16_t red = vampire_boot_color(T_BG,T_THEME,f < 10u ? f*3u : 32u);
    cv_begin(240,124,T_BG);
    for (i=0;i<22u;i++) {
        int32_t sx = (int32_t)((i*67u+23u)%226u)+7;
        int32_t sy = (int32_t)((i*19u+11u)%50u)+5;
        cv_rect(sx,sy,1,1,((f+i*3u)&15u)<3u ? T_MID : T_DIM);
    }
    cv_rrect(192,12,23,23,11,T_SURF,T_BG);
    cv_rrect(185,7,23,23,11,T_BG,T_BG); /* crescent moon */
    vampire_bat(20+(int32_t)f*3,28+(vampire_wave(f*0x06000000u)*4>>15),3,f/2u,red);
    vampire_bat(220-(int32_t)f*2,44+(vampire_wave(f*0x08000000u)*3>>15),2,f/2u+4u,T_DIM);
    /* Crown and fangs settle over the gothic wordmark. */
    cv_line_t(109,31,113,45,red,2); cv_line_t(113,45,120,34,red,2);
    cv_line_t(120,34,127,45,red,2); cv_line_t(127,45,131,31,red,2);
    if (f > 10u && f < 28u) cv_rrect(119,49+(int32_t)(f-10u),3,5,1,red,T_BG);
    vampire_logo(17,65,ink);
    cv_blit(0,0);
    cv_begin(240,116,T_BG);
    cv_text_in(0,3,240,&AF_M,"(VAMPIRE) OS",red,T_BG);
    cv_text_in(0,29,240,&AF_S,"GROOVE / SAMPLE / SYNTH",T_MID,T_BG);
    for (x=14;x<226;x++) {
        uint32_t ph = (uint32_t)x*0x01900000u + f*0x06800000u;
        int32_t wave = (vampire_wave(ph)*10 + vampire_wave(ph*3u)*3) >> 15;
        int32_t env = x < 120 ? x-14 : 225-x;
        int32_t y = 65 + wave*env/106;
        if (x>14) {
            cv_line(x-1,prev+1,x,y+1,T_SURF);
            cv_line(x-1,prev,x,y,red);
        }
        prev = y;
    }
    cv_text_in(0,84,240,&AF_S,FELUCCA_VERSION,T_MID,T_BG);
    cv_rrect(18,108,204,3,1,T_SURF,T_BG);
    if (f) cv_rrect(18,108,(int32_t)(204u*f/47u),3,1,red,T_BG);
    cv_blit(0,124);
}
