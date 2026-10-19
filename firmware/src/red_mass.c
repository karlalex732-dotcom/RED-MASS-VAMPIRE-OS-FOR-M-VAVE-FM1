/* SPDX-License-Identifier: GPL-3.0-only
 * RED MASS: native screen, live data, six-track rail, no full framebuffer. */
static void rm_rail(int32_t top,uint32_t first,uint32_t end)
{
    char b[2]={0,0};for(uint32_t i=first;i<end;i++) {
        int32_t y=top+21*(int32_t)(i-first);uint16_t ink=T_TEXT;
        if(i==song.sel){cv_rect(207,y,25,19,T_THEME);ink=T_INK;}
        else {cv_rect(207,y,25,1,T_TEXT);cv_rect(207,y+18,25,1,T_TEXT);cv_rect(207,y,1,19,T_TEXT);cv_rect(231,y,1,19,T_TEXT);}
        b[0]=(char)('1'+i);cv_text_in(207,y+1,25,&AF_M,b,ink,i==song.sel?T_THEME:T_BG);
        if(trk[i].p[P_MUTE])cv_rect(209,y+17,4,1,ink);
    }
}
static void red_mass_home(void)
{
    char name[16],value[16],status[12];const char *unit;track_t *t=TSEL;
    draw_head();
    cv_begin(240,124,T_BG);
    rm_runs(9,11,RM_DIGIT[0],RM_DIGIT_0_H,T_TEXT);
    uint32_t digit=(song.sel%NTRK)+1u;
    rm_runs(53,11+(RM_DIGIT_0_H-RM_DIGIT_H[digit])/2,RM_DIGIT[digit],RM_DIGIT_H[digit],T_TEXT);
    cv_text(111,16,&AF_S,ENGINES[t->eng_req%NENGINES]->name,T_TEXT);
    param_format(track_desc(t,P_E0),t->p[P_E0],value,&unit);
    cv_free_text_fit(111,34,&AF_M,value,T_TEXT,T_BG,86);
    fmt_named=0;
    cv_rect(110,65,83,18,T_THEME);
    cv_icon_mid(114,74,16,song.playing?ICON_X_PLAY:ICON_X_STOP,T_INK,T_THEME);
    fmt_int(status,song.playing?t->seq_idx+1u:ui.cursor+1u);str_cpy(status+str_len(status),"/",sizeof status-str_len(status));fmt_int(status+str_len(status),t->p[P_SLEN]);
    cv_text_on(133,64,&AF_S,status,T_INK,T_THEME);
    sound_name(t,name);
    cv_free_text_fit(10,85,&AF_M,name,T_TEXT,T_BG,193);
    rm_rail(11,0,5);cv_blit(0,24);
    cv_begin(240,40,T_BG);
    uint32_t w=scope_w;int32_t peak=1800;
    for(uint32_t i=0;i<192u;i++){int32_t v=scope_buf[(w+i)&(SCOPE_N-1u)];if(v<0)v=-v;if(v>peak)peak=v;}
    for(uint32_t i=0;i<24u;i++) {
        int32_t amp=0;for(uint32_t j=0;j<8u;j++){int32_t v=scope_buf[(w+i*8u+j)&(SCOPE_N-1u)];if(v<0)v=-v;if(v>amp)amp=v;}
        amp=amp*14/peak;cv_rect(11+(int32_t)i*8,18-amp,4,amp?amp*2:1,T_TEXT);
    }
    rm_rail(7,5,6);
    if(ui.msg_t)cv_free_text_fit(9,24,&AF_S,ui.msg,T_TEXT,T_BG,191);
    else if(song.batt_raw&&batt_level()==0)cv_text(9,24,&AF_S,"LOW BATTERY",T_TEXT);
    cv_blit(0,148);
}
