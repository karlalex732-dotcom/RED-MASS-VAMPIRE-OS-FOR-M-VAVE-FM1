/* SPDX-License-Identifier: GPL-3.0-only */

static void upgrade_draw_columns(void)
{
    const page_t *pg=cur_page();uint32_t k=pg->id[0];char val[12];const char *unit;
    for(uint32_t c=0;c<4u;c++) {
        int16_t *vp;const param_desc_t *d=upgrade_page_desc(pg,c,&vp);
        if(!d||!vp) { if(UG_LABEL[k][c][0])draw_act_column(c,UG_LABEL[k][c],VAL(c),ICON_NONE);else draw_column(c,"","","",T_THEME,-1,ICON_NONE);continue; }
        param_format(d,*vp,val,&unit);
        if((k==UG_CTRL&&!c)||(k==UG_LOCK&&c==1u)||(k==UG_MACSET&&c==2u)) {
            uint32_t id=k==UG_CTRL ? UG_CTRL_IDS[ug_ui.ctrl] : k==UG_LOCK ? ug_ui.lock_param : upgrade.macro[song.sel][ug_ui.mac][ug_ui.target].param;
            str_cpy(val,track_desc(TSEL,id)->label,sizeof val);
        }
        if(k==UG_SONGROW&&c==1u&&!*vp)str_cpy(val,"PAT",sizeof val);
        if(k==UG_SONGROW&&c==2u&&!*vp)str_cpy(val,"KEEP",sizeof val);
        if((k==UG_SONGROW&&c==3u)||(k==UG_MUTES&&c==1u)) { for(uint32_t i=0;i<NTRK;i++)val[i]=(*vp>>i)&1u ? 'M' : '-';val[NTRK]=0; }
        if(k==UG_MICRO&&c==1u)unit="/32";
        if(k==UG_HISTORY&&c==2u)unit="/16";
        draw_column(c,d->label,val,unit,VAL(c),RATIO(d,*vp),ICON_NONE);
    }
}
static void upgrade_graph(void)
{
    uint32_t k=cur_page()->id[0];char a[32],b[32];
    if(k==UG_BANK){graph_browse();return;}
    if(k==UG_SONGROW||k==UG_SONGSET){graph_song();return;}
    if(k==UG_LOCK||k==UG_MICRO){graph_roll(TSEL,ACC);return;}
    if(k==UG_COPY){
        if(upgrade_clip.valid){str_cpy(a,"COPIED ",sizeof a);fmt_int(a+7,upgrade_clip.count);str_cpy(a+str_len(a)," STEPS",sizeof a-str_len(a));}
        else str_cpy(a,"CLIPBOARD EMPTY",sizeof a);
        panel_note(a,"[K2] COPY  [K3] PASTE","[OCT+] APPLY");return;
    }
    if(k==UG_CTRL){panel_note(upgrade_ctrl_valid ? "SNAPSHOT READY" : "TURN AMT TO CAPTURE","ALL SIX TRACKS","[OCT+] CAPTURE / RELOAD");return;}
    if(k==UG_MACRO||k==UG_MACSET){
        uint32_t m=k==UG_MACSET ? (uint32_t)ug_ui.mac : ui.hot_t ? ui.hot_col : 0;
        for(uint32_t i=0;i<4u;i++){
            const upgrade_macro_target_t *v=&upgrade.macro[song.sel][m][i];
            fmt_int(a,(int32_t)i+1);str_cpy(a+str_len(a)," ",sizeof a-str_len(a));str_cpy(a+str_len(a),v->amount ? track_desc(TSEL,v->param)->label : "--",sizeof a-str_len(a));
            fmt_int(b,v->amount);list_row(LIST_Y(i+1u),k==UG_MACSET&&i==(uint32_t)ug_ui.target,b,T_THEME,a,T_TEXT,232);
        }return;
    }
    if(k==UG_MUTES){panel_note("8 STORED MUTE GROUPS","M = MUTED TRACK 1..6","[OCT+] CAPTURE / RECALL");return;}
    if(k==UG_DRUM){panel_note(GROOVE_PAD[ui.lane],"PAD CHANCE + STEP CHANCE","[OCT+] APPLY ALL PADS");return;}
    if(k==UG_HISTORY){fmt_int(a,upgrade_history.pos);str_cpy(a+str_len(a)," OF ",sizeof a-str_len(a));fmt_int(a+str_len(a),upgrade_history.count);panel_note(a,"16 STOPPED STEP EDITS","[OCT+] UNDO / REDO");return;}
    if(k==UG_MELODY){panel_note("ROOT + SCALE SAFE","SEED REPEATS THE MELODY","[OCT+] GENERATE");return;}
    if(k==UG_MUTATE){panel_note("PITCH + LEVEL PROTECTED","SEED REPEATS THE MUTATION","[OCT+] APPLY / UNDO");return;}
    panel_note(UG_FEATURES[ug_ui.tool],"[K1] CHOOSE TOOL","[OCT+] OPEN");
}
